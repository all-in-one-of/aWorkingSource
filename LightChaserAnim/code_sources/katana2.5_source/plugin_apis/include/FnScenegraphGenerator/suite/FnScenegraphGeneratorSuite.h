// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef FnScenegraphGeneratorSuite_H
#define FnScenegraphGeneratorSuite_H

extern "C" {

#include <FnAttribute/suite/FnAttributeSuite.h>

typedef struct FnScenegraphGeneratorStruct * FnScenegraphGeneratorHandle;

typedef struct FnSGContextStruct * FnSGContextHandle;

#define FnScenegraphGeneratorPluginSuite_version 1

struct FnScenegraphGeneratorPluginSuite_v1
{
    // DSO
    FnScenegraphGeneratorHandle (*create)();
    void (*destroy)(FnScenegraphGeneratorHandle handle);
    FnAttributeHandle (*getArgumentTemplate)();
    bool (*setArgs)(FnScenegraphGeneratorHandle handle, FnAttributeHandle args);
    bool (*checkArgs)(FnScenegraphGeneratorHandle handle, FnAttributeHandle args);
    FnSGContextHandle (*getRoot)(FnScenegraphGeneratorHandle handle);

    // Context
    FnSGContextHandle (*getFirstChild)(FnSGContextHandle handle);
    FnSGContextHandle (*getNextSibling)(FnSGContextHandle handle);
    const char **(*getLocalAttrNames)(FnSGContextHandle handle, int *nameCount);
    FnAttributeHandle (*getLocalAttr)(FnSGContextHandle handle, const char* name);
    void (*destroyContext)(FnSGContextHandle handle);
};

}
#endif // FnScenegraphGeneratorSuite_H
