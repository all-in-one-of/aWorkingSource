// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "LocalLocation.h"

#include <cstdlib>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <pystring/pystring.h>



LocalLocation::LocalLocation() : Foundry::Katana::FnRenderOutputLocationPlugin()
{
    // Empty
}

LocalLocation::~LocalLocation()
{
    // Empty
}

Foundry::Katana::FnRenderOutputLocationPlugin * LocalLocation::create()
{
    return new LocalLocation();
}

FnAttribute::GroupAttribute LocalLocation::getLocationSettingsAttr(
        const std::string & outputType,
        const FnAttribute::GroupAttribute & incomingOutputAttr)
{
    FnAttribute::GroupBuilder gb;
    return gb.build();
}

std::string LocalLocation::computeFilePath(
        const FnAttribute::GroupAttribute & outputAttrs,
        const FnAttribute::GroupAttribute & locationAttrs,
        const FnAttribute::GroupAttribute & renderSettingsAttrs,
        const FnAttribute::GroupAttribute & imageInfo,
        bool makeVersionTemplate)
{
#ifdef _WIN32
    std::string tmpDir;
#else
    std::string tmpDir = "/tmp";
#endif // _WIN32
    const char *tmpDirEnv = ::getenv("KATANA_TMPDIR");
    if (tmpDirEnv)
    {
        tmpDir = tmpDirEnv;
    }
#ifdef _WIN32
    else
    {
        tmpDirEnv = ::getenv("TEMP");
        if (tmpDirEnv)
        {
            tmpDir = tmpDirEnv;
        }
    }
#endif // _WIN32

    std::string imageName;
    const char *imageKeys[] = {"passName", "outputName", "channel",
                               "convertedResolution", "colorspace", 0x0};

    for (const char **imageKey = imageKeys; *imageKey; ++imageKey)
    {
        const std::string value = FnAttribute::StringAttribute(
                imageInfo.getChildByName(*imageKey)).getValue("", false);

        if (value.empty())
        {
            continue;
        }

        if (!imageName.empty())
        {
            imageName += "_";
        }
        imageName += value;
    }

    std::string fileExt = FnAttribute::StringAttribute(
            imageInfo.getChildByName("fileExtension")).getValue("error", false);

#ifdef _WIN32
    return tmpDir + "\\" + imageName + ".#." + fileExt;
#else
    return tmpDir + "/" + imageName + ".#." + fileExt;
#endif // _WIN32
}

std::string LocalLocation::computeLocation(
        const FnAttribute::GroupAttribute & outputAttrs,
        const FnAttribute::GroupAttribute & locationAttrs,
        const FnAttribute::GroupAttribute & renderSettingsAttrs,
        const FnAttribute::GroupAttribute & imageInfo)
{
    return computeFilePath(outputAttrs, locationAttrs, renderSettingsAttrs,
                           imageInfo, true);
}

// --- Register plug-in ------------------------

DEFINE_RENDEROUTPUTLOCATION_PLUGIN(LocalLocation)

void registerPlugins()
{
    REGISTER_PLUGIN(LocalLocation, "local", 0, 1);
}
