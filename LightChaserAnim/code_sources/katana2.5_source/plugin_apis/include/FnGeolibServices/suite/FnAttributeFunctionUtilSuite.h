// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnAttributeFunctionUtilSuite_H
#define FnAttributeFunctionUtilSuite_H

#include <stdint.h>

#include <FnAttribute/suite/FnAttributeSuite.h>

extern "C" {

#define FnAttributeFunctionUtilHostSuite_version 1

/** @brief AttributeFunctionUtil host suite
*/
struct FnAttributeFunctionUtilHostSuite_v1
{
    FnAttributeHandle (*run)(
            const char * fncName, FnAttributeHandle args);
    
    FnAttributeHandle (*getRegisteredFunctionNames)();
    void (*flushCache)();

};

}
#endif // FnAttributeFunctionUtilSuite_H
