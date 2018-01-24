// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef FnAttributeModifierSuite_H
#define FnAttributeModifierSuite_H

extern "C" {

typedef struct FnAttributeModifierStruct * FnAttributeModifierHandle;
typedef struct FnAttributeModifierHostStruct * FnAttributeModifierHostHandle;

#include <FnAttribute/suite/FnAttributeSuite.h>

#define FnAttributeModifierPluginSuite_version 1

struct FnAttributeModifierPluginSuite_v1
{
    // static
    FnAttributeModifierHandle (*create)(FnAttributeHandle args);
    FnAttributeHandle (*getArgumentTemplate)();

    // non-static
    void (*destroy)(FnAttributeModifierHandle handle);
    void (*cook)(FnAttributeModifierHandle handle,FnAttributeModifierHostHandle host);
    const char* (*getCacheId)(FnAttributeModifierHandle handle);
 };

#define FnAttributeModifierHostSuite_version 2

struct FnAttributeModifierHostSuite_v2
{
    // mirror basic query methods of a Katana GeometryProducer
    const char * (*getName)(FnAttributeModifierHostHandle handle);
    const char * (*getType)(FnAttributeModifierHostHandle handle);
    const char * (*getFullName)(FnAttributeModifierHostHandle handle);

    // basic host info
    float (*getFrameTime)(FnAttributeModifierHostHandle handle);
    const char * (*getHostString)(FnAttributeModifierHostHandle handle);
    int (*getNumSamples)(FnAttributeModifierHostHandle handle);
    float (*getShutterOpen)(FnAttributeModifierHostHandle handle);
    float (*getShutterClose)(FnAttributeModifierHostHandle handle);

    // basic attribute access
    FnAttributeHandle (*getAttribute)(FnAttributeModifierHostHandle handle, const char * name);
    const char ** (*getAttributeNames)(FnAttributeModifierHostHandle handle, unsigned int * nameCount);
    FnAttributeHandle (*getDelimitedLocalAttribute)(FnAttributeModifierHostHandle handle, const char * name);
    FnAttributeHandle (*getGlobalAttribute)(FnAttributeModifierHostHandle handle, const char * name);
    const char ** (*getGlobalAttributeNames)(FnAttributeModifierHostHandle handle, unsigned int * nameCount);
    FnAttributeHandle (*getDelimitedGlobalAttribute)(FnAttributeModifierHostHandle handle, const char * name);

    // attribute access at location
    FnAttributeHandle (*getAttributeAtLocation)(FnAttributeModifierHostHandle handle,
        const char * location, const char * name);
    FnAttributeHandle (*getDelimitedLocalAttributeAtLocation)(FnAttributeModifierHostHandle handle,
        const char * location, const char * name);
    FnAttributeHandle (*getGlobalAttributeAtLocation)(FnAttributeModifierHostHandle handle,
        const char * location, const char * name);
    FnAttributeHandle (*getDelimitedGlobalAttributeAtLocation)(FnAttributeModifierHostHandle handle,
        const char * location, const char * name);

    // child names
    const char **(*getChildNames)(FnAttributeModifierHostHandle handle,
        unsigned int *nameCount, const char * atLocation);

    // xforms
    FnAttributeHandle (*getWorldSpaceXform)(FnAttributeModifierHostHandle handle);
    FnAttributeHandle (*getWorldSpaceXformAtLocation)(FnAttributeModifierHostHandle handle,
        const char * location);
    FnAttributeHandle (*getMultiSampleWorldSpaceXform)(FnAttributeModifierHostHandle handle);
    FnAttributeHandle (*getMultiSampleWorldSpaceXformAtLocation)(FnAttributeModifierHostHandle handle,
        const char * location);
    FnAttributeHandle (*getWorldSpaceXformForSamples)(FnAttributeModifierHostHandle handle,
        const float *samples, unsigned int sampleCount);
    FnAttributeHandle (*getWorldSpaceXformAtLocationForSamples)(FnAttributeModifierHostHandle handle,
        const char * location, const float *samples, unsigned int sampleCount);

    // override
    void (*addOverride)(FnAttributeModifierHostHandle handle, const char * name, FnAttributeHandle attr,
        bool inheritGroup);
};


}
#endif // FnAttributeModifierSuite_H
