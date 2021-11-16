// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeArmExecution.hpp"

#include <yarp/conf/version.h>

#include <yarp/os/LogStream.h>
#include <yarp/os/Property.h>
#include <yarp/os/SystemClock.h>
#include <yarp/os/Vocab.h>

#include "../FollowMeVocabs.hpp"

using namespace roboticslab;

#if YARP_VERSION_MINOR >= 5
constexpr auto VOCAB_STATE_ARM_SWINGING = yarp::os::createVocab32('s','w','i','n');
constexpr auto VOCAB_STATE_REST = yarp::os::createVocab32('r','e','s','t');
#else
constexpr auto VOCAB_STATE_ARM_SWINGING = yarp::os::createVocab('s','w','i','n');
constexpr auto VOCAB_STATE_REST = yarp::os::createVocab('r','e','s','t');
#endif
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

    if (!inDialogPort.open(DEFAULT_PREFIX + std::string("/dialogueManager/rpc:s")))
    {
        yError() << "Failed to open dialogue manager port" << inDialogPort.getName();
        return false;
    }

    inDialogPort.setReader(*this);

    state = VOCAB_STATE_REST;
    phase = false;

    return yarp::os::Thread::start();
}

bool FollowMeArmExecution::interruptModule()
{
    yarp::os::Thread::stop();
    inDialogPort.interrupt();
    return true;
}

bool FollowMeArmExecution::close()
{
    inDialogPort.close();
    leftArmDevice.close();
    rightArmDevice.close();
    return true;
}

double FollowMeArmExecution::getPeriod()
{
    return 4.0; // [s]
}

bool FollowMeArmExecution::updateModule()
{
    return true;
}

bool FollowMeArmExecution::armJointsMoveAndWait(const std::vector<double> & leftArmQ, const std::vector<double> & rightArmQ)
{
    if (!leftArmIPositionControl->positionMove(leftArmQ.data()))
    {
        yError() << "Failed to move left arm joints";
        return false;
    }

    if (!rightArmIPositionControl->positionMove(rightArmQ.data()))
    {
        yError() << "Failed to move right arm joints";
        return false;
    }

    bool leftArmMotionDone = false;

    while (!leftArmMotionDone)
    {
        yarp::os::SystemClock::delaySystem(0.1);
        leftArmIPositionControl->checkMotionDone(&leftArmMotionDone);
    }

    bool rightArmMotionDone = false;

    while (!rightArmMotionDone)
    {
        yarp::os::SystemClock::delaySystem(0.1);
        rightArmIPositionControl->checkMotionDone(&rightArmMotionDone);
    }

    return true;
}

bool FollowMeArmExecution::read(yarp::os::ConnectionReader & connection)
{
    yarp::os::Bottle b;

    if (!b.read(connection))
    {
        yError() << "Failed to read bottle from dialogue manager";
        return false;
    }

    yDebug() << "Got:" << b.toString();

#if YARP_VERSION_MINOR >= 5
    switch (b.get(0).asVocab32())
#else
    switch (b.get(0).asVocab())
#endif
    {
    case VOCAB_FOLLOW_ME:
    case VOCAB_STATE_SALUTE:
        state = VOCAB_STATE_SALUTE;
        break;
    case VOCAB_STOP_FOLLOWING:
        state = VOCAB_STOP_FOLLOWING;
        break;
    case VOCAB_STATE_SIGNALIZE_LEFT:
        state = VOCAB_STATE_SIGNALIZE_LEFT;
        break;
    case VOCAB_STATE_SIGNALIZE_RIGHT:
        state = VOCAB_STATE_SIGNALIZE_RIGHT;
        break;
    }

    return true;
}

