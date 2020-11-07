#ifndef __FOLLOW_ME_VOCABS_HPP__
#define __FOLLOW_ME_VOCABS_HPP__

#include <yarp/os/Vocab.h>

constexpr yarp::conf::vocab32_t VOCAB_FOLLOW_ME = yarp::os::createVocab('f','o','l','l');
constexpr yarp::conf::vocab32_t VOCAB_STOP_FOLLOWING = yarp::os::createVocab('s','f','o','l');

constexpr yarp::conf::vocab32_t VOCAB_STATE_SALUTE = yarp::os::createVocab('s','a','l','u');
constexpr yarp::conf::vocab32_t VOCAB_STATE_SIGNALIZE_RIGHT = yarp::os::createVocab('s','i','g','r');
constexpr yarp::conf::vocab32_t VOCAB_STATE_SIGNALIZE_LEFT = yarp::os::createVocab('s','i','g','l');

const yarp::conf::vocab32_t VOCAB_GET_ENCODER_POSITION = yarp::os::createVocab('g','e','p','s');

#endif // __FOLLOW_ME_VOCABS_HPP__
