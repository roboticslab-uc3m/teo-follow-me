// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __FOLLOW_ME_ARM_SWING_HPP__
#define __FOLLOW_ME_ARM_SWING_HPP__

#include <yarp/os/all.h>
#include <yarp/dev/all.h>

#define DEFAULT_ROBOT "/teo"

namespace roboticslab
{

/**
 * @ingroup follow-me_programs
 *
 * @brief Arm Execution Core.
 *
 */
class FollowMeArmExecution : public yarp::os::RFModule, public yarp::os::PortReader, public yarp::os::Thread
{
public:
    virtual bool configure(yarp::os::ResourceFinder &rf);

private:
    bool interruptModule() override;
    double getPeriod() override;
    bool updateModule() override;

    yarp::dev::PolyDriver leftArmDevice;
    yarp::dev::IControlMode *leftArmIControlMode;
    yarp::dev::IPositionControl *leftArmIPositionControl;

    yarp::dev::PolyDriver rightArmDevice;
    yarp::dev::IControlMode *rightArmIControlMode;
    yarp::dev::IPositionControl *rightArmIPositionControl;

    /** Phase of arm swing movement */
    bool phase;

    /** Arm Joints Move And Wait */
    bool armJointsMoveAndWait(std::vector<double>& leftArmQ, std::vector<double>& rightArmQ);

    /** State */
    int state;

    /** Input port from dialogue manager */
    yarp::os::RpcServer inDialogPort;

    /** Treats data received from input port from speech recognition */
    bool read(yarp::os::ConnectionReader& connection) override;

    /** Thread run */
    void run() override;

    static const yarp::conf::vocab32_t VOCAB_FOLLOW_ME;

    static const yarp::conf::vocab32_t VOCAB_STATE_SALUTE;
    static const yarp::conf::vocab32_t VOCAB_STATE_ARM_SWINGING;
    static const yarp::conf::vocab32_t VOCAB_STOP_FOLLOWING;
    static const yarp::conf::vocab32_t VOCAB_STATE_SIGNALIZE_RIGHT;
    static const yarp::conf::vocab32_t VOCAB_STATE_SIGNALIZE_LEFT;
};

} // namespace roboticslab

#endif  // __FOLLOW_ME_ARM_SWING_HPP__
