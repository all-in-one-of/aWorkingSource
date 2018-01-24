#ifndef FnPluginManager_H
#define FnPluginManager_H

#include <stdint.h>
#include <stddef.h>

extern "C" {

typedef struct FnPluginStruct * FnPluginHandle;
typedef struct FnPluginNamesStruct * FnPluginNamesHandle;
typedef struct FnPluginVersionsStruct * FnPluginVersionsHandle;

#define FnPluginManagerHostSuite_version 1

struct FnPluginManagerHostSuite_v1
{
    FnPluginHandle (*getPlugin)(const char* pluginName, const char* apiName, int32_t apiVersion);
    FnPluginNamesHandle (*getApiNames)();
    FnPluginNamesHandle (*getPluginNames)(const char* apiName, int32_t apiVersion);
    FnPluginVersionsHandle (*getPluginApiVersions)(const char* apiName);
    size_t (*getPluginApiVersionsListSize)(FnPluginVersionsHandle);
    int (*getPluginApiVersionFromList)(FnPluginVersionsHandle h, int32_t index);
    size_t (*getPluginListSize)(FnPluginNamesHandle);
    const char* (*getPluginNameFromList)(FnPluginNamesHandle h, int32_t index);
    const void* (*getPluginSuite)(FnPluginHandle h);
    const char* (*getPluginPath)(FnPluginHandle h);
    void (*addHostSuite)(void* suite, const char * apiName, uint32_t apiVersion);
    void (*addSearchPath)(const char ** searchPath, int32_t numPaths,
                          bool replace);
    void (*findPlugins)(const char * cacheFilePath);
    void (*flushPluginCaches)();
    void (*flushSpecificPluginCache)(FnPluginHandle pluginHandle);

    const void* (*getSuite)(FnPluginHandle h);
    const void* (*getHostSuite)(const char * apiName, uint32_t apiVersion);
    void* (*getHost)();

    void (*releasePluginHandle)(FnPluginHandle h);
    void (*releasePluginNamesHandle)(FnPluginNamesHandle h);
    void (*releasePluginApiVersionsHandle)(FnPluginVersionsHandle h);

    /// Returns the list of plug-in search paths consulted by the plug-in
    /// manager.
    ///
    /// \param buf Buffer in which all string data is stored.
    /// \param bufSize On entry, points at the size of \c buf. On return,
    /// points to the number of bytes written to \c buf.
    /// \return a NULL-terminated array of search paths.
    /// \note If the buffer is too small, returns \c NULL and sets \c *bufSize
    /// to the required buffer size.
    const char** (*getSearchPaths)(char* buf, size_t* bufSize);

    /// Unload all plug-in binaries.
    void (*unload)();
};

}

#endif // FnPluginManager_H

