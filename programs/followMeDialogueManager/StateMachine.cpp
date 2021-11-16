// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "StateMachine.hpp"

#include <yarp/os/LogStream.h>
#include <yarp/os/SystemClock.h>

#include "../FollowMeVocabs.hpp"

using namespace roboticslab;

bool StateMachine::threadInit()
{
    _machineState = 3;
    sentence = 'a';
    return true;
}

void StateMachine::run()
{
    ttsSay(presentation1);
    bool following = false;

    while (!isStopping())
    {
        // follow only (no speach)
        if (!microAct)
        {
            following = true;
            ttsSay(okFollow);
            yarp::os::Bottle cmd = {yarp::os::Value(VOCAB_STATE_SALUTE, true)};
            armExecutionClient->write(cmd);
            cmd = {yarp::os::Value(VOCAB_FOLLOW_ME, true)};
            headExecutionClient->write(cmd);
        }

        if (_machineState == 0)
        {
            ttsSay(presentation2);
            ttsSay(presentation3);
            _machineState = 3;
        }

        if (_machineState == 1)
        {
            ttsSay(askName);
            yarp::os::Bottle cmd = {yarp::os::Value(VOCAB_STATE_SALUTE, true)};
            armExecutionClient->write(cmd);
            _machineState = 2;
        }
        else if (_machineState == 2)
        {
            std::string inStr = asrListen();
            // Blocking
            _inStrState1 = inStr;

            if (_inStrState1.find(stopFollowing) != std::string::npos)
            {
                _machineState = 5;
            }
            else if (_inStrState1.find(myNameIs) != std::string::npos)
            {
                switch (sentence)
                {
                case 'a':
                    ttsSay(answer1);
                    sentence = 'b';
                    break;
                case 'b':
                    ttsSay(answer2);
                    sentence = 'c';
                    break;
                case 'c':
                    ttsSay(answer3);
                    sentence = 'a';
                    break;
                default:
                    break;
                }

                _machineState = 3;
            }
            else
            {
                ttsSay(notUnderstand);
                _machineState = 1;
            }
        }
        else if (_machineState == 3)
        {
            std::string inStr = following ? asrListenWithPeriodicWave() : asrListen();

            // Blocking
            _inStrState1 = inStr;

            if (_inStrState1.find(hiTeo) != std::string::npos) _machineState = 0;
            else if (_inStrState1.find(followMe) != std::string::npos) _machineState = 4;
            else if (_inStrState1.find(stopFollowing) != std::string::npos) _machineState = 5;
            else _machineState = 3;
        }
        else if (_machineState == 4)
        {
            following = true;
            ttsSay(okFollow);
            yarp::os::Bottle cmd = {yarp::os::Value(VOCAB_FOLLOW_ME, true)};
            headExecutionClient->write(cmd);
            _machineState = 1;
        }
        else if (_machineState == 5)
        {
            following = false;
            ttsSay(stopFollow);
            yarp::os::Bottle cmd = {yarp::os::Value(VOCAB_STOP_FOLLOWING, true)};
            armExecutionClient->write(cmd);
            headExecutionClient->write(cmd);
            _machineState = 3;

        }
        else
        {
            ttsSay(std::string("ANOMALY"));
            _machineState = 1;
        }
    }
}

void StateMachine::ttsSay(const std::string &sayString)
{
    // -- mute microphone
    bSpRecOut = {yarp::os::Value("setMic"), yarp::os::Value("mute")};
    asrConfigClient->write(bSpRecOut);

    // -- speaking
    yarp::os::Bottle bRes = {yarp::os::Value("say"), yarp::os::Value(sayString)};

    if (!speech->say(sayString))
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
    asrConfigClient->write(bSpRecOut);
}

std::string StateMachine::asrListen()
{
    yarp::os::Bottle * bIn = inAsrPort->read(true); // shouldWait
    yDebug() << "[StateMachine] Listened:" << bIn->toString();
    return bIn->get(0).asString();
}

