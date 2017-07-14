// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <buildCommons.h>
#include <FnRenderOutputUtils/FnRenderOutputUtils.h>
#include <FnAsset/FnDefaultAssetPlugin.h>
#include "arnold_render.h"

#include <pystring/pystring.h>

#include "arnold_capabilities.h"

#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnAttributeUtils.h>

#include <buildMaterial.h>

#include <FnGeolibServices/FnArbitraryOutputAttr.h>
#include <FnGeolibServices/FnXFormUtil.h>

using namespace FnKat;

void setTextureUserDataForInstance(FnScenegraphIterator sgIterator, AtNode* node, ArnoldPluginState* sharedState)
{
   std::string type = sgIterator.getType();

   if (type == "polymesh" || type == "subdmesh")
   {
      //We need to know how many polygons in reference node but
      //that information is only available right after reference node
      //get created. Once that node is used (ray hit or write to ass file)
      //arnold create internal node and compress its data, as well as
      //removing duplicated data.
      //That point on original data is no longer accessible.
      //Because of this mechanism we need to keep information of
      //number of polygons data ourself here.
      //(because so far nobody else need it)
      int numPolys = 0;
      AtNode* ref = (AtNode*)AiNodeGetPtr(node, "node");
      if (ref)
      {
         int numPolys = sharedState->getNumPolys(AiNodeGetStr(ref, "name"));

         if (numPolys == -1)
         {
            AtArray *array = AiNodeGetArray(ref, "nsides");
            if (array)
            {
               numPolys = array->nelements;
               sharedState->setNumPolys(AiNodeGetStr(ref, "name"), numPolys);
            }
            else
            {
                numPolys = 0;
            }
         }
      } else {
         AiMsgError("[kat] FATAL: (%s) ginstance node cannot find reference node.", AiNodeGetStr(node, "name"));
      }
      setTextureUserDataForPolymesh(sgIterator, node, numPolys);
   }
   else if(type == "nurbspatch")
   {
      setTextureUserDataForNurbspatch(sgIterator, node);
   }
}

void setArnoldParam_RGB(GroupAttribute globals, const char* attrName, AtNode* node, const char* arnoldName)
{
    if (0 == arnoldName) arnoldName = attrName;
    if (node == NULL) node = AiUniverseGetOptions();

    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);
    const AtParamEntry* pentry = AiNodeEntryLookUpParameter(baseNodeEntry, arnoldName);
    if (!pentry)
    {
         AiMsgWarning("[kat] attempt to set \"%s\" on node \"%s\" failed "
                "because the parameter couldn't be found.",
                        arnoldName, AiNodeGetStr(node, "name"));
         return;
    }

    FnAttribute::FloatAttribute attrRef;
    if (globals.isValid())
    {
        attrRef = globals.getChildByName(attrName);
    }
    if (!attrRef.isValid())
    {
        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
             AiNodeSetArray(node, arnoldName, AiArrayCopy(AiParamGetDefault(pentry)->ARRAY));
        else
             AiNodeSetRGB(node, arnoldName,
                                        AiParamGetDefault(pentry)->RGB.r,
                                        AiParamGetDefault(pentry)->RGB.g,
                                        AiParamGetDefault(pentry)->RGB.b);
    }
    else
    {
        FloatConstVector v = attrRef.getNearestSample(0);
        AiNodeSetRGB(node, arnoldName, v[0], v[1], v[2]);
    }
}


void setArnoldParam_String(GroupAttribute globals, const char* attrName, AtNode* node, const char* arnoldName)
{
    if (0 == arnoldName) arnoldName = attrName;
    if (node == NULL)
    {
        node = AiUniverseGetOptions();
    }
    FnAttribute::StringAttribute attrRef;
    if (globals.isValid())
    {
        attrRef = globals.getChildByName(attrName);
    }

    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);
    const AtParamEntry* pentry = AiNodeEntryLookUpParameter(baseNodeEntry, arnoldName);
    if (!pentry)
    {
        AiMsgWarning("[kat] attempt to set \"%s\" on node \"%s\" failed "
                "because the parameter couldn't be found.",
                        arnoldName, AiNodeGetStr(node, "name"));
        return;
    }

    if (!attrRef.isValid())
    {
        //cast failed, attr still exists, warn
        if (globals.isValid() && globals.getChildByName(attrName).isValid() &&
            !(NullAttribute(globals.getChildByName(attrName)).isValid()))
        {
            AiMsgWarning("[kat] attempt to set \"%s\" on node \"%s\" failed "
                "because of parameter/attr type mismatch. Expecting StringAttr.",
                        arnoldName, AiNodeGetStr(node, "name"));
        }

        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
        {
            AiNodeSetArray(node, arnoldName, AiArrayCopy(AiParamGetDefault(pentry)->ARRAY));
        }
        else
        {
            AiNodeSetStr(node, arnoldName, AiParamGetDefault(pentry)->STR);
        }
    }
    else
    {
        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
        {
            StringConstVector attrValue = attrRef.getNearestSample(0);

            AiNodeSetArray(node, arnoldName,
                    AiArrayConvert(attrValue.size(), 1, AI_TYPE_STRING,
                        (void*)(&attrValue[0])));
        }
        else
        {
          AiNodeSetStr(node, arnoldName, attrRef.getValue().c_str());
        }
    }
}

void setArnoldParam_Pointer(GroupAttribute globals, const char* attrName,
        FnScenegraphIterator sgIterator, ArnoldPluginState* sharedState, AtNode* node, const char* arnoldName,
                bool obeyMute)
{
    if (0 == arnoldName) arnoldName = attrName;

    if (node == NULL)
        node = AiUniverseGetOptions();

    FnAttribute::StringAttribute attrRef;
    if (globals.isValid()) attrRef = globals.getChildByName(attrName);
    if (!attrRef.isValid())
    {
        const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);
        const AtParamEntry* pentry = AiNodeEntryLookUpParameter(baseNodeEntry, arnoldName);
        if (!pentry)
        {
            AiMsgWarning("[kat] attempt to set \"%s\" on node \"%s\" failed "
                    "because the parameter couldn't be found.",
                    arnoldName, AiNodeGetStr(node, "name"));
            return;
        }
        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
            AiNodeSetArray(node, arnoldName, AiArrayCopy(AiParamGetDefault(pentry)->ARRAY));
        else
            AiNodeSetPtr(node, arnoldName, AiParamGetDefault(pentry)->PTR);
    }
    else
    {
        std::string scenegraphLocation = attrRef.getValue();

        AtNode* shaderNode = getShaderByPath(scenegraphLocation, sgIterator, NULL, true, sharedState);

        if (shaderNode)
        {
            AiNodeSetPtr(node, arnoldName, shaderNode);
        }
    }
}

