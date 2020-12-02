// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/**
 *
 * @ingroup follow-me_programs
 * \defgroup followMeDialogueManager followMeDialogueManager
 *
 * @brief Creates an instance of roboticslab::FollowMeDialogueManager.
 *
 * @section followMeDialogueManager_legal Legal
 *
 * Copyright: 2016 (C) Universidad Carlos III de Madrid
 *
 * Author: <a href="http://roboticslab.uc3m.es/roboticslab/persona.php?id_pers=72">Juan G. Victores</a>
 *
 * CopyPolicy: This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License 3.0 or later
 *
 * <hr>
 *
 * This file can be edited at followMeDialogueManager
 *
 */

#include "FollowMeDialogueManager.hpp"

int main(int argc, char **argv)
{
    yarp::os::ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultContext("followMeDialogueManager");
    rf.setDefaultConfigFile("followMeDialogueManager.ini");
    rf.configure(argc, argv);

    roboticslab::FollowMeDialogueManager mod;
    if(rf.check("help"))
    {
        return mod.runModule(rf);
    }

    printf("Run \"%s --help\" for options.\n",argv[0]);
    printf("%s checking for yarp network... ",argv[0]);
    fflush(stdout);
    yarp::os::Network yarp;
    if (!yarp.checkNetwork())
    {
        fprintf(stderr,"[fail]\n%s found no yarp network (try running \"yarpserver &\"), bye!\n",argv[0]);
        return 1;
    }
    printf("[ok]\n");

    return mod.runModule(rf);
}
