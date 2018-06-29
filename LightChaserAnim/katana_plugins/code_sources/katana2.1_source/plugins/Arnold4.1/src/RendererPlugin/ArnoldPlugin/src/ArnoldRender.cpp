// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************
#include <ArnoldRender.h>
#include <string>
#include <iostream>
#include <cstdio>

#include <FnRenderOutputUtils/FnRenderOutputUtils.h>
#include <RerenderUtils.h>
#include <FnRendererInfo/suite/RendererObjectDefinitions.h>
#include <FnRendererInfo/plugin/RenderMethod.h>

#include <ai.h>

#include <pystring/pystring.h>

#include <buildCamera.h>
#include <buildCommons.h>
#include <objectId.h>

#include <iterator>

using namespace FnKat;

ArnoldRender::ArnoldRender( FnScenegraphIterator rootIterator, GroupAttribute arguments ) : RenderBase( rootIterator, arguments )
{
    local_output_address = "";
    local_output_channel = "RGBA";
    local_output_info_file = "";
    local_render_finished_filename = "";

    local_filter_filename = "/tmp/arnold_filter.py";
    local_output_width = 512;
    local_output_height = 512;
    local_overscan[0] = 0;
    local_overscan[1] = 0;
    local_overscan[2] = 0;
    local_overscan[3] = 0;
    local_frame_origin[0] = 0;
    local_frame_origin[1] = 0;
    local_display_window_size[0] = 0;
    local_display_window_size[1] = 0;
    local_dumpfile_filename = "";
    local_expand_procedurals_for_ass_dump = true;
    local_crop_window_default = "0.0, 1.0, 0.0, 1.0";
    local_crop_window = local_crop_window_default;
    original_crop_window = local_crop_window;

    local_frame_id = 100;
    local_frame_name = "Arnold Render";
    local_force_expand = 0;
    local_interactive_render = 0;
    local_batch_render = 0;
    local_generate_ids = 0;

    local_any_checkpoint_file = false;
}

int ArnoldRender::start()
{
    renderMethodName = getRenderMethodName();

    FnScenegraphIterator rootIterator = getRootIterator();
    ArnoldRenderSettings renderSettings( rootIterator );
    FnKatRender::GlobalSettings globalSettings( rootIterator, "arnold" );

    local_root_iterator = rootIterator;

    local_dumpfile_filename = getRenderOutputFile();
    local_expand_procedurals_for_ass_dump = isExpandProceduralActive(true);

    GroupAttribute arnoldGlobalStatements = globalSettings.getGlobalSettingsAttr();

    SetArguments( renderSettings );

    SetupRender( rootIterator, renderSettings );
    buildGlobals( rootIterator, &sharedState );
    buildGlobalCoordinateSystems( rootIterator, &sharedState );

    // build default render camera
    buildRenderCamera( rootIterator, &sharedState, local_overscan, local_display_window_size );

    if( renderMethodName == FnKat::RendererInfo::LiveRenderMethod::kDefaultName )
        sharedState.setIsRerendering(true);
    else
        sharedState.setIsRerendering(false);

    buildGeometry( rootIterator, ( local_force_expand != 0 ), false, &sharedState );

    //If forceExpand is on and it's not an interactive render, flush GeoAPI caches and root producer now
    if ( local_force_expand && !local_interactive_render )
    {
        AiMsgInfo( "[kat] Forced expansion complete, flushing katana caches\n" );
        RenderOutputUtils::flushProceduralDsoCaches();
    }

    int threads = AiNodeGetInt( AiUniverseGetOptions(), "threads" );
    applyRenderThreadsOverride( threads );

    AiMsgInfo( "[kat]   ============ Using %d threads ============", threads );

    if( local_output_info_map.size() > 0 )
    {
        //if user just wants to write .ass, do that and exit - don't render
        if( !local_dumpfile_filename.empty() )
        {
            std::vector<const char *> outputCStrings;
            std::vector<const char *> oslPathRegexpsCStrings;

            // setup outputs array with outputs from default camera, so we have some valid
            // outputs in the ass file.
            std::string defaultCamera = getDefaultRenderCameraName( rootIterator );
            RenderPassOutputMap::iterator mapiter = local_render_passes.find( RenderPassKey( defaultCamera, std::set<std::string>() ) );
            if ( mapiter != local_render_passes.end() )
            {
                const std::vector<std::string> & outputNames = (*mapiter).second;
                for ( std::vector<std::string>::const_iterator iter = outputNames.begin(); iter != outputNames.end(); ++iter )
                {
                    const OutputInfo & outputInfo = _getOutputInfoByName(*iter);
                    outputCStrings.push_back( outputInfo.outputString.c_str() );

                    for ( std::vector<std::string>::const_iterator I = outputInfo.lightExpressions.begin(); I!= outputInfo.lightExpressions.end(); ++I )
                    {
                        oslPathRegexpsCStrings.push_back( (*I).c_str() );
                    }
                }
            }

            // add in lightAgnostic outputs for default camera as well
            StringVectorMap::iterator svmiter = local_light_agnostic_passes.find( defaultCamera );
            if ( svmiter != local_light_agnostic_passes.end() )
            {
                const std::vector<std::string> & outputNames = (*svmiter).second;
                for ( std::vector<std::string>::const_iterator iter = outputNames.begin(); iter != outputNames.end(); ++iter )
                {
                    const OutputInfo & outputInfo = _getOutputInfoByName(*iter);
                    outputCStrings.push_back( outputInfo.outputString.c_str() );

                    for ( std::vector<std::string>::const_iterator I = outputInfo.lightExpressions.begin(); I!= outputInfo.lightExpressions.end(); ++I )
                    {
                        oslPathRegexpsCStrings.push_back( (*I).c_str() );
                    }

                }
            }

            if ( !outputCStrings.empty() )
            {
                AiNodeSetArray( AiUniverseGetOptions(), "outputs", AiArrayConvert( outputCStrings.size(), 1, AI_TYPE_STRING, &outputCStrings[0] ) );
            }

            if ( !oslPathRegexpsCStrings.empty() )
            {
                AiNodeSetArray( AiUniverseGetOptions(), "osl_path_regexps",
                                AiArrayConvert( oslPathRegexpsCStrings.size(), 1, AI_TYPE_STRING,
                                &oslPathRegexpsCStrings[0] ) );
            }


            //set AA_seed prior to each AiRender in case camera has changed and the
            //user hasn't specified the seed directly
            setPerCameraDefaultAASeed( rootIterator, &sharedState );

            // write ass file
            renderResult = AiASSWrite( local_dumpfile_filename.c_str(), getAssWriteIncludeOptions( arnoldGlobalStatements ), local_expand_procedurals_for_ass_dump );

            return 0;
        }
        else
        {
            // Hot Render
            renderResult = AI_SUCCESS;
            bool firstRender = true;

            std::string origBucketScan = AiNodeGetStr( AiUniverseGetOptions(), "bucket_scanning" );

            // get background shader node
            std::string backgroundPath;
            if (arnoldGlobalStatements.isValid())
            {
                FnAttribute::StringAttribute bgPathAttr = arnoldGlobalStatements.getChildByName( "background" );
                if (bgPathAttr.isValid())
                {
                    backgroundPath = bgPathAttr.getValue("", false);
                    AiMsgInfo("[kat] GI background material: %s", backgroundPath.c_str());
                }
            }

            AtNode *backgroundNode = reinterpret_cast<AtNode*>(AiNodeGetPtr(AiUniverseGetOptions(), "background"));

            // get original light intensities
            LightMap_t lightMap;
            sharedState.duplicateLightMapByPath(lightMap);

            std::map<std::string, float> origLightIntensityMap;
            for ( LightMap_t::const_iterator iter = lightMap.begin(); iter != lightMap.end(); ++iter )
            {
                AtNode *lightNode = (*iter).second;
                if (!lightNode) continue;

                float intensity = AiNodeGetFlt(lightNode, "intensity");
                origLightIntensityMap[(*iter).first] = intensity;
            }

            // count output passes so we can give reliable progress information
            int passTotal = 0;
            std::set<std::string> passCountCameras;
            for (RenderPassOutputMap::iterator iter = local_render_passes.begin();
                iter != local_render_passes.end(); ++iter)
            {
                ++passTotal;
                passCountCameras.insert((*iter).first.cameraName);
            }
            for (StringVectorMap::iterator iter = local_light_agnostic_passes.begin();
                iter != local_light_agnostic_passes.end(); ++iter)
            {
                // add in any agnostic passes whose cameras weren't counted above
                if (passCountCameras.find((*iter).first) == passCountCameras.end())
                {
                    ++passTotal;
                }
            }

            int passCounter = 0;
            // loop through render passes and call render for each
            for (RenderPassOutputMap::iterator iter = local_render_passes.begin();
                iter != local_render_passes.end(); ++iter)
            {
                ++passCounter;
                if (!renderPassSetup((*iter).first.cameraName, (*iter).second, !firstRender,
                                     origBucketScan, passCounter, passTotal))
                {
                    AiMsgInfo("[kat] Skipped render pass %d of %d", passCounter, passTotal);
                    continue;
                }

                firstRender = false;

                const std::set<std::string> &isolateLightPaths = (*iter).first.isolateLights;

                if (isolateLightPaths.empty())
                {
                    AiMsgInfo("[kat] Rendering with all lights active (%d lights)",
                        sharedState.sizeOfLightMapByPath());
                }
                else
                {
                    AiMsgInfo("[kat] Rendering with active light(s):");
                    for (std::set<std::string>::const_iterator lightIter=isolateLightPaths.begin();
                        lightIter!=isolateLightPaths.end(); ++lightIter)
                    {
                        const std::string & lightPath = (*lightIter);
                        std::string msg = "[kat]      %s";
                        if (lightPath == backgroundPath)
                        {
                            msg += " (GI background)";
                        }
                        AiMsgInfo(msg.c_str(), lightPath.c_str());
                    }
                }

                // solo light(s) in isolateLightPaths
                for (LightMap_t::const_iterator lgtIter=lightMap.begin();
                    lgtIter!=lightMap.end(); ++lgtIter)
                {
                    std::string lightPath = (*lgtIter).first;
                    AtNode *lightNode = (*lgtIter).second;
                    if (!lightNode) continue;

                    if (isolateLightPaths.empty() || isolateLightPaths.find(lightPath) != isolateLightPaths.end())
                    {
                        AiNodeSetFlt(lightNode,  "intensity", origLightIntensityMap[lightPath]);
                    }
                    else
                    {
                        AiNodeSetFlt(lightNode,  "intensity", 0);
                    }
                }

                // enable/disable GI background (skydome) shader
                if (!backgroundPath.empty() &&
                    (isolateLightPaths.empty() ||
                     isolateLightPaths.find(backgroundPath) != isolateLightPaths.end()))
                {
                    AiNodeSetPtr(AiUniverseGetOptions(), "background", backgroundNode);
                }
                else
                {
                    AiNodeSetPtr(AiUniverseGetOptions(), "background", 0x0);
                }

                //set AA_seed prior to each AiRender in case camera has changed and the
                //user hasn't specified the seed directly
                setPerCameraDefaultAASeed(rootIterator, &sharedState);

                renderResult = AiRender(AI_RENDER_MODE_CAMERA);
                AiMsgInfo("[kat] Finished render pass %d of %d", passCounter, passTotal);

                if (renderResult != AI_SUCCESS) break;
            }

            // render remaining lightAgnostic passes last
            if (renderResult == AI_SUCCESS)
            {
                LightMap_t lightMap;
                sharedState.duplicateLightMapByPath(lightMap);

                // turn off all lights for remaining lightAgnostic passes
                for (LightMap_t::const_iterator lgtIter=lightMap.begin();
                     lgtIter!=lightMap.end(); ++lgtIter)
                {
                    std::string lightPath = (*lgtIter).first;
                    AtNode *lightNode = (*lgtIter).second;
                    if (!lightNode) continue;

                    AiNodeSetFlt(lightNode,  "intensity", 0);
                }

                // disable GI background (skydome) shader for remaining lightAgnostic passes
                AiNodeSetPtr(AiUniverseGetOptions(), "background", 0x0);

                // renderPassSetup will add lightAgnostic outputs based on camera name
                // and clear entry from update local_light_agnostic_passes map.
                while (!local_light_agnostic_passes.empty())
                {
                    StringVectorMap::iterator iter = local_light_agnostic_passes.begin();

                    std::vector<std::string> outputNames; // intentionally empty.
                    ++passCounter;
                    if (!renderPassSetup((*iter).first, outputNames, !firstRender,
                                          origBucketScan, passCounter, passTotal))
                    {
                        AiMsgInfo("[kat] Skipped render pass %d of %d", passCounter, passTotal);
                        continue;
                    }

                    AiMsgInfo("[kat] Rendering with no lights active");
                    firstRender = false;

                    //set AA_seed prior to each AiRender in case camera has changed and the
                    //user hasn't specified the seed directly
                    setPerCameraDefaultAASeed(getRootIterator(), &sharedState);

                    renderResult = AiRender(AI_RENDER_MODE_CAMERA);
                    AiMsgInfo("[kat] Finished render pass %d of %d", passCounter, passTotal);

                    if (renderResult != AI_SUCCESS) break;
                }
            }

            if (renderResult == AI_SUCCESS)
            {
                killCheckpointWatchProcesses();
            }
        }
    }
    else
    {
        //if user just wants to write .ass, do that and exit - don't render
        if( !local_dumpfile_filename.empty() )
        {
            renderResult = AiASSWrite( local_dumpfile_filename.c_str(), getAssWriteIncludeOptions( arnoldGlobalStatements ), local_expand_procedurals_for_ass_dump );

            return 0;
        }
        else
        {
            if( local_batch_render )
            {
                //do final or one-and-only pass
                AiMsgInfo( "[kat] Starting final render." );
                renderResult = AiRender( AI_RENDER_MODE_CAMERA );
            }
            else
            {
                AA_samples = AiNodeGetInt( AiUniverseGetOptions(), "AA_samples" );
                AA_samples_prepass = 1;
                AA_samples_prepass_rerender = -3;
                AA_samples_step = 2;
                AiNodeSetInt( AiUniverseGetOptions(), "AA_samples", AA_samples );

                if ( arnoldGlobalStatements.isValid() )
                {
                    FnAttribute::IntAttribute aaProgressiveStartAttr = arnoldGlobalStatements.getChildByName( "AA_samples_progressive_begin" );
                    if ( aaProgressiveStartAttr.isValid() )
                    {
                        AA_samples_prepass = aaProgressiveStartAttr.getValue( 1, false );
                        AA_samples_prepass_rerender = AA_samples_prepass;
                    }

                    FnAttribute::IntAttribute aaStepAttr = arnoldGlobalStatements.getChildByName( "AA_samples_progressive_step" );
                    if ( aaStepAttr.isValid() )
                    {
                        AA_samples_step = aaStepAttr.getValue( 2, false );
                    }
                }

                if( AA_samples_prepass > AA_samples )
                {
                    AA_samples_prepass = AA_samples;
                }

                if( renderMethodName == FnKat::RendererInfo::LiveRenderMethod::kDefaultName || AA_samples_prepass < AA_samples )
                {
                    if( renderMethodName == FnKat::RendererInfo::LiveRenderMethod::kDefaultName )
                    {
                        skip_reset_options = true;
                        bool suppressLoggingMessages = true;

                        if( arnoldGlobalStatements.isValid() )
                        {
                            FnAttribute::IntAttribute disableLoggingAttr = arnoldGlobalStatements.getChildByName( "disable_logging_when_rerendering" );
                            if( disableLoggingAttr.isValid() && disableLoggingAttr.getValue() == 0 )
                            {
                                suppressLoggingMessages = false;
                            }
                        }

                        if( suppressLoggingMessages )
                            AiMsgSetConsoleFlags( AI_LOG_NONE );
                    }

                    int i = AA_samples_prepass;
                    int step = ( AA_samples_step > 0 ) ? AA_samples_step : 1;
                    renderResult = AI_SUCCESS;

                    for( ; i < AA_samples; i += step )
                    {
                        //AiMsgInfo( "[kat] Starting progressive %d", i );
                        AiNodeSetInt( AiUniverseGetOptions(), "AA_samples", i );
                        renderResult = AiRender( AI_RENDER_MODE_CAMERA );
                        if( renderResult == AI_INTERRUPT )
                            return 0;

                        if( renderResult != AI_SUCCESS )
                        {
                            if( renderResult == AI_ABORT )
                                return 0;

                            break;
                        }
                    }

                    if( renderResult == AI_SUCCESS )
                    {
                        AiNodeSetInt( AiUniverseGetOptions(), "AA_samples", AA_samples );
                        renderResult = AiRender( AI_RENDER_MODE_CAMERA );
                    }
                }
                else
                {
                    //do final or one-and-only pass
                    AiMsgInfo("[kat] Starting final render.");
                    renderResult = AiRender( AI_RENDER_MODE_CAMERA );
                }
            }
        }
    }


    return 0;
}

