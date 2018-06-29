// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FoundryKatanaPluginManager_H
#define FoundryKatanaPluginManager_H

#include <string>
#include <vector>

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnPluginManager/FnPluginManagerAPI.h>
#include <FnPluginManager/suite/FnPluginManagerSuite.h>

#include "ns.h"

FNPLUGINMANAGER_NAMESPACE_ENTER
{
    class FnPluginManagerAPI PluginManager
    {
    public:

        static void addSearchPath(const std::vector<std::string> &searchPath,
                                  bool replace = false);
        static void findPlugins(const std::string &cacheFilePath);
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
}
FNPLUGINMANAGER_NAMESPACE_EXIT

#endif // FoundryKatanaPluginManager_H
