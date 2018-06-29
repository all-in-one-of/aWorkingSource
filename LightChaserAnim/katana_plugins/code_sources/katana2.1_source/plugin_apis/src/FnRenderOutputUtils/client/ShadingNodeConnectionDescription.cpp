// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRenderOutputUtils/ShadingNodeConnectionDescription.h"
#include "FnRenderOutputUtils/FnRenderOutputUtils.h"

namespace Foundry
{
namespace Katana
{
namespace RenderOutputUtils
{

ShadingNodeConnectionDescription::ShadingNodeConnectionDescription(
    ShadingNodeConnectionDescriptionHandle handle) : _handle(handle)
{
    if (_handle && _suite)
    {
        if (!_suite->isShadingNodeConnectionDescriptionValid(_handle))
        {
            _suite->releaseShadingNodeConnectionDescription(_handle);
            _handle = 0x0;
        }
    }
}

ShadingNodeConnectionDescription::~ShadingNodeConnectionDescription()
{
    if (_handle && _suite)
    {
        _suite->releaseShadingNodeConnectionDescription(_handle);
    }
}

ShadingNodeConnectionDescription::ShadingNodeConnectionDescription(
    const ShadingNodeConnectionDescription& rhs) : _handle(0x0)
{
    acceptHandle(rhs);
}

ShadingNodeConnectionDescription& ShadingNodeConnectionDescription::operator=(
    const ShadingNodeConnectionDescription& rhs)
{
    acceptHandle(rhs);
    return *this;
}

std::string ShadingNodeConnectionDescription::getName() const
{
    return std::string(
        _suite->getShadingNodeConnectionDescriptionName(_handle));
}

std::string ShadingNodeConnectionDescription::getConnectedNodeName() const
{
    return std::string(
        _suite->getShadingNodeConnectionDescriptionConnectedNodeName(_handle));
}

std::string ShadingNodeConnectionDescription::getConnectedPortName() const
{
    return std::string(
        _suite->getShadingNodeConnectionDescriptionConnectedPortName(_handle));
}

void ShadingNodeConnectionDescription::acceptHandle(
    const ShadingNodeConnectionDescription &rhs)
{
    _suite->retainShadingNodeConnectionDescription(rhs._handle);
    if (_handle!=0x0) _suite->releaseShadingNodeConnectionDescription(_handle);
    _handle = rhs._handle;
}

}
}
}
