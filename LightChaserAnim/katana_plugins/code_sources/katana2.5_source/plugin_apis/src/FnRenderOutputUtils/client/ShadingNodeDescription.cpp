// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRenderOutputUtils/ShadingNodeDescription.h"
#include "FnRenderOutputUtils/FnRenderOutputUtils.h"

namespace Foundry
{
namespace Katana
{
namespace RenderOutputUtils
{

ShadingNodeDescription::ShadingNodeDescription(ShadingNodeDescriptionHandle handle) : _handle(handle)
{
    if (_handle && _suite)
    {
        if (!_suite->isShadingNodeDescriptionValid(_handle))
        {
            _suite->releaseShadingNodeDescription(_handle);
            _handle = 0x0;
        }
    }
}

ShadingNodeDescription::~ShadingNodeDescription()
{
    if (_handle && _suite)
    {
        _suite->releaseShadingNodeDescription(_handle);
    }
}

ShadingNodeDescription::ShadingNodeDescription(const ShadingNodeDescription& rhs) : _handle(0x0)
{
    acceptHandle(rhs);
}

ShadingNodeDescription& ShadingNodeDescription::operator=(const ShadingNodeDescription& rhs)
{
    acceptHandle(rhs);
    return *this;
}

std::string ShadingNodeDescription::getName() const
{
    return std::string(
        _suite->getShadingNodeDescriptionName(_handle));
}

std::string ShadingNodeDescription::getType() const
{
    return std::string(
        _suite->getShadingNodeDescriptionType(_handle));
}

unsigned int ShadingNodeDescription::getNumberOfParameterNames() const
{
    return _suite->getShadingNodeDescriptionNumberOfParameterNames(_handle);
}

std::string ShadingNodeDescription::getParameterName(unsigned int index) const
{
    return std::string(
        _suite->getShadingNodeDescriptionParameterName(_handle, index));
}

FnAttribute::Attribute ShadingNodeDescription::getParameter(const std::string &name) const
{
    return FnAttribute::Attribute::CreateAndSteal(
        _suite->getShadingNodeDescriptionParameter(_handle, name.c_str()));
}

unsigned int ShadingNodeDescription::getNumberOfConnectionNames() const
{
    return _suite->getShadingNodeDescriptionNumberOfConnectionNames(_handle);
}

std::string ShadingNodeDescription::getConnectionName(unsigned int index) const
{
    return std::string(
        _suite->getShadingNodeDescriptionConnectionName(_handle, index));
}

ShadingNodeConnectionDescription ShadingNodeDescription::getConnection(const std::string &name) const
{
    return ShadingNodeConnectionDescription(
        _suite->getShadingNodeDescriptionConnection(_handle, name.c_str()));
}

void ShadingNodeDescription::acceptHandle(const ShadingNodeDescription &rhs)
{
    _suite->retainShadingNodeDescription(rhs._handle);
    if (_handle!=0x0) _suite->releaseShadingNodeDescription(_handle);
    _handle = rhs._handle;
}

}
}
}
