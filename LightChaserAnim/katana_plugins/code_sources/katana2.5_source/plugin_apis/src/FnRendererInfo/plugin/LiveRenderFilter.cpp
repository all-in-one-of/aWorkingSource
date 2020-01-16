// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnRendererInfo/plugin/LiveRenderFilter.h>

namespace Foundry
{
namespace Katana
{
namespace RendererInfo
{

LiveRenderFilter::LiveRenderFilter()
    : _sceneGraphTraversalMode(kSceneGraphTraversalMode_LocationOrigin)
{
}

LiveRenderFilter::LiveRenderFilter(const std::string& name,
                                   const std::string& updateType,
                                   const std::string& sgLocationType,
                                   const std::string& sgLocationOrigin,
                                   SceneGraphTraversalMode sgTraversalMode)
    : _name(name),
      _updateType(updateType),
      _sceneGraphLocationType(sgLocationType),
      _sceneGraphLocationOrigin(sgLocationOrigin),
      _sceneGraphTraversalMode(sgTraversalMode)
{
}

void LiveRenderFilter::addAttribute(const std::string& attributeName)
{
    _attributes.push_back(attributeName);
}

void LiveRenderFilter::buildAttribute(FnAttribute::GroupBuilder& builder, const std::string& prefix) const
{
    builder.set(prefix + "name", FnAttribute::StringAttribute(_name));
    builder.set(prefix + "type", FnAttribute::StringAttribute(_sceneGraphLocationType));
    builder.set(prefix + "location", FnAttribute::StringAttribute(_sceneGraphLocationOrigin));
    builder.set(prefix + "traversalMode", FnAttribute::IntAttribute(_sceneGraphTraversalMode));
    builder.set(prefix + "alias", FnAttribute::StringAttribute(_updateType));

    std::vector<std::string>::const_iterator sgAttributesIt;
    for(sgAttributesIt = _attributes.begin(); sgAttributesIt != _attributes.end(); ++sgAttributesIt)
    {
        const std::string attributeName = (*sgAttributesIt);
        const std::string attributeIdentifier = prefix + std::string("attributes.attribute_") +
                                                attributeName + std::string(".name");

        builder.set(attributeIdentifier, FnAttribute::StringAttribute(attributeName));
    }
}

}
}
}
