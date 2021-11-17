// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeArmExecution.hpp"

#include <yarp/os/LogStream.h>
#include <yarp/os/Property.h>

using namespace roboticslab;

constexpr auto DEFAULT_ROBOT = "/teo";
constexpr auto DEFAULT_PREFIX = "/followMeArmExecution";
constexpr auto DEFAULT_REF_SPEED = 30.0;
constexpr auto DEFAULT_REF_ACCELERATION = 30.0;

bool FollowMeArmExecution::configure(yarp::os::ResourceFinder & rf)
{
    auto robot = rf.check("robot", yarp::os::Value(DEFAULT_ROBOT), "remote robot port prefix").asString();
    auto armSpeed = rf.check("armSpeed", yarp::os::Value(DEFAULT_REF_SPEED), "arm speed").asFloat64();

    if (rf.check("help"))
    {
        yInfo("FollowMeArmExecution options:");
        yInfo("\t--help (this help)\t--from [file.ini]\t--context [path]");
        yInfo("\t--robot: %s [%s]", robot.c_str(), DEFAULT_ROBOT);
        yInfo("\t--armSpeed: %f [%f]", armSpeed, DEFAULT_REF_SPEED);
        return false;
    }

    // ------ LEFT ARM -------

    yarp::os::Property leftArmOptions {
        {"device", yarp::os::Value("remote_controlboard")},
        {"remote", yarp::os::Value(robot + "/leftArm")},
        {"local", yarp::os::Value(std::string(DEFAULT_PREFIX) + "/leftArm")}
    };

    if (!leftArmDevice.open(leftArmOptions))
    {
        yError() << "Failed to open left arm device";
        return false;
    }

    if (!leftArmDevice.view(leftArmIControlMode) || !leftArmDevice.view(leftArmIPositionControl))
    {
        yError() << "Failed to view left arm device interfaces";
        return false;
    }

    // ------ RIGHT ARM -------

    yarp::os::Property rightArmOptions {
        {"device", yarp::os::Value("remote_controlboard")},
        {"remote", yarp::os::Value(robot + "/rightArm")},
        {"local", yarp::os::Value(std::string(DEFAULT_PREFIX) + "/rightArm")}
    };

    if (!rightArmDevice.open(rightArmOptions))
    {
        yError() << "Failed to open right arm device";
        return false;
    }

    if (!rightArmDevice.view(rightArmIControlMode) || !rightArmDevice.view(rightArmIPositionControl))
    {
        yError() << "Failed to view right arm device interfaces";
        return false;
    }

    //-- Set control modes for both arms

    int leftArmAxes;
    leftArmIPositionControl->getAxes(&leftArmAxes);

    if (!leftArmIControlMode->setControlModes(std::vector<int>(leftArmAxes, VOCAB_CM_POSITION).data()))
    {
        yError() << "Failed to set position control mode for left arm";
        return false;
    }

    int rightArmAxes;
    rightArmIPositionControl->getAxes(&rightArmAxes);

    if (!rightArmIControlMode->setControlModes(std::vector<int>(rightArmAxes, VOCAB_CM_POSITION).data()))
    {
        yError() << "Failed to set position control mode for right arm";
        return false;
    }

    // -- Configuring reference speeds and accelerations

    if (!leftArmIPositionControl->setRefSpeeds(std::vector<double>(leftArmAxes, DEFAULT_REF_SPEED).data()))
    {
        yError() << "Failed to set reference speeds for left arm";
        return false;
    }

    if (!rightArmIPositionControl->setRefSpeeds(std::vector<double>(rightArmAxes, DEFAULT_REF_SPEED).data()))
    {
        yError() << "Failed to set reference speeds for right arm";
        return false;
    }

    if (!leftArmIPositionControl->setRefAccelerations(std::vector<double>(leftArmAxes, DEFAULT_REF_ACCELERATION).data()))
    {
        yError() << "Failed to set reference accelerations for left arm";
        return false;
    }

    if (!rightArmIPositionControl->setRefAccelerations(std::vector<double>(rightArmAxes, DEFAULT_REF_ACCELERATION).data()))
    {
        yError() << "Failed to set reference accelerations for right arm";
        return false;
    }

    if (!serverPort.open(DEFAULT_PREFIX + std::string("/dialogueManager/rpc:s")))
    {
        yError() << "Failed to open dialogue manager port" << serverPort.getName();
        return false;
    }

    yarp::os::Wire::yarp().attachAsServer(serverPort);
    return true;
}

bool FollowMeArmExecution::interruptModule()
{
    serverPort.interrupt();
    return stop();
}

bool FollowMeArmExecution::close()
{
    serverPort.close();
    leftArmDevice.close();
    rightArmDevice.close();
    return true;
}

