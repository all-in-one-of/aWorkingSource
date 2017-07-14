// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <FnPluginSystem/FnPlugin.h>
#include <iostream>
#include <vector>

#ifndef FnPluginAPI_h__
#define FnPluginAPI_h__
#ifdef _WIN32
    #define FnPluginAPI __declspec(dllexport)
#else
    #define FnPluginAPI __attribute__ ((visibility("default")))
#endif
#endif

namespace // anonymous
{
    std::vector<FnPlugin> g_pluginsListInLibrary;
}

// Adds a plugin to the list
void registerPlugin(const FnPlugin & plugin)
{
    g_pluginsListInLibrary.push_back(plugin);
}


// The only visible symbol - entry point of the plugin's shared lib
extern "C" FnPluginAPI FnPlugin*  getPlugins(unsigned int* numPlugins)
{
    // Call the user's plugin registering function
    g_pluginsListInLibrary.clear();

    registerPlugins();

    // Set the number of plugins
    *numPlugins = (unsigned int)g_pluginsListInLibrary.size();

    if(*numPlugins > 0)
    {
        // Convert the vector into an array of FnPlugin
        return (FnPlugin*)&g_pluginsListInLibrary[0];
    }
    else
    {
        return 0x0;
    }
}

