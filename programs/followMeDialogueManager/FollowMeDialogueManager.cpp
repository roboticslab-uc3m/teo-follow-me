// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeDialogueManager.hpp"

#include <yarp/os/LogStream.h>
#include <yarp/os/SystemClock.h>

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
constexpr auto DEFAULT_MICRO = false;
constexpr auto ASR_DICTIONARY = "follow-me";
constexpr auto SIGNAL_THRESHOLD = 10.0; // [deg]
constexpr auto CENTER_THRESHOLD = 3.0; // [deg]

bool FollowMeDialogueManager::configure(yarp::os::ResourceFinder & rf)
{
    auto language = rf.check("language", yarp::os::Value(DEFAULT_LANGUAGE), "language to be used").asString();
    usingMic = rf.check("useMic", "enable microphone");

    if (rf.check("help"))
    {
        yInfo("FollowMeDialogueManager options:");
        yInfo("\t--help (this help)\t--from [file.ini]\t--context [path]");
        yInfo("\t--language: %s [%s]", language.c_str(), DEFAULT_LANGUAGE);
        yInfo("\t--useMic: %d [%d]", usingMic, DEFAULT_MICRO);
        return false;
    }

    if (!armExecutionClient.open(DEFAULT_PREFIX + std::string("/arms/rpc:c")))
    {
        yError() << "Failed to open arm execution client port" << armExecutionClient.getName();
        return false;
    }

    if (!headExecutionClient.open(DEFAULT_PREFIX + std::string("/head/rpc:c")))
    {
        yError() << "Failed to open head execution client port" << headExecutionClient.getName();
        return false;
    }

    if (!ttsClient.open(DEFAULT_PREFIX + std::string("/tts/rpc:c")))
    {
        yError() << "Failed to open TTS client port" << ttsClient.getName();
        return false;
    }

    if (usingMic && !asrConfigClient.open(DEFAULT_PREFIX + std::string("/speechRecognition/rpc:c")))
    {
        yError() << "Failed to open ASR config client port" << asrConfigClient.getName();
        return false;
    }

    if (usingMic && !inAsrPort.open(DEFAULT_PREFIX + std::string("/speechRecognition:i")))
    {
        yError() << "Failed to open ASR listener port" << inAsrPort.getName();
        return false;
    }

    armCommander.yarp().attachAsClient(armExecutionClient);
    headCommander.yarp().attachAsClient(headExecutionClient);
    tts.yarp().attachAsClient(ttsClient);

    if (usingMic)
    {
        asr.yarp().attachAsClient(asrConfigClient);
    }

    std::string voice, langCode;

    if (language == "english")
    {
        voice = "mb-en1";
        langCode = "en-us";
        sentences = englishSentences;
        voiceCommands = englishCommands;
    }
    else if (language == "spanish")
    {
        voice = "mb-es1";
        langCode = "es";
        sentences = spanishSentences;
        voiceCommands = spanishCommands;
    }
    else
    {
        yError() << "Unsupported language, please use '--language english' or '--language spanish', got:" << language;
        return false;
    }

    if (!tts.setLanguage(voice))
    {
        yError() << "Failed to set TTS voice to" << voice;
        return false;
    }

    if (usingMic && !asr.setDictionary(ASR_DICTIONARY, langCode))
    {
        yError() << "Failed to set ASR dictionary to" << ASR_DICTIONARY << "and language code to" << langCode;
        return false;
    }

    return true;
}

double FollowMeDialogueManager::getPeriod()
{
    return 0.1; // [s]
}

