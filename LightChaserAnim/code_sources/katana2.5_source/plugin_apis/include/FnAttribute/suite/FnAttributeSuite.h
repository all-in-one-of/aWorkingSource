// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnAttributeSuite_H
#define FnAttributeSuite_H

#include <stdint.h>

extern "C" {

/** @brief Blind declaration of an attribute handle
*/
typedef struct FnAttributeStruct * FnAttributeHandle;

/** @brief Blind declaration of a group builder handle
*/
typedef struct FnGroupBuilderStruct * FnGroupBuilderHandle;

typedef int32_t FnKatAttributeType;
#define kFnKatAttributeTypeNull 0
#define kFnKatAttributeTypeInt 1
#define kFnKatAttributeTypeFloat 2
#define kFnKatAttributeTypeDouble 3
#define kFnKatAttributeTypeString 4
#define kFnKatAttributeTypeGroup 5
#define kFnKatAttributeTypeError -1

typedef int32_t BuilderModeType;
#define kFnKatGroupBuilderModeNormal 0
#define kFnKatGroupBuilderModeStrict 1

typedef int32_t BuilderBuildMode;
#define kFnKatGroupBuilderBuildAndFlush  0
#define kFnKatGroupBuilderBuildAndRetain 1

typedef int32_t FnKatStreamType;
#define kFnKatStreamTypeBinary 0
#define kFnKatStreamTypeXML 1

struct FnAttributeHash
{
    uint64_t hash1;
    uint64_t hash2;
};

/** @brief Pointer to function for writing 'size' bytes from 'buf' to 'stream'.
Returns the number of bytes actually written.
*/
typedef int64_t (*FnAttributeWriteStreamFunc)(void *stream, const void *buf, int64_t size);

/** @brief Pointer to function for reading 'size' bytes from 'stream' into 'buf'.
Returns the number of bytes actually read.
*/
typedef int64_t (*FnAttributeReadStreamFunc)(void *stream, void *buf, int64_t size);


#define FnAttributeHostSuite_version 2

/** @brief Attribute suite

This suite provides functions needed by a plugin for creating and querying
attributes.

Any FnAttributeHandle instance returned from a function in this suite will
already have a single reference.  It is the responsibility of the caller to
release the attribute by calling releaseAttr().
*/
struct FnAttributeHostSuite_v2
{
    // All Attrs
    // TODO remove streamType and auto determine it when we can
    FnAttributeHandle (*readAttributeStream)(void *stream,
        FnAttributeReadStreamFunc func,
        FnAttributeHandle *errorMessage,
        FnKatStreamType streamType);

    uint8_t (*writeAttributeStream)(FnAttributeHandle handle,
        void *stream, FnAttributeWriteStreamFunc func,
        FnAttributeHandle *errorMessage,
        FnKatStreamType streamType);

    FnKatAttributeType (*getType)(FnAttributeHandle handle);
    void (*retainAttr)(FnAttributeHandle handle);
    void (*releaseAttr)(FnAttributeHandle handle);
    uint8_t (*isEqual)(FnAttributeHandle handle, FnAttributeHandle otherHandle);
    FnAttributeHash (*getHash)(FnAttributeHandle handle);

    // DataAttrs
    int64_t (*getTupleSize)(FnAttributeHandle handle);
    int64_t (*getNumberOfValues)(FnAttributeHandle handle);
    int64_t (*getNumberOfTuples)(FnAttributeHandle handle);
    int64_t (*getNumberOfTimeSamples)(FnAttributeHandle handle);
    float (*getSampleTime)(FnAttributeHandle handle, int64_t index);
    uint8_t (*getBoundingSampleTimes)(FnAttributeHandle handle,
        float *ltime, float *rtime, float time);

    // NullAttr
    FnAttributeHandle (*createNullAttr)();

    // IntAttr
    FnAttributeHandle (*createIntAttr0)();
    FnAttributeHandle (*createIntAttr1)(int32_t value);
    FnAttributeHandle (*createIntAttr2)(const int32_t *values, int64_t valueCount,
        int64_t tupleSize);
    FnAttributeHandle (*createIntAttr3)(const float *times, int64_t timeCount,
        const int32_t **values, int64_t valueCount, int64_t tupleSize);
    const int32_t *(*getIntNearestSample)(FnAttributeHandle handle, float time,
        int64_t *valueCount);

    // FloatAttr
    FnAttributeHandle (*createFloatAttr0)();
    FnAttributeHandle (*createFloatAttr1)(float value);
    FnAttributeHandle (*createFloatAttr2)(const float *values, int64_t valueCount,
        int64_t tupleSize);
    FnAttributeHandle (*createFloatAttr3)(const float *times, int64_t timeCount,
        const float **values, int64_t valueCount, int64_t tupleSize);
    const float *(*getFloatNearestSample)(FnAttributeHandle handle, float time,
        int64_t *valueCount);

    // DoubleAttr
    FnAttributeHandle (*createDoubleAttr0)();
    FnAttributeHandle (*createDoubleAttr1)(double value);
    FnAttributeHandle (*createDoubleAttr2)(const double *values, int64_t valueCount,
        int64_t tupleSize);
    FnAttributeHandle (*createDoubleAttr3)(const float *times, int64_t timeCount,
        const double **values, int64_t valueCount, int64_t tupleSize);
    const double *(*getDoubleNearestSample)(FnAttributeHandle handle,
        float time, int64_t *valueCount);

    // StringAttr
    FnAttributeHandle (*createStringAttr0)();
    FnAttributeHandle (*createStringAttr1)(const char *value);
    FnAttributeHandle (*createStringAttr2)(const char **values, int64_t valueCount,
        int64_t tupleSize);
    FnAttributeHandle (*createStringAttr3)(const float *times, int64_t timeCount,
        const char ***values, int64_t valueCount, int64_t tupleSize);
    const char **(*getStringNearestSample)(FnAttributeHandle handle, float time,
        int64_t *valueCount);

    // GroupAttr
    FnAttributeHandle (*createGroupAttr)(const char **names,
        FnAttributeHandle *children, int64_t childCount, uint8_t groupInherit);
    int64_t (*getNumberOfChildren)(FnAttributeHandle handle);
    const char *(*getChildNameByIndex)(FnAttributeHandle, int64_t index,
        int32_t * nameSize);
    FnAttributeHandle (*getChildByIndex)(FnAttributeHandle handle,
        int64_t index);
    FnAttributeHandle (*getChildByName)(FnAttributeHandle handle,
        const char *name, int32_t stringlen);
    uint8_t (*getGroupInherit)(FnAttributeHandle handle);

    // GroupBuilder
    FnGroupBuilderHandle (*createGroupBuilder)();
    FnGroupBuilderHandle (*createGroupBuilder2)(BuilderModeType builderMode);
    void (*releaseGroupBuilder)(FnGroupBuilderHandle handle);
    void (*setGroupBuilder)(FnGroupBuilderHandle handle, const char *path,
        int32_t stringlen, FnAttributeHandle attr, uint8_t groupInherit);
    void (*setGroupBuilderUnique)(FnGroupBuilderHandle handle, const char *path,
        int32_t stringlen, FnAttributeHandle attr, uint8_t groupInherit);
    void (*delGroupBuilder)(FnGroupBuilderHandle handle, const char *path,
        int32_t stringlen);
    void (*updateGroupBuilder)(FnGroupBuilderHandle handle,
        FnAttributeHandle attr);
    void (*deepUpdateGroupBuilder)(FnGroupBuilderHandle handle,
        FnAttributeHandle attr);
    void (*reserveGroupBuilder)(FnGroupBuilderHandle handle, int64_t n);
    FnAttributeHandle (*buildGroupBuilder)(FnGroupBuilderHandle handle,
        BuilderBuildMode buildMode);
    void (*setGroupBuilderInherit)(FnGroupBuilderHandle handle,
        uint8_t groupInherit);
    void (*sortGroupBuilder)(FnGroupBuilderHandle handle);

    // Name Utils
    // Currently swaps '.' and '/' to non-ambiguous alternatives
    void (*delimiterEncode)(char * str, uint64_t len);
    void (*delimiterDecode)(char * str, uint64_t len);
};

}
#endif // FnAttributeSuite_H
