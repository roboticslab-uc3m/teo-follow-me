// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/**
 * @ingroup teo-follow-me_programs
 * @defgroup followMeDialogueManager followMeDialogueManager
 * @brief Creates an instance of roboticslab::FollowMeDialogueManager.
 */

#include <yarp/os/LogStream.h>
#include <yarp/os/Network.h>
#include <yarp/os/ResourceFinder.h>

#include "FollowMeDialogueManager.hpp"

int main(int argc, char * argv[])
{
    yarp::os::ResourceFinder rf;
    rf.setDefaultContext("followMeDialogueManager");
    rf.setDefaultConfigFile("followMeDialogueManager.ini");
    rf.configure(argc, argv);

    roboticslab::FollowMeDialogueManager mod;

    if (rf.check("help"))
    {
        return mod.runModule(rf);
    }

    yInfo("Run \"%s --help\" for options", argv[0]);
    yInfo("%s checking for yarp network...", argv[0]);

    yarp::os::Network yarp;

    if (!yarp::os::Network::checkNetwork())
    {
        yError() << argv[0] << "found no yarp network (try running \"yarpserver &\")";
        return 1;
    }

    return mod.runModule(rf);
}
