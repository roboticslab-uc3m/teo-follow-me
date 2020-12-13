// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeDialogueManager.hpp"

namespace roboticslab
{

/************************************************************************/

bool FollowMeDialogueManager::configure(yarp::os::ResourceFinder &rf) {

    std::string language = rf.check("language",yarp::os::Value(DEFAULT_LANGUAGE),"language to be used").asString();
    std::string micro = rf.check("micro",yarp::os::Value(DEFAULT_MICRO),"use or not microphone").asString();

    printf("--------------------------------------------------------------\n");
    printf("FollowMeDialogueManager options:\n");
    printf("\t--help (this help)\t--from [file.ini]\t--context [path]\n");
    printf("\t--language (default: \"%s\")\n",language.c_str());
    printf("\t--micro (default: \"%s\")\n",micro.c_str());
    printf("--------------------------------------------------------------\n");

    if(micro == "on") microOn = true;
    else if(micro == "off") microOn = false;
    else
    {
        printf("You need to specify if you want to use microphone or not in this demo\n. Please use '--micro on' or '--micro off'\n");
        return false;
    }

    //-----------------OPEN LOCAL PORTS------------//
    headExecutionClient.open("/followMeDialogueManager/head/rpc:c");
    armExecutionClient.open("/followMeDialogueManager/arms/rpc:c");
    ttsClient.open("/followMeDialogueManager/tts/rpc:c");
    asrConfigClient.open("/followMeDialogueManager/speechRecognition/rpc:c"); // -- setDictionary (client)
    inAsrPort.open("/followMeDialogueManager/speechRecognition/speech:i"); // -- words (input)

    stateMachine.setHeadExecutionClient(&headExecutionClient);
    stateMachine.setArmExecutionClient(&armExecutionClient);
    stateMachine.setTtsClient(&ttsClient);
    stateMachine.setAsrConfigClient(&asrConfigClient);
    stateMachine.setInAsrPort(&inAsrPort);

    if(microOn)
    {
        while(0 == asrConfigClient.getOutputCount())
        {
            if(isStopping())
                return false;
            printf("Waiting for \"/followMeDialogueManager/speechRecognition/rpc:c\" to be connected to ASR to configure it...\n");
            yarp::os::Time::delay(0.5);
        }
    }

    while(0 == ttsClient.getOutputCount())
    {
        if(isStopping())
            return false;
        printf("Waiting for \"/followMeDialogueManager/tts/rpc:c\" to be connected to TTS...\n");
        yarp::os::Time::delay(0.5);
    }

    //--------------------------
    // clearing yarp bottles
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

    ttsClient.write(bTtsOut);
    asrConfigClient.write(bSpRecOut);

    // set functions
    stateMachine.setMicro(microOn);
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
    headExecutionClient.interrupt();
    armExecutionClient.interrupt();
    ttsClient.interrupt();
    asrConfigClient.interrupt();
    inAsrPort.interrupt();
    stateMachine.stop();
    headExecutionClient.close();
    armExecutionClient.close();
    ttsClient.close();
    ttsClient.close();
    inAsrPort.close();
    return true;
}

/************************************************************************/

} // namespace roboticslab
