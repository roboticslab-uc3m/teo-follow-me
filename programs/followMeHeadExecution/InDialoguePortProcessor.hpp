// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __IN_SR_PORT_HPP__
#define __IN_SR_PORT_HPP__

#include <yarp/os/PortReader.h>

#include <yarp/dev/ControlBoardInterfaces.h>

#include "InCvPort.hpp"

namespace roboticslab
{

/**
 * @ingroup followMeExecutionCore
 *
 * @brief Input port of speech recognition data.
 *
 */

class InDialoguePortProcessor : public yarp::os::PortReader
{
public:
    void setInCvPortPtr(InCvPort *inCvPortPtr)
    {
        this->inCvPortPtr = inCvPortPtr;
    }

    void setIEncoders(yarp::dev::IEncoders *iEncoders)
    {
        this->iEncoders = iEncoders;
    }

private:
    /** Getting replies **/
    bool read(yarp::os::ConnectionReader& connection) override;

    //-- Cv Port
    InCvPort* inCvPortPtr;

    //-- Robot device
    yarp::dev::IEncoders *iEncoders;
};

} // namespace roboticslab

#endif  // __IN_SR_PORT_HPP__
