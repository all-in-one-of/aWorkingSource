// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef INCLUDED_FILELOCATION_H
#define INCLUDED_FILELOCATION_H

#include <FnAttribute/FnAttribute.h>
#include <FnRenderOutputLocation/plugin/FnRenderOutputLocationPlugin.h>


class LocalLocation : public Foundry::Katana::FnRenderOutputLocationPlugin
{
public:
    LocalLocation();
    virtual ~LocalLocation();

    static FnAttribute::GroupAttribute getLocationSettingsAttr(
            const std::string & outputType,
            const FnAttribute::GroupAttribute & incomingOutputAttr);

    virtual std::string computeFilePath(
            const FnAttribute::GroupAttribute & outputAttrs,
            const FnAttribute::GroupAttribute & locationAttrs,
            const FnAttribute::GroupAttribute & renderSettingsAttrs,
            const FnAttribute::GroupAttribute & imageInfo,
            bool makeVersionTemplate);

    virtual std::string computeLocation(
            const FnAttribute::GroupAttribute & outputAttrs,
            const FnAttribute::GroupAttribute & locationAttrs,
            const FnAttribute::GroupAttribute & renderSettingsAttrs,
            const FnAttribute::GroupAttribute & imageInfo);

    static Foundry::Katana::FnRenderOutputLocationPlugin * create();
};

#endif  // INCLUDED_FILELOCATION_H
