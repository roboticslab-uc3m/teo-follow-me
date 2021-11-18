// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __FOLLOW_ME_DIALOGUE_MANAGER_HPP__
#define __FOLLOW_ME_DIALOGUE_MANAGER_HPP__

#include <string>
#include <tuple>
#include <unordered_map>

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Thread.h>

#include <TextToSpeechIDL.h>
#include <SpeechRecognitionIDL.h>

#include "FollowMeHeadCommandsIDL.h"
#include "FollowMeArmCommandsIDL.h"

namespace roboticslab
{

/**
 * @ingroup follow-me_programs
 * @brief Dialogue Manager.
 */
class FollowMeDialogueManager : public yarp::os::RFModule,
                                public yarp::os::Thread
{
public:
    ~FollowMeDialogueManager()
    { close(); }

    bool configure(yarp::os::ResourceFinder & rf) override;
    bool close() override;
    bool interruptModule() override;
    double getPeriod() override;
    bool updateModule() override;

    void run() override;

private:
    enum class state { PRESENTATION, ASK_NAME, DIALOGUE, LISTEN, FOLLOW, STOP_FOLLOWING };

    std::tuple<bool, std::string, std::string> checkOutputConnections();
    void ttsSayAndWait(const std::string & sayString);
    std::string asrListenAndWait();
    std::string asrListenAndLinger();
    static std::string getStateDescription(state s);

    FollowMeArmCommandsIDL armCommander;
    FollowMeHeadCommandsIDL headCommander;
    TextToSpeechIDL tts;
    SpeechRecognitionIDL asr;

    yarp::os::BufferedPort<yarp::os::Bottle> inAsrPort;
    yarp::os::RpcClient ttsClient;
    yarp::os::RpcClient asrConfigClient;
    yarp::os::RpcClient headExecutionClient;
    yarp::os::RpcClient armExecutionClient;

    bool usingMic;
    state machineState {state::LISTEN};

    std::unordered_map<std::string, std::string> sentences;
    std::unordered_map<std::string, std::string> voiceCommands;
};

} // namespace roboticslab

#endif // __FOLLOW_ME_DIALOGUE_MANAGER_HPP__
