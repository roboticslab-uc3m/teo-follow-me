// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "StateMachine.hpp"

#include "../FollowMeVocabs.hpp"

namespace roboticslab
{

/************************************************************************/

const yarp::conf::vocab32_t StateMachine::VOCAB_WAVE_APPROPRIATE_HAND = yarp::os::createVocab('w','a','p','h');
const yarp::conf::vocab32_t StateMachine::VOCAB_GET_ENCODER_POSITION = yarp::os::createVocab('g','e','p','s');

/************************************************************************/

bool StateMachine::threadInit() {
    _machineState = 3;
    sentence = 'a';
    return true;
}

/************************************************************************/

void StateMachine::run() {
    ttsSay( presentation1 );
    bool following = false;

    while(!isStopping()) {

        // follow only (no speach)
        if(!microAct) {
            following = true;
            ttsSay( okFollow );
            yarp::os::Bottle cmd;
            cmd.addVocab(VOCAB_STATE_SALUTE);
            outCmdArmPort->write(cmd);
            cmd.clear();
            cmd.addVocab(VOCAB_FOLLOW_ME);
            outCmdHeadPort->write(cmd);
        }

        if(_machineState == 0) {
            ttsSay( presentation2 );
            ttsSay( presentation3 );
            _machineState = 3;
        }

        if(_machineState == 1)
        {
            ttsSay( askName );
            yarp::os::Bottle cmd;
            cmd.addVocab(VOCAB_STATE_SALUTE);
            outCmdArmPort->write(cmd);
            _machineState=2;
        }
        else if(_machineState == 2)
        {

            std::string inStr = asrListen();
            // Blocking
            _inStrState1 = inStr;
            if((_inStrState1.find(stopFollowing) != std::string::npos))
                _machineState = 5;

            else if((_inStrState1.find(myNameIs) != std::string::npos))
            {

                switch (sentence) {
                case 'a':
                    ttsSay( answer1 );
                    sentence = 'b';
                    break;
                case 'b':
                    ttsSay( answer2 );
                    sentence = 'c';
                    break;
                case 'c':
                    ttsSay( answer3 );
                    sentence = 'a';
                    break;
                default:
                    break;
                }
                _machineState=3;
            }
            else
            {
                ttsSay( notUnderstand );
                _machineState=1;
            }
        }
        else if(_machineState==3)
        {

            std::string inStr;
            if(following) inStr = asrListenWithPeriodicWave();
            else inStr = asrListen();

            // Blocking
            _inStrState1 = inStr;
            if( _inStrState1.find(hiTeo) != std::string::npos ) _machineState=0;
            else if( _inStrState1.find(followMe) != std::string::npos ) _machineState=4;
            else if ( _inStrState1.find(stopFollowing) != std::string::npos ) _machineState=5;
            else _machineState=3;

        } else if (_machineState==4) {

            following = true;
            ttsSay( okFollow );
            yarp::os::Bottle cmd;
            cmd.addVocab(VOCAB_FOLLOW_ME);
            outCmdHeadPort->write(cmd);
            _machineState=1;


        } else if (_machineState==5) {

            following = false;
            ttsSay( stopFollow );
            yarp::os::Bottle cmd;
            cmd.addVocab(VOCAB_STOP_FOLLOWING);
            outCmdArmPort->write(cmd);
            outCmdHeadPort->write(cmd);
            _machineState=3;

        } else {
            ttsSay( std::string("ANOMALY") );
            _machineState=1;
        }
    }
}

/************************************************************************/

void StateMachine::ttsSay(const std::string &sayString) {

    // -- mute microphone
    bSpRecOut.clear();
    bSpRecOut.addString("setMic");
    bSpRecOut.addString("mute");
    outSrecPort->write(bSpRecOut);

    // -- speaking
    yarp::os::Bottle bRes;
    bTtsOut.clear();
    bTtsOut.addString("say");
    bTtsOut.addString(sayString);
    outTtsPort->write(bTtsOut,bRes);
    printf("[StateMachine] Said: %s [%s]\n", sayString.c_str(), bRes.toString().c_str());
    yarp::os::Time::delay(0.5);

    // -- unmute microphone
    bSpRecOut.clear();
    bSpRecOut.addString("setMic");
    bSpRecOut.addString("unmute");
    outSrecPort->write(bSpRecOut);

    return;
}

/************************************************************************/

std::string StateMachine::asrListen()
{
    yarp::os::Bottle* bIn = inSrPort->read(true);  // shouldWait
    printf("[StateMachine] Listened: %s\n", bIn->toString().c_str());
    return bIn->get(0).asString();
}

/************************************************************************/

std::string StateMachine::asrListenWithPeriodicWave() {
    char position = '0'; //-- char position (l = left, c = center, r = right)

    while( true ) // read loop
    {
        yarp::os::Bottle* bIn = inSrPort->read(false);  //-- IMPORTANT: should not wait
        //-- If we read something, we return it immediately
        if ( bIn != NULL)
        {
            printf("[StateMachine] Listened: %s\n", bIn->toString().c_str());
            return bIn->get(0).asString();
        }

        // It is reading the encoder position all the time
        yarp::os::Bottle cmd, encValue;
        cmd.clear();
        encValue.clear();
        cmd.addVocab(VOCAB_GET_ENCODER_POSITION);
        outCmdHeadPort->write(cmd, encValue);
        printf("EncValue -> %f\n", encValue.get(0).asDouble());

        if( (encValue.get(0).asDouble() > 10) && (position!='l') ) {
            yarp::os::Bottle cmd;
            cmd.addVocab(VOCAB_STATE_SIGNALIZE_LEFT);
            outCmdArmPort->write(cmd);
            yarp::os::Time::delay(5);
            ttsSay( onTheLeft );
            position = 'l';
        }
        else if( (encValue.get(0).asDouble() < -10) && (position!='r') ) {
            yarp::os::Bottle cmd;
            cmd.addVocab(VOCAB_STATE_SIGNALIZE_RIGHT);
            outCmdArmPort->write(cmd);
            yarp::os::Time::delay(5);
            ttsSay( onTheRight );
            position = 'r';
        }
        else if( (encValue.get(0).asDouble() > -3) && (encValue.get(0).asDouble() < 3) && (position!='c') ) {
            ttsSay( onTheCenter );
            position = 'c';
        }

        //-- ...to finally continue the read loop.
    }
}

/************************************************************************/

int StateMachine::getMachineState() {
    return _machineState;
}

/************************************************************************/

void StateMachine::setMicro(bool microAct) {
    this->microAct = microAct;
}

/************************************************************************/

void StateMachine::setInSrPort(yarp::os::BufferedPort<yarp::os::Bottle>* inSrPort) {
    this->inSrPort = inSrPort;
}

/************************************************************************/

void StateMachine::setOutCmdHeadPort(yarp::os::RpcClient* outCmdPort) {
    this->outCmdHeadPort = outCmdPort;
}

/************************************************************************/

void StateMachine::setOutCmdArmPort(yarp::os::RpcClient* outCmdPort) {
    this->outCmdArmPort = outCmdPort;
}

/************************************************************************/

void StateMachine::setOutTtsPort(yarp::os::RpcClient* outTtsPort) {
    this->outTtsPort = outTtsPort;
}

/************************************************************************/

void StateMachine::setOutSrecPort(yarp::os::RpcClient* outSrecPort) {
    this->outSrecPort = outSrecPort;
}

/************************************************************************/
bool StateMachine::setLanguage(std::string language)
{
    _language = language;
    if("english" == language)
    {
        //-- recognition sentences
        hiTeo = std::string ("hi teo");
        followMe = std::string ("follow me");
        myNameIs = std::string ("my name is");
        stopFollowing = std::string ("stop following");

        return true;
    }
    else if("spanish" == language)
    {
        //-- frases de reconociomiento
        hiTeo = std::string ("hola teo");
        followMe = std::string ("sigueme");
        myNameIs = std::string ("me llamo");
        stopFollowing = std::string ("para teo");

        return true;
    }
    else
    {
        printf("error! %s????\n",language.c_str());
        return false;
    }
}

/************************************************************************/

bool StateMachine::setSpeakLanguage(std::string language) {

    if("english" == language)
    {
        //-- speak sentences
        presentation1 = std::string("Follow me, demostration started");
        presentation2 = std::string("Hello. My name is TEO. I am, a humanoid robot, of Carlos tercero, university.");
        presentation3 = std::string("Now, I will follow you. Please, tell me");
        askName = std::string("Could you tell me your name");
        answer1 = std::string("Is, a beatifull name. I love it");
        answer2 = std::string("Is, a wonderfull name. My human creator, has the same name");
        answer3 = std::string("My parents, didn't want to baptize me, with that name.");
        notUnderstand = std::string("Sorry, I don't understand");
        okFollow = std::string("Okay, I will follow you");
        stopFollow = std::string("Okay, I will stop following you. See you later");
        onTheRight = std::string("You are, on my, right");
        onTheLeft = std::string("You are, on my, left");
        onTheCenter = std::string("You are, on the, center");

        return true;

    }
    else if("spanish" == language)
    {
        //-- speak sentences
        presentation1 = std::string("Demostración de detección de caras iniciada");
        presentation2 = std::string("Hola. Me yamo Teo, y soy un grobot humanoide diseñado por ingenieros de la universidad carlos tercero");
        presentation3 = std::string("Por favor, dime qué quieres que haga");
        askName = std::string("Podrías decirme tu nombre");
        answer1 = std::string("Uuooooo ouu, que nombre más bonito. Me encanta");
        answer2 = std::string("Que gran nombre. Mi creador humano se yama igual");
        answer3 = std::string("Mis padres no quisieron bauuutizarme con ese nombre. Malditos.");
        notUnderstand = std::string("Lo siento. No te he entendido");
        okFollow = std::string("Vale. Voy, a comenzar a seguirte");
        stopFollow = std::string("De acuerdo. Voy, a dejar de seguirte. Hasta pronto.");
        onTheRight = std::string("Ahora, estás, a mi derecha");
        onTheLeft = std::string("Ahora, estás, a mi izquierda");
        onTheCenter = std::string("Ahora, estás, en el centro");
    }
    else
    {
        printf("error! %s????\n",language.c_str());
        return false;
    }
}

} // namespace roboticslab
