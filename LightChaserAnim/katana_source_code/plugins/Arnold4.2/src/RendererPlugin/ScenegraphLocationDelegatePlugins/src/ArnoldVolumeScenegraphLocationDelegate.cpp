// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <buildCommons.h>
#include <ArnoldVolumeScenegraphLocationDelegate.h>
#include <vector>
#include <objectSettings.h>
#include <buildCamera.h>
#include <ArnoldPluginState.h>

using namespace FnKat;

AtNode* buildVolume(FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes )
{
    if (!isVisible(sgIterator)) return 0;

    std::string name = sgIterator.getFullName();

    if (AiNodeLookUpByName(name.c_str()))
    {
        AiMsgError("[kat] '%s' is already in the scene!", name.c_str());
        return 0;
    }

    GroupAttribute geometryAttr = sgIterator.getAttribute("geometry");
    if (!geometryAttr.isValid())
    {
        AiMsgError("[kat] '%s' is missing the geometry attribute.", name.c_str());
        return 0;
    }

    StringAttribute typeAttr = geometryAttr.getChildByName("type");
    if (!typeAttr.isValid())
    {
        AiMsgError("[kat] Volume '%s' is missing the geometry.type attribute.", name.c_str());
        return 0;
    }
    std::string type = typeAttr.getValue();

    AtNode *volumeNode = 0x0;
    if (type == "sphere")
    {
        DoubleAttribute radiusAttr = geometryAttr.getChildByName("radius");
        if (!radiusAttr.isValid())
        {
            AiMsgError("[kat] Volume '%s' is missing the geometry.radius attribute.", name.c_str());
            return 0;
        }
        float radius = radiusAttr.getValue();

        volumeNode = AiNode("sphere");
        AiNodeSetFlt(volumeNode, "radius", radius);

    }
    else if (type == "box")
    {
        DoubleAttribute boundAttr = sgIterator.getAttribute("bound", false);
        if (!boundAttr.isValid())
        {
            AiMsgError("[kat] Volume '%s': the 'bound' attribute is missing or invalid.", name.c_str());
            return 0;
        }

        DoubleConstVector bound = boundAttr.getNearestSample(0.0);
        if (bound.size() != 6)
        {
            AiMsgError("[kat] Volume '%s' has an invalid 'bound' attribute", name.c_str());
            return 0;
        }

        volumeNode = AiNode("box");
        AiNodeSetPnt(volumeNode, "min", (float)bound[0], (float)bound[2], (float)bound[4]);
        AiNodeSetPnt(volumeNode, "max", (float)bound[1], (float)bound[3], (float)bound[5]);
    }
    else
    {
        AiMsgError("[kat] Volume '%s' has an invalid geometry.type value.", name.c_str());
        return 0;
    }

    if(childNodes && volumeNode)
    {
        AiNodeSetStr(volumeNode, "name", name.c_str());

        FloatAttribute stepSizeAttr = geometryAttr.getChildByName("step_size");
        if (stepSizeAttr.isValid())
        {
            double stepSize = stepSizeAttr.getValue();
            AiNodeSetFlt(volumeNode, "step_size", stepSize);
        }

        childNodes->push_back(volumeNode);
    }
    return volumeNode;
}

void* ArnoldVolumeScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
{
    ArnoldSceneGraphLocationDelegateInput* state = reinterpret_cast<ArnoldSceneGraphLocationDelegateInput*>(optionalInput);
    void* returnValue = buildVolume(sgIterator, state->childNodes);
    if(returnValue)
    {
        applyObjectSettings(sgIterator, reinterpret_cast<AtNode*>(returnValue), state->childNodes, state->sharedState);
    }
    return returnValue;
}

ArnoldVolumeScenegraphLocationDelegate::~ArnoldVolumeScenegraphLocationDelegate()
{

}

void ArnoldVolumeScenegraphLocationDelegate::flush()
{

}

ArnoldVolumeScenegraphLocationDelegate::ArnoldVolumeScenegraphLocationDelegate()
{

}

ArnoldVolumeScenegraphLocationDelegate* ArnoldVolumeScenegraphLocationDelegate::create()
{
    return new ArnoldVolumeScenegraphLocationDelegate();
}

std::string ArnoldVolumeScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("arnold");
}

void ArnoldVolumeScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("volume"));
}
