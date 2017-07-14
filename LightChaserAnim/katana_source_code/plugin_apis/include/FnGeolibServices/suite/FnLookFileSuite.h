// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnLookFileSuite_H
#define FnLookFileSuite_H

#include <stdint.h>

#include <FnAttribute/suite/FnAttributeSuite.h>

extern "C" {

typedef struct FnLookFileStruct * FnLookFileHandle;
typedef struct FnLookFileAttrMapStruct * FnLookFileAttrMapHandle;

#define FnLookFileHostSuite_version 2

/** @brief LookFile host suite
*/
struct FnLookFileHostSuite_v1
{
    FnLookFileHandle (*getLookFile)(const char *filePath, const char *passName);
    void (*releaseLookFile)(FnLookFileHandle h);

    FnLookFileAttrMapHandle (*getLookFileMaterials)(FnLookFileHandle h);
    FnAttributeHandle (*getLookFileMaterialType)(FnLookFileHandle h,
            const char *materialLocationPath);
    FnAttributeHandle (*getLookFileMaterial)(FnLookFileHandle h,
            const char *locationPath, uint8_t global);

    FnAttributeHandle (*getLookFileRootIdNames)(FnLookFileHandle h);
    FnAttributeHandle (*getLookFileRootIdType)(FnLookFileHandle h,
            const char *rootId);
    FnAttributeHandle (*getLookFilePathsWithOverrides)(FnLookFileHandle h,
            const char *rootId);
    FnLookFileAttrMapHandle (*getLookFileAttrs)(FnLookFileHandle h,
            const char * locationPath, const char *rootId);
    FnLookFileAttrMapHandle (*getLookFileRootOverrides)(FnLookFileHandle h);

    FnAttributeHandle (*getAttrMapValue)(FnLookFileAttrMapHandle h,
            const char *key);
    FnAttributeHandle (*getAttrMapKeys)(FnLookFileAttrMapHandle h);
    void (*releaseLookFileAttrMap)(FnLookFileAttrMapHandle h);
    void (*flushCache)();
};

struct FnLookFileHostSuite_v2
{
    FnLookFileHandle (*getLookFile)(const char *filePath, const char *passName);
    void (*releaseLookFile)(FnLookFileHandle h);

    FnLookFileAttrMapHandle (*getLookFileMaterials)(FnLookFileHandle h);
    FnAttributeHandle (*getLookFileMaterialType)(FnLookFileHandle h,
            const char *materialLocationPath);
    FnAttributeHandle (*getLookFileMaterial)(FnLookFileHandle h,
            const char *locationPath, uint8_t global);

    FnAttributeHandle (*getLookFileRootIdNames)(FnLookFileHandle h);
    FnAttributeHandle (*getLookFileRootIdType)(FnLookFileHandle h,
            const char *rootId);
    FnAttributeHandle (*getLookFilePathsWithOverrides)(FnLookFileHandle h,
            const char *rootId);
    FnLookFileAttrMapHandle (*getLookFileAttrs)(FnLookFileHandle h,
            const char * locationPath, const char *rootId);
    FnLookFileAttrMapHandle (*getLookFileRootOverrides)(FnLookFileHandle h);

    FnAttributeHandle (*getAttrMapValue)(FnLookFileAttrMapHandle h,
            const char *key);
    FnAttributeHandle (*getAttrMapKeys)(FnLookFileAttrMapHandle h);
    void (*releaseLookFileAttrMap)(FnLookFileAttrMapHandle h);

    FnAttributeHandle (*getPassNamesForLookFileAsset)(const char *asset);
    FnAttributeHandle (*getSafePath)(const char *asset, uint8_t includeVersion);

    void (*flushCache)();
};

}
#endif // FnLookFileSuite_H
