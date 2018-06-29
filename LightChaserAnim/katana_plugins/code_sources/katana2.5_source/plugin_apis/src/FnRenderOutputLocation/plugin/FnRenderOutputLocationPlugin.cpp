#include <FnRenderOutputLocation/plugin/FnRenderOutputLocationPlugin.h>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnAsset/FnDefaultAssetPlugin.h>
#include <FnAsset/FnDefaultFileSequencePlugin.h>

namespace // anonymous
{

void destroy(FnRenderOutputLocationPluginHandle handle)
{
    delete handle;
}

FnAttributeHandle computeFilePath(
        FnRenderOutputLocationPluginHandle handle,
        FnAttributeHandle outputAttrs,
        FnAttributeHandle locationAttrs,
        FnAttributeHandle renderSettingsAttrs,
        FnAttributeHandle imageInfo,
        uint8_t makeVersionTemplate)
{
    std::string result = handle->getRenderOutputLocationPlugin().computeFilePath(
            FnAttribute::Attribute::CreateAndRetain(outputAttrs),
            FnAttribute::Attribute::CreateAndRetain(locationAttrs),
            FnAttribute::Attribute::CreateAndRetain(renderSettingsAttrs),
            FnAttribute::Attribute::CreateAndRetain(imageInfo),
            makeVersionTemplate);
    return FnAttribute::StringAttribute(result).getRetainedHandle();
}

FnAttributeHandle computeLocation(
        FnRenderOutputLocationPluginHandle handle,
        FnAttributeHandle outputAttrs,
        FnAttributeHandle locationAttrs,
        FnAttributeHandle renderSettingsAttrs,
        FnAttributeHandle imageInfo)
{
    // don't steal the reference to input attrs
    std::string result = handle->getRenderOutputLocationPlugin().computeLocation(
            FnAttribute::Attribute::CreateAndRetain(outputAttrs),
            FnAttribute::Attribute::CreateAndRetain(locationAttrs),
            FnAttribute::Attribute::CreateAndRetain(renderSettingsAttrs),
            FnAttribute::Attribute::CreateAndRetain(imageInfo));
    return FnAttribute::StringAttribute(result).getRetainedHandle();
}

} // namespace anonymous

namespace Foundry
{
namespace Katana
{

FnPlugStatus FnRenderOutputLocationPlugin::setHost(FnPluginHost* host)
{
    _host = host;
    FnAsset::DefaultAssetPlugin::setHost(host);
    FnAsset::DefaultFileSequencePlugin::setHost(host);
    FnAttribute::GroupBuilder::setHost(host);
    return FnAttribute::Attribute::setHost(host);
}

FnPluginHost* FnRenderOutputLocationPlugin::getHost()
{
    return _host;
}

FnRenderOutputLocationPluginSuite_v2 FnRenderOutputLocationPlugin::createSuite(
        FnRenderOutputLocationPluginHandle (*create)(),
        FnAttributeHandle (*getLocationSettingsAttr)(
            const char * outputType,
            FnAttributeHandle incomingOutputAttr))
{
    FnRenderOutputLocationPluginSuite_v2 suite;

    suite.create                    = create;
    suite.destroy                   = ::destroy;
    suite.getLocationSettingsAttr   = getLocationSettingsAttr;
    suite.computeFilePath           = ::computeFilePath;
    suite.computeLocation           = ::computeLocation;

    return suite;
};

FnRenderOutputLocationPluginHandle FnRenderOutputLocationPlugin::newPluginHandle(
        FnRenderOutputLocationPlugin * renderOutputLocationPlugin)
{
    if (!renderOutputLocationPlugin)
    {
        return 0x0;
    }

    FnRenderOutputLocationPluginHandle h =
            new FnRenderOutputLocationPluginStruct(renderOutputLocationPlugin);
    return h;
}

void FnRenderOutputLocationPlugin::flush()
{
}

unsigned int FnRenderOutputLocationPlugin::_apiVersion = 2;
const char* FnRenderOutputLocationPlugin::_apiName = "RenderOutputLocationPlugin";
FnPluginHost* FnRenderOutputLocationPlugin::_host = 0x0;

} // namespace Katana
} // namespace Foundry