int ArnoldRender::pause()
{
    AiRenderAbort();

    return 0;
}

int ArnoldRender::stop()
{
    AiEnd();

    if( renderResult == AI_SUCCESS )
    {
        return 0;
    }
    else
    {
        PrintRenderResults( renderResult );
        return 1;
    }
}

int ArnoldRender::processControlCommand(const std::string & command )
{
    if( command == "abort" )
    {
        AiRenderAbort();
    }

    return 0;
}

int ArnoldRender::queueDataUpdates( GroupAttribute updateAttribute )
{
    for ( unsigned int i = 0; i < updateAttribute.getNumberOfChildren(); i++ )
    {
        std::string type( ".type" );
        GroupAttribute commandAttr = updateAttribute.getChildByIndex( i );

        if ( commandAttr.isValid() )
        {
            FnAttribute::StringAttribute typeAttr = commandAttr.getChildByName( "type" );
            FnAttribute::StringAttribute locationAttr = commandAttr.getChildByName( "location" );
            GroupAttribute attributesAttr = commandAttr.getChildByName( "attributes" );

            std::string location;
            if( locationAttr.isValid() )
                location = locationAttr.getValue( "", false );

            FnAttribute::StringAttribute partialUpdateAttr = attributesAttr.getChildByName( "partialUpdate" );
            if( partialUpdateAttr.isValid() && partialUpdateAttr.getValue() == "True" )
                partialUpdate = true;
            else
                partialUpdate = false;

            if( typeAttr.isValid() )
            {
                std::string type = typeAttr.getValue( "", false );

                if( type == "renderSettings" && attributesAttr.isValid() )
                {
                    FnAttribute::StringAttribute renderCameraAttr = attributesAttr.getChildByName( "renderSettings.cameraName" );
                    if( renderCameraAttr.isValid() )
                    {
                        std::string newCamera = renderCameraAttr.getValue( "", false );
                        if( !newCamera.empty() )
                            renderSettingsCamera = newCamera;
                    }

                    AtNode *cameraNode = NULL;
                    // Camera update received, we need to update the universe
                    // options to ensure the Arnold renders from the correct
                    // camera.
                    //
                    // The system employed by Katana to do this is as follows,
                    //
                    // 1). If 'Same as Viewer' is selected and we're using say
                    // persp/top/left etc, we apply any changes (matrix etc.)
                    // to the default camera specified in the RenderSettings
                    // node.
                    // 2). If we're viewing through a camera, we apply any
                    // changes onto that camera.
                    // 3). If we're viewing through a light we take the same
                    // approach described in (1).
                    //
                    // The above rules we're previously implicitly stated the
                    // following logic makes this explicit.
                    if(renderSettingsCamera == "__VIEWERPATH__")
                    {
                        activeLiveRenderCamera =
                          getDefaultRenderCameraName(getRootIterator());
                    }
                    else if( (cameraNode = AiNodeLookUpByName(renderSettingsCamera.c_str())) )
                    {
                        activeLiveRenderCamera = renderSettingsCamera;
                    }
                    else
                    {
                        // Handles the case (3) where we're looking through a light.
                        activeLiveRenderCamera = getDefaultRenderCameraName(getRootIterator());
                    }

                    if(!cameraNode)
                    {
                        cameraNode =
                          AiNodeLookUpByName( activeLiveRenderCamera.c_str() );
                    }

                    if(cameraNode)
                    {
                        KatParamChange updateCamera;
                        updateCamera.node = AiUniverseGetOptions();
                        updateCamera.param_name = strdup("camera");
                        updateCamera.param_type = AI_TYPE_POINTER;
                        updateCamera.param_value.STR = strdup(
                                activeLiveRenderCamera.c_str());
                        updateCamera.array_type = 0;
                        AddNodeChange(&updateCamera);
                    }
                    else
                    {
                        std::cerr << "ArnoldRender::queueDataUpdates: Cannot find node '"
                                  << renderSettingsCamera
                                  << "' to apply camera changes to." << std::endl;
                    }

                    RerenderRenderSettings( location, attributesAttr );
                }

                if( type == "globals" && attributesAttr.isValid() )
                {
                    GroupAttribute globalsAttr = attributesAttr.getChildByName( "arnoldGlobalStatements" );
                    if( globalsAttr.isValid() )
                        RerenderGlobals( activeLiveRenderCamera, globalsAttr );
                }

                if(type == "liveRenderSettings" && attributesAttr.isValid())
                {
                    GroupAttribute liveRenderSettingsAttr = attributesAttr.getChildByName("liveRenderSettings.arnold");
                    if(liveRenderSettingsAttr.isValid())
                    {
                        RerenderLiveRenderSettings(activeLiveRenderCamera, liveRenderSettingsAttr);
                    }
                }

                if (  (location == renderSettingsCamera)
                        && attributesAttr.isValid() )
                {
                    RerenderXform( activeLiveRenderCamera, attributesAttr );
                    RerenderCamera( activeLiveRenderCamera, attributesAttr );
                }

                if( type == "light" && attributesAttr.isValid() )
                {
                    AtNode* lightNode = AiNodeLookUpByName((location +
                            std::string(":arnoldLightShader")).c_str());
                    if (!lightNode)
                    {
                        lightNode = AiNodeLookUpByName((location +
                                std::string(":arnoldSurfaceShader")).c_str());
                    }
                    // TODO: if (!lightNode) { /* Create a light node. */ }
                    // A good implementation candidate is to call
                    // buildLight(lightIterator, 0, &sharedState) in
                    // buildLights.cpp, which should then be exposed through
                    // arnold_render.h
                    // However, buildLight can't be used while the scene graph
                    // iterator does not know about new locations created since
                    // the start of the live render.
                    if (lightNode)
                    {
                        queueLightLinkUpdates(deferredLightLinks, location);
                    }

                    RerenderLightsXform( location, attributesAttr );
                    RerenderLights( location, attributesAttr );
                }
                else if(type == "lightLink" && attributesAttr.isValid())
                {
                    RerenderLightLink(location, attributesAttr);
                }
                else if( type == "geoMaterial" && attributesAttr.isValid() )
                {
                    FnAttribute::StringAttribute parentLocationAttr = commandAttr.getChildByName( "parentLocation" );
                    RerenderMaterials( location, attributesAttr, parentLocationAttr );
                }
                else if( type == "geoXform" && attributesAttr.isValid() )
                {
                    RerenderXform( location, attributesAttr );
                }
                else if( type == "AI_CACHE_TEXTURE" )
                {
                    std::cerr << "Flushing Texture Cache" << std::endl;
                    if (!AiUniverseCacheFlush( AI_CACHE_TEXTURE ))
                    {
                        AiMsgWarning("Texture cache not flushed! Render still running.");
                    }
                    AiRenderInterrupt();
                }
                else if( type == "ShadingOverrides" && attributesAttr.isValid() )
                {
                    RerenderShadingOverrides( location, attributesAttr );
                }

            }
        }
    }

    return 0;
}

int ArnoldRender::RerenderGlobals( std::string& itemName, GroupAttribute attr )
{
    // Cache global settings
    globalSettings = attr;

    // Override attributes
    GroupBuilder gb;
    gb.update(globalSettings);
    gb.update(liveRenderSettingOverrides);
    GroupAttribute attrOverridden = gb.build();

    // Apply overridden attributes
    ApplyOptionAttributeChanges( attrOverridden );
    ApplyCameraAttributeChanges( itemName, attrOverridden );

    // Check and apply shader override
    FnAttribute::IntAttribute ignoreShadersAttr =
            attrOverridden.getChildByName("ignore_shaders");
    if (ignoreShadersAttr.isValid() && ignoreShadersAttr.getValue(0, false))
    {
        RerenderShadingOverrides(itemName, attrOverridden);
    }

    return 0;
}

int ArnoldRender::RerenderLiveRenderSettings(std::string& itemName, FnAttribute::GroupAttribute attr)
{
    // Cache overridden attributes
    liveRenderSettingOverrides = attr;

    // Override attributes
    GroupBuilder gb;
    gb.update(globalSettings);
    gb.update(liveRenderSettingOverrides);
    GroupAttribute attrOverridden = gb.build();

    // Apply overridden attributes
    ApplyOptionAttributeChanges(attrOverridden);

    // Check and apply shader override
    FnAttribute::IntAttribute ignoreShadersAttr =
            liveRenderSettingOverrides.getChildByName("ignore_shaders");
    if (ignoreShadersAttr.isValid() && ignoreShadersAttr.getValue(0, false))
    {
        RerenderShadingOverrides(itemName, attrOverridden);
    }

    return 0;
}

int ArnoldRender::RerenderShadingOverrides( std::string& itemName, GroupAttribute attr )
{
    KatParamChange change;
    change.node = AiNodeLookUpByName( "ai_default_reflection_shader" );

    if ( change.node )
    {
       FnAttribute::IntAttribute colorAttr = attr.getChildByName( "utility_shader_color_mode" );
       if ( colorAttr.isValid() )
       {
           int color = colorAttr.getValue();

           change.array_type  = 0;
           change.param_name  = strdup( "color_mode" );
           change.param_type  = AI_TYPE_ENUM;
           change.param_value.INT = color;
           AddNodeChange( &change );
           AiRenderInterrupt();
       }

       FnAttribute::IntAttribute shadeAttr = attr.getChildByName( "utility_shader_shade_mode" );
       if ( shadeAttr.isValid() )
       {
           int shade = shadeAttr.getValue();

           change.array_type  = 0;
           change.param_name  = strdup( "shade_mode" );
           change.param_type  = AI_TYPE_ENUM;
           change.param_value.INT = shade;
           AddNodeChange( &change );
           AiRenderInterrupt();
       }
    }
    else
    {
        return 1;
    }

    return 0;
}

