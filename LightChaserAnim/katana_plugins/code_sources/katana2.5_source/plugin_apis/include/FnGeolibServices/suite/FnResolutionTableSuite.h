// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnResolutionTableSuite_H
#define FnResolutionTableSuite_H

#include <stdint.h>

#include <FnAttribute/suite/FnAttributeSuite.h>

extern "C" {

#define FnResolutionTableHostSuite_version 1

typedef struct FnResolutionTableEntryStruct * FnResolutionTableEntryHandle;
typedef struct FnResolutionTableEntryListStruct * FnResolutionTableEntryListHandle;

/** @brief ResolutionTable host suite - read-only access to default resolution table
*/
struct FnResolutionTableHostSuite_v1
{
    // ResolutionTable
    uint8_t (*hasResolution)(const char *name);
    FnResolutionTableEntryHandle (*getResolution)(const char *name);
    FnResolutionTableEntryHandle (*findResolution)(const int32_t xres,
            const int32_t yres, const float aspectRatio);
    FnAttributeHandle (*extractValidResolutionName)(const char *str);
    FnAttributeHandle (*getGroups)();
    FnResolutionTableEntryListHandle (*getEntries)();
    FnResolutionTableEntryListHandle (*getEntriesForGroup)(const char *group);

    // ResolutionTableEntry
    const char * (*entryName)(FnResolutionTableEntryHandle handle);
    int32_t (*entryXres)(FnResolutionTableEntryHandle handle);
    int32_t (*entryYres)(FnResolutionTableEntryHandle handle);
    float (*entryAspectRatio)(FnResolutionTableEntryHandle handle);
    const char * (*entryFullName)(FnResolutionTableEntryHandle handle);
    const char * (*entryProxyName)(FnResolutionTableEntryHandle handle);
    const char * (*entryGroupName)(FnResolutionTableEntryHandle handle);
    void (*releaseEntry)(FnResolutionTableEntryHandle handle);

    // ResolutionTableEntryList
    int64_t (*entryListNumEntries)(FnResolutionTableEntryListHandle handle);
    FnResolutionTableEntryHandle (*entryListEntry)(
            FnResolutionTableEntryListHandle handle, int64_t index);
    void (*releaseEntryList)(FnResolutionTableEntryListHandle handle);
};

}
#endif // FnResolutionTableSuite_H
