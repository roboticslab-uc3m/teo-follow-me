// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __IN_DIALOGUE_PORT_HPP__
#define __IN_DIALOGUE_PORT_HPP__

#include <yarp/os/PortReader.h>

#include <yarp/dev/IEncoders.h>

#include "InCvPort.hpp"

namespace roboticslab
{

/**
 * @ingroup followMeHeadExecution
 *
 * @brief Input port of cv recognition data.
 *
 */

class InDialoguePortProcessor : public yarp::os::PortReader
{
public:
    bool read(yarp::os::ConnectionReader & connection) override;

    void setInCvPortPtr(InCvPort * inCvPortPtr)
    {
        this->inCvPortPtr = inCvPortPtr;
    }

    void setIEncoders(yarp::dev::IEncoders * iEncoders)
    {
        this->iEncoders = iEncoders;
    }

private:
    InCvPort * inCvPortPtr;
    yarp::dev::IEncoders * iEncoders;
};

} // namespace roboticslab

#endif // __IN_DIALOGUE_PORT_HPP__
