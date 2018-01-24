// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/RenderBase.h"
#include "FnAttribute/FnGroupBuilder.h"

#include "FnAsset/FnDefaultAssetPlugin.h"
#include "FnAsset/FnDefaultFileSequencePlugin.h"

#include <FnPluginManager/FnPluginManager.h>
#include <FnLogging/FnLogging.h>

#include <memory>

class FnRenderPluginStruct
{
public:
    FnRenderPluginStruct(Foundry::Katana::Render::RenderBase* render)
        : _render(render) {}
    ~FnRenderPluginStruct() {};
    Foundry::Katana::Render::RenderBase& getRenderPlugin() { return *_render; }

private:
    std::auto_ptr<Foundry::Katana::Render::RenderBase> _render;
};

////////////////////////////////////////////////////
// C callbacks implementations for the plugin suite
////////////////////////////////////////////////////
void destroy(FnRenderPluginHandle handle)
{
    delete handle;
}

int start(FnRenderPluginHandle handle)
{
    return handle->getRenderPlugin().start();
}

int pause(FnRenderPluginHandle handle)
{
    return handle->getRenderPlugin().pause();
}

int resume(FnRenderPluginHandle handle)
{
    return handle->getRenderPlugin().resume();
}

int stop(FnRenderPluginHandle handle)
{
    return handle->getRenderPlugin().stop();
}

int startLiveEditing(FnRenderPluginHandle handle)
{
    return handle->getRenderPlugin().startLiveEditing();
}

int stopLiveEditing(FnRenderPluginHandle handle)
{
    return handle->getRenderPlugin().stopLiveEditing();
}

int processControlCommand(FnRenderPluginHandle handle, const char* command)
{
    return handle->getRenderPlugin().processControlCommand(command);
}

int queueDataUpdates(FnRenderPluginHandle handle,
                     FnAttributeHandle commandAttribute)
{
    return handle->getRenderPlugin().queueDataUpdates(
        FnAttribute::Attribute::CreateAndRetain(commandAttribute));
}

bool hasPendingDataUpdates(FnRenderPluginHandle handle)
{
    return handle->getRenderPlugin().hasPendingDataUpdates();
}

int applyPendingDataUpdates(FnRenderPluginHandle handle)
{
    return handle->getRenderPlugin().applyPendingDataUpdates();
}

FnAttributeHandle configureDiskRenderOutputProcess(FnRenderPluginHandle handle,
                                                   const char* outputName,
                                                   const char* outputPath,
                                                   const char* renderMethodName,
                                                   const float* frameTime)
{
    return handle->getRenderPlugin()._configureDiskRenderOutputProcess(outputName,
        outputPath,
        renderMethodName,
        *frameTime).getRetainedHandle();
}


///////////////////////////
// FnRender implementation
///////////////////////////
namespace Foundry
{
namespace Katana
{
namespace Render
{

std::string RenderBase::getRenderMethodName() const
{
    return findArgument("renderMethodName");
}

std::string RenderBase::findArgument(const std::string& argumentName, const std::string & defaultValue) const
{
    return FnAttribute::StringAttribute(
        _arguments.getChildByName(argumentName)).getValue(defaultValue, false);
}

bool RenderBase::applyRenderThreadsOverride(int& numberOfRenderThreads) const
{
    std::string threadArg = findArgument("threads");
    if(threadArg == "")
        return false;

    numberOfRenderThreads = atoi(threadArg.c_str());
    return true;
}

float RenderBase::getRenderTime() const
{
    std::string renderTime = findArgument("renderTime");
    if(renderTime == "")
        return 0.0f;

    return float(atof(renderTime.c_str()));
}

bool RenderBase::useRenderPassID() const
{
    std::string useID = findArgument("useID");
    if(useID == "True")
        return true;

    return false;
}

bool RenderBase::isExpandProceduralActive(bool defaultValue) const
{
    std::string expandProcedural = findArgument(
        "expandProcedural", (defaultValue ? "True" : "False"));
    if(expandProcedural == "True")
        return true;
    return false;
}

std::string RenderBase::getEnvironmentVariable(const std::string& customModuleName) const
{
    char const* env = getenv(customModuleName.c_str());
    if(env == NULL)
        return "";
    return std::string(env);
}

FnAttribute::Attribute RenderBase::_configureDiskRenderOutputProcess(const std::string& outputName,
                                                        const std::string& outputPath,
                                                        const std::string& renderMethodName,
                                                        const float& frameTime) const
{
    DiskRenderOutputProcess diskRenderOutputProcess;
    configureDiskRenderOutputProcess(diskRenderOutputProcess,
                                     outputName,
                                     outputPath,
                                     renderMethodName,
                                     frameTime);
    return diskRenderOutputProcess.buildRenderOutputAttribute();
}

FnRenderPluginHandle RenderBase::newRenderHandle(RenderBase* render)
{
    if(!render)
        return 0x0;

    FnRenderPluginHandle h = new FnRenderPluginStruct(render);
    return h;
}

FnPlugStatus RenderBase::setHost(FnPluginHost* host)
{
    _host = host;
    FnPluginManager::PluginManager::setHost(host);
    RenderOutputUtils::setHost(host);
    FnScenegraphIterator::setHost(host);
    FnAttribute::GroupBuilder::setHost(host);
    FnAsset::DefaultAssetPlugin::setHost(host);
    FnAsset::DefaultFileSequencePlugin::setHost(host);
    FnLogging::setHost(host);
    return FnAttribute::Attribute::setHost(host);
}

FnPluginHost* RenderBase::getHost() { return _host; }

FnRenderPluginSuite_v1 RenderBase::createSuite(
        FnRenderPluginHandle (*create)(FnSgIteratorHandle, FnAttributeHandle))
{
    FnRenderPluginSuite_v1 suite = {
        create,
        ::destroy,
        ::start,
        ::pause,
        ::resume,
        ::stop,
        ::startLiveEditing,
        ::stopLiveEditing,
        ::processControlCommand,
        ::queueDataUpdates,
        ::hasPendingDataUpdates,
        ::applyPendingDataUpdates,
        ::configureDiskRenderOutputProcess
    };

    return suite;
}

unsigned int RenderBase::_apiVersion = 1;
const char* RenderBase::_apiName     = "RenderPlugin";
FnPluginHost *RenderBase::_host      = 0x0;

}
}
}
