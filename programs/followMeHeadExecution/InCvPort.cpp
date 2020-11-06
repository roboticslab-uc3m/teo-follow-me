// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "InCvPort.hpp"

namespace roboticslab
{

/************************************************************************/

void InCvPort::onRead(yarp::os::Bottle& b) {
    if ( ! follow ) {
        iPositionControl->positionMove(0, 0.0);
        iPositionControl->positionMove(1, 0.0);
        return;
    }
    if (b.size() < 3) return;

    double x = b.get(0).asDouble();
    double y = b.get(1).asDouble();
    double z = b.get(2).asDouble();
    printf("%f %f %f\n",x,y,z);
    if( x > 0.30 ) iPositionControl->relativeMove(0, 2);
    if( x < -0.30 ) iPositionControl->relativeMove(0, -2);
    //
    if( y > 0.30 ) iPositionControl->relativeMove(1, 2);
    if( y < -0.30 ) iPositionControl->relativeMove(1, -2);

}

/************************************************************************/

void InCvPort::setFollow(bool value)
{
    follow = value;
}

/************************************************************************/

} // namespace roboticslab
