// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnGeolibServicesResolutionTable_H
#define FnGeolibServicesResolutionTable_H

#include <vector>
#include <string>

#include <FnGeolibServices/suite/FnResolutionTableSuite.h>

#include <tr1/memory>
#define PLUGIN_SHARED_PTR std::tr1::shared_ptr

#include "ns.h"

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    class FnResolutionTableEntry
    {
    public:
        typedef PLUGIN_SHARED_PTR<FnResolutionTableEntry> Ptr;

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
        bool hasResolution(const std::string & name);
        FnResolutionTableEntry::Ptr getResolution(const std::string & name);
        FnResolutionTableEntry::Ptr findResolution(
                int xres, int yres, float aspectRatio);
        std::string extractValidResolutionName(const std::string & str);
        void getGroups(std::vector<std::string> & outGroups);
        void getEntries(std::vector<FnResolutionTableEntry::Ptr> & outEntries);
        void getEntriesForGroup(const std::string & group,
                std::vector<FnResolutionTableEntry::Ptr> & outEntries);
    }
}
FNGEOLIBSERVICES_NAMESPACE_EXIT

#endif // FnGeolibServicesResolutionTable_H
