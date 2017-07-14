// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRenderOutputUtils/CameraInfo.h"
#include "FnRenderOutputUtils/FnRenderOutputUtils.h"

namespace Foundry
{
namespace Katana
{
namespace RenderOutputUtils
{

CameraInfo::CameraInfo(CameraInfoHandle handle): _handle(handle)
{
}

CameraInfo::~CameraInfo()
{
    if(_handle && _suite)
    {
        _suite->releaseCameraInfo(_handle);
    }
}

CameraInfo::CameraInfo(const CameraInfo& rhs) :
    _handle(0x0)
{
    acceptHandle(rhs);
}

CameraInfo & CameraInfo::operator=(const CameraInfo& rhs)
{
    acceptHandle(rhs);
    return *this;
}


void CameraInfo::acceptHandle(const CameraInfo &rhs)
{
    _suite->retainCameraInfo(rhs._handle);
    if (_handle!=0x0) _suite->releaseCameraInfo(_handle);
    _handle = rhs._handle;
}


float CameraInfo::getFov() const
{
    return _suite->getCameraInfoFov(_handle);
}

float CameraInfo::getNear() const
{
    return _suite->getCameraInfoNear(_handle);
}

float CameraInfo::getFar() const
{
    return _suite->getCameraInfoFar(_handle);
}

float CameraInfo::getLeft() const
{
    return _suite->getCameraInfoLeft(_handle);
}

float CameraInfo::getRight() const
{
    return _suite->getCameraInfoRight(_handle);
}

float CameraInfo::getTop() const
{
    return _suite->getCameraInfoTop(_handle);
}

float CameraInfo::getBottom() const
{
    return _suite->getCameraInfoBottom(_handle);
}

int CameraInfo::getOrtho() const
{
    return _suite->getCameraInfoOrtho(_handle);
}

float CameraInfo::getOrthoWidth() const
{
    return _suite->getCameraInfoOrthoWidth(_handle);
}

double* CameraInfo::getXForm() const
{
    return _suite->getCameraInfoXForm(_handle);
}

}
}
}
