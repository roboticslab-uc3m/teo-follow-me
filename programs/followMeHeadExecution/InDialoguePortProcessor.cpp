// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "InDialoguePortProcessor.hpp"

#include <yarp/os/LogStream.h>

#include <yarp/dev/GenericVocabs.h> // VOCAB_FAILED

#include "../FollowMeVocabs.hpp"

using namespace roboticslab;

bool InDialoguePortProcessor::read(yarp::os::ConnectionReader & connection)
{
    yarp::os::Bottle in;

    if (!in.read(connection))
    {
        yError() << "Failed to read from connection";
        return false;
    }

    double encValue;

    auto * returnToSender = connection.getWriter();

#if YARP_VERSION_MINOR >= 5
    switch (in.get(0).asVocab32())
#else
    switch (in.get(0).asVocab())
#endif
    {
    case VOCAB_FOLLOW_ME:
        yInfo() << "follow";
        inCvPortPtr->setFollow(true);
        break;

    case VOCAB_STOP_FOLLOWING:
        yInfo() << "stopFollowing";
        inCvPortPtr->setFollow(false);
        break;

    case VOCAB_GET_ENCODER_POSITION:
        if (!iEncoders->getEncoder(0, &encValue))
        {
            yError() << "getEncoder failed";
            yarp::os::Bottle out = {yarp::os::Value(VOCAB_FAILED, true)};

            if (returnToSender)
            {
                out.write(*returnToSender);
            }

            return false;
        }
        else
        {
            yarp::os::Bottle out = {yarp::os::Value(encValue)};

            if (returnToSender)
            {
                out.write(*returnToSender);
            }
        }

        break;
    }

    return true;
}
