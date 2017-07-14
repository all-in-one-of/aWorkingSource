// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <objectSettings.h>
#include <buildCommons.h>
#include <pystring/pystring.h>
#include <buildMaterial.h>
#include <objectId.h>

#include <string.h>

using namespace FnKat;

void linkLights(FnScenegraphIterator sgIterator, AtNode* arnoldNode, ArnoldPluginState* sharedState)
{
    if(!sgIterator.isValid() || !arnoldNode) return;

    //we will stuff all lights whihc are "ON" in here
    std::vector<AtNode*> lights;
    std::vector<AtNode*> geoShadowLights;

    GroupAttribute globalLightList = sgIterator.getAttribute("lightList", true);
    if(!globalLightList.isValid()) return;


    bool found = false;
    bool matchesEveryLight = true;

    //TODO, handle "shadow_group" in here as well
    bool foundShadowGroup = false;

    // Walk through the light list, querying local enable flags
    for (int i=0; i<globalLightList.getNumberOfChildren(); ++i)
    {
        // Get the locally defined lightList Attr
        GroupAttribute lightGroup = globalLightList.getChildByIndex(i);
        if (!lightGroup.isValid()) continue;

        // Get the globally defined lightList.<name>.path attribute
        FnAttribute::StringAttribute pathattr = lightGroup.getChildByName("path");
        if (!pathattr.isValid()) continue;

        // Get the locally defined 'enable' attr
        FnAttribute::IntAttribute enableAttr = lightGroup.getChildByName("enable");
        if (enableAttr.isValid())
        {
            found = true;
        }

        if (enableAttr.isValid())
        {
            if  (enableAttr.getValue())
            {
                // lookup arnold light pointer
                AtNode* lightHandle = sharedState->getLightHandleForLocation(pathattr.getValue());
                if (lightHandle)
                {
                    lights.push_back(lightHandle);
                }
            }
            else
            {
                matchesEveryLight = false;
            }
        }


        FnAttribute::IntAttribute geoShadowEnableAttr =
            lightGroup.getChildByName("geoShadowEnable");

        if (geoShadowEnableAttr.isValid())
        {
            if (geoShadowEnableAttr.getValue())
            {
                AtNode* lightHandle = sharedState->getLightHandleForLocation(pathattr.getValue());
                if (lightHandle)
                {
                    geoShadowLights.push_back(lightHandle);
                }
            }
            else
            {
                //only add the list if we find one which ISN'T on
                //if they're all on, leave as-is
                foundShadowGroup = true;
            }
        }
        else
        {
            AtNode* lightHandle = sharedState->getLightHandleForLocation(pathattr.getValue());
            if (lightHandle)
            {
                geoShadowLights.push_back(lightHandle);
            }
        }
    }

    //if we found light lists that are on,
    //set light_group and turn light linking on
    if (found && !matchesEveryLight)
    {
        //any lights?
        if(lights.size())
            AiNodeSetArray(arnoldNode, "light_group", AiArrayConvert(lights.size(), 1, AI_TYPE_POINTER, &lights[0]));
        //no lights, make empty array
        else
            AiNodeSetArray(arnoldNode, "light_group", AiArray(0, 0, AI_TYPE_POINTER));

        //turn on light linking
        AiNodeSetBool(arnoldNode, "use_light_group", true);
    }

    if (foundShadowGroup)
    {
        if (!geoShadowLights.empty())
        {
            AiNodeSetArray(arnoldNode, "shadow_group",
                    AiArrayConvert(geoShadowLights.size(), 1,
                            AI_TYPE_POINTER, &geoShadowLights[0]));
        }
        else
        {
            AiNodeSetArray(arnoldNode, "shadow_group",
                    AiArray(0, 0, AI_TYPE_POINTER));
        }

        AiNodeSetBool(arnoldNode, "use_shadow_group", true);
    }
}

