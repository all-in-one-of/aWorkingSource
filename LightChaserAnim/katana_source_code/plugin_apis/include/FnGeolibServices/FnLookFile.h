// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnGeolibServicesLookFile_H
#define FnGeolibServicesLookFile_H

#include <string>
#include <inttypes.h>

#include <tr1/memory>
#define PLUGIN_SHARED_PTR std::tr1::shared_ptr

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnAttribute/FnAttribute.h>

#include <FnGeolibServices/suite/FnLookFileSuite.h>

#include "ns.h"

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    class FnLookFile
    {
    public:
        typedef PLUGIN_SHARED_PTR<FnLookFile> Ptr;

        class AttrMap
        {
        public:
            typedef PLUGIN_SHARED_PTR<AttrMap> Ptr;

            FnAttribute::Attribute get(const std::string & key) const;
            FnAttribute::StringAttribute getKeys() const;

            ~AttrMap();

        private:
            friend class FnLookFile; // only FnLookFile can create these
            AttrMap(FnLookFileAttrMapHandle handle);

            static const FnLookFileHostSuite_v2 * _suite;

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
