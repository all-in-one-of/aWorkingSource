// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnResourceFilesSuite_H
#define FnResourceFilesSuite_H

#include <stdint.h>

#include <FnAttribute/suite/FnAttributeSuite.h>

extern "C" {

#define FnResourceFilesHostSuite_version 1

/** @brief ResourceFiles host suite
*/
struct FnResourceFilesHostSuite_v1
{
    FnAttributeHandle (*getSearchPaths)(const char *subDir);
};

}

#endif // FnResourceFilesSuite_H