void applyObjectSettings(FnScenegraphIterator sgIterator, AtNode * leafNode,
        std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState)
{
    GroupAttribute arnoldStatementsGroup =
            sgIterator.getAttribute("arnoldStatements", true);

    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(leafNode);

    bool smoothing = false;
    if (arnoldStatementsGroup.isValid())
    {
        FnAttribute::IntAttribute intAttr = arnoldStatementsGroup.getChildByName("smoothing");
        if (intAttr.isValid())
        {
            smoothing = intAttr.getValue();
        }
        else //special case to handle legacy float values for this parameter
        {
            FnAttribute::FloatAttribute smoothingAttr = arnoldStatementsGroup.getChildByName("smoothing");
            if (smoothingAttr.isValid())
            {
                smoothing = (smoothingAttr.getValue() != 0);
            }
        }
    }

    std::string type = sgIterator.getType();
    std::string name = sgIterator.getFullName();

    if (type == "polymesh" || type == "subdmesh")
    {
        if (smoothing &&
                AiNodeEntryLookUpParameter(baseNodeEntry, "smoothing"))
        {
            AiNodeSetBool(leafNode, "smoothing", smoothing);
        }
    }
    
    if (type == "volume")
    {
        if (AiNodeEntryLookUpParameter(baseNodeEntry, "step_size"))
        {
            FloatAttribute stepSizeAttr =
                arnoldStatementsGroup.getChildByName("step_size");

            if (stepSizeAttr.isValid())
            {
                AiNodeSetFlt(leafNode, "step_size", stepSizeAttr.getValue());
            }
        }
    }

    //by default visible to all rays
    AtByte visibilityMask = AI_RAY_ALL;
    AtByte autobumpVisibilityMask = AI_RAY_ALL;

    //handle AtNodePrimitive base parameters
    if (arnoldStatementsGroup.isValid())
    {
        //self_shadows
        FnAttribute::IntAttribute selfShadowsAttr =
            arnoldStatementsGroup.getChildByName("self_shadows");
        if (selfShadowsAttr.isValid())
        {
            AiNodeSetBool(leafNode, "self_shadows",
                selfShadowsAttr.getValue());
        }

        //receive_shadows
        FnAttribute::IntAttribute receiveShadowsAttr =
            arnoldStatementsGroup.getChildByName("receive_shadows");
        if (receiveShadowsAttr.isValid())
        {
            AiNodeSetBool(leafNode, "receive_shadows",
                receiveShadowsAttr.getValue());
        }

        //invert_normals
        FnAttribute::IntAttribute invNormalsAttr =
            arnoldStatementsGroup.getChildByName("invert_normals");
        if (invNormalsAttr.isValid())
        {
            AiNodeSetBool(leafNode, "invert_normals", invNormalsAttr.getValue());
        }

        //opaque
        FnAttribute::IntAttribute opaqueAttr =
            arnoldStatementsGroup.getChildByName("opaque");
        if (opaqueAttr.isValid())
        {
            AiNodeSetBool(leafNode, "opaque", opaqueAttr.getValue());
        }

        //matte
        FnAttribute::IntAttribute matteAttr =
            arnoldStatementsGroup.getChildByName("matte");
        if (matteAttr.isValid())
        {
            AiNodeSetBool(leafNode, "matte", matteAttr.getValue());
        }


        if (type == "nurbspatch")
        {
            FnAttribute::IntAttribute tesselateUAttr =
                arnoldStatementsGroup.getChildByName("nurbs_tesselate_u");
            if (tesselateUAttr.isValid())
            {
                AiNodeSetInt(leafNode, "tesselate_u",
                    tesselateUAttr.getValue());
            }

            FnAttribute::IntAttribute tesselateVAttr =
                arnoldStatementsGroup.getChildByName("nurbs_tesselate_v");
            if (tesselateVAttr.isValid())
            {
                AiNodeSetInt(leafNode, "tesselate_v",
                    tesselateVAttr.getValue());
            }
        }

        //visibility
        GroupAttribute visibilityGroup =
            arnoldStatementsGroup.getChildByName("visibility");
        //legacy attr support
        if (!visibilityGroup.isValid())
        {
            visibilityGroup = arnoldStatementsGroup.getChildByName("Visibility");
        }

        if (visibilityGroup.isValid())
        {
            FnAttribute::IntAttribute rayAttr =
                visibilityGroup.getChildByName("AI_RAY_CAMERA");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                visibilityMask &= ~AI_RAY_CAMERA;
            }

            rayAttr = visibilityGroup.getChildByName("AI_RAY_SHADOW");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                visibilityMask &= ~AI_RAY_SHADOW;
            }

            rayAttr = visibilityGroup.getChildByName("AI_RAY_REFLECTED");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                visibilityMask &= ~AI_RAY_REFLECTED;
            }

            rayAttr = visibilityGroup.getChildByName("AI_RAY_REFRACTED");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                visibilityMask &= ~AI_RAY_REFRACTED;
            }

            rayAttr = visibilityGroup.getChildByName("AI_RAY_SUBSURFACE");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                visibilityMask &= ~AI_RAY_SUBSURFACE;
            }

            rayAttr = visibilityGroup.getChildByName("AI_RAY_DIFFUSE");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                visibilityMask &= ~AI_RAY_DIFFUSE;
            }

            rayAttr = visibilityGroup.getChildByName("AI_RAY_GLOSSY");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                visibilityMask &= ~AI_RAY_GLOSSY;
            }
        }

        //autobump_visibility
        GroupAttribute autobumpVisibilityGroup =
            arnoldStatementsGroup.getChildByName("autobump_visibility");
        if (autobumpVisibilityGroup.isValid() &&
            AiNodeEntryLookUpParameter(baseNodeEntry, "autobump_visibility"))
        {
            FnAttribute::IntAttribute rayAttr =
                autobumpVisibilityGroup.getChildByName("AI_RAY_CAMERA");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                autobumpVisibilityMask &= ~AI_RAY_CAMERA;
            }

            rayAttr = autobumpVisibilityGroup.getChildByName("AI_RAY_SHADOW");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                autobumpVisibilityMask &= ~AI_RAY_SHADOW;
            }

            rayAttr = autobumpVisibilityGroup.getChildByName("AI_RAY_REFLECTED");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                autobumpVisibilityMask &= ~AI_RAY_REFLECTED;
            }

            rayAttr = autobumpVisibilityGroup.getChildByName("AI_RAY_REFRACTED");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                autobumpVisibilityMask &= ~AI_RAY_REFRACTED;
            }

            rayAttr = autobumpVisibilityGroup.getChildByName("AI_RAY_SUBSURFACE");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                autobumpVisibilityMask &= ~AI_RAY_SUBSURFACE;
            }

            rayAttr = autobumpVisibilityGroup.getChildByName("AI_RAY_DIFFUSE");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                autobumpVisibilityMask &= ~AI_RAY_DIFFUSE;
            }

            rayAttr = autobumpVisibilityGroup.getChildByName("AI_RAY_GLOSSY");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                autobumpVisibilityMask &= ~AI_RAY_GLOSSY;
            }

            if (autobumpVisibilityMask != (AI_RAY_ALL & ~(AI_RAY_DIFFUSE | AI_RAY_GLOSSY)))
            {
                AiNodeSetByte(leafNode, "autobump_visibility", autobumpVisibilityMask);
            }
        }

        //sidedness
        GroupAttribute sidednessGroup =
            arnoldStatementsGroup.getChildByName("sidedness");
        if (sidednessGroup.isValid())
        {
            AtByte sidedness = AI_RAY_ALL;

            FnAttribute::IntAttribute rayAttr =
                    sidednessGroup.getChildByName("AI_RAY_CAMERA");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                sidedness &= ~AI_RAY_CAMERA;
            }

            rayAttr = sidednessGroup.getChildByName("AI_RAY_SHADOW");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                sidedness &= ~AI_RAY_SHADOW;
            }

            rayAttr = sidednessGroup.getChildByName("AI_RAY_REFLECTED");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                sidedness &= ~AI_RAY_REFLECTED;
            }

            rayAttr = sidednessGroup.getChildByName("AI_RAY_REFRACTED");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                sidedness &= ~AI_RAY_REFRACTED;
            }

            rayAttr = sidednessGroup.getChildByName("AI_RAY_SUBSURFACE");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                sidedness &= ~AI_RAY_SUBSURFACE;
            }

            rayAttr = sidednessGroup.getChildByName("AI_RAY_DIFFUSE");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                sidedness &= ~AI_RAY_DIFFUSE;
            }

            rayAttr = sidednessGroup.getChildByName("AI_RAY_GLOSSY");
            if (rayAttr.isValid() && !rayAttr.getValue())
            {
                sidedness &= ~AI_RAY_GLOSSY;
            }

            if (sidedness != AI_RAY_ALL)
            {
                AiNodeSetByte(leafNode, "sidedness", sidedness);
            }
        }

        //tracesets
        FnAttribute::StringAttribute tracesetsAttr =
            arnoldStatementsGroup.getChildByName("trace_sets");
        if (tracesetsAttr.isValid())
        {
            std::vector<std::string> tracesets;
            pystring::split(tracesetsAttr.getValue(), tracesets, ",");
            for(size_t i = 0; i < tracesets.size(); i++)
            {
                tracesets[i] = pystring::strip(tracesets[i]);
            }
            std::vector<const char*> stringVector;
            stringVector.resize(tracesets.size());
            for(size_t i = 0; i < tracesets.size(); i++)
            {
                stringVector[i] = tracesets[i].c_str();
            }
            AiNodeSetArray(leafNode,"trace_sets",
                AiArrayConvert(stringVector.size(), 1,
                    AI_TYPE_STRING,
                    &(stringVector[0])));
        }

        static const char * boolParamNames[] = {
            "autobump",
            0
        };


        for (int i = 0; boolParamNames[i]; ++i)
        {
            const char * name = boolParamNames[i];
            FnAttribute::IntAttribute attr = arnoldStatementsGroup.getChildByName(name);

            if (!attr.isValid()) continue;

            if (AiNodeEntryLookUpParameter(baseNodeEntry, name))
            {
                AiNodeSetBool(leafNode, name, attr.getValue());
            }
        }

        buildArbitraryStatements(arnoldStatementsGroup, leafNode);
    }

    //set visibility
    AiNodeSetByte(leafNode, "visibility", visibilityMask);
    if (!strcmp(AiNodeEntryGetName(baseNodeEntry), "ginstance"))
    {
        setTextureUserDataForInstance(sgIterator, leafNode, sharedState);

        //set arbitrary
        GroupAttribute geometryAttr = sgIterator.getAttribute("geometry", true);
        if (geometryAttr.isValid())
        {
            GroupAttribute arbAttr = geometryAttr.getChildByName("arbitrary");
            if (arbAttr.isValid())
            {
                writeArbitrary(leafNode,
                        sgIterator.getFullName(), sgIterator.getType(),
                                geometryAttr, arbAttr);
            }
        }
    }

    //set transfrom
    applyTransform(leafNode, sgIterator, sharedState);

    //set light links
    linkLights(sgIterator, leafNode, sharedState);

    //we now apply the material(s) to the instance
    applyMaterial(leafNode, sgIterator, childNodes, sharedState);

    //apply "id" attribute (only does work if initObjectId has been called during setup)
    applyObjectId(leafNode, sgIterator, sharedState);
}
