// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "InDialoguePortProcessor.hpp"

#include "../FollowMeVocabs.hpp"

namespace roboticslab
{

/************************************************************************/

const yarp::conf::vocab32_t InDialoguePortProcessor::VOCAB_GET_ENCODER_POSITION = yarp::os::createVocab('g','e','p','s');
const yarp::conf::vocab32_t InDialoguePortProcessor::VOCAB_FAILED = yarp::os::createVocab('f','a','i','l');
const yarp::conf::vocab32_t InDialoguePortProcessor::VOCAB_STOP_FOLLOWING = yarp::os::createVocab('s','f','o','l');

/************************************************************************/

bool InDialoguePortProcessor::read(ConnectionReader& connection) {
    Bottle in, out;
    double encValue;
    bool ok = in.read(connection);
    if (!ok) return false;
    // -- Gets a way to reply to the message, if possible.
    ConnectionWriter *returnToSender = connection.getWriter();

    switch ( in.get(0).asVocab() ) { //-- b.get(0).asVocab()
    case VOCAB_FOLLOW_ME:
        printf("follow\n");
        inCvPortPtr->setFollow(true);
        break;

    case VOCAB_STOP_FOLLOWING:
        printf("stopFollowing\n");
        inCvPortPtr->setFollow(false);
        break;

    case VOCAB_GET_ENCODER_POSITION:
        if ( ! iEncoders->getEncoder(0, &encValue) )  // 0 es el tilt del cuello (http://robots.uc3m.es/index.php/TEO_Diagrams)
        {
            printf("Error: getEncoder failed\n");
            out.addVocab(VOCAB_FAILED);
            if (returnToSender!=NULL)
                out.write(*returnToSender);
            return true;
        }

        out.addDouble(encValue);
        if (returnToSender!=NULL)
            out.write(*returnToSender);
        break;

        return true;
    }
}

/************************************************************************/

} // namespace roboticslab
