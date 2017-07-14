// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <buildCommons.h>
#include <ArnoldSphereScenegraphLocationDelegate.h>
#include <vector>
#include <objectSettings.h>
#include <buildCamera.h>
#include <ArnoldPluginState.h>

using namespace FnKat;

AtNode* buildSphere(FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes )
{
    if (!isVisible(sgIterator)) return 0;

    std::string name = sgIterator.getFullName();

    if (AiNodeLookUpByName(name.c_str())) {
        AiMsgError("[kat] '%s' is already in the scene!", name.c_str());
        return 0;
    }

    GroupAttribute geometryAttr = sgIterator.getAttribute("geometry");
    if (!geometryAttr.isValid()) {
        AiMsgError("[kat] '%s' is missing the geometry attribute.", name.c_str());
        return 0;
    }

    FnAttribute::DoubleAttribute radiusattr = geometryAttr.getChildByName("radius");
    if (!radiusattr.isValid()) {
        AiMsgError("[kat] Sphere '%s' is missing the geometry attribute.", name.c_str());
        return 0;
    }
    float radius = radiusattr.getValue();

    AtNode *sphere = AiNode("sphere");
    AiNodeSetStr(sphere, "name", name.c_str());
    AiNodeSetFlt(sphere, "radius", radius);

    if(childNodes){
        childNodes->push_back(sphere);
    }
    return sphere;
}

void* ArnoldSphereScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
{
    ArnoldSceneGraphLocationDelegateInput* state = reinterpret_cast<ArnoldSceneGraphLocationDelegateInput*>(optionalInput);
    void* returnValue = buildSphere(sgIterator, state->childNodes);
    if(returnValue){
        applyObjectSettings(sgIterator, reinterpret_cast<AtNode*>(returnValue), state->childNodes, state->sharedState);
    }
    return returnValue;
}

ArnoldSphereScenegraphLocationDelegate::~ArnoldSphereScenegraphLocationDelegate()
{

}

void ArnoldSphereScenegraphLocationDelegate::flush()
{

}

ArnoldSphereScenegraphLocationDelegate::ArnoldSphereScenegraphLocationDelegate()
{

}

ArnoldSphereScenegraphLocationDelegate* ArnoldSphereScenegraphLocationDelegate::create()
{
    return new ArnoldSphereScenegraphLocationDelegate();
}

std::string ArnoldSphereScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("arnold");
}

void ArnoldSphereScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("sphere"));
}
