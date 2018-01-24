#include <cassert>
#include <iostream>

#include <FnPluginManager/FnPluginManager.h>

FNPLUGINMANAGER_NAMESPACE_ENTER
{

PluginManager::PluginManager() {}
PluginManager::~PluginManager() {}

FnPluginHandle PluginManager::getPlugin(const std::string& pluginName, const std::string& apiName, unsigned int apiVersion)
{
    return getSuite()->getPlugin(pluginName.c_str(), apiName.c_str(), apiVersion);
}

const void * PluginManager::getPluginSuite(FnPluginHandle h)
{
    return getSuite()->getPluginSuite(h);
}

void PluginManager::getApiNames(std::vector<std::string>& names)
{
    names.clear();
    FnPluginNamesHandle h = getSuite()->getApiNames();
    const size_t size = getSuite()->getPluginListSize(h);
    for (size_t i = 0; i < size; ++i)
    {
        names.push_back(getSuite()->getPluginNameFromList(h, static_cast<int>(i)));
    }
    return getSuite()->releasePluginNamesHandle(h);
}


void  PluginManager::getPluginNames(const std::string& apiName,
                                    std::vector<std::string>& names,
                                    unsigned int apiVersion)
{
    names.clear();
    FnPluginNamesHandle h = getSuite()->getPluginNames(apiName.c_str(), apiVersion);
    const size_t size = getSuite()->getPluginListSize(h);
    for (size_t i = 0; i < size; ++i)
    {
        names.push_back(getSuite()->getPluginNameFromList(h, static_cast<int>(i)));
    }
    return getSuite()->releasePluginNamesHandle(h);
}

void  PluginManager::getPluginApiVersions(const std::string& apiName,
                                    std::vector<int>& versions)
{
    versions.clear();
    FnPluginVersionsHandle h = getSuite()->getPluginApiVersions(apiName.c_str());
    const size_t size = getSuite()->getPluginApiVersionsListSize(h);
    for (size_t i = 0; i < size; ++i)
    {
        versions.push_back(getSuite()->getPluginApiVersionFromList(h, static_cast<int>(i)));
    }
    return getSuite()->releasePluginApiVersionsHandle(h);
}

const char* PluginManager::getPluginPath(FnPluginHandle h)
{
    return getSuite()->getPluginPath(h);
}

void PluginManager::releasePluginHandle(FnPluginHandle h)
{
    return getSuite()->releasePluginHandle(h);
}

const FnPluginManagerHostSuite_v1 *PluginManager::getSuite()
{
    assert(_pluginMgrSuite);
    return _pluginMgrSuite;
}

const void * PluginManager::getHostSuite(const std::string& apiName,
                                         unsigned int apiVersion)
{
    return getSuite()->getHostSuite(apiName.c_str(), apiVersion);
}

void PluginManager::addSearchPath(const std::vector<std::string> &searchPath, bool replace)
{
    const int s = static_cast<int>(searchPath.size());
    const char ** p = new const char*[s];
    for (int i = 0; i < s; ++i)
    {
        const std::string & sp = searchPath[i];
        p[i] = sp.c_str();
    }
    getSuite()->addSearchPath(p, s, replace);
    delete [] p;
}

void PluginManager::findPlugins(const std::string &cacheFilePath)
{
    getSuite()->findPlugins(cacheFilePath.c_str());
}

void PluginManager::flushPluginCaches()
{
    getSuite()->flushPluginCaches();
}

void PluginManager::flushSpecificPluginCache(FnPluginHandle pluginHandle)
{
    getSuite()->flushSpecificPluginCache(pluginHandle);
}


FnPluginStatus PluginManager::setHost(FnPluginHost* host)
{
    if (host)
    {
        _pluginMgrSuite = (FnPluginManagerHostSuite_v1*)(host->getSuite("PluginManager", 1));
        if (_pluginMgrSuite)
        {
            return FnPluginStatusOK;
        }
    }
    return FnPluginStatusError;

}

FnPluginHost* PluginManager::getHost()
{
    return (FnPluginHost*)getSuite()->getHost();
}

const FnPluginManagerHostSuite_v1* PluginManager::_pluginMgrSuite = 0x0;

}
FNPLUGINMANAGER_NAMESPACE_EXIT

