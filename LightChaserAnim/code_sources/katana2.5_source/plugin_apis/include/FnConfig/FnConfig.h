// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnConfig_H
#define FnConfig_H

#include <string>

#include <FnConfig/FnConfigAPI.h>
#include <FnConfig/ns.h>
#include <FnConfig/suite/FnConfigSuite.h>

#include <FnPluginSystem/FnPluginSystem.h>

FNCONFIG_NAMESPACE_ENTER
{
    class FNCONFIG_API Config
    {
    public:

        virtual ~Config();

        // Returns true if there is an entry for the given key.
        static bool has(const std::string& key);

        // Returns the value of a configuration entry with the given key. If no
        // entry exists for the given key an empty string will be returned.
        static const std::string get(const std::string& key);

        static const FnConfigHostSuite_v1* getSuite();
        static FnPluginStatus setHost(FnPluginHost* host);

    protected:

        Config();

        static const FnConfigHostSuite_v1 * _configSuite;
    };
}
FNCONFIG_NAMESPACE_EXIT

#endif // FnConfig_H
