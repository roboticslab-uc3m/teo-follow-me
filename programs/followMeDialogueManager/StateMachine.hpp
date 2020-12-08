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
    void setInAsrPort(yarp::os::BufferedPort<yarp::os::Bottle>* inAsrPort);

    /** Register an output Port for [HEAD] commands. */
    void setHeadExecutionClient(yarp::os::RpcClient* headExecutionClient);

    /** Register an output Port for [ARMS] commands. */
    void setArmExecutionClient(yarp::os::RpcClient* armExecutionClient);

    /** Register an output Port for tts. */
    void setTtsClient(yarp::os::RpcClient *ttsClient);

    /** Register an output Port to configure Speech Recognition. */
    void setAsrConfigClient(yarp::os::RpcClient* asrConfigClient);

    /** set language in speechRecognition port */
    bool setLanguage(std::string language);

    /** set language for speaking */
    bool setSpeakLanguage(std::string language);

private:

    yarp::os::BufferedPort<yarp::os::Bottle> *inAsrPort;

    yarp::os::RpcClient *headExecutionClient;
    yarp::os::RpcClient *armExecutionClient;

    yarp::os::RpcClient *ttsClient;
    yarp::os::RpcClient *asrConfigClient;

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
};

} // namespace roboticslab

#endif
