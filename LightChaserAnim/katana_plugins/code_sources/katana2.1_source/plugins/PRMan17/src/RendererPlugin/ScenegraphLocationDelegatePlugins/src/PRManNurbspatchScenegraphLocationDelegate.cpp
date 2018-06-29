// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <rx.h>
#include <iostream>
#include <PRManNurbspatchScenegraphLocationDelegate.h>
#include <ErrorReporting.h>
using namespace PRManProcedural;
#include <WriteRI_Util.h>
#include <WriteRI_Attributes.h>
#include <WriteRI_ObjectID.h>
#include <WriteRI_Shaders.h>
#include <WriteRI_Statements.h>

using namespace FnKat;

void* processNurbspatch(FnScenegraphIterator sgIterator)
{
    if (!sgIterator.isValid()) return 0x0;
    if (!IsVisible(sgIterator)) return 0x0;

    FnAttribute::GroupAttribute geometryGroupAttr = sgIterator.getAttribute("geometry");
    if (!geometryGroupAttr.isValid())
    {
        Report_Error("NURBS Surface missing geometry group.", sgIterator);
        return 0x0;
    }

    FnAttribute::GroupAttribute pointGroupAttr = geometryGroupAttr.getChildByName("point");
    if (!pointGroupAttr.isValid())
    {
        Report_Error("NURBS Surface missing the point group", sgIterator);
        return 0x0;
    }

    FnAttribute::GroupAttribute uGroupAttr = geometryGroupAttr.getChildByName("u");
    if (!uGroupAttr.isValid())
    {
        Report_Error("NURBS Surface missing the u group.", sgIterator);
        return 0x0;
    }


    FnAttribute::IntAttribute uOrderAttr = uGroupAttr.getChildByName("order");
    FnAttribute::FloatAttribute uKnotsAttr = uGroupAttr.getChildByName("knots");
    FnAttribute::FloatAttribute uMinAttr = uGroupAttr.getChildByName("min");
    FnAttribute::FloatAttribute uMaxAttr = uGroupAttr.getChildByName("max");

    if (!uOrderAttr.isValid() || !uKnotsAttr.isValid() ||
            !uMinAttr.isValid() || !uMaxAttr.isValid())
    {
        Report_Error("NURBS Surface missing a required attribute in the u group.", sgIterator);
        return 0x0;
    }

    FnAttribute::GroupAttribute vGroupAttr = geometryGroupAttr.getChildByName("v");
    if (!vGroupAttr.isValid())
    {
        Report_Error("NURBS Surface missing the v group.", sgIterator);
        return 0x0;
    }

    FnAttribute::IntAttribute   vOrderAttr = vGroupAttr.getChildByName("order");
    FnAttribute::FloatAttribute vKnotsAttr = vGroupAttr.getChildByName("knots");
    FnAttribute::FloatAttribute vMinAttr = vGroupAttr.getChildByName("min");
    FnAttribute::FloatAttribute vMaxAttr = vGroupAttr.getChildByName("max");
    if (!vOrderAttr.isValid() || !vKnotsAttr.isValid() ||
            !vMinAttr.isValid() || !vMaxAttr.isValid())
    {
        Report_Error("NURBS Surface missing a required attribute in the v group.", sgIterator);
        return 0x0;
    }

    FnAttribute::FloatAttribute P_Attr = pointGroupAttr.getChildByName("P");
    FnAttribute::FloatAttribute Pw_Attr = pointGroupAttr.getChildByName("Pw");
    if (!P_Attr.isValid() && !Pw_Attr.isValid())
    {
        Report_Error("NURBS Surface missing the points.P and points.Pw attribute.", sgIterator);
        return 0x0;
    }

    RtInt uorder = uOrderAttr.getValue();
    RtInt nu = uKnotsAttr.getNumberOfTuples() - uorder;
    RtFloat* uknot = const_cast<RtFloat*>(uKnotsAttr.getNearestSample(0).data());
    RtFloat umin = uMinAttr.getValue();
    RtFloat umax = uMaxAttr.getValue();

    RtInt vorder = vOrderAttr.getValue();
    RtInt nv = vKnotsAttr.getNumberOfTuples() - vorder;
    RtFloat* vknot = const_cast<RtFloat*>(vKnotsAttr.getNearestSample(0).data());
    RtFloat vmin = vMinAttr.getValue();
    RtFloat vmax = vMaxAttr.getValue();

    AttrList attrList;
    if (P_Attr.isValid())
    {
        if (P_Attr.getNumberOfTuples() != (unsigned int)(nu * nv))
        {
            Report_Error("NURBS Surface has an invalid number of points", sgIterator);
            return 0x0;
        }

        attrList.push_back(AttrListEntry(RI_P, RI_P, P_Attr, true));
    }
    else
    {
        if (Pw_Attr.getNumberOfTuples() != (unsigned int)(nu * nv))
        {
            Report_Error("NURBS Surface has an invalid number of points", sgIterator);
            return 0x0;
        }

        attrList.push_back( AttrListEntry(RI_PW, RI_PW, Pw_Attr, true));
    }

    FnAttribute::GroupAttribute texturesGroup = sgIterator.getAttribute("textures", true);
    if (texturesGroup.isValid())
    {
        AddTextureAttrs(&attrList, texturesGroup, 1, sgIterator);
    }

    FnAttribute::GroupAttribute globalGeometryGroup = sgIterator.getAttribute("geometry");
    if (globalGeometryGroup.isValid())
    {
        AddArbitraryParameters(&attrList, globalGeometryGroup, sgIterator);
        AddRefParameters(&attrList, globalGeometryGroup, sgIterator);
    }

    AttrList_Converter converter(attrList);
    std::vector<float>& sampleTimes = converter.getSampleTimes();
    if (sampleTimes.size() > 1)
    {
        RiMotionBeginV(sampleTimes.size(), &sampleTimes[0]);
    }

    for (std::vector<float>::const_iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
    {
        RiNuPatchV(nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax,
                    converter.getSize(*it), converter.getDeclarationTokens(*it), converter.getParameters(*it));
    }

    if (sampleTimes.size() > 1)
    {
        RiMotionEnd();
    }
    return 0x0;

}

void* PRManNurbspatchScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
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

    void* returnValue = processNurbspatch(sgIterator);

    RiAttributeEnd();
    return returnValue;
}

PRManNurbspatchScenegraphLocationDelegate::~PRManNurbspatchScenegraphLocationDelegate()
{

}

void PRManNurbspatchScenegraphLocationDelegate::flush()
{

}

PRManNurbspatchScenegraphLocationDelegate* PRManNurbspatchScenegraphLocationDelegate::create()
{
    return new PRManNurbspatchScenegraphLocationDelegate();
}

std::string PRManNurbspatchScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("prman");
}

void PRManNurbspatchScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("nurbspatch"));
}
