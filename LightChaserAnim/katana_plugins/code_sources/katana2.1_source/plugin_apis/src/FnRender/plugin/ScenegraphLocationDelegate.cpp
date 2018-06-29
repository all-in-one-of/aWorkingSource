// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnPluginManager/FnPluginManager.h>

#include "FnRender/plugin/ScenegraphLocationDelegate.h"
#include "FnRender/suite/FnScenegraphLocationDelegateSuite.h"
#include "FnAttribute/FnAttribute.h"
#include "FnAttribute/FnGroupBuilder.h"
#include "FnRenderOutputUtils/FnRenderOutputUtils.h"
#include <FnAsset/FnDefaultAssetPlugin.h>
#include <FnAsset/FnDefaultFileSequencePlugin.h>

namespace FnKat = Foundry::Katana;
using namespace FnKat;

class FnScenegraphLocationDelegateStruct
{
public:

    FnScenegraphLocationDelegateStruct(Foundry::Katana::Render::ScenegraphLocationDelegate * location) :
        _location(location)
    {
    }

    virtual ~FnScenegraphLocationDelegateStruct()
    {
    };

    Foundry::Katana::Render::ScenegraphLocationDelegate & getLocation()
    {
        return *_location;
    }

private:
    std::auto_ptr<Foundry::Katana::Render::ScenegraphLocationDelegate> _location;
};

void destroy(FnLocationHandle handle)
{
    delete handle;
}

void* process(FnLocationHandle handle, FnSgIteratorHandle it, void* optionalInput)
{
    return handle->getLocation().process(FnScenegraphIterator(it), optionalInput);
}

FnAttributeHandle getSupportedRenderer(FnLocationHandle handle)
{
    return FnAttribute::StringAttribute(
        handle->getLocation().getSupportedRenderer()).getRetainedHandle();
}

FnAttributeHandle fillSupportedLocationList(FnLocationHandle handle)
{
    std::vector<std::string> locs;
    handle->getLocation().fillSupportedLocationList(locs);
    return FnAttribute::StringAttribute(locs, 1).getRetainedHandle();
}

namespace Foundry
{
namespace Katana
{
namespace Render
{

unsigned int ScenegraphLocationDelegate::_apiVersion = 1;
const char* ScenegraphLocationDelegate::_apiName = "ScenegraphLocationDelegatePlugin";
FnPluginHost * ScenegraphLocationDelegate::_host = 0x0;

FnScenegraphLocationDelegatePluginSuite_v1 ScenegraphLocationDelegate::createSuite(FnLocationHandle(*create)())
{
    FnScenegraphLocationDelegatePluginSuite_v1 suite =
    {
        create,
        ::destroy,
        ::process,
        ::getSupportedRenderer,
        ::fillSupportedLocationList
    };

    return suite;
}

FnPlugStatus ScenegraphLocationDelegate::setHost(FnPluginHost* host)
{
    _host = host;
    FnPluginManager::PluginManager::setHost(host);
    RenderOutputUtils::setHost(host);
    FnScenegraphIterator::setHost(host);
    DefaultAssetPlugin::setHost(host);
    DefaultFileSequencePlugin::setHost(host);
    GroupBuilder::setHost(host);
    return FnAttribute::Attribute::setHost(host);
}

FnLocationHandle ScenegraphLocationDelegate::newLocationHandle(ScenegraphLocationDelegate* location)
{
    if (!location)
        return 0x0;

    FnLocationHandle h = new FnScenegraphLocationDelegateStruct(location);
    return h;
}

}
}
}
