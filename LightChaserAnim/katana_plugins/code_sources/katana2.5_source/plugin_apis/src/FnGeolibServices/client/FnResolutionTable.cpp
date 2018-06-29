// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnGeolibServices/FnResolutionTable.h>

#include <FnAttribute/FnAttribute.h>

#include <FnPluginManager/FnPluginManager.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{

    namespace // anonymous
    {
        const FnResolutionTableHostSuite_v1 * _getSuite()
        {
            static FnPluginManager::LazyHostSuite<FnResolutionTableHostSuite_v1>
                suite = { "ResolutionTableHost", 1 };
            return suite.get();
        }

        FnResolutionTableEntry::Ptr _newEntryFromHandle(
                FnResolutionTableEntryHandle handle)
        {
            if (handle)
            {
                return FnResolutionTableEntry::Ptr(
                        new FnResolutionTableEntry(handle));
            }
            else
            {
                return FnResolutionTableEntry::Ptr();
            }
        }

        void _fillEntryVectorFromEntryListHandle(
                FnResolutionTableEntryListHandle handle,
                std::vector<FnResolutionTableEntry::Ptr> & outEntries)
        {
            const FnResolutionTableHostSuite_v1 * suite = _getSuite();
            if (!suite) return;

            int64_t numEntries = suite->entryListNumEntries(handle);
            for (int64_t i=0; i<numEntries; ++i)
            {
                FnResolutionTableEntry::Ptr entryPtr = _newEntryFromHandle(
                    suite->entryListEntry(handle, i));
                outEntries.push_back(entryPtr);
            }
        }
    } // namespace anonymous

    namespace FnResolutionTable
    {
        bool hasResolution(const std::string & name)
        {
            const FnResolutionTableHostSuite_v1 * suite = _getSuite();
            if (!suite) return false;

            return suite->hasResolution(name.c_str()) != 0;
        }

        FnResolutionTableEntry::Ptr getResolution(const std::string & name)
        {
            const FnResolutionTableHostSuite_v1 * suite = _getSuite();
            if (!suite) return FnResolutionTableEntry::Ptr();

            return _newEntryFromHandle(
                    suite->getResolution(name.c_str()));
        }

        FnResolutionTableEntry::Ptr findResolution(
                int xres, int yres, float aspectRatio)
        {
            const FnResolutionTableHostSuite_v1 * suite = _getSuite();
            if (!suite) return FnResolutionTableEntry::Ptr();

            return _newEntryFromHandle(
                    suite->findResolution(xres, yres, aspectRatio));
        }

        std::string extractValidResolutionName(const std::string & str)
        {
            const FnResolutionTableHostSuite_v1 * suite = _getSuite();
            if (!suite) return std::string();

            FnAttribute::StringAttribute resultAttr =
                    FnAttribute::Attribute::CreateAndSteal(
                            suite->extractValidResolutionName(str.c_str()));
            return resultAttr.getValue("", false);
        }

        void getGroups(std::vector<std::string> & outGroups)
        {
            outGroups.clear();

            const FnResolutionTableHostSuite_v1 * suite = _getSuite();
            if (!suite) return;

            FnAttribute::StringAttribute resultAttr =
                    FnAttribute::Attribute::CreateAndSteal(
                            suite->getGroups());
            if (resultAttr.isValid())
            {
                FnAttribute::StringConstVector resultValue =
                        resultAttr.getNearestSample(0.0f);
                outGroups.insert(outGroups.begin(),
                        resultValue.begin(), resultValue.end());
            }
        }


        void getEntries(std::vector<FnResolutionTableEntry::Ptr> & outEntries)
        {
            outEntries.clear();

            const FnResolutionTableHostSuite_v1 * suite = _getSuite();
            if (!suite) return;

            FnResolutionTableEntryListHandle handle = suite->getEntries();

            _fillEntryVectorFromEntryListHandle(handle, outEntries);

            suite->releaseEntryList(handle);
        }

        void getEntriesForGroup(const std::string & group,
                std::vector<FnResolutionTableEntry::Ptr> & outEntries)
        {
            outEntries.clear();

            const FnResolutionTableHostSuite_v1 * suite = _getSuite();
            if (!suite) return;

            FnResolutionTableEntryListHandle handle = suite->getEntriesForGroup(
                    group.c_str());

            _fillEntryVectorFromEntryListHandle(handle, outEntries);

            suite->releaseEntryList(handle);
        }
    }

    FnResolutionTableEntry::FnResolutionTableEntry(
            FnResolutionTableEntryHandle handle) : _handle(handle)
    {
    }

    FnResolutionTableEntry::~FnResolutionTableEntry()
    {
        const FnResolutionTableHostSuite_v1 * suite = _getSuite();
        if (suite && _handle)
        {
            suite->releaseEntry(_handle);
        }
    }

    std::string FnResolutionTableEntry::name() const
    {
        const FnResolutionTableHostSuite_v1 * suite = _getSuite();
        if (suite && _handle)
        {
            return suite->entryName(_handle);
        }

        return std::string();
    }

    int FnResolutionTableEntry::xres() const
    {
        const FnResolutionTableHostSuite_v1 * suite = _getSuite();
        if (suite && _handle)
        {
            return suite->entryXres(_handle);
        }

        return 0;
    }

    int FnResolutionTableEntry::yres() const
    {
        const FnResolutionTableHostSuite_v1 * suite = _getSuite();
        if (suite && _handle)
        {
            return suite->entryYres(_handle);
        }

        return 0;
    }

    float FnResolutionTableEntry::aspectRatio() const
    {
        const FnResolutionTableHostSuite_v1 * suite = _getSuite();
        if (suite && _handle)
        {
            return suite->entryAspectRatio(_handle);
        }

        return 0.0f;
    }

    std::string FnResolutionTableEntry::fullName() const
    {
        const FnResolutionTableHostSuite_v1 * suite = _getSuite();
        if (suite && _handle)
        {
            return suite->entryFullName(_handle);
        }

        return std::string();
    }

    std::string FnResolutionTableEntry::proxyName() const
    {
        const FnResolutionTableHostSuite_v1 * suite = _getSuite();
        if (suite && _handle)
        {
            return suite->entryProxyName(_handle);
        }

        return std::string();
    }

    std::string FnResolutionTableEntry::groupName() const
    {
        const FnResolutionTableHostSuite_v1 * suite = _getSuite();
        if (suite && _handle)
        {
            return suite->entryGroupName(_handle);
        }

        return std::string();
    }
}
FNGEOLIBSERVICES_NAMESPACE_EXIT
