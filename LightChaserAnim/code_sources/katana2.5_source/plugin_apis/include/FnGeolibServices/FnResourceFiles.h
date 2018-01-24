// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnGeolibServicesResourceFiles_H
#define FnGeolibServicesResourceFiles_H

#include <string>

#include <FnAttribute/FnAttribute.h>

#include <FnGeolibServices/FnGeolibServicesAPI.h>
#include <FnGeolibServices/ns.h>
#include <FnGeolibServices/suite/FnResourceFilesSuite.h>

#include <FnPluginSystem/FnPluginSystem.h>

FNGEOLIBSERVICES_NAMESPACE_ENTER
{
    class FNGEOLIBSERVICES_API FnResourceFiles
    {
    public:
        static void getSearchPaths(
                std::vector<std::string> & searchPaths,
                const std::string & subDir=std::string());

    private:
        FnResourceFiles();

        static const FnResourceFilesHostSuite_v1 *_getSuite();
    };
}
FNGEOLIBSERVICES_NAMESPACE_EXIT

#endif // FnGeolibServicesResourceFiles_H
