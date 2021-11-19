// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/**
 *
 * @ingroup teo-follow-me_programs
 * @defgroup followMeHeadExecution followMeHeadExecution
 * @brief Creates an instance of roboticslab::FollowMeHeadExecution.
 */

#include <yarp/os/LogStream.h>
#include <yarp/os/Network.h>
#include <yarp/os/ResourceFinder.h>

#include "FollowMeHeadExecution.hpp"

int main(int argc, char * argv[])
{
    yarp::os::ResourceFinder rf;
    rf.setDefaultContext("followMeHeadExecution");
    rf.setDefaultConfigFile("followMeHeadExecution.ini");
    rf.configure(argc, argv);

    roboticslab::FollowMeHeadExecution mod;

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
