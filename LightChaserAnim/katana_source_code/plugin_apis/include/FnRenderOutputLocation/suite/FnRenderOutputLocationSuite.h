#ifndef FnRenderOutputLocationSuite_H
#define FnRenderOutputLocationSuite_H

extern "C" {

#include <FnAttribute/suite/FnAttributeSuite.h>

typedef struct FnRenderOutputLocationPluginStruct *
        FnRenderOutputLocationPluginHandle;

#define FnRenderOutputLocationPluginSuite_version 2

struct FnRenderOutputLocationPluginSuite_v1
{
    FnRenderOutputLocationPluginHandle (*create)();
    void (*destroy)(FnRenderOutputLocationPluginHandle handle);

    FnAttributeHandle (*getLocationSettingsAttr)(
            const char * outputType,
            FnAttributeHandle incomingOutputAttr);

    FnAttributeHandle (*computeFilePath)(
            FnRenderOutputLocationPluginHandle handle,
            FnAttributeHandle outputAttrs,
            FnAttributeHandle locationAttrs,
            FnAttributeHandle imageInfo,
            uint8_t makeVersionTemplate);

    FnAttributeHandle (*computeLocation)(
            FnRenderOutputLocationPluginHandle handle,
            FnAttributeHandle outputAttrs,
            FnAttributeHandle locationAttrs,
            FnAttributeHandle imageInfo);
};

struct FnRenderOutputLocationPluginSuite_v2
{
    FnRenderOutputLocationPluginHandle (*create)();
    void (*destroy)(FnRenderOutputLocationPluginHandle handle);

    FnAttributeHandle (*getLocationSettingsAttr)(
            const char * outputType,
            FnAttributeHandle incomingOutputAttr);

    FnAttributeHandle (*computeFilePath)(
            FnRenderOutputLocationPluginHandle handle,
            FnAttributeHandle outputAttrs,
            FnAttributeHandle locationAttrs,
            FnAttributeHandle renderSettingAttrs,
            FnAttributeHandle imageInfo,
            uint8_t makeVersionTemplate);

    FnAttributeHandle (*computeLocation)(
            FnRenderOutputLocationPluginHandle handle,
            FnAttributeHandle outputAttrs,
            FnAttributeHandle locationAttrs,
            FnAttributeHandle renderSettingAttrs,
            FnAttributeHandle imageInfo);
};

}
#endif // FnRenderOutputLocationSuite_H
