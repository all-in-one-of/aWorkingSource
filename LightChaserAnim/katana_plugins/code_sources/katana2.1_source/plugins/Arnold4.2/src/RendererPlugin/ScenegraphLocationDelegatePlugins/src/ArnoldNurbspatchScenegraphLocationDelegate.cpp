// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <buildCommons.h>
#include <ArnoldNurbspatchScenegraphLocationDelegate.h>
#include <vector>
#include <objectSettings.h>
#include <buildCamera.h>
#include <ArnoldPluginState.h>

/******************************************************************************
 *
 * buildNurbspatch
 *
 ******************************************************************************/

using namespace FnKat;

AtNode* buildNurbspatch(FnScenegraphIterator sgIterator,
                        std::vector<AtNode*>* childNodes,
                        ArnoldPluginState* sharedState)
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
        AiMsgError("[kat] Nurbspatch '%s' is missing the geometry attribute.", name.c_str());
        return 0;
    }

    GroupAttribute uAttr = geometryAttr.getChildByName("u");
    GroupAttribute vAttr = geometryAttr.getChildByName("v");
    GroupAttribute pointAttr = geometryAttr.getChildByName("point");

    FnAttribute::FloatAttribute PAttr, PrefAttr;

    int orderU;
    float minU, maxU;
    FnAttribute::FloatAttribute knotsAttrU;

    int orderV;
    float minV, maxV;
    FnAttribute::FloatAttribute knotsAttrV;

    try
    {
        if (!uAttr.isValid() || !vAttr.isValid() || !pointAttr.isValid()) throw -1;

        knotsAttrU = uAttr.getChildByName("knots");  
        if (!knotsAttrU.isValid()) throw -1;
        orderU = FnAttribute::IntAttribute(uAttr.getChildByName("order")).getValue();
        minU = FnAttribute::FloatAttribute(uAttr.getChildByName("min")).getValue();
        maxU = FnAttribute::FloatAttribute(uAttr.getChildByName("max")).getValue();

        knotsAttrV = vAttr.getChildByName("knots");  
        if (!knotsAttrV.isValid()) throw -1;
        orderV = FnAttribute::IntAttribute(vAttr.getChildByName("order")).getValue();
        minV = FnAttribute::FloatAttribute(vAttr.getChildByName("min")).getValue();
        maxV = FnAttribute::FloatAttribute(vAttr.getChildByName("max")).getValue();

        //try to get either P or Pw
        PAttr = pointAttr.getChildByName("P");
        if(!PAttr.isValid())
        {
            PAttr = pointAttr.getChildByName("Pw");
            if(!PAttr.isValid()) throw(-1);
        }
        
        PrefAttr = pointAttr.getChildByName("Pref");
    } 
    catch (...) 
    {
        AiMsgError("[kat] Nurbspatch '%s' has an invalid geometry attribute.", name.c_str());
        return 0;
    }    
    
    //Arnold native NURBS
    if (orderU || orderV)
    {
        AtNode* nurbs = AiNode( "nurbs" );

            if(childNodes){
                childNodes->push_back(nurbs);
            }      
        std::string name = sgIterator.getFullName();
        AiNodeSetStr(nurbs, "name", name.c_str());      

        AiNodeSetBool( nurbs, "smoothing", true);
        AiNodeSetInt( nurbs, "degree_u", orderU-1 );
        AiNodeSetInt( nurbs, "degree_v", orderV-1 );
        
        FloatConstVector kfU = knotsAttrU.getNearestSample(0);
        FloatConstVector kfV = knotsAttrV.getNearestSample(0);

        //get num samples for cvs
        std::set<float> sampleTimes;
        getSampleTimes(sgIterator.getRoot(), PAttr, &sampleTimes, sharedState);
        
        std::vector<float> cvs;
        cvs.reserve(sampleTimes.size() * PAttr.getNumberOfValues());

        //concat all samples into one big .ass array 
        for(std::set<float>::iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
        {
            FloatConstVector sampleData = PAttr.getNearestSample(*it);
            cvs.insert(cvs.end(), sampleData.begin(), sampleData.end());
        }
        
        if (sampleTimes.size() > 1)
        {
            std::vector<float> outputSampleTimes;
            outputSampleTimes.reserve(sampleTimes.size());
            outputSampleTimes.insert(outputSampleTimes.end(), sampleTimes.begin(), sampleTimes.end());
            const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(nurbs);
            if (AiNodeEntryLookUpParameter(baseNodeEntry, "deform_time_samples"))
            {
                AiNodeSetArray(nurbs, "deform_time_samples",
                        AiArrayConvert(outputSampleTimes.size(), 1, AI_TYPE_FLOAT,
                                &outputSampleTimes[0]));
            }
            else if (outputSampleTimes.size() > 1 &&
                        (outputSampleTimes.front() != 0 || outputSampleTimes.back() != 1))
            {
                AiMsgWarning(
                    std::string("[kat] non 0 to 1 geometry samples found on " +
                        sgIterator.getFullName() +
                                ". This version of arnold does not "
                                "support this and will likely result in "
                                "inaccurate motion blur.").c_str());
            }
        }
        
        AiNodeSetArray( nurbs, "knots_u", AiArrayConvert( kfU.size(), 1,
            AI_TYPE_FLOAT, (void *)(&kfU[0])) );
        AiNodeSetArray( nurbs, "knots_v", AiArrayConvert( kfV.size(), 1,
            AI_TYPE_FLOAT, (void *)(&kfV[0])) );
        AiNodeSetArray( nurbs, "cvs",    AiArrayConvert(PAttr.getNumberOfValues(), sampleTimes.size(),
            AI_TYPE_FLOAT, (void*)(&(cvs[0]))));

        //got Pref?
        if(PrefAttr.isValid())
        {
            //TODO: single sample Pref sample should be enough, right? never need MB?
            FloatConstVector pref = PrefAttr.getNearestSample(0);
            
            //we only want to give them a point, aka 3 floats as we don't support HPoint at the moment
            int elemSize = PrefAttr.getTupleSize();
            int numPrefPoints = pref.size()/elemSize;
            std::vector<AtPoint> arnoldPref;
            arnoldPref.reserve(numPrefPoints);
            for(int i = 0; i < numPrefPoints; i++)
            {
                AtPoint p;
                int index = i*elemSize;
                p.x = pref[index];
                p.y = pref[index+1];
                p.z = pref[index+2];
                arnoldPref[i] = p;
            }

            
            //TODO: it is unclear what the correct format for this is here. cliff? 
            AiNodeDeclare(nurbs, "Pref", "varying POINT");
            AiNodeSetArray(nurbs, "Pref", 
                AiArrayConvert(numPrefPoints, 1, AI_TYPE_POINT, (void*)&arnoldPref[0]));
        }
        
        setTextureUserDataForNurbspatch(sgIterator, nurbs);

        //get arbitrary and handle it
        GroupAttribute arbitraryAttr = sgIterator.getAttribute("geometry.arbitrary", true);
        if (arbitraryAttr.isValid())
        {
            writeArbitrary(nurbs, sgIterator.getFullName(), sgIterator.getType(),
                    geometryAttr, arbitraryAttr);
        }
        
        return nurbs;
    
    }
    else 
    {
        AiMsgError("[kat] Nurbspatch '%s' has U and/or V order of 0.", name.c_str());
        return 0;
    }


}


void* ArnoldNurbspatchScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator,
                                                          void* optionalInput)
{
    ArnoldSceneGraphLocationDelegateInput* state = reinterpret_cast<ArnoldSceneGraphLocationDelegateInput*>(optionalInput);
    void* returnValue = buildNurbspatch(sgIterator, state->childNodes, state->sharedState);
    if(returnValue){
        applyObjectSettings(sgIterator, reinterpret_cast<AtNode*>(returnValue), state->childNodes, state->sharedState);
    }
    return returnValue;
}

ArnoldNurbspatchScenegraphLocationDelegate::~ArnoldNurbspatchScenegraphLocationDelegate()
{

}

void ArnoldNurbspatchScenegraphLocationDelegate::flush()
{

}

ArnoldNurbspatchScenegraphLocationDelegate::ArnoldNurbspatchScenegraphLocationDelegate()
{

}

ArnoldNurbspatchScenegraphLocationDelegate* ArnoldNurbspatchScenegraphLocationDelegate::create()
{
    return new ArnoldNurbspatchScenegraphLocationDelegate();
}

std::string ArnoldNurbspatchScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("arnold");
}

void ArnoldNurbspatchScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("nurbspatch"));
}
