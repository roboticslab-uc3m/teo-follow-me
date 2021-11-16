#ifndef __FOLLOW_ME_VOCABS_HPP__
#define __FOLLOW_ME_VOCABS_HPP__

#include <yarp/conf/version.h>

#include <yarp/os/Vocab.h>

#if YARP_VERSION_MINOR >= 5
constexpr auto VOCAB_FOLLOW_ME = yarp::os::createVocab32('f','o','l','l');
constexpr auto VOCAB_STOP_FOLLOWING = yarp::os::createVocab32('s','f','o','l');
constexpr auto VOCAB_STATE_SALUTE = yarp::os::createVocab32('s','a','l','u');
constexpr auto VOCAB_STATE_SIGNALIZE_RIGHT = yarp::os::createVocab32('s','i','g','r');
constexpr auto VOCAB_STATE_SIGNALIZE_LEFT = yarp::os::createVocab32('s','i','g','l');
constexpr auto VOCAB_GET_ENCODER_POSITION = yarp::os::createVocab32('g','e','p','s');
#else
constexpr auto VOCAB_FOLLOW_ME = yarp::os::createVocab('f','o','l','l');
constexpr auto VOCAB_STOP_FOLLOWING = yarp::os::createVocab('s','f','o','l');
constexpr auto VOCAB_STATE_SALUTE = yarp::os::createVocab('s','a','l','u');
constexpr auto VOCAB_STATE_SIGNALIZE_RIGHT = yarp::os::createVocab('s','i','g','r');
constexpr auto VOCAB_STATE_SIGNALIZE_LEFT = yarp::os::createVocab('s','i','g','l');
constexpr auto VOCAB_GET_ENCODER_POSITION = yarp::os::createVocab('g','e','p','s');
#endif

#endif // __FOLLOW_ME_VOCABS_HPP__
