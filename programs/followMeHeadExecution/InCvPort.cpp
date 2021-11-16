// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "InCvPort.hpp"

#include <yarp/os/LogStream.h>

using namespace roboticslab;

constexpr auto DETECTION_DEADBAND = 0.3; // [mm]
constexpr auto RELATIVE_INCREMENT = 2.0; // [deg]

void InCvPort::onRead(yarp::os::Bottle & b)
{
    if (!follow)
    {
        iPositionControl->positionMove(std::vector<double>(2, 0.0).data());
        return;
    }

    if (b.size() != 3)
    {
        yWarning() << "InCvPort protocol error, expected 3 elements, got" << b.size();
        return;
    }

    auto x = b.get(0).asFloat64();
    auto y = b.get(1).asFloat64();
    auto z = b.get(2).asFloat64(); // depth, unused

    yDebug() << "InCvPort got:" << x << y << z;

    // X -> Head joint 0
    if (x > DETECTION_DEADBAND) iPositionControl->relativeMove(0, RELATIVE_INCREMENT);
    if (x < -DETECTION_DEADBAND) iPositionControl->relativeMove(0, -RELATIVE_INCREMENT);

    // Y -> Head joint 1
    if (y > DETECTION_DEADBAND) iPositionControl->relativeMove(1, RELATIVE_INCREMENT);
    if (y < -DETECTION_DEADBAND) iPositionControl->relativeMove(1, -RELATIVE_INCREMENT);
}

void InCvPort::setIPositionControl(yarp::dev::IPositionControl * iPositionControl)
{
    this->iPositionControl = iPositionControl;
}

void InCvPort::setFollow(bool value)
{
    follow = value;
}
