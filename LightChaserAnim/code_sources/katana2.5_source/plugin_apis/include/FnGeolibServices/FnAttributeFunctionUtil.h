// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnGeolibServicesAttributeFunctionUtil_H
#define FnGeolibServicesAttributeFunctionUtil_H

#include <stdint.h>
#include <string>

#include <FnAttribute/FnAttribute.h>

#include <FnGeolibServices/FnGeolibServicesAPI.h>
#include <FnGeolibServices/ns.h>
#include <FnGeolibServices/suite/FnAttributeFunctionUtilSuite.h>

#include <FnPluginSystem/FnPluginSystem.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{

    class FNGEOLIBSERVICES_API FnAttributeFunctionUtil
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
