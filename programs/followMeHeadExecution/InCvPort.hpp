// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __IN_CV_PORT_HPP__
#define __IN_CV_PORT_HPP__

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>

#include <yarp/dev/ControlBoardInterfaces.h>

namespace roboticslab
{

/**
 * @ingroup followMeExecutionCore
 *
 * @brief Input port of computer vision data.
 *
 */
class InCvPort : public yarp::os::BufferedPort<yarp::os::Bottle>
{
public:
    InCvPort(): follow(false) {}

    void setIPositionControl(yarp::dev::IPositionControl *iPositionControl) {
        this->iPositionControl = iPositionControl;
    }

    void setFollow(bool value);


private:
    bool follow;

    /** Callback on incoming Bottle. **/
    void onRead(yarp::os::Bottle& b) override;

    yarp::dev::IEncoders * iEncoder;
    yarp::dev::IPositionControl *iPositionControl;
};

} // namespace roboticslab

#endif  // __IN_CV_PORT_HPP__
