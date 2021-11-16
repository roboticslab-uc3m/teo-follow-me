// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __FOLLOW_ME_DIALOGUE_MANAGER_HPP__
#define __FOLLOW_ME_DIALOGUE_MANAGER_HPP__

#include <unordered_map>

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RpcClient.h>

#include <SpeechIDL.h>

#include "FollowMeHeadCommandsIDL.h"

namespace roboticslab
{

/**
 * @ingroup follow-me_programs
 * @brief Dialogue Manager.
 */
class FollowMeDialogueManager : public yarp::os::RFModule
{
public:
    ~FollowMeDialogueManager()
    { close(); }

    bool configure(yarp::os::ResourceFinder & rf) override;
    bool close() override;
    bool interruptModule() override;
    double getPeriod() override;
    bool updateModule() override;

private:
    void ttsSay(const std::string & sayString);
    std::string asrListen();
    std::string asrListenWithPeriodicWave();

    FollowMeHeadCommandsIDL headCommander;
    SpeechIDL speech;

    yarp::os::BufferedPort<yarp::os::Bottle> inAsrPort;
    yarp::os::RpcClient ttsClient;
    yarp::os::RpcClient asrConfigClient;
    yarp::os::RpcClient headExecutionClient;
    yarp::os::RpcClient armExecutionClient;

    // micro (on/off) to give speaking orders to TEO
    bool isFollowing {false};
    bool microOn;
    int machineState {3};
    char sentence {'a'};
    std::string _inStrState1;

    std::unordered_map<std::string, std::string> sentences;
    std::unordered_map<std::string, std::string> voiceCommands;
};

} // namespace roboticslab

#endif // __FOLLOW_ME_DIALOGUE_MANAGER_HPP__
