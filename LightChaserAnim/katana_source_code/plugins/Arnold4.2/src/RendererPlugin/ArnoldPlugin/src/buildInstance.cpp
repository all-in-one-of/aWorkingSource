// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include "arnold_render.h"

#include <sstream>
#include <iostream>
#include <boost/thread.hpp>

#include <pystring/pystring.h>

#include <FnAttribute/FnGroupBuilder.h>

#include <buildCommons.h>
#include <objectId.h>

using namespace FnKat;

struct InstanceSource
{
    struct LeafEntry
    {
        GroupAttribute xformAttr;
        AtNode * node;
        AtByte srcVisibility;
        
        GroupAttribute geometryAttr;
        std::string locationType;
    };
    
    typedef std::tr1::shared_ptr<LeafEntry> LeafEntryRefPtr;
    
    typedef std::map<std::string, LeafEntryRefPtr> LeafMap;
    LeafMap leafMap;
};


typedef std::tr1::shared_ptr<InstanceSource> InstanceSourceRefPtr;
typedef std::map<std::string, InstanceSourceRefPtr> InstanceSourceMap;

namespace
{
    InstanceSourceMap g_instanceSources;
    boost::recursive_mutex instanceSourcesMutex;
}

///////////////////////////////////////////////////////////////////////////////