bool FollowMeDialogueManager::updateModule()
{
    constexpr auto throttle = 1.0; // [s]
    auto connectionState = checkOutputConnections();

    if (!std::get<0>(connectionState))
    {
        if (yarp::os::Thread::isRunning())
        {
            yInfo() << "Port" << std::get<1>(connectionState) << "disconnected, forcing presentation stop";

            if (!yarp::os::Thread::stop())
            {
                yError() << "Unable to stop presentation thread";
                return false;
            }
        }
        else
        {
            yInfoThrottle(throttle) << "Waiting for" << std::get<1>(connectionState) << "to be connected to" << std::get<2>(connectionState);
        }
    }
    else
    {
        if (yarp::os::Thread::isRunning())
        {
            yDebugThrottle(throttle) << "Presentation is running in state" << getStateDescription(machineState);
        }
        else
        {
            yInfo() << "Starting presentation thread";

            if (!yarp::os::Thread::start())
            {
                yError() << "Unable to start presentation thread";
                return false;
            }
        }
    }

    return true;
}

bool FollowMeDialogueManager::interruptModule()
{
    headCommander.stop();
    armCommander.stop();
    tts.stop();

    headExecutionClient.interrupt();
    armExecutionClient.interrupt();
    ttsClient.interrupt();

    if (usingMic)
    {
        asrConfigClient.interrupt();
        inAsrPort.interrupt();
    }

    return yarp::os::Thread::stop();
}

bool FollowMeDialogueManager::close()
{
    headExecutionClient.close();
    armExecutionClient.close();
    ttsClient.close();

    if (usingMic)
    {
        asrConfigClient.close();
        inAsrPort.close();
    }

    return true;
}

void FollowMeDialogueManager::run()
{
    ttsSayAndWait(sentences["presentation1"]);
    bool isFollowing = false;

    enum class answers { ANSWER1, ANSWER2, ANSWER3 };
    answers answer = answers::ANSWER1;

    std::string listened;

    if (!usingMic)
    {
        // override state machine if no mic found, keep lingering until thread is stopped
        armCommander.doGreet();
        headCommander.enableFollowing();
        ttsSayAndWait(sentences["okFollow"]);
        asrListenAndLinger(); // this will loop indefinitely
    }

    while (!yarp::os::Thread::isStopping())
    {
        switch (machineState)
        {
        case state::PRESENTATION:
            ttsSayAndWait(sentences["presentation2"]);
            ttsSayAndWait(sentences["presentation3"]);
            machineState = state::LISTEN;
            break;

        case state::ASK_NAME:
            armCommander.doGreet();
            ttsSayAndWait(sentences["askName"]);
            machineState = state::DIALOGUE;
            break;

        case state::DIALOGUE:
            listened = asrListenAndWait();

            if (listened.find(voiceCommands["stopFollowing"]) != std::string::npos)
            {
                machineState = state::STOP_FOLLOWING;
            }
            else if (listened.find(voiceCommands["myNameIs"]) != std::string::npos)
            {
                switch (answer)
                {
                case answers::ANSWER1:
                    ttsSayAndWait(sentences["answer1"]);
                    answer = answers::ANSWER2;
                    break;
                case answers::ANSWER2:
                    ttsSayAndWait(sentences["answer2"]);
                    answer = answers::ANSWER3;
                    break;
                case answers::ANSWER3:
                    ttsSayAndWait(sentences["answer3"]);
                    answer = answers::ANSWER1;
                    break;
                }

                machineState = state::LISTEN;
            }
            else
            {
                ttsSayAndWait(sentences["notUnderstand"]);
                machineState = state::ASK_NAME;
            }

            break;

        case state::LISTEN:
            listened = isFollowing ? asrListenAndLinger() : asrListenAndWait();

            if (listened.find(voiceCommands["hiTeo"]) != std::string::npos)
                machineState = state::PRESENTATION;
            else if (listened.find(voiceCommands["followMe"]) != std::string::npos)
                machineState = state::FOLLOW;
            else if (listened.find(voiceCommands["stopFollowing"]) != std::string::npos)
                machineState = state::STOP_FOLLOWING;
            else
                machineState = state::LISTEN;

            break;

        case state::FOLLOW:
            headCommander.enableFollowing();
            ttsSayAndWait(sentences["okFollow"]);
            machineState = state::ASK_NAME;
            isFollowing = true;
            break;

        case state::STOP_FOLLOWING:
            armCommander.disableArmSwinging();
            headCommander.disableFollowing();
            ttsSayAndWait(sentences["stopFollow"]);
            machineState = state::LISTEN;
            isFollowing = false;
            break;
        }

        yarp::os::SystemClock::delaySystem(0.5);
    }
}

