// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <rx.h>
#include <iostream>
#include <PRManSpheresScenegraphLocationDelegate.h>
#include <ErrorReporting.h>
using namespace PRManProcedural;
#include <WriteRI_Util.h>
#include <set>
#include <WriteRI_Attributes.h>
#include <WriteRI_ObjectID.h>
#include <WriteRI_Shaders.h>
#include <WriteRI_Statements.h>

using namespace FnKat;

void* processSpheres(FnScenegraphIterator sgIterator)
{
    if (!sgIterator.isValid()) return 0x0;
    if (!IsVisible(sgIterator)) return 0x0;

    AttrList attrList;

    FnAttribute::GroupAttribute geometryGroupAttr = sgIterator.getAttribute("geometry");
    if (!geometryGroupAttr.isValid())
    {
        Report_Error("Spheres missing geometry attribute.", sgIterator);
        return 0x0;
    }

    FnAttribute::GroupAttribute pointGroupAttr = geometryGroupAttr.getChildByName("point");
    if (!pointGroupAttr.isValid())
    {
        Report_Error("Spheres missing geometry.point attribute.", sgIterator);
        return 0x0;
    }

    FnAttribute::FloatAttribute pointAttr = pointGroupAttr.getChildByName("P");
    if (!pointAttr.isValid())
    {
        Report_Error("Spheres missing geometry.point.P attribute.", sgIterator);
        return 0x0;
    }
    attrList.push_back(AttrListEntry(RI_P, RI_P, pointAttr, true));

    FnAttribute::FloatAttribute radiusAttr = pointGroupAttr.getChildByName("radius");
    FnAttribute::FloatAttribute constantRadiusAttr = geometryGroupAttr.getChildByName("constantRadius");
    if (radiusAttr.isValid())
    {
        if (constantRadiusAttr.isValid())
        {
            Report_Warning("Spheres has both geometry.point.radius and "
                           "geometry.constantRadius attrs.  Using geometry.point.radius.",
                           sgIterator);

        }
    }
    else if (constantRadiusAttr.isValid())
    {
        radiusAttr = constantRadiusAttr;
    }

    std::vector<float> radiusSampleTimes;
    if (radiusAttr.isValid())
    {
        int64_t numSampleTimes = radiusAttr.getNumberOfTimeSamples();
        radiusSampleTimes.resize(numSampleTimes);
        for (int64_t i = 0; i < numSampleTimes; ++i)
        {
            radiusSampleTimes[i] = radiusAttr.getSampleTime(i);
        }
    }

    FnAttribute::GroupAttribute globalGeometryGroup = sgIterator.getAttribute("geometry", true);
    if (globalGeometryGroup.isValid())
    {
        AddArbitraryParameters(&attrList, globalGeometryGroup, sgIterator);
        AddRefParameters(&attrList, globalGeometryGroup, sgIterator);
    }

    int64_t numPointSampleTimes = pointAttr.getNumberOfTimeSamples();

    std::vector<float> pointSampleTimes(numPointSampleTimes);
    for (int64_t i = 0; i < numPointSampleTimes; ++i)
    {
        pointSampleTimes[i] = pointAttr.getSampleTime(i);
    }

    unsigned int numSpheres = pointAttr.getNumberOfValues()/3;

    FnAttribute::FloatAttribute pointXformAttr = pointGroupAttr.getChildByName("xform");
    std::vector<float> pointXformSampleTimes;
    if (pointXformAttr.isValid())
    {
        int64_t numXformSampleTimes = pointXformAttr.getNumberOfTimeSamples();
        pointXformSampleTimes.resize(numXformSampleTimes);
        for (int64_t i = 0; i < numXformSampleTimes; ++i)
        {
            pointXformSampleTimes[i] = pointXformAttr.getSampleTime(i);
        }

        if (pointXformAttr.getNumberOfValues() != numSpheres*9)
        {
            Report_Error("Spheres geometry.point.xform attribute length does not match"
                         "geometry.point.P attribute length.", sgIterator);

            return 0x0;
        }
    }

    for (unsigned int sphereIndex=0; sphereIndex<numSpheres; ++sphereIndex)
    {
        RiTransformBegin();

        // sphere position
        if (pointSampleTimes.size() > 1)
        {
            RiMotionBeginV(pointSampleTimes.size(), const_cast<float *>(&pointSampleTimes[0]));
        }
        for (std::vector<float>::const_iterator it = pointSampleTimes.begin();
             it != pointSampleTimes.end(); ++it)
        {
            const float *p = &pointAttr.getNearestSample(*it)[sphereIndex*3];
            RiTranslate(p[0], p[1], p[2]);
        }
        if (pointSampleTimes.size() > 1)
        {
            RiMotionEnd();
        }

        // sphere xform
        if (pointXformAttr.isValid())
        {
            if (pointXformSampleTimes.size() > 1)
            {
                RiMotionBeginV(pointXformSampleTimes.size(), &(pointXformSampleTimes[0]));
            }
            for (std::vector<float>::const_iterator it = pointXformSampleTimes.begin();
                 it != pointXformSampleTimes.end(); ++it)
            {
                const float *xform = &pointXformAttr.getNearestSample(*it)[sphereIndex*9];
                RtMatrix mat = {{xform[0], xform[1], xform[2], 0.0},
                                {xform[3], xform[4], xform[5], 0.0},
                                {xform[6], xform[7], xform[8], 0.0},
                                {0.0, 0.0, 0.0, 1.0}};
                RiConcatTransform(mat);
            }
            if (pointXformSampleTimes.size() > 1)
            {
                RiMotionEnd();
            }
        }

        // extra sphere parameters (bake down to single constant value for sphere)
        AttrList sphereAttrList;
        for (AttrList::iterator attrIter = attrList.begin(); attrIter != attrList.end(); ++attrIter)
        {
            const AttrListEntry &attrListEntry = (*attrIter);

            if (attrListEntry._name == "P") continue;

            DataAttribute dataAttr = attrListEntry._value;
            if (!dataAttr.isValid()) continue;

            Attribute newAttr;
            if (dataAttr.getNumberOfTuples() == 1)
            {
                newAttr = dataAttr;
            }
            else if (sphereIndex < dataAttr.getNumberOfTuples())
            {
                newAttr = createConstantAttrByIndex(dataAttr, sphereIndex);
            }
            else
            {
                continue;
            }

            sphereAttrList.push_back(AttrListEntry(attrListEntry._declaration, attrListEntry._name,
                                     newAttr, attrListEntry._mightBlur));
        }

        AttrList_Converter converter(sphereAttrList);
        std::vector<float>& attrListSampleTimes = converter.getSampleTimes();


        // combine attrlist sample times with radius sample times
        std::set<float> sampleTimeSet;
        sampleTimeSet.insert(attrListSampleTimes.begin(), attrListSampleTimes.end());
        sampleTimeSet.insert(radiusSampleTimes.begin(), radiusSampleTimes.end());

        std::vector<float> sampleTimes(sampleTimeSet.begin(), sampleTimeSet.end());

        // sphere primitive
        if (sampleTimes.size() > 1)
        {
            RiMotionBeginV(sampleTimes.size(), &(sampleTimes[0]));
        }
        for (std::vector<float>::const_iterator it = sampleTimes.begin();
             it != sampleTimes.end(); ++it)
        {
            float radius = 1.0f;
            if (radiusAttr.isValid())
            {
                FnAttribute::FloatConstVector radiusData = radiusAttr.getNearestSample(*it);
                if (radiusData.size() == 1)
                {
                    radius = radiusData[0];
                }
                else if (sphereIndex < radiusData.size())
                {
                    radius = radiusData[sphereIndex];
                }
            }

            RiSphereV(radius, -radius, radius, 360.0f,
                converter.getSize(*it), converter.getDeclarationTokens(*it),
                converter.getParameters(*it));
        }
        if (sampleTimes.size() > 1)
        {
            RiMotionEnd();
        }

        RiTransformEnd();
    }
    return 0x0;
}

void* PRManSpheresScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
{
    PRManSceneGraphLocationDelegateInput* state = reinterpret_cast<PRManSceneGraphLocationDelegateInput*>(optionalInput);

    WriteRI_Object_LevelOfDetail(sgIterator, state->sharedState);
    RiAttributeBegin();
    WriteRI_Object_Name(sgIterator, state->sharedState);
    WriteRI_Object_Id(sgIterator, state->sharedState);
    FnAttribute::GroupAttribute xformAttr = sgIterator.getAttribute("xform");
    WriteRI_Object_Transform(xformAttr, sgIterator, state->sharedState);
    WriteRI_Object_RelativeScopedCoordinateSystems(sgIterator, state->sharedState);
    WriteRI_Object_Statements(sgIterator, state->sharedState);
    WriteRI_Object_Material(sgIterator, state->sharedState);
    WriteRI_Object_IlluminationList(sgIterator, state->sharedState);

    void* returnValue = processSpheres(sgIterator);

    RiAttributeEnd();
    return returnValue;
}

PRManSpheresScenegraphLocationDelegate::~PRManSpheresScenegraphLocationDelegate()
{

}

void PRManSpheresScenegraphLocationDelegate::flush()
{

}

PRManSpheresScenegraphLocationDelegate* PRManSpheresScenegraphLocationDelegate::create()
{
    return new PRManSpheresScenegraphLocationDelegate();
}

std::string PRManSpheresScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("prman");
}

void PRManSpheresScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("spheres"));
}
