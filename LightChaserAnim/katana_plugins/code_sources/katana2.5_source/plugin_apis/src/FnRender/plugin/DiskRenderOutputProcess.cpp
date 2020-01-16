// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/DiskRenderOutputProcess.h"

#include <sstream>

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

    if(_renderAction.get())
        _renderAction->buildAttribute(builder);

    for(size_t i = 0; i < _preCommands.size(); ++i)
    {
        std::ostringstream os;
        os << "pre.command" << i;
        builder.set(os.str(), FnAttribute::StringAttribute(_preCommands.at(i)));
    }

    for(size_t i = 0; i < _postCommands.size(); ++i)
    {
        std::ostringstream os;
        os << "post.command" << i;
        builder.set(os.str(), FnAttribute::StringAttribute(_postCommands.at(i)));
    }

    return builder.build();
}

}
}
}
