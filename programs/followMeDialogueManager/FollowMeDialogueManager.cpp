// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeDialogueManager.hpp"

#include <yarp/os/LogStream.h>
#include <yarp/os/SystemClock.h>

using namespace roboticslab;

constexpr auto DEFAULT_PREFIX = "/followMeDialogueManager";
constexpr auto DEFAULT_LANGUAGE = "english";
constexpr auto DEFAULT_MICRO = "off";

bool FollowMeDialogueManager::configure(yarp::os::ResourceFinder & rf)
{
    auto language = rf.check("language", yarp::os::Value(DEFAULT_LANGUAGE), "language to be used").asString();
    auto micro = rf.check("micro", yarp::os::Value(DEFAULT_MICRO), "use or not microphone").asString();

    if (rf.check("help"))
    {
        yInfo("FollowMeDialogueManager options:");
        yInfo("\t--help (this help)\t--from [file.ini]\t--context [path]");
        yInfo("\t--language: %s [%s]", language.c_str(), DEFAULT_LANGUAGE);
        yInfo("\t--micro: %s [%s]", micro.c_str(), DEFAULT_MICRO);
        return false;
    }

    if (micro == "on")
    {
        microOn = true;
    }
    else if (micro == "off")
    {
        microOn = false;
    }
    else
    {
        yError() << "Invalid --micro value, expected 'on' or 'off', got:" << micro;
        return false;
    }

    //-----------------OPEN LOCAL PORTS------------//

    if (!armExecutionClient.open(DEFAULT_PREFIX + std::string("/arms/rpc:c")))
    {
        yError() << "Failed to open arm execution client port:" << armExecutionClient.getName();
        return false;
    }

    if (!headExecutionClient.open(DEFAULT_PREFIX + std::string("/head/rpc:c")))
    {
        yError() << "Failed to open head execution client port:" << headExecutionClient.getName();
        return false;
    }

    if (!ttsClient.open(DEFAULT_PREFIX + std::string("/tts/rpc:c")))
    {
        yError() << "Failed to open tts client port:" << ttsClient.getName();
        return false;
    }

    if (!asrConfigClient.open(DEFAULT_PREFIX + std::string("/speechRecognition/rpc:c")))
    {
        yError() << "Failed to open asr config client port:" << asrConfigClient.getName();
        return false;
    }

    if (!inAsrPort.open(DEFAULT_PREFIX + std::string("/speechRecognition/speech:i")))
    {
        yError() << "Failed to open inAsrPort" << inAsrPort.getName();
        return false;
    }

    stateMachine.setHeadExecutionClient(&headExecutionClient);
    stateMachine.setArmExecutionClient(&armExecutionClient);
    stateMachine.setTtsClient(&ttsClient);
    stateMachine.setAsrConfigClient(&asrConfigClient);
    stateMachine.setInAsrPort(&inAsrPort);

    if (microOn)
    {
        while (asrConfigClient.getOutputCount() == 0)
        {
            if (isStopping())
            {
                return false;
            }

            yInfo() << "Waiting for" << asrConfigClient.getName() << "to be connected to ASR to configure it...";
            yarp::os::SystemClock::delaySystem(0.5);
        }
    }

    while (ttsClient.getOutputCount() == 0)
    {
        if (isStopping())
        {
            return false;
        }

        yInfo() << "Waiting for" << ttsClient.getName() << "to be connected to TTS to configure it...";
        yarp::os::SystemClock::delaySystem(0.5);
    }

    while (armExecutionClient.getOutputCount() == 0)
    {
        if (isStopping())
        {
            return false;
        }

        yInfo() << "Waiting for" << armExecutionClient.getName() << "to be connected to ArmExecution to configure it...";
        yarp::os::SystemClock::delaySystem(0.5);
    }

    while (headExecutionClient.getOutputCount() == 0)
    {
        if (isStopping())
        {
            return false;
        }

        yInfo() << "Waiting for" << headExecutionClient.getName() << "to be connected to HeadExecution to configure it...";
        yarp::os::SystemClock::delaySystem(0.5);
    }

    bTtsOut.addString("setLanguage");
    bSpRecOut.addString("setDictionary");
    bSpRecOut.addString("follow-me");

    if (language == "english")
    {
        bTtsOut.addString("mb-en1");
        bSpRecOut.addString(language);
    }
    else if (language == "spanish")
    {
        bTtsOut.addString("mb-es1");
        bSpRecOut.addString(language);
    }
    else
    {
        yError() << "Language not found, please use '--language english' or '--language spanish', got:" << language;
        return false;
    }

    ttsClient.write(bTtsOut);
    asrConfigClient.write(bSpRecOut);

    // set functions
    stateMachine.setMicro(microOn);
    stateMachine.setLanguage(language);
    stateMachine.setSpeakLanguage(language);

    return stateMachine.start();
}

double FollowMeDialogueManager::getPeriod()
{
    return 2.0; // [s]
}

bool FollowMeDialogueManager::updateModule()
{
    yInfo() << "StateMachine in state" << stateMachine.getMachineState();
    return true;
}

bool FollowMeDialogueManager::interruptModule()
{
    headExecutionClient.interrupt();
    armExecutionClient.interrupt();
    ttsClient.interrupt();
    asrConfigClient.interrupt();
    inAsrPort.interrupt();
    stateMachine.stop();
    return true;
}

bool FollowMeDialogueManager::close()
{
    headExecutionClient.close();
    armExecutionClient.close();
    ttsClient.close();
    ttsClient.close();
    inAsrPort.close();
    return true;
}
