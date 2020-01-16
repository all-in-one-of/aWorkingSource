// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FoundryKatanaPluginManager_H
#define FoundryKatanaPluginManager_H

#include <string>
#include <vector>

#include <FnPlatform/internal/Atomic.h>

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnPluginManager/FnPluginManagerAPI.h>
#include <FnPluginManager/suite/FnPluginManagerSuite.h>

#include "ns.h"

FNPLUGINMANAGER_NAMESPACE_ENTER
{
    class FNPLUGINMANAGER_API PluginManager
    {
    public:

        static void addSearchPath(const std::vector<std::string> &searchPath,
                                  bool replace = false);
        static std::vector<std::string> getSearchPaths();
        static void findPlugins();
        static void flushPluginCaches();
        static void flushSpecificPluginCache(FnPluginHandle pluginHandle);
        static void getApiNames(std::vector<std::string>& names);
        static void getPluginApiVersions(const std::string& apiName,
                                           std::vector<int>& names);
        static void getPluginNames(const std::string& apiName,
                                   std::vector<std::string>& names,
                                   unsigned int apiVersion);
        static FnPluginHandle getPlugin(const std::string& pluginName,
                                        const std::string& apiName,
                                        unsigned int apiVersion);
        static const void* getPluginSuite(FnPluginHandle h);
        static const char* getPluginPath(FnPluginHandle h);

        static void releasePluginHandle(FnPluginHandle h);

        static const FnPluginManagerHostSuite_v1* getSuite();
        static const void *getHostSuite(const std::string& apiName,
                                        unsigned int apiVersion);

        static FnPluginStatus setHost(FnPluginHost* host);
        static FnPluginHost* getHost();

    protected:

        PluginManager();
        ~PluginManager();

        static const FnPluginManagerHostSuite_v1 * _pluginMgrSuite;
    };

    /**
     * Class that facilitates lazy, thread-safe fetching of a host suite
     * pointer from \c PluginManager.
     *
     * \code
     * static LazyHostSuite<MySuite_v2> suite = {"MySuite", 2};
     * suite->createPony();
     * \endcode
     */
    template <typename SuiteT>
    class LazyHostSuite
    {
    public:
        const SuiteT* get()
        {
            using FnPlatform::internal::AtomicCompareExchangeStrong;
            using FnPlatform::internal::AtomicThreadFence;

            const SuiteT* suite = suite_;
            AtomicThreadFence();
            if (!suite)
            {
                const SuiteT* newVal = static_cast<const SuiteT*>(
                    FnPluginManager::PluginManager::getHostSuite(apiName_,
                                                                 apiVersion_));

                const SuiteT* oldVal = NULL;
                suite = AtomicCompareExchangeStrong(&suite_, &oldVal, newVal)
                            ? newVal
                            : oldVal;
            }
            return suite;
        }

        const SuiteT* operator->()
        {
            return get();
        }

        const SuiteT& operator*()
        {
            return *get();
        }

        const char* apiName_;
        unsigned int apiVersion_;
        const SuiteT* volatile suite_;
    };
}
FNPLUGINMANAGER_NAMESPACE_EXIT

#endif // FoundryKatanaPluginManager_H
