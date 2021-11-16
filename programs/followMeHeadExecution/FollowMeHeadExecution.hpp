// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __FOLLOW_ME_HEAD_EXECUTION_HPP__
#define __FOLLOW_ME_HEAD_EXECUTION_HPP__

#include <vector>

#include <yarp/os/RFModule.h>
#include <yarp/os/RpcServer.h>

#include <yarp/dev/IControlMode.h>
#include <yarp/dev/IEncoders.h>
#include <yarp/dev/IPositionControl.h>
#include <yarp/dev/PolyDriver.h>

#include "InCvPort.hpp"
#include "InDialoguePortProcessor.hpp"

namespace roboticslab
{

/**
 * @ingroup follow-me_programs
 * @brief Head Execution Core.
 */
class FollowMeHeadExecution : public yarp::os::RFModule
{
public:
    ~FollowMeHeadExecution()
    { close(); }

    bool configure(yarp::os::ResourceFinder & rf) override;
    bool close() override;
    bool interruptModule() override;
    double getPeriod() override;
    bool updateModule() override;

private:
    //-- Rpc port, server for retrieving encoder position (reply position port), etc.
    yarp::os::RpcServer inDialoguePort;
    InDialoguePortProcessor inDialoguePortProcessor;
    InCvPort inCvPort;

    /** Head Device */
    yarp::dev::PolyDriver headDevice;
    yarp::dev::IControlMode * headIControlMode;
    yarp::dev::IEncoders * iEncoders;
    yarp::dev::IPositionControl * headIPositionControl;
};

} // namespace roboticslab

#endif // __FOLLOW_ME_HEAD_EXECUTION_HPP__
