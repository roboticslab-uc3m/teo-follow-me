// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeDialogueManager.hpp"

namespace roboticslab
{

/************************************************************************/

bool FollowMeDialogueManager::configure(yarp::os::ResourceFinder &rf) {

    std::string language = rf.check("language",yarp::os::Value(DEFAULT_LANGUAGE),"language to be used").asString();
    std::string micro = rf.check("micro",yarp::os::Value(DEFAULT_MICRO),"use or not microphone").asString();

    printf("--------------------------------------------------------------\n");
    if (rf.check("help")) {
        printf("FollowMeDialogueManager options:\n");
        printf("\t--help (this help)\t--from [file.ini]\t--context [path]\n");
        printf("\t--language (default: \"%s\")\n",language.c_str());
        printf("\t--micro (default: \"%s\")\n",micro.c_str());
        //printf("\t--file (default: \"%s\")\n",fileName.c_str());
    }
    //if (rf.check("file")) fileName = rf.find("file").asString();
    //printf("FollowMeDialogueManager using file: %s\n",fileName.c_str());

    printf("--------------------------------------------------------------\n");
    if(rf.check("help")) {
        ::exit(1);
    }

    if(micro == "on") microState = true;
    else if(micro == "off") microState = false;
    else
    {
        printf("You need to specify if you want to use microphone or not in this demo\n. Please use '--micro on' or '--micro off'\n");
        return false;
    }


    //-----------------OPEN LOCAL PORTS------------//
    outCmdHeadPort.open("/followMeDialogueManager/head/rpc:c");
    outCmdArmPort.open("/followMeDialogueManager/arms/rpc:c");
    outTtsPort.open("/followMeDialogueManager/tts/rpc:c");
    outSrecPort.open("/followMeDialogueManager/speechRecognition/rpc:c"); // -- setDictionary (client)
    inSrPort.open("/followMeDialogueManager/speechRecognition/speech:i"); // -- words (input)

    stateMachine.setOutCmdHeadPort(&outCmdHeadPort);
    stateMachine.setOutCmdArmPort(&outCmdArmPort);
    stateMachine.setOutTtsPort(&outTtsPort);
    stateMachine.setOutSrecPort(&outSrecPort);
    stateMachine.setInSrPort(&inSrPort);

    if(microState)
    {
        while(0 == outSrecPort.getOutputCount())
        {
            if(isStopping())
                return false;
            printf("Waiting for \"/followMeDialogueManager/speechRecognition/rpc:c\" to be connected to something...\n");
            yarp::os::Time::delay(0.5);
        }
    }

    while(0 == outTtsPort.getOutputCount())
    {
        if(isStopping())
            return false;
        printf("Waiting for \"/followMeDialogueManager/tts/rpc:c\" to be connected to something...\n");
        yarp::os::Time::delay(0.5);
    }

    //--------------------------
    // cleaning yarp bottles
    bTtsOut.clear();
    bSpRecOut.clear();

    bTtsOut.addString("setLanguage");
    bSpRecOut.addString("setDictionary");
    bSpRecOut.addString("follow-me");

    if( language == "english" )
    {
        bTtsOut.addString("mb-en1");
        bSpRecOut.addString(language);
    }
    else if ( language == "spanish" )
    {
        bTtsOut.addString("mb-es1");
        bSpRecOut.addString(language); // -- cambiar a "language" cuando tengamos reconocimiento en español
    }
    else
    {
        printf("Language not found. Please use '--language english' or '--language spanish'");
        return false;
    }

    outTtsPort.write(bTtsOut);
    outSrecPort.write(bSpRecOut);

    // set functions
    stateMachine.setMicro(microState);
    stateMachine.setLanguage(language);
    stateMachine.setSpeakLanguage(language);

    stateMachine.start();
    return true;
}

/************************************************************************/

double FollowMeDialogueManager::getPeriod()
{
    return 2.0;  // Fixed, in seconds, the slow thread that calls updateModule below
}

/************************************************************************/
bool FollowMeDialogueManager::updateModule()
{
    printf("StateMachine in state [%d]. FollowMeDialogueManager alive...\n", stateMachine.getMachineState());
    return true;
}

/************************************************************************/

bool FollowMeDialogueManager::interruptModule()
{
    printf("FollowMeDialogueManager closing...\n");
    outCmdHeadPort.interrupt();
    outCmdArmPort.interrupt();
    outTtsPort.interrupt();
    outSrecPort.interrupt();
    inSrPort.interrupt();
    stateMachine.stop();
    outCmdHeadPort.close();
    outCmdArmPort.close();
    outTtsPort.close();
    outTtsPort.close();
    inSrPort.close();
    return true;
}

/************************************************************************/

} // namespace roboticslab
