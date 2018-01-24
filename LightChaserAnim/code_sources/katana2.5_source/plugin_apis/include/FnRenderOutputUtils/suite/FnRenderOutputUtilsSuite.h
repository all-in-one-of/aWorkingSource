// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDEROUTPUTUTILS_RENDEROUTPUTUTILSSUITE_H
#define FNRENDEROUTPUTUTILS_RENDEROUTPUTUTILSSUITE_H

#include <FnAttribute/suite/FnAttributeSuite.h>
#include <FnScenegraphIterator/suite/FnScenegraphIteratorSuite.h>
#include <FnRender/suite/FnRenderPluginSuite.h>

#include <stdlib.h>
#include <stdint.h>

extern "C"
{
    typedef struct CameraInfoStruct*                        CameraInfoHandle;

    typedef struct ShadingNodeDescriptionStruct*            ShadingNodeDescriptionHandle;
    typedef struct ShadingNodeConnectionDescriptionStruct*  ShadingNodeConnectionDescriptionHandle;
    typedef struct ShadingNodeDescriptionMapStruct*         FnShadingNodeDescriptionMapHandle;

    typedef enum
    {
        kFnKatProceduralArgsClassic         = 1,
        kFnKatProceduralArgsScenegraphAttr  = 2,
    } FnKatProceduralArgsType;

    #define kFnKatArbAttrScopePrimitive     "primitive"
    #define kFnKatArbAttrScopeFace          "face"
    #define kFnKatArbAttrScopePoint         "point"
    #define kFnKatArbAttrScopeVertex        "vertex"

    #define kFnKatArbAttrBaseTypeFloat      "float"
    #define kFnKatArbAttrBaseTypeDouble     "double"
    #define kFnKatArbAttrBaseTypeInt        "int"
    #define kFnKatArbAttrBaseTypeString     "string"

    #define kFnKatArbAttrTypeFloat          "float"
    #define kFnKatArbAttrTypeDouble         "double"
    #define kFnKatArbAttrTypeInt            "int"
    #define kFnKatArbAttrTypeString         "string"
    #define kFnKatArbAttrTypeColor3         "color3"
    #define kFnKatArbAttrTypeColor4         "color4"
    #define kFnKatArbAttrTypeNormal2        "normal2"
    #define kFnKatArbAttrTypeNormal3        "normal3"
    #define kFnKatArbAttrTypeVector2        "vector2"
    #define kFnKatArbAttrTypeVector3        "vector3"
    #define kFnKatArbAttrTypeVector4        "vector4"
    #define kFnKatArbAttrTypePoint2         "point2"
    #define kFnKatArbAttrTypePoint3         "point3"
    #define kFnKatArbAttrTypePoint4         "point4"
    #define kFnKatArbAttrTypeMatrix9        "matrix9"
    #define kFnKatArbAttrTypeMatrix16       "matrix16"
    #define kFnKatArbAttrTypeUInt           "uint"
    #define kFnKatArbAttrTypeBool           "bool"
    #define kFnKatArbAttrTypeByte           "byte"

    #define RenderOutputUtilsSuite_version 3

    struct RenderOutputUtilsSuite_v2
    {
        FnAttributeHandle   (*buildProceduralArgsString)(
                                FnSgIteratorHandle sgIteratorHandle,
                                FnKatProceduralArgsType type,
                                const char *argsAttrName,
                                int frameNumber,
                                float shutterOpen,
                                float shutterClose,
                                float cropWindowXMin,
                                float cropWindowXMax,
                                float cropWindowYMin,
                                float cropWindowYMax,
                                int xres,
                                int yres);

        void                (*flushProceduralDsoCaches)();

        int                 (*getPixelAreaThroughCamera)(
                                FnSgIteratorHandle sgIteratorHandle,
                                const char* cameraLocation,
                                const char* resolution);

        FnAttributeHandle   (*findSampleTimesRelevantToShutterRange)(
                                const float *inputSamples,
                                unsigned int inputSamplesCount,
                                float shutterOpen,
                                float shutterClose);

        FnAttributeHandle   (*getRenderResolution)(
                                FnSgIteratorHandle rootIterator,
                                int *width,
                                int *height);

        FnAttributeHandle   (*getCameraPath)(
                                FnSgIteratorHandle rootIterator);

        // Textures
        FnAttributeHandle   (*convertTexturesToArbitraryAttr)(
                                FnAttributeHandle texturesAttr);

        // Architecture
        FnAttributeHandle   (*getArchString)();
        FnAttributeHandle   (*expandArchPath)(
                                const char *path);

        // ShadingNetworks
        FnAttributeHandle   (*getFlattenedMaterialAttr)(
                                FnSgIteratorHandle sgIteratorHandle,
                                FnAttributeHandle terminalNamesAttr);

        void                (*emptyFlattenedMaterialCache)();

        FnShadingNodeDescriptionMapHandle (*getShadingNetworkNodes)(
                                FnAttributeHandle materialAttr);

        FnShadingNodeDescriptionMapHandle (*getShadingNetworkNodesFromSgIterator)(
                                FnSgIteratorHandle sgIteratorHandle);

        void                (*destroyShadingNodeDescriptionMap)(
                                FnShadingNodeDescriptionMapHandle handle);

        void                (*retainShadingNodeDescription)(
                                ShadingNodeDescriptionHandle handle);

        void                (*releaseShadingNodeDescription)(
                                ShadingNodeDescriptionHandle handle);

        void                (*retainShadingNodeConnectionDescription)(
                                ShadingNodeConnectionDescriptionHandle handle);

        void                (*releaseShadingNodeConnectionDescription)(
                                ShadingNodeConnectionDescriptionHandle handle);

        ShadingNodeDescriptionHandle (*getShadingNodeDescriptionByName)(
                                FnShadingNodeDescriptionMapHandle handle,
                                const char *name);

        const char *        (*getShadingNodeDescriptionName)(
                                ShadingNodeDescriptionHandle handle);

        const char *        (*getShadingNodeDescriptionType)(
                                ShadingNodeDescriptionHandle handle);

        unsigned int        (*getShadingNodeDescriptionNumberOfParameterNames)(
                                ShadingNodeDescriptionHandle handle);

        const char *        (*getShadingNodeDescriptionParameterName)(
                                ShadingNodeDescriptionHandle handle,
                                unsigned int index);

        FnAttributeHandle   (*getShadingNodeDescriptionParameter)(
                                ShadingNodeDescriptionHandle handle,
                                const char *name);

        unsigned int        (*getShadingNodeDescriptionNumberOfConnectionNames)(
                                ShadingNodeDescriptionHandle handle);
        const char *        (*getShadingNodeDescriptionConnectionName)(
                                ShadingNodeDescriptionHandle handle,
                                unsigned int index);

        ShadingNodeConnectionDescriptionHandle (*getShadingNodeDescriptionConnection)(
                                ShadingNodeDescriptionHandle handle,
                                const char *name);

        bool                (*isShadingNodeDescriptionValid)(
                                ShadingNodeDescriptionHandle handle);

        const char *        (*getShadingNodeConnectionDescriptionName)(
                                ShadingNodeConnectionDescriptionHandle handle);

        const char *        (*getShadingNodeConnectionDescriptionConnectedNodeName)(
                                ShadingNodeConnectionDescriptionHandle handle);

        const char *        (*getShadingNodeConnectionDescriptionConnectedPortName)(
                                ShadingNodeConnectionDescriptionHandle handle);

        bool                (*isShadingNodeConnectionDescriptionValid)(
                                ShadingNodeConnectionDescriptionHandle handle);

        // CameraInfo object
        CameraInfoHandle    (*getCameraInfo)(
                                FnSgIteratorHandle rootSgIteratorHandle,
                                const char* cameraInfoPath);

        void                (*retainCameraInfo)(
                                CameraInfoHandle cameraInfoHandle);

        void                (*releaseCameraInfo)(
                                CameraInfoHandle cameraInfoHandle);

        float               (*getCameraInfoFov)(
                                CameraInfoHandle cameraInfoHandle);

        float               (*getCameraInfoNear)(
                                CameraInfoHandle cameraInfoHandle);

        float               (*getCameraInfoFar)(
                                CameraInfoHandle cameraInfoHandle);

        float               (*getCameraInfoLeft)(
                                CameraInfoHandle cameraInfoHandle);

        float               (*getCameraInfoRight)(
                                CameraInfoHandle cameraInfoHandle);

        float               (*getCameraInfoTop)(
                                CameraInfoHandle cameraInfoHandle);

        float               (*getCameraInfoBottom)(
                                CameraInfoHandle cameraInfoHandle);

        int                 (*getCameraInfoOrtho)(
                                CameraInfoHandle cameraInfoHandle);

        float               (*getCameraInfoOrthoWidth)(
                                CameraInfoHandle cameraInfoHandle);

        double *            (*getCameraInfoXForm)(
                                CameraInfoHandle cameraInfoHandle);

        bool                (*processLocation)(
                                FnSgIteratorHandle sgIteratorHandle,
                                const char* rendererName,
                                const char* locationName,
                                void* optionalInput,
                                void** optionalOutput);

        FnAttributeHandle   (*fillDelegateHandledLocationTypesList)(
                                const char* rendererName);

        FnAttributeHandle   (*buildTempRenderLocation)(
                                FnSgIteratorHandle sgIteratorHandle,
                                const char* outputName,
                                const char* prefix,
                                const char* fileExtension,
                                float frameTime);

        FnAttributeHandle   (*buildTileLocation)(
                                FnSgIteratorHandle sgIteratorHandle,
                                const char* outputPath);

    };

    struct RenderOutputUtilsSuite_v3
        {
            FnAttributeHandle   (*buildProceduralArgsString)(
                                    FnSgIteratorHandle sgIteratorHandle,
                                    FnKatProceduralArgsType type,
                                    const char *argsAttrName,
                                    int frameNumber,
                                    float shutterOpen,
                                    float shutterClose,
                                    float cropWindowXMin,
                                    float cropWindowXMax,
                                    float cropWindowYMin,
                                    float cropWindowYMax,
                                    int xres,
                                    int yres);

            void                (*flushProceduralDsoCaches)(const char* apiName);

            int                 (*getPixelAreaThroughCamera)(
                                    FnSgIteratorHandle sgIteratorHandle,
                                    const char* cameraLocation,
                                    const char* resolution);

            FnAttributeHandle   (*findSampleTimesRelevantToShutterRange)(
                                    const float *inputSamples,
                                    unsigned int inputSamplesCount,
                                    float shutterOpen,
                                    float shutterClose);

            FnAttributeHandle   (*getRenderResolution)(
                                    FnSgIteratorHandle rootIterator,
                                    int *width,
                                    int *height);

            FnAttributeHandle   (*getCameraPath)(
                                    FnSgIteratorHandle rootIterator);

            // Textures
            FnAttributeHandle   (*convertTexturesToArbitraryAttr)(
                                    FnAttributeHandle texturesAttr);

            // Architecture
            FnAttributeHandle   (*getArchString)();
            FnAttributeHandle   (*expandArchPath)(
                                    const char *path);

            // ShadingNetworks
            FnAttributeHandle   (*getFlattenedMaterialAttr)(
                                    FnSgIteratorHandle sgIteratorHandle,
                                    FnAttributeHandle terminalNamesAttr);

            void                (*emptyFlattenedMaterialCache)();

            FnShadingNodeDescriptionMapHandle (*getShadingNetworkNodes)(
                                    FnAttributeHandle materialAttr);

            FnShadingNodeDescriptionMapHandle (*getShadingNetworkNodesFromSgIterator)(
                                    FnSgIteratorHandle sgIteratorHandle);

            void                (*destroyShadingNodeDescriptionMap)(
                                    FnShadingNodeDescriptionMapHandle handle);

            void                (*retainShadingNodeDescription)(
                                    ShadingNodeDescriptionHandle handle);

            void                (*releaseShadingNodeDescription)(
                                    ShadingNodeDescriptionHandle handle);

            void                (*retainShadingNodeConnectionDescription)(
                                    ShadingNodeConnectionDescriptionHandle handle);

            void                (*releaseShadingNodeConnectionDescription)(
                                    ShadingNodeConnectionDescriptionHandle handle);

            ShadingNodeDescriptionHandle (*getShadingNodeDescriptionByName)(
                                    FnShadingNodeDescriptionMapHandle handle,
                                    const char *name);

            const char *        (*getShadingNodeDescriptionName)(
                                    ShadingNodeDescriptionHandle handle);

            const char *        (*getShadingNodeDescriptionType)(
                                    ShadingNodeDescriptionHandle handle);

            unsigned int        (*getShadingNodeDescriptionNumberOfParameterNames)(
                                    ShadingNodeDescriptionHandle handle);

            const char *        (*getShadingNodeDescriptionParameterName)(
                                    ShadingNodeDescriptionHandle handle,
                                    unsigned int index);

            FnAttributeHandle   (*getShadingNodeDescriptionParameter)(
                                    ShadingNodeDescriptionHandle handle,
                                    const char *name);

            unsigned int        (*getShadingNodeDescriptionNumberOfConnectionNames)(
                                    ShadingNodeDescriptionHandle handle);
            const char *        (*getShadingNodeDescriptionConnectionName)(
                                    ShadingNodeDescriptionHandle handle,
                                    unsigned int index);

            ShadingNodeConnectionDescriptionHandle (*getShadingNodeDescriptionConnection)(
                                    ShadingNodeDescriptionHandle handle,
                                    const char *name);

            bool                (*isShadingNodeDescriptionValid)(
                                    ShadingNodeDescriptionHandle handle);

            const char *        (*getShadingNodeConnectionDescriptionName)(
                                    ShadingNodeConnectionDescriptionHandle handle);

            const char *        (*getShadingNodeConnectionDescriptionConnectedNodeName)(
                                    ShadingNodeConnectionDescriptionHandle handle);

            const char *        (*getShadingNodeConnectionDescriptionConnectedPortName)(
                                    ShadingNodeConnectionDescriptionHandle handle);

            bool                (*isShadingNodeConnectionDescriptionValid)(
                                    ShadingNodeConnectionDescriptionHandle handle);

            // CameraInfo object
            CameraInfoHandle    (*getCameraInfo)(
                                    FnSgIteratorHandle rootSgIteratorHandle,
                                    const char* cameraInfoPath);

            void                (*retainCameraInfo)(
                                    CameraInfoHandle cameraInfoHandle);

            void                (*releaseCameraInfo)(
                                    CameraInfoHandle cameraInfoHandle);

            float               (*getCameraInfoFov)(
                                    CameraInfoHandle cameraInfoHandle);

            float               (*getCameraInfoNear)(
                                    CameraInfoHandle cameraInfoHandle);

            float               (*getCameraInfoFar)(
                                    CameraInfoHandle cameraInfoHandle);

            float               (*getCameraInfoLeft)(
                                    CameraInfoHandle cameraInfoHandle);

            float               (*getCameraInfoRight)(
                                    CameraInfoHandle cameraInfoHandle);

            float               (*getCameraInfoTop)(
                                    CameraInfoHandle cameraInfoHandle);

            float               (*getCameraInfoBottom)(
                                    CameraInfoHandle cameraInfoHandle);

            int                 (*getCameraInfoOrtho)(
                                    CameraInfoHandle cameraInfoHandle);

            float               (*getCameraInfoOrthoWidth)(
                                    CameraInfoHandle cameraInfoHandle);

            double *            (*getCameraInfoXForm)(
                                    CameraInfoHandle cameraInfoHandle);

            bool                (*processLocation)(
                                    FnSgIteratorHandle sgIteratorHandle,
                                    const char* rendererName,
                                    const char* locationName,
                                    void* optionalInput,
                                    void** optionalOutput);

            FnAttributeHandle   (*fillDelegateHandledLocationTypesList)(
                                    const char* rendererName);

            FnAttributeHandle   (*buildTempRenderLocation)(
                                    FnSgIteratorHandle sgIteratorHandle,
                                    const char* outputName,
                                    const char* prefix,
                                    const char* fileExtension,
                                    float frameTime);

            FnAttributeHandle   (*buildTileLocation)(
                                    FnSgIteratorHandle sgIteratorHandle,
                                    const char* outputPath);

        };

} // external "C"

#endif
