// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/CopyRenderAction.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{

CopyRenderAction::CopyRenderAction(const std::string& renderTargetLocation,
                                   const std::string& tempRenderLocation) :
        RenderAction(renderTargetLocation),
        _tempRenderLocation(tempRenderLocation)
{}

void CopyRenderAction::buildAttribute(GroupBuilder& builder) const
{
    RenderAction::buildAttribute(builder);
    builder.set("action", FnAttribute::StringAttribute("renderAndCopy"));
    builder.set("tempRenderLocation", FnAttribute::StringAttribute(_tempRenderLocation));
    builder.set("cleanup", FnAttribute::StringAttribute(_tempRenderLocation));
}

}
}
}
