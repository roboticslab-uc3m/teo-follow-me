// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeDialogueManager.hpp"

#include <yarp/os/LogStream.h>
#include <yarp/os/SystemClock.h>

#include "../FollowMeVocabs.hpp"

using namespace roboticslab;

namespace
{
    const std::unordered_map<std::string, std::string> englishSentences = {
        {"presentation1", "Follow me, demostration started."},
        {"presentation2", "Hello. My name is TEO. I am, a humanoid robot, of Carlos tercero, university."},
        {"presentation3", "Now, I will follow you. Please, tell me."},
        {"askName", "Could you tell me your name."},
        {"answer1", "Is, a beatifull name. I love it."},
        {"answer2", "Is, a wonderfull name. My human creator, has the same name."},
        {"answer3", "My parents, didn't want to baptize me, with that name."},
        {"notUnderstand", "Sorry, I don't understand."},
        {"okFollow", "Okay, I will follow you."},
        {"stopFollow", "Okay, I will stop following you. See you later."},
        {"onTheRight", "You are, on my, right."},
        {"onTheLeft", "You are, on my, left."},
        {"onTheCenter", "You are, on the, center."},
    };

    const std::unordered_map<std::string, std::string> englishCommands = {
        {"hiTeo", "hi teo"},
        {"followMe", "follow me"},
        {"myNameIs", "my name is"},
        {"stopFollowing", "stop following"},
    };

    const std::unordered_map<std::string, std::string> spanishSentences = {
        {"presentation1", "Demostración de detección de caras iniciada."},
        {"presentation2", "Hola. Me yamo Teo, y soy un grobot humanoide diseñado por ingenieros de la universidad carlos tercero."},
        {"presentation3", "Por favor, dime qué quieres que haga."},
        {"askName", "Podrías decirme tu nombre."},
        {"answer1", "Uuooooo ouu, que nombre más bonito. Me encanta."},
        {"answer2", "Que gran nombre. Mi creador humano se yama igual."},
        {"answer3", "Mis padres no quisieron bauuutizarme con ese nombre. Malditos."},
        {"notUnderstand", "Lo siento. No te he entendido."},
        {"okFollow", "Vale. Voy, a comenzar a seguirte."},
        {"stopFollow", "De acuerdo. Voy, a dejar de seguirte. Hasta pronto."},
        {"onTheRight", "Ahora, estás, a mi derecha."},
        {"onTheLeft", "Ahora, estás, a mi izquierda."},
        {"onTheCenter", "Ahora, estás, en el centro."},
    };

    const std::unordered_map<std::string, std::string> spanishCommands = {
        {"hiTeo", "hola teo"},
        {"followMe", "sigueme"},
        {"myNameIs", "me llamo"},
        {"stopFollowing", "para teo"},
    };
}

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

    speech.yarp().attachAsClient(ttsClient);
    headCommander.yarp().attachAsClient(headExecutionClient);

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

    std::string voice;
    yarp::os::Bottle bSpRecOut = {yarp::os::Value("setDictionary"), yarp::os::Value("follow-me")};

    if (language == "english")
    {
        voice = "mb-en1";
        bSpRecOut.addString(language);
        sentences = englishSentences;
        voiceCommands = englishCommands;
    }
    else if (language == "spanish")
    {
        voice = "mb-es1";
        bSpRecOut.addString(language);
        sentences = spanishSentences;
        voiceCommands = spanishCommands;
    }
    else
    {
        yError() << "Language not found, please use '--language english' or '--language spanish', got:" << language;
        return false;
    }

    if (!speech.setLanguage(voice))
    {
        yError() << "Failed to set TTS voice to" << voice;
        return false;
    }

    asrConfigClient.write(bSpRecOut);

    ttsSay(sentences["presentation1"]);

    return true;
}

double FollowMeDialogueManager::getPeriod()
{
    return 0.1; // [s]
}

