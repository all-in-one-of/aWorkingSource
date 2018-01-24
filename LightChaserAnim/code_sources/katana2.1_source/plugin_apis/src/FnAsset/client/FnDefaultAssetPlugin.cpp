// Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnAsset/FnDefaultAssetPlugin.h>

#include <FnAttribute/FnAttribute.h>

FNASSET_NAMESPACE_ENTER
{
    const FnAssetHostSuite_v2* DefaultAssetPlugin::_hostSuite = NULL;

    bool DefaultAssetPlugin::isAssetId(const std::string& str,
                                       bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;
        bool status = _hostSuite->isAssetId(str.c_str(), &errorMessageHandle);
        FnAttribute::StringAttribute errorMessageAttr =
            FnAttribute::Attribute::CreateAndSteal(errorMessageHandle);
        if (throwOnError && errorMessageAttr.isValid())
        {
            throw std::runtime_error(errorMessageAttr.getValue());
        }
        return status;
    }

    bool DefaultAssetPlugin::containsAssetId(const std::string& str,
                                             bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;
        bool status =
            _hostSuite->containsAssetId(str.c_str(), &errorMessageHandle);
        FnAttribute::StringAttribute errorMessageAttr =
            FnAttribute::Attribute::CreateAndSteal(errorMessageHandle);
        if (throwOnError && errorMessageAttr.isValid())
        {
            throw std::runtime_error(errorMessageAttr.getValue());
        }
        return status;
    }

    std::string DefaultAssetPlugin::resolveAsset(const std::string& assetId,
                                                 bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;
        FnAttribute::StringAttribute attr(
            FnAttribute::Attribute::CreateAndSteal(_hostSuite->resolveAsset(
                assetId.c_str(), &errorMessageHandle)));

        FnAttribute::StringAttribute errorMessageAttr =
            FnAttribute::Attribute::CreateAndSteal(errorMessageHandle);
        if (errorMessageAttr.isValid())
        {
            if (throwOnError)
            {
                throw std::runtime_error(errorMessageAttr.getValue());
            }
            else
            {
                return assetId;  // return input unresolved
            }
        }
        return attr.getValue(assetId, false);
    }

    std::string DefaultAssetPlugin::resolvePath(
        const std::string& path, int frame, bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;
        FnAttribute::StringAttribute attr(
            FnAttribute::Attribute::CreateAndSteal(_hostSuite->resolvePath(
                path.c_str(), frame, &errorMessageHandle)));

        FnAttribute::StringAttribute errorMessageAttr =
            FnAttribute::Attribute::CreateAndSteal(errorMessageHandle);
        if (errorMessageAttr.isValid())
        {
            if (throwOnError)
            {
                throw std::runtime_error(errorMessageAttr.getValue());
            }
            else
            {
                return path;  // return input unresolved
            }
        }
        return attr.getValue(path, false);
    }

    std::string DefaultAssetPlugin::getUniqueScenegraphLocationFromAssetId(
        const std::string& assetId, bool includeVersion, bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;
        FnAttribute::StringAttribute attr(
            FnAttribute::Attribute::CreateAndSteal(
                _hostSuite->getUniqueScenegraphLocationFromAssetId(
                    assetId.c_str(), includeVersion, &errorMessageHandle)));

        FnAttribute::StringAttribute errorMessageAttr =
            FnAttribute::Attribute::CreateAndSteal(errorMessageHandle);
        if (errorMessageAttr.isValid())
        {
            if (throwOnError)
            {
                throw std::runtime_error(errorMessageAttr.getValue());
            }
            else
            {
                return assetId;  // return input unresolved
            }
        }
        return attr.getValue("", false);
    }

    std::string DefaultAssetPlugin::getRelatedAssetId(
        const std::string& assetId,
        const std::string& relation,
        bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;
        FnAttribute::StringAttribute attr(
            FnAttribute::Attribute::CreateAndSteal(
                _hostSuite->getRelatedAssetId(
                    assetId.c_str(), relation.c_str(), &errorMessageHandle)));

        FnAttribute::StringAttribute errorMessageAttr =
            FnAttribute::Attribute::CreateAndSteal(errorMessageHandle);
        if (errorMessageAttr.isValid())
        {
            if (throwOnError)
            {
                throw std::runtime_error(errorMessageAttr.getValue());
            }
            else
            {
                return "";  // return input unresolved
            }
        }
        return attr.getValue("", false);
    }

    FnPlugStatus DefaultAssetPlugin::setHost(FnPluginHost * host)
    {
        if (host)
        {
            _hostSuite = (FnAssetHostSuite_v2*)host->getSuite("Asset", 2);
            if (_hostSuite)
            {
                return FnPluginStatusOK;
            }
        }
        return FnPluginStatusError;
    }
}
FNASSET_NAMESPACE_EXIT
