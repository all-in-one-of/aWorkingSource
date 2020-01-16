// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDEROUTPUTUTILS_RENDEROUTPUTUTILS_H
#define FNRENDEROUTPUTUTILS_RENDEROUTPUTUTILS_H

#include "FnPluginSystem/FnPluginSystem.h"
#include "FnAttribute/FnAttribute.h"
#include "FnScenegraphIterator/FnScenegraphIterator.h"

// Support classes:
#include "FnRenderOutputUtils/XFormMatrix.h"
#include "FnRenderOutputUtils/ShadingNodeConnectionDescription.h"
#include "FnRenderOutputUtils/ShadingNodeDescription.h"
#include "FnRenderOutputUtils/ShadingNodeDescriptionMap.h"
#include "FnRenderOutputUtils/CameraInfo.h"
#include "FnRenderOutputUtils/FnRenderOutputUtilsAPI.h"

#include <string>
#include <vector>
#include <set>

namespace Foundry
{
namespace Katana
{
/**
 * \defgroup RenderOutputUtils RenderOutputUtils
 * \ingroup RenderAPI
 * @{
 *
 * @brief A collection of utility functions and classes for processing
 *        the Katana recipe.
 */
namespace RenderOutputUtils
{
    /**
     * \ingroup RenderAPI
     */
    enum ProceduralArgsType
    {
        kProceduralArgsType_Classic,
        kProceduralArgsType_ScenegraphAttr
    };

    /**
     * \ingroup RenderAPI
     * ProceduralOutputContextInfo
     */
    struct FNRENDEROUTPUTUTILS_API ProceduralOutputContextInfo
    {
        int   _frameNumber;
        float _shutterOpen;
        float _shutterClose;
        float _cropWindowXMin;
        float _cropWindowXMax;
        float _cropWindowYMin;
        float _cropWindowYMax;
        int   _xres;
        int   _yres;

        ProceduralOutputContextInfo()
        : _frameNumber(1)
        , _shutterOpen(0)
        , _shutterClose(0)
        , _cropWindowXMin(0)
        , _cropWindowXMax(1)
        , _cropWindowYMin(0)
        , _cropWindowYMax(1)
        , _xres(0)
        , _yres(0)
        {}
    };

    /**
     * \ingroup RenderAPI
     */
    FNRENDEROUTPUTUTILS_API std::string buildProceduralArgsString(FnScenegraphIterator sgIterator,
                                                               ProceduralArgsType type,
                                                               const std::string& argsAttrName,
                                                               const ProceduralOutputContextInfo& contextInfo);

    /**
     * \ingroup RenderAPI
     * flushProceduralDsoCaches
     */
    FNRENDEROUTPUTUTILS_API void flushProceduralDsoCaches(const std::string& apiName="");

    /**
     * \ingroup RenderAPI
     * getPixelAreaThroughCamera
     */
    FNRENDEROUTPUTUTILS_API int getPixelAreaThroughCamera(FnScenegraphIterator sgIterator,
                                                       const std::string& cameraName,
                                                       const std::string& resolution);

    /**
     * \ingroup RenderAPI
     * findSampleTimesRelevantToShutterRange
     */
    FNRENDEROUTPUTUTILS_API void findSampleTimesRelevantToShutterRange(std::vector<float>& sampleTimes,
                                                                    const std::set<float>& inputSamples,
                                                                    float shutterOpen,
                                                                    float shutterClose);

    /**
     * \ingroup RenderAPI
     * getRenderResolution
     */
    FNRENDEROUTPUTUTILS_API std::string getRenderResolution(FnScenegraphIterator rootIterator,
                                                         int *width,
                                                         int *height);

    /**
     * \ingroup RenderAPI
     * getCameraPath
     */
    FNRENDEROUTPUTUTILS_API std::string getCameraPath(FnScenegraphIterator rootIterator);

    typedef std::vector<float>                    TransformData;
    typedef std::pair<std::string, TransformData> TransformPair;
    typedef std::vector<TransformPair>            TransformList;

    /**
     * \ingroup RenderAPI
     * Transform
     */
    struct Transform
    {
        TransformList transformList;    /**< */
        std::vector<float> sampleTimes; /**< */
    };

    /**
     * \ingroup RenderAPI
     * fillXFormListForLocation
     */
    FNRENDEROUTPUTUTILS_API void fillXFormListForLocation(std::vector<Transform>& xFormList,
                                                       FnScenegraphIterator sgIterator,
                                                       float shutterClose = 0.0f);

    /**
     * \ingroup RenderAPI
     * fillXFormListFromAttributes
     */
    FNRENDEROUTPUTUTILS_API bool fillXFormListFromAttributes(std::vector<Transform>& xFormList,
                                                          const FnAttribute::GroupAttribute& xformAttr,
                                                          float shutterClose = 0.0f,
                                                          bool invertMatrix = true);