void setGlobalsDefault(FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState)
{
    GroupAttribute g = rootIterator.getAttribute("arnoldGlobalStatements");

    //set directly where katana defaults differ
   AiNodeSetBool(AiUniverseGetOptions(), "shadows_obey_light_linking", true);
    AiNodeSetInt(AiUniverseGetOptions(), "threads", 1);

    // Geolib is still not thread-safe, so we need to set this option
    AiNodeSetBool(AiUniverseGetOptions(), "enable_threaded_procedurals", true);

    // only set these when the attr is present {

    // threads can come from general setting or arnold-specific setting
    GroupAttribute rs = rootIterator.getAttribute("renderSettings");
    if (rs.isValid() && rs.getChildByName("renderThreads").isValid())
    {
        setArnoldParam_Int(rs, "renderThreads", 0, "threads");
    }
    else if (g.isValid() && g.getChildByName("threads").isValid())
    {
        setArnoldParam_Int(g, "threads");
    }

    if (g.isValid() && g.getChildByName("shadows_obey_light_linking").isValid())
    {
        setArnoldParam_Bool(g, "shadows_obey_light_linking");
    }

    if (g.isValid() && g.getChildByName("auto_transparency_mode").isValid())
    {
        setArnoldParam_Int(g, "auto_transparency_mode");
    }
    // }

    setArnoldParam_Pointer(g, "background", rootIterator, sharedState);
    setArnoldParam_Pointer(g, "atmosphere", rootIterator, sharedState);

    const AtNodeEntry *globals_baseNodeEntry = AiNodeGetNodeEntry(AiUniverseGetOptions());

    if (AiNodeEntryLookUpParameter(globals_baseNodeEntry, "background_visibility"))
    {
        setBackgroundVisibility(AiUniverseGetOptions(), "background_visibility",
                rootIterator.getAttribute(
                    "arnoldGlobalStatements.background_visibility"));
    }



    static const char * boolParamNames[] = {
        "AA_sample_clamp_affects_aovs",
        "save_on_abort",
        "abort_on_error",
        "abort_on_license_fail",
        "ignore_textures", //"ignore_tmaps",
        "ignore_shaders",
        "ignore_atmosphere", //"ignore_atm_shaders",
        "ignore_lights",
        "ignore_shadows",
        "ignore_subdivision",
        "ignore_displacement",
        "ignore_smoothing",
        "ignore_sss",
        "ignore_motion_blur",
        "ignore_direct_lighting",
        "ignore_bump",
        "shadow_terminator_fix", //"shadow_term_fix",
        //"shadows_obey_light_linking", //special behavior
        "skip_background_atmosphere",

        "texture_accept_untiled",
        "texture_automip",
        "texture_conservative_lookups",
        "texture_per_file_stats",
        "texture_accept_unmipped",
        
        "sss_use_autobump",

        "shadow_terminator_fix",
        "shader_nan_checks",
        "CCW_points",

        "enable_aov_composition",
        "enable_displacement_derivs",
        "enable_procedural_cache",
        "procedural_force_expand",
        "ignore_dof",
        "enable_fast_lights",
        "enable_threaded_displacement",


        0
    };

    for (int i = 0;; ++i)
    {
        const char * name = boolParamNames[i];
        if (!name)
        {
            break;
        }
        if (AiNodeEntryLookUpParameter(globals_baseNodeEntry, name))
        {
            setArnoldParam_Bool(g, name);
        }
    }

    // Special case: Arnold's winding order is counterclockwise by default
    // (CCW_points = true), Katana's is clockwise. Hence, if CCW_points is not
    // set explicitly, set it to false.
    if(!g.isValid() || !g.getChildByName("CCW_points").isValid()) {
        // Set the polygon winding to Katana default.
        AiNodeSetBool(AiUniverseGetOptions(), "CCW_points", false);
    }

    static const char * intParamNames[] = {
        "AA_samples",
        "AA_motionblur_pattern", //"AA_mblur_pattern",
        "AA_pattern",
        "thread_priority",
        "diagnostics",
        "diag_max_count",
        "bucket_size",
        "bucket_scanning",
        "max_subdivisions",
        "mipmap_level_estimator",
        //"auto_transparency_mode", //special behavior
        "auto_transparency_depth",
        "GI_diffuse_depth",
        "GI_glossy_depth",
        "GI_reflection_depth",
        "GI_refraction_depth",
        "GI_total_depth",
        "GI_diffuse_samples", //"GI_hemi_samples",
        "GI_glossy_samples", //"GI_specular_samples",
        "GI_refraction_samples",
        "GI_single_scatter_samples",
        "volume_indirect_samples",

        "texture_autotile",
        "texture_max_open_files",

        "pin_threads",
        "sss_bssrdf_samples",

        0
    };

    for (int i = 0;; ++i)
    {
        const char * name = intParamNames[i];
        if (!name)
        {
            break;
        }
        if (AiNodeEntryLookUpParameter(globals_baseNodeEntry, name))
        {
            setArnoldParam_Int(g, name);
        }
    }

    static const char * floatParamNames[] = {
        "AA_sample_clamp",
        "bump_multiplier",
        "luminaire_bias",
        "auto_transparency_threshold",
        "GI_falloff_start_dist",
        "GI_falloff_stop_dist",
        "texture_diffuse_blur",
        "texture_glossy_blur",
        "texture_max_memory_MB",
        "texture_max_sharpen",
        "texture_gamma",
        "shader_gamma",

        "reference_time",

        0
    };

    for (int i = 0;; ++i)
    {
        const char * name = floatParamNames[i];
        if (!name)
        {
            break;
        }
        if (AiNodeEntryLookUpParameter(globals_baseNodeEntry, name))
        {
            setArnoldParam_Float(g, name);
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    static const char * rgbParamNames[] = {
        "error_color_bad_texture", //"error_color_bad_map",
        "error_color_bad_mesh",
        "error_color_bad_pixel", //"error_color_bad_pix",
        "error_color_bad_shader",
        0
    };

    for (int i = 0;; ++i)
    {
        const char * name = rgbParamNames[i];
        if (!name)
        {
            break;
        }
        if (AiNodeEntryLookUpParameter(globals_baseNodeEntry, name))
        {
            setArnoldParam_RGB(g, name);
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    static const char * stringParamNames[] = {
        "texture_searchpath",
        "procedural_searchpath",
        "shader_searchpath",

        0
    };

    for (int i = 0;; ++i)
    {
        const char * name = stringParamNames[i];
        if (!name)
        {
            break;
        }
        if (AiNodeEntryLookUpParameter(globals_baseNodeEntry, name))
        {
            setArnoldParam_String(g, name);
        }
    }

    buildArbitraryStatements(g, AiUniverseGetOptions(), rootIterator);
}

void buildArbitraryStatements(GroupAttribute enclosingGroup, AtNode * node,
        FnScenegraphIterator sgIterator)
{
    if (!enclosingGroup.isValid()) return;

    GroupAttribute arbitrary = enclosingGroup.getChildByName("arbitrary");
    if (!arbitrary.isValid())
    {
        return;
    }

    for (int i=0; i<arbitrary.getNumberOfChildren(); ++i)
    {
        std::string attrName = arbitrary.getChildName(i);
        Attribute attr = arbitrary.getChildByIndex(i);

        const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);
        const AtParamEntry* pentry = AiNodeEntryLookUpParameter(
                baseNodeEntry, attrName.c_str());

        //only attempt to create user parameters if the sgIterator is provided.
        //currently, only /root provides this.
        if (!pentry && sgIterator.isValid())
        {
            //create user data if the specified type can't be found

            //if it's in arbitrary output attr form, use that
            //otherwise, apply based on attr type

            if (attr.getType() == kFnKatAttributeTypeString)
            {
                FnAttribute::StringAttribute strAttr = attr;
                StringConstVector attrValues = strAttr.getNearestSample(0);

                UserDefDeclare(node, attrName.c_str(), "constant string");

                SetUserData(node, attrName.c_str(),
                        attrValues.size(), AI_TYPE_STRING, (void*) &attrValues[0]);
            }
            else if (attr.getType() == kFnKatAttributeTypeInt)
            {
                FnAttribute::IntAttribute intAttr = attr;
                IntConstVector values = intAttr.getNearestSample(0);
                UserDefDeclare(node, attrName.c_str(), "constant int");
                SetUserData(node, attrName.c_str(),
                        values.size(), AI_TYPE_INT, (void*) &values[0]);
            }
            else if (attr.getType() == kFnKatAttributeTypeFloat)
            {
                FnAttribute::FloatAttribute floatAttr = attr;
                FloatConstVector values = floatAttr.getNearestSample(0);
                UserDefDeclare(node, attrName.c_str(), "constant float");
                SetUserData(node, attrName.c_str(),
                        values.size(), AI_TYPE_FLOAT, (void*) &values[0]);
            }
            else if (attr.getType() == kFnKatAttributeTypeGroup)
            {
                GroupAttribute groupAttr = attr;
                //confirm that scope is primitive as anything else
                //wouldn't make sense in this context
                FnAttribute::StringAttribute scopeAttr =
                        groupAttr.getChildByName("scope");

                if (!scopeAttr.isValid() ||
                    scopeAttr.getValue() != "primitive")
                {
                    AiMsgWarning(
                            "[kat] invalid arbitrary attribute"
                            " scope for \"%s\" on /root.",
                                    attrName.c_str());
                    continue;
                }

                //WEIRDNESS NOTE:
                //writeArbitrary expects the parent group attribute
                //make a temporary group as we want to be able to hand
                //these off one at a time
                //
                //Also: it expects a geometryAttr. We won't have one
                //in this case so we can send along any group attr
                //as it won't be necessary with a scope of primitive
                GroupBuilder gb;
                gb.set(attrName, groupAttr);
                writeArbitrary(node, sgIterator.getFullName(), sgIterator.getType(),
                        enclosingGroup, gb.build());
            }

            continue;
        }

        int paramType = AiParamGetType(pentry);
        if (paramType == AI_TYPE_ARRAY)
        {
            paramType = AiParamGetDefault(pentry)->ARRAY->type;
        }

        switch (paramType)
        {
        case AI_TYPE_INT:
        case AI_TYPE_ENUM:
            setArnoldParam_Int(arbitrary, attrName.c_str(), node);
            break;
        case AI_TYPE_BOOLEAN:
            setArnoldParam_Bool(arbitrary, attrName.c_str(), node);
            break;
        case AI_TYPE_FLOAT:
            setArnoldParam_Float(arbitrary, attrName.c_str(), node);
            break;
        case AI_TYPE_STRING:
            setArnoldParam_String(arbitrary, attrName.c_str(), node);
            break;
        }
    }
}

void setPerCameraDefaultAASeed(FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState)
{
    //exit early if it's been manually specified

    if (rootIterator.isValid())
    {
        GroupAttribute g = rootIterator.getAttribute("arnoldGlobalStatements");
        if (g.isValid())
        {
            FnAttribute::IntAttribute a = g.getChildByName("arbitrary.AA_seed");
            if (a.isValid())
            {
                return;
            }
        }
    }

    AtNode * cameraNode =
            (AtNode*) AiNodeGetPtr(AiUniverseGetOptions(), "camera");

    if (cameraNode)
    {
        FnAttribute::StringAttribute camLocation(AiNodeGetName(cameraNode));
        int hashedValue = int(camLocation.getHash().uint64());
        hashedValue += (int)sharedState->getFrameTime();
        AiNodeSetInt(AiUniverseGetOptions(), "AA_seed", hashedValue);
    }
    else
    {
        AiNodeSetInt(AiUniverseGetOptions(), "AA_seed",
                (int)sharedState->getFrameTime());
    }
}

void buildGlobals(FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState)
{
    AiNodeSetInt(AiUniverseGetOptions(), "AA_seed", (int)sharedState->getFrameTime());

    setGlobalsDefault(rootIterator, sharedState);

    GroupAttribute g = rootIterator.getAttribute("arnoldGlobalStatements");
    if (!g.isValid()) return;

    FnAttribute::IntAttribute attrRef = g.getChildByName("ignore_shaders");

    if (attrRef.getValue(0, false))
    {
       AtNode *node = AiNodeLookUpByName("ai_default_reflection_shader");
       setArnoldParam_Int(g, "utility_shader_color_mode",node,"color_mode");
       setArnoldParam_Int(g, "utility_shader_shade_mode",node,"shade_mode");
       setArnoldParam_Int(g, "utility_shader_overlay_mode",node,"overlay_mode");
    }


    FnAttribute::StringAttribute assIncludeWhenAttr = g.getChildByName("assIncludeWhen");

    if (assIncludeWhenAttr.getValue("", false) == "at start up")
    {
        processAssIncludes(rootIterator, sharedState);
    }
}

void processAssIncludes(FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState)
{
    GroupAttribute g = rootIterator.getAttribute("arnoldGlobalStatements");
    if (!g.isValid()) return;

    FnAttribute::StringAttribute assIncludeAttr = g.getChildByName("assInclude");
    if (assIncludeAttr.isValid())
    {
        StringConstVector attrValues = assIncludeAttr.getNearestSample(0);
        for (StringConstVector::const_iterator I = attrValues.begin();
             I != attrValues.end(); ++I)
        {
            std::vector<std::string> commaSep;
            pystring::split((*I), commaSep, ",");

            for (std::vector<std::string>::iterator I = commaSep.begin();
                    I!=commaSep.end(); ++I)
            {
                std::vector<std::string> assFiles;
                pystring::split((*I), assFiles);

                for (std::vector<std::string>::iterator I = assFiles.begin();
                        I!=assFiles.end(); ++I)
                {
                    std::string assFile = DefaultAssetPlugin::resolvePath(
                        (*I), (int)sharedState->getFrameTime());

                    if (!assFile.empty())
                    {
                        AiASSLoad(assFile.c_str());
                    }
                }
            }
        }
    }
}


bool buildCropWindow(int* crop, std::string crop_window, std::string crop_window_default,
                     int width, int height, ArnoldPluginState* sharedState)
{

    if(!crop) return false;

    if(crop_window != crop_window_default)
    {
        int flip;
        unsigned int cur_version = getArnoldVersion();
        unsigned int arnold_flip_version;
        /*
         * arnold 2.19+ has flipped around the image coordinates such that
         * 0,0 is the upper-left (2.18 and before this was the lower-left)
         */
        arnold_flip_version = quantifyArnoldVersion("2","19","0","0");


        if (cur_version < arnold_flip_version)
           flip = 1;
        else
           flip = 0;

        std::vector<std::string> cropWindowSplit;
        pystring::split(std::string(crop_window), cropWindowSplit, ",");
        if (cropWindowSplit.size() != 4)
        {
            std::cerr << "[kat] FATAL: unable to parse --cropWindow arg." << std::endl;
            exit(1);
        }

        float cropWindow[4];
        for(unsigned int i=0; i<4; ++i)
        {
            cropWindow[i] = atof(pystring::strip(cropWindowSplit[i]).c_str());
            if (cropWindow[i] < 0.)
               cropWindow[i] = 0;
            else if (cropWindow[i] > 1.)
               cropWindow[i] = 1;
        }

        SetGlobalNormalizedCropWindow(cropWindow[0], cropWindow[1],
                cropWindow[2], cropWindow[3], sharedState);


        crop[0] = int(width*cropWindow[0]+.5);
        crop[1] = int(width*cropWindow[1]+.5)-1;

        if (flip)
        {
           crop[2] = int(height*(1.0f-cropWindow[3])+.5);
           crop[3] = int(height*(1.0f-cropWindow[2])+.5)-1;
        }
        else
        {
           crop[2] = int(height*cropWindow[2]+.5);
           crop[3] = int(height*cropWindow[3]+.5)-1;
        }

        if(crop[0] != -1 && crop[1] != -1 && crop[2] != -1 && crop[3] != -1) return true;
        else return false;

    }
    else
    {
        return false;
    }

}

void setLogFlags(FnScenegraphIterator rootIterator)
{
    if(!rootIterator.isValid()) return;

    int log_flags = AI_LOG_WARNINGS |
                      AI_LOG_INFO |
                      AI_LOG_STATS |
                      AI_LOG_PROGRESS |
                      AI_LOG_NAN |
                      AI_LOG_BACKTRACE |
                      AI_LOG_ERRORS |
                      AI_LOG_TIMESTAMP |
                      AI_LOG_MEMORY;
    int max_warnings = 5;
    //int tab_size = 1;


    // Process some attributes that are in arnoldGlobalStatements, but aren't actually arnold globals.
    // This takes care of verbose logging and assFile dumping, which are special cases.
    GroupAttribute arnoldGlobalStatements = rootIterator.getAttribute("arnoldGlobalStatements");
    if (arnoldGlobalStatements.isValid())
    {
        /* AI_LOG_PRINT2CONSOLE -- as of Arnold 2.2.1.0 this is obsolte. this now works like so:

        if you want to print messages to the console, call AiSetLogOptions(NULL,....)....
        if you want to print messages to a file, AiSetLogOptions(filename,....) --
        the interface of AiSetLogOptions hasn't changed, just the "functionality".
        You'd now call it twice--once for setting up the console logging options
        and once for the file logging options

        */


        // Check if there is a maxWarnings Arnold Global Setting
        FnAttribute::IntAttribute maxWarningsAttr = arnoldGlobalStatements.getChildByName("max_warnings");
        if (maxWarningsAttr.isValid())
        {
            max_warnings = maxWarningsAttr.getValue();
        }

        //check in both the legacy and 2.7+ location for logging values
        std::vector<GroupAttribute> loggingAttrs;

        GroupAttribute attr = arnoldGlobalStatements.getChildByName("Logging");
        if (attr.isValid())
        {
            loggingAttrs.push_back(attr);
        }

        loggingAttrs.push_back(arnoldGlobalStatements);

        for (std::vector<GroupAttribute>::iterator I = loggingAttrs.begin();
                I != loggingAttrs.end(); ++I)
        {
            GroupAttribute logging = (*I);

            //All but AI_LOG_WARNINGS and AI_LOG_DEBUG have been removed from the gui
            //support only these as attributes for now
            {
                FnAttribute::IntAttribute logWarnings = logging.getChildByName("AI_LOG_WARNINGS");
                if (logWarnings.isValid())
                {
                    if (logWarnings.getValue(0, false))
                    {
                        log_flags |= AI_LOG_WARNINGS;
                    }
                    else
                    {
                        log_flags &= ~AI_LOG_WARNINGS;
                    }
                }

                FnAttribute::IntAttribute logMaps = logging.getChildByName("AI_LOG_DEBUG");
                if (logMaps.isValid())
                {
                    if (logMaps.getValue(0, false))
                    {
                        log_flags |= AI_LOG_DEBUG;
                    }
                    else
                    {
                        log_flags &= ~AI_LOG_DEBUG;
                    }
                }
            }
        }

        //set log options for render log stdout
        //AiSetLogOptions(NULL, log_flags, max_warnings, tab_size);
        AiMsgSetConsoleFlags(log_flags);
        AiMsgSetMaxWarnings(max_warnings);

        //set log options for log file
        FnAttribute::StringAttribute logFile = arnoldGlobalStatements.getChildByName("log_file");
        if (logFile.isValid())
        {
            std::string log_file = logFile.getValue("", false);
            if (log_file.size())
            {
                AiMsgSetLogFileName(log_file.c_str());
                AiMsgSetConsoleFlags(log_flags);
                //AiSetLogOptions((char*)log_file.c_str(), log_flags, max_warnings, tab_size);
                AiMsgSetMaxWarnings(max_warnings);
            }
        }
    }
    else
    {
        //AiSetLogOptions(NULL, log_flags, max_warnings, tab_size);
        AiMsgSetConsoleFlags(log_flags);
        AiMsgSetMaxWarnings(max_warnings);
    }
}


//
// get an Arnold version in an 'int' format so
// that it's easy to check arnold versions
//
unsigned getArnoldVersion()
{
    char major_ver[16];
    char minor_ver[16];
    char beta_ver[16];
    char fix_ver[16];

    AiGetVersion(major_ver, minor_ver, beta_ver, fix_ver);

    return quantifyArnoldVersion(major_ver, minor_ver, beta_ver, fix_ver);
}

//
// builds a number which quantifies different arnold versions
// This assumes that we never exceed 100 for any fix, beta, or minor
// version number
//
unsigned int quantifyArnoldVersion(
    const char *major_ver,
    const char *minor_ver,
    const char *beta_ver,
    const char *fix_ver)
{
    unsigned int iMajor = atoi(major_ver);
    unsigned int iMinor = atoi(minor_ver);
    unsigned int iBeta  = atoi(beta_ver);
    unsigned int iFix   = atoi(fix_ver);

    return (((iMajor*100u) + iMinor)*100u + iBeta)*100u + iFix;
}

void buildGlobalCoordinateSystems(FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState)
{
    //confirm transform node type exists in current arnold
    if (!AiNodeEntryLookUp("transformation")) return;
    if (!rootIterator.isValid()) return;

    FnScenegraphIterator worldIterator = rootIterator.getByPath("/root/world");

    if (!worldIterator.isValid()) return;

    GroupAttribute coordSysAttr =
        worldIterator.getAttribute("globals.coordinateSystems");

    if (!coordSysAttr.isValid()) return;

    for (int i=0; i<coordSysAttr.getNumberOfChildren(); ++i)
    {
        const std::string name = coordSysAttr.getChildName(i);
        FnAttribute::StringAttribute pathAttr = coordSysAttr.getChildByIndex(i);
        if (!pathAttr.isValid()) continue;

        FnScenegraphIterator coordSysIterator =
            rootIterator.getByPath(pathAttr.getValue("", false));

        if (!coordSysIterator.isValid()) continue;

        AtNode * xformNode = AiNode("transformation");

        if (!xformNode) continue;

        AiNodeSetStr(xformNode, "name", name.c_str());
        applyTransform(xformNode, coordSysIterator, sharedState);
    }
}

void setBackgroundVisibility(AtNode * node, const char * paramName, GroupAttribute visAttr)
{
    AtByte visibilityMask = 0;

    visibilityMask |= AI_RAY_REFLECTED;
    visibilityMask |= AI_RAY_REFRACTED;
    visibilityMask |= AI_RAY_SHADOW;


    if (visAttr.isValid())
    {
        //only act if these are set to true {

        FnAttribute::IntAttribute rayAttr = visAttr.getChildByName("AI_RAY_CAMERA");
        if (rayAttr.isValid() && rayAttr.getValue())
        {
            visibilityMask |= AI_RAY_CAMERA;
        }

        rayAttr = visAttr.getChildByName("AI_RAY_DIFFUSE");
        if (rayAttr.isValid() && rayAttr.getValue())
        {
            visibilityMask |= AI_RAY_DIFFUSE;
        }

        rayAttr = visAttr.getChildByName("AI_RAY_GLOSSY");
        if (rayAttr.isValid() && rayAttr.getValue())
        {
            visibilityMask |= AI_RAY_GLOSSY;
        }

        //}

        //only act if these are set to false {

        rayAttr = visAttr.getChildByName("AI_RAY_SHADOW");
        if (rayAttr.isValid() && !rayAttr.getValue())
        {
            visibilityMask &= ~AI_RAY_SHADOW;
        }


        rayAttr = visAttr.getChildByName("AI_RAY_REFLECTED");
        if (rayAttr.isValid() && !rayAttr.getValue())
        {
            visibilityMask &= ~AI_RAY_REFLECTED;
        }

        rayAttr = visAttr.getChildByName("AI_RAY_REFRACTED");
        if (rayAttr.isValid() && !rayAttr.getValue())
        {
            visibilityMask &= ~AI_RAY_REFRACTED;
        }

        //}
    }

    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);

    if (AiNodeEntryLookUpParameter(baseNodeEntry, paramName))
    {
        AiNodeSetInt(node, paramName, visibilityMask);
    }

}

Imath::M44d XFormAttrToIMath(const FnAttribute::DoubleAttribute & attr, float t)
{
    if (!attr.isValid()) return Imath::M44d();
    FnAttribute::DoubleAttribute::array_type v = attr.getNearestSample(t);
    if (v.size() < 16) return Imath::M44d();
    return Imath::M44d((double(*)[4]) v.data());
}

void XFormAttrToIMathVec(std::vector<Imath::M44d> & vec,
    const FnAttribute::DoubleAttribute & attr)
{
    for (int64_t ti = 0,
        ei = attr.getNumberOfTimeSamples();
        ti<ei; ++ti)  // timeindex
    {
        vec.push_back(XFormAttrToIMath(attr, attr.getSampleTime(ti)));
    }
}

void getTransformSamples(std::map<float, Imath::M44d>* samples,
        FnScenegraphIterator sgIter, ArnoldPluginState* sharedState, bool clampToShutter,
                GroupAttribute * overrideXformAttr)
{
    GroupAttribute xformAttr;

    if (overrideXformAttr && overrideXformAttr->isValid())
    {
        xformAttr = *overrideXformAttr;
    }
    else
    {
        xformAttr = sgIter.getGlobalXFormGroup();
    }

    float shutterOpen = clampToShutter ? sharedState->getShutterOpen() : -std::numeric_limits<float>::max();
    float shutterClose = clampToShutter ? sharedState->getShutterClose() : std::numeric_limits<float>::max();

    FnAttribute::DoubleAttribute matrix = FnAttribute::RemoveTimeSamplesIfAllSame(
        FnAttribute::RemoveTimeSamplesUnneededForShutter(
            FnGeolibServices::FnXFormUtil::CalcTransformMatrixAtExistingTimes(xformAttr).first,
            shutterOpen, shutterClose));

    std::vector<Imath::M44d> xformsImath;
    XFormAttrToIMathVec(xformsImath, matrix);

    samples->clear();
    for (unsigned int i = 0; i < xformsImath.size(); ++i)
    {
        (*samples)[matrix.getSampleTime(i)] = xformsImath[i];
    }
}


bool isVisible(FnScenegraphIterator sgIterator) {
    FnAttribute::IntAttribute visibleAttr = sgIterator.getAttribute("visible", true);
    if (!visibleAttr.isValid()) return true;
    return visibleAttr.getValue() != 0;
}


void getSampleTimes(FnScenegraphIterator rootIterator, DataAttribute dataAttr, std::set<float>* samples, ArnoldPluginState* sharedState)
{
    if(!dataAttr.isValid() || !samples) return;

    std::set<float> attrSampleTimes;

    int64_t numSampleTimes = dataAttr.getNumberOfTimeSamples();
    for (int64_t i = 0; i < numSampleTimes; ++i)
    {
        attrSampleTimes.insert(dataAttr.getSampleTime(i));
    }

    std::vector<float> sampleTimes;
    RenderOutputUtils::findSampleTimesRelevantToShutterRange(
        sampleTimes,
        attrSampleTimes,
        sharedState->getShutterOpen(),
        sharedState->getShutterClose());
    samples->insert(sampleTimes.begin(), sampleTimes.end());

}

void writeArbitrary(AtNode* node,
        const std::string & locationFullName, const std::string & locationType,
                    GroupAttribute geometryAttr, GroupAttribute arbitraryAttr)
{
    //validate incoming
    if (!node || !geometryAttr.isValid() || !arbitraryAttr.isValid()) return;

    if (!arbitraryAttr.getNumberOfChildren()) return;

    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);

    bool isGInstance = (std::string(AiNodeEntryGetName(baseNodeEntry)) == "ginstance");

    //make sure all textures are the same size (multi or single)
    for (int i = 0; i < arbitraryAttr.getNumberOfChildren(); ++i)
    {
        std::string attrName = arbitraryAttr.getChildName(i);

        // "st" is special!
        if (attrName == "st") continue;

        GroupAttribute groupAttr = arbitraryAttr.getChildByIndex(i);

        std::string geometryType = locationType;

        FnGeolibServices::ArbitraryOutputAttr arbitrary(attrName, groupAttr, geometryType, geometryAttr);
        if (!arbitrary.isValid())
        {
            AiMsgWarning("[kat] Unrecognized arbitrary attr %s at %s: %s",
                    attrName.c_str(), locationFullName.c_str(),
                    arbitrary.getError().c_str());

            continue;
        }

        std::string name = arbitrary.getName();

        //Allow some parameters to be excluded from instances for memory savings
        if (isGInstance)
        {
            if (groupAttr.isValid())
            {
                FnAttribute::IntAttribute intAttr = groupAttr.getChildByName("excludeFromInstances");
                if (intAttr.isValid())
                {
                    if (intAttr.getValue())
                    {
                        continue;
                    }
                }
            }
        }

        std::string scope            = arbitrary.getScope();
        std::string outputType       = arbitrary.getOutputType();
        int apiType                  = AI_TYPE_FLOAT;
        int elementSize              = arbitrary.getElementSize();
        unsigned int dataSize        = 0;
        void* dataStart              = NULL;
        bool isArray                 = arbitrary.isArray();

        //arnold doesn't permit varying userdata to live on instances
        if (isGInstance)
        {
            if (scope == kFnKatArbAttrScopePoint || scope == kFnKatArbAttrScopeVertex)
            {
                continue;
            }
        }

        // keep attrs alive until dataStart/dataSize are no longer in use
        FnAttribute::FloatAttribute floatAttr;
        FnAttribute::DoubleAttribute doubleAttr;
        FnAttribute::IntAttribute intAttr;
        FnAttribute::StringAttribute stringAttr;

        ////////////////////////////////////////////////////////////////////////
        // Get the appropriate arbitrary data from the scene graph.
        ////////////////////////////////////////////////////////////////////////
        if (outputType == kFnKatArbAttrTypeColor3)
        {
            floatAttr = arbitrary.getValueAttr(kFnKatArbAttrBaseTypeFloat);
            if (!floatAttr.isValid())
            {
                AiMsgWarning("[kat] %s arbitrary %s %s %s missing float data @ %s",
                    arbitrary.getError().c_str(), scope.c_str(), outputType.c_str(),
                    name.c_str(), locationFullName.c_str());
                continue;
            }

            outputType = "RGB";
            apiType    = AI_TYPE_RGB;

            if(elementSize)
            {
               FloatConstVector floatData = floatAttr.getNearestSample(0);
               dataStart  = (void*) &floatData[0];
               dataSize   = floatData.size()/3;
            }
        }
        else if (outputType == kFnKatArbAttrTypePoint3)
        {
            floatAttr = arbitrary.getValueAttr(kFnKatArbAttrBaseTypeFloat);
            if(!floatAttr.isValid())
            {
                AiMsgWarning("[kat] %s arbitrary %s %s %s missing float data @ %s",
                    arbitrary.getError().c_str(), scope.c_str(), outputType.c_str(),
                    name.c_str(), locationFullName.c_str());
                continue;
            }

            outputType = "POINT";
            apiType    = AI_TYPE_POINT;

            if(elementSize)
            {
                FloatConstVector floatData = floatAttr.getNearestSample(0);
                dataStart  = (void*) &floatData[0];
                dataSize   = floatData.size()/3;
            }
        }
        else if (outputType == kFnKatArbAttrTypePoint2)
        {
            floatAttr = arbitrary.getValueAttr(kFnKatArbAttrBaseTypeFloat);
            if (!floatAttr.isValid())
            {
                AiMsgWarning("[kat] %s arbitrary %s %s %s missing float data @ %s",
                    arbitrary.getError().c_str(), scope.c_str(), outputType.c_str(),
                    name.c_str(), locationFullName.c_str());
                continue;
            }

            outputType = "POINT2";
            apiType    = AI_TYPE_POINT2;

            if(elementSize)
            {
                FloatConstVector floatData = floatAttr.getNearestSample(0);
                dataStart  = (void*) &floatData[0];
                dataSize   = floatData.size()/2;
            }
        }
        else if (outputType == kFnKatArbAttrTypeVector3)
        {
            floatAttr = arbitrary.getValueAttr(kFnKatArbAttrBaseTypeFloat);
            if (!floatAttr.isValid())
            {
                AiMsgWarning("[kat] %s arbitrary %s %s %s missing double data @ %s",
                    arbitrary.getError().c_str(), scope.c_str(), outputType.c_str(),
                    name.c_str(), locationFullName.c_str());
                continue;
            }

            outputType = "VECTOR";
            apiType    = AI_TYPE_VECTOR;

            if(elementSize)
            {
                FloatConstVector floatData = floatAttr.getNearestSample(0);
                dataStart  = (void*) &floatData[0];
                dataSize   = floatData.size()/3;
            }
        }
        else if (outputType == kFnKatArbAttrTypeNormal3)
        {
            doubleAttr = arbitrary.getValueAttr(kFnKatArbAttrBaseTypeDouble);
            if (!doubleAttr.isValid())
            {
                AiMsgWarning("[kat] %s arbitrary %s %s %s missing double data @ %s",
                    arbitrary.getError().c_str(), scope.c_str(), outputType.c_str(),
                    name.c_str(), locationFullName.c_str());
                continue;
            }

            outputType = "VECTOR";
            apiType    = AI_TYPE_VECTOR;

            if(elementSize)
            {
                DoubleConstVector doubleData = doubleAttr.getNearestSample(0);
                dataStart  = (void*) &doubleData[0];
                dataSize   = doubleData.size()/3;
            }
        }
        else if (outputType == kFnKatArbAttrTypeFloat ||
                 outputType == kFnKatArbAttrTypeDouble)
        {
            floatAttr = arbitrary.getValueAttr(kFnKatArbAttrBaseTypeFloat);
            if (!floatAttr.isValid())
            {
                AiMsgWarning("[kat] %s arbitrary %s %s %s missing float data @ %s",
                    arbitrary.getError().c_str(), scope.c_str(), outputType.c_str(),
                    name.c_str(), locationFullName.c_str());
                continue;
            }

            outputType = "FLOAT";
            apiType    = AI_TYPE_FLOAT;

            if(elementSize)
            {
                FloatConstVector floatData = floatAttr.getNearestSample(0);
                dataStart  = (void*) &floatData[0];
                dataSize   = floatData.size();
            }
        }
        else if (outputType == kFnKatArbAttrTypeInt)
        {
            intAttr = arbitrary.getValueAttr(kFnKatArbAttrBaseTypeInt);
            if (!intAttr.isValid())
            {
                AiMsgWarning("[kat] %s arbitrary %s %s %s missing int data @ %s",
                    arbitrary.getError().c_str(), scope.c_str(), outputType.c_str(),
                    name.c_str(), locationFullName.c_str());
                continue;
            }

            outputType = "INT";
            apiType    = AI_TYPE_INT;

            if(elementSize)
            {
                IntConstVector intData = intAttr.getNearestSample(0);
                dataStart  = (void*) &intData[0];
                dataSize   = intData.size();
            }
        }
        else if (outputType == kFnKatArbAttrTypeString)
        {
            stringAttr = arbitrary.getValueAttr(kFnKatArbAttrBaseTypeString);
            if (!stringAttr.isValid())
            {
                AiMsgWarning("[kat] %s arbitrary %s %s %s missing string data @ %s",
                    arbitrary.getError().c_str(), scope.c_str(), outputType.c_str(),
                    name.c_str(), locationFullName.c_str());
                continue;
            }

            // StringConstVector is a weird animal.  It creates an internal
            // vector of std::string objects built from the const char ** data
            // returned by attribute... which we then convert to a vector
            // of const char *.  Would need CharArrayConstVector type (see TODO
            // in FnAttribute.h) to be efficient.

            // Instead, using FnAttribute C API directly to get access to
            // original const char **.
            int64_t valueCount=0;
            const char ** stringData = Attribute::getSuite()->getStringNearestSample(
                stringAttr.getHandle(), 0, &valueCount);

            outputType = "STRING";
            apiType    = AI_TYPE_STRING;
            dataStart  = (void*)stringData;
            dataSize   = valueCount;
        }
        else if (outputType == kFnKatArbAttrTypeMatrix16)
        {
            floatAttr = arbitrary.getValueAttr(kFnKatArbAttrBaseTypeFloat);
            if (!floatAttr.isValid())
            {
                AiMsgWarning("[kat] %s arbitrary %s %s %s missing float data @ %s",
                    arbitrary.getError().c_str(), scope.c_str(), outputType.c_str(),
                    name.c_str(), locationFullName.c_str());
                continue;
            }

            outputType = "MATRIX";
            apiType    = AI_TYPE_MATRIX;

            if(elementSize)
            {
                FloatConstVector floatData = floatAttr.getNearestSample(0);
                dataStart  = (void*) &floatData[0];
                dataSize   = floatData.size()/16;
            }
        }
        else if ( outputType == kFnKatArbAttrTypeUInt )
        {
            intAttr = arbitrary.getValueAttr(kFnKatArbAttrBaseTypeInt);
            if (!intAttr.isValid())
            {
                AiMsgWarning("[kat] %s arbitrary %s %s %s missing int data @ %s",
                    arbitrary.getError().c_str(), scope.c_str(), outputType.c_str(),
                    name.c_str(), locationFullName.c_str());
                continue;
            }

            outputType = "UINT";
            apiType    = AI_TYPE_UINT;

            if(elementSize)
            {
                IntConstVector intData = intAttr.getNearestSample(0);
                dataStart  = (void*) &intData[0];
                dataSize   = intData.size();
            }
        }
        else if ( outputType == kFnKatArbAttrTypeBool )
        {
            intAttr = arbitrary.getValueAttr(kFnKatArbAttrBaseTypeInt);
            if (!intAttr.isValid())
            {
                AiMsgWarning("[kat] %s arbitrary %s %s %s missing int data @ %s",
                    arbitrary.getError().c_str(), scope.c_str(), outputType.c_str(),
                    name.c_str(), locationFullName.c_str());
                continue;
            }

            outputType = "BOOL";
            apiType    = AI_TYPE_BOOLEAN;

            if(elementSize)
            {
                IntConstVector intData = intAttr.getNearestSample(0);
                dataStart  = (void*) &intData[0];
                dataSize   = intData.size();
            }
        }
        else if ( outputType == kFnKatArbAttrTypeByte )
        {
            intAttr = arbitrary.getValueAttr(kFnKatArbAttrBaseTypeInt);
            if (!intAttr.isValid())
            {
                AiMsgWarning("[kat] %s arbitrary %s %s %s missing int data @ %s",
                    arbitrary.getError().c_str(), scope.c_str(), outputType.c_str(),
                    name.c_str(), locationFullName.c_str());
                continue;
            }

            outputType = "BYTE";
            apiType    = AI_TYPE_BYTE;

            if(elementSize)
            {
                IntConstVector intData = intAttr.getNearestSample(0);
                dataStart  = (void*) &intData[0];
                dataSize   = intData.size();
            }
        }
        else
        {
            AiMsgWarning("[kat] unsupported arbitrary type %s %s attribute %s @ %s",
                scope.c_str(), outputType.c_str(), name.c_str(), locationFullName.c_str());
            continue;
        }

        ////////////////////////////////////////////////////////////////////////
        // Error checking
        ////////////////////////////////////////////////////////////////////////
        if (!isArray && !(dataStart && dataSize))
        {
            AiMsgWarning("[kat] data error in arbitrary %s %s attribute %s @ %s",
                scope.c_str(), outputType.c_str(), name.c_str(), locationFullName.c_str());
            continue;
        }

        ////////////////////////////////////////////////////////////////////////
        // Actually declare to the arbitrary data to Arnold.
        ////////////////////////////////////////////////////////////////////////
        if (scope == kFnKatArbAttrScopePrimitive)
        {
            std::string userType("constant ");

            // Check if we are explicitly declaring an array.
            // Support arrays only at the primitive level
            if(isArray)
            {
                userType += "ARRAY ";
            }
            else
            {
                // Maintain old behaviour
                if (dataSize > 1)
                {
                    userType += "ARRAY ";
                }
            }

            userType += outputType;

            if(!UserDefDeclare(node, name.c_str(), userType.c_str())) continue;

            if(isArray)
            {
                AiNodeSetArray(node, name.c_str(), AiArrayConvert(dataSize, 1, apiType, dataStart));
            }
            else
            {
                SetUserData(node, name.c_str(), dataSize, apiType, dataStart);
            }

        }
        else if (scope == kFnKatArbAttrScopePoint)
        {
            std::string userType;
            if (geometryType == "pointcloud")
               userType = "uniform ";
            else
               userType = "varying ";
            userType += outputType;

            if(!UserDefDeclare(node, name.c_str(), userType.c_str())) continue;
            SetUserData(node, name.c_str(), dataSize, apiType, dataStart, true);

        }
        else if (scope == kFnKatArbAttrScopeFace)
        {
            std::string userType("uniform ");
            userType += outputType;

            if(!UserDefDeclare(node, name.c_str(), userType.c_str())) continue;
            SetUserData(node, name.c_str(), dataSize, apiType, dataStart, true);

        }
        else
        {
            AiMsgWarning("[kat] unsupported arbitrary scope %s %s attribute %s @ %s",
                scope.c_str(), outputType.c_str(), name.c_str(), locationFullName.c_str());
            continue;
        }
    }

}

// wrapper for AiNodeDeclare
// we want to make sure that the user data declare worked, dum warning if it failed
bool UserDefDeclare(AtNode* node, const char* name, const char* type)
{
    if(!node || !name || !type)
        return false;

    if(!AiNodeDeclare(node, name, type))
    {
        AiMsgWarning("[kat] unable to declare user data %s %s", name, type);
        return false;
    }
    else
    {
        return true;
    }
}

// wrapper arround AiNodeSet*
// we need to switch on type her in case we are not an array
void SetUserData(AtNode* node, const char* name, int dataSize, int apiType, void* dataStart, bool forceArray)
{
    if(!node || !name || !dataSize || !dataStart) return;


    if(dataSize > 1 || forceArray)
    {
        AiNodeSetArray(node, name, AiArrayConvert(dataSize, 1, apiType, dataStart));
    }
    else
    {
        switch(apiType)
        {
            case AI_TYPE_INT:
                AiNodeSetInt(node, name, *((int*)dataStart));
                break;
            case AI_TYPE_FLOAT:
                AiNodeSetFlt(node, name, *((float*)dataStart));
                break;
            case AI_TYPE_STRING:
                AiNodeSetStr(node, name, *((const char**)dataStart));
                break;

            case AI_TYPE_RGB:
            {
                float* p = (float*)dataStart;
                AiNodeSetRGB(node, name, p[0], p[1], p[2]);
                break;
            }
            case AI_TYPE_POINT:
            {
                double* p = (double*)dataStart;
                AiNodeSetPnt(node, name, p[0], p[1], p[2]);
                break;
            }
            case AI_TYPE_VECTOR:
            {
                double* p = (double*)dataStart;
                AiNodeSetVec(node, name, p[0], p[1], p[2]);
                break;
            }
            case AI_TYPE_POINT2:
            {
                float* p = (float*)dataStart;
                AiNodeSetPnt2(node, name, p[0], p[1]);
                break;
            }
            case AI_TYPE_MATRIX:
            {
                float* p = (float*)dataStart;
                AtMatrix m;
                for (int y = 0; y < 4; ++y)
                {
                    for(int x = 0; x < 4; ++x)
                    {
                        m[y][x] = *p++;
                    }
                }
                AiNodeSetMatrix(node, name, m);
                break;
            }
            case AI_TYPE_UINT:
                AiNodeSetUInt(node, name, *((unsigned int*)dataStart));
                break;
            case AI_TYPE_BOOLEAN:
                AiNodeSetBool(node, name, *((bool*)dataStart));
                break;
            case AI_TYPE_BYTE:
                AiNodeSetByte(node, name, *((unsigned char*)dataStart));
                break;

            default:
                break;

        }

    }
}

bool setTextureUserDataAsArbitrary(FnScenegraphIterator sgIterator, AtNode* node,
        GroupAttribute texturesAttr, GroupAttribute geometryAttr)
{
    FnAttribute::StringAttribute textureStyleAttr = sgIterator.getAttribute(
        "arnoldStatements.texturesOutputStyle", true);
    if (textureStyleAttr.isValid())
    {
        if (textureStyleAttr.getValue() == "scoped")
        {
            if (!geometryAttr.isValid())
            {
                //no need to query globally because we won't be inheriting
                //relevant topology information
                geometryAttr = sgIterator.getAttribute("geometry");
                if (!geometryAttr.isValid())
                {
                    return false;
                }
            }

            writeArbitrary(node, sgIterator.getFullName(), sgIterator.getType(), geometryAttr,
                FnKat::RenderOutputUtils::convertTexturesToArbitraryAttr(texturesAttr));

            return true;
        }
    }

    return false;
}

void setTextureUserDataForNurbspatch(FnScenegraphIterator sgIterator, AtNode* nurbs)
{
    if(!sgIterator.isValid() || !nurbs) return;

    //TODO: currently only handling textures, should be generalized to handle arbitrary data
    GroupAttribute texturesAttr = sgIterator.getAttribute("textures", true);
    if (!texturesAttr.isValid()) return;

    if (setTextureUserDataAsArbitrary(sgIterator, nurbs, texturesAttr))
    {
        return;
    }

    //get children of the textures attr and check size
    if (!texturesAttr.getNumberOfChildren())
    {
        AiMsgWarning("[kat] texturesAttr present but no children %s", sgIterator.getFullName().c_str());
        return;
    }

    for (int i=0; i<texturesAttr.getNumberOfChildren(); ++i)
    {
        //get maptype name
        std::string mapType = texturesAttr.getChildName(i);

        //get string attr
        FnAttribute::StringAttribute stringAttr = texturesAttr.getChildByIndex(i);
        if (!stringAttr.isValid()) continue;

        //declare and set map
        AiNodeDeclare(nurbs, mapType.c_str(), "constant ARRAY STRING");
        std::string mapName = stringAttr.getValue();
        const char *mapNameChar = mapName.c_str();
        AiNodeSetArray(nurbs, mapType.c_str(), AiArrayConvert(1, 1, AI_TYPE_STRING, &mapNameChar));
    }

    //create const long of value "0" so we always index into first elem of array
    AiNodeDeclare(nurbs, "user_data_idxs", "constant INT");
    AiNodeSetUInt(nurbs, "user_data_idxs", 0);

}

void setTextureUserDataForPolymesh(FnScenegraphIterator sgIterator, AtNode* node,
    unsigned int numPolys, GroupAttribute geometryAttr)
{
    if (!sgIterator.isValid() || !node) return;

    //TODO: currently only handling textures, should be generalized to handle arbitrary data
    GroupAttribute texturesAttr = sgIterator.getAttribute("textures", true);
    if (!texturesAttr.isValid()) return;

    if (setTextureUserDataAsArbitrary(sgIterator, node, texturesAttr, geometryAttr))
    {
        return;
    }

    if (!texturesAttr.getNumberOfChildren())
    {
        AiMsgWarning("[kat] texturesAttr present but no children %s", sgIterator.getFullName().c_str());
        return;
    }

    std::vector<int> indexDataCopy;
    bool             indexDataCopied = false;

    unsigned int maxIndexSize = 0, maxMapsSize = 0;
    for (int i=0; i<texturesAttr.getNumberOfChildren(); ++i)
    {
        GroupAttribute groupAttr = texturesAttr.getChildByIndex(i);
        if (groupAttr.isValid())
        {
            FnAttribute::IntAttribute indicesAttr = groupAttr.getChildByName("indicies");
            if (!indicesAttr.isValid()) indicesAttr = groupAttr.getChildByName("indices");
            FnAttribute::StringAttribute mapsAttr = groupAttr.getChildByName("maps");
            if(!indicesAttr.isValid() || !mapsAttr.isValid()) continue;

            maxIndexSize = std::max(maxIndexSize, (unsigned int)indicesAttr.getNumberOfValues());
            maxMapsSize = std::max(maxMapsSize, (unsigned int) mapsAttr.getNumberOfValues());
        }
        else
        {
            maxIndexSize = std::max(maxIndexSize, (unsigned int) 1);
            maxMapsSize = std::max(maxMapsSize, (unsigned int) 1);
        }
    }

    for (int i=0; i<texturesAttr.getNumberOfChildren(); ++i)
    {
        GroupAttribute groupAttr = texturesAttr.getChildByIndex(i);

        //maptype name
        std::string mapType = texturesAttr.getChildName(i);

        if (groupAttr.isValid())
        {
            //array
            FnAttribute::IntAttribute indicesAttr = groupAttr.getChildByName("indicies");
            if (!indicesAttr.isValid()) indicesAttr = groupAttr.getChildByName("indices");
            FnAttribute::StringAttribute mapsAttr = groupAttr.getChildByName("maps");
            if (!indicesAttr.isValid() || !mapsAttr.isValid()) continue;

            StringConstVector mapData = mapsAttr.getNearestSample(0);
            IntConstVector indexData = indicesAttr.getNearestSample(0);
            if (indexData.size() != numPolys)
            {
                AiMsgWarning("[kat] Mismatched texture array lengths %d->%d %s @ %s",
                    (int)indexData.size(), numPolys, mapType.c_str(), sgIterator.getFullName().c_str());
                continue;
            }

            // if this is the first one, make a copy
            // TODO: this assumes that all the index arrays are identical! should test for that
            if (!indexDataCopy.size())
            {
                indexDataCopied = true;
                indexDataCopy.insert(indexDataCopy.end(), indexData.begin(), indexData.end());
            }


            // map type string array ----------------------------------- | this is NOT an implicit array since uniform, need to specify
            if(!UserDefDeclare(node, (char*) mapType.c_str(), "constant ARRAY STRING")) continue;
            std::vector<const char*> stringVector;

            stringVector.reserve(maxMapsSize);
            for(size_t i = 0; i <maxMapsSize; i++)
            {
                if (i < mapData.size())
                {
                    stringVector[i] = mapData[i];
                }
                else
                {
                    stringVector[i] = mapData[0];
                }
            }

            AiNodeSetArray(node, mapType.c_str(),
                AiArrayConvert(maxMapsSize, 1, AI_TYPE_STRING, (void*)(&(stringVector[0]))));
        }
        else
        {
            //single val
            FnAttribute::StringAttribute stringAttr = texturesAttr.getChildByIndex(i);
            if (!stringAttr.isValid()) continue;
            if(!UserDefDeclare(node, (char*) mapType.c_str(), "constant ARRAY STRING")) continue;
            std::vector<const char*> stringVector;

            StringConstVector stringData = stringAttr.getNearestSample(0);

            stringVector.reserve(maxMapsSize);
            for (unsigned int i = 0; i < maxMapsSize; ++i)
            {
                stringVector.push_back(stringData[0]);
            }

            AiNodeSetArray(node, mapType.c_str(), AiArrayConvert(maxMapsSize, 1, AI_TYPE_STRING, (void*)(&(stringVector[0]))));
        }


    }

    //have something to set user index data?
    if(indexDataCopied && indexDataCopy.size() == numPolys)
    {
        // user index array ----------------------------- | this is an implicit array if uniform or varying!
        if(UserDefDeclare(node, "user_data_idxs", "uniform INT"))
        {
            AiNodeSetArray(node, "user_data_idxs",
                AiArrayConvert(indexDataCopy.size(), 1, AI_TYPE_INT, (void*)(&(indexDataCopy[0]))));
        }
    }
    else
    {
        //no indices, means we have single textures, create const long of value "0" so we always index into first elem of array
        if(UserDefDeclare(node, "user_data_idxs", "constant INT"))
            AiNodeSetInt(node, "user_data_idxs", 0);
    }
}
void applyTransform(AtNode* node, FnScenegraphIterator sgIterator, ArnoldPluginState* sharedState,
        std::map<float, Imath::M44d> *returnSamples,
                GroupAttribute * overrideXformAttr)
{
    std::map<float, Imath::M44d> samples;
    getTransformSamples(&samples, sgIterator, sharedState,  false, overrideXformAttr);

    if (returnSamples != 0x0)
    {
        *returnSamples = samples;
    }

    //check to see that we're not a single identity matrix
    if (samples.size() > 1 || samples.begin()->second != Imath::M44d())
    {
        std::vector<float> sampleTimes;
        sampleTimes.reserve(samples.size());

        //std::vector<double> mlist;
        std::vector<float> mlist;
        mlist.reserve(16*samples.size());



        for (std::map<float, Imath::M44d>::iterator it = samples.begin();
                 it != samples.end(); ++it)
        {
            sampleTimes.push_back((*it).first);
            for(int i = 0; i < 16; i++)
            {
                mlist.push_back((*it).second.getValue()[i]);
            }
        }

        AiNodeSetArray(node, "matrix",
                AiArrayConvert(1, samples.size(),
                        AI_TYPE_MATRIX, &mlist[0]));

        //confirm presence of these attributes and warn if we
        //don't start with 0 and 1 or if we're not evenly-spaced.

        if (sampleTimes.size() > 1 )
        {
            const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);
            if (AiNodeEntryLookUpParameter(baseNodeEntry, "transform_time_samples"))
            {
                AiNodeSetArray(node, "transform_time_samples",
                        AiArrayConvert(sampleTimes.size(), 1, AI_TYPE_FLOAT,
                                &sampleTimes[0]));
            }
            else if (AiNodeEntryLookUpParameter(baseNodeEntry, "time_samples"))
            {
                AiNodeSetArray(node, "time_samples",
                        AiArrayConvert(sampleTimes.size(), 1, AI_TYPE_FLOAT,
                                &sampleTimes[0]));
            }

            //check to see if our samples are anything but 0 and 1
            else if (sampleTimes.size() > 1 &&
                        (sampleTimes.front() != 0 || sampleTimes.back() != 1))
            {
                AiMsgWarning(
                    std::string("[kat] non 0 to 1 transform samples found on " +
                        sgIterator.getFullName() +
                                ". This version of arnold does not "
                                "support this and will likely result in "
                                "inaccurate motion blur.").c_str());
            }
        }

    }

    GroupAttribute bakedCoordSystemsAttr =
            sgIterator.getAttribute("bakedCoordinateSystems", true);
    if (bakedCoordSystemsAttr.isValid())
    {
        //GroupAttr::NamedAttrVector_Type childList;
        //bakedCoordSystemsAttr->getChildVector(&childList);

        for (int i=0; i<bakedCoordSystemsAttr.getNumberOfChildren(); ++i)
        {
            FnAttribute::DoubleAttribute matrixAttr = bakedCoordSystemsAttr.getChildByIndex(i);
            if (!matrixAttr.isValid()) continue;
            if (matrixAttr.getNumberOfValues() != 16) continue;

            std::string paramName = ("coordsys_"+bakedCoordSystemsAttr.getChildName(i));
            AiNodeDeclare(node, paramName.c_str(), "constant MATRIX");
            AtMatrix m;
            DoubleConstVector mdata = matrixAttr.getNearestSample(0);
            for (unsigned int j = 0; j < 16; ++j)
            {
                *((&m[0][0])+j) = mdata[j];
            }
            AiNodeSetMatrix(node, paramName.c_str(), m);
        }
    }
}

