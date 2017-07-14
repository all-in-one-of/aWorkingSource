// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnGeolibServicesArgsFile_H
#define FnGeolibServicesArgsFile_H

#include <string>
#include <inttypes.h>

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnAttribute/FnAttribute.h>

#include <FnGeolibServices/suite/FnArgsFileSuite.h>

#include "ns.h"

FNGEOLIBSERVICES_NAMESPACE_ENTER
{

    class FnArgsFile
    {
    public:
        static FnAttribute::Attribute parseArgsFile(
                const std::string & filePath);

        static void flushCache();

        static FnAttribute::Attribute getProceduralArgs(
                const std::string & assetId);

    private:
        FnArgsFile();

        static const FnArgsFileHostSuite_v1 *_getSuite();
    };
}
FNGEOLIBSERVICES_NAMESPACE_EXIT

#endif // FnGeolibServicesArgsFile_H
