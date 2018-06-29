// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/PostCommandsRenderAction.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{

PostCommandsRenderAction::PostCommandsRenderAction(
        const std::string& renderTargetLocation,
        const std::string& tempRenderLocation,
        const std::vector<std::string>& postCommands) :
            CopyRenderAction(renderTargetLocation, tempRenderLocation),
            _postCommands(postCommands)
{
    // We're going to override cleanup below, so add CopyRenderAction's default
    // of tempRenderLocation to the front of the list.
    _cleanupFiles.push_back(tempRenderLocation);
}

void PostCommandsRenderAction::addCleanupFile(const std::string & cleanupFile)
{
    _cleanupFiles.push_back(cleanupFile);
}

void PostCommandsRenderAction::buildAttribute(GroupBuilder& builder) const
{
    CopyRenderAction::buildAttribute(builder);
    builder.set("action", FnKat::StringAttribute("renderAndRunPostCommands"));
    builder.set("postCommands", FnKat::StringAttribute(_postCommands));
    builder.set("cleanup", FnKat::StringAttribute(_cleanupFiles));
}

}
}
}