double FollowMeArmExecution::getPeriod()
{
    return 0.1; // [s]
}

bool FollowMeArmExecution::updateModule()
{
    bool isMotionDone = checkMotionDone();
    std::unique_lock<std::mutex> lock(actionMutex);

    yDebugThrottle(1.0) << "Current action:" << getStateDescription(currentState);

    if (hasNewSetpoints || (isMotionDone && !currentSetpoints.empty()))
    {
        auto setpoints = currentSetpoints.front();
        currentSetpoints.pop_front();
        hasNewSetpoints = false;
        lock.unlock();

        if (!leftArmIPositionControl->positionMove(std::get<0>(setpoints).data()))
        {
            yWarning() << "Failed to send new setpoints to left arm";
        }

        if (!rightArmIPositionControl->positionMove(std::get<1>(setpoints).data()))
        {
            yWarning() << "Failed to send new setpoints to right arm";
        }
    }
    else if (!hasNewSetpoints && isMotionDone && currentSetpoints.empty())
    {
        // motion done and no more points to send

        switch (currentState)
        {
        case state::GREET:
        case state::SIGNAL_LEFT:
        case state::SIGNAL_RIGHT:
        case state::SWING:
            lock.unlock(); // avoid deadlock due to the next call
            enableArmSwinging(); // send moar points!
            break;
        case state::HOMING:
            currentState = state::REST;
            break;
        case state::REST:
            // just stay calm
            break;
        }
    }

    return true;
}

void FollowMeArmExecution::doGreet()
{
    registerSetpoints(state::GREET, {
        {armZeros, {-45.0, 0.0, -20.0, -80.0, 0.0, 0.0}},
    });
}

void FollowMeArmExecution::doSignalLeft()
{
    registerSetpoints(state::SIGNAL_LEFT, {
        {{-50.0, 20.0, -10.0, -70.0, -20.0, -40.0}, armZeros},
        {{-50.0, 20.0, -10.0, -70.0, -20.0, 0.0}, armZeros},
    });
}

void FollowMeArmExecution::doSignalRight()
{
    registerSetpoints(state::SIGNAL_RIGHT, {
        {armZeros, {-50.0, 20.0, -10.0, -70.0, -20.0, -40.0}},
        {armZeros, {-50.0, 20.0, -10.0, -70.0, -20.0, 0.0}},
    });
}

void FollowMeArmExecution::enableArmSwinging()
{
    registerSetpoints(state::SWING, {
        {{20.0, 5.0, 0.0, 0.0, 0.0, 0.0}, {-20.0, -5.0, 0.0, 0.0, 0.0, 0.0}},
        {{-20.0, 5.0, 0.0, 0.0, 0.0, 0.0}, {20.0, -5.0, 0.0, 0.0, 0.0, 0.0}},
    });
}

void FollowMeArmExecution::disableArmSwinging()
{
    registerSetpoints(state::HOMING, {
        {armZeros, armZeros}
    });
}

bool FollowMeArmExecution::stop()
{
    yInfo() << "Received stop command";

    {
        std::lock_guard<std::mutex> lock(actionMutex);
        currentState = state::REST;
        currentSetpoints.clear();
        hasNewSetpoints = false;
    }

    bool ok = true;

    if (!leftArmIPositionControl->stop())
    {
        yError() << "Failed to stop left arm";
        ok = false;
    }

    if (!rightArmIPositionControl->stop())
    {
        yError() << "Failed to stop right arm";
        ok = false;
    }

    return ok;
}

void FollowMeArmExecution::registerSetpoints(state newState, std::initializer_list<setpoints_t> setpoints)
{
    yInfo() << "Registered new action:" << getStateDescription(newState);

    std::lock_guard<std::mutex> lock(actionMutex);
    currentState = newState;
    currentSetpoints.clear();
    currentSetpoints.insert(currentSetpoints.end(), setpoints);
    hasNewSetpoints = true;
}

bool FollowMeArmExecution::checkMotionDone()
{
    bool leftArmDone = true;

    if (!leftArmIPositionControl->checkMotionDone(&leftArmDone))
    {
        yWarning() << "Unable to check motion state of left arm";
    }

    bool rightArmDone = true;

    if (!rightArmIPositionControl->checkMotionDone(&rightArmDone))
    {
        yWarning() << "Unable to check motion state of right arm";
    }

    return leftArmDone && rightArmDone;
}

std::string FollowMeArmExecution::getStateDescription(state s)
{
    switch (s)
    {
        case state::GREET:
            return "greet";
        case state::SIGNAL_LEFT:
            return "signal left";
        case state::SIGNAL_RIGHT:
            return "signal right";
        case state::SWING:
            return "swinging arms";
        case state::HOMING:
            return "homing";
        case state::REST:
            return "none";
        default:
            return "unknown";
    }
}
