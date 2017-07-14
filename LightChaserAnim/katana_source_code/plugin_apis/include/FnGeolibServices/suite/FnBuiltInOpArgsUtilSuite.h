#ifndef FnBuiltInOpArgsUtilSuite_H
#define FnBuiltInOpArgsUtilSuite_H

#include <FnAttribute/suite/FnAttributeSuite.h>

#include <stdint.h>

extern "C" {

typedef struct StaticSceneCreateOpArgsBuilderStruct *
        StaticSceneCreateOpArgsBuilderHandle;

typedef struct AttributeSetOpArgsBuilderStruct *
        AttributeSetOpArgsBuilderHandle;

#define FnBuiltInOpArgsUtilHostSuite_version 1

struct FnBuiltInOpArgsUtilHostSuite_v1
{

    // StaticSceneCreateOpArgsBuilder

    StaticSceneCreateOpArgsBuilderHandle (*staticSceneCreateOpArgsBuilderCtor)(
            uint8_t pathsAreAbsolute);

    void (*releaseStaticSceneCreateOpArgsBuilder)
            (StaticSceneCreateOpArgsBuilderHandle h);

    FnAttributeHandle (*staticSceneCreateOpArgsBuilderBuild)(
            StaticSceneCreateOpArgsBuilderHandle h);

    void (*staticSceneCreateOpArgsBuilderCreateEmptyLocation)(
            StaticSceneCreateOpArgsBuilderHandle h,
                    const char * locationPath, const char * locationType);

    void (*staticSceneCreateOpArgsBuilderSetAttrAtLocation)(
            StaticSceneCreateOpArgsBuilderHandle h,
                    const char * locationPath, const char * attrName,
                            FnAttributeHandle attrHandle);

    void (*staticSceneCreateOpArgsBuilderSetAttrsAtLeafStateForLocation)(
            StaticSceneCreateOpArgsBuilderHandle h,
                    const char * locationPath, uint8_t state);

    void (*staticSceneCreateOpArgsBuilderAddSubOpAtLocation)(
            StaticSceneCreateOpArgsBuilderHandle h,
                    const char * locationPath, const char * opType,
                            FnAttributeHandle opArgsHandle);

    void (*staticSceneCreateOpArgsBuilderSkipLocalActionsIfInputExistsAtLocation)(
            StaticSceneCreateOpArgsBuilderHandle h,
                    const char * locationPath, uint8_t state);


    // AttributeSetOpArgsBuilder

    AttributeSetOpArgsBuilderHandle (*attributeSetOpArgsBuilderCtor)();

    void (*releaseAttributeSetOpArgsBuilder)(
            AttributeSetOpArgsBuilderHandle h);

    FnAttributeHandle (*attributeSetOpArgsBuilderBuild)(
            AttributeSetOpArgsBuilderHandle h);

    void (*attributeSetOpArgsBuilderSetLocationPaths)(
            AttributeSetOpArgsBuilderHandle h,
            FnAttributeHandle locationPathsAttrHandle, const char * batch);

    void (*attributeSetOpArgsBuilderSetCEL)(
            AttributeSetOpArgsBuilderHandle h,
            FnAttributeHandle celAttrHandle, const char * batch);

    void (*attributeSetOpArgsBuilderSetAttr)(
            AttributeSetOpArgsBuilderHandle h,
            const char * attrName,  FnAttributeHandle setAttrHandle,
            const char * batch,
            const bool inherit);

    void (*attributeSetOpArgsBuilderDeleteAttr)(
            AttributeSetOpArgsBuilderHandle h,
            const char * deleteAttr, const char * batch);

    void (*attributeSetOpArgsBuilderAddSubOp)(
            AttributeSetOpArgsBuilderHandle h,
            const char * opType, FnAttributeHandle opArgsAttrHandle,
            const char * batch);

};

}


#endif // FnLookFileSuite_H
