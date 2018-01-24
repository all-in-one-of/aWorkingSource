// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "TemplateRenderPlugin.h"

#include <FnRendererInfo/plugin/RenderMethod.h>

#include <FnRenderOutputUtils/FnRenderOutputUtils.h>

#include <FnScenegraphIterator/FnScenegraphIterator.h>

namespace CompanyName
{
namespace Katana
{

TemplateRenderPlugin::TemplateRenderPlugin(
    FnKat::FnScenegraphIterator rootIterator,
    FnKat::GroupAttribute arguments)

    : FnKat::Render::RenderBase(rootIterator, arguments)
{

}

TemplateRenderPlugin::~TemplateRenderPlugin()
{

}

int TemplateRenderPlugin::start()
{
    // TODO: Run render here.

    return 0;
}

int TemplateRenderPlugin::pause()
{
    // Default:
    return FnKat::Render::RenderBase::pause();
}

int TemplateRenderPlugin::resume()
{
    // Default:
    return FnKat::Render::RenderBase::resume();
}

int TemplateRenderPlugin::stop()
{
    // Default:
    return FnKat::Render::RenderBase::stop();
}

int TemplateRenderPlugin::startLiveEditing()
{
    // Default:
    return FnKat::Render::RenderBase::startLiveEditing();
}

int TemplateRenderPlugin::stopLiveEditing()
{
    // Default:
    return FnKat::Render::RenderBase::stopLiveEditing();
}

int TemplateRenderPlugin::processControlCommand(const std::string& command)
{
    // Default:
    return FnKat::Render::RenderBase::processControlCommand(command);
}

int TemplateRenderPlugin::queueDataUpdates(FnKat::GroupAttribute updateAttribute)
{
    // Default:
    return FnKat::Render::RenderBase::queueDataUpdates(updateAttribute);
}

int TemplateRenderPlugin::applyPendingDataUpdates()
{
    // Default:
    return FnKat::Render::RenderBase::applyPendingDataUpdates();
}

bool TemplateRenderPlugin::hasPendingDataUpdates() const
{
    // Default:
    return FnKat::Render::RenderBase::hasPendingDataUpdates();
}

// Disk Render

void TemplateRenderPlugin::configureDiskRenderOutputProcess(
    FnKat::Render::DiskRenderOutputProcess& diskRenderOutputProcess,
    const std::string& outputName,
    const std::string& outputPath,
    const std::string& renderMethodName,
    const float& frameTime) const
{
    // e.g.

    // The render action used for this render output:
    std::auto_ptr<FnKat::Render::RenderAction> renderAction;

    // Set the render action to do nothing:
    renderAction.reset(new FnKat::Render::NoOutputRenderAction());

    // Pass ownership of the renderAction to the diskRenderOutputProcess:
    diskRenderOutputProcess.setRenderAction(renderAction);
}

DEFINE_RENDER_PLUGIN(TemplateRenderPlugin)

}
}

void registerPlugins()
{
    REGISTER_PLUGIN(CompanyName::Katana::TemplateRenderPlugin, "template", 0, 1);
}