int ArnoldRender::RerenderXform( std::string& itemName, GroupAttribute attributesAttr )
{
    AtNode *node = AiNodeLookUpByName( itemName.c_str() );

    if( node )
    {
        if ( attributesAttr.isValid() )
        {
            GroupAttribute xformAttr = attributesAttr.getChildByName( "xform" );
            if ( xformAttr.isValid() )
            {
                // Update
                KatParamChange change;
                change.node        = node;
                change.param_name  = strdup( "matrix" );
                change.param_type  = AI_TYPE_MATRIX;
                bool performUpdate = false;

                std::map<float, Imath::M44d> samples;
                getTransformSamples( &samples, local_root_iterator, &sharedState, false, &xformAttr );

                if ( samples.size() > 0 && samples.begin()->second != Imath::M44d() )
                {
                    std::vector<float> sampleTimes;
                    sampleTimes.reserve( samples.size() );
                    std::vector<float> mlist;
                    mlist.reserve( 16 * samples.size() );

                    for ( std::map<float, Imath::M44d>::iterator it = samples.begin();
                          it != samples.end(); ++it )
                    {
                        sampleTimes.push_back( (*it).first );

                        for( int i = 0; i < 16; i++ )
                        {
                            mlist.push_back( (*it).second.getValue()[i] );
                        }
                    }

                    change.array_type = 1;
                    change.param_value.ARRAY = AiArrayConvert(1, samples.size(),
                                                              AI_TYPE_MATRIX, &mlist[0]);
                    performUpdate = true;
                }
                else
                {
                    FnAttribute::DoubleAttribute xformMatrixAttr = xformAttr.getChildByName( "matrix" );

                    if ( xformMatrixAttr.isValid() )
                    {
                        AtMatrix* m = (AtMatrix*) malloc( sizeof( AtMatrix ) );
                        DoubleConstVector doubleData = xformMatrixAttr.getNearestSample(0);

                        int k = 0;
                        for( int y = 0; y < 4; ++y )
                        {
                            for( int x = 0; x < 4; ++x, ++k )
                            {
                                (*m)[y][x] = (float) doubleData[k];
                            }
                        }

                        change.array_type = 0;
                        change.param_value.pMTX = m;
                        performUpdate = true;
                    }
                }

                if( performUpdate )
                {
                    AddNodeChange( &change );
                    AiRenderInterrupt();
                }
            }
        }
    }

    return 0;
}

int ArnoldRender::RerenderMaterials( std::string& itemName, GroupAttribute attributesAttr, FnAttribute::StringAttribute parentLocationAttr )
{
    if ( !attributesAttr.isValid() ) return 1;
    GroupAttribute materialAttr = attributesAttr.getChildByName( "material" );
    if( !materialAttr.isValid() )
    {
        GroupAttribute materialUpstreamAttr = attributesAttr.getChildByName( "material_upstream" );
        if( materialUpstreamAttr.isValid() )
        {
            for( int i=0; i < materialUpstreamAttr.getNumberOfChildren(); i++)
            {
                materialAttr = materialUpstreamAttr.getChildByIndex(i);
                if(materialAttr.isValid())
                    break;
            }
        }

        if(!materialAttr.isValid())
            return 1;
    }

    std::string activeName = itemName;
    // Check for the presence of faces and consequently face id which indicates
    // that we want to change node attached to a faceset
    int faceIdValue = -1;
    FnAttribute::IntAttribute facesAttr = attributesAttr.getChildByName( "faces" );
    if( facesAttr.isValid() )
    {
        IntConstVector faces = facesAttr.getNearestSample(0);
        if( faces.size() > 0 )
        {
            faceIdValue = faces[0];

            if( parentLocationAttr.isValid() )
                activeName = parentLocationAttr.getValue( "", false );
        }
    }

    AtNode *geoNode = AiNodeLookUpByName( activeName.c_str() );

    if ( !geoNode )
    {
        activeName = itemName + "/unusedFaces";
        geoNode = AiNodeLookUpByName( activeName.c_str() );
    }

    if( geoNode )
    {
        AtNode * shaderNode = 0;

        if( faceIdValue != -1 )
        {
            AtArray* shaderList = NULL;

            const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry( geoNode );
            if ( AiNodeEntryLookUpParameter( baseNodeEntry, "shader" ) )
            {
                shaderList = AiNodeGetArray( geoNode, "shader" );
            }

            ByteVectorRcPtr_t indexArray = GetCachedShaderIndexArray(activeName, &sharedState);

            if ( shaderList && shaderList->data && shaderList->nelements && indexArray && indexArray->size() )
            {
                if ( faceIdValue >= 0 && (unsigned) faceIdValue < indexArray->size() )
                {
                    unsigned char shaderIndex = (*indexArray)[faceIdValue];

                    if ( shaderIndex < shaderList->nelements )
                    {
                        shaderNode = ( (AtNode**) shaderList->data )[shaderIndex];
                    }
                }
            }
        }

        if ( !shaderNode )
        {
            shaderNode = (AtNode *) AiNodeGetPtr( geoNode, "shader" );
        }

        if ( !shaderNode ) return 1;

        // Let's check for a networked shader
        FnAttribute::StringAttribute styleAttr = materialAttr.getChildByName( "style" );

        if ( styleAttr.isValid() && styleAttr.getValue() == std::string( "network" ) )
        {
            const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry( shaderNode );
            if ( !strcmp( AiNodeEntryGetName( baseNodeEntry ), "osl_shader" ) )
            {
                KatParamChange change;
                change.node        = shaderNode;
                change.array_type  = 0;
                change.param_name  = strdup(""); //dummy value
                change.param_type  = AI_TYPE_INT; //dummy value
                GroupBuilder gb;
                gb.update( materialAttr );
                gb.set( "terminalName", FnAttribute::StringAttribute( "arnoldSurface" ) );
                change.attr_data = gb.build();
                change.attr_callback = SetOSLTerminalNameShaderNetworkCallback;
                AddNodeChange( &change );
                AiRenderInterrupt();
            }
            else
            {
                RenderOutputUtils::ShadingNodeDescriptionMap nodes( materialAttr );

                FnAttribute::StringAttribute terminalShaderAttr = materialAttr.getChildByName("terminals.arnoldSurface");

                std::set<AtNode *> alreadyWalkedSet;

                if ( terminalShaderAttr.isValid() )
                {
                    RenderOutputUtils::ShadingNodeDescription nodeDesc = nodes.getShadingNodeDescriptionByName( terminalShaderAttr.getValue() );

                    if ( nodeDesc.isValid() )
                    {
                        ApplyNetworkedShaderChanges( shaderNode, nodeDesc, nodes, alreadyWalkedSet );
                    }
                }

                AtNode * bumpNode = AiNodeGetLink( shaderNode, "@before" );
                if ( bumpNode )
                {
                    terminalShaderAttr = materialAttr.getChildByName( "terminals.arnoldBump" );

                    RenderOutputUtils::ShadingNodeDescription nodeDesc = nodes.getShadingNodeDescriptionByName( terminalShaderAttr.getValue() );

                    if ( nodeDesc.isValid() )
                    {
                        std::set<AtNode *> alreadyWalkedSet;
                        ApplyNetworkedShaderChanges( bumpNode, nodeDesc, nodes, alreadyWalkedSet );
                    }
                }
            }
        }
        //otherwise, check for surface and bump manually
        else
        {
            const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry( shaderNode );
            if ( !strcmp( AiNodeEntryGetName( baseNodeEntry ), "osl_shader" ) )
            {
                KatParamChange change;
                change.node        = shaderNode;
                change.array_type  = 0;
                change.param_name  = strdup(""); //dummy value
                change.param_type  = AI_TYPE_INT; //dummy value
                GroupBuilder gb;
                gb.update( materialAttr );
                gb.set( "terminalName", FnAttribute::StringAttribute( "arnoldSurface" ) );
                change.attr_data = gb.build();
                change.attr_callback = SetOSLMonolithicMaterialCallback;
                AddNodeChange( &change );
                AiRenderInterrupt();
            }
            else
            {
                GroupAttribute arnoldShaderParams = materialAttr.getChildByName( "arnoldSurfaceParams" );
                ApplyAttributeChanges( AiNodeGetStr(shaderNode, "name"), arnoldShaderParams );

                AtNode * bumpNode = AiNodeGetLink( shaderNode, "@before" );
                if ( bumpNode )
                {
                    GroupAttribute arnoldBumpParams = materialAttr.getChildByName( "arnoldBumpParams" );
                    ApplyAttributeChanges( AiNodeGetStr( bumpNode, "name" ), arnoldBumpParams );
                }
            }
        }
    }

    return 0;
}

int ArnoldRender::RerenderLights( std::string& itemName, FnAttribute::GroupAttribute attributesAttr )
{
    GroupAttribute materialAttr = attributesAttr.getChildByName( "material" );
    FnAttribute::IntAttribute muteAttr = attributesAttr.getChildByName( "mute" );
    FnAttribute::IntAttribute deletedAttr = attributesAttr.getChildByName( "deleted" );

    if ( deletedAttr.isValid() && deletedAttr.getValue() == 1)
    {
        // Light has been deleted - for now we just mute the light
        muteAttr = deletedAttr;
    }

    // Find the node
    AtNode * node = AiNodeLookUpByName( ( itemName + std::string( ":arnoldLightShader" ) ).c_str() );
    if ( !node )
    {
        node = AiNodeLookUpByName( (itemName + std::string( ":arnoldSurfaceShader") ).c_str() );
    }

    // Find the default intensity, in case no local override is set
    float defaultIntensity = 1.0f;
    if ( node )
    {
        const AtNodeEntry* nodeEntry = AiNodeGetNodeEntry(node);
        if (nodeEntry)
        {
            const AtParamEntry* param = AiNodeEntryLookUpParameter(nodeEntry, "intensity");
            if(param)
            {
                defaultIntensity = AiParamGetDefault(param)->FLT;
            }
        }
    }

    // Check mute
    if( muteAttr.isValid() )
    {
        AtNode * lightPrim = AiNodeLookUpByName( ( itemName+std::string( ":prim" ) ).c_str() );

        if ( lightPrim )
        {
            if ( muteAttr.getValue() )
            {
                KatParamChange samplesChange;
                samplesChange.node = lightPrim;
                samplesChange.param_name = strdup( "emit_samples" );
                samplesChange.array_type = 0;
                samplesChange.param_type = AI_TYPE_INT;
                samplesChange.param_value.INT = 0;
                AddNodeChange( &samplesChange );

                samplesChange.param_name = strdup( "emit_sample_scale" );
                samplesChange.param_type = AI_TYPE_FLOAT;
                samplesChange.param_value.FLT = 0;
                AddNodeChange( &samplesChange );
            }
        }
        else
        {
            KatParamChange change;

            if ( node )
            {
                int mute = muteAttr.getValue();

                change.node = node;
                change.array_type  = 0;
                change.param_name  = strdup( "intensity" );
                change.param_type  = AI_TYPE_FLOAT;
                change.param_value.FLT = mute ? 0 : defaultIntensity;
                AddNodeChange( &change );
                AiRenderInterrupt();

                return 0;
            }
        }
    }
    else if ( materialAttr.isValid() )
    {
        // restore state
        KatParamChange change;

        if ( node )
        {
            GroupAttribute lightParams = materialAttr.getChildByName( "arnoldLightParams" );
            FnAttribute::FloatAttribute intensityAttr = lightParams.getChildByName("intensity");

            if( !intensityAttr.isValid() )
            {
                change.node = node;
                change.array_type  = 0;
                change.param_name  = strdup( "intensity" );
                change.param_type  = AI_TYPE_FLOAT;
                change.param_value.FLT = defaultIntensity;
                AddNodeChange( &change );
                AiRenderInterrupt();
            }
        }
    }

    if ( !materialAttr.isValid() ) return 1;

    // Create the set of parameter names that cannot be set to default if
    // they are not present in materialAttr. These are parameters that are
    // set for the lights on other functions and should not be touched here.
    std::set<std::string> ignoreDefaultsList;
    ignoreDefaultsList.insert("matrix");
    ignoreDefaultsList.insert("vertices");
    ignoreDefaultsList.insert("use_light_group");
    ignoreDefaultsList.insert("light_group");

    FnAttribute::StringAttribute styleAttr = materialAttr.getChildByName( "style" );

    bool handledAsLight = false;

    if ( styleAttr.isValid() && styleAttr.getValue() == "network" )
    {
        handledAsLight = true;
        static const char * terminalNames[] = {
            "arnoldLight",
            "arnoldLightFilter",
            "arnoldSurface",
            0 //sentinel
        };

        for ( unsigned int i = 0;; ++i )
        {
            if ( !terminalNames[i] )
            {
                break;
            }

            FnAttribute::StringAttribute terminalAttr = materialAttr.getChildByName( std::string( "terminals." ) + terminalNames[i] );
            if ( terminalAttr.isValid() )
            {
                // Get the node, determine whether it's OSL or not
                std::string nodeName = itemName + std::string(":") + terminalNames[i] + std::string( "Shader" );
                AtNode * node = AiNodeLookUpByName( nodeName.c_str() );

                if( !node )
                {
                    continue;
                }

                const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry( node );
                if ( node && !strcmp( AiNodeEntryGetName( baseNodeEntry ), "osl_shader" ) )
                {
                    KatParamChange change;
                    change.node        = node;
                    change.array_type  = 0;
                    change.param_name  = strdup(""); //dummy value
                    change.param_type  = AI_TYPE_INT; //dummy value
                    GroupBuilder gb;
                    gb.update( materialAttr );
                    gb.set( "terminalName", FnAttribute::StringAttribute( terminalNames[i] ) );
                    change.attr_data = gb.build();
                    change.attr_callback = SetOSLTerminalNameShaderNetworkCallback;
                    AddNodeChange( &change );
                    AiRenderInterrupt();
                }
                else
                {
                    // Light network, non-OSL
                    RenderOutputUtils::ShadingNodeDescriptionMap nodes( materialAttr );

                    FnAttribute::StringAttribute terminalShaderAttr = materialAttr.getChildByName( std::string( "terminals." ) + terminalNames[i] );

                    std::set<AtNode *> alreadyWalkedSet;

                    if ( terminalShaderAttr.isValid() )
                    {
                        RenderOutputUtils::ShadingNodeDescription nodeDesc = nodes.getShadingNodeDescriptionByName( terminalShaderAttr.getValue() );

                        if ( nodeDesc.isValid() )
                        {
                            ApplyNetworkedShaderChanges( node, nodeDesc, nodes,
                                                         alreadyWalkedSet,
                                                         ignoreDefaultsList);
                        }
                    }
                }
            }
        }
    }
    else
    {
        static const char * shaderNames[] = {
            "arnoldLight",
            "arnoldSurface",
            0 //sentinel
        };

        for ( unsigned int i = 0;; ++i )
        {
            if ( !shaderNames[i] )
            {
                break;
            }

            std::string lightShaderName = itemName + std::string( ":" ) + std::string( shaderNames[i] ) + "Shader";

            AtNode *node = AiNodeLookUpByName( lightShaderName.c_str() );

            if ( !node )
            {
                continue;
            }

            handledAsLight = true;

            const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry( node );
            if ( !strcmp(AiNodeEntryGetName( baseNodeEntry ), "osl_shader" ) )
            {
                KatParamChange change;
                change.node        = node;
                change.array_type  = 0;
                change.param_name  = strdup(""); //dummy value
                change.param_type  = AI_TYPE_INT; //dummy value
                GroupBuilder gb;
                gb.update( materialAttr );
                gb.set( "terminalName", FnAttribute::StringAttribute( shaderNames[i] ) );
                change.attr_data = gb.build();
                change.attr_callback = SetOSLMonolithicMaterialCallback;
                AddNodeChange( &change );
                AiRenderInterrupt();
            }
            else
            {
                std::string shaderParams = shaderNames[i] + std::string( "Params" );

                GroupAttribute lightParams = materialAttr.getChildByName( shaderParams );
                if ( lightParams.isValid() )
                {
                    ApplyAttributeChanges(lightShaderName, lightParams,
                                          ignoreDefaultsList);
                }
            }
        }
    }

    if ( !handledAsLight )
    {
       GroupAttribute cameraParams = materialAttr.getChildByName( "arnoldCameraParams" );
       ApplyAttributeChanges( itemName, cameraParams );
    }

    return 0;
}

