// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRenderOutputUtils/FnRenderOutputUtils.h"

#include "FnAttribute/FnDataBuilder.h"
#include "FnAttribute/FnGroupBuilder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <cstring>

namespace Foundry
{
namespace Katana
{
namespace RenderOutputUtils
{

static FnPluginHost* gHost = 0x0;
RenderOutputUtilsSuite_v3* _suite = 0x0;

bool processLocation(FnScenegraphIterator sgIterator,
                     const std::string& rendererName,
                     const std::string& locationName,
                     void* optionalInput,
                     void** optionalOutput)
{
    return _suite->processLocation(sgIterator.getHandle(), rendererName.c_str(), locationName.c_str(), optionalInput, optionalOutput);
}

void fillDelegateHandledLocationTypesList(std::vector<std::string>& delegateHandledLocationTypesList,
                                          const std::string& renderer /*= std::string()*/)
{
    delegateHandledLocationTypesList.clear();

    FnAttribute::StringAttribute result =
        FnAttribute::Attribute::CreateAndSteal(
            _suite->fillDelegateHandledLocationTypesList(renderer.c_str()));
    if (!result.isValid()) return;

    FnAttribute::StringConstVector resultValues = result.getNearestSample(0);

    delegateHandledLocationTypesList.insert(delegateHandledLocationTypesList.begin(),
                                            resultValues.begin(),
                                            resultValues.end());

}

std::string buildTempRenderLocation(FnScenegraphIterator sgIterator,
                                    const std::string& outputName,
                                    const std::string& prefix,
                                    const std::string& fileExtension,
                                    const float frameTime)
{
    FnAttribute::StringAttribute result =
        FnAttribute::Attribute::CreateAndSteal(
            _suite->buildTempRenderLocation(sgIterator.getHandle(),
                                                           outputName.c_str(),
                                                           prefix.c_str(),
                                                           fileExtension.c_str(),
                                                           frameTime));

    return result.getValue("", false);
}

std::string buildTileLocation(FnScenegraphIterator sgIterator,
                              const std::string& outputPath)
{
    FnAttribute::StringAttribute result =
        FnAttribute::Attribute::CreateAndSteal(
            _suite->buildTileLocation(sgIterator.getHandle(),
                                                     outputPath.c_str()));

    return result.getValue("", false);
}


std::string buildProceduralArgsString(FnScenegraphIterator sgIterator,
                                      ProceduralArgsType type,
                                      const std::string& argsAttrName,
                                      const ProceduralOutputContextInfo& contextInfo)
{
    // Convert 'type' from the  ProceduralArgsType into the suite int format
    FnKatProceduralArgsType argsType;

    switch (type)
    {
        case kProceduralArgsType_Classic:
            argsType = kFnKatProceduralArgsClassic;
            break;

        case kProceduralArgsType_ScenegraphAttr:
            argsType = kFnKatProceduralArgsScenegraphAttr;
            break;

        default: return std::string();
    }

    // call the suite function
    FnAttribute::StringAttribute result =
        FnAttribute::Attribute::CreateAndSteal(
        _suite->buildProceduralArgsString(
            sgIterator.getHandle(),
            argsType,
            argsAttrName.c_str(),
            contextInfo._frameNumber,
            contextInfo._shutterOpen,
            contextInfo._shutterClose,
            contextInfo._cropWindowXMin,
            contextInfo._cropWindowXMax,
            contextInfo._cropWindowYMin,
            contextInfo._cropWindowYMax,
            contextInfo._xres,
            contextInfo._yres));

    return result.getValue("", false);
}

void flushProceduralDsoCaches(const std::string &apiName)
{
    _suite->flushProceduralDsoCaches(apiName.c_str());
}

int getPixelAreaThroughCamera(FnScenegraphIterator sgIterator,
                              const std::string &cameraName,
                              const std::string &resolution)
{
    return _suite->getPixelAreaThroughCamera(sgIterator.getHandle(),
                                             cameraName.c_str(),
                                             resolution.c_str());
}

void fillXFormListForLocation(std::vector<Transform>& xFormList,
                              FnScenegraphIterator sgIterator,
                              float shutterClose /*= 0.0f*/)
{
    while(sgIterator.isValid())
    {
        FnAttribute::GroupAttribute xformAttr = sgIterator.getAttribute("xform");
        if(xformAttr.isValid())
        {
            if(fillXFormListFromAttributes(xFormList, xformAttr, shutterClose))
                return;
        }

        sgIterator = sgIterator.getParent();
    }
}

bool fillXFormListFromAttributes(std::vector<Transform>& xFormList,
                                 const FnAttribute::GroupAttribute& xformAttr,
                                 float shutterClose /*= 0.0f*/,
                                 bool invertMatrix /*= true*/)
{
    int64_t noChildren = xformAttr.getNumberOfChildren();
    for(int64_t i = noChildren - 1; i >= 0; i--)
    {
        Transform transform;

        std::string name = xformAttr.getChildName(i);
        FnAttribute::Attribute attr = xformAttr.getChildByIndex(i);
        FnAttribute::GroupAttribute groupAttr = attr;
        if(groupAttr.isValid())
        {
            if(fillXFormListFromAttributes(xFormList, attr, shutterClose))
                return true;

            continue;
        }

        FnAttribute::DoubleAttribute doubleAttr = attr;
        if(!doubleAttr.isValid())
        {
            continue;
        }

        int64_t numSampleTimes = doubleAttr.getNumberOfTimeSamples();
        for(int64_t i = 0; i < numSampleTimes; ++i)
        {
            float curTime = doubleAttr.getSampleTime(i);


            transform.sampleTimes.push_back(curTime);

            FnAttribute::DoubleConstVector samples = doubleAttr.getNearestSample(curTime);
            int64_t numberOfSamples = samples.size();

            if(name.compare(0, 6, "origin") == 0)
            {
                return true;
            }

            if(name.compare(0, 6, "rotate") == 0)
            {
                if(numberOfSamples == 4)
                {
                    std::vector<float> values;
                    values.push_back((float) -samples[0]);
                    values.push_back((float) samples[1]);
                    values.push_back((float) samples[2]);
                    values.push_back((float) samples[3]);

                    transform.transformList.push_back(TransformPair("rotate", values));
                }
            }
            else if(name.compare(0, 9, "translate") == 0)
            {
                if(numberOfSamples == 3)
                {
                    std::vector<float> values;
                    for(FnAttribute::DoubleConstVector::const_iterator it = samples.begin(); it != samples.end(); ++it)
                    {
                        float value = (float) (*it) * -1.0f;
                        values.push_back(value);
                    }

                    transform.transformList.push_back(TransformPair("translate", values));
                }
            }
            else if(name.compare(0, 5, "scale") == 0)
            {
                if(numberOfSamples == 3)
                {
                    std::vector<float> values;
                    for(FnAttribute::DoubleConstVector::const_iterator it = samples.begin(); it != samples.end(); ++it)
                    {
                        float value = 1.0f / (float) (*it);
                        values.push_back(value);
                    }

                    transform.transformList.push_back(TransformPair("scale", values));
                }
            }
            else if(name.compare(0, 6, "matrix") == 0)
            {
                if(numberOfSamples == 16)
                {
                    // Construct a matrix from the sample data
                    // Invert the matrix
                    // Convert values to float vector
                    XFormMatrix m(samples.data());
                    if(invertMatrix)
                        m.invert();

                    std::vector<float> values;
                    for(int i = 0; i < 16; ++i)
                    {
                        values.push_back(static_cast<float>(m.getValues()[i]));
                    }
                    transform.transformList.push_back( TransformPair( "matrix", values ) );
                }
            }
        }

        xFormList.push_back(transform);
    }

    return false;
}

FnAttribute::GroupAttribute convertTexturesToArbitraryAttr(const FnAttribute::GroupAttribute& texturesAttr)
{
    return FnAttribute::Attribute::CreateAndSteal(
            _suite->convertTexturesToArbitraryAttr(texturesAttr.getHandle()));
}


std::string getArchString()
{
    FnAttribute::StringAttribute result =
        FnAttribute::Attribute::CreateAndSteal(_suite->getArchString());
    return result.getValue("", false);
}

std::string expandArchPath(const std::string &path)
{
    FnAttribute::StringAttribute result =
        FnAttribute::Attribute::CreateAndSteal(
            _suite->expandArchPath(path.c_str()));
    return result.getValue("", false);
}

FnAttribute::GroupAttribute getFlattenedMaterialAttr(FnScenegraphIterator sgIterator,
                                        const FnAttribute::StringAttribute& terminalNamesAttr)
{
    return FnAttribute::Attribute::CreateAndSteal(
        _suite->getFlattenedMaterialAttr(sgIterator.getHandle(),
            terminalNamesAttr.getHandle()));
}

void emptyFlattenedMaterialCache()
{
    _suite->emptyFlattenedMaterialCache();
}

CameraInfo getCameraInfo(FnScenegraphIterator sgIterator,
                         const std::string& cameraInfoPath)
{
    CameraInfoHandle cameraInfoHandle = _suite->getCameraInfo(sgIterator.getHandle(),
                                                              cameraInfoPath.c_str());
    return CameraInfo(cameraInfoHandle);
}

void findSampleTimesRelevantToShutterRange(std::vector<float>& sampleTimes,
                                           const std::set<float>& inputSamples,
                                           float shutterOpen,
                                           float shutterClose)
{
    sampleTimes.clear();
    std::vector<float> inputSamplesVector(inputSamples.begin(), inputSamples.end());

    FnAttribute::FloatAttribute result =
        FnAttribute::Attribute::CreateAndSteal(
            _suite->findSampleTimesRelevantToShutterRange(inputSamplesVector.data(),
                                                      (unsigned int)(inputSamplesVector.size()),
                                                      shutterOpen,
                                                      shutterClose));

    if (!result.isValid()) return;

    FnAttribute::FloatConstVector times = result.getNearestSample(0.0f);
    sampleTimes.insert(sampleTimes.end(), times.begin(), times.end());
}

std::string getRenderResolution(FnScenegraphIterator rootIterator,
                                int *width,
                                int *height)
{
    FnAttribute::StringAttribute result =
        FnAttribute::Attribute::CreateAndSteal(
            _suite->getRenderResolution(rootIterator.getHandle(), width, height));
    return result.getValue("", false);
}

std::string getCameraPath(FnScenegraphIterator rootIterator)
{
    FnAttribute::StringAttribute result =
        FnAttribute::Attribute::CreateAndSteal(
            _suite->getCameraPath(rootIterator.getHandle()));
    return result.getValue("", false);
}

FnPluginHost* getHost()
{
    return gHost;
}

FnPlugStatus setHost(FnPluginHost *host)
{
    gHost = host;

    if (host)
    {
        _suite = (RenderOutputUtilsSuite_v3*)host->getSuite("RenderOutputUtilsHost", 3);
        if (_suite)
        {
            return FnPluginStatusOK;
        }

    }

    return FnPluginStatusError;
}

}
}
}