std::tuple<bool, std::string, std::string> FollowMeDialogueManager::checkOutputConnections()
{
    if (armExecutionClient.getOutputCount() == 0)
    {
        return {false, armExecutionClient.getName(), "arm execution server"};
    }

    if (headExecutionClient.getOutputCount() == 0)
    {
        return {false, headExecutionClient.getName(), "head execution server"};
    }

    if (ttsClient.getOutputCount() == 0)
    {
        return {false, ttsClient.getName(), "TTS server"};
    }

    if (usingMic && asrConfigClient.getOutputCount() == 0)
    {
        return {false, asrConfigClient.getName(), "ASR config server"};
    }

    if (usingMic && inAsrPort.getInputCount() == 0)
    {
        return {false, inAsrPort.getName(), "ASR listener server"};
    }

    return {true, {}, {}};
}

void FollowMeDialogueManager::ttsSayAndWait(const std::string & sayString)
{
    if (usingMic && !asr.muteMicrophone())
    {
        yWarning() << "Failed to mute microphone";
    }

    if (!tts.say(sayString))
    {
        yWarning() << "Failed to say:" << sayString;
    }
    else
    {
        yDebug() << "Now saying:" << sayString;

        do
        {
            yarp::os::SystemClock::delaySystem(1.0); // more time due to ASR
        }
        while (!yarp::os::Thread::isStopping() && !tts.checkSayDone());
    }

    if (usingMic && !asr.unmuteMicrophone())
    {
        yWarning() << "Failed to unmute microphone";
    }
}

std::string FollowMeDialogueManager::asrListenAndWait()
{
    const yarp::os::Bottle * b = nullptr;

    while (!yarp::os::Thread::isStopping() && !b)
    {
        b = inAsrPort.read(false); // don't block
        yarp::os::SystemClock::delaySystem(0.1);
    }

    if (b && b->size() > 0)
    {
        auto text = b->get(0).asString();
        yDebug() << "Listened:" << text;
        return text;
    }

    return {};
}

std::string FollowMeDialogueManager::asrListenAndLinger()
{
    enum class position { UNKNOWN, LEFT, CENTER, RIGHT };
    position pos = position::UNKNOWN;

    while (!yarp::os::Thread::isStopping())
    {
        const auto * b = inAsrPort.read(false); // don't wait

        if (b && b->size() > 0)
        {
            auto text = b->get(0).asString();
            yDebug() << "Listened:" << text;
            return text; // return as soon as something is heard
        }

        double encValue = headCommander.getOrientationAngle();

        if (encValue > SIGNAL_THRESHOLD && pos != position::LEFT)
        {
            armCommander.doSignalLeft();
            ttsSayAndWait(sentences["onTheLeft"]);
            pos = position::LEFT;
        }
        else if (encValue < -SIGNAL_THRESHOLD && pos != position::RIGHT)
        {
            armCommander.doSignalRight();
            ttsSayAndWait(sentences["onTheRight"]);
            pos = position::RIGHT;
        }
        else if (encValue > -CENTER_THRESHOLD && encValue < CENTER_THRESHOLD && pos != position::CENTER)
        {
            ttsSayAndWait(sentences["onTheCenter"]);
            pos = position::CENTER;
        }

        yarp::os::SystemClock::delaySystem(0.5);
    }

    return {};
}

std::string FollowMeDialogueManager::getStateDescription(state s)
{
    switch (s)
    {
    case state::PRESENTATION:
        return "presentation";
    case state::ASK_NAME:
        return "ask name";
    case state::DIALOGUE:
        return "dialogue";
    case state::LISTEN:
        return "listen";
    case state::FOLLOW:
        return "follow";
    case state::STOP_FOLLOWING:
        return "stop following";
    default:
        return "unknown";
    }
}
