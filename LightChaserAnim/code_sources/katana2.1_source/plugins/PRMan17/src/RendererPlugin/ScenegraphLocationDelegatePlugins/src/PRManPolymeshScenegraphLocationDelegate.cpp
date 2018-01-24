// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <rx.h>
#include <iostream>
#include <PRManPolymeshScenegraphLocationDelegate.h>

#include <PRManProcedural.h>
#include <sstream>
#include <ErrorReporting.h>
using namespace PRManProcedural;
#include <WriteRI_Util.h>

#include <WriteRI_Attributes.h>
#include <WriteRI_ObjectID.h>
#include <WriteRI_Shaders.h>
#include <WriteRI_Statements.h>

using namespace FnKat;

void* processPolymesh(FnScenegraphIterator sgIterator, PRManPluginState* sharedState)
{
    if (!sgIterator.isValid()) return 0x0;
    if (!IsVisible(sgIterator)) return 0x0;


    RtInt num_polys;
    std::vector<RtInt> num_vertices, vertices;
    AttrList attrList;
    if (!BuildPolymeshData(sgIterator, &num_polys, &num_vertices, &vertices, &attrList))
    {
        // Error already reported by BuildPolymeshData... just return
        return 0x0;
    }


    std::vector<RtInt> num_loops(num_polys, 1); // num_polys elements, initialized to 1.
    FnAttribute::GroupAttribute texturesGroup = sgIterator.getAttribute("textures", true);
    if (texturesGroup.isValid())
    {
        AddTextureAttrs(&attrList, texturesGroup, num_polys, sgIterator);
    }

    FnAttribute::GroupAttribute geometryGroup = sgIterator.getAttribute("geometry", true);
    if (geometryGroup.isValid())
    {
        AddArbitraryParameters(&attrList, geometryGroup, sgIterator);
        AddRefParameters(&attrList, geometryGroup, sgIterator);
    }

    AttrList_Converter converter(attrList);
    std::vector<float>& sampleTimes = converter.getSampleTimes();
    if (sampleTimes.size() > 1)
    {
        RiMotionBeginV(sampleTimes.size(), &sampleTimes[0]);
    }

    std::ostringstream os;
    os << "Sample times are:";
    {
        for (std::vector<float>::const_iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
        {
            os << *it << " ";
        }
    }
    Report_Debug(os.str(), sharedState, sgIterator);

    for (std::vector<float>::const_iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
    {
        RiPointsGeneralPolygonsV(num_polys,
                &num_loops[0], &num_vertices[0], &vertices[0],
                converter.getSize(*it), converter.getDeclarationTokens(*it), converter.getParameters(*it));

        for(unsigned int i = 0; i < converter.getSize(*it); i++)
        {
            std::string s = (char *)converter.getNameTokens(*it)[i];
        }
    }

    if (sampleTimes.size() > 1)
    {
        RiMotionEnd();
    }
    return 0x0;
}

void* PRManPolymeshScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
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

    void* returnValue = processPolymesh(sgIterator, state->sharedState);

    RiAttributeEnd();
    return returnValue;
}


PRManPolymeshScenegraphLocationDelegate::~PRManPolymeshScenegraphLocationDelegate()
{

}

void PRManPolymeshScenegraphLocationDelegate::flush()
{

}

PRManPolymeshScenegraphLocationDelegate* PRManPolymeshScenegraphLocationDelegate::create()
{
    return new PRManPolymeshScenegraphLocationDelegate();
}

std::string PRManPolymeshScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("prman");
}

void PRManPolymeshScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("polymesh"));
}
