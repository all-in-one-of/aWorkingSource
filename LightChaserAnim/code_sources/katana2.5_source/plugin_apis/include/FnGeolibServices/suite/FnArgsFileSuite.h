// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnArgsFileSuite_H
#define FnArgsFileSuite_H

#include <stdint.h>

#include <FnAttribute/suite/FnAttributeSuite.h>

extern "C" {

#define FnArgsFileHostSuite_version 1

/**
 * The name of the "top-level" container, to which widget hints such as "help"
 * can be attached.
 */
#define kFnArgsFileTopLevelContainerName "__topLevelContainer"

/** @brief ArgsFile host suite
*/
struct FnArgsFileHostSuite_v1
{
    FnAttributeHandle (*parseArgsFile)(const char * filePath);
    void (*flushCache)();


    FnAttributeHandle (*getProceduralArgs)(const char * asset);






};

}
#endif // FnArgsFileSuite_H
