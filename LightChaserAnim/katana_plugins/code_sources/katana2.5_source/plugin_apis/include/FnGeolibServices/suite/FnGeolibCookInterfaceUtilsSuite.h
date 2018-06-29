// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnGeolibCookInterfaceUtilsSuite_H
#define FnGeolibCookInterfaceUtilsSuite_H

#include <stdint.h>

#include <FnAttribute/suite/FnAttributeSuite.h>
#include <FnGeolib/suite/FnGeolibCookInterfaceSuite.h>

extern "C" {

struct FnGeolibCookInterfaceUtilsMatchesCELInfo
{
    uint8_t matches;
    uint8_t canMatchChildren;
};

struct FnGeolibCookInterfaceUtilsFnMatchInfo
{
    uint8_t matches;
    uint8_t canMatchChildren;
};

struct FnGeolibCookInterfaceUtilsBuildLocalsAndGlobalsInfo
{
    FnAttributeHandle locals;
    FnAttributeHandle globals;
};

enum
{
    kFnGeolibGlobalAttributeProviderStatus_NoError = 0,
    kFnGeolibGlobalAttributeProviderStatus_QueryAbort,
    kFnGeolibGlobalAttributeProviderStatus_InvalidLocation,
};
typedef int32_t FnGeolibGlobalAttributeProviderStatus;

/**
 * @typedef FnGeolibGlobalAttributeProviderProc
 * Callback for getGlobalAttr() to request attribute values in order to compute
 * a global attribute.
 *
 * The callback function will be invoked multiple times at different query
 * locations, and needs to:
 * - return Status_QueryAbort if the CookInterface aborted the query
 * - return Status_InvalidLocation if the query location not exist is invalid.
 *   This serves as an indication to the getGlobalAttr() implementation that it
 *   should stop querying at this location, or any locations shallower than the
 *   current location.
 * - otherwise, return the requested attribute (if any) in \c
 *   attributeHandlePtr, and return Status_NoError.
 *
 * The location to query is determined by the values of \c leafLocationPath and
 * of \c relativeLocationDepth. If \c relativeLocationDepth is 0, the callback
 * should query at leafLocationPath directly, if 1 its parent, if 2 its
 * grandparent, and so on.
 */
typedef FnGeolibGlobalAttributeProviderStatus (
        *FnGeolibGlobalAttributeProviderProc)(
        void *context,
        const char *attrPath,
        int32_t attrPathLen,
        const char *leafLocationPath,
        int32_t leafLocationPathLen,
        int32_t relativeLocationDepth,
        FnAttributeHandle *attributeHandlePtr);

#define kFnGeolibGlobalAttributeProviderDefault \
    ((FnGeolibGlobalAttributeProviderProc)-1)

// A pointer to an instance of this structure must be provided to getGlobalAttr
// when using kFnGeolibGlobalAttributeProviderDefault, with cookInterfaceHandle
// and cookInterfaceSuite set.
struct FnGeolibGlobalAttributeProviderContextDefault
{
    FnGeolibCookInterfaceHandle cookInterfaceHandle;
    struct FnGeolibCookInterfaceSuite_v1 *cookInterfaceSuite;
    int32_t inputIndex;
};

#define FnGeolibCookInterfaceUtilsHostSuite_version 2

/** @brief GeolibCookInterfaceUtils host suite
*/
struct FnGeolibCookInterfaceUtilsHostSuite_v1
{
    void (*matchesCEL)(struct FnGeolibCookInterfaceUtilsMatchesCELInfo *matchesCELInfo,
            FnGeolibCookInterfaceHandle cookInterfaceHandle,
            FnGeolibCookInterfaceSuite_v1 * cookInterfaceSuite,
            FnAttributeHandle celAttr,
            const char *inputLocationPath, int32_t stringlen,
            int32_t inputIndex,
            uint8_t *didAbort);


    FnAttributeHandle (*cookDaps)(
            FnGeolibCookInterfaceHandle cookInterfaceHandle,
            FnGeolibCookInterfaceSuite_v1 * cookInterfaceSuite,
            const char *attrRoot, int32_t attrRootLen,
            const char *inputLocationPath, int32_t stringlen,
            int32_t inputIndex,
            FnAttributeHandle cookOrderAttr,
            uint8_t *didAbort);

    void (*buildLocalsAndGlobals)(
            struct FnGeolibCookInterfaceUtilsBuildLocalsAndGlobalsInfo *info,
            FnGeolibCookInterfaceHandle cookInterfaceHandle,
            FnGeolibCookInterfaceSuite_v1 * cookInterfaceSuite,
            const char *attrRoot, int32_t attrRootLen,
            uint8_t *didAbort);

    FnAttributeHandle (*getDefaultDapCookOrder)();

    FnAttributeHandle (*resolveFileNamesWithinGroup)(
            FnGeolibCookInterfaceHandle cookInterfaceHandle,
            FnGeolibCookInterfaceSuite_v1 * cookInterfaceSuite,
            FnAttributeHandle inputGrp,
            FnAttributeHandle celAttr,
            int frameNumber,
            uint8_t *didAbort);

    FnAttributeHandle (*resolveFileNameInString)(
            FnGeolibCookInterfaceHandle cookInterfaceHandle,
            FnGeolibCookInterfaceSuite_v1 * cookInterfaceSuite,
            FnAttributeHandle inputString,
            int frameNumber,
            uint8_t *didAbort);
};

struct FnGeolibCookInterfaceUtilsHostSuite_v2
{
    void (*matchesCEL)(struct FnGeolibCookInterfaceUtilsMatchesCELInfo *matchesCELInfo,
            FnGeolibCookInterfaceHandle cookInterfaceHandle,
            FnGeolibCookInterfaceSuite_v1 * cookInterfaceSuite,
            FnAttributeHandle celAttr,
            const char *inputLocationPath, int32_t stringlen,
            int32_t inputIndex,
            uint8_t *didAbort);


    FnAttributeHandle (*cookDaps)(
            FnGeolibCookInterfaceHandle cookInterfaceHandle,
            FnGeolibCookInterfaceSuite_v1 * cookInterfaceSuite,
            const char *attrRoot, int32_t attrRootLen,
            const char *inputLocationPath, int32_t stringlen,
            int32_t inputIndex,
            FnAttributeHandle cookOrderAttr,
            uint8_t *didAbort);

    void (*buildLocalsAndGlobals)(
            struct FnGeolibCookInterfaceUtilsBuildLocalsAndGlobalsInfo *info,
            FnGeolibCookInterfaceHandle cookInterfaceHandle,
            FnGeolibCookInterfaceSuite_v1 * cookInterfaceSuite,
            const char *attrRoot, int32_t attrRootLen,
            uint8_t *didAbort);

    FnAttributeHandle (*getDefaultDapCookOrder)();

    FnAttributeHandle (*resolveFileNamesWithinGroup)(
            FnGeolibCookInterfaceHandle cookInterfaceHandle,
            FnGeolibCookInterfaceSuite_v1 * cookInterfaceSuite,
            FnAttributeHandle inputGrp,
            FnAttributeHandle celAttr,
            int frameNumber,
            uint8_t *didAbort);

    FnAttributeHandle (*resolveFileNameInString)(
            FnGeolibCookInterfaceHandle cookInterfaceHandle,
            FnGeolibCookInterfaceSuite_v1 * cookInterfaceSuite,
            FnAttributeHandle inputString,
            int frameNumber,
            uint8_t *didAbort);

    FnAttributeHandle (*getGlobalAttrGeneric)(
            FnGeolibGlobalAttributeProviderProc attributeProvider,
            void *attributeProviderContext,
            const char *attrPath,
            int32_t attrPathLen,
            const char *locationPath,
            int32_t locationPathLen,
            uint8_t *didAbort);

    FnGeolibCookInterfaceUtilsFnMatchInfo (*FnMatch)(
            const char* testpath,
            int testpathLen,
            const char* pattern,
            int patternLen);
};
}

#endif // FnGeolibCookInterfaceUtilsSuite_H
