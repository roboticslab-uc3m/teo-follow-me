// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __FOLLOW_ME_ARM_EXECUTION_HPP__
#define __FOLLOW_ME_ARM_EXECUTION_HPP__

#include <string>
#include <vector>

#include <yarp/os/PortReader.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Thread.h>

#include <yarp/dev/IControlMode.h>
#include <yarp/dev/IPositionControl.h>
#include <yarp/dev/PolyDriver.h>

namespace roboticslab
{

/**
 * @ingroup follow-me_programs
 * @brief Arm Execution Core.
 */
class FollowMeArmExecution : public yarp::os::RFModule,
                             public yarp::os::PortReader,
                             public yarp::os::Thread
{
public:
    ~FollowMeArmExecution()
    { close(); }

    bool configure(yarp::os::ResourceFinder & rf) override;
    bool close() override;
    bool interruptModule() override;
    double getPeriod() override;
    bool updateModule() override;

    /** Treats data received from input port from speech recognition */
    bool read(yarp::os::ConnectionReader& connection) override;

    /** Thread run */
    void run() override;

private:
    yarp::dev::PolyDriver leftArmDevice;
    yarp::dev::IControlMode * leftArmIControlMode;
    yarp::dev::IPositionControl * leftArmIPositionControl;

    yarp::dev::PolyDriver rightArmDevice;
    yarp::dev::IControlMode * rightArmIControlMode;
    yarp::dev::IPositionControl * rightArmIPositionControl;

    /** Phase of arm swing movement */
    bool phase;

    /** Arm Joints Move And Wait */
    bool armJointsMoveAndWait(const std::vector<double> & leftArmQ, const std::vector<double> & rightArmQ);

    /** State */
    int state;

    /** Input port from dialogue manager */
    yarp::os::RpcServer inDialogPort;
};

} // namespace roboticslab

#endif  // __FOLLOW_ME_ARM_EXECUTION_HPP__
