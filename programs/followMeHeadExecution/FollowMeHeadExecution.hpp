// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __FM_EXECUTION_CORE_HPP__
#define __FM_EXECUTION_CORE_HPP__

#include <vector>

#include <yarp/os/RFModule.h>
#include <yarp/os/RpcServer.h>

#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/PolyDriver.h>

#include "InCvPort.hpp"
#include "InDialoguePortProcessor.hpp"

#define VOCAB_FOLLOW_ME VOCAB4('f','o','l','l')
#define VOCAB_STOP_FOLLOWING VOCAB4('s','f','o','l')

namespace roboticslab
{

/**
 * @ingroup follow-me_programs
 *
 * @brief Head Execution Core.
 *
 */
class FollowMeHeadExecution : public yarp::os::RFModule
{
public:
    bool configure(ResourceFinder &rf) override;

private:
    //-- Rpc port, server to knowing encoder position (reply position port), etc...
    RpcServer inDialoguePort;
    InDialoguePortProcessor inDialoguePortProcessor; // old (InSrPort)
    InCvPort inCvPort;

    /** Head Device */
    yarp::dev::PolyDriver headDevice;
    /** Head ControlMode Interface */
    yarp::dev::IControlMode *headIControlMode;
    /** Head PositionControl Interface */
    yarp::dev::IPositionControl *headIPositionControl;

    yarp::dev::IEncoders *iEncoders;

    bool interruptModule() override;
    double getPeriod() override;
    bool updateModule() override;
};

} // namespace roboticslab

#endif  // __FM_EXECUTION_CORE_HPP__
