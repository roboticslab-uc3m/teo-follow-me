// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __FM_DIALOGUE_MANAGER_HPP__
#define __FM_DIALOGUE_MANAGER_HPP__

#include <yarp/os/Bottle.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RpcClient.h>

#include "StateMachine.hpp"

#define DEFAULT_LANGUAGE "english"
#define DEFAULT_MICRO "off"

namespace roboticslab
{

/**
 * @ingroup follow-me_programs
 *
 * @brief Dialogue Manager.
 */
class FollowMeDialogueManager : public yarp::os::RFModule
{
public:
    bool configure(yarp::os::ResourceFinder &rf) override;

private:
    StateMachine stateMachine;
    yarp::os::BufferedPort<yarp::os::Bottle> inAsrPort;
    yarp::os::RpcClient ttsClient;
    yarp::os::RpcClient asrConfigClient;
    yarp::os::RpcClient headExecutionClient;
    yarp::os::RpcClient armExecutionClient;

    // bTtsOut     -> to config or send tts commands
    // bSpRecOut   -> to config or send SpeechRecognition commands
    yarp::os::Bottle bTtsOut, bSpRecOut;

    bool interruptModule();
    double getPeriod();
    bool updateModule();

    // micro (on/off) to give speaking orders to TEO
    bool microState;
    void setMicro(bool microAct);
};

} // namespace roboticslab

#endif  // __FM_DIALOGUE_MANAGER_HPP__
