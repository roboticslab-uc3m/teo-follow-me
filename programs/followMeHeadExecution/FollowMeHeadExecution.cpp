// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeHeadExecution.hpp"

#include <cmath> // std::abs, std::copysign

#include <array>
#include <vector>

#include <yarp/os/LogStream.h>
#include <yarp/os/Property.h>

using namespace roboticslab;

constexpr auto DEFAULT_ROBOT = "/teo";
constexpr auto DEFAULT_PREFIX = "/followMeHeadExecution";
constexpr auto DEFAULT_REF_SPEED = 30.0;
constexpr auto DEFAULT_REF_ACCELERATION = 30.0;
constexpr auto DETECTION_DEADBAND = 0.03; // [m]
constexpr auto RELATIVE_INCREMENT = 2.0; // [deg]

constexpr std::array<double, 2> headZeros {0.0, 0.0};

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

    if (!headDevice.view(iControlMode) || !headDevice.view(iEncoders) || !headDevice.view(iPositionControl))
    {
        yError() << "Failed to view head device interfaces";
        return false;
    }

    if (!iControlMode->setControlModes(std::vector(2, VOCAB_CM_POSITION).data()))
    {
        yError() << "Failed to set position control mode";
        return false;
    }

    if (!iPositionControl->setRefSpeeds(std::vector(2, DEFAULT_REF_SPEED).data()))
    {
        yError() << "Failed to set reference speeds";
        return false;
    }

    if (!iPositionControl->setRefAccelerations(std::vector(2, DEFAULT_REF_ACCELERATION).data()))
    {
        yError() << "Failed to set reference accelerations";
        return false;
    }

    if (!serverPort.open(DEFAULT_PREFIX + std::string("/dialogueManager/rpc:s")))
    {
        yError() << "Failed to open dialogue server port" << serverPort.getName();
        return false;
    }

    if (!detectionPort.open(DEFAULT_PREFIX + std::string("/cv/state:i")))
    {
        yError() << "Failed to open detection client port" << detectionPort.getName();
        return false;
    }

    yarp::os::Wire::yarp().attachAsServer(serverPort);
    detectionPort.useCallback(*this);

    return true;
}

double FollowMeHeadExecution::getPeriod()
{
    return 0.1; // [s]
}

bool FollowMeHeadExecution::updateModule()
{
    if (detectionPort.getInputCount() == 0)
    {
        yDebugThrottle(1.0) << "Waiting for" << detectionPort.getName() << "to be connected to vision...";
    }

    return true;
}

bool FollowMeHeadExecution::interruptModule()
{
    serverPort.interrupt();
    detectionPort.interrupt();
    detectionPort.disableCallback();
    return stop();
}

bool FollowMeHeadExecution::close()
{
    serverPort.close();
    detectionPort.close();
    headDevice.close();
    return true;
}

void FollowMeHeadExecution::onRead(yarp::os::Bottle & b)
{
    if (!isFollowing)
    {
        return;
    }

    if (b.size() != 3)
    {
        yWarning() << "InCvPort protocol error, expected 3 elements, got" << b.size();
        return;
    }

    auto x = b.get(0).asFloat64(); // [m]
    auto y = b.get(1).asFloat64(); // [m]
    auto z = b.get(2).asFloat64(); // [m] (depth, unused)

    if (std::abs(x) > DETECTION_DEADBAND || std::abs(y) > DETECTION_DEADBAND)
    {
        // On the received frame, positive X is to the right, positive Y is down.
        // First axis (global Z roll) is positive to the left (frame-wise).
        // Second axis (global Y pitch) is positive down (frame-wise).

        std::vector<double> target {
            std::abs(x) > DETECTION_DEADBAND ? std::copysign(RELATIVE_INCREMENT, -x) : 0.0,
            std::abs(y) > DETECTION_DEADBAND ? std::copysign(RELATIVE_INCREMENT, y) : 0.0
        };

        yDebug() << "Detection port got:" << x << y << z << "|| performing relative motion:" << target;

        if (!iPositionControl->relativeMove(target.data()))
        {
            yError() << "Failed to move head";
        }
    }
    else
    {
        yDebug() << "Detection port got (x,y,z):" << x << y << z;
    }
}

void FollowMeHeadExecution::enableFollowing()
{
    yInfo() << "Received start following signal";
    isFollowing = true;
}

void FollowMeHeadExecution::disableFollowing()
{
    yInfo() << "Received stop following signal, moving to home position";
    isFollowing = false;

    if (!iPositionControl->positionMove(headZeros.data()))
    {
        yError() << "Failed to perform homing";
    }
}

double FollowMeHeadExecution::getOrientationAngle()
{
    double angle = 0.0;

    if (!iEncoders->getEncoder(0, &angle))
    {
        yError() << "Failed to get head orientation encoder value";
    }

    return angle;
}

bool FollowMeHeadExecution::stop()
{
    yInfo() << "Received stop command";
    isFollowing = false;

    if (!iPositionControl->stop())
    {
        yError() << "Failed to stop head";
        return false;
    }

    return true;
}
