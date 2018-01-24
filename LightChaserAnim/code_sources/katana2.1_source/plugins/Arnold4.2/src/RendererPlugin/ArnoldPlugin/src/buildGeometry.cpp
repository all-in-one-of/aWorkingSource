// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include "arnold_render.h"

#include <string.h>
#include <boost/thread.hpp>
#include <pystring/pystring.h>

#include <FnRenderOutputUtils/FnRenderOutputUtils.h>

#include <buildCamera.h>
#include <buildCommons.h>
#include <FnRender/plugin/ScenegraphLocationDelegate.h>
#include <ArnoldPluginState.h>
#include <objectSettings.h>

using namespace FnKat;

std::string getInstanceID(FnScenegraphIterator sgIterator)
{
    // never instance procedural nodes
    if (sgIterator.getType() == "arnold procedural"
            || sgIterator.getType() == "renderer procedural")
    {
        return "";
    }
    
    FnAttribute::StringAttribute instanceIDAttr = sgIterator.getAttribute("instance.ID");
    
    if (instanceIDAttr.isValid())
    {
         return instanceIDAttr.getValue();
    }
    
    // TODO: remove in Katana 2.1?
    // For backwards compatibility with the initial arnold logic, also check for instanceID.
    // Note: this is deprecated and will be removed from future Katana versions.
    
    instanceIDAttr =  sgIterator.getAttribute("instanceID");
    if (instanceIDAttr.isValid())
    {
        return instanceIDAttr.getValue();
    }
    
    return std::string();
}

namespace
{
    InstanceMap_t global_instanceMap;
    boost::recursive_mutex instanceMapMutex;
}

/******************************************************************************
 *
 * buildLocation
 *
 *Currently no ScenegraphLocationDelegate is defined for locator, instance, camera, light, instance name, error location types
 ******************************************************************************/
