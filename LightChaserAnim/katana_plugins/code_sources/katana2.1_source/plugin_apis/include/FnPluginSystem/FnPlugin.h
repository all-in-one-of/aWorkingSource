// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnPlugin_H
#define FnPlugin_H

#include <FnPluginSystem/FnPluginSystem.h>

#define REGISTER_PLUGIN(PLUGIN_CLASS, PLUGIN_NAME, PLUGIN_MAJOR_VERSION, PLUGIN_MINOR_VERSION) \
            PLUGIN_CLASS##_plugin.name = PLUGIN_NAME;                             \
            PLUGIN_CLASS##_plugin.apiName = PLUGIN_CLASS::_apiName;               \
            PLUGIN_CLASS##_plugin.apiVersion = PLUGIN_CLASS::_apiVersion;         \
            PLUGIN_CLASS##_plugin.pluginVersionMajor = PLUGIN_MAJOR_VERSION;      \
            PLUGIN_CLASS##_plugin.pluginVersionMinor = PLUGIN_MINOR_VERSION;      \
            PLUGIN_CLASS##_plugin.setHost = PLUGIN_CLASS::setHost;                \
            PLUGIN_CLASS##_plugin.getSuite = PLUGIN_CLASS##_getSuite;             \
            PLUGIN_CLASS##_plugin.flush = PLUGIN_CLASS::flush;                    \
            registerPlugin(PLUGIN_CLASS##_plugin);                                \

// Adds a plugin to the list
extern "C" void registerPlugin(const FnPlugin & plugin);

// Function that needs to be implemented by the user once per shared object.
// This function will add all the apropriate plugins to g_pluginsList
extern "C" void registerPlugins();


#endif
