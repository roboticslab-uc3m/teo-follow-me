// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __IN_SR_PORT_HPP__
#define __IN_SR_PORT_HPP__

#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <stdlib.h>

#include "InCvPort.hpp"

#define VOCAB_FOLLOW_ME VOCAB4('f','o','l','l')
#define VOCAB_GET_ENCODER_POSITION VOCAB4('g','e','p','s')
#define VOCAB_FAILED VOCAB4('f','a','i','l')

using namespace yarp::os;

namespace teo
{

/**
 * @ingroup followMeExecutionCore
 *
 * @brief Input port of speech recognition data.
 *
 */


class InDialoguePortProcessor : public PortReader {
    public:
        void setInCvPortPtr(InCvPort *inCvPortPtr) {
            this->inCvPortPtr = inCvPortPtr;
        }

        void setIEncoders(yarp::dev::IEncoders *iEncoders) {
            this->iEncoders = iEncoders;
        }


    protected:
        /** Getting replies **/
        virtual bool read(ConnectionReader& connection);

        //-- Cv Port
        InCvPort* inCvPortPtr;

        //-- Robot device
        yarp::dev::IEncoders *iEncoders;
};


}  // namespace teo

#endif  // __IN_SR_PORT_HPP__
