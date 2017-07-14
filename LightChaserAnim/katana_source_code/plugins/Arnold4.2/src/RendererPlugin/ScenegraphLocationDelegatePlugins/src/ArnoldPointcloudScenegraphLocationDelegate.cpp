// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <buildCommons.h>
#include <ArnoldPointcloudScenegraphLocationDelegate.h>
#include <objectSettings.h>
#include <buildCamera.h>
#include <ArnoldPluginState.h>

using namespace FnKat;

AtNode* buildPointcloud(FnScenegraphIterator sgIterator,
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
        AiMsgError("[kat] Curves '%s' is missing the geometry attribute.", name.c_str());
        return 0;
    }

    GroupAttribute pointAttr = geometryAttr.getChildByName("point");
    if (!pointAttr.isValid())
    {
        AiMsgError("[kat] Curves '%s' is missing the geometry.point attribute.", name.c_str());
        return 0;
    }

    FnAttribute::FloatAttribute PAttr = pointAttr.getChildByName("P");
    if (!PAttr.isValid())
    {
        AiMsgError("[kat] Points '%s' is missing the point.P attribute.", name.c_str());
        return 0;
    }

    if (PAttr.getNumberOfTuples() == 0)
    {
        return 0;
    }

    FnAttribute::FloatAttribute widthAttr = pointAttr.getChildByName("width");
    FnAttribute::FloatAttribute constantwidthAttr = pointAttr.getChildByName("constantwidth");

    std::vector<float> width;
    //to adjust size of point primitive need to mutiply 0.5 to width.
    if (widthAttr.isValid())
    {
        FloatConstVector fcv = widthAttr.getNearestSample(0);
        width.resize(fcv.size());
        for (unsigned int i=0; i<width.size(); i++)
        {
            width[i] = fcv[i] * 0.5f;
        }
    }
    else if (constantwidthAttr.isValid())
    {
        width.push_back(0.5f*constantwidthAttr.getValue());
    }
    else
    {
        width.push_back(0.05f);
    }
   
    AtNode* pointnode = AiNode("points");
    if (!pointnode)
    {
        AiMsgError("[kat] unable to create points primitive for %s", name.c_str());
        return NULL;
    }    

    //get num samples for P
    std::set<float> sampleTimes;
    getSampleTimes(sgIterator.getRoot(), PAttr, &sampleTimes, sharedState);
    
    //reserve all P's
    std::vector<float> vlist;
    vlist.reserve(sampleTimes.size() * PAttr.getNumberOfValues());

    //concat all samples into one big .ass array 
    for (std::set<float>::iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it)
    {
        FloatConstVector sampleData = PAttr.getNearestSample(*it);
        vlist.insert(vlist.end(), sampleData.begin(), sampleData.end());
    }
    
    if (sampleTimes.size() > 1)
    {
        std::vector<float> outputSampleTimes;
        outputSampleTimes.reserve(sampleTimes.size());
        outputSampleTimes.insert(outputSampleTimes.end(), sampleTimes.begin(), sampleTimes.end());
        const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(pointnode);
        if (AiNodeEntryLookUpParameter(baseNodeEntry, "deform_time_samples"))
        {
            AiNodeSetArray(pointnode, "deform_time_samples",
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

    //get arbitrary and handle it
    GroupAttribute arbitraryAttr = sgIterator.getAttribute("geometry.arbitrary", true);
    if (arbitraryAttr.isValid())
    {
        writeArbitrary(pointnode, sgIterator.getFullName(), sgIterator.getType(),
                geometryAttr, arbitraryAttr);
    }

    AiNodeSetStr(pointnode, "name", name.c_str());

    //send P samples to arnold
    AiNodeSetArray(pointnode, "points", AiArrayConvert(PAttr.getNumberOfValues(), sampleTimes.size(), AI_TYPE_FLOAT, (void*)(&(vlist[0]))));
    AiNodeSetArray(pointnode, "radius", AiArrayConvert(width.size(), 1, AI_TYPE_FLOAT, &width[0]));

    GroupAttribute arnoldStatementsGroup = sgIterator.getAttribute("arnoldStatements", true);
    if (arnoldStatementsGroup.isValid()) 
    {
        FnAttribute::StringAttribute modeAttr = arnoldStatementsGroup.getChildByName("point_mode");
        
        //legacy attr fallback
        if (!modeAttr.isValid())
        {
            modeAttr = arnoldStatementsGroup.getChildByName("point_prim_type");
        }
        
        if (modeAttr.isValid())
        {
            AiNodeSetStr(pointnode, "mode", modeAttr.getValue().c_str());
        }
    }
    
    //point-specific settings which live in arnoldStatements
    //TODO: consider moving this to general handling with parameter testing
    if (arnoldStatementsGroup.isValid())
    {
        FnAttribute::FloatAttribute floatAttr = arnoldStatementsGroup.getChildByName("min_pixel_width");
        if (floatAttr.isValid())
        {
            AiNodeSetFlt(pointnode, "min_pixel_width", floatAttr.getValue());
        }
    }
    
    if(childNodes){
        childNodes->push_back(pointnode);
    }

    return pointnode;
}



void* ArnoldPointcloudScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator,
                                                          void* optionalInput)
{
    ArnoldSceneGraphLocationDelegateInput* state = reinterpret_cast<ArnoldSceneGraphLocationDelegateInput*>(optionalInput);
    void* returnValue = buildPointcloud(sgIterator, state->childNodes, state->sharedState);
    if(returnValue){
        applyObjectSettings(sgIterator, reinterpret_cast<AtNode*>(returnValue), state->childNodes, state->sharedState);
    }
    return returnValue;
}

ArnoldPointcloudScenegraphLocationDelegate::~ArnoldPointcloudScenegraphLocationDelegate()
{

}

void ArnoldPointcloudScenegraphLocationDelegate::flush()
{

}

ArnoldPointcloudScenegraphLocationDelegate::ArnoldPointcloudScenegraphLocationDelegate()
{

}

ArnoldPointcloudScenegraphLocationDelegate* ArnoldPointcloudScenegraphLocationDelegate::create()
{
    return new ArnoldPointcloudScenegraphLocationDelegate();
}

std::string ArnoldPointcloudScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("arnold");
}

void ArnoldPointcloudScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("pointcloud"));
}
