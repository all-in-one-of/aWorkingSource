// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnAsset/FnDefaultFileSequencePlugin.h>

#include <FnAttribute/FnAttribute.h>

FNASSET_NAMESPACE_ENTER
{
    FnAssetHostSuite_v2* DefaultFileSequencePlugin::_hostSuite = 0x0;

    bool DefaultFileSequencePlugin::isFileSequence(const std::string& path, bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = 0x0;
        bool status = _hostSuite->isFileSequence(path.c_str(), &errorMessageHandle);
        FnAttribute::StringAttribute errorMessageAttr =
            FnAttribute::Attribute::CreateAndSteal(errorMessageHandle);
        if (throwOnError && errorMessageAttr.isValid())
        {
            throw std::runtime_error(errorMessageAttr.getValue());
        }
        return status;
    }


    std::string DefaultFileSequencePlugin::buildFileSequenceString(const std::string& prefix, const std::string& suffix,
                                                                   int padding, bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = 0x0;
        FnAttribute::StringAttribute result =
                        FnAttribute::Attribute::CreateAndSteal(
                            _hostSuite->buildFileSequenceString(
                                prefix.c_str(), suffix.c_str(),
                                padding, &errorMessageHandle));
        FnAttribute::StringAttribute errorMessageAttr =
            FnAttribute::Attribute::CreateAndSteal(errorMessageHandle);
        if (throwOnError && errorMessageAttr.isValid())
        {
            throw std::runtime_error(errorMessageAttr.getValue());
        }

        return result.getValue("", false);
    }


    std::string DefaultFileSequencePlugin::resolveFileSequence(const std::string& path, int frame, bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = 0x0;
        FnAttribute::StringAttribute result =
            FnAttribute::Attribute::CreateAndSteal(
                _hostSuite->resolveFileSequence(path.c_str(),
                    frame, &errorMessageHandle));
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
                return path; // return input unresolved
            }
        }

        return result.getValue("", false);
    }


    bool DefaultFileSequencePlugin::isFrameInFileSequence(const  std::string& path, int frame, bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = 0x0;
        bool status = _hostSuite->isFrameInFileSequence(path.c_str(), frame, &errorMessageHandle);
        FnAttribute::StringAttribute errorMessageAttr =
            FnAttribute::Attribute::CreateAndSteal(errorMessageHandle);
        if (throwOnError && errorMessageAttr.isValid())
        {
            throw std::runtime_error(errorMessageAttr.getValue());
        }
        return status;
    }


    FnPlugStatus DefaultFileSequencePlugin::setHost(FnPluginHost *host)
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