void setArnoldParam_Bool(GroupAttribute globals, const char* attrName, AtNode* node, const char* arnoldName)
{
    if (0 == arnoldName) arnoldName = attrName;

    if (node == NULL) node = AiUniverseGetOptions();

    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);
    const AtParamEntry* pentry = AiNodeEntryLookUpParameter(baseNodeEntry, arnoldName);
    if (!pentry)
    {
         AiMsgWarning("[kat] attempt to set \"%s\" on node \"%s\" failed "
                "because the parameter couldn't be found.",
                        arnoldName, AiNodeGetStr(node, "name"));
         return;
    }

    FnAttribute::IntAttribute attrRef;
    if (globals.isValid())
    {
        attrRef = globals.getChildByName(attrName);
    }
    if (!attrRef.isValid())
    {
        //cast failed, attr still exists, warn
        if (globals.isValid() && globals.getChildByName(attrName).isValid() &&
            !(NullAttribute(globals.getChildByName(attrName)).isValid()))
        {
            AiMsgWarning("[kat] attempt to set \"%s\" on node \"%s\" failed "
                "because of parameter/attr type mismatch. Expecting IntAttr.",
                        arnoldName, AiNodeGetStr(node, "name"));
        }

        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
             AiNodeSetArray(node, arnoldName, AiArrayCopy(AiParamGetDefault(pentry)->ARRAY));
        else
             AiNodeSetBool(node, arnoldName, AiParamGetDefault(pentry)->BOOL);
    }
    else
    {
        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
        {
            std::vector<char> arrayValues;
            IntConstVector attrValue = attrRef.getNearestSample(0);
            arrayValues.reserve(attrValue.size());

            for (IntConstVector::const_iterator I = attrValue.begin();
                 I!=attrValue.end(); ++I)
            {
                arrayValues.push_back((*I) != 0);
            }

            AiNodeSetArray(node, arnoldName,
                    AiArrayConvert(arrayValues.size(), 1, AI_TYPE_BOOLEAN,
                        (void*)(&arrayValues[0])));
        }
        else
        {
            AiNodeSetBool(node, arnoldName, attrRef.getValue(0, false) != 0);
        }
    }
}
void setArnoldParam_Float(GroupAttribute globals, const char* attrName, AtNode* node, const char* arnoldName)
{
    if (0 == arnoldName) arnoldName = attrName;
    if (node == NULL) node = AiUniverseGetOptions();

    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);
    const AtParamEntry* pentry = AiNodeEntryLookUpParameter(baseNodeEntry, arnoldName);
    if (!pentry)
    {
        AiMsgWarning("[kat] attempt to set \"%s\" on node \"%s\" failed "
                "because the parameter couldn't be found.",
                        arnoldName, AiNodeGetStr(node, "name"));
        return;
    }

    FnAttribute::FloatAttribute attrRef;
    if (globals.isValid())
    {
        attrRef = globals.getChildByName(attrName);
    }
    if (!attrRef.isValid())
    {
        //cast failed, attr still exists, warn
        if (globals.isValid() && globals.getChildByName(attrName).isValid() &&
            !(NullAttribute(globals.getChildByName(attrName)).isValid()))
        {
            AiMsgWarning("[kat] attempt to set \"%s\" on node \"%s\" failed "
                "because of parameter/attr type mismatch. Expecting FloatAttr.",
                        arnoldName, AiNodeGetStr(node, "name"));
        }

        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
             AiNodeSetArray(node, arnoldName, AiArrayCopy(AiParamGetDefault(pentry)->ARRAY));
        else
             AiNodeSetFlt(node, arnoldName, AiParamGetDefault(pentry)->FLT);
    }
    else
    {
        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
        {
            FloatConstVector attrValue = attrRef.getNearestSample(0);
            AiNodeSetArray(node, arnoldName,
                    AiArrayConvert(attrValue.size(), 1, AI_TYPE_FLOAT,
                        (void*)(&attrValue[0])));
        }
        else
        {
            AiNodeSetFlt(node, arnoldName, attrRef.getValue());
        }
    }
}

