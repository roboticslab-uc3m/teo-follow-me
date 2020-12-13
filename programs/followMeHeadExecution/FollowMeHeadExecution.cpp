// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeHeadExecution.hpp"

namespace roboticslab
{

/************************************************************************/

const std::string FollowMeHeadExecution::defaultRobot = "/teo";

/************************************************************************/

bool FollowMeHeadExecution::configure(yarp::os::ResourceFinder &rf)
{
    std::string robot = rf.check("robot",yarp::os::Value(defaultRobot),"name of /robot to be used").asString();

    printf("--------------------------------------------------------------\n");
    printf("FollowMeHeadExecution options:\n");
    printf("\t--help (this help)\t--from [file.ini]\t--context [path]\n");
    printf("\t--robot: %s [%s]\n",robot.c_str(),defaultRobot.c_str());
    printf("--------------------------------------------------------------\n");

    std::string followMeHeadExecutionStr("/followMeHeadExecution");

    yarp::os::Property headOptions;
    headOptions.put("device","remote_controlboard");
    headOptions.put("remote",robot+"/head");
    headOptions.put("local",followMeHeadExecutionStr+robot+"/head");
    headDevice.open(headOptions);
    if( ! headDevice.isValid() )
    {
        printf("head remote_controlboard instantiation not worked.\n");
        return false;
    }

    if (!headDevice.view(headIControlMode) ) // connecting our device with "control mode" interface, initializing which control mode we want (position)
    {
        printf("[warning] Problems acquiring headIControlMode interface\n");
        return false;
    }
    printf("[success] Acquired headIControlMode interface\n");

    if (!headDevice.view(headIPositionControl) ) // connecting our device with "position control 2" interface (configuring our device: speed, acceleration... and sending joint positions)
    {
        printf("[warning] Problems acquiring headIPositionControl interface\n");
        return false;
    }
    printf("[success] Acquired headIPositionControl interface\n");

    if( ! headDevice.view(iEncoders) )
    {
        printf("view(iEncoders) not worked.\n");
        return false;
    }

    //-- Set control mode
    int headAxes;
    headIPositionControl->getAxes(&headAxes);
    std::vector<int> headControlModes(headAxes,VOCAB_CM_POSITION);
    if(! headIControlMode->setControlModes( headControlModes.data() ))
    {
        printf("[warning] Problems setting position control mode of: head\n");
        return false;
    }

    // -- Configure Speed and Acc
    std::vector<double> speed(2, 30);
    std::vector<double> acc(2, 30);

    if(!headIPositionControl->setRefSpeeds(speed.data()))
    {
        printf("[ERROR] Problems setting reference speed on head joints.\n");
        return false;
    }

    if(!headIPositionControl->setRefAccelerations(acc.data()))
    {
        printf("[ERROR] Problems setting reference acc on head joints.\n");
        return false;
    }

    inCvPort.setIPositionControl(headIPositionControl);
    inDialoguePortProcessor.setIEncoders(iEncoders);

    //-----------------OPEN LOCAL PORTS------------//
    inDialoguePortProcessor.setInCvPortPtr(&inCvPort);
    inCvPort.useCallback();
    inDialoguePort.setReader(inDialoguePortProcessor);
    inDialoguePort.open("/followMeHeadExecution/dialogueManager/rpc:s");
    inCvPort.open("/followMeHeadExecution/cv/state:i");

    while(0 == inCvPort.getInputCount())
    {
        if(isStopping())
            return false;
        printf("Waiting for \"/followMeHeadExecution/cv/state:i\" to be connected to vision...\n");
        yarp::os::Time::delay(0.5);
    }

    return true;
}

/************************************************************************/

double FollowMeHeadExecution::getPeriod()
{
    return 2.0;  // Fixed, in seconds, the slow thread that calls updateModule below
}

/************************************************************************/
bool FollowMeHeadExecution::updateModule()
{
    //printf("StateMachine in state [%d]. FollowMeHeadExecution alive...\n", stateMachine.getMachineState());
    return true;
}

/************************************************************************/

bool FollowMeHeadExecution::interruptModule()
{
    printf("FollowMeHeadExecution closing...\n");
    inCvPort.disableCallback();
    inCvPort.interrupt();
    inDialoguePort.interrupt();
    inCvPort.close();
    inDialoguePort.close();
    return true;
}

/************************************************************************/

} // namespace roboticslab
