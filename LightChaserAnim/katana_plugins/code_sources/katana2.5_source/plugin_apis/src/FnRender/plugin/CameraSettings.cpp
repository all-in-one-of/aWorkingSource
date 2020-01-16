// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/CameraSettings.h"

#include <iostream>

namespace Foundry
{
namespace Katana
{
namespace Render
{

int CameraSettings::initialise(int displayWindow[4], int overscan[4], float shutterClose)
{
    if(!_iterator.isValid() || _iterator.getType() != "camera")
    {
        _iterator = _iterator.getByPath(_cameraName);
    }

    if(!_iterator.isValid())
        return 1;

    FnAttribute::StringAttribute projectionAttr = _iterator.getAttribute("geometry.projection");
    if(projectionAttr.isValid())
        _projection = projectionAttr.getValue();

    FnAttribute::DoubleAttribute fovAttr = _iterator.getAttribute("geometry.fov");
    if(fovAttr.isValid())
        _fov = float(fovAttr.getValue());

    FnAttribute::DoubleAttribute orthographicWidthAttr = _iterator.getAttribute("geometry.orthographicWidth");
    if(orthographicWidthAttr.isValid())
        _orthographicWidth = static_cast<float>(
            orthographicWidthAttr.getValue());

    double near = 0.0;
    double far = 0.0;
    double left = 0.0;
    double bottom = 0.0;
    double right = 0.0;
    double top = 0.0;

    FnAttribute::DoubleAttribute nearAttr = _iterator.getAttribute("geometry.near");
    if(nearAttr.isValid())
        near = nearAttr.getValue();

    FnAttribute::DoubleAttribute farAttr = _iterator.getAttribute("geometry.far");
    if(farAttr.isValid())
        far = farAttr.getValue();

    FnAttribute::DoubleAttribute leftAttr = _iterator.getAttribute("geometry.left");
    if(leftAttr.isValid())
        left = leftAttr.getValue();

    FnAttribute::DoubleAttribute bottomAttr = _iterator.getAttribute("geometry.bottom");
    if(bottomAttr.isValid())
        bottom = bottomAttr.getValue();

    FnAttribute::DoubleAttribute rightAttr = _iterator.getAttribute("geometry.right");
    if(rightAttr.isValid())
        right = rightAttr.getValue();

    FnAttribute::DoubleAttribute topAttr = _iterator.getAttribute("geometry.top");
    if(topAttr.isValid())
        top = topAttr.getValue();

    _clipping[0] = float(near);
    _clipping[1] = float(far);

    _screenWindow[0] = float(left);
    _screenWindow[1] = float(bottom);
    _screenWindow[2] = float(right);
    _screenWindow[3] = float(top);

    if( _projection == "orthographic" )
    {
        // Compute the orthographic screen window based on the orthographic width of the camera.
        float screenLeft = _screenWindow[0];
        float screenRight = _screenWindow[2];
        float screenBottom = _screenWindow[1];
        float screenTop = _screenWindow[3];
        float screenWidth = screenRight - screenLeft;
        float screenHeight = screenTop - screenBottom;

        float orthographicHeight = _orthographicWidth  * screenHeight / screenWidth;

        screenLeft = screenLeft * _orthographicWidth / screenWidth;
        screenRight = screenRight * _orthographicWidth / screenWidth;
        screenTop = screenTop * orthographicHeight / screenHeight;
        screenBottom = screenBottom * orthographicHeight / screenHeight;

        _screenWindow[0] = screenLeft;
        _screenWindow[1] = screenBottom;
        _screenWindow[2] = screenRight;
        _screenWindow[3] = screenTop;
    }

    // Adjust screen window for overscan
    float displayWindowW2 = (float) (displayWindow[2] - displayWindow[0]) / 2.0f;
    float displayWindowH2 = (float) (displayWindow[3] - displayWindow[1]) / 2.0f;

    // Reconstruct overscan values
    float overscanMultiplier[4];
    overscanMultiplier[0] = (displayWindowW2 + (float)overscan[0]) / displayWindowW2;
    overscanMultiplier[1] = (displayWindowH2 + (float)overscan[1]) / displayWindowH2;
    overscanMultiplier[2] = (displayWindowW2 + (float)overscan[2]) / displayWindowW2;
    overscanMultiplier[3] = (displayWindowH2 + (float)overscan[3]) / displayWindowH2;

    float screenWindowCx = (_screenWindow[2] + _screenWindow[0]) / 2.0f;
    float screenWindowCy = (_screenWindow[3] + _screenWindow[1]) / 2.0f;

    _screenWindow[0] = screenWindowCx + (_screenWindow[0] - screenWindowCx) * overscanMultiplier[0];
    _screenWindow[1] = screenWindowCy + (_screenWindow[1] - screenWindowCy) * overscanMultiplier[1];
    _screenWindow[2] = screenWindowCx + (_screenWindow[2] - screenWindowCx) * overscanMultiplier[2];
    _screenWindow[3] = screenWindowCy + (_screenWindow[3] - screenWindowCy) * overscanMultiplier[3];

    FnKat::RenderOutputUtils::fillXFormListForLocation(_transforms, _iterator, shutterClose);

    return 0;
}

void CameraSettings::getClipping(float clipping[2]) const
{
    clipping[0] = _clipping[0];
    clipping[1] = _clipping[1];
}

void CameraSettings::getScreenWindow(float screenWindow[4]) const
{
    screenWindow[0] = _screenWindow[0];
    screenWindow[1] = _screenWindow[1];
    screenWindow[2] = _screenWindow[2];
    screenWindow[3] = _screenWindow[3];
}

}
}
}