void M44d_to_AtMatrix(AtMatrix m, Imath::M44d xform)
{
    for(int y = 0; y < 4; ++y)
    {
        for(int x = 0; x < 4; ++x)
        {
            m[y][x] = xform[y][x];
        }
    }
}

void setArnoldParam_Int(GroupAttribute globals, const char* attrName, AtNode* node, const char* arnoldName)
{
    if (0 == arnoldName) arnoldName = attrName;

    if (node == NULL) node = AiUniverseGetOptions();

    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);
    const AtParamEntry* pentry = AiNodeEntryLookUpParameter(baseNodeEntry, arnoldName);
    if (!pentry)
    {
        AiMsgWarning("[kat] attempt to set \"%s\" on node \"%s\" failed "
                "because the parameter couldn't be found.",
                        arnoldName, AiNodeGetStr(node, "name"));
        return;
    }

    FnAttribute::IntAttribute attrRef;
    if (globals.isValid())
    {
        attrRef = globals.getChildByName(attrName);
    }
    if (!attrRef.isValid())
    {
        //cast failed, attr still exists, warn
        if (globals.isValid() && globals.getChildByName(attrName).isValid() &&
            !(NullAttribute(globals.getChildByName(attrName)).isValid()))
        {
            AiMsgWarning("[kat] attempt to set \"%s\" on node \"%s\" failed "
                "because of parameter/attr type mismatch. Expecting IntAttr.",
                        arnoldName, AiNodeGetStr(node, "name"));
        }

        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
             AiNodeSetArray(node, arnoldName, AiArrayCopy(AiParamGetDefault(pentry)->ARRAY));
        else
             AiNodeSetInt(node, arnoldName, AiParamGetDefault(pentry)->INT);
    }
    else
    {
        if (AiParamGetType(pentry) == AI_TYPE_ARRAY)
        {
            IntConstVector attrValue = attrRef.getNearestSample(0);
            AiNodeSetArray(node, arnoldName,
                    AiArrayConvert(attrValue.size(), 1, AI_TYPE_INT,
                        (void*)(&attrValue[0])));
        }
        else
        {
            AiNodeSetInt(node, arnoldName, attrRef.getValue());
        }
    }
}

void SetGlobalNormalizedCropWindow(float c0, float c1, float c2, float c3, ArnoldPluginState* sharedState)
{
    sharedState->setCropLeft(c0);
    sharedState->setCropBottom(c1);
    sharedState->setCropWidth(c2);
    sharedState->setCropHeight(c3);
}

void GetGlobalNormalizedCropWindow(
        float & c0, float & c1, float & c2, float & c3, ArnoldPluginState* sharedState)
{
    c0 = sharedState->getCropLeft();
    c1 = sharedState->getCropBottom();
    c2 = sharedState->getCropWidth();
    c3 = sharedState->getCropHeight();
}

float GetFrameTime(FnScenegraphIterator sgIterator, ArnoldPluginState* sharedState)
{
    float frameNumber = sharedState->getFrameTime();

    FnAttribute::FloatAttribute frameTimeAttr = sgIterator.getAttribute("time.frameTime", true);
    if (frameTimeAttr.isValid())
    {
        frameNumber = frameTimeAttr.getValue();
    }

    return frameNumber;
}
