// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeArmExecution.hpp"

namespace teo
{

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

    if (!leftArmDevice.view(leftArmIControlMode2) ) { // connecting our device with "control mode 2" interface, initializing which control mode we want (position)
        printf("[warning] Problems acquiring leftArmIControlMode2 interface\n");
        return false;
    } else printf("[success] Acquired leftArmIControlMode2 interface\n");

    if (!leftArmDevice.view(leftArmIPositionControl2) ) { // connecting our device with "position control 2" interface (configuring our device: speed, acceleration... and sending joint positions)
        printf("[warning] Problems acquiring leftArmIPositionControl2 interface\n");
        return false;
    } else printf("[success] Acquired leftArmIPositionControl2 interface\n");

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

    if (!rightArmDevice.view(rightArmIControlMode2) ) { // connecting our device with "control mode 2" interface, initializing which control mode we want (position)
        printf("[warning] Problems acquiring rightArmIControlMode2 interface\n");
        return false;
    } else printf("[success] Acquired rightArmIControlMode2 interface\n");


    if ( ! rightArmDevice.view(rightArmIPositionControl2) ) {
        printf("[warning] Problems acquiring rightArmIPositionControl2 interface\n");
        return false;
    } else printf("[success] Acquired rightArmIPositionControl2 interface\n");

    //-- Set control modes for both arms

    int leftArmAxes;
    leftArmIPositionControl2->getAxes(&leftArmAxes);
    std::vector<int> leftArmControlModes(leftArmAxes,VOCAB_CM_POSITION);
    if(! leftArmIControlMode2->setControlModes( leftArmControlModes.data() )){
        printf("[warning] Problems setting position control mode of: left-arm\n");
        return false;
    }

    int rightArmAxes;
    rightArmIPositionControl2->getAxes(&rightArmAxes);
    std::vector<int> rightArmControlModes(rightArmAxes,VOCAB_CM_POSITION);
    if(! rightArmIControlMode2->setControlModes(rightArmControlModes.data())){
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
    rightArmIPositionControl2->getAxes(&armAxes); // number of axes is the same in both arms

    std::vector<double> armSpeeds(armAxes,30.0);
    std::vector<double> armAccelerations(armAxes,30.0);

    rightArmIPositionControl2->setRefSpeeds(armSpeeds.data());
    leftArmIPositionControl2->setRefSpeeds(armSpeeds.data());
    rightArmIPositionControl2->setRefAccelerations(armAccelerations.data());
    leftArmIPositionControl2->setRefAccelerations(armAccelerations.data());
    rightArmIPositionControl2->positionMove( rightArmQ.data() );
    leftArmIPositionControl2->positionMove( leftArmQ.data() );

    //printf("Waiting for right arm.");
    bool doneRight = false;
    bool doneLeft = false;
    while((!doneRight)&&(!Thread::isStopping()))
    {
        rightArmIPositionControl2->checkMotionDone(&doneRight);
        yarp::os::Time::delay(0.1);
    }

    while((!doneLeft)&&(!Thread::isStopping()))
    {
         leftArmIPositionControl2->checkMotionDone(&doneLeft);
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
                leftArmQ[3] = 70;
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

}  // namespace teo

