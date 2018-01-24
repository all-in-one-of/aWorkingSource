// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRenderOutputUtils/ShadingNodeDescriptionMap.h"
#include "FnRenderOutputUtils/FnRenderOutputUtils.h"

namespace Foundry
{
namespace Katana
{
namespace RenderOutputUtils
{

ShadingNodeDescriptionMap::ShadingNodeDescriptionMap(FnAttribute::GroupAttribute materialAttr) : _handle(0x0)
{
    if (_suite)
    {
        _handle = _suite->getShadingNetworkNodes(materialAttr.getHandle());
    }
}

ShadingNodeDescriptionMap::ShadingNodeDescriptionMap(FnScenegraphIterator sgIterator) : _handle(0x0)
{
    if (_suite)
    {
        _handle = _suite->getShadingNetworkNodesFromSgIterator(sgIterator.getHandle());
    }
}

ShadingNodeDescriptionMap::~ShadingNodeDescriptionMap()
{
    if (_handle && _suite)
    {
        _suite->destroyShadingNodeDescriptionMap(_handle);
    }
}

ShadingNodeDescription ShadingNodeDescriptionMap::getShadingNodeDescriptionByName(const std::string &name) const
{
    return ShadingNodeDescription(_suite->getShadingNodeDescriptionByName(_handle, name.c_str()));
}

}
}
}
