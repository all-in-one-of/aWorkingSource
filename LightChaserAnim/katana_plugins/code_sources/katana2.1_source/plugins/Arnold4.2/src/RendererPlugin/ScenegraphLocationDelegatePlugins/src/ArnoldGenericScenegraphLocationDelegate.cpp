// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <buildCommons.h>
#include <ArnoldGenericScenegraphLocationDelegate.h>
#include <vector>
#include <buildMaterial.h>
#include <objectSettings.h>
#include <buildCamera.h>
#include <ArnoldPluginState.h>

using namespace FnKat;

AtNode* buildGeneric(FnScenegraphIterator sgIterator,
                     std::vector<AtNode*>* childNodes,
                     ArnoldPluginState* sharedState)
{
    if (!isVisible(sgIterator)) return 0;
   
    std::string name = sgIterator.getFullName();
    
    GroupAttribute geometryAttr = sgIterator.getAttribute("geometry");
    if (!geometryAttr.isValid())
    {
        AiMsgWarning("[kat] arnold generic: '%s' is missing the geometry attribute.", name.c_str());
        return 0;
    }
    
    FnAttribute::StringAttribute nodePathAttr = geometryAttr.getChildByName("node");
    
    if (!nodePathAttr.isValid())
    {
        AiMsgWarning("[kat] arnold generic: '%s' is missing the geometry.node attribute.", name.c_str());
        return 0;
    }
    
    
    AtNode* result = getShaderByPath(nodePathAttr.getValue(), sgIterator, childNodes, false, sharedState);
    
    GroupAttribute arbitraryAttr =
            sgIterator.getAttribute("geometry.arbitrary", true);
    
    if (arbitraryAttr.isValid())
    {
        writeArbitrary(result, sgIterator.getFullName(), sgIterator.getType(),
                geometryAttr, arbitraryAttr);
    }
    
    return result;
}



void* ArnoldGenericScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator,
                                                       void* optionalInput)
{
    ArnoldSceneGraphLocationDelegateInput* state = reinterpret_cast<ArnoldSceneGraphLocationDelegateInput*>(optionalInput);
    void* returnValue = buildGeneric(sgIterator, state->childNodes, state->sharedState);
    if(returnValue){
        applyObjectSettings(sgIterator, reinterpret_cast<AtNode*>(returnValue), state->childNodes, state->sharedState);
    }
    return returnValue;
}

ArnoldGenericScenegraphLocationDelegate::~ArnoldGenericScenegraphLocationDelegate()
{

}

void ArnoldGenericScenegraphLocationDelegate::flush()
{

}

ArnoldGenericScenegraphLocationDelegate::ArnoldGenericScenegraphLocationDelegate()
{

}

ArnoldGenericScenegraphLocationDelegate* ArnoldGenericScenegraphLocationDelegate::create()
{
    return new ArnoldGenericScenegraphLocationDelegate();
}

std::string ArnoldGenericScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("arnold");
}

void ArnoldGenericScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("arnold generic primitive"));
}
