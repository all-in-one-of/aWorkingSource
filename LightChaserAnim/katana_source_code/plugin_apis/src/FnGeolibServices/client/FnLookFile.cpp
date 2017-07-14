// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnGeolibServices/FnLookFile.h>

#include <FnPluginManager/FnPluginManager.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    FnLookFile::FnLookFile(FnLookFileHandle handle) : _handle(handle)
    {
    }

    FnLookFile::~FnLookFile()
    {
        const FnLookFileHostSuite_v2 * suite = _getSuite();
        if (suite)
        {
            suite->releaseLookFile(_handle);
        }
    }

    FnLookFile::Ptr FnLookFile::getLookFile(const std::string & filePath,
            const std::string & passName)
    {
        const FnLookFileHostSuite_v2 * suite = _getSuite();
        if (!suite) return FnLookFile::Ptr();

        FnLookFileHandle handle = suite->getLookFile(
                filePath.c_str(), passName.c_str());
        if (!handle) return FnLookFile::Ptr();

        return FnLookFile::Ptr(new FnLookFile(handle));
    }

    FnLookFile::AttrMap::Ptr FnLookFile::getMaterials() const
    {
        const FnLookFileHostSuite_v2 * suite = _getSuite();
        if (!suite || !_handle) return FnLookFile::AttrMap::Ptr();

        FnLookFileAttrMapHandle attrMapHandle =
                suite->getLookFileMaterials(_handle);
        if (!attrMapHandle) return FnLookFile::AttrMap::Ptr();

        return FnLookFile::AttrMap::Ptr(new FnLookFile::AttrMap(attrMapHandle));
    }

    std::string FnLookFile::getMaterialType(
            const std::string & materialName) const
    {
        const FnLookFileHostSuite_v2 * suite = _getSuite();
        if (!suite || !_handle) return std::string();

        FnAttribute::StringAttribute attr =
                FnAttribute::Attribute::CreateAndSteal(
                        suite->getLookFileMaterialType(
                                _handle, materialName.c_str()));

        return attr.getValue("", false);
    }

    FnAttribute::Attribute FnLookFile::getMaterial(const std::string & name,
            bool global) const
    {
        const FnLookFileHostSuite_v2 * suite = _getSuite();
        if (!suite || !_handle) return FnAttribute::Attribute();

        return FnAttribute::Attribute::CreateAndSteal(
                suite->getLookFileMaterial(_handle, name.c_str(), global));
    }

    FnAttribute::StringAttribute FnLookFile::getRootIdNames() const
    {
        const FnLookFileHostSuite_v2 * suite = _getSuite();
        if (!suite || !_handle) return FnAttribute::StringAttribute();

        return FnAttribute::Attribute::CreateAndSteal(
                suite->getLookFileRootIdNames(_handle));
    }

    std::string FnLookFile::getRootIdType(const std::string & rootId) const
    {
        const FnLookFileHostSuite_v2 * suite = _getSuite();
        if (!suite || !_handle) return std::string();

        FnAttribute::StringAttribute attr =
            FnAttribute::Attribute::CreateAndSteal(
                suite->getLookFileRootIdType(_handle, rootId.c_str()));

        return attr.getValue("", false);
    }

    FnAttribute::StringAttribute FnLookFile::getPathsWithOverrides(
            const std::string & rootId) const
    {
        const FnLookFileHostSuite_v2 * suite = _getSuite();
        if (!suite || !_handle) return FnAttribute::StringAttribute();

        return FnAttribute::Attribute::CreateAndSteal(
                suite->getLookFilePathsWithOverrides(_handle, rootId.c_str()));
    }

    FnLookFile::AttrMap::Ptr FnLookFile::getAttrs(const std::string & locationName,
            const std::string & rootId) const
    {
        const FnLookFileHostSuite_v2 * suite = _getSuite();
        if (!suite || !_handle) return FnLookFile::AttrMap::Ptr();

        FnLookFileAttrMapHandle attrMapHandle =
                suite->getLookFileAttrs(_handle, locationName.c_str(),
                        rootId.c_str());
        if (!attrMapHandle) return FnLookFile::AttrMap::Ptr();

        return FnLookFile::AttrMap::Ptr(new FnLookFile::AttrMap(attrMapHandle));
    }

    FnLookFile::AttrMap::Ptr FnLookFile::getRootOverrides() const
    {
        const FnLookFileHostSuite_v2 * suite = _getSuite();
        if (!suite || !_handle) return FnLookFile::AttrMap::Ptr();

        FnLookFileAttrMapHandle attrMapHandle =
                suite->getLookFileRootOverrides(_handle);
        if (!attrMapHandle) return FnLookFile::AttrMap::Ptr();

        return FnLookFile::AttrMap::Ptr(new FnLookFile::AttrMap(attrMapHandle));
    }

    FnAttribute::StringAttribute FnLookFile::getPassNamesForLookFileAsset(
            const std::string & asset)
    {
        const FnLookFileHostSuite_v2 * suite = _getSuite();
        if (!suite) return FnAttribute::StringAttribute();

        return FnAttribute::Attribute::CreateAndSteal(
                suite->getPassNamesForLookFileAsset(asset.c_str()));
    }

    FnAttribute::StringAttribute FnLookFile::getSafePath(
            const std::string & asset, bool includeVersion)
    {
        const FnLookFileHostSuite_v2 * suite = _getSuite();
        if (!suite) return FnAttribute::StringAttribute();

        return FnAttribute::Attribute::CreateAndSteal(
                suite->getSafePath(asset.c_str(), includeVersion));
    }

    void FnLookFile::flushCache()
    {
        const FnLookFileHostSuite_v2 * suite = _getSuite();
        if (!suite) return;

        suite->flushCache();
    }

    const FnLookFileHostSuite_v2 * FnLookFile::AttrMap::_suite = 0x0;

    FnLookFile::AttrMap::AttrMap(FnLookFileAttrMapHandle handle)
            : _handle(handle)
    {
    }

    FnLookFile::AttrMap::~AttrMap()
    {
        if (_suite)
        {
            _suite->releaseLookFileAttrMap(_handle);
        }
    }

    FnAttribute::Attribute FnLookFile::AttrMap::get(
            const std::string & key) const
    {
        if (!_suite) return FnAttribute::Attribute();

        return FnAttribute::Attribute::CreateAndSteal(
                _suite->getAttrMapValue(_handle, key.c_str()));
    }

    FnAttribute::StringAttribute FnLookFile::AttrMap::getKeys() const
    {
        if (!_suite || !_handle) return FnAttribute::StringAttribute();

        return FnAttribute::Attribute::CreateAndSteal(
                _suite->getAttrMapKeys(_handle));
    }

    ////////////////////////////////////////////////////////////////////////////

    const FnLookFileHostSuite_v2 *FnLookFile::_getSuite()
    {
        static const FnLookFileHostSuite_v2 * suite = 0x0;

        if (!suite)
        {
            suite = reinterpret_cast<const FnLookFileHostSuite_v2 *>(
                    FnPluginManager::PluginManager::getHostSuite(
                            "LookFileHost", FnLookFileHostSuite_version));
            AttrMap::_suite = suite;
        }

        return suite;
    }
}
FNGEOLIBSERVICES_NAMESPACE_EXIT
