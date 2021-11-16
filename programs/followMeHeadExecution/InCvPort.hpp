// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __IN_CV_PORT_HPP__
#define __IN_CV_PORT_HPP__

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>

#include <yarp/dev/IEncoders.h>
#include <yarp/dev/IPositionControl.h>

namespace roboticslab
{

/**
 * @ingroup followMeExecutionCore
 * @brief Input port of computer vision data.
 */
class InCvPort : public yarp::os::BufferedPort<yarp::os::Bottle>
{
public:
    void onRead(yarp::os::Bottle& b) override;
    void setIPositionControl(yarp::dev::IPositionControl * iPositionControl);
    void setFollow(bool value);

private:
    bool follow {false};

    yarp::dev::IEncoders * iEncoder;
    yarp::dev::IPositionControl * iPositionControl;
};

} // namespace roboticslab

#endif // __IN_CV_PORT_HPP__