struct NodeNameComp {
    bool operator() (const AtNode* lhs, const AtNode* rhs) const
    {
        return strcmp(AiNodeGetName(lhs), AiNodeGetName(rhs)) < 0;
    }
};

int ArnoldRender::RerenderLightLink(std::string& itemName,
        FnAttribute::GroupAttribute attributesAttr)
{
    // Validate input
    if (!attributesAttr.isValid()) return 1;

    // Get attribute with light list changes
    FnAttribute::GroupAttribute lightListAttr =
            attributesAttr.getChildByName("localizedLightList");
    if (!lightListAttr.isValid()) return 0;

    LocationLightLinkMap lightLinks;
    for (unsigned i = 0; i < lightListAttr.getNumberOfChildren(); ++i)
    {
        FnAttribute::GroupAttribute lightGrpAttr =
                lightListAttr.getChildByIndex(i);
        if (!lightGrpAttr.isValid()) continue;

        // Get the light location path
        FnAttribute::StringAttribute lightPathAttr =
                lightGrpAttr.getChildByName("path");
        if (!lightPathAttr.isValid()) continue;
        std::string lightLocation = lightPathAttr.getValue("", false);

        // Get the light enabled/disabled state
        FnAttribute::IntAttribute lightEnableAttr =
                lightGrpAttr.getChildByName("enable");
        if (!lightEnableAttr.isValid()) continue;
        bool enable = lightEnableAttr.getValue(1, false);

        // Check if the light node exists
        AtNode* lightNode = AiNodeLookUpByName((lightLocation +
                std::string(":arnoldLightShader")).c_str());
        if (!lightNode)
        {
            lightNode = AiNodeLookUpByName((lightLocation +
                    std::string(":arnoldSurfaceShader")).c_str());
        }
        if (!lightNode)
        {
            // Defer light link when the light is fully created
            deferredLightLinks[itemName][lightLocation] = enable;
            continue;
        }
        else
        {
            lightLinks[itemName][lightLocation] = enable;
        }
    }
    queueLightLinkUpdates(lightLinks);

    return 0;
}

void ArnoldRender::queueLightLinkUpdates(LocationLightLinkMap &lightLinks,
        const std::string &lightFilter)
{
    // Loop for all nodes with light link updates
    for (LocationLightLinkMap::iterator LocationIt = lightLinks.begin(),
            LocationItEnd = lightLinks.end(); LocationIt != LocationItEnd;)
    {
        AtNode* node = AiNodeLookUpByName(LocationIt->first.c_str());
        if (!node)
        {
            lightLinks.erase(LocationIt++);
            continue;
        }

        // Check if light linking is enabled for this node
        const AtNodeEntry* nodeEntry = AiNodeGetNodeEntry(node);
        const AtParamEntry* paramEntry = AiNodeEntryLookUpParameter(nodeEntry,
                "use_light_group");
        if (!paramEntry || !AiNodeGetBool(node, "use_light_group"))
        {
            // Turn on light linking
            KatParamChange useLightGroupChange;
            useLightGroupChange.node = node;
            useLightGroupChange.param_name = strdup("use_light_group");
            useLightGroupChange.param_type = AI_TYPE_BOOLEAN;
            useLightGroupChange.param_value.BOOL = true;
            useLightGroupChange.array_type = 0;
            AddNodeChange(&useLightGroupChange);
        }

        // Get current Arnold list of lights at this location
        AtArray* lightGroupList = AiNodeGetArray(node, "light_group");
        // Unique lights with fast insert/delete access to enable/disable lights:
        std::set<AtNode*, NodeNameComp> lights;
        if (lightGroupList)
        {
            AtNode** nodes = (AtNode**) lightGroupList->data;
            lights.insert(nodes, nodes + lightGroupList->nelements);
        }

        // Loop for all lights with updates
        for (LightLinkMap::iterator LightIt = LocationIt->second.begin(),
                LightItEnd = LocationIt->second.end(); LightIt != LightItEnd;)
        {
            if (!lightFilter.empty() && lightFilter != LightIt->first)
            {
                // Iterate and continue
                ++LightIt;
                continue;
            }

            AtNode* lightNode = AiNodeLookUpByName((LightIt->first +
                    std::string(":arnoldLightShader")).c_str());
            if (!lightNode)
            {
                lightNode = AiNodeLookUpByName((LightIt->first +
                        std::string(":arnoldSurfaceShader")).c_str());
            }
            if (!lightNode) continue;

            if (LightIt->second)
            {
                lights.insert(lightNode);
            }
            else
            {
                lights.erase(lightNode);
            }

            if (!lightFilter.empty())
            {
                // Remove this entry
                LocationIt->second.erase(LightIt++);
            }
            else
            {
                // Iterate
                ++LightIt;
            }
        }

        // Queue light link changes
        KatParamChange lightGroupChange;
        lightGroupChange.node = node;
        lightGroupChange.param_name = strdup("light_group");
        lightGroupChange.array_type = 1;
        lightGroupChange.param_type = AI_TYPE_NODE;

        // Convert set of lights into a continuous array
        std::vector<AtNode*> lightGroupArray(lights.begin(), lights.end());
        lightGroupChange.param_value.ARRAY = AiArrayConvert(
                lightGroupArray.size(), 1, AI_TYPE_NODE, lightGroupArray.data());

        AddNodeChange(&lightGroupChange);

        if (LocationIt->second.size() == 0)
        {
            // Remove this entry as there are no more light updates
            lightLinks.erase(LocationIt++);
        }
        else
        {
            // Iterate
            ++LocationIt;
        }
    }
}

int ArnoldRender::RerenderLightsXform( std::string& itemName, GroupAttribute attributesAttr )
{
    std::string lightShaderLocation = itemName + std::string( ":arnoldLightShader" );

    if ( attributesAttr.isValid() )
    {
        GroupAttribute xformAttr = attributesAttr.getChildByName( "xform" );
        if ( xformAttr.isValid() )
        {
            FnAttribute::DoubleAttribute xformMatrixAttr = xformAttr.getChildByName( "matrix" );

            if ( xformMatrixAttr.isValid() )
            {
                AtPoint pos, look_at;
                AtVector up;

                ProcessXFormMatrix( xformMatrixAttr.getNearestSample(0), pos, look_at, up );

                AtNode *node = AiNodeLookUpByName( lightShaderLocation.c_str() );

                // Don't look at osl_shader nodes, instead query for the :prim suffix
                const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry( node );
                if ( node && !strcmp( AiNodeEntryGetName( baseNodeEntry ), "osl_shader" ) )
                {
                    node = AiNodeLookUpByName( ( itemName + std::string( ":prim" ) ).c_str() );
                }

                if ( !node )
                {
                    std::cerr << "ArnoldRender::RerenderLightsXform: Cannot find node '" << itemName  << "'" << std::endl;
                }
                else
                {
                    // Covers the OSL light position case
                    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry( node );

                    if ( AiNodeEntryLookUpParameter( baseNodeEntry, "emit_light" ) )
                    {
                        KatParamChange change;
                        change.node        = node;
                        change.array_type  = 0;
                        change.param_name  = strdup( "" ); //dummy value
                        change.param_type  = AI_TYPE_INT; //dummy value
                        change.attr_data = xformMatrixAttr;
                        change.attr_callback = SetOSLLightXformAttrCallback;
                        AddNodeChange( &change );
                        AiRenderInterrupt();
                    }
                    else if ( !strcmp( AiNodeEntryGetName( baseNodeEntry ), "quad_light" ) ||
                            pystring::startswith(AiNodeEntryGetName( baseNodeEntry ), "physical_quad_light" ) )
                    {
                        KatParamChange change;
                        change.node        = node;
                        change.array_type  = 1;

                        change.param_name  = strdup( "vertices" );
                        change.param_type  = AI_TYPE_ARRAY;

                        DoubleConstVector m = xformMatrixAttr.getNearestSample(0);

                        Imath::M44d xform( m[0], m[1], m[2], m[3],
                                           m[4], m[5], m[6], m[7],
                                           m[8], m[9], m[10], m[11],
                                           m[12], m[13], m[14], m[15] );

                        Imath::Vec3<double> one(   1, -1, 0 );
                        Imath::Vec3<double> two(   1,  1, 0 );
                        Imath::Vec3<double> three(-1,  1, 0 );
                        Imath::Vec3<double> four( -1, -1, 0 );
                        Imath::Vec3<double> one1, two1, three1, four1;

                        xform.multVecMatrix( one,   one1 );
                        xform.multVecMatrix( two,   two1 );
                        xform.multVecMatrix( three, three1 );
                        xform.multVecMatrix( four,  four1 );

                        std::vector<float> verts;
                        verts.push_back( four1[0] ); verts.push_back( four1[1] ); verts.push_back( four1[2] );
                        verts.push_back( three1[0] ); verts.push_back( three1[1] ); verts.push_back( three1[2] );
                        verts.push_back( two1[0] ); verts.push_back( two1[1] ); verts.push_back( two1[2] );
                        verts.push_back( one1[0] ); verts.push_back( one1[1] ); verts.push_back( one1[2] );

                        change.param_value.ARRAY = AiArrayConvert( 4, 1, AI_TYPE_POINT, &verts[0] );

                        AddNodeChange( &change );
                        AiRenderInterrupt();
                    }
                    else
                    {
                        RerenderXform( lightShaderLocation, attributesAttr );
                    }
                }
            }
        }
    }

    return 0;
}

