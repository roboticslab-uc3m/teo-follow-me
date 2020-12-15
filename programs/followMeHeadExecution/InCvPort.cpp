// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "InCvPort.hpp"

namespace roboticslab
{

/************************************************************************/

void InCvPort::onRead(yarp::os::Bottle& b)
{
    if ( ! follow )
    {
        iPositionControl->positionMove(0, 0.0);
        iPositionControl->positionMove(1, 0.0);
        return;
    }

    //printf("InCvPort got %zu elems [%s]\n", b.size(), b.toString().c_str());

    if(b.size() < 1)
    {
        printf("InCvPort review yor protocol! (b.size() is %zu < 1)\n", b.size());
        return;
    }

    if(!b.get(0).isDict())
    {
        printf("InCvPort review yor protocol! (!b.get(0).isDict())\n");
        return;
    }

    yarp::os::Property* detectedObject = b.get(0).asDict();
    double x = detectedObject->find("mmX").asDouble();
    double y = detectedObject->find("mmY").asDouble();
    double z = detectedObject->find("mmZ").asDouble();
    printf("InCvPort detectedObjects[0]: %f %f %f\n", x, y, z);

    // X -> Head joint 0
    if( x > 0.30 ) iPositionControl->relativeMove(0, 2);
    if( x < -0.30 ) iPositionControl->relativeMove(0, -2);

    // Y -> Head joint 1
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
