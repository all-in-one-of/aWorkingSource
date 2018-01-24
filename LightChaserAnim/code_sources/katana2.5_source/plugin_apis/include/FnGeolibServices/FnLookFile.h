// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnGeolibServicesLookFile_H
#define FnGeolibServicesLookFile_H

#include <string>
#include <stdint.h>

#include <FnAttribute/FnAttribute.h>

#include <FnGeolibServices/FnGeolibServicesAPI.h>
#include <FnGeolibServices/ns.h>
#include <FnGeolibServices/suite/FnLookFileSuite.h>

#include <FnPlatform/internal/SharedPtr.h>

#include <FnPluginSystem/FnPluginSystem.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    class FNGEOLIBSERVICES_API FnLookFile
    {
    public:
        typedef FnPlatform::internal::SharedPtr<FnLookFile>::type Ptr;

        class FNGEOLIBSERVICES_API AttrMap
        {
        public:
            typedef FnPlatform::internal::SharedPtr<AttrMap>::type Ptr;

            FnAttribute::Attribute get(const std::string & key) const;
            FnAttribute::StringAttribute getKeys() const;

            ~AttrMap();

        private:
            friend class FnLookFile; // only FnLookFile can create these
            AttrMap(FnLookFileAttrMapHandle handle);

            FnLookFileAttrMapHandle _handle;
        };

        ~FnLookFile();

        static Ptr getLookFile(const std::string & filePath,
                const std::string & passName=std::string());

        AttrMap::Ptr getMaterials() const;
        std::string getMaterialType(const std::string & materialName) const;
        FnAttribute::Attribute getMaterial(const std::string & name,
                bool global=true) const;

        FnAttribute::StringAttribute getRootIdNames() const;
        std::string getRootIdType(const std::string & rootId) const;
        FnAttribute::StringAttribute getPathsWithOverrides(
                const std::string & rootId=std::string()) const;
        AttrMap::Ptr getAttrs(const std::string & locationName,
                const std::string & rootId=std::string()) const;
        AttrMap::Ptr getRootOverrides() const;

        static FnAttribute::StringAttribute getPassNamesForLookFileAsset(
                const std::string & asset);

        static FnAttribute::StringAttribute getSafePath(
                const std::string & asset, bool includeVersion);

        static void flushCache();

    private:
        FnLookFile(FnLookFileHandle handle);

        static const FnLookFileHostSuite_v2 * _getSuite();

        FnLookFileHandle _handle;
    };
}
FNGEOLIBSERVICES_NAMESPACE_EXIT




#endif // FnGeolibServicesLookFile_H
