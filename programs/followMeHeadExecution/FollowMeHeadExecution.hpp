// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __FOLLOW_ME_HEAD_EXECUTION_HPP__
#define __FOLLOW_ME_HEAD_EXECUTION_HPP__

#include <atomic>

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RpcServer.h>
#include <yarp/os/TypedReaderCallback.h>

#include <yarp/dev/IControlMode.h>
#include <yarp/dev/IEncoders.h>
#include <yarp/dev/IPositionControl.h>
#include <yarp/dev/PolyDriver.h>

#include "FollowMeHeadCommands.h"

namespace roboticslab
{

/**
 * @ingroup followMeHeadExecution
 * @brief Head Execution Core.
 */
class FollowMeHeadExecution : public yarp::os::RFModule,
                              public yarp::os::TypedReaderCallback<yarp::os::Bottle>,
                              public FollowMeHeadCommands
{
public:
    ~FollowMeHeadExecution()
    { close(); }

    bool configure(yarp::os::ResourceFinder & rf) override;
    bool close() override;
    bool interruptModule() override;
    double getPeriod() override;
    bool updateModule() override;

    void onRead(yarp::os::Bottle & bot) override;

    void enableFollowing() override;
    void disableFollowing() override;
    double getOrientationAngle() override;
    bool stop() override;

private:
    yarp::os::RpcServer serverPort;
    yarp::os::BufferedPort<yarp::os::Bottle> detectionPort;

    yarp::dev::PolyDriver headDevice;
    yarp::dev::IControlMode * iControlMode;
    yarp::dev::IEncoders * iEncoders;
    yarp::dev::IPositionControl * iPositionControl;

    std::atomic_bool isFollowing {false};
};

} // namespace roboticslab

#endif // __FOLLOW_ME_HEAD_EXECUTION_HPP__
