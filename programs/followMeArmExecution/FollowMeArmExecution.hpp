// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __FOLLOW_ME_ARM_EXECUTION_HPP__
#define __FOLLOW_ME_ARM_EXECUTION_HPP__

#include <array>
#include <deque>
#include <initializer_list>
#include <mutex>
#include <string>
#include <tuple>

#include <yarp/os/RFModule.h>

#include <yarp/dev/IControlMode.h>
#include <yarp/dev/IPositionControl.h>
#include <yarp/dev/PolyDriver.h>

#include "FollowMeArmCommandsIDL.h"

namespace roboticslab
{

/**
 * @ingroup follow-me_programs
 * @brief Arm Execution Core.
 */
class FollowMeArmExecution : public yarp::os::RFModule,
                             public FollowMeArmCommandsIDL
{
public:
    ~FollowMeArmExecution()
    { close(); }

    bool configure(yarp::os::ResourceFinder & rf) override;
    bool close() override;
    bool interruptModule() override;
    double getPeriod() override;
    bool updateModule() override;

    void doGreet() override;
    void doSignalLeft() override;
    void doSignalRight() override;
    void enableArmSwinging() override;
    void disableArmSwinging() override;
    bool stop() override;

private:
    enum class state { GREET, SIGNAL_LEFT, SIGNAL_RIGHT, SWING, HOMING, REST };

    using setpoints_arm_t = std::array<double, 6>;
    using setpoints_t = std::tuple<setpoints_arm_t, setpoints_arm_t>;

    void registerSetpoints(state newState, std::initializer_list<setpoints_t> setpoints);
    bool checkMotionDone();
    std::string getStateDescription(state s);

    const setpoints_arm_t armZeros {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    std::deque<setpoints_t> currentSetpoints;
    std::mutex actionMutex;
    bool hasNewSetpoints {false};
    state currentState {state::REST};

    yarp::dev::PolyDriver leftArmDevice;
    yarp::dev::IControlMode * leftArmIControlMode;
    yarp::dev::IPositionControl * leftArmIPositionControl;

    yarp::dev::PolyDriver rightArmDevice;
    yarp::dev::IControlMode * rightArmIControlMode;
    yarp::dev::IPositionControl * rightArmIPositionControl;

    yarp::os::RpcServer serverPort;
};

} // namespace roboticslab

#endif // __FOLLOW_ME_ARM_EXECUTION_HPP__
