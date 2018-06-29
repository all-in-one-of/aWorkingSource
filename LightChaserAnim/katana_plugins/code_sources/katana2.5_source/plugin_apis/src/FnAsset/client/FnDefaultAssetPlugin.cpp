// Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnAsset/FnDefaultAssetPlugin.h>

#include <FnAttribute/FnAttribute.h>
#include <iostream>
FNASSET_NAMESPACE_ENTER
{
    const FnAssetHostSuite* DefaultAssetPlugin::_hostSuite = NULL;

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

    std::string DefaultAssetPlugin::resolveAllAssets(
        const std::string &inputString, bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;
        FnAttribute::StringAttribute attr(
            FnAttribute::Attribute::CreateAndSteal(
                _hostSuite->resolveAllAssets(
                     inputString.c_str(), &errorMessageHandle)));

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
                return inputString;  // return input unresolved
            }
        }
        return attr.getValue(inputString, false);
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

    /**
     * Reads the list of key-value string pairs stored inside a
     * 'StringAttribute' and fills the given map with them.
     *
     * @param attr The 'StringAttribute' which stores a flat list of key-value
     *     pairs of attributes.
     * @param attributesMap The map to be filled with the attributes.
     * @return True if the function succeeded reading a valid attribute.
     */
    bool convertAttributettributeToStringMap(
        const FnAttribute::StringAttribute &attr,
        std::map<std::string, std::string> &attributesMap)
    {
        if (!attr.isValid())
        {
            return false;
        }
        FnAttribute::StringConstVector data = attr.getNearestSample(0.0);
        for (size_t i = 0; i < data.size(); i += 2)
        {
            attributesMap.insert(std::make_pair(data[i], data[i + 1]));
        }
        return true;
    }

    /**
     * Reads the list of string values from a given attributes and copy them
     * into the given vector.
     *
     * @param attr The 'StringAttribute' which stores a flat list of key-value
     *     pairs of attributes.
     * @param attributesVector The vector to be filled with the attributes.
     * @return True if the function succeeded reading a valid attribute.
     */
    bool convertAttributeToStringVector(
        const FnAttribute::StringAttribute &attr,
        std::vector<std::string> &attributesVector)
    {
        if (!attr.isValid())
        {
            return false;
        }
        FnAttribute::StringConstVector data = attr.getNearestSample(0.0);
        for (size_t i = 0; i < data.size(); ++i)
        {
            attributesVector.push_back(data[i]);
        }
        return true;
    }

    /**
     * Reads the string key-value pairs from the given map and store them
     * as a flat sequence into the given vector.
     *
     * @param map A string map to read keys and values from.
     * @param keyValueVector A vector to be filled with the sequence of
     *     key-value pair strings read from the map.
     */
    void convertStringMapToVector(
        const std::map<std::string, std::string> &map,
        std::vector<const char*> &keyValueVector)
    {
        for (std::map<std::string, std::string>::const_iterator it =
                map.begin();
             it != map.end();
             ++it)
        {
            keyValueVector.push_back(it->first.c_str());
            keyValueVector.push_back(it->second.c_str());
        }
    }

     bool DefaultAssetPlugin::getAssetAttributes(
        const std::string &assetId, const std::string &scope,
        std::map<std::string, std::string> &attributes, bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;
        FnAttribute::StringAttribute attr(
            FnAttribute::Attribute::CreateAndSteal(
                _hostSuite->getAssetAttributes(
                    assetId.c_str(), scope.c_str(), &errorMessageHandle)));

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
                return false;
            }
        }
        return convertAttributettributeToStringMap(attr, attributes);
    }

    bool DefaultAssetPlugin::checkPermissions(
        const std::string &assetId,
        const std::map<std::string, std::string> &context,
        bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;

        std::vector<const char*> keyValueVector;
        convertStringMapToVector(context, keyValueVector);

        bool ret =
            _hostSuite->checkPermissions(
                 assetId.c_str(), &(keyValueVector[0]),
                 keyValueVector.size(), &errorMessageHandle);

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
                return false;
            }
        }
        return ret;
    }

    std::string DefaultAssetPlugin::resolveAssetVersion(
        const std::string &assetId, const std::string &versionTag,
        bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;

        FnAttribute::StringAttribute ret(
            FnAttribute::Attribute::CreateAndSteal(
                _hostSuite->resolveAssetVersion(
                    assetId.c_str(), versionTag.c_str(),
                    &errorMessageHandle)));

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
                return "";
            }
        }
        return ret.getValue("", throwOnError);;
    }

    std::string DefaultAssetPlugin::getAssetDisplayName(
        const std::string &assetId, bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;

        FnAttribute::StringAttribute ret(
            FnAttribute::Attribute::CreateAndSteal(
                _hostSuite->getAssetDisplayName(
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
                return "";
            }
        }
        return ret.getValue("", throwOnError);
    }

    bool DefaultAssetPlugin::getAssetVersions(
        const std::string &assetId,
        std::vector<std::string> &versions, bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;

        FnAttribute::StringAttribute attr(
            FnAttribute::Attribute::CreateAndSteal(
                _hostSuite->getAssetVersions(
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
                return false;
            }
        }
        return convertAttributeToStringVector(attr, versions);
    }

    bool DefaultAssetPlugin::getAssetFields(
        const std::string &assetId, bool includeDefaults,
        std::map<std::string, std::string> &returnFields, bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;
        FnAttribute::StringAttribute attr(
            FnAttribute::Attribute::CreateAndSteal(
                _hostSuite->getAssetFields(
                    assetId.c_str(), includeDefaults, &errorMessageHandle)));

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
                return false;
            }
        }
        return convertAttributettributeToStringMap(attr, returnFields);
    }

    std::string DefaultAssetPlugin::buildAssetId(
        const std::map<std::string, std::string> &fields, bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;

        std::vector<const char*> keyValueVector;
        convertStringMapToVector(fields, keyValueVector);

        FnAttribute::StringAttribute ret(
            FnAttribute::Attribute::CreateAndSteal(
                _hostSuite->buildAssetId(
                    &(keyValueVector[0]), keyValueVector.size(),
                    &errorMessageHandle)));

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
                return "";
            }
        }
        return ret.getValue("", throwOnError);
    }

    std::string DefaultAssetPlugin::getAssetIdForScope(
        const std::string &assetId, const std::string &scope, bool throwOnError)
    {
        FnAttributeHandle errorMessageHandle = NULL;

        FnAttribute::StringAttribute ret(
            FnAttribute::Attribute::CreateAndSteal(
                _hostSuite->getAssetIdForScope(
                    assetId.c_str(), scope.c_str(), &errorMessageHandle)));

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
                return "";
            }
        }
        return ret.getValue("", throwOnError);
    }

    FnPlugStatus DefaultAssetPlugin::setHost(FnPluginHost * host)
    {
        if (host)
        {
            _hostSuite = static_cast<const FnAssetHostSuite*>(
                host->getSuite("Asset", FnAssetHostSuite_version));
            if (_hostSuite)
            {
                return FnPluginStatusOK;
            }
        }
        return FnPluginStatusError;
    }
}
FNASSET_NAMESPACE_EXIT
