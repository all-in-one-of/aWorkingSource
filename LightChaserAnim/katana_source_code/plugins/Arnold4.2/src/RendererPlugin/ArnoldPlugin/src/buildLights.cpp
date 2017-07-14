// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include "arnold_render.h"
#include "arnold_capabilities.h"

#include <FnAttribute/FnGroupBuilder.h>

#include <OpenEXR/ImathMatrixAlgo.h>

#include <string.h>

#include <pystring/pystring.h>

#include <buildMaterial.h>
#include <buildCommons.h>

using namespace FnKat;

typedef struct
{
    std::vector<AtPoint> position;
    std::vector<AtPoint> look_at;
    std::vector<AtPoint> up;
    std::vector<AtPoint> dir;
    
    //for quad lights
    std::vector<AtPoint> origin;
    std::vector<AtPoint> edge_x;
    std::vector<AtPoint> edge_y;
    
} LightXformVectors;

void calcLightXformVectors(const std::map<float, Imath::M44d> & xformSamples,
        LightXformVectors & output, std::vector<float> &sampleTimes)
{
    output.position.clear();
    output.look_at.clear();
    output.up.clear();
    output.dir.clear();
    
    
    output.origin.clear();
    output.edge_x.clear();
    output.edge_y.clear();
    
    
    Imath::Vec3<double> o(1, -1, 0);
    Imath::Vec3<double> x(-1,  -1, 0 );
    Imath::Vec3<double> y(1,  1, 0 );
    
    Imath::M44d lastM;
    bool b = false;
    for(std::map<float, Imath::M44d>::const_iterator it=xformSamples.begin(); it!=xformSamples.end(); ++it)
    {
        Imath::M44d xform = it->second;
        
        if(b && lastM == xform) continue;
        else
        {    
            b = true;
            lastM = xform;
        }
        
        sampleTimes.push_back(it->first);
        
        AtPoint p;
        AtVector v;
        
        AtPoint trg;
        AtPoint pos;
        
        
        //pos
        AiV3Create(p, xform[3][0],  xform[3][1], xform[3][2]);
        output.position.push_back(p);
        pos = p;
        
        
        //look at, trg
        AiV3Create(p, xform[3][0] - xform[2][0], xform[3][1] - xform[2][1], xform[3][2] - xform[2][2]);
        output.look_at.push_back(p);
        trg = p;
        
        //up
        AiV3Create(p, xform[1][0],  xform[1][1], xform[1][2]);
        output.up.push_back(p);
        
        
        AiV3Sub(v, trg, pos);
        output.dir.push_back(v);
        
        
        Imath::Vec3<double> oOut, xOut, yOut;
        xform.multVecMatrix(o, oOut);
        xform.multVecMatrix(x, xOut);
        xform.multVecMatrix(y, yOut);
        
        AiV3Create(p, oOut[0], oOut[1], oOut[2]);
        output.origin.push_back(p);
        
        AiV3Create(v, xOut[0]-oOut[0], xOut[1]-oOut[1], xOut[2]-oOut[2]);
        output.edge_x.push_back(v);
        
        AiV3Create(v, yOut[0]-oOut[0], yOut[1]-oOut[1], yOut[2]-oOut[2]);
        output.edge_y.push_back(v);
        
        
    }
}

///////////////////////////////////////////////////////////////////////////////