std::string StateMachine::asrListenWithPeriodicWave()
{
    char position = '0'; //-- char position (l = left, c = center, r = right)

    while (true) // read loop
    {
        yarp::os::Bottle * bIn = inAsrPort->read(false); //-- IMPORTANT: should not wait

        //-- If we read something, we return it immediately
        if (!bIn)
        {
            yDebug() << "[StateMachine] Listened:" << bIn->toString();
            return bIn->get(0).asString();
        }

        // It is reading the encoder position all the time
        yarp::os::Bottle cmd = {yarp::os::Value(VOCAB_GET_ENCODER_POSITION, true)};
        yarp::os::Bottle encValue;
        headExecutionClient->write(cmd, encValue);

        if (encValue.get(0).asFloat64() > 10.0 && position != 'l')
        {
            cmd = {yarp::os::Value(VOCAB_STATE_SIGNALIZE_LEFT, true)};
            armExecutionClient->write(cmd);
            yarp::os::SystemClock::delaySystem(5.0);
            ttsSay(onTheLeft);
            position = 'l';
        }
        else if (encValue.get(0).asFloat64() < -10.0 && position != 'r')
        {
            cmd = {yarp::os::Value(VOCAB_STATE_SIGNALIZE_RIGHT, true)};
            armExecutionClient->write(cmd);
            yarp::os::SystemClock::delaySystem(5.0);
            ttsSay(onTheRight);
            position = 'r';
        }
        else if (encValue.get(0).asFloat64() > -3.0 && encValue.get(0).asFloat64() < 3.0 && position != 'c')
        {
            ttsSay(onTheCenter);
            position = 'c';
        }

        //-- ...to finally continue the read loop.
    }
}

int StateMachine::getMachineState()
{
    return _machineState;
}

void StateMachine::setMicro(bool microAct)
{
    this->microAct = microAct;
}

void StateMachine::setInAsrPort(yarp::os::BufferedPort<yarp::os::Bottle> * inAsrPort)
{
    this->inAsrPort = inAsrPort;
}

void StateMachine::setHeadExecutionClient(yarp::os::RpcClient * headExecutionClient)
{
    this->headExecutionClient = headExecutionClient;
}

void StateMachine::setArmExecutionClient(yarp::os::RpcClient * armExecutionClient)
{
    this->armExecutionClient = armExecutionClient;
}

void StateMachine::setTtsClient(SpeechIDL * speech)
{
    this->speech = speech;
}

void StateMachine::setAsrConfigClient(yarp::os::RpcClient * asrConfigClient)
{
    this->asrConfigClient = asrConfigClient;
}

bool StateMachine::setLanguage(const std::string & language)
{
    _language = language;

    if (language == "english")
    {
        //-- recognition sentences
        hiTeo = "hi teo";
        followMe = "follow me";
        myNameIs = "my name is";
        stopFollowing = "stop following";

        return true;
    }
    else if (language == "spanish")
    {
        //-- frases de reconociomiento
        hiTeo = "hola teo";
        followMe = "sigueme";
        myNameIs = "me llamo";
        stopFollowing = "para teo";

        return true;
    }
    else
    {
        yError() << "Unsupported language:" << language;
        return false;
    }
}

bool StateMachine::setSpeakLanguage(const std::string & language)
{
    if (language == "english")
    {
        //-- speak sentences
        presentation1 = "Follow me, demostration started";
        presentation2 = "Hello. My name is TEO. I am, a humanoid robot, of Carlos tercero, university.";
        presentation3 = "Now, I will follow you. Please, tell me";
        askName = "Could you tell me your name";
        answer1 = "Is, a beatifull name. I love it";
        answer2 = "Is, a wonderfull name. My human creator, has the same name";
        answer3 = "My parents, didn't want to baptize me, with that name.";
        notUnderstand = "Sorry, I don't understand";
        okFollow = "Okay, I will follow you";
        stopFollow = "Okay, I will stop following you. See you later";
        onTheRight = "You are, on my, right";
        onTheLeft = "You are, on my, left";
        onTheCenter = "You are, on the, center";
        return true;
    }
    else if (language == "spanish")
    {
        //-- speak sentences
        presentation1 = "Demostración de detección de caras iniciada";
        presentation2 = "Hola. Me yamo Teo, y soy un grobot humanoide diseñado por ingenieros de la universidad carlos tercero";
        presentation3 = "Por favor, dime qué quieres que haga";
        askName = "Podrías decirme tu nombre";
        answer1 = "Uuooooo ouu, que nombre más bonito. Me encanta";
        answer2 = "Que gran nombre. Mi creador humano se yama igual";
        answer3 = "Mis padres no quisieron bauuutizarme con ese nombre. Malditos.";
        notUnderstand = "Lo siento. No te he entendido";
        okFollow = "Vale. Voy, a comenzar a seguirte";
        stopFollow = "De acuerdo. Voy, a dejar de seguirte. Hasta pronto.";
        onTheRight = "Ahora, estás, a mi derecha";
        onTheLeft = "Ahora, estás, a mi izquierda";
        onTheCenter = "Ahora, estás, en el centro";
        return true;
    }
    else
    {
        yError() << "Unsupported language:" << language;
        return false;
    }
}
