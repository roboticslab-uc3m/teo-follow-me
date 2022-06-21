// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "FollowMeDialogueManager.hpp"

#include <yarp/os/LogStream.h>
#include <yarp/os/SystemClock.h>

using namespace roboticslab;

namespace
{
    using state = FollowMeDialogueManager::state;
    using snt = FollowMeDialogueManager::sentence;
    using cmd = FollowMeDialogueManager::command;

    const std::unordered_map<snt, std::string> englishSentences = {
        {snt::PRESENTATION_1, "Follow me, demostration started."},
        {snt::PRESENTATION_2, "Hello. My name is TEO. I am, a humanoid robot, of Carlos tercero, university."},
        {snt::PRESENTATION_3, "Now, I will follow you. Please, tell me."},
        {snt::ASK_NAME, "Could you tell me your name."},
        {snt::ANSWER_1, "Is, a beatifull name. I love it."},
        {snt::ANSWER_2, "Is, a wonderfull name. My human creator, has the same name."},
        {snt::ANSWER_3, "My parents, didn't want to baptize me, with that name."},
        {snt::NOT_UNDERSTAND, "Sorry, I don't understand."},
        {snt::FOLLOW, "Okay, I will follow you."},
        {snt::STOP_FOLLOWING, "Okay, I will stop following you. See you later."},
        {snt::ON_THE_RIGHT, "You are, on my, right."},
        {snt::ON_THE_LEFT, "You are, on my, left."},
        {snt::ON_THE_CENTER, "You are, on the, center."},
    };

    const std::unordered_map<cmd, std::string> englishCommands = {
        {cmd::HI_TEO, "hi teo"},
        {cmd::FOLLOW_ME, "follow me"},
        {cmd::MY_NAME_IS, "my name is"},
        {cmd::STOP_FOLLOWING, "stop following"},
    };

    const std::unordered_map<snt, std::string> spanishSentences = {
        {snt::PRESENTATION_1, "Demostración de detección de caras iniciada."},
        {snt::PRESENTATION_2, "Hola. Me yamo Teo, y soy un grobot humanoide diseñado por ingenieros de la universidad carlos tercero."},
        {snt::PRESENTATION_3, "Por favor, dime qué quieres que haga."},
        {snt::ASK_NAME, "Podrías decirme tu nombre."},
        {snt::ANSWER_1, "Uuooooo ouu, que nombre más bonito. Me encanta."},
        {snt::ANSWER_2, "Que gran nombre. Mi creador humano se yama igual."},
        {snt::ANSWER_3, "Mis padres no quisieron bauuutizarme con ese nombre. Malditos."},
        {snt::NOT_UNDERSTAND, "Lo siento. No te he entendido."},
        {snt::FOLLOW, "Vale. Voy, a comenzar a seguirte."},
        {snt::STOP_FOLLOWING, "De acuerdo. Voy, a dejar de seguirte. Hasta pronto."},
        {snt::ON_THE_RIGHT, "Ahora, estás, a mi derecha."},
        {snt::ON_THE_LEFT, "Ahora, estás, a mi izquierda."},
        {snt::ON_THE_CENTER, "Ahora, estás, en el centro."},
    };

    const std::unordered_map<cmd, std::string> spanishCommands = {
        {cmd::HI_TEO, "hola teo"},
        {cmd::FOLLOW_ME, "sigueme"},
        {cmd::MY_NAME_IS, "me llamo"},
        {cmd::STOP_FOLLOWING, "para teo"},
    };

    std::string getStateDescription(state s)
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

    return true;
}

double FollowMeDialogueManager::getPeriod()
{
    return 0.1; // [s]
}

