// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __FOLLOW_ME_ARM_SWING_HPP__
#define __FOLLOW_ME_ARM_SWING_HPP__

#include <yarp/os/all.h>
#include <yarp/dev/all.h>

#define DEFAULT_ROBOT "/teo"

namespace teo
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
    /** RFModule interruptModule. */
    virtual bool interruptModule();
    /** RFModule getPeriod. */
    virtual double getPeriod();
    /** RFModule updateModule. */
    virtual bool updateModule();

    /** Left Arm Device */
    yarp::dev::PolyDriver leftArmDevice;
    /** Left Arm ControlMode2 Interface */
    yarp::dev::IControlMode *leftArmIControlMode;
    /** Left Arm PositionControl2 Interface */
    yarp::dev::IPositionControl *leftArmIPositionControl;

    /** Right Arm Device */
    yarp::dev::PolyDriver rightArmDevice;
    /** Right Arm ControlMode2 Interface */
    yarp::dev::IControlMode *rightArmIControlMode;
    /** Right Arm PositionControl2 Interface */
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
    virtual bool read(yarp::os::ConnectionReader& connection);

    /** Thread run */
    virtual void run();

    static const yarp::conf::vocab32_t VOCAB_FOLLOW_ME;

    static const yarp::conf::vocab32_t VOCAB_STATE_SALUTE;
    static const yarp::conf::vocab32_t VOCAB_STATE_ARM_SWINGING;
    static const yarp::conf::vocab32_t VOCAB_STOP_FOLLOWING;
    static const yarp::conf::vocab32_t VOCAB_STATE_SIGNALIZE_RIGHT;
    static const yarp::conf::vocab32_t VOCAB_STATE_SIGNALIZE_LEFT;
};

}  // namespace teo

#endif  // __FOLLOW_ME_ARM_SWING_HPP__
