// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <rx.h>
#include <iostream>
#include <PRManBrickMapScenegraphLocationDelegate.h>
#include <ErrorReporting.h>
using namespace PRManProcedural;
#include <WriteRI_Util.h>
#include <WriteRI_Attributes.h>
#include <WriteRI_ObjectID.h>
#include <WriteRI_Shaders.h>
#include <WriteRI_Statements.h>

using namespace FnKat;

void* processBrickMap(FnScenegraphIterator sgIterator) {

    if (!sgIterator.isValid()) return 0x0;
    if (!IsVisible(sgIterator)) return 0x0;

    FnAttribute::GroupAttribute geometryGroupAttr = sgIterator.getAttribute("geometry");
    if (!geometryGroupAttr.isValid())
    {
        Report_Error("BrickMap missing geometry attribute.", sgIterator);
        return 0x0;
    }

    FnAttribute::StringAttribute stringAttr = geometryGroupAttr.getChildByName("filename");
    if (!stringAttr.isValid())
    {
        Report_Error("BrickMap missing geometry.filename attribute.", sgIterator);
        return 0x0;
    }

    //TODO: use the asser managemetn system to decode the path
    std::string brickmapPath = stringAttr.getValue();

    //not as clean as the converter stuff but seems to work.
    //if someone feels they want to do this via the converter, please go ahead and chenge it. rl
    RtPointer tmpArray[1] = { reinterpret_cast< RtPointer >( const_cast< char* >( brickmapPath.c_str() ) ) };
    RiGeometry((char*)"brickmap", (char*)"filename", tmpArray, RI_NULL);
    return 0x0;
}

PRManBrickMapScenegraphLocationDelegate::~PRManBrickMapScenegraphLocationDelegate()
{

}

void PRManBrickMapScenegraphLocationDelegate::flush()
{

}

PRManBrickMapScenegraphLocationDelegate* PRManBrickMapScenegraphLocationDelegate::create()
{
    return new PRManBrickMapScenegraphLocationDelegate();
}

void* PRManBrickMapScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
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

    void* returnValue = processBrickMap(sgIterator);

    RiAttributeEnd();
    return returnValue;
}

std::string PRManBrickMapScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("prman");
}

void PRManBrickMapScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("brickmap"));
}

