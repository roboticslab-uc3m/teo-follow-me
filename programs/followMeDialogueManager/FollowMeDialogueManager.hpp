// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __FM_DIALOGUE_MANAGER_HPP__
#define __FM_DIALOGUE_MANAGER_HPP__

#include <yarp/os/all.h>
#include <stdlib.h>

#include "StateMachine.hpp"

#define DEFAULT_LANGUAGE "english"
#define DEFAULT_MICRO "off"

namespace teo
{

/**
 * @ingroup follow-me_programs
 *
 * @brief Dialogue Manager.
 */
class FollowMeDialogueManager : public yarp::os::RFModule
{
public:
    bool configure(yarp::os::ResourceFinder &rf);

private:
    StateMachine stateMachine;
    yarp::os::BufferedPort<yarp::os::Bottle> inSrPort;
    yarp::os::RpcClient outTtsPort;  // Tts port
    yarp::os::RpcClient outSrecPort; // SpeechRecognition port
    yarp::os::RpcClient outCmdHeadPort;
    yarp::os::RpcClient outCmdArmPort;

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

}  // namespace teo

#endif  // __FM_DIALOGUE_MANAGER_HPP__