bool FollowMeDialogueManager::updateModule()
{
    static const auto throttle = 1.0; // [s]
    auto [isConnected, port, description] = checkOutputConnections();

    if (!isConnected)
    {
        if (yarp::os::Thread::isRunning())
        {
            yInfo() << "Port" << port << "disconnected, forcing presentation stop";

            if (!yarp::os::Thread::stop())
            {
                yError() << "Unable to stop presentation thread";
                return false;
            }
        }
        else
        {
            yInfoThrottle(throttle) << "Waiting for" << port << "to be connected to" << description;
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

bool FollowMeDialogueManager::threadInit()
{
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

void FollowMeDialogueManager::run()
{
    ttsSayAndWait(sentence::PRESENTATION_1);
    bool isFollowing = false;

    enum class answers { ANSWER_1, ANSWER_2, ANSWER_3 };
    auto answer = answers::ANSWER_1;

    std::string listened;

    if (!usingMic)
    {
        // override state machine if no mic found, keep lingering until thread is stopped
        armCommander.doGreet();
        headCommander.enableFollowing();
        ttsSayAndWait(sentence::FOLLOW);
        asrListenAndLinger(); // this will loop indefinitely
    }

    while (!yarp::os::Thread::isStopping())
    {
        switch (machineState)
        {
        case state::PRESENTATION:
            ttsSayAndWait(sentence::PRESENTATION_2);
            ttsSayAndWait(sentence::PRESENTATION_3);
            machineState = state::LISTEN;
            break;

        case state::ASK_NAME:
            armCommander.doGreet();
            ttsSayAndWait(sentence::ASK_NAME);
            machineState = state::DIALOGUE;
            break;

        case state::DIALOGUE:
            listened = asrListenAndWait();

            if (listened.find(voiceCommands[command::STOP_FOLLOWING]) != std::string::npos)
            {
                machineState = state::STOP_FOLLOWING;
            }
            else if (listened.find(voiceCommands[command::MY_NAME_IS]) != std::string::npos)
            {
                switch (answer)
                {
                case answers::ANSWER_1:
                    ttsSayAndWait(sentence::ANSWER_1);
                    answer = answers::ANSWER_2;
                    break;
                case answers::ANSWER_2:
                    ttsSayAndWait(sentence::ANSWER_2);
                    answer = answers::ANSWER_3;
                    break;
                case answers::ANSWER_3:
                    ttsSayAndWait(sentence::ANSWER_3);
                    answer = answers::ANSWER_1;
                    break;
                }

                machineState = state::LISTEN;
            }
            else
            {
                ttsSayAndWait(sentence::NOT_UNDERSTAND);
                machineState = state::ASK_NAME;
            }

            break;

        case state::LISTEN:
            listened = isFollowing ? asrListenAndLinger() : asrListenAndWait();

            if (listened.find(voiceCommands[command::HI_TEO]) != std::string::npos)
                machineState = state::PRESENTATION;
            else if (listened.find(voiceCommands[command::FOLLOW_ME]) != std::string::npos)
                machineState = state::FOLLOW;
            else if (listened.find(voiceCommands[command::STOP_FOLLOWING]) != std::string::npos)
                machineState = state::STOP_FOLLOWING;
            else
                machineState = state::LISTEN;

            break;

        case state::FOLLOW:
            headCommander.enableFollowing();
            ttsSayAndWait(sentence::FOLLOW);
            machineState = state::ASK_NAME;
            isFollowing = true;
            break;

        case state::STOP_FOLLOWING:
            armCommander.disableArmSwinging();
            headCommander.disableFollowing();
            ttsSayAndWait(sentence::STOP_FOLLOWING);
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

void FollowMeDialogueManager::ttsSayAndWait(sentence snt)
{
    if (usingMic && !asr.muteMicrophone())
    {
        yWarning() << "Failed to mute microphone";
    }

    if (auto sayString = sentences[snt]; !tts.say(sayString))
    {
        yWarning() << "Failed to say:" << sayString;
    }
    else
    {
        yDebug() << "Now saying:" << sayString;

        do
        {
            yarp::os::SystemClock::delaySystem(0.1);
        }
        while (!yarp::os::Thread::isStopping() && !tts.checkSayDone());
    }

    yarp::os::SystemClock::delaySystem(1.0); // more time due to ASR

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
    auto pos = position::UNKNOWN;

    while (!yarp::os::Thread::isStopping())
    {
        // don't wait
        if (const auto * b = inAsrPort.read(false); b && b->size() > 0)
        {
            auto text = b->get(0).asString();
            yDebug() << "Listened:" << text;
            return text; // return as soon as something is heard
        }

        double encValue = headCommander.getOrientationAngle();

        if (encValue > SIGNAL_THRESHOLD && pos != position::LEFT)
        {
            armCommander.doSignalLeft();
            ttsSayAndWait(sentence::ON_THE_LEFT);
            pos = position::LEFT;
        }
        else if (encValue < -SIGNAL_THRESHOLD && pos != position::RIGHT)
        {
            armCommander.doSignalRight();
            ttsSayAndWait(sentence::ON_THE_RIGHT);
            pos = position::RIGHT;
        }
        else if (encValue > -CENTER_THRESHOLD && encValue < CENTER_THRESHOLD && pos != position::CENTER)
        {
            ttsSayAndWait(sentence::ON_THE_CENTER);
            pos = position::CENTER;
        }

        yarp::os::SystemClock::delaySystem(0.5);
    }

    return {};
}
