// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __STATE_MACHINE__
#define __STATE_MACHINE__

#include <yarp/os/BufferedPort.h>
#include <yarp/os/Port.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>

#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/CartesianControl.h>
#include <yarp/dev/ControlBoardInterfaces.h>

namespace roboticslab
{

/**
 * @ingroup followMeDialogueManager
 *
 * @brief implements a specific state machine for followMeDialogueManager.
 */
class StateMachine : public yarp::os::Thread
{
public:
    int getMachineState();

    /** Micro On/Off **/
    void setMicro(bool microAct);

    /** Register an input callback port for asr. */
    void setInSrPort(yarp::os::BufferedPort<yarp::os::Bottle>* inSrPort);

    /** Register an output Port for [HEAD] commands. */
    void setOutCmdHeadPort(yarp::os::RpcClient* outCmdPort);

    /** Register an output Port for [ARMS] commands. */
    void setOutCmdArmPort(yarp::os::RpcClient* outCmdPort);

    /** Register an output Port for tts. */
    void setOutTtsPort(yarp::os::RpcClient *outTtsPort);

    /** Register an output Port to configure Speech Recognition. */
    void setOutSrecPort(yarp::os::RpcClient* outSrecPort);

    /** set language in speechRecognition port */
    bool setLanguage(std::string language);

    /** set language for speaking */
    bool setSpeakLanguage(std::string language);

private:

    yarp::os::BufferedPort<yarp::os::Bottle> *inSrPort;

    yarp::os::RpcClient *outCmdHeadPort;
    yarp::os::RpcClient *outCmdArmPort;

    yarp::os::RpcClient *outTtsPort;
    yarp::os::RpcClient *outSrecPort;

    std::string _inStrState1;

    int _machineState;
    char sentence;

    std::string _language;
    bool microAct;

    // input variables
    std::string hiTeo;
    std::string followMe;
    std::string myNameIs;
    std::string stopFollowing;
    // output variables
    std::string presentation1;
    std::string presentation2;
    std::string presentation3;
    std::string askName;
    std::string answer1;
    std::string answer2;
    std::string answer3;
    std::string notUnderstand;
    std::string okFollow;
    std::string stopFollow;
    std::string onTheLeft;
    std::string onTheRight;
    std::string onTheCenter;

    // bTtsOut     -> to config or send tts commands
    // bSpRecOut   -> to config or send SpeechRecognition commands
    yarp::os::Bottle bTtsOut, bSpRecOut;


    void ttsSay(const std::string& sayString);
    std::string asrListen();
    std::string asrListenWithPeriodicWave();

    bool threadInit() override;
    void run() override;

    static const yarp::conf::vocab32_t VOCAB_STATE_SALUTE;
    static const yarp::conf::vocab32_t VOCAB_WAVE_APPROPRIATE_HAND;
    static const yarp::conf::vocab32_t VOCAB_GET_ENCODER_POSITION;
    static const yarp::conf::vocab32_t VOCAB_STATE_SIGNALIZE_RIGHT;
    static const yarp::conf::vocab32_t VOCAB_STATE_SIGNALIZE_LEFT;
};

} // namespace roboticslab

#endif
