// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnGeolibServicesResolutionTable_H
#define FnGeolibServicesResolutionTable_H

#include <vector>
#include <string>

#include <FnGeolibServices/FnGeolibServicesAPI.h>
#include <FnGeolibServices/ns.h>
#include <FnGeolibServices/suite/FnResolutionTableSuite.h>

#include <FnPlatform/internal/SharedPtr.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    class FNGEOLIBSERVICES_API FnResolutionTableEntry
    {
    public:
        typedef FnPlatform::internal::SharedPtr<FnResolutionTableEntry>::type
            Ptr;

        FnResolutionTableEntry(FnResolutionTableEntryHandle handle);
        ~FnResolutionTableEntry();

        std::string name() const;
        int xres() const;
        int yres() const;
        float aspectRatio() const;
        std::string fullName() const;
        std::string proxyName() const;
        std::string groupName() const;

    private:
        FnResolutionTableEntryHandle _handle;
    };

    namespace FnResolutionTable
    {
        FNGEOLIBSERVICES_API
        bool hasResolution(const std::string& name);

        FNGEOLIBSERVICES_API
        FnResolutionTableEntry::Ptr getResolution(const std::string& name);

        FNGEOLIBSERVICES_API
        FnResolutionTableEntry::Ptr findResolution(int xres,
                                                   int yres,
                                                   float aspectRatio);
        FNGEOLIBSERVICES_API
        std::string extractValidResolutionName(const std::string& str);

        FNGEOLIBSERVICES_API
        void getGroups(std::vector<std::string>& outGroups);

        FNGEOLIBSERVICES_API
        void getEntries(std::vector<FnResolutionTableEntry::Ptr>& outEntries);

        FNGEOLIBSERVICES_API
        void getEntriesForGroup(
            const std::string& group,
            std::vector<FnResolutionTableEntry::Ptr>& outEntries);
    }
}
FNGEOLIBSERVICES_NAMESPACE_EXIT

#endif // FnGeolibServicesResolutionTable_H