int ArnoldRender::RerenderCamera( std::string& itemName, GroupAttribute attributesAttr )
{
    GroupAttribute geomParams = attributesAttr.getChildByName( "geometry" );

    AtNode *node = AiNodeLookUpByName( itemName.c_str() );
    if (!node)
    {
        std::cerr << "ArnoldRender::RerenderCamera: Cannot find node '" << itemName  << "'" << std::endl;
        return 1;
    }

    if( !geomParams.isValid() )
    {
        std::cerr << "ArnoldRender::RerenderCamera: Node '" << itemName  << "' has no attribute 'geometry'." << std::endl;
        return 1;
    }

    KatParamChange change;
    change.node = node;

    FnAttribute::StringAttribute cameraTypeAttr = geomParams.getChildByName("projection");
    FnAttribute::DoubleAttribute leftAttr = geomParams.getChildByName( "left" );
    FnAttribute::DoubleAttribute rightAttr = geomParams.getChildByName( "right" );
    FnAttribute::DoubleAttribute bottomAttr = geomParams.getChildByName( "bottom" );
    FnAttribute::DoubleAttribute topAttr = geomParams.getChildByName( "top" );

    if ( leftAttr.isValid() && rightAttr.isValid() &&
        bottomAttr.isValid() && topAttr.isValid() )
    {
        double left = leftAttr.getValue();
        double right = rightAttr.getValue();
        double bottom = bottomAttr.getValue();
        double top = topAttr.getValue();

        // Work with orthographic cameras
        if( cameraTypeAttr.isValid() && cameraTypeAttr.getValue() == "orthographic" )
        {
            DoubleAttribute orthoWidthAttr = geomParams.getChildByName("orthographicWidth");
            double orthoWidth = orthoWidthAttr.getValue(0.0, false);
            if (orthoWidth > 0)
            {
                double w = (right - left);
                double h = (top - bottom);

                double orthoHeight = orthoWidth  * h / w;

                left *= 1.0 / w * orthoWidth;
                right *= 1.0 / w * orthoWidth;

                top *= 1.0 / h * orthoHeight;
                bottom *= 1.0 / h * orthoHeight;
            }
        }

        // Compensate for overscan
        int displayWindow[4] = {0, 0, local_display_window_size[0], local_display_window_size[1]};
        double displayWindow_w = (displayWindow[2] - displayWindow[0]) * 0.5;
        double displayWindow_h = (displayWindow[3] - displayWindow[1]) * 0.5;
        double overscanMult[4] = {(displayWindow_w + local_overscan[0]) / displayWindow_w,
                                  (displayWindow_h + local_overscan[1]) / displayWindow_h,
                                  (displayWindow_w + local_overscan[2]) / displayWindow_w,
                                  (displayWindow_h + local_overscan[3]) / displayWindow_h};
        double screen_window_x = (right + left) * 0.5;
        double screen_window_y = (top + bottom) * 0.5;

        left = screen_window_x + (left - screen_window_x) * overscanMult[0];
        bottom = screen_window_y + (bottom - screen_window_y) * overscanMult[1];
        right = screen_window_x + (right - screen_window_x) * overscanMult[2];
        top = screen_window_y + (top - screen_window_y) * overscanMult[3];

        double screen_window_w = (right - left);
        double screen_window_h = (top - bottom);

        // To compensate for non-square pixels we supply a square screen window
        float maxDimension = std::max(screen_window_w, screen_window_h);

        change.array_type  = 0;
        change.param_name  = strdup( "screen_window_min" );
        change.param_type  = AI_TYPE_POINT2;
        change.param_value.PNT2.x = screen_window_x - maxDimension * 0.5;
        change.param_value.PNT2.y = screen_window_y - maxDimension * 0.5;
        AddNodeChange(&change);

        change.param_name  = strdup( "screen_window_max" );
        change.param_type  = AI_TYPE_POINT2;
        change.param_value.PNT2.x = screen_window_x + maxDimension * 0.5;
        change.param_value.PNT2.y = screen_window_y + maxDimension * 0.5;
        AddNodeChange( &change );
        AiRenderInterrupt();
    }

    FnAttribute::DoubleAttribute fovAttr = geomParams.getChildByName( "fov" );
    if ( fovAttr.isValid() )
    {
        std::vector<float> fovValues;
        std::vector<float> sampleTimes;

        int64_t numSampleTimes = fovAttr.getNumberOfTimeSamples();
        std::set<float> attrSampleTimesSet;
        for (int64_t i = 0; i < numSampleTimes; ++i)
        {
            attrSampleTimesSet.insert(fovAttr.getSampleTime(i));
        }

        FnKat::RenderOutputUtils::findSampleTimesRelevantToShutterRange(
            sampleTimes,
            attrSampleTimesSet,
            sharedState.getShutterOpen(),
            sharedState.getShutterClose());

        for ( std::vector<float>::iterator it = sampleTimes.begin(); it != sampleTimes.end(); ++it )
        {
            fovValues.push_back(fovAttr.getNearestSample(*it)[0]);
        }

        change.array_type  = 1;
        change.param_name  = strdup( "fov" );
        change.param_type  = AI_TYPE_FLOAT;
        change.param_value.ARRAY = AiArrayConvert( 1, fovValues.size(), AI_TYPE_FLOAT, &fovValues[0] );
        AddNodeChange( &change );
        AiRenderInterrupt();
    }

    return 0;
}

int ArnoldRender::RerenderRenderSettings( std::string& itemName, GroupAttribute attr )
{
    GroupAttribute renderSettingsAttr = attr.getChildByName( "renderSettings" );
    if( !renderSettingsAttr.isValid() )
        return 1;

    FnAttribute::IntAttribute roiAttr = renderSettingsAttr.getChildByName( "ROI" );
    if( roiAttr.isValid() )
    {
        IntConstVector incrop = roiAttr.getNearestSample(0);
        int crop[4];
        int xres = sharedState.getXResolution();
        int yres = sharedState.getYResolution();

        KatParamChange change;

        std::ostringstream os;
        os << incrop[0] / (float) xres << ",";
        os << (incrop[0] + incrop[2] - 1) / (float) xres << ",";
        os << (yres - (incrop[1] + incrop[3] - 1)) / (float) yres << ",";
        os << (yres - incrop[1]) / (float) yres;
        std::string crop_window = os.str();

        const std::string crop_window_default = "";
        buildCropWindow( crop, crop_window, crop_window_default, xres, yres, &sharedState );

        AtNode *driverKatanaNode = sharedState.getDriverKatanaNode();
        if ( driverKatanaNode )
        {
            KatParamChange originChange;
            originChange.node = driverKatanaNode;
            originChange.param_name = strdup( "frame_origin" );
            originChange.array_type = 1;
            originChange.param_type = AI_TYPE_INT;
            originChange.param_value.ARRAY = AiArray( 2, 1, AI_TYPE_INT, crop[0], crop[2] );
            AddNodeChange( &originChange );
        }

        change.node        = AiUniverseGetOptions();
        change.array_type  = 0;

        change.param_name  = strdup( "region_min_x" );
        change.param_type  = AI_TYPE_INT;
        change.param_value.INT = crop[0];
        AddNodeChange( &change );

        change.param_name  = strdup( "region_max_x" );
        change.param_type  = AI_TYPE_INT;
        change.param_value.INT = crop[1];
        AddNodeChange( &change );

        change.param_name  = strdup( "region_min_y" );
        change.param_type  = AI_TYPE_INT;
        change.param_value.INT = crop[2];
        AddNodeChange( &change );

        change.param_name  = strdup( "region_max_y" );
        change.param_type  = AI_TYPE_INT;
        change.param_value.INT = crop[3];
        AddNodeChange( &change );

        AiRenderInterrupt();
    }

    return 0;
}

bool ArnoldRender::hasPendingDataUpdates() const
{
    if( NodeChangesAvailable() )
        return true;

    return false;
}

