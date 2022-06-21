// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeArmExecution.hpp"

#include <string>
#include <vector>

#include <yarp/os/LogStream.h>
#include <yarp/os/Property.h>

using namespace roboticslab;

constexpr auto DEFAULT_ROBOT = "/teo";
constexpr auto DEFAULT_PREFIX = "/followMeArmExecution";
constexpr auto DEFAULT_REF_SPEED = 30.0;
constexpr auto DEFAULT_REF_ACCELERATION = 30.0;

constexpr FollowMeArmExecution::setpoints_arm_t armZeros {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

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

    yarp::os::Property armsOptions {
        {"device", yarp::os::Value("remotecontrolboardremapper")},
        {"localPortPrefix", yarp::os::Value(DEFAULT_PREFIX)}
    };

    yarp::os::Bottle remotePorts {
        yarp::os::Value(robot + "/leftArm"),
        yarp::os::Value(robot + "/rightArm")
    };

    armsOptions.put("remoteControlBoards", yarp::os::Value::makeList(remotePorts.toString().c_str()));

    yarp::os::Bottle axesNames {
        yarp::os::Value("FrontalLeftShoulder"), yarp::os::Value("SagittalLeftShoulder"), yarp::os::Value("AxialLeftShoulder"),
        yarp::os::Value("FrontalLeftElbow"), yarp::os::Value("AxialLeftWrist"), yarp::os::Value("FrontalLeftWrist"),
        yarp::os::Value("FrontalRightShoulder"), yarp::os::Value("SagittalRightShoulder"), yarp::os::Value("AxialRightShoulder"),
        yarp::os::Value("FrontalRightElbow"), yarp::os::Value("AxialRightWrist"), yarp::os::Value("FrontalRightWrist")
    };

    armsOptions.put("axesNames", yarp::os::Value::makeList(axesNames.toString().c_str()));

    if (!armsDevice.open(armsOptions))
    {
        yError() << "Failed to open arms device";
        return false;
    }

    if (!armsDevice.view(armsIControlMode) || !armsDevice.view(armsIPositionControl))
    {
        yError() << "Failed to view arms device interfaces";
        return false;
    }

    if (!armsIControlMode->setControlModes(std::vector(axesNames.size(), VOCAB_CM_POSITION).data()))
    {
        yError() << "Failed to set position control mode for arms";
        return false;
    }

    if (!armsIPositionControl->setRefSpeeds(std::vector(axesNames.size(), DEFAULT_REF_SPEED).data()))
    {
        yError() << "Failed to set reference speeds for arms";
        return false;
    }

    if (!armsIPositionControl->setRefAccelerations(std::vector(axesNames.size(), DEFAULT_REF_ACCELERATION).data()))
    {
        yError() << "Failed to set reference accelerations for arms";
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
    armsDevice.close();
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
        auto [leftArm, rightArm] = currentSetpoints.front();
        currentSetpoints.pop_front();
        hasNewSetpoints = false;
        lock.unlock();

        std::vector<double> values;
        values.insert(values.end(), leftArm.begin(), leftArm.end());
        values.insert(values.end(), rightArm.begin(), rightArm.end());

        if (!armsIPositionControl->positionMove(values.data()))
        {
            yWarning() << "Failed to send new setpoints to arms";
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

    if (!armsIPositionControl->stop())
    {
        yError() << "Failed to stop arms";
        return false;
    }

    return true;
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
    bool motionDone = true;

    if (!armsIPositionControl->checkMotionDone(&motionDone))
    {
        yWarning() << "Unable to check motion state of arms";
    }

    return motionDone;
}

const char * FollowMeArmExecution::getStateDescription(state s)
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
