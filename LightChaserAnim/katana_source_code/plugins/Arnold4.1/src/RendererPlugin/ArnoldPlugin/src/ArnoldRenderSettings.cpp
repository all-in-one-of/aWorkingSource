// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************
#include <ArnoldRenderSettings.h>
#include <sstream>

int ArnoldRenderSettings::initialise()
{
    return 0;
}

std::string ArnoldRenderSettings::getCropWindowAsString() const
{
    std::string separator = ", ";
    float cropWindow[4];
    calculateCropWindow(cropWindow);

    std::stringstream shutterStream;
    shutterStream << cropWindow[0] << separator << cropWindow[1] << separator
                  << cropWindow[2] << separator << cropWindow[3];
    return shutterStream.str();
}
