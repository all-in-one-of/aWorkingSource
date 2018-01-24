// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnGeolibServicesAttributeFunctionUtil_H
#define FnGeolibServicesAttributeFunctionUtil_H

#include <string>
#include <inttypes.h>

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnAttribute/FnAttribute.h>

#include <FnGeolibServices/suite/FnAttributeFunctionUtilSuite.h>

#include "ns.h"

FNGEOLIBSERVICES_NAMESPACE_ENTER
{

    class FnAttributeFunctionUtil
    {
    public:
        static FnAttribute::Attribute run(
                const std::string & fncName, FnAttribute::Attribute args);
        
        static void getRegisteredFunctionNames(std::vector<std::string> & names);
        static void flushCache();

    private:
        FnAttributeFunctionUtil();

        static const FnAttributeFunctionUtilHostSuite_v1 *_getSuite();
    };
}
FNGEOLIBSERVICES_NAMESPACE_EXIT

#endif // FnGeolibServicesAttributeFunctionUtil_H
