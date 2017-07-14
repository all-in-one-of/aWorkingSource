// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <rx.h>
#include <iostream>
#include <PRManCurvesScenegraphLocationDelegate.h>
#include <ErrorReporting.h>
using namespace PRManProcedural;
#include <WriteRI_Util.h>

#include <WriteRI_Attributes.h>
#include <WriteRI_ObjectID.h>
#include <WriteRI_Shaders.h>
#include <WriteRI_Statements.h>

using namespace FnKat;

void* processCurves(FnScenegraphIterator sgIterator)
{
    if (!sgIterator.isValid()) return 0x0;
    if (!IsVisible(sgIterator)) return 0x0;

    FnAttribute::GroupAttribute geometryGroupAttr = sgIterator.getAttribute("geometry");
    if (!geometryGroupAttr.isValid())
    {
        Report_Error("Curves objects missing geometry group", sgIterator);
        return 0x0;
    }

    RtToken curveType;
    {
        FnAttribute::IntAttribute degreeAttr = geometryGroupAttr.getChildByName("degree");
        if (!degreeAttr.isValid())
        {
            Report_Error("Curves objects missing 'degree' attribute", sgIterator);
            return 0x0;
        }
        if (degreeAttr.getValue() == 1)
        {
            curveType = (char*)"linear";
        }
        else if (degreeAttr.getValue() == 3)
        {
            curveType = (char*)"cubic";
        }
        else
        {
            Report_Error("Curves type is neither linear or cubic", sgIterator);
            return 0x0;
        }
    }

    RtToken wrap = (char*)"nonperiodic";
    {
        FnAttribute::IntAttribute closedAttr = geometryGroupAttr.getChildByName((char*)"closed");
        if (closedAttr.isValid() && closedAttr.getValue() != 0)
        {
            wrap = (char*)"periodic";
        }
    }


    AttrList attrList;
    const int *numVertArray;
    int numNumVert;

    FnAttribute::IntAttribute ncurvesAttr = geometryGroupAttr.getChildByName("curves_ncurves");
    if (ncurvesAttr.isValid())
    {
        FnAttribute::IntAttribute nverticesAttr = geometryGroupAttr.getChildByName("curves_nvertices");
        if (!nverticesAttr.isValid())
        {
            Report_Error("Curves object specifies multiple curves but lacks 'curves_nvertices' attribute.", sgIterator);
            return 0x0;
        }
        if (nverticesAttr.getNumberOfTuples() == 0)
        {
            Report_Error("Curves object has zero size 'curves_nvertices' attribute.", sgIterator);
            return 0x0;
        }
        FnAttribute::IntConstVector numVert = nverticesAttr.getNearestSample(0);
        numVertArray = numVert.data();
        numNumVert = numVert.size();

        FnAttribute::FloatAttribute pointAttr = geometryGroupAttr.getChildByName("curves_pointList");
        if (!pointAttr.isValid())
        {
            Report_Error("Curves object specifies multiple curves but lacks 'curves_pointList' attribute.", sgIterator);
            return 0x0;
        }
        attrList.push_back(AttrListEntry(RI_P, RI_P, pointAttr, true));

        FnAttribute::FloatAttribute widthAttr = geometryGroupAttr.getChildByName("curves_width");
        if (widthAttr.isValid())
        {
            attrList.push_back(AttrListEntry("varying float width", "width", widthAttr, true));
        }

        FnAttribute::FloatAttribute cwidthAttr = geometryGroupAttr.getChildByName("curves_constantwidth");
        if (cwidthAttr.isValid())
        {
            //?? RiDeclare("constantwidth", "constant float");
            attrList.push_back(AttrListEntry("constant float constantwidth", "constantwidth", cwidthAttr, true));
        }
    }
    else
    {
        FnAttribute::IntAttribute numVertAttr = geometryGroupAttr.getChildByName("numVertices");
        if (!numVertAttr.isValid())
        {
            Report_Error("Curves object missing 'numVertices' attribute.", sgIterator);
            return 0x0;
        }

        if (numVertAttr.getNumberOfTuples() == 0)
        {
            Report_Error("Curves object 'numVertices' attribute has zero size.", sgIterator);
            return 0x0;
        }

        FnAttribute::IntConstVector numVert = numVertAttr.getNearestSample(0);
        numVertArray = numVert.data();
        numNumVert = numVert.size();

        FnAttribute::GroupAttribute pointGroupAttr = geometryGroupAttr.getChildByName("point");
        if (!pointGroupAttr.isValid())
        {
            Report_Error("Curves missing point attribute.", sgIterator);
            return 0x0;
        }

        FnAttribute::FloatAttribute pointAttr = pointGroupAttr.getChildByName("P");
        if (!pointAttr.isValid())
        {
            Report_Error("Curves missing point.P attribute.", sgIterator);
            return 0x0;
        }
        attrList.push_back(AttrListEntry(RI_P, RI_P, pointAttr, true));

        FnAttribute::FloatAttribute normalAttr = pointGroupAttr.getChildByName("N");
        if (normalAttr.isValid())
        {
            attrList.push_back(AttrListEntry(RI_N, RI_N, normalAttr, true));
        }

        FnAttribute::FloatAttribute widthAttr = pointGroupAttr.getChildByName("width");
        if (widthAttr.isValid())
        {
            //RiDeclare("width", "varying float");
            attrList.push_back(AttrListEntry("varying float width", "width", widthAttr, true));
        }

        FnAttribute::FloatAttribute cwidthAttr = geometryGroupAttr.getChildByName("constantWidth");
        if (cwidthAttr.isValid())
        {
            //RiDeclare("constantwidth", "constant float");
            attrList.push_back(AttrListEntry("constant float constantwidth", "constantwidth", cwidthAttr, true));
        }
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
        RiMotionBeginV(sampleTimes.size(), &sampleTimes.front());

    for (std::vector<float>::const_iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
    {
        RiCurvesV(curveType,
                  numNumVert,
                  (RtInt*)numVertArray,
                  wrap,
                  converter.getSize(*it),
                  converter.getDeclarationTokens(*it),
                  converter.getParameters(*it));
    }

    if (sampleTimes.size() > 1)
        RiMotionEnd();

    return 0x0;

}

PRManCurvesScenegraphLocationDelegate::~PRManCurvesScenegraphLocationDelegate()
{

}

void PRManCurvesScenegraphLocationDelegate::flush()
{

}

PRManCurvesScenegraphLocationDelegate* PRManCurvesScenegraphLocationDelegate::create()
{
    return new PRManCurvesScenegraphLocationDelegate();
}

void* PRManCurvesScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
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

    void* returnValue = processCurves(sgIterator);

    RiAttributeEnd();
    return returnValue;
}

std::string PRManCurvesScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("prman");
}

void PRManCurvesScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("curves"));
}