bool FollowMeDialogueManager::updateModule()
{
    yInfoThrottle(2.0) << "StateMachine in state" << machineState;

    // follow only (no speech)
    if (!microOn)
    {
        isFollowing = true;
        ttsSay(sentences["okFollow"]);
        yarp::os::Bottle cmd = {yarp::os::Value(VOCAB_STATE_SALUTE, true)};
        armExecutionClient.write(cmd);
        headCommander.enableFollowing();
    }

    if (machineState == 0)
    {
        ttsSay(sentences["presentation2"]);
        ttsSay(sentences["presentation3"]);
        machineState = 3;
    }

    if (machineState == 1)
    {
        ttsSay(sentences["askName"]);
        yarp::os::Bottle cmd = {yarp::os::Value(VOCAB_STATE_SALUTE, true)};
        armExecutionClient.write(cmd);
        machineState = 2;
    }
    else if (machineState == 2)
    {
        std::string inStr = asrListen();
        // Blocking
        _inStrState1 = inStr;

        if (_inStrState1.find(voiceCommands["stopFollowing"]) != std::string::npos)
        {
            machineState = 5;
        }
        else if (_inStrState1.find(voiceCommands["myNameIs"]) != std::string::npos)
        {
            switch (sentence)
            {
            case 'a':
                ttsSay(sentences["answer1"]);
                sentence = 'b';
                break;
            case 'b':
                ttsSay(sentences["answer2"]);
                sentence = 'c';
                break;
            case 'c':
                ttsSay(sentences["answer3"]);
                sentence = 'a';
                break;
            default:
                break;
            }

            machineState = 3;
        }
        else
        {
            ttsSay(sentences["notUnderstand"]);
            machineState = 1;
        }
    }
    else if (machineState == 3)
    {
        std::string inStr = isFollowing ? asrListenWithPeriodicWave() : asrListen();

        // Blocking
        _inStrState1 = inStr;

        if (_inStrState1.find(voiceCommands["hiTeo"]) != std::string::npos) machineState = 0;
        else if (_inStrState1.find(voiceCommands["followMe"]) != std::string::npos) machineState = 4;
        else if (_inStrState1.find(voiceCommands["stopFollowing"]) != std::string::npos) machineState = 5;
        else machineState = 3;
    }
    else if (machineState == 4)
    {
        isFollowing = true;
        ttsSay(sentences["okFollow"]);
        headCommander.enableFollowing();
        machineState = 1;
    }
    else if (machineState == 5)
    {
        isFollowing = false;
        ttsSay(sentences["stopFollow"]);
        yarp::os::Bottle cmd = {yarp::os::Value(VOCAB_STOP_FOLLOWING, true)};
        armExecutionClient.write(cmd);
        headCommander.disableFollowing();
        machineState = 3;
    }
    else
    {
        ttsSay("ANOMALY");
        machineState = 1;
    }

    return true;
}

bool FollowMeDialogueManager::interruptModule()
{
    headExecutionClient.interrupt();
    armExecutionClient.interrupt();
    ttsClient.interrupt();
    asrConfigClient.interrupt();
    inAsrPort.interrupt();
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

void FollowMeDialogueManager::ttsSay(const std::string & sayString)
{
    // -- mute microphone
    yarp::os::Bottle bSpRecOut = {yarp::os::Value("setMic"), yarp::os::Value("mute")};
    asrConfigClient.write(bSpRecOut);

    // -- speaking
    yarp::os::Bottle bRes = {yarp::os::Value("say"), yarp::os::Value(sayString)};

    if (!speech.say(sayString))
    {
        yWarning() << "StateMachine::ttsSay() failed to say:" << sayString;
    }
    else
    {
        yDebug() << "StateMachine::ttsSay() said:" << sayString;
    }

    yarp::os::SystemClock::delaySystem(0.5);

    // -- unmute microphone
    bSpRecOut = {yarp::os::Value("setMic"), yarp::os::Value("unmute")};
    asrConfigClient.write(bSpRecOut);
}

std::string FollowMeDialogueManager::asrListen()
{
    yarp::os::Bottle * bIn = inAsrPort.read(true); // shouldWait
    yDebug() << "[FollowMeDialogueManager] Listened:" << bIn->toString();
    return bIn->get(0).asString();
}

std::string FollowMeDialogueManager::asrListenWithPeriodicWave()
{
    char position = '0'; //-- char position (l = left, c = center, r = right)

    while (true) // read loop
    {
        yarp::os::Bottle * bIn = inAsrPort.read(false); //-- IMPORTANT: should not wait

        //-- If we read something, we return it immediately
        if (!bIn)
        {
            yDebug() << "[StateMachine] Listened:" << bIn->toString();
            return bIn->get(0).asString();
        }

        // It is reading the encoder position all the time
        double encValue = headCommander.getOrientationAngle();

        if (encValue > 10.0 && position != 'l')
        {
            yarp::os::Bottle cmd = {yarp::os::Value(VOCAB_STATE_SIGNALIZE_LEFT, true)};
            armExecutionClient.write(cmd);
            yarp::os::SystemClock::delaySystem(5.0);
            ttsSay(sentences["onTheLeft"]);
            position = 'l';
        }
        else if (encValue < -10.0 && position != 'r')
        {
            yarp::os::Bottle cmd = {yarp::os::Value(VOCAB_STATE_SIGNALIZE_RIGHT, true)};
            armExecutionClient.write(cmd);
            yarp::os::SystemClock::delaySystem(5.0);
            ttsSay(sentences["onTheRight"]);
            position = 'r';
        }
        else if (encValue > -3.0 && encValue < 3.0 && position != 'c')
        {
            ttsSay(sentences["onTheCenter"]);
            position = 'c';
        }

        //-- ...to finally continue the read loop.
    }
}