void applySkydomeTransform(AtNode* node, FnScenegraphIterator sgIterator, ArnoldPluginState* sharedState)
{
    std::map<float, Imath::M44d> samples;
    getTransformSamples(&samples, sgIterator, sharedState);
    
    //check to see that we're not a single identity matrix
    if (samples.size() > 1 || samples.begin()->second != Imath::M44d())
    {
        std::vector<float> sampleTimes;
        sampleTimes.reserve(samples.size());
        
        std::vector<float> mlist;
        mlist.reserve(16*samples.size());
        
        for (std::map<float, Imath::M44d>::iterator it = samples.begin();
                 it != samples.end(); ++it)
        {
            //TODO, decompose the matrix, rebuilt it without transform
            //with only signed scale and with rotation
            
            Imath::V3d s, h, r, t;
            Imath::extractSHRT((*it).second, s, h, r, t);
            
            for (int i = 0; i < 3; ++i)
            {
                if (s[i] < 0)
                {
                    s[i] = -1;
                }
                else
                {
                    s[i] = 1;
                }
            }
            
            Imath::M44d m;
            m.setScale(s);
            m.rotate(r);
            
            sampleTimes.push_back((*it).first);
            for(int i = 0; i < 16; i++)
            {
                mlist.push_back(m.getValue()[i]);
            }
        }
        
        AiNodeSetArray(node, "matrix",
                AiArrayConvert(1, samples.size(),
                        AI_TYPE_MATRIX, &mlist[0]));
        
        if (sampleTimes.size() > 1 )
        {
            const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);
            if (AiNodeEntryLookUpParameter(baseNodeEntry, "time_samples"))
            {
                AiNodeSetArray(node, "time_samples",
                        AiArrayConvert(sampleTimes.size(), 1, AI_TYPE_FLOAT,
                                &sampleTimes[0]));
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////


//Work-in-progress for shared code path between build and rerendering for OSL lights
void setLightXform(
        AtNode * lightNode, const std::map<float, Imath::M44d> & xformSamples)
{
    std::vector<float> sampleTimes;
    sampleTimes.reserve(xformSamples.size());
    
    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(lightNode);

    LightXformVectors xv;
    calcLightXformVectors(xformSamples, xv, sampleTimes);
    
    if (sampleTimes.size() > 1 &&
                    (sampleTimes.front() != 0 || sampleTimes.back() != 1))
    {
        
        if (AiNodeEntryLookUpParameter(baseNodeEntry, "transform_time_samples"))
        {
            AiNodeSetArray(lightNode, "transform_time_samples",
                        AiArrayConvert(sampleTimes.size(), 1, AI_TYPE_FLOAT,
                                &sampleTimes[0]));
        }
    }
    else
    {
        //if there's an array set, unset it
        //AiNodeSetArray(lightNode, "transform_time_samples",
        //            AiArrayConvert(0, 1, AI_TYPE_FLOAT,
        //                    0));
    }
    
    std::string nodeType = AiNodeEntryGetName(baseNodeEntry);
    
    
    if (nodeType == "quad")
    {
        AiNodeSetArray(lightNode, "origin", AiArrayConvert(1,
                sampleTimes.size(), AI_TYPE_POINT, &xv.origin[0]));
        AiNodeSetArray(lightNode, "edge_x", AiArrayConvert(1,
                sampleTimes.size(), AI_TYPE_VECTOR, &xv.edge_x[0]));
        AiNodeSetArray(lightNode, "edge_y", AiArrayConvert(1,
                sampleTimes.size(), AI_TYPE_VECTOR, &xv.edge_y[0]));
        
        AiNodeDeclare(lightNode, "lgtWidth", "constant float");
        AiNodeSetFlt(lightNode, "lgtWidth",
                Imath::Vec3<float>(xv.edge_x[0].x, xv.edge_x[0].y, xv.edge_x[0].z).length());
        
        AiNodeDeclare(lightNode, "lgtHeight", "constant float");
        AiNodeSetFlt(lightNode, "lgtHeight",
                Imath::Vec3<float>(xv.edge_y[0].x, xv.edge_y[0].y, xv.edge_y[0].z).length());
    }
    
    
    if (const AtParamEntry * pentry =
            AiNodeEntryLookUpParameter(baseNodeEntry, "center"))
    {
        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
        {
            AiNodeSetArray(lightNode, "center",
                    AiArrayConvert(1, xv.position.size(), AI_TYPE_POINT,
                             &xv.position[0]));
        }
        else
        {
            AiNodeSetPnt(lightNode, "center", xv.position[0].x, xv.position[0].y,
                    xv.position[0].z);
        }
    }
    
    if (const AtParamEntry * pentry =
            AiNodeEntryLookUpParameter(baseNodeEntry, "direction"))
    {
        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
        {
            int arrayType = AI_TYPE_VECTOR;
            const AtParamValue* dval = AiParamGetDefault(pentry);
            if (dval->ARRAY)
            {
                arrayType = dval->ARRAY->type;
            }
            
            
            AiNodeSetArray(lightNode, "direction",
                    AiArrayConvert(1, xv.dir.size(), arrayType,
                             &xv.dir[0]));
        }
        else if (AiParamGetType(pentry) == AI_TYPE_POINT)
        {
            AiNodeSetPnt(lightNode, "direction", xv.dir[0].x, xv.dir[0].y,
                    xv.dir[0].z);
        }
        else if (AiParamGetType(pentry) == AI_TYPE_VECTOR)
        {
            AiNodeSetVec(lightNode, "direction", xv.dir[0].x, xv.dir[0].y,
                    xv.dir[0].z);
        }
    }
    
    if (const AtParamEntry * pentry =
            AiNodeEntryLookUpParameter(baseNodeEntry, "up"))
    {
        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
        {
            int arrayType = AI_TYPE_VECTOR;
            const AtParamValue* dval = AiParamGetDefault(pentry);
            if (dval->ARRAY)
            {
                arrayType = dval->ARRAY->type;
            }
            
            AiNodeSetArray(lightNode, "up",
                    AiArrayConvert(1, xv.up.size(), arrayType,
                             &xv.up[0]));
        }
        else if (AiParamGetType(pentry) == AI_TYPE_POINT)
        {
            AiNodeSetPnt(lightNode, "up", xv.up[0].x, xv.up[0].y,
                    xv.up[0].z);
        }
        else if (AiParamGetType(pentry) == AI_TYPE_VECTOR)
        {
            AiNodeSetVec(lightNode, "up", xv.up[0].x, xv.up[0].y,
                    xv.up[0].z);
        }
    }
    
    if (const AtParamEntry * pentry =
            AiNodeEntryLookUpParameter(baseNodeEntry, "normal"))
    {
        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
        {
            AiNodeSetArray(lightNode, "normal",
                    AiArrayConvert(1, xv.dir.size(), AI_TYPE_VECTOR,  &xv.dir[0]));
        }
        else
        {
            AiNodeSetVec(lightNode, "normal", xv.dir[0].x, xv.dir[0].y, xv.dir[0].z);
        }
    }
    
    if (AiNodeEntryLookUpParameter(baseNodeEntry, "sky_matrix"))
    {
        AtMatrix m;
        
        Imath::M44d inputM = xformSamples.begin()->second;
        Imath::V3d scl, shr;
        Imath::extractAndRemoveScalingAndShear(inputM, scl, shr);
        
        M44d_to_AtMatrix(m, inputM);
        AiNodeSetMatrix(lightNode, "sky_matrix", m);
    }
    
}

void setLightGeometry(AtNode * lightNode, GroupAttribute geoAttr)
{
    if (!lightNode) return;

    FnAttribute::IntAttribute intAttr;
    FnAttribute::FloatAttribute floatAttr;

    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(lightNode);

    if (AiNodeEntryLookUpParameter(baseNodeEntry, "radius"))
    {
        AiNodeSetFlt(lightNode, "radius", 1);
        
        AiNodeDeclare(lightNode, "lgtRadius", "constant float");
        
        if (geoAttr.isValid())
        {
            floatAttr = geoAttr.getChildByName("radius");
            if (floatAttr.isValid())
            {
                AiNodeSetFlt(lightNode, "radius", floatAttr.getValue());
                
                AiNodeSetFlt(lightNode, "lgtRadius", floatAttr.getValue());
            }
        }
        else
        {
            AiNodeSetFlt(lightNode, "lgtRadius", 1);
        }
    }
    
    if (lightNode)
    {
        intAttr = geoAttr.getChildByName("samples");
        if (intAttr.isValid())
        {
            if (AiNodeEntryLookUpParameter(baseNodeEntry, "emit_samples"))
            {
                AiNodeSetInt(lightNode, "emit_samples", intAttr.getValue());
            }
        }
        
        intAttr = geoAttr.getChildByName("resolution");
        if (intAttr.isValid())
        {
            if (AiNodeEntryLookUpParameter(baseNodeEntry, "resolution"))
            {
                AiNodeSetInt(lightNode, "resolution", intAttr.getValue());
            }
            
            if (AiNodeEntryLookUpParameter(baseNodeEntry, "emit_resolution"))
            {
                AiNodeSetInt(lightNode, "emit_resolution", intAttr.getValue());
            }
        }
        
        intAttr = geoAttr.getChildByName("emit_resolution");
        if (intAttr.isValid())
        {
            if (AiNodeEntryLookUpParameter(baseNodeEntry, "emit_resolution"))
            {
                AiNodeSetInt(lightNode, "emit_resolution", intAttr.getValue());
            }
        }
        
        
        if (AiNodeEntryLookUpParameter(baseNodeEntry, "emit_sample_scale"))
        {
            floatAttr = geoAttr.getChildByName("emit_sample_scale");
            if (floatAttr.isValid())
            {
                AiNodeSetFlt(lightNode, "emit_sample_scale", floatAttr.getValue());
            }
            else
            {
                AiNodeSetFlt(lightNode, "emit_sample_scale", 1.0);
            }
        }
        
        floatAttr = geoAttr.getChildByName("emit_bounding_cone");
        if (floatAttr.isValid())
        {
            if (AiNodeEntryLookUpParameter(baseNodeEntry, "emit_bounding_cone"))
            {
                AiNodeSetFlt(lightNode, "emit_bounding_cone", floatAttr.getValue());
            }
        }
        else
        {
            if (AiNodeEntryLookUpParameter(baseNodeEntry, "emit_bounding_cone"))
            {
                AiNodeSetFlt(lightNode, "emit_bounding_cone", 360);
            }
        }
    }
}

// TODO: function is unused?
void setLightVisibility(AtNode * node, const char * paramName, GroupAttribute visAttr)
{
    AtByte visibilityMask = AI_RAY_ALL;
    visibilityMask &= ~AI_RAY_CAMERA;
    visibilityMask &= ~AI_RAY_SHADOW;
    visibilityMask &= ~AI_RAY_REFRACTED;
    
    if (visAttr.isValid())
    {
        //only act if these are set to true {
        FnAttribute::IntAttribute rayAttr = visAttr.getChildByName("AI_RAY_CAMERA");
        if (rayAttr.isValid() && rayAttr.getValue())
        {
            visibilityMask |= AI_RAY_CAMERA;
        }
        
        rayAttr = visAttr.getChildByName("AI_RAY_SHADOW");
        if (rayAttr.isValid() && rayAttr.getValue())
        {
            visibilityMask |= AI_RAY_SHADOW;
        }
        
        rayAttr = visAttr.getChildByName("AI_RAY_REFRACTED");
        if (rayAttr.isValid() && rayAttr.getValue())
        {
            visibilityMask |= AI_RAY_REFRACTED;
        }
        
        //}
        
        //only act if these are set to false {
        
        rayAttr = visAttr.getChildByName("AI_RAY_REFLECTED");
        if (rayAttr.isValid() && !rayAttr.getValue())
        {
            visibilityMask &= ~AI_RAY_REFLECTED;
        }
        
        rayAttr = visAttr.getChildByName("AI_RAY_DIFFUSE");
        if (rayAttr.isValid() && !rayAttr.getValue())
        {
            visibilityMask &= ~AI_RAY_DIFFUSE;
        }
        
        rayAttr = visAttr.getChildByName("AI_RAY_GLOSSY");
        if (rayAttr.isValid() && !rayAttr.getValue())
        {
            visibilityMask &= ~AI_RAY_GLOSSY;
        }
        
        //}
    }
    
    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);
    if (AiNodeEntryLookUpParameter(baseNodeEntry, paramName))
    {
        AiNodeSetByte(node, paramName, visibilityMask);
    }
}





/******************************************************************************
 *
 * buildLight - process just a single light
 *
 ******************************************************************************/
AtNode* buildLight(FnScenegraphIterator lightIterator, std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState)
{
    // Get the material here.
    GroupAttribute materialAttr = lightIterator.getAttribute("material");
    if (!materialAttr.isValid())
    {
        AiMsgInfo("[kat] The light '%s' is missing a material.", lightIterator.getFullName().c_str());
        return NULL;
    }
    
    std::string lightKeyString("arnoldLight");
    
    bool isSkyDome = false;
    
    if (lightIterator.getType() == "geometry material")
    {
        lightKeyString = "arnoldSurface";
        isSkyDome = true;
    }
    
    FnAttribute::StringAttribute lightTypeAttr = materialAttr.getChildByName(lightKeyString+"Shader");
    if (!lightTypeAttr.isValid())
    {
        lightTypeAttr = materialAttr.getChildByName("terminals."+lightKeyString);

        if (!lightTypeAttr.isValid())
        { 
            AiMsgInfo("[kat] The light '%s' is missing the light type.", lightIterator.getFullName().c_str());
            return NULL;
        }
    }
    //getting this information just for warning.
    std::string lightType = lightTypeAttr.getValue();

    //check to see if the node is already created
    
    bool newlyCreated = true;
    
    AtNode* lightNode = AiNodeLookUpByName(
            (lightIterator.getFullName()+":"+lightKeyString+"Shader").c_str());
    
    if (!lightNode)
    {
        lightNode = getShader(lightKeyString, lightIterator, childNodes, true,true,false, sharedState);
    }
    else
    {
        newlyCreated = false;
    }
    
    //AtNode* lightNode = AiNode(lightType.c_str());
    if (!lightNode)
    {
        AiMsgInfo("[kat] The light '%s' of type '%s' could not be created.", lightIterator.getFullName().c_str(), lightType.c_str());
        return NULL;
    }
    
    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(lightNode);
    lightType = AiNodeEntryGetName(baseNodeEntry);
    
    //set name and add to procedural list if we have one
    //AiNodeSetStr(lightNode, "name", lightIterator.getFullName().c_str());
    if (newlyCreated && childNodes)
    {
        childNodes->push_back(lightNode);
    }
    
    std::string lgtname = AiNodeGetStr(lightNode, "name");
    
    
    
    std::map<float, Imath::M44d> xformSamples;
    //don't bother clamping to shutter range because there won't be enough lights
    //for it to matter
    getTransformSamples(&xformSamples, lightIterator, sharedState, false);
    
    //only add it to the light list
    if (!strcmp(AiNodeEntryGetTypeName(baseNodeEntry), "light") ||
        !strcmp(AiNodeEntryGetTypeName(baseNodeEntry), "shape"))
    {
        //add light and state to global light map
        //AiMsgInfo("[kat] adding light %s", lightIterator.getFullName().c_str());
        sharedState->setLightHandle(lgtname, lightNode);
        sharedState->setLightHandleForLocation(lightIterator.getFullName(), lightNode);
    }
    
    
    //If the light node has a "filters" parameter, check for arnoldLightFilter
    if (AiNodeEntryLookUpParameter(baseNodeEntry, "filters"))
    {
        
        std::vector<AtNode *> filterNodes;


        if (AtNode * lightFilterNode = getShader("arnoldLightFilter",
                lightIterator, childNodes, true,true,false, sharedState))
        {
            //AtNode * nodes[] = {lightFilterNode};
            //AiNodeSetArray(lightNode, "filters",
            //        AiArrayConvert(1, 1, AI_TYPE_POINTER, &nodes[0]));
            filterNodes.push_back(lightFilterNode);
        }

        FnAttribute::GroupAttribute coshadersAttr =
                lightIterator.getAttribute("material.arnoldCoshaders");

        if (coshadersAttr.isValid())
        {
            for (int64_t i = 0, e = coshadersAttr.getNumberOfChildren();
                    i < e; ++i)
            {
                if (AtNode * coshaderNode = getShader(
                        "arnoldCoshaders." +  coshadersAttr.getChildName(i),
                                lightIterator, childNodes, true,true,false,
                                        sharedState))
                {
                    filterNodes.push_back(coshaderNode);
                }
            }
        }

        if (!filterNodes.empty())
        {
            AiNodeSetArray(lightNode, "filters",
                    AiArrayConvert(filterNodes.size(), 1, AI_TYPE_POINTER,
                            &filterNodes[0]));
        }
        //materialAttr.getChildByName("terminals."+lightKeyString);


    }
    
    
    
    //NOTE: don't apply the xform to the skydome_light for now. Arnold team might decide
    //to use this in the future. For now, it'd just cause problems
    if (pystring::find(lightType, "skydome_light") > -1)
    {
        applySkydomeTransform(lightNode, lightIterator, sharedState);
    }
    else if (pystring::find(lightType, "quad_light") > -1)
    {
        //TODO: we may be able to yank all special handling for quad lights,
        //it appears we can just use the default applyTransform() and that's it

        //indicates that we are a quad 
        if (AiNodeEntryLookUpParameter(baseNodeEntry, "vertices"))
        {
            //these are the defaults for the quad_light.vertices parameter, but just in case we'll set them explicitly
            float verts[12] =
            {
                -1, -1, 0,
                -1,  1, 0,
                 1,  1, 0,
                 1, -1, 0
            };
            AiNodeSetArray(lightNode, "vertices", AiArrayConvert(4, 1, AI_TYPE_POINT, &verts[0]));
            applyTransform(lightNode, lightIterator, sharedState);
        }
    }
    else
    {
        applyTransform(lightNode, lightIterator, sharedState);
    }
    
    return lightNode;
}

/******************************************************************************
 *
 * buildLights
 *
 ******************************************************************************/
void buildLights(FnScenegraphIterator worldIterator, std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState)
{
    GroupAttribute lightListAttr = worldIterator.getAttribute("lightList");
    if (!lightListAttr.isValid())
    {
        AiMsgInfo("[kat] No lightlist found at /root/world.  No lights will be present in this scene.");
        
        //allow this to go forward with an empty GroupAttr so that the background handling is present
        GroupBuilder gb;
        lightListAttr = gb.build();
    }
    
    std::set<std::string> lightPaths;
    
    for (int i=0; i<lightListAttr.getNumberOfChildren(); ++i)
    {
        FnKat::RenderOutputUtils::emptyFlattenedMaterialCache();

        GroupAttribute lightAttr = lightListAttr.getChildByIndex(i);
        if (!lightAttr.isValid()) continue;

        FnAttribute::StringAttribute lightPathAttr = lightAttr.getChildByName("path");
        
        if (lightPathAttr.isValid())
        {
            std::string lightPath = lightPathAttr.getValue();
            
            lightPaths.insert(lightPath);
            
            FnScenegraphIterator lightIterator;
            
            if (pystring::startswith(lightPath, "/root/world/"))
            {
                lightPath = lightPath.substr(5);
                lightIterator = worldIterator.getByPath(lightPath);
            }
            else
            {
                lightIterator = worldIterator.getRoot().getByPath(lightPath);
            }
            
            if (!lightIterator.isValid())
            {
                AiMsgInfo("[kat] The light '%s' could not be found in the scene.", lightPathAttr.getValue().c_str());
                continue;
            }


            AtNode* lgt = buildLight(lightIterator, childNodes, sharedState);
            if (lgt)
            {
               // Muted lights don't get used.
               FnAttribute::IntAttribute muteAttr = lightIterator.getAttribute("mute", true);
               //Just create light anyway and set intencity 0. to mute light 
               //instead of not creating muted light.
               //This is necessary for interactive rendering.
               if (muteAttr.isValid() && muteAttr.getValue() != 0)
               {
                    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(lgt);
                    if (AiNodeEntryLookUpParameter(baseNodeEntry, "intensity"))
                    {
                        AiNodeSetFlt(lgt, "intensity", 0.);
                    }
                    
                    if (AiNodeEntryLookUpParameter(baseNodeEntry, "emit_samples"))
                    {
                        AiNodeSetInt(lgt, "emit_samples", 0);
                    }
                    
                    if (AiNodeEntryLookUpParameter(baseNodeEntry, "emit_sample_scale"))
                    {
                        AiNodeSetFlt(lgt, "emit_sample_scale", 0);
                    }
               }
            }
        }
    }
    
    
    //for backwards compatibility, check for "background" path that's not in
    //the light list. If it isn't there, build it as a light anyway. This
    //is because GafferPro adds skydomes to the light list while Gaffer does
    //not. We can stop doing this once through the transition.
    
    FnAttribute::StringAttribute backgroundAttr = worldIterator.getRoot().getAttribute(
        "arnoldGlobalStatements.background");
    
    if (backgroundAttr.isValid())
    {
        std::string backgroundPath = backgroundAttr.getValue();
        
        if (lightPaths.find(backgroundPath) == lightPaths.end())
        {
            FnScenegraphIterator lightIterator = worldIterator.getRoot().getByPath(backgroundPath);
            
            if (lightIterator.isValid())
            {
                AtNode* lgt = buildLight(lightIterator, childNodes, sharedState);
                if (lgt)
                {
                    FnAttribute::IntAttribute muteAttr =
                            lightIterator.getAttribute("mute", true);
                    if (muteAttr.isValid() && muteAttr.getValue() != 0)
                    {
                        const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(lgt);
                        if (AiNodeEntryLookUpParameter(baseNodeEntry, "intensity"))
                        {
                            AiNodeSetFlt(lgt, "intensity", 0.);
                        }
                    }
                }
            }
        }
    }
}