int ArnoldRender::applyPendingDataUpdates()
{
    /*
     * Apply any outstanding node changes
     */
    // Check if ROI is set
    bool ROI = FindNodeChange( "region_min_x" );
    // Check if AA_samples is set
    try
    {
        KatParamChange change = GetNodeChange( "AA_samples" );
        AA_samples = GetIntValue( &change );
    }
    catch (...) { }
    // Check if AA_samples_progressive_begin is set
    try
    {
        KatParamChange change = GetExtraChange( "AA_samples_progressive_begin" );
        AA_samples_prepass = GetIntValue( &change );
    }
    catch (...) { }
    // Check if AA_samples_progressive_step is set
    try
    {
        KatParamChange change = GetExtraChange( "AA_samples_progressive_step" );
        AA_samples_step = GetIntValue( &change );
    }
    catch (...) { }

    /*
     * Apply any outstanding node changes
     */
    if ( !skip_reset_options )
    {
        AtNode* camera = (AtNode*) AiNodeGetPtr( AiUniverseGetOptions(), "camera" );
        setCameraDefault( camera );
        setGlobalsDefault( getRootIterator(), &sharedState );
    }

    ApplyNodeChanges( getRootIterator(), &sharedState );
    ClearExtraChanges();

    int crop[4] = { -1, -1, -1, -1 };
    if ( ROI || partialUpdate )
    {
        crop[0] = AiNodeGetInt( AiUniverseGetOptions(), "region_min_x" );
        crop[1] = AiNodeGetInt( AiUniverseGetOptions(), "region_max_x" );
        crop[2] = AiNodeGetInt( AiUniverseGetOptions(), "region_min_y" );
        crop[3] = AiNodeGetInt( AiUniverseGetOptions(), "region_max_y" );
    }
    else
    {
        /*
         * Reset crop window
         */
        resetCropWindow( crop );
    }

    // Reset crop window from command line argument.
    //local_crop_window = original_crop_window;

    int i = AA_samples_prepass_rerender;
    int step = ( AA_samples_step > 0 ) ? AA_samples_step : 1;
    renderResult = AI_SUCCESS;

    for( ; i < AA_samples; i += step )
    {
        //AiMsgInfo( "[kat] Starting progressive %d", i );
        AiNodeSetInt( AiUniverseGetOptions(), "AA_samples", i );
        renderResult = AiRender( AI_RENDER_MODE_CAMERA );

        if( renderMethodName == FnKat::RendererInfo::LiveRenderMethod::kDefaultName && renderResult == AI_INTERRUPT )
        {
            break;
        }
        else if( renderResult != AI_SUCCESS )
        {
            if( renderResult == AI_ABORT )
                return 0;

            PrintRenderResults( renderResult );
            break;
        }
    }

    if( renderResult == AI_SUCCESS )
    {
        AiNodeSetInt( AiUniverseGetOptions(), "AA_samples", AA_samples );
        renderResult = AiRender( AI_RENDER_MODE_CAMERA );
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ArnoldRender::SetArguments( ArnoldRenderSettings &renderSettings )
{
    local_output_address = getKatanaHost();

    // Frame IDs and Channels
    FnKatRender::RenderSettings::ChannelBuffers interactiveBuffers;
    renderSettings.getChannelBuffers( interactiveBuffers );

    for( FnKatRender::RenderSettings::ChannelBuffers::const_iterator it = interactiveBuffers.begin(); it != interactiveBuffers.end(); ++it )
    {
        FnKatRender::RenderSettings::ChannelBuffer buffer = it->second;
        int frameID = atoi( buffer.bufferId.c_str() );

        local_frame_ids.push_back( frameID );
        local_output_channels.push_back( buffer.channelName );
    }

    // Frame time
    sharedState.setFrameTime(getRenderTime());
}

void ArnoldRender::SetupRender( FnScenegraphIterator rootIterator, ArnoldRenderSettings &renderSettings )
{
    // Treat batch renders as disk renders
    if( renderMethodName == FnKat::RendererInfo::DiskRenderMethod::kBatchName )
        renderMethodName = FnKat::RendererInfo::DiskRenderMethod::kDefaultName;

    if( renderMethodName == FnKat::RendererInfo::DiskRenderMethod::kDefaultName )
        processOutputs( rootIterator, renderSettings );

    GroupAttribute arnoldGlobalStatements = rootIterator.getAttribute("arnoldGlobalStatements");
    if (arnoldGlobalStatements.isValid())
    {
        // If this is already set e.g. through the command line then it will overwrite the ArnoldGlobalSettings
        if(local_dumpfile_filename.empty())
        {
            FnAttribute::StringAttribute assFileOption = arnoldGlobalStatements.getChildByName("assFile");

            if (assFileOption.isValid() )
            {
                local_dumpfile_filename = assFileOption.getValue("", false);
            }

            FnAttribute::IntAttribute forceExpandAttr = arnoldGlobalStatements.getChildByName("forceExpand");
            if (forceExpandAttr.isValid())
            {
                local_force_expand = forceExpandAttr.getValue(0, false);
            }

            FnAttribute::IntAttribute attr = arnoldGlobalStatements.getChildByName("suppressProceduralExpansion");
            if (attr.isValid())
            {
                if (!local_dumpfile_filename.empty())
                {
                    if (attr.getValue())
                    {
                        local_force_expand = true;
                        local_expand_procedurals_for_ass_dump = false;
                    }
                }
            }
        }
    }

    renderSettingsCamera = renderSettings.getCameraName();

    local_crop_window = renderSettings.getCropWindowAsString();

    int overscan[4];
    renderSettings.getOverscan( overscan );
    local_overscan[0] = overscan[0];
    local_overscan[1] = overscan[1];
    local_overscan[2] = overscan[2];
    local_overscan[3] = overscan[3];

    renderSettings.getDisplayWindowSize( local_display_window_size );

    int dataWindowSize[2];
    renderSettings.getDataWindowSize( dataWindowSize );
    local_output_width = dataWindowSize[0];
    local_output_height = dataWindowSize[1];
    sharedState.setXResolution(local_output_width);
    sharedState.setYResolution(local_output_height);

    int windowOrigin[2];
    renderSettings.getWindowOrigin( windowOrigin );
    local_frame_origin[0] = windowOrigin[0];
    local_frame_origin[1] = windowOrigin[1];

    if( useRenderPassID() )
        local_generate_ids = 1;

    InitArnold( rootIterator, renderSettings );

    adjustCheckpointDataForCropWindow();
    local_root_iterator = rootIterator;
}

AtNode* ArnoldRender::SetupDisplayDriver( const std::string& driverName, const std::string& frameName, const bool bucketCorners )
{
    AtNode* driverNode = AiNode( "driver_katana" );
    if ( !driverNode )
    {
        AiMsgError( "[kat] FATAL: unable to build the driver node." );
        exit( 1 );
    }

    AiNodeSetStr( driverNode, "name", driverName.c_str() );
    AiNodeSetStr( driverNode, "port", local_output_address.c_str() );
    AiNodeSetInt( driverNode, "frame_id", local_frame_id );
    AiNodeSetFlt( driverNode, "frame_time", sharedState.getFrameTime() );
    AiNodeSetStr( driverNode, "frame_name", frameName.c_str() );
    AiNodeSetBool( driverNode, "bucket_corners", bucketCorners );
    AiNodeSetBool( driverNode, "allow_id_pass", local_generate_ids );

    const AtNodeEntry* baseNodeEntry = AiNodeGetNodeEntry( driverNode );
    if ( AiNodeEntryLookUpParameter( baseNodeEntry, "frame_origin" ) )
    {
        AiNodeSetArray( driverNode, "frame_origin",
                        AiArray( 2, 1, AI_TYPE_INT, local_frame_origin[0], local_frame_origin[1] ) );
    }

    if ( !local_display_window_size[0] )
    {
        local_display_window_size[0] = local_output_width;
    }
    if ( !local_display_window_size[1] )
    {
        local_display_window_size[1] = local_output_height;
    }

    if ( AiNodeEntryLookUpParameter( baseNodeEntry, "display_window_size" ) )
    {
        AiNodeSetArray( driverNode, "display_window_size",
                        AiArray( 2, 1, AI_TYPE_INT, local_display_window_size[0], local_display_window_size[1] ) );
    }

    return driverNode;
}

void ArnoldRender::InitArnold( FnScenegraphIterator rootIterator, ArnoldRenderSettings &renderSettings )
{
    AiBegin(); // Setup Arnold.

    loadDriverKatana();
    setLogFlags(rootIterator);

    // If this is a checkpoint render, initialize flush_buckets_on_halt to false.
    // (can still be overridden explicitly in the scene globals).
    if( local_any_checkpoint_file )
    {
        AiNodeSetBool(AiUniverseGetOptions(), "flush_buckets_on_halt", false);
    }

    {
        // Load shader plugins
        const char* shaderPath = getenv("ARNOLD_SHADERLIB_PATH");
        if (!shaderPath)
        {
            AiMsgInfo("[kat] ARNOLD_SHADERLIB_PATH not set!");
        }
        else
        {
            typedef std::vector<std::string> str_vec;
            str_vec pathList;
            pystring::split(shaderPath, pathList, ":");
            for (str_vec::iterator it = pathList.begin(); it != pathList.end(); ++it)
                AiLoadPlugins(it->c_str());
        }
    }

    GroupAttribute arnoldGlobalStatements = rootIterator.getAttribute("arnoldGlobalStatements");

    // map old AA_filter and AA_filter_width global settings to primary filter
    std::string primaryFilterType = "gaussian_filter";
    float primaryFilterWidth = 2.0;
    std::string filterDomain = "";
    int scalarMode = -1;
    bool useMinMax = false;
    float filterMinimum = 0;
    float filterMaximum = 1;

    if( arnoldGlobalStatements.isValid() )
    {
        FnAttribute::IntAttribute AA_filter = arnoldGlobalStatements.getChildByName("AA_filter");
        if (AA_filter.isValid())
        {
            int index = AA_filter.getValue(0, false);

            switch (index)
            {
                case 0: primaryFilterType = "box_filter"; break;
                case 1: primaryFilterType = "disk_filter"; break;
                case 2: primaryFilterType = "triangle_filter"; break;
                case 3: primaryFilterType = "cone_filter"; break;
                case 4: primaryFilterType = "cubic_filter"; break;
                case 5: primaryFilterType = "catrom_filter"; break;
                case 6: primaryFilterType = "cook_filter"; break;
                case 7: primaryFilterType = "mitnet_filter"; break;
                case 9: primaryFilterType = "gaussian_filter"; break;
                case 10: primaryFilterType = "catrom2d_filter"; break;
                case 11: primaryFilterType = "closest_filter"; break;
                case 12: primaryFilterType = "farthest_filter"; break;
                case 13: primaryFilterType = "sinc_filter"; break;
                case 14: primaryFilterType = "variance_filter"; break;
                case 15: primaryFilterType = "video_filter"; break;
                //case 16: primaryFilterType = "filter_s3d_avg"; break;
                case 16: primaryFilterType = "blackman_harris_filter"; break;
                case 17: primaryFilterType = "heatmap_filter"; break;
            }
        }

        FnAttribute::IntAttribute AA_filter_domain = arnoldGlobalStatements.getChildByName("AA_filter_domain");
        if( primaryFilterType == "farthest_filter" && AA_filter_domain.isValid() )
        {
            int domainIndex = AA_filter_domain.getValue(0, false);
            if( domainIndex == 0 )
                filterDomain = "first_hit";
            else if( domainIndex == 1 )
                filterDomain = "all_hits";
        }

        FnAttribute::IntAttribute AA_filter_scalar_mode = arnoldGlobalStatements.getChildByName("AA_filter_scalar_mode");
        if (primaryFilterType == "variance_filter" && AA_filter_scalar_mode.isValid())
        {
            scalarMode = AA_filter_scalar_mode.getValue(0, false);
        }

        FnAttribute::FloatAttribute AA_filter_minimum = arnoldGlobalStatements.getChildByName("AA_filter_minimum");
        if (primaryFilterType == "heatmap_filter" && AA_filter_minimum.isValid())
        {
            filterMinimum = AA_filter_minimum.getValue(0.0, false);
        }

        FnAttribute::FloatAttribute AA_filter_maximum = arnoldGlobalStatements.getChildByName("AA_filter_maximum");
        if (primaryFilterType == "heatmap_filter" && AA_filter_maximum.isValid())
        {
            useMinMax = true;
            filterMaximum = AA_filter_maximum.getValue(1.0, false);
        }

        FnAttribute::FloatAttribute AA_filter_width =
            arnoldGlobalStatements.getChildByName("AA_filter_width");
        if (AA_filter_width.isValid())
        {
            useMinMax = true;
            primaryFilterWidth = AA_filter_width.getValue(2.0, false);
        }
    }

    //build crop values
    int crop[4] = {-1, -1, -1, -1};
    resetCropWindow(crop);

    {
        // This must be done BEFORE the output driver is set up,
        // or the output driver gets terribly, terribly confused at initialization.
        AiNodeSetInt(AiUniverseGetOptions(), "xres", local_output_width);
        AiNodeSetInt(AiUniverseGetOptions(), "yres", local_output_height);

        // Determine the correct pixel aspect ratio
        FnScenegraphIterator cameraIterator = rootIterator.getByPath(
                getDefaultRenderCameraName(rootIterator));
        GroupAttribute cameraGeometryAttr = cameraIterator.getAttribute("geometry");
        if (cameraGeometryAttr.isValid())
        {
            DoubleAttribute leftAttr = cameraGeometryAttr.getChildByName("left");
            DoubleAttribute rightAttr = cameraGeometryAttr.getChildByName("right");
            DoubleAttribute bottomAttr = cameraGeometryAttr.getChildByName("bottom");
            DoubleAttribute topAttr = cameraGeometryAttr.getChildByName("top");
            if (leftAttr.isValid() && rightAttr.isValid() &&
                        bottomAttr.isValid() && topAttr.isValid())
            {
                double screenWindowWidth = rightAttr.getValue() - leftAttr.getValue();
                double screenWindowHeight = topAttr.getValue() - bottomAttr.getValue();

                if (screenWindowWidth > 0.0 && screenWindowHeight > 0.0
                        && local_display_window_size[0] > 0.0
                        && local_display_window_size[1] > 0.0) {
                    float pixelAspectRatio =
                            (float(local_display_window_size[0]) / screenWindowWidth)
                            / (float(local_display_window_size[1]) / screenWindowHeight);

                    AiNodeSetFlt(AiUniverseGetOptions(), "aspect_ratio", pixelAspectRatio);
                }
            }
        }

        // If an output address is specified on the command, this is a bucket render.
        if( renderMethodName == FnKat::RendererInfo::PreviewRenderMethod::kDefaultName ||
            renderMethodName == FnKat::RendererInfo::LiveRenderMethod::kDefaultName )
        {
            if( local_output_address.empty() || local_output_address.find(':') == std::string::npos )
            {
                AiMsgError("[kat] FATAL: Host address missing in interactive render.");
            }

            std::vector<std::string> outputStrs;

            int noChannels = local_output_channels.size();
            int noFrameIds = local_frame_ids.size();

            FnAttribute::StringAttribute filterAttr;
            GroupAttribute filterParamsAttr;
            FnAttribute::StringAttribute channelAttr;
            FnAttribute::StringAttribute typeAttr;


            ////
            // Generate ids if flag set and this is the first iteration
            ////
            if( local_generate_ids && local_frame_ids.size())
            {
                local_frame_id = local_frame_ids.at(0);

                char frameIdChar[4];
                sprintf(frameIdChar, "%i", local_frame_id);
                std::string frameIdStr = frameIdChar;

                std::string idFilterName = "id_filter_" + frameIdStr;
                std::string idKatanaName = "id_katana_" + frameIdStr;

                initObjectId(local_output_address.c_str(), local_frame_id, &sharedState);

                AtNode * filterNode = AiNode("closest_filter");
                if (!filterNode)
                {
                    AiMsgError("[kat] FATAL: unable to build the closest_filter node.");
                    exit(1);
                }
                AiNodeSetStr(filterNode, "name", idFilterName.c_str());

                SetupDisplayDriver( idKatanaName, "__id", false );

                outputStrs.push_back("ID INT " + idFilterName + " " + idKatanaName);
            }


            int noIterations = std::max(noChannels, noFrameIds);

            for( int itNo = 0; itNo < noIterations; itNo++ )
            {
                local_output_channel = local_output_channels.at(std::min(itNo, noChannels));
                local_frame_id = local_frame_ids.at(std::min(itNo, noFrameIds));

                char frameIdChar[4];
                sprintf(frameIdChar, "%i", local_frame_id);
                std::string frameIdStr = frameIdChar;

                if( pystring::lower( local_output_channel ) != "rgba" && arnoldGlobalStatements.isValid() )
                {
                    std::string dcPrefix = "outputChannels." + local_output_channel;
                    filterAttr = arnoldGlobalStatements.getChildByName( dcPrefix + ".filter" );
                    filterParamsAttr = arnoldGlobalStatements.getChildByName( dcPrefix + ".filterParameters" );
                    channelAttr = arnoldGlobalStatements.getChildByName( dcPrefix + ".channel" );
                    typeAttr = arnoldGlobalStatements.getChildByName( dcPrefix + ".type" );
                }

                std::string filterStr = primaryFilterType;
                bool usePrimaryFilterWidth = true;
                if( filterAttr.isValid() )
                {
                    filterStr = filterAttr.getValue( "", false );
                    usePrimaryFilterWidth = false;
                }

                AtNode * filterNode = AiNode( filterStr.c_str() );
                const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry( filterNode );
                if( filterNode == 0x0 )
                {
                    AiMsgError( "[kat] FATAL: unable to build the interactive filter node of type '%s'.", filterStr.c_str() );
                    exit( 1 );
                }
                if( filterParamsAttr.isValid() )
                {
                    applyShaderParams( "filter", filterNode, filterParamsAttr, rootIterator, NULL, &sharedState );
                }
                else if( usePrimaryFilterWidth )
                {
                    if( AiNodeEntryLookUpParameter( baseNodeEntry, "width" ) )
                    {
                        AiNodeSetFlt( filterNode, "width", primaryFilterWidth );
                    }
                }

                if( filterDomain != "" )
                {
                    if( AiNodeEntryLookUpParameter( baseNodeEntry, "domain" ) )
                    {
                        AiNodeSetStr( filterNode, "domain", filterDomain.c_str() );
                    }
                }

                if ( scalarMode != -1 )
                {
                    if ( AiNodeEntryLookUpParameter( baseNodeEntry, "scalar_mode" ) )
                    {
                        AiNodeSetBool( filterNode, "scalar_mode", scalarMode );
                    }
                }

                if( useMinMax )
                {
                    if( AiNodeEntryLookUpParameter( baseNodeEntry, "minimum" ) )
                    {
                        AiNodeSetFlt( filterNode, "minimum", filterMinimum );
                    }
                    if( AiNodeEntryLookUpParameter( baseNodeEntry, "maximum" ) )
                    {
                        AiNodeSetFlt( filterNode, "maximum", filterMaximum );
                    }
                }

                std::string interactiveFilter = "interactive_filter_" + frameIdStr;

                // Set this last, just in case it happens to also be in filterParameters
                AiNodeSetStr(filterNode, "name", interactiveFilter.c_str());

                std::string interactiveChannel = channelAttr.getValue("RGBA", false);
                std::string interactiveType = typeAttr.getValue("RGBA", false);
                std::string interactiveName = "interactive_katana_" + frameIdStr;

                if (local_dumpfile_filename.empty())
                {
                    AtNode* driverNode = SetupDisplayDriver( interactiveName, local_frame_name, true );
                    if( driverNode )
                        sharedState.setDriverKatanaNode(driverNode);
                }
                else
                {
                    // If we're just going to dump an ass file, use dummy exr driver
                    AtNode* driverNode = AiNode("driver_exr");
                    if (!driverNode)
                    {
                        AiMsgError("[kat] FATAL: unable to build the driver node.");
                        exit(1);
                    }

                    interactiveName = "interactive_exr";
                    AiNodeSetStr(driverNode, "name", interactiveName.c_str());
                    AiNodeSetStr(driverNode, "filename", "/tmp/a.exr");
                }

                outputStrs.push_back(interactiveChannel + " " + interactiveType + " " + interactiveFilter + " " + interactiveName);
            }

            ////
            // Set outputs in the ass stream
            ////
            const int numOutputs = outputStrs.size();
            std::vector<const char*> outputs(numOutputs);

            for (int i = 0; i < numOutputs; ++i)
            {
                outputs[i] = outputStrs[i].c_str();
            }
            AiNodeSetArray(AiUniverseGetOptions(), "outputs",
                AiArrayConvert(numOutputs, 1, AI_TYPE_STRING, &outputs[0]));


            std::vector<std::string> lightExpressions;
            getLightExpressionsForBucketRender(rootIterator, lightExpressions);
            if (!lightExpressions.empty())
            {
                std::vector<const char *> oslPathRegexpsCStrings;
                for (std::vector<std::string>::const_iterator I =
                        lightExpressions.begin(); I!= lightExpressions.end(); ++I)
                {
                    oslPathRegexpsCStrings.push_back((*I).c_str());
                }

                AiNodeSetArray(AiUniverseGetOptions(), "osl_path_regexps",
                        AiArrayConvert(oslPathRegexpsCStrings.size(), 1, AI_TYPE_STRING, &oslPathRegexpsCStrings[0]));
            }
        }
        else if ( renderMethodName == FnKat::RendererInfo::DiskRenderMethod::kDefaultName && local_output_info_map.size() > 0 )
        {
            std::string defaultCamera = getDefaultRenderCameraName(rootIterator);
            std::string mainChannel = local_output_info_map[local_output_info_list[0]].channel;

            for( OutputInfoList::iterator I = local_output_info_list.begin(); I != local_output_info_list.end(); ++I)
            {
                const std::string & name = (*I);
                OutputInfo & outputInfo = local_output_info_map[name];

                std::string cameraName = outputInfo.cameraName;
                if (cameraName.empty())
                {
                    cameraName = defaultCamera;
                }

                std::string dcPrefix = "outputChannels." + outputInfo.channel;


                FnAttribute::StringAttribute driverAttr;
                if (arnoldGlobalStatements.isValid())
                {
                    driverAttr = arnoldGlobalStatements.getChildByName(dcPrefix + ".driver");
                }

                std::string driverStr = "driver_exr";
                bool setExr32BitUncompressed = true;
                if (driverAttr.isValid())
                {
                    driverStr = driverAttr.getValue("driver_exr", false);

                    // If user sets driver explicitly, rely on the driver defaults and
                    // the user's driverParameters (even if set to driver_exr).
                    setExr32BitUncompressed = false;
                }

                std::string driverName = "kat_driver_" + name;
                AtNode *driver = AiNode(driverStr.c_str());
                if (!driver)
                {
                    AiMsgDebug("[kat] unable to allocate output node '%s'", driverStr.c_str());
                    continue;
                }

                bool rawDriver;
                const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(driver);
                if (!AiMetaDataGetBool(baseNodeEntry, NULL, "raw_driver", &rawDriver))
                {
                    rawDriver = false;
                }

                if (rawDriver)
                {
                    GroupAttribute driverParamsAttr;
                    if (arnoldGlobalStatements.isValid())
                    {
                        driverParamsAttr = arnoldGlobalStatements.getChildByName(
                            dcPrefix + ".driverParameters");
                    }

                    if (driverParamsAttr.isValid())
                    {
                        applyShaderParams("driver", driver, driverParamsAttr, rootIterator, NULL, &sharedState);
                    }

                    // Set these last, just in case they happen to also be in driverParameters
                    AiNodeSetStr(driver, "name", driverName.c_str());
                    AiNodeSetStr(driver, "filename", outputInfo.file.c_str());

                    outputInfo.outputString = driverName;

                    if (outputInfo.lightAgnostic)
                    {
                        local_light_agnostic_passes[cameraName].push_back(name);
                    }
                    else
                    {
                        RenderPassKey key(cameraName, outputInfo.isolateLights);
                        local_render_passes[key].push_back(name);
                    }
                }
                else
                {
                    if (setExr32BitUncompressed)
                    {
                        AiNodeSetBool(driver, "half_precision", false);
                        AiNodeSetStr(driver, "compression", "none");
                    }

                    FnAttribute::StringAttribute typeAttr;
                    FnAttribute::StringAttribute channelAttr;
                    FnAttribute::StringAttribute filterAttr;
                    GroupAttribute filterParamsAttr;
                    GroupAttribute driverParamsAttr;
                    if (arnoldGlobalStatements.isValid())
                    {
                        typeAttr = arnoldGlobalStatements.getChildByName(
                            dcPrefix + ".type");
                        channelAttr = arnoldGlobalStatements.getChildByName(
                            dcPrefix + ".channel");
                        filterAttr = arnoldGlobalStatements.getChildByName(
                            dcPrefix + ".filter");
                        filterParamsAttr = arnoldGlobalStatements.getChildByName(
                            dcPrefix + ".filterParameters");
                        driverParamsAttr = arnoldGlobalStatements.getChildByName(
                            dcPrefix + ".driverParameters");


                        FnAttribute::StringAttribute attr = arnoldGlobalStatements.getChildByName(
                            dcPrefix+".lightExpressions");
                        if (attr.isValid())
                        {
                            StringConstVector le = attr.getNearestSample(0);
                            outputInfo.lightExpressions =
                                std::vector<std::string>(le.begin(), le.end());
                        }
                    }

                    std::string type = typeAttr.getValue("RGBA", false);

                    // Bail on unsupported types here
                    if (type == "MATRIX")
                    {
                        AiMsgDebug("[kat] skipping unimplemented output data type MATRIX");
                        continue;
                    }

                    if (driverParamsAttr.isValid())
                    {
                        applyShaderParams("driver", driver, driverParamsAttr, rootIterator, NULL, &sharedState);
                    }

                    // Set these last, just in case they happen to also be in driverParameters
                    AiNodeSetStr(driver, "name", driverName.c_str());
                    AiNodeSetStr(driver, "filename", outputInfo.file.c_str());

                    std::string filterName = "kat_filter_" + name;
                    std::string filterStr = primaryFilterType;
                    bool usePrimaryFilterWidth = true;
                    if (filterAttr.isValid())
                    {
                        filterStr = filterAttr.getValue(filterStr, false);
                        if (filterStr == "<inherit>")
                        {
                            filterAttr = arnoldGlobalStatements.getChildByName("outputChannels." + mainChannel + ".filter");
                            if (filterAttr.isValid())
                            {
                                filterStr = filterAttr.getValue();
                                // In case they set inherit on the primary for some reason.
                                if (filterStr == "<inherit>")
                                {
                                    filterStr = primaryFilterType;
                                    AiMsgInfo("[kat] WARNING: primary channel filter is set to <inherit>, falling back to default.");
                                }
                                else
                                {
                                    filterParamsAttr = arnoldGlobalStatements.getChildByName(
                                        "outputChannels." + mainChannel + ".filterParameters");
                                    AiMsgInfo("[kat] %s inheriting '%s' filter.", name.c_str(), filterStr.c_str());
                                }
                            }
                        }
                        else
                        {
                            usePrimaryFilterWidth = false;
                        }
                    }
                    AtNode *filter = AiNode(filterStr.c_str());
                    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(filter);
                    if (!filter)
                    {
                        AiMsgDebug("[kat] unable to allocate filter node '%s'", filterStr.c_str());
                        continue;
                    }
                    if (filterParamsAttr.isValid())
                    {
                        applyShaderParams("filter", filter, filterParamsAttr, rootIterator, NULL, &sharedState);
                    }
                    else if (usePrimaryFilterWidth)
                    {
                        if (AiNodeEntryLookUpParameter(baseNodeEntry, "width"))
                        {
                            AiNodeSetFlt(filter, "width", primaryFilterWidth);
                        }
                    }

                    if( filterDomain != "" )
                    {
                        if (AiNodeEntryLookUpParameter(baseNodeEntry, "domain"))
                        {
                            AiNodeSetStr(filter, "domain", filterDomain.c_str());
                        }
                    }

                    if( scalarMode != -1 )
                    {
                        if (AiNodeEntryLookUpParameter(baseNodeEntry, "scalar_mode"))
                        {
                            AiNodeSetBool(filter, "scalar_mode", scalarMode);
                        }
                    }

                    if( useMinMax )
                    {
                        if (AiNodeEntryLookUpParameter(baseNodeEntry, "minimum"))
                        {
                            AiNodeSetFlt(filter, "minimum", filterMinimum);
                        }
                        if (AiNodeEntryLookUpParameter(baseNodeEntry, "maximum"))
                        {
                            AiNodeSetFlt(filter, "maximum", filterMaximum);
                        }
                    }

                    // Set these last, just in case they happen to also be in driverParameters
                    AiNodeSetStr(filter, "name", filterName.c_str());

                    std::string channel = channelAttr.getValue("RGBA", false);

                    outputInfo.outputString = channel + " " + type + " " + filterName + " " + driverName;
                    // Prepend the lightExpression passes with channel and type
                    for (std::vector<std::string>::iterator I = outputInfo.lightExpressions.begin(); I!= outputInfo.lightExpressions.end(); ++I)
                    {
                        if (!(*I).empty())
                        {
                            (*I) = channel + " " + type + " " + (*I);
                        }
                    }

                    if (outputInfo.lightAgnostic)
                    {
                        local_light_agnostic_passes[cameraName].push_back(name);
                    }
                    else
                    {
                        RenderPassKey key(cameraName, outputInfo.isolateLights);
                        local_render_passes[key].push_back(name);
                    }
                }
            }
        }
        else
        {
            AiMsgError("[kat] FATAL: no output address or output file info for hot render.");
            exit(1);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Outputs
void ArnoldRender::processOutputs( FnScenegraphIterator rootIterator, ArnoldRenderSettings &renderSettings )
{
    FnKatRender::RenderSettings::RenderOutputs outputs = renderSettings.getRenderOutputs();
    std::vector<std::string> renderOutputNames = renderSettings.getRenderOutputNames();

    for( std::vector<std::string>::const_iterator it = renderOutputNames.begin(); it != renderOutputNames.end(); ++it )
    {
        FnKatRender::RenderSettings::RenderOutput renderOutput = outputs[(*it)];

        if( renderOutput.type == "color" || renderOutput.type == "raw" )
        {
            std::string name = *it;
            std::string file = renderOutput.renderLocation;
            std::string channel = renderOutput.channel;

            // Renderer settings
            // Isolate light
            std::set<std::string> isolateLights;
            FnKatRender::RenderSettings::AttributeSettings rendererSettings = renderOutput.rendererSettings;
            FnAttribute::StringAttribute isolateLightAttr = rendererSettings["isolateLight"];
            if( isolateLightAttr.isValid() )
            {
                StringConstVector isolateLightVector = isolateLightAttr.getNearestSample( 0.0f );
                isolateLights.clear();

                for ( StringConstVector::const_iterator I = isolateLightVector.begin(); I != isolateLightVector.end(); ++I )
                {
                    if ((*I) && (*I)[0])
                    {
                        isolateLights.insert( (*I) );
                    }
                }
            }

            // Camera name
            std::string cameraName = getStringAttrValue( rendererSettings["cameraName"] );

            // Light agnostic
            bool lightAgnostic = getAttrValue<int, FnAttribute::IntAttribute>( rendererSettings["lightAgnostic"], 0 );

            local_output_info_map[name] = OutputInfo(file, channel, isolateLights, cameraName, lightAgnostic);
            local_output_info_list.push_back(name);
        }
    }
}

void ArnoldRender::adjustCheckpointDataForCropWindow()
{
    int min_x = AiNodeGetInt(AiUniverseGetOptions(), "region_min_x");
    int min_y = AiNodeGetInt(AiUniverseGetOptions(), "region_min_y");

    for (OutputInfoMap::iterator iter=local_output_info_map.begin();
         iter!=local_output_info_map.end(); ++iter)
    {
        OutputInfo & outputInfo = (*iter).second;

        // Intentional copy, since we have to modify and reinsert everything
        TilePointSet origPoints = outputInfo.validTilePoints;
        TilePointSet &tilePoints = outputInfo.validTilePoints;
        tilePoints.clear();

        for (TilePointSet::iterator tilePointIter=origPoints.begin();
             tilePointIter!=origPoints.end(); ++tilePointIter)
        {
            tilePoints.insert(TilePoint((*tilePointIter).x+min_x,
                                        (*tilePointIter).y+min_y));
        }
    }

    // Intentional copy, since we have to modify and reinsert everything
    TilePointSet origAllTilePoints = local_checkpoint_data.allTilePoints;
    TilePointSet &allTilePoints = local_checkpoint_data.allTilePoints;
    allTilePoints.clear();
    for (TilePointSet::iterator tilePointIter=origAllTilePoints.begin();
         tilePointIter!=origAllTilePoints.end(); ++tilePointIter)
    {
        allTilePoints.insert(TilePoint((*tilePointIter).x+min_x,
                    (*tilePointIter).y+min_y));
    }
}

bool ArnoldRender::renderPassSetup( const std::string &cameraName,
                      const std::vector<std::string> &outputNames,
                      bool flushCache,
                      const std::string &origBucketScan,
                      int passCounter, int passTotal )
{
    TilePointSet renderTiles;
    bool calcRenderTiles = (local_checkpoint_data.tileSize[0] > 0 &&
                            local_checkpoint_data.tileSize[1] > 0);

    std::vector<std::string> checkpointWatchCmd;

    AiMsgInfo("[kat] ------------------------------------------------------");
    AiMsgInfo("[kat] Starting render pass %d of %d", passCounter, passTotal);
    AiMsgInfo("[kat] Rendering from camera: %s", cameraName.c_str());
    AtNode *cameraNode = buildCameraLocationByPath(cameraName, getRootIterator(), local_overscan, &sharedState, local_display_window_size);
    AiNodeSetPtr(AiUniverseGetOptions(), "camera", cameraNode);

    // Setup outputs array and build checkpoint_watch command
    std::vector<const char *> outputCStrings;
    std::vector<const char *> oslPathRegexpsCStrings;

    for (std::vector<std::string>::const_iterator strIter = outputNames.begin();
         strIter != outputNames.end(); ++strIter)
    {
        const OutputInfo & outputInfo = _getOutputInfoByName(*strIter);
        AiMsgInfo("[kat] Rendering output: %s", outputInfo.outputString.c_str());
        outputCStrings.push_back(outputInfo.outputString.c_str());

        for (std::vector<std::string>::const_iterator I =
                outputInfo.lightExpressions.begin();
                        I!= outputInfo.lightExpressions.end(); ++I)
        {
            oslPathRegexpsCStrings.push_back((*I).c_str());
        }


        if (!outputInfo.file.empty() && !outputInfo.checkpointFile.empty())
        {
            checkpointWatchCmd.push_back(outputInfo.file);
            checkpointWatchCmd.push_back(outputInfo.checkpointFile);
        }
        if (calcRenderTiles)
        {
            std::set_difference(local_checkpoint_data.allTilePoints.begin(),
                                local_checkpoint_data.allTilePoints.end(),
                                outputInfo.validTilePoints.begin(),
                                outputInfo.validTilePoints.end(),
                                std::inserter(renderTiles, renderTiles.begin()));
        }
    }

    // This must live till after the AiNodeSetArray call below (we will be
    // referencing the c_str() ptrs of its contents).
    std::vector<std::string> lightAgnosticOutputs;

    // Add in "lightAgnostic" outputs for matching camera, and remove from map
    StringVectorMap::iterator iter = local_light_agnostic_passes.find(cameraName);
    if (iter != local_light_agnostic_passes.end())
    {
        lightAgnosticOutputs = (*iter).second; // intentional copy, since map entry is erased.
        for (std::vector<std::string>::const_iterator strIter = lightAgnosticOutputs.begin();
             strIter != lightAgnosticOutputs.end(); ++strIter)
        {
            const OutputInfo & outputInfo = _getOutputInfoByName(*strIter);
            AiMsgInfo("[kat] Rendering lightAgnostic output: %s", outputInfo.outputString.c_str());
            outputCStrings.push_back(outputInfo.outputString.c_str());

            for (std::vector<std::string>::const_iterator I = outputInfo.lightExpressions.begin(); I != outputInfo.lightExpressions.end(); ++I)
            {
                oslPathRegexpsCStrings.push_back((*I).c_str());
            }

            if (!outputInfo.file.empty() && !outputInfo.checkpointFile.empty())
            {
                checkpointWatchCmd.push_back(outputInfo.file);
                checkpointWatchCmd.push_back(outputInfo.checkpointFile);
            }
            if (calcRenderTiles)
            {
                std::set_difference(local_checkpoint_data.allTilePoints.begin(),
                                    local_checkpoint_data.allTilePoints.end(),
                                    outputInfo.validTilePoints.begin(),
                                    outputInfo.validTilePoints.end(),
                                    std::inserter(renderTiles, renderTiles.begin()));
            }
        }

        local_light_agnostic_passes.erase(iter);
    }

    AiNodeSetArray(AiUniverseGetOptions(), "outputs",
        AiArrayConvert(outputCStrings.size(), 1, AI_TYPE_STRING, &outputCStrings[0]));

    AiNodeSetArray(AiUniverseGetOptions(), "osl_path_regexps",
                    AiArrayConvert(oslPathRegexpsCStrings.size(), 1, AI_TYPE_STRING,
                            &oslPathRegexpsCStrings[0]));
    if (calcRenderTiles)
    {
        if (renderTiles.empty())
        {
            AiMsgInfo("[kat] Don't need to render any buckets for this pass.  Skipping.");
            return false;
        }
        else
        {
            int totalTileNum = local_checkpoint_data.numTiles[0] *
                               local_checkpoint_data.numTiles[1];
            if (totalTileNum != (int)renderTiles.size())
            {
                std::vector<float> points;
                points.reserve(renderTiles.size()*2);
                for (TilePointSet::const_iterator iter = renderTiles.begin();
                    iter != renderTiles.end(); ++iter)
                {
                    points.push_back((float)(*iter).x);
                    points.push_back((float)(*iter).y);
                }
                AiNodeSetStr(AiUniverseGetOptions(), "bucket_scanning", "list");
                AiNodeSetArray(AiUniverseGetOptions(), "buckets",
                    AiArrayConvert(points.size()/2, 1, AI_TYPE_POINT2, &points[0]));

                AiMsgInfo("[kat] Rendering bucket list for checkpoint pickup:");
                AiMsgInfo("[kat]   (rendering %d of %d tiles (%0.1f%%)", (int)renderTiles.size(), totalTileNum, (int)renderTiles.size()*100.0/totalTileNum);
            }
            else
            {
                AiMsgInfo("[kat] Rendering full pass.");
                AiNodeSetStr(AiUniverseGetOptions(), "bucket_scanning", origBucketScan.c_str());
            }
        }
    }

    if (!checkpointWatchCmd.empty())
    {
        // Push these on front
        {
            std::ostringstream tmp;
            tmp << getpid();
            checkpointWatchCmd.insert(checkpointWatchCmd.begin(), tmp.str());
        }

        // Push command name on front
        {
            std::ostringstream tmp;
            tmp << ::getenv("KATANA_ROOT");
            tmp << "/MAIN/bin/checkpoint_watch";
            checkpointWatchCmd.insert(checkpointWatchCmd.begin(), tmp.str());
        }

        pid_t pid = _forkCommand(checkpointWatchCmd);
        if (pid > 0)
        {
            local_checkpoint_watch_pids.push_back(pid);
        }
    }

    return true;
}

void ArnoldRender::killCheckpointWatchProcesses()
{
    // Finished successfully.  Checkpoint processes not needed.
    for (std::vector<pid_t>::iterator iter=local_checkpoint_watch_pids.begin();
         iter!=local_checkpoint_watch_pids.end(); ++iter)
    {
#ifdef _WIN32
        std::cerr << "Kill not implemented for WIN32 platform!";
        assert(false); // NOT IMPLEMENTED!!
#else
        ::kill((*iter), SIGUSR1);
#endif
    }

    local_checkpoint_watch_pids.clear();
}

int ArnoldRender::getAssWriteIncludeOptions( GroupAttribute ags )
{
    if (!ags.isValid()) return AI_NODE_ALL;

    FnAttribute::StringAttribute contentsAttr = ags.getChildByName("assFileContents");
    if (contentsAttr.isValid())
    {
        const std::string value = contentsAttr.getValue("", false);

        if (value == "geometry")
        {
            return AI_NODE_SHAPE;
        }
        else if (value == "geometry and materials")
        {
            return AI_NODE_SHAPE | AI_NODE_SHADER;
        }
    }

    return AI_NODE_ALL;
}

/******************************************************************************
 *
 * resetCropWindow
 *
 ******************************************************************************/
void ArnoldRender::resetCropWindow( int* crop )
{
    //build crop values
    if( buildCropWindow( crop, local_crop_window, local_crop_window_default, local_output_width, local_output_height, &sharedState ) )
    {
        std::ostringstream infoMsg;
        infoMsg << "[kat] Rendering with cropWindow (";
        for ( unsigned int i = 0; i < 4; ++i )
        {
            if( i != 0 ) infoMsg << ", ";
            infoMsg << crop[i];
        }
        infoMsg << ")";
        AiMsgInfo( infoMsg.str().c_str() );
    }
    else
    {
        crop[0] = crop[2] = 0;
        crop[1] = local_output_width - 1;
        crop[3] = local_output_height - 1;
    }
    AiNodeSetInt( AiUniverseGetOptions(), "region_min_x", crop[0] );
    AiNodeSetInt( AiUniverseGetOptions(), "region_max_x", crop[1] );
    AiNodeSetInt( AiUniverseGetOptions(), "region_min_y", crop[2] );
    AiNodeSetInt( AiUniverseGetOptions(), "region_max_y", crop[3] );

    AtNode *driverNode = sharedState.getDriverKatanaNode();
    if(driverNode)
    {
        AiNodeSetArray( driverNode, "frame_origin",
                    AiArray( 2, 1, AI_TYPE_INT,
                    local_frame_origin[0], local_frame_origin[1] ) );
    }
}

void ArnoldRender::getLightExpressionsForBucketRender( FnScenegraphIterator rootIterator, std::vector<std::string> & output )
{
    std::string primaryChannel = "rgba";

    FnAttribute::StringAttribute attr = rootIterator.getAttribute( "renderSettings.outputs.primary.channel" );
    if ( attr.isValid() )
    {
        primaryChannel = attr.getValue( "rgba", false );
    }

    attr = rootIterator.getAttribute( "arnoldGlobalStatements.outputChannels." + primaryChannel + ".lightExpressions" );
    if ( attr.isValid() )
    {
        StringConstVector values = attr.getNearestSample( 0.0f );

        for ( StringConstVector::const_iterator I = values.begin(); I != values.end(); ++I )
        {
            std::string val(*I);
            if ( !val.empty() )
            {
                output.push_back( "RGBA RGBA " + val );
            }
        }
    }
}

void ArnoldRender::configureDiskRenderOutputProcess(
        FnKatRender::DiskRenderOutputProcess& diskRenderOutputProcess,
        const std::string& outputName,
        const std::string& outputPath,
        const std::string& renderMethodName,
        const float& frameTime) const
{
    // Get the recipe and retrieve the render settings
    FnScenegraphIterator rootIterator = getRootIterator();
    ArnoldRenderSettings renderSettings( rootIterator );

    // Build temporary render and final target locations
    std::string tempRenderLocation = FnKat::RenderOutputUtils::buildTempRenderLocation( rootIterator, outputName, "render", "exr", frameTime );
    std::string targetRenderLocation = outputPath;

    if( renderSettings.isTileRender() )
        targetRenderLocation = FnKat::RenderOutputUtils::buildTileLocation( rootIterator, outputPath );

    // Get the attributes for the render output from the recipe
    FnKatRender::RenderSettings::RenderOutput output = renderSettings.getRenderOutputByName( outputName );

    // The render action used for this render output
    std::auto_ptr<FnKatRender::RenderAction> renderAction;

    // Determine the rendering behaviour based on the output type
    if( output.type == "color" )
    {
        if( renderSettings.isTileRender() )
        {
            renderAction.reset( new FnKatRender::CopyRenderAction( targetRenderLocation, tempRenderLocation ) );
        }
        else
        {
            renderAction.reset( new FnKatRender::CopyAndConvertRenderAction( targetRenderLocation,
                    tempRenderLocation, output.clampOutput, output.colorConvert, output.computeStats,
                    output.convertSettings ) );
        }
    }
    else if( output.type == "raw" )
    {
        int rawHasOutput = getAttrValue<int, FnAttribute::IntAttribute>( output.rendererSettings["rawHasOutput"], 0 );
        if( rawHasOutput )
        {
            renderAction.reset( new FnKatRender::CopyRenderAction( targetRenderLocation, tempRenderLocation ) );
        }
        else
        {
            renderAction.reset( new FnKatRender::TemporaryRenderAction( tempRenderLocation ) );
            renderAction->setLoadOutputInMonitor( false );
        }
    }

    diskRenderOutputProcess.setRenderAction( renderAction );
}

std::string ArnoldRender::GetCheckpointFileName( const std::string& outputPath, const std::string& outputFileName )
{
    return outputPath + "/.checkpoint/" + outputFileName + ".CHECKPOINT";
}


// Plugin Registration code
DEFINE_RENDER_PLUGIN( ArnoldRender )

void registerPlugins()
{
    REGISTER_PLUGIN( ArnoldRender, "arnold", 0, 1 );
}
