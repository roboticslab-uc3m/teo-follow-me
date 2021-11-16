// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeHeadExecution.hpp"

#include <yarp/os/LogStream.h>
#include <yarp/os/Property.h>
#include <yarp/os/SystemClock.h>

using namespace roboticslab;

constexpr auto DEFAULT_ROBOT = "/teo";
constexpr auto DEFAULT_PREFIX = "/followMeHeadExecution";
constexpr auto DEFAULT_REF_SPEED = 30.0;
constexpr auto DEFAULT_REF_ACCELERATION = 30.0;

bool FollowMeHeadExecution::configure(yarp::os::ResourceFinder &rf)
{
    auto robot = rf.check("robot", yarp::os::Value(DEFAULT_ROBOT), "remote robot port prefix").asString();

    if (rf.check("help"))
    {
        yInfo("FollowMeHeadExecution options:");
        yInfo("\t--help (this help)\t--from [file.ini]\t--context [path]");
        yInfo("\t--robot: %s [%s]", robot.c_str(), DEFAULT_ROBOT);
        return false;
    }

    yarp::os::Property headOptions {
        {"device", yarp::os::Value("remote_controlboard")},
        {"remote", yarp::os::Value(robot + "/head")},
        {"local", yarp::os::Value(DEFAULT_PREFIX + std::string("/head"))}
    };

    if (!headDevice.open(headOptions))
    {
        yError() << "Failed to open head device";
        return false;
    }

    if (!headDevice.view(headIControlMode) || !headDevice.view(iEncoders) || !headDevice.view(headIPositionControl))
    {
        yError() << "Failed to view head device interfaces";
        return false;
    }

    //-- Set control mode

    int headAxes;
    headIPositionControl->getAxes(&headAxes);

    if (!headIControlMode->setControlModes(std::vector<int>(headAxes, VOCAB_CM_POSITION).data()))
    {
        yError() << "Failed to set position control mode";
        return false;
    }

    // -- Configure reference speeds and accelerations

    if (!headIPositionControl->setRefSpeeds(std::vector<double>(headAxes, DEFAULT_REF_SPEED).data()))
    {
        yError() << "Failed to set reference speeds";
        return false;
    }

    if (!headIPositionControl->setRefAccelerations(std::vector<double>(headAxes, DEFAULT_REF_ACCELERATION).data()))
    {
        yError() << "Failed to set reference accelerations";
        return false;
    }

    inCvPort.setIPositionControl(headIPositionControl);
    inDialoguePortProcessor.setIEncoders(iEncoders);

    //-----------------OPEN LOCAL PORTS------------//

    inDialoguePortProcessor.setInCvPortPtr(&inCvPort);
    inCvPort.useCallback();
    inDialoguePort.setReader(inDialoguePortProcessor);

    if (!inDialoguePort.open(DEFAULT_PREFIX + std::string("/dialogueManager/rpc:s")))
    {
        yError() << "Failed to open inDialoguePort" << inDialoguePort.getName();
        return false;
    }

    if (!inCvPort.open(DEFAULT_PREFIX + std::string("/cv/state:i")))
    {
        yError() << "Failed to open inCvPort" << inCvPort.getName();
        return false;
    }

    while (inCvPort.getInputCount() == 0)
    {
        if (isStopping())
        {
            return false;
        }

        yInfo() << "Waiting for" << inCvPort.getName() << "to be connected to vision...";
        yarp::os::SystemClock::delaySystem(0.5);
    }

    return true;
}

double FollowMeHeadExecution::getPeriod()
{
    return 2.0; // [s]
}

bool FollowMeHeadExecution::updateModule()
{
    return true;
}

bool FollowMeHeadExecution::interruptModule()
{
    inCvPort.disableCallback();
    inCvPort.interrupt();
    inDialoguePort.interrupt();
    return true;
}

bool FollowMeHeadExecution::close()
{
    inDialoguePort.close();
    inCvPort.close();
    headDevice.close();
    return true;
}
