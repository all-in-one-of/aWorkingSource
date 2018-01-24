#ifndef FnRenderOutputLocationPlugin_H
#define FnRenderOutputLocationPlugin_H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnPluginSystem/FnPlugin.h>
#include <FnAttribute/FnAttribute.h>

#include <FnRenderOutputLocation/suite/FnRenderOutputLocationSuite.h>
#include <FnRenderOutputLocation/FnRenderOutputLocationAPI.h>
#include <FnGeolib/op/FnGeolibCookInterface.h>

#include <vector>
#include <string>
#include <memory>

///@cond FN_INTERNAL_DEV

namespace Foundry
{
namespace Katana
{
    class FNRENDEROUTPUTLOCATION_API FnRenderOutputLocationPlugin
    {
    public:
        FnRenderOutputLocationPlugin() {}
        virtual ~FnRenderOutputLocationPlugin() {}

        static FnAttribute::GroupAttribute getLocationSettingsAttr(
                const std::string & outputType,
                const FnAttribute::GroupAttribute & incomingOutputAttr);

        virtual std::string computeFilePath(
                const FnAttribute::GroupAttribute & outputAttrs,
                const FnAttribute::GroupAttribute & locationAttrs,
                const FnAttribute::GroupAttribute & renderSettingsAttrs,
                const FnAttribute::GroupAttribute & imageInfo,
                bool makeVersionTemplate) = 0;

        virtual std::string computeLocation(
                const FnAttribute::GroupAttribute & outputAttrs,
                const FnAttribute::GroupAttribute & locationAttrs,
                const FnAttribute::GroupAttribute & renderSettingsAttrs,
                const FnAttribute::GroupAttribute & imageInfo) = 0;

        static FnPlugStatus setHost(FnPluginHost *host);
        static FnPluginHost *getHost();

        static FnRenderOutputLocationPluginSuite_v2 createSuite(
                FnRenderOutputLocationPluginHandle (*create)(),
                FnAttributeHandle (*getLocationSettingsAttr)(
                    const char * outputType,
                    FnAttributeHandle incomingOutputAttr));
        static FnRenderOutputLocationPluginHandle newPluginHandle(
                FnRenderOutputLocationPlugin * renderOutputLocationPlugin);

        static void flush();

        static unsigned int _apiVersion;
        static const char* _apiName;

    private:
        static FnPluginHost *_host;
    };

} // namespace Katana
} // namespace Foundry

namespace FnKat = Foundry::Katana;

// Plugin Registering Macro.

#define DEFINE_RENDEROUTPUTLOCATION_PLUGIN(PLUGIN_CLASS)                       \
                                                                               \
    FnPlugin PLUGIN_CLASS##_plugin;                                            \
                                                                               \
    FnRenderOutputLocationPluginHandle PLUGIN_CLASS##_create()                 \
    {                                                                          \
        return Foundry::Katana::FnRenderOutputLocationPlugin::newPluginHandle( \
            PLUGIN_CLASS::create());                                           \
    }                                                                          \
                                                                               \
    FnAttributeHandle PLUGIN_CLASS##_getLocationSettingsAttr(                  \
        const char * outputType, FnAttributeHandle incomingOutputAttr)         \
    {                                                                          \
        FnAttribute::GroupAttribute attr =                                     \
            PLUGIN_CLASS::getLocationSettingsAttr(                             \
                outputType,                                                    \
                FnAttribute::Attribute::CreateAndRetain(incomingOutputAttr));  \
        return attr.getRetainedHandle();                                       \
    }                                                                          \
                                                                               \
    FnRenderOutputLocationPluginSuite_v2 PLUGIN_CLASS##_suite =                \
        Foundry::Katana::FnRenderOutputLocationPlugin::createSuite(            \
            PLUGIN_CLASS##_create, PLUGIN_CLASS##_getLocationSettingsAttr);    \
                                                                               \
    const void* PLUGIN_CLASS##_getSuite()                                      \
    {                                                                          \
        return &PLUGIN_CLASS##_suite;                                          \
    }

struct FnRenderOutputLocationPluginStruct
{
public:
    FnRenderOutputLocationPluginStruct(
        Foundry::Katana::FnRenderOutputLocationPlugin * renderOutputLocationPlugin) :
            _renderOutputLocationPlugin(renderOutputLocationPlugin) {}
    ~FnRenderOutputLocationPluginStruct() {}

    Foundry::Katana::FnRenderOutputLocationPlugin & getRenderOutputLocationPlugin()
    {
        return *_renderOutputLocationPlugin;
    }

private:
    std::auto_ptr<Foundry::Katana::FnRenderOutputLocationPlugin> _renderOutputLocationPlugin;
};

///@endcond

#endif // FnRenderOutputLocationPlugin_H
