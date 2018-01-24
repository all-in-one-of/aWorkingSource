// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/TemporaryRenderAction.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{

TemporaryRenderAction::TemporaryRenderAction(const std::string& tempRenderLocation) :
        RenderAction(tempRenderLocation)
{
    setLoadOutputInMonitor(false);
}

void TemporaryRenderAction::buildAttribute(FnAttribute::GroupBuilder& builder) const
{
    RenderAction::buildAttribute(builder);
    builder.set("cleanup", FnAttribute::StringAttribute(_renderTargetLocation));
}

}
}
}