void buildInstanceGroup(FnScenegraphIterator sgIterator,
        std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState)
{
    if (!isVisible(sgIterator))
    {
        return;
    }
    
    FnAttribute::StringAttribute srcPathAttr =
            sgIterator.getAttribute("geometry.instanceSource");
    
    if (!srcPathAttr.isValid())
    {
        AiMsgWarning("[kat] No \"geometry.instanceSource\""
                " attribute found at %s", sgIterator.getFullName().c_str());
    }
    
    InstanceSourceRefPtr instSrc;
    
    std::string rawSrcPath = srcPathAttr.getValue();
    std::string srcPath = pystring::os::path::abspath(
            rawSrcPath, sgIterator.getFullName());
    
    boost::recursive_mutex::scoped_lock lock(instanceSourcesMutex);
    InstanceSourceMap::iterator gI = g_instanceSources.find(srcPath);
    
    if (gI == g_instanceSources.end())
    {
        FnScenegraphIterator sourceIterator;
        
        
        if (pystring::startswith(rawSrcPath, "../"))
        {
            sourceIterator = sgIterator.getByPath(rawSrcPath);
        }
        else
        {
            sourceIterator =
                    sgIterator.getRoot().getByPath(srcPath);
        }
        
        
        if (sourceIterator.isValid() && sourceIterator.getType() == "instance source")
        {
            instSrc = InstanceSourceRefPtr(new InstanceSource);
            
            //go make the child source nodes and then set their visibility
            //to off
            
            std::vector<AtNode*> sourceNodes;
            
            FnScenegraphIterator childIterator =
                    sourceIterator.getFirstChild();
            while (childIterator.isValid())
            {
                buildLocation(childIterator, &sourceNodes,
                        true, //forceExpand
                        true, //noOverride
                        sharedState,
                        false //don't instance
                        );
                childIterator = childIterator.getNextSibling();
            }
            
            //find the primitive nodes
            for (std::vector<AtNode*>::const_iterator I = sourceNodes.begin();
                    I != sourceNodes.end(); ++I)
            {
                AtNode * createdNode = (*I);
                
                if (AiNodeEntryGetType(AiNodeGetNodeEntry(createdNode)) == AI_NODE_SHAPE)
                {
                    InstanceSource::LeafEntryRefPtr leafEntry =
                            InstanceSource::LeafEntryRefPtr(
                                    new InstanceSource::LeafEntry);
                    
                    leafEntry->node = createdNode;
                    
                    //store the natural visibility prior to blanking it
                    leafEntry->srcVisibility = AiNodeGetByte(createdNode, "visibility");
                    
                    AiNodeSetByte(createdNode, "visibility", 0);
                    
                    
                    //find the relative path from the name.
                    //walk the producer again to grab the AOS
                    std::string nodeName = AiNodeGetName(createdNode);
                    std::string relativePath = pystring::slice(nodeName,
                            sourceIterator.getFullName().size() + 1);
                    
                    instSrc->leafMap[relativePath] = leafEntry;
                    
                    FnScenegraphIterator shapeIterator =
                            sourceIterator.getByPath("./"+relativePath);
                    
                    if (shapeIterator.isValid())
                    {
                        FnScenegraphIterator p = shapeIterator;
                        
                        std::vector<GroupAttribute> xformAttrs;
                        while (p.isValid() && p.getFullName() != srcPath)
                        {
                            GroupAttribute a = p.getAttribute("xform");
                            if (a.isValid())
                            {
                                xformAttrs.push_back(a);
                            }
                            p = p.getParent();
                        }
                        
                        if (!xformAttrs.empty())
                        {
                            GroupBuilder gb;
                            
                            int i = 0;
                            for (std::vector<GroupAttribute>::reverse_iterator I =
                                    xformAttrs.rbegin(); I != xformAttrs.rend(); ++I, ++i)
                            {
                                std::ostringstream name;
                                name << "xform" << i;
                                
                                gb.set(name.str(), (*I));
                            }
                            
                            leafEntry->xformAttr = gb.build();
                            
                        }
                        
                        
                        leafEntry->geometryAttr =
                                shapeIterator.getAttribute("geometry");
                        
                        leafEntry->locationType = shapeIterator.getType();
                    }
                }
            }
            
            if (childNodes)
            {
                (*childNodes).reserve(childNodes->size() + sourceNodes.size());
                
                childNodes->insert(childNodes->end(),
                        sourceNodes.begin(), sourceNodes.end());
            }
        }
        
        g_instanceSources[srcPath] = instSrc;
    }
    else
    {
        instSrc = (*gI).second;
    }
    lock.unlock();
    
    if (!instSrc)
    {
        AiMsgWarning("[kat] Cannot find instance source \"%s\" from"
                " %s", srcPath.c_str(), sgIterator.getFullName().c_str());
        return;
    }
    
    //applied the inherited "geometry.arbitrary" of the "instance" location to
    //all of the resulting ginstance locations.
    GroupAttribute geomArbitrary = sgIterator.getAttribute(
            "geometry.arbitrary", true);
    
    GroupAttribute instanceXform = sgIterator.getGlobalXFormGroup();
    
    for (InstanceSource::LeafMap::iterator I = instSrc->leafMap.begin();
            I != instSrc->leafMap.end(); ++I)
    {
        const std::string & relativePath = (*I).first;
        
        
        InstanceSource::LeafEntryRefPtr leafEntry = (*I).second;
        
        AtNode* instanceNode = AiNode("ginstance");
        if (childNodes)
        {
            childNodes->push_back(instanceNode);
        }
        
        AiNodeSetStr(instanceNode, "name",
                (sgIterator.getFullName() + "/" + relativePath).c_str());
        AiNodeSetPtr(instanceNode, "node", leafEntry->node);
        AiNodeSetBool(instanceNode, "inherit_xform", false);
        
        GroupBuilder gb;
        
        gb.set("instanceXform", instanceXform);
        
        if (leafEntry->xformAttr.isValid())
        {
            gb.update(leafEntry->xformAttr);
        }
        
        
        
        GroupAttribute fullXformAttr = gb.build();
        
        applyTransform(instanceNode, sgIterator, sharedState, 0, &fullXformAttr );
        
        AiNodeSetByte(instanceNode, "visibility", leafEntry->srcVisibility);
        
        
        if (geomArbitrary.isValid())
        {
            writeArbitrary(instanceNode, sgIterator.getFullName() + relativePath,
                    leafEntry->locationType, leafEntry->geometryAttr,
                            geomArbitrary);
        }
        
        applyObjectId(instanceNode, sgIterator, sharedState);
    }
    
}

