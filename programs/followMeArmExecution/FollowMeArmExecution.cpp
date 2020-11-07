// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeArmExecution.hpp"

#include "../FollowMeVocabs.hpp"

namespace roboticslab
{

/************************************************************************/

const yarp::conf::vocab32_t FollowMeArmExecution::VOCAB_STATE_SALUTE = yarp::os::createVocab('s','a','l','u');
const yarp::conf::vocab32_t FollowMeArmExecution::VOCAB_STATE_ARM_SWINGING  = yarp::os::createVocab('s','w','i','n');
const yarp::conf::vocab32_t FollowMeArmExecution::VOCAB_STATE_SIGNALIZE_RIGHT = yarp::os::createVocab('s','i','g','r');
const yarp::conf::vocab32_t FollowMeArmExecution::VOCAB_STATE_SIGNALIZE_LEFT = yarp::os::createVocab('s','i','g','l');

/************************************************************************/

bool FollowMeArmExecution::configure(yarp::os::ResourceFinder &rf)
{
    std::string robot = rf.check("robot",yarp::os::Value(DEFAULT_ROBOT),"name of /robot to be used").asString();

    printf("--------------------------------------------------------------\n");
    if (rf.check("help"))
    {
        printf("FollowMeArmExecution options:\n");
        printf("\t--help (this help)\t--from [file.ini]\t--context [path]\n");
        printf("\t--robot: %s [%s]\n",robot.c_str(),DEFAULT_ROBOT);
        ::exit(0);
    }

    state = VOCAB_STATE_ARM_SWINGING;

    std::string followMeArmExecutionStr("/followMeArmExecution");

    // ------ LEFT ARM -------
    yarp::os::Property leftArmOptions;
    leftArmOptions.put("device","remote_controlboard");
    leftArmOptions.put("remote",robot+"/leftArm");
    leftArmOptions.put("local",followMeArmExecutionStr+robot+"/leftArm");
    leftArmDevice.open(leftArmOptions);
    if(!leftArmDevice.isValid()) {
        printf("robot leftArm device not available.\n");
        leftArmDevice.close();
        yarp::os::Network::fini();
        return false;
    }

    if (!leftArmDevice.view(leftArmIControlMode) ) { // connecting our device with "control mode" interface, initializing which control mode we want (position)
        printf("[warning] Problems acquiring leftArmIControlMode interface\n");
        return false;
    } else printf("[success] Acquired leftArmIControlMode interface\n");

    if (!leftArmDevice.view(leftArmIPositionControl) ) { // connecting our device with "position control 2" interface (configuring our device: speed, acceleration... and sending joint positions)
        printf("[warning] Problems acquiring leftArmIPositionControl interface\n");
        return false;
    } else printf("[success] Acquired leftArmIPositionControl interface\n");

    // ------ RIGHT ARM -------
    yarp::os::Property rightArmOptions;
    rightArmOptions.put("device","remote_controlboard");
    rightArmOptions.put("remote",robot+"/rightArm");
    rightArmOptions.put("local",followMeArmExecutionStr+robot+"/rightArm");
    rightArmDevice.open(rightArmOptions);
    if(!rightArmDevice.isValid()) {
        printf("robot rightArm device not available.\n");
        rightArmDevice.close();
        yarp::os::Network::fini();
        return false;
    }

    if (!rightArmDevice.view(rightArmIControlMode) ) { // connecting our device with "control mode" interface, initializing which control mode we want (position)
        printf("[warning] Problems acquiring rightArmIControlMode interface\n");
        return false;
    } else printf("[success] Acquired rightArmIControlMode interface\n");


    if ( ! rightArmDevice.view(rightArmIPositionControl) ) {
        printf("[warning] Problems acquiring rightArmIPositionControl interface\n");
        return false;
    } else printf("[success] Acquired rightArmIPositionControl interface\n");

    //-- Set control modes for both arms

    int leftArmAxes;
    leftArmIPositionControl->getAxes(&leftArmAxes);
    std::vector<int> leftArmControlModes(leftArmAxes,VOCAB_CM_POSITION);
    if(! leftArmIControlMode->setControlModes( leftArmControlModes.data() )) {
        printf("[warning] Problems setting position control mode of: left-arm\n");
        return false;
    }

    int rightArmAxes;
    rightArmIPositionControl->getAxes(&rightArmAxes);
    std::vector<int> rightArmControlModes(rightArmAxes,VOCAB_CM_POSITION);
    if(! rightArmIControlMode->setControlModes(rightArmControlModes.data())) {
        printf("[warning] Problems setting position control mode of: right-arm\n");
        return false;
    }

    phase = false;

    inDialogPort.open("/followMeArmExecution/dialogueManager/rpc:s");
    inDialogPort.setReader(*this);  //-- Callback reader: avoid need to call inSrPort.read().

    return this->start();  //-- Start the thread (calls run).
}

/************************************************************************/

bool FollowMeArmExecution::interruptModule()
{
    this->stop();
    inDialogPort.interrupt();
    leftArmDevice.close();
    return true;
}

/************************************************************************/

double FollowMeArmExecution::getPeriod()
{
    return 4.0; // Fixed, in seconds, the slow thread that calls updateModule below
}

/************************************************************************/

bool FollowMeArmExecution::updateModule()
{
    printf("Entered updateModule...\n");

    return true;
}

/************************************************************************/

bool FollowMeArmExecution::armJointsMoveAndWait(std::vector<double>& leftArmQ, std::vector<double> &rightArmQ)
{
    // -- Configuring Speeds and Accelerations
    int armAxes;
    rightArmIPositionControl->getAxes(&armAxes); // number of axes is the same in both arms

    std::vector<double> armSpeeds(armAxes,30.0);
    std::vector<double> armAccelerations(armAxes,30.0);

    rightArmIPositionControl->setRefSpeeds(armSpeeds.data());
    leftArmIPositionControl->setRefSpeeds(armSpeeds.data());
    rightArmIPositionControl->setRefAccelerations(armAccelerations.data());
    leftArmIPositionControl->setRefAccelerations(armAccelerations.data());
    rightArmIPositionControl->positionMove( rightArmQ.data() );
    leftArmIPositionControl->positionMove( leftArmQ.data() );

    //printf("Waiting for right arm.");
    bool doneRight = false;
    bool doneLeft = false;
    while((!doneRight)&&(!Thread::isStopping()))
    {
        rightArmIPositionControl->checkMotionDone(&doneRight);
        yarp::os::Time::delay(0.1);
    }

    while((!doneLeft)&&(!Thread::isStopping()))
    {
        leftArmIPositionControl->checkMotionDone(&doneLeft);
        yarp::os::Time::delay(0.1);
    }

    //printf("\n");
    return true;
}

/************************************************************************/

bool FollowMeArmExecution::read(yarp::os::ConnectionReader& connection)
{
    yarp::os::Bottle b;
    b.read(connection);
    // process data in b
    printf("[FollowMeArmExecution] Got %s\n", b.toString().c_str());
    if( (VOCAB_FOLLOW_ME ==b.get(0).asVocab()) || (VOCAB_STATE_SALUTE ==b.get(0).asVocab()) )
    {
        state = VOCAB_STATE_SALUTE;
    }
    else if (VOCAB_STOP_FOLLOWING ==b.get(0).asVocab())
        state = VOCAB_STOP_FOLLOWING;

    else if (VOCAB_STATE_SIGNALIZE_RIGHT == b.get(0).asVocab())
        state = VOCAB_STATE_SIGNALIZE_RIGHT;

    else if (VOCAB_STATE_SIGNALIZE_LEFT == b.get(0).asVocab())
        state = VOCAB_STATE_SIGNALIZE_LEFT;

    return true;
}

/************************************************************************/

void FollowMeArmExecution::run()
{
    while( !Thread::isStopping() )
    {
        switch (state)
        {
        case VOCAB_STATE_ARM_SWINGING:
            if(phase)
            {
                printf("Phase: true\n");
                std::vector<double> leftArmQ(7,0.0);
                leftArmQ[0] = 20;
                leftArmQ[1] = 5;
                std::vector<double> rightArmQ(7,0.0);
                rightArmQ[1] = -5;
                rightArmQ[0] = -20;
                armJointsMoveAndWait(leftArmQ,rightArmQ);
                phase = false;
            }
            else
            {
                printf("Phase: false\n");
                std::vector<double> leftArmQ(7,0.0);
                leftArmQ[0] = -20;
                leftArmQ[1] = 5;
                std::vector<double> rightArmQ(7,0.0);
                rightArmQ[1] = -5;
                rightArmQ[0] = 20;
                armJointsMoveAndWait(leftArmQ,rightArmQ);
                phase = true;
            }
            break;

        case VOCAB_STATE_SALUTE:
            printf("Salute\n");
            {
                std::vector<double> leftArmQ(7,0.0);
                std::vector<double> rightArmQ(7,0.0);
                leftArmQ[1] = 4;    // Tray security position
                rightArmQ[0] = -45;
                rightArmQ[2] = -20;
                rightArmQ[3] = -80;
                armJointsMoveAndWait(leftArmQ,rightArmQ);
            }
            {
                std::vector<double> leftArmQ(7,0.0);
                std::vector<double> rightArmQ(7,0.0);
                leftArmQ[1] = 4;    // Tray security position
                rightArmQ[0] = -45;
                rightArmQ[2] = -20;
                rightArmQ[3] = -80;
                armJointsMoveAndWait(leftArmQ,rightArmQ);
            }
            {
                std::vector<double> leftArmQ(7,0.0);
                std::vector<double> rightArmQ(7,0.0);
                leftArmQ[1] = 4;    // Tray security position
                rightArmQ[0] = -45;
                rightArmQ[2] = -20;
                rightArmQ[3] = -80;
                armJointsMoveAndWait(leftArmQ,rightArmQ);
            }
            state = VOCAB_STATE_ARM_SWINGING;
            break;

        case VOCAB_STATE_SIGNALIZE_RIGHT:
            printf("Signalize right\n");
            {
                std::vector<double> leftArmQ(7,0.0);
                std::vector<double> rightArmQ(7,0.0);
                leftArmQ[1] = 4;    // Tray security position
                rightArmQ[0] = -50;
                rightArmQ[1] = -20;
                rightArmQ[2] = 10;
                rightArmQ[3] = -70;
                rightArmQ[4] = 20;
                rightArmQ[5] = -40;
                armJointsMoveAndWait(leftArmQ,rightArmQ);
            }
            {
                std::vector<double> leftArmQ(7,0.0);
                std::vector<double> rightArmQ(7,0.0);
                leftArmQ[1] = 4;    // Tray security position
                rightArmQ[0] = -50;
                rightArmQ[1] = -20;
                rightArmQ[2] = 10;
                rightArmQ[3] = -70;
                rightArmQ[4] = 20;
                rightArmQ[5] = 0;
                armJointsMoveAndWait(leftArmQ,rightArmQ);
            }
            state = VOCAB_STATE_ARM_SWINGING;
            break;

        case VOCAB_STATE_SIGNALIZE_LEFT:
            printf("Signalize left\n");
            {
                std::vector<double> leftArmQ(7,0.0);
                std::vector<double> rightArmQ(7,0.0);
                leftArmQ[0] = -50;
                leftArmQ[1] = 20;
                leftArmQ[2] = -10;
                leftArmQ[3] = -70;
                leftArmQ[4] = -20;
                leftArmQ[5] = -40;
                armJointsMoveAndWait(leftArmQ,rightArmQ);
            }
            {
                std::vector<double> leftArmQ(7,0.0);
                std::vector<double> rightArmQ(7,0.0);
                leftArmQ[0] = -50;
                leftArmQ[1] = 20;
                leftArmQ[2] = -10;
                leftArmQ[3] = -70;
                leftArmQ[4] = -20;
                leftArmQ[5] = 0;
                armJointsMoveAndWait(leftArmQ,rightArmQ);
            }
            state = VOCAB_STATE_ARM_SWINGING;
            break;

        case VOCAB_STOP_FOLLOWING:
            printf("Stop Following\n");
            {
                std::vector<double> leftArmQ(7,0.0);
                std::vector<double> rightArmQ(7,0.0);
                leftArmQ[1] = 4;    // Tray security position
                armJointsMoveAndWait(leftArmQ,rightArmQ);
            }
            break;

        default:
            printf("Bad state!\n");
            break;
        }
    }

    return;
}

/************************************************************************/

} // namespace roboticslab