void FollowMeArmExecution::run()
{
    static const std::vector<double> armZeros(6, 0.0);

    while (!yarp::os::Thread::isStopping())
    {
        switch (state)
        {
        case VOCAB_STATE_ARM_SWINGING:
            if (phase)
            {
                yInfo() << "Phase: true";
                auto leftArmQ = armZeros;
                leftArmQ[0] = 20.0;
                leftArmQ[1] = 5.0;
                auto rightArmQ = armZeros;
                rightArmQ[0] = -20.0;
                rightArmQ[1] = -5.0;
                armJointsMoveAndWait(leftArmQ, rightArmQ);
                phase = false;
            }
            else
            {
                yInfo() << "Phase: false";
                auto leftArmQ = armZeros;
                leftArmQ[0] = -20.0;
                leftArmQ[1] = 5.0;
                auto rightArmQ = armZeros;
                rightArmQ[0] = 20.0;
                rightArmQ[1] = -5.0;
                armJointsMoveAndWait(leftArmQ,rightArmQ);
                phase = true;
            }
            break;

        case VOCAB_STATE_SALUTE:
            yInfo() << "Saluting";
            {
                auto leftArmQ = armZeros;
                auto rightArmQ = armZeros;
                leftArmQ[1] = 4.0; // Tray safety position
                rightArmQ[0] = -45.0;
                rightArmQ[2] = -20.0;
                rightArmQ[3] = -80.0;
                armJointsMoveAndWait(leftArmQ, rightArmQ);
            }
            {
                auto leftArmQ = armZeros;
                auto rightArmQ = armZeros;
                leftArmQ[1] = 4.0; // Tray safety position
                rightArmQ[0] = -45.0;
                rightArmQ[2] = -20.0;
                rightArmQ[3] = -80.0;
                armJointsMoveAndWait(leftArmQ, rightArmQ);
            }
            {
                auto leftArmQ = armZeros;
                auto rightArmQ = armZeros;
                leftArmQ[1] = 4.0; // Tray safety position
                rightArmQ[0] = -45.0;
                rightArmQ[2] = -20.0;
                rightArmQ[3] = -80.0;
                armJointsMoveAndWait(leftArmQ, rightArmQ);
            }
            state = VOCAB_STATE_ARM_SWINGING;
            break;

        case VOCAB_STATE_SIGNALIZE_LEFT:
            yInfo() << "Signaling left";
            {
                auto leftArmQ = {-50.0, 20.0, -10.0, -70.0, -20.0, -40.0};
                auto rightArmQ = armZeros;
                armJointsMoveAndWait(leftArmQ, rightArmQ);
            }
            {
                auto leftArmQ = {-50.0, 20.0, -10.0, -70.0, -20.0, 0.0};
                auto rightArmQ = armZeros;
                armJointsMoveAndWait(leftArmQ, rightArmQ);
            }
            state = VOCAB_STATE_ARM_SWINGING;
            break;

        case VOCAB_STATE_SIGNALIZE_RIGHT:
            yInfo() << "Signaling right";
            {
                auto leftArmQ = armZeros;
                auto rightArmQ = {-50.0, -20.0, 10.0, -70.0, 20.0, -40.0};
                leftArmQ[1] = 4.0; // Tray safety position
                armJointsMoveAndWait(leftArmQ, rightArmQ);
            }
            {
                auto leftArmQ = armZeros;
                auto rightArmQ = {-50.0, -20.0, 10.0, -70.0, 20.0, 0.0};
                leftArmQ[1] = 4.0; // Tray safety position
                armJointsMoveAndWait(leftArmQ, rightArmQ);
            }
            state = VOCAB_STATE_ARM_SWINGING;
            break;

        case VOCAB_STOP_FOLLOWING:
            yInfo() << "Stopping following";
            {
                auto leftArmQ = armZeros;
                auto rightArmQ = armZeros;
                leftArmQ[1] = 4.0; // Tray safety position
                armJointsMoveAndWait(leftArmQ, rightArmQ);
            }
            state = VOCAB_STATE_REST;
            break;

        case VOCAB_STATE_REST:
            // do nothing, just sit and wait
            yInfo() << "Resting";
            break;

        default:
#if YARP_VERSION_MINOR >= 5
            yWarning() << "Bad state:" << yarp::os::Vocab32::decode(state);
#else
            yWarning() << "Bad state:" << yarp::os::Vocab::decode(state);
#endif
            state = VOCAB_STATE_REST;
            break;
        }

        yarp::os::SystemClock::delaySystem(0.1);
    }
}
