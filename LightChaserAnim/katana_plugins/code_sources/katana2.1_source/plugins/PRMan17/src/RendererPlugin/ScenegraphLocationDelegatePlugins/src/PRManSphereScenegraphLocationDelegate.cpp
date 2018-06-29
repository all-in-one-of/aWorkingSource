// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <rx.h>
#include <iostream>
#include <PRManSphereScenegraphLocationDelegate.h>
#include <ErrorReporting.h>
#include <WriteRI_Attributes.h>
#include <WriteRI_ObjectID.h>
#include <WriteRI_Shaders.h>
#include <WriteRI_Statements.h>
using namespace PRManProcedural;

using namespace FnKat;

void* processSphere(FnScenegraphIterator sgIterator)
{
    if (!sgIterator.isValid()) {
        std::cout << "SphereLocation::render: Scene graph iterator is not valid" << std::endl;
        return 0x0;
    }

    FnAttribute::GroupAttribute geometryGroupAttr = sgIterator.getAttribute("geometry");

    if (!geometryGroupAttr.isValid()) {
        std::cout << "SphereLocation::render: Group Attribute is not valid" << std::endl;
        return 0x0;
    }

    FnAttribute::DoubleAttribute radiusattr = geometryGroupAttr.getChildByName("radius");

    if (!radiusattr.isValid()) {
        std::cout << "SphereLocation::render: Radius Attribute is not valid" << std::endl;
        return 0x0;
    }

    float radius = radiusattr.getValue();
    RiSphere(radius, -radius, radius, 360.0f, RI_NULL);
    return 0x0;
}

void* PRManSphereScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
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

    void* returnValue = processSphere(sgIterator);

    RiAttributeEnd();
    return returnValue;
}

PRManSphereScenegraphLocationDelegate::~PRManSphereScenegraphLocationDelegate()
{

}

void PRManSphereScenegraphLocationDelegate::flush()
{

}

PRManSphereScenegraphLocationDelegate* PRManSphereScenegraphLocationDelegate::create()
{
    return new PRManSphereScenegraphLocationDelegate();
}

std::string PRManSphereScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("prman");
}

void PRManSphereScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("sphere"));
}
