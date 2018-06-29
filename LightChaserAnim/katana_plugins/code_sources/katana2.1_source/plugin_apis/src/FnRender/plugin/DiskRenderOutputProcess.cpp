// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/DiskRenderOutputProcess.h"

#include <cstdio>

namespace Foundry
{
namespace Katana
{
namespace Render
{

DiskRenderOutputProcess::DiskRenderOutputProcess()
{
}

void DiskRenderOutputProcess::setRenderAction(std::auto_ptr<RenderAction> renderAction)
{
    _renderAction = renderAction;
}

void DiskRenderOutputProcess::addPreCommand(const std::string & command)
{
    _preCommands.push_back(command);
}

void DiskRenderOutputProcess::addPostCommand(const std::string & command)
{
    _postCommands.push_back(command);
}

FnAttribute::GroupAttribute DiskRenderOutputProcess::buildRenderOutputAttribute() const
{
    FnAttribute::GroupBuilder builder;
    char attrName[1024];

    if(_renderAction.get())
        _renderAction->buildAttribute(builder);

    for(size_t i = 0; i < _preCommands.size(); ++i)
    {
        sprintf(attrName, "pre.command%zu", i);
        builder.set(attrName, FnAttribute::StringAttribute(_preCommands.at(i)));
    }

    for(size_t i = 0; i < _postCommands.size(); ++i)
    {
        sprintf(attrName, "post.command%zu", i);
        builder.set(attrName, FnAttribute::StringAttribute(_postCommands.at(i)));
    }

    return builder.build();
}

}
}
}