void buildLocation(FnScenegraphIterator sgIterator,
        std::vector<AtNode*>* childNodes, bool forceExpand, bool nooverride, ArnoldPluginState* sharedState, bool obeyInstanceId)
{
    std::string type = sgIterator.getType();
    std::string name = sgIterator.getFullName();
    
    FnAttribute::StringAttribute a = sgIterator.getAttribute("message");
    if (a.isValid())
    {
        AiMsgInfo("[kat] %s", a.getValue(
                "Malformed message attribute.", false).c_str());
    }
    
    a = sgIterator.getAttribute("warningMessage");
    if (a.isValid())
    {
        AiMsgWarning("[kat] %s", a.getValue(
                "Malformed warningMessage attribute.", false).c_str());
    }
    
    if (type == "error")
    {
        // error locations cause the render to fail
        FnAttribute::StringAttribute errorMessage = sgIterator.getAttribute("errorMessage");
        AiMsgError("[kat] FATAL: Error location found at '%s'.  %s", sgIterator.getFullName().c_str(),
            errorMessage.getValue("", false).c_str());
        exit(-1);
    }
    else
    {
        FnAttribute::StringAttribute a = sgIterator.getAttribute("errorMessage");
        if (a.isValid())
        {
            AiMsgWarning("[kat] %s", a.getValue(
                    "Malformed errorMessage attribute.", false).c_str());
        }
    }
    
    if (type == "light" || type == "instance source")
    {
        return;
    }
    
    if (type == "camera")
    {
        AtNode* cam = buildCameraLocation(sgIterator, sharedState);
        if (cam && childNodes)
        {
            childNodes->push_back(cam);
        }
        
        return;
    }


    if (type == "instance")
    {
        buildInstanceGroup(sgIterator, childNodes, sharedState);
        return;
    }

    
    if (sgIterator.getType() == "locator")
    {
        // TODO: 'transformation' nodes don't exist in SA Arnold
        if (AiNodeEntryLookUp("transformation"))
        {
            AtNode * xformNode = AiNode("transformation");

            if (xformNode)
            {
                AiNodeSetStr(xformNode, "name",
                        sgIterator.getFullName().c_str());

                applyTransform(xformNode, sgIterator, sharedState);

                if (childNodes)
                {
                    childNodes->push_back(xformNode);
                }
            }
        }
        buildGroup(sgIterator, childNodes, forceExpand, nooverride, sharedState, obeyInstanceId);
        FnKat::RenderOutputUtils::emptyFlattenedMaterialCache();
        return;    
    }

    AtNode * primNode = 0, * srcPrimNode = 0;
    std::string instanceID;
    if (obeyInstanceId)
    {
        instanceID = getInstanceID(sgIterator);
    }

    if (!instanceID.empty())
    {
        boost::recursive_mutex::scoped_lock lock(instanceMapMutex);
        InstanceMap_t::iterator I = global_instanceMap.find(instanceID);

        if (I != global_instanceMap.end())
        {
            srcPrimNode = (*I).second;
        }
    }

    if (!srcPrimNode)
    {
        if (type == "nurbspatch" || type == "polymesh" || type == "subdmesh"){
            //allow procedurals beneath, do this regardless of the primitive's visibility
            for (FnScenegraphIterator child = sgIterator.getFirstChild();
                 child.isValid(); child = child.getNextSibling())
            {
                if (child.getType() == "renderer procedural" ||
                    child.getType() == "arnold procedural")
                {
                    buildLocation(child, childNodes, false, false, sharedState);
                }
            }
        }


        // If there is already a node for this location we will not create a new
        // one, otherwise we will call the delegate plugin to do it.
        primNode = AiNodeLookUpByName(name.c_str());
        if(primNode)
        {
           AiMsgDebug("[kat] '%s' is already in the scene, skipping it.", name.c_str());
        }
        else
        {
           // Call the Delegate plugin to generate the node for this location
           ArnoldSceneGraphLocationDelegateInput sceneGraphLocationDelegateInput;
           sceneGraphLocationDelegateInput.childNodes = childNodes;
           sceneGraphLocationDelegateInput.sharedState = sharedState;

           bool pluginFound = RenderOutputUtils::processLocation(sgIterator, "arnold", type, &sceneGraphLocationDelegateInput, (void**)(&primNode));

           if(pluginFound){
               if(!primNode){
                   std::string msg;
                   msg += "[kat] DEBUG: plug-in for type ";
                   msg += type.c_str();
                   msg += " returned null for ";
                   msg += name;
                   AiMsgDebug(msg.c_str());
                   return;
               }
           }
        }

        if (primNode)
        {
            std::string primName = sgIterator.getFullName();

            if (!instanceID.empty()) //intended to be instanced
            {
                boost::recursive_mutex::scoped_lock lock(instanceMapMutex);
                global_instanceMap[instanceID] = primNode;
                primName += ":ref";
                AiNodeSetInt(primNode, "visibility", 0);
                srcPrimNode = primNode;
            }

            AiNodeSetStr(primNode, "name", primName.c_str());
        }
    }

    if (srcPrimNode)
    {
        // If srcPrimNode is set but primNode is not, it means that we are an instance
        // of something already created. Since we're not going to call buildPolymesh/etc,
        // we'll need to give any procedurals beneath us an opportunity to build.
        if (!primNode)
        {
            //allow procedurals beneath polymeshes, do this regardless of the primitive's visibility
            for (FnScenegraphIterator child = sgIterator.getFirstChild();
                    child.isValid(); child = child.getNextSibling())
            {
                if (child.getType() == "renderer procedural" ||
                        child.getType() == "arnold procedural")
                {
                    buildLocation(child, childNodes, false, false, sharedState);
                }
            }
        }

        // If srcPrimNode is set but we're not visible, it means that we're an invisible
        // instance of something that was visible and already created -- otherwise
        // srcPrimNode would never have been set. In that case, we need to return.
        if (!isVisible(sgIterator))
        {
            return;
        }


        AtNode* instanceNode = AiNode("ginstance");
        if (childNodes)
        {
            childNodes->push_back(instanceNode);
        }

        AiNodeSetStr(instanceNode, "name",
                sgIterator.getFullName().c_str());
        AiNodeSetPtr(instanceNode, "node", srcPrimNode);
        AiNodeSetBool(instanceNode, "inherit_xform", false);

        applyObjectSettings(sgIterator, instanceNode, childNodes, sharedState);
    }
    
    if(!primNode && !srcPrimNode)
    {
        buildGroup(sgIterator, childNodes, forceExpand, nooverride, sharedState, obeyInstanceId);
    }

    FnKat::RenderOutputUtils::emptyFlattenedMaterialCache();
}

void buildGeometry(FnScenegraphIterator rootIterator, bool forceExpand, bool nooverride, ArnoldPluginState* sharedState)
{
    // Walk down to world, write from there.
    FnScenegraphIterator worldIterator = rootIterator.getChildByName("world");

    if (!worldIterator.isValid())
    {
        AiMsgError("[kat] FATAL: Could not find world.");
        exit(-1);
    }

    AiMsgInfo("[kat] Building scene.");
    buildCameras(worldIterator, sharedState);
    buildLights(worldIterator, 0, sharedState);
    
    
    FnAttribute::StringAttribute whenAttr = rootIterator.getAttribute(
        "arnoldGlobalStatements.assIncludeWhen");
    if (whenAttr.isValid())
    {
        if (whenAttr.getValue("", false) == "following lights")
        {
            processAssIncludes(rootIterator, sharedState);
        }
    }
    
    buildLocation(worldIterator, 0, forceExpand, nooverride, sharedState);
    AiMsgInfo("[kat] Building scene done.");
}
