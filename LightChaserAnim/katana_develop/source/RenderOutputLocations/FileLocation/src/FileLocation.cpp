// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnAsset/FnDefaultAssetPlugin.h>
#include <FnDefaultAttributeProducer/plugin/FnDefaultAttributeProducerUtil.h>
namespace FnDapUtil = Foundry::Katana::DapUtil;

#include <pystring/pystring.h>

#include <FileLocation.h>


FileLocation::FileLocation() : Foundry::Katana::FnRenderOutputLocationPlugin()
{
    // Empty
}

FileLocation::~FileLocation()
{
    // Empty
}

Foundry::Katana::FnRenderOutputLocationPlugin * FileLocation::create()
{
    return new FileLocation();
}

FnAttribute::GroupAttribute FileLocation::getLocationSettingsAttr(
        const std::string & outputType,
        const FnAttribute::GroupAttribute & incomingOutputAttr)
{
    FnAttribute::GroupBuilder gb;

    gb.set("renderLocation", FnAttribute::StringAttribute(""));

    FnDapUtil::SetAttrHints(gb, "renderLocation",
            FnAttribute::GroupBuilder()
                .set("widget", FnAttribute::StringAttribute("assetIdOutput"))
                .build());

    return gb.build();
}

std::string FileLocation::computeFilePath(
        const FnAttribute::GroupAttribute & outputAttrs,
        const FnAttribute::GroupAttribute & locationAttrs,
        const FnAttribute::GroupAttribute & renderSettingsAttrs,
        const FnAttribute::GroupAttribute & imageInfo,
        bool makeVersionTemplate)
{
    std::string assetId = FnAttribute::StringAttribute(
            locationAttrs.getChildByName("renderLocation")).getValue("", false);
    std::string filePath =
            Foundry::Katana::DefaultAssetPlugin::isAssetId(assetId, false) ?
            Foundry::Katana::DefaultAssetPlugin::resolveAsset(assetId, false) :
            assetId;

    return pystring::os::path::expandvars(
            pystring::os::path::expanduser(filePath));

}

std::string FileLocation::computeLocation(
        const FnAttribute::GroupAttribute & outputAttrs,
        const FnAttribute::GroupAttribute & locationAttrs,
        const FnAttribute::GroupAttribute & renderSettingsAttrs,
        const FnAttribute::GroupAttribute & imageInfo)
{
    std::string assetId = FnAttribute::StringAttribute(
            locationAttrs.getChildByName("renderLocation")).getValue("", false);
    return assetId;
}

// --- Register plugin ------------------------

DEFINE_RENDEROUTPUTLOCATION_PLUGIN(FileLocation)

void registerPlugins()
{
    REGISTER_PLUGIN(FileLocation, "file", 0, 1);
}
