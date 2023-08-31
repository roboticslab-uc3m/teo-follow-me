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

#include <SpeechSynthesis.h>
#include <SpeechRecognition.h>

#include "FollowMeHeadCommands.h"
#include "FollowMeArmCommands.h"

namespace roboticslab
{

/**
 * @ingroup followMeDialogueManager
 * @brief Dialogue Manager.
 */
class FollowMeDialogueManager : public yarp::os::RFModule,
                                public yarp::os::Thread
{
public:
    enum class state { PRESENTATION, ASK_NAME, DIALOGUE, LISTEN, FOLLOW, STOP_FOLLOWING };
    enum class sentence { PRESENTATION_1, PRESENTATION_2, PRESENTATION_3, ASK_NAME, ANSWER_1, ANSWER_2, ANSWER_3,
                          NOT_UNDERSTAND, FOLLOW, STOP_FOLLOWING, ON_THE_RIGHT, ON_THE_LEFT, ON_THE_CENTER };
    enum class command { HI_TEO, FOLLOW_ME, MY_NAME_IS, STOP_FOLLOWING };

    ~FollowMeDialogueManager()
    { close(); }

    bool configure(yarp::os::ResourceFinder & rf) override;
    bool close() override;
    bool interruptModule() override;
    double getPeriod() override;
    bool updateModule() override;

    bool threadInit() override;
    void run() override;

private:
    std::tuple<bool, std::string, std::string> checkOutputConnections();
    void ttsSayAndWait(sentence snt);
    std::string asrListenAndWait();
    std::string asrListenAndLinger();

    FollowMeArmCommands armCommander;
    FollowMeHeadCommands headCommander;
    SpeechSynthesis tts;
    SpeechRecognition asr;

    yarp::os::BufferedPort<yarp::os::Bottle> inAsrPort;
    yarp::os::RpcClient ttsClient;
    yarp::os::RpcClient asrConfigClient;
    yarp::os::RpcClient headExecutionClient;
    yarp::os::RpcClient armExecutionClient;

    std::string voice;
    std::string langCode;
    bool usingMic;
    state machineState {state::LISTEN};

    std::unordered_map<sentence, std::string> sentences;
    std::unordered_map<command, std::string> voiceCommands;
};

} // namespace roboticslab

#endif // __FOLLOW_ME_DIALOGUE_MANAGER_HPP__
