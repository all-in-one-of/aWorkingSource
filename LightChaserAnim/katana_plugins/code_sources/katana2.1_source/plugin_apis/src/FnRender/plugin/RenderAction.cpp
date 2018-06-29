// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/RenderAction.h"

#include <iostream>

namespace Foundry
{
namespace Katana
{
namespace Render
{

RenderAction::RenderAction(const std::string& renderTargetLocation) :
        _renderTargetLocation(renderTargetLocation),
        _loadOutputInMonitor(true),
        _forceOverwriteTarget(true),
        _createDestinationDirectories(true)
{}

void RenderAction::setRenderTargetLocation(const std::string& renderTargetLocation)
{
    _renderTargetLocation = renderTargetLocation;
}

void RenderAction::setLoadOutputInMonitor(bool loadOutputInMonitor)
{
    _loadOutputInMonitor = loadOutputInMonitor;
}

void RenderAction::setForceOverwriteTarget(bool forceOverwriteTarget)
{
    _forceOverwriteTarget = forceOverwriteTarget;
}

void RenderAction::setCreateDestinationDirectories(bool createDestinationDirectories)
{
    _createDestinationDirectories = createDestinationDirectories;
}

void RenderAction::buildAttribute(FnAttribute::GroupBuilder& builder) const
{
    builder.set("action", FnAttribute::StringAttribute("render"));
    builder.set("renderTargetLocation", FnAttribute::StringAttribute(_renderTargetLocation));

    if(!_loadOutputInMonitor)
        builder.set("loadOutputInMonitor", FnAttribute::IntAttribute(0));

    if(!_forceOverwriteTarget)
        builder.set("forceOverwriteTarget", FnAttribute::IntAttribute(0));

    if(!_createDestinationDirectories)
        builder.set("createDestinationDirectories", FnAttribute::IntAttribute(0));
}

}
}
}