    /**
     * \ingroup RenderAPI
     * convertTexturesToArbitraryAttr
     */
    FNRENDEROUTPUTUTILS_API FnAttribute::GroupAttribute convertTexturesToArbitraryAttr(const FnAttribute::GroupAttribute& texturesAttr);


    FNRENDEROUTPUTUTILS_API std::string getArchString();
    FNRENDEROUTPUTUTILS_API std::string expandArchPath(const std::string& path);

    /**
     * \ingroup RenderAPI
     * getFlattenedMaterialAttr
     */
    FNRENDEROUTPUTUTILS_API FnAttribute::GroupAttribute getFlattenedMaterialAttr(FnScenegraphIterator sgIterator,
                                                                              const FnAttribute::StringAttribute& terminalNamesAttr);

    /**
     * \ingroup RenderAPI
     * emptyFlattenedMaterialCache
     */
    FNRENDEROUTPUTUTILS_API void emptyFlattenedMaterialCache();

    /**
     * \ingroup RenderAPI
     * getCameraInfo
     */
    FNRENDEROUTPUTUTILS_API CameraInfo getCameraInfo(FnScenegraphIterator sgIterator,
                                                  const std::string& cameraInfoPath);

    FNRENDEROUTPUTUTILS_API FnPlugStatus setHost(FnPluginHost* host);
    FNRENDEROUTPUTUTILS_API FnPluginHost *getHost();

    /**
     * \ingroup RenderAPI
     * @fn processLocation(FnScenegraphIterator sgIterator, const std::string &rendererName, const std::string &locationName,
     *                     void *optionalInput, void **optionalOutput)
     * Process a scene graph location for given a location type and renderer name.
     * Looks up a scene graph location delegate plug-in given a location type and renderer name.
     * If a plug-in is found the rendering/processing for this location is delegated to the plug-in
     * where it operates on the corresponding scene graph iterator.
     *
     * @param sgIterator: A scene graph iterator for the location to process.
     * @param rendererName: The name of the processor/renderer that should be used (can be empty to signify any-renderer).
     * @param locationName: The location type as retrieved from Foundry::Katana::SceneGraphIterator::getType()  (must not empty).
     * @param optionalInput: Optional input data for the plug-in.
     * @param optionalOutput: Optional pointer that will point to the result of the processing/rendering.
     * @return true if a plug-in is found, false otherwise.
     */
    FNRENDEROUTPUTUTILS_API bool processLocation(FnScenegraphIterator sgIterator,
                                              const std::string& rendererName,
                                              const std::string& locationName,
                                              void* optionalInput,
                                              void** optionalOutput);

    /**
     * \ingroup RenderAPI
     * @fn RenderOutputUtils::fillDelegateHandledLocationTypesList(std::vector<std::string>& delegateHandledLocationTypesList,
                                                                   const std::string& renderer = std::string())
     * Returns a vector containing the location types for which there are registered
     * ScenegraphLocationDelegate plugins. Specifying an empty string for the renderer
     * forces a search for location types with ScenegraphLocationDelegate plugins
     * for any renderer.
     *
     * @param delegateHandledLocationTypesList: a reference to a vector that will be filled with
     *                                          location type names; one for each location type
     *                                          that is handled by a registered delegate.
     * @param renderer: The renderer name, can be empty to denote any-renderer
     *
     */
    FNRENDEROUTPUTUTILS_API void fillDelegateHandledLocationTypesList(std::vector<std::string>& delegateHandledLocationTypesList,
                                                                   const std::string& renderer = std::string());

    /**
     * \ingroup RenderAPI
     * @fn RenderOutputUtils::buildTempRenderLocation(FnScenegraphIterator sgIterator,
                                                      const std::string& outputName,
                                                      const std::string& prefix,
                                                      const std::string& fileExtension,
                                                      const float frameTime)
     * Build a render location in the temporary directory for a render output.
     *
     * @param sgIterator The root scene graph iterator
     * @param outputName The name of the render output
     * @param prefix The filename prefix
     * @param fileExtension The filename extension
     * @param frameTime The current frame time
     * @return The built temporary location for the render output
     */
    FNRENDEROUTPUTUTILS_API std::string buildTempRenderLocation(FnScenegraphIterator sgIterator,
                                                             const std::string& outputName,
                                                             const std::string& prefix,
                                                             const std::string& fileExtension,
                                                             const float frameTime);

    /**
     * \ingroup RenderAPI
     * @fn RenderOutputUtils::buildTileLocation(FnScenegraphIterator sgIterator,
                                                const std::string& outputPath)
     * Build a tile location from the render settings for a given output path.
     *
     * @param sgIterator The root scene graph iterator
     * @param outputPath The output path for the built tile filename
     * @return The location to a tile location which consists of the output
     *         path and the built tile filename using the tile render
     *         settings
     */
    FNRENDEROUTPUTUTILS_API std::string buildTileLocation(FnScenegraphIterator sgIterator,
                                                       const std::string& outputPath);

    extern RenderOutputUtilsSuite_v3* _suite ;

}
/**
 * @}
 */
}
}

namespace FnKat = Foundry::Katana;

#endif
