// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include "PRManRenderSettings.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>

int PRManRenderSettings::initialise()
{
    int frameWidth  = _dataWindow[2] - _dataWindow[0];
    int frameHeight = _dataWindow[3] - _dataWindow[1];

    _xRes = static_cast<int>( ceil( _sampleRate[0] * frameWidth ) );
    _yRes = static_cast<int>( ceil( _sampleRate[1] * frameHeight ) );

    return 0;
}

std::string PRManRenderSettings::getDataWindowWidthAsString() const
{
    std::stringstream dataWidthStream;
    dataWidthStream << (_dataWindow[2] - _dataWindow[0]);
    return dataWidthStream.str();
}

std::string PRManRenderSettings::getDataWindowHeightAsString() const
{
    std::stringstream dataHeightStream;
    dataHeightStream << (_dataWindow[3] - _dataWindow[1]);
    return dataHeightStream.str();
}

std::string PRManRenderSettings::getShutterAsString() const
{
    std::stringstream shutterStream;
    shutterStream << _shutterOpen << " " << _shutterClose;
    return shutterStream.str();
}

std::string PRManRenderSettings::getCropWindowAsString() const
{
    std::string separator = " ";
    float cropWindow[4];
    calculateCropWindow(cropWindow);

    std::stringstream shutterStream;
    shutterStream << cropWindow[0] << separator << cropWindow[1] << separator
                  << cropWindow[2] << separator << cropWindow[3];
    return shutterStream.str();
}
