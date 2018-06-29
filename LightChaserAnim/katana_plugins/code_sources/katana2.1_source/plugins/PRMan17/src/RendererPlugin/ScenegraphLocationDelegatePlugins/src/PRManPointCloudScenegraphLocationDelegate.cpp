// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <rx.h>
#include <iostream>
#include <PRManPointCloudScenegraphLocationDelegate.h>
#include <ErrorReporting.h>
using namespace PRManProcedural;
#include <WriteRI_Util.h>

#include <WriteRI_Attributes.h>
#include <WriteRI_ObjectID.h>
#include <WriteRI_Shaders.h>
#include <WriteRI_Statements.h>

using namespace FnKat;

void* processPointCloud(FnScenegraphIterator sgIterator)
{
    if (!sgIterator.isValid()) return 0x0;
    if (!IsVisible(sgIterator)) return 0x0;

    AttrList attrList;

    FnAttribute::GroupAttribute geometryGroupAttr = sgIterator.getAttribute("geometry");
    if (!geometryGroupAttr.isValid())
    {
        Report_Error("Pointcloud missing geometry attribute.", sgIterator);
        return 0x0;
    }

    FnAttribute::GroupAttribute pointGroupAttr = geometryGroupAttr.getChildByName("point");
    if (!pointGroupAttr.isValid())
    {
        Report_Error("Pointcloud missing geometry.point attribute.", sgIterator);
        return 0x0;
    }

    FnAttribute::FloatAttribute pointAttr = pointGroupAttr.getChildByName("P");
    if (!pointAttr.isValid())
    {
        Report_Error("Pointcloud missing geometry.point.P attribute.", sgIterator);
        return 0x0;
    }
    attrList.push_back(AttrListEntry(RI_P, RI_P, pointAttr, true));

    RtInt num_points = pointAttr.getNumberOfTuples();
    if (num_points == 0)
    {
        return 0x0;
    }

    FnAttribute::FloatAttribute normalAttr = pointGroupAttr.getChildByName("N");
    if (normalAttr.isValid())
    {
        attrList.push_back(AttrListEntry(RI_N, RI_N, normalAttr, true));
    }

    FnAttribute::FloatAttribute uvAttr = pointGroupAttr.getChildByName("uv");
    if (uvAttr.isValid())
    {
        attrList.push_back(AttrListEntry("varying float[2] st", "st", uvAttr, false));
    }

    FnAttribute::FloatAttribute widthAttr = pointGroupAttr.getChildByName("width");
    if (widthAttr.isValid())
    {
        attrList.push_back(AttrListEntry("width", "width", widthAttr, true));
    }

    FnAttribute::FloatAttribute constantwidthAttr = pointGroupAttr.getChildByName("constantwidth");
    if (constantwidthAttr.isValid())
    {
        attrList.push_back(AttrListEntry("constantwidth", "constantwidth", constantwidthAttr, false));
    }

    FnAttribute::GroupAttribute globalGeometryGroup = sgIterator.getAttribute("geometry", true);
    if (globalGeometryGroup.isValid())
    {
        AddArbitraryParameters(&attrList, globalGeometryGroup, sgIterator);
        AddRefParameters(&attrList, globalGeometryGroup, sgIterator);
    }

    AttrList_Converter converter(attrList);
    std::vector<float>& sampleTimes = converter.getSampleTimes();

    if (sampleTimes.size() > 1)
    {
        RiMotionBeginV(converter.getSampleTimes().size(), &(converter.getSampleTimes()[0]));
    }

    for (std::vector<float>::const_iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
    {
        RiPointsV(num_points, converter.getSize(*it), converter.getDeclarationTokens(*it), converter.getParameters(*it));
    }

    if (sampleTimes.size() > 1)
    {
        RiMotionEnd();
    }
    return 0x0;
}

void* PRManPointCloudScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
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

    void* returnValue = processPointCloud(sgIterator);

    RiAttributeEnd();
    return returnValue;
}

PRManPointCloudScenegraphLocationDelegate::~PRManPointCloudScenegraphLocationDelegate()
{

}

void PRManPointCloudScenegraphLocationDelegate::flush()
{

}

PRManPointCloudScenegraphLocationDelegate* PRManPointCloudScenegraphLocationDelegate::create()
{
    return new PRManPointCloudScenegraphLocationDelegate();
}


std::string PRManPointCloudScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("prman");
}

void PRManPointCloudScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("pointcloud"));
}
