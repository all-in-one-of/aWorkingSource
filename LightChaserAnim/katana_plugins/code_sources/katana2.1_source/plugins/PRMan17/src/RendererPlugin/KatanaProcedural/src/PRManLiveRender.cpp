// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <PRManLiveRender.h>
#include <PRManProcedural.h>
#include <WriteRI_Attributes.h>

#include <Ric.h>

#include <pystring/pystring.h>

bool hasDeletedAttribute( FnAttribute::GroupAttribute updateAttribute )
{
    FnAttribute::IntAttribute deletedAttr = updateAttribute.getChildByName( "deleted" );
    if ( deletedAttr.isValid() && deletedAttr.getValue() == 1)
        return true;

    return false;
}


PRManLiveRender::PRManLiveRender( bool raytrace, PRManPluginState* sharedState ) : raytrace( raytrace ),
                                                                                   sharedState( sharedState ),
                                                                                   flushMarker( 0 ),
                                                                                   prmanRerenderCount( 0 )
{
}

void PRManLiveRender::processUpdates( FnAttribute::GroupAttribute updateAttribute )
{
    lightsEdited = "";
    bool hasLightUpdates = false;

    for( int i = 0; i < updateAttribute.getNumberOfChildren(); i++ )
    {
        std::string type( ".type" );
        FnAttribute::GroupAttribute commandAttr = updateAttribute.getChildByIndex( i );

        if( commandAttr.isValid() )
        {
            FnAttribute::StringAttribute typeAttr = commandAttr.getChildByName( "type" );
            FnAttribute::StringAttribute locationAttr = commandAttr.getChildByName( "location" );
            FnAttribute::GroupAttribute attributesAttr = commandAttr.getChildByName( "attributes" );

            std::string location;
            if( locationAttr.isValid() )
                location = locationAttr.getValue( "", false );

            if( typeAttr.isValid() )
            {
                std::string type = typeAttr.getValue( "", false );

                PRManLiveRender::Update update;
                update.type = type;
                update.location = location;
                update.attributesAttr = attributesAttr;

                FnAttribute::GroupAttribute interactiveAttr;
                FnAttribute::GroupAttribute xformAttr = attributesAttr.getChildByName( "xform" );
                if ( xformAttr.isValid() )
                {
                    update.xformAttr = xformAttr;
                }

                if( type == "renderSettings" && attributesAttr.isValid() )
                {
                    FnAttribute::StringAttribute renderCameraAttr = attributesAttr.getChildByName( "renderSettings.cameraName" );
                    renderCamera = renderCameraAttr.getValue( "", false );
                }

                if( location == renderCamera && raytrace && xformAttr.isValid())
                {
                    cameras.push( update );
                }

                if ( type == "light" )
                {
                    hasLightUpdates = true;
                    lights.push( update );
                    lightsEdited += location + std::string( ":" );
                }
                else if ( type == "lightLink" )
                {
                    if(queueLightLinkUpdate(update))
                        lightLinks.push_back(update);
                }
                else if ( type == "geoMaterial" && raytrace )
                {
                    materials.push( update );
                }
                else if ( type == "coordinateSystem" && raytrace )
                {
                    coordinateSystems.push( update );
                }
            }
        }
    }

    // Always interrupt if raytracing or if any light updates are available in REYES mode
    if( ( raytrace || hasLightUpdates ) && getNumberOfUpdates() > 0 )
    {
        std::string marker = intToString( prmanRerenderCount );
        RicFlush( (char*) marker.c_str(), 0, RI_SUSPENDRENDERING );
    }
}

bool PRManLiveRender::queueLightLinkUpdate( Update& update )
{
    // Light linking is quite intensive due to the fact that the enable state
    // for each light can be set and overridden for each scenegraph location.
    // If we took the same approach as other update types - detecting the type
    // of update and blindly appending it to a queue - we would end up triggering
    // a rerender for every location even if there was nothing that needed updating.
    // Instead we use this function to determine if an update really is necessary
    // and if so we override the update.attributesAttr with only the information
    // we need (eg. The light location and its enabled state) and return true.
    //
    // Note that this function will still be called a lot during light linking
    // updates (so we should try to keep it fast), but at least it will not trigger
    // a rerender each time.
    FnAttribute::GroupAttribute attributesAttr = update.attributesAttr;

    if (!attributesAttr.isValid())
    {
        return false;
    }

    std::string location = update.location;
    FnAttribute::GroupAttribute lightListAttr =
                    attributesAttr.getChildByName("lightList");

    FnAttribute::GroupAttribute localLightListAttr =
            attributesAttr.getChildByName("localizedLightList");
    if (!localLightListAttr.isValid())
    {
        return false;
    }

    bool updateIsValid = false;
    FnAttribute::GroupBuilder optimizedAttributesGB;
    // Loop through the children of the light list - these are the attributes
    // specific to each light.
    for (unsigned i = 0; i < localLightListAttr.getNumberOfChildren(); ++i)
    {
        FnAttribute::GroupAttribute lightGrpAttr =
                localLightListAttr.getChildByIndex(i);
        if (!lightGrpAttr.isValid())
        {
            continue;
        }

        //Need to get the path for locations with light list changes
        FnAttribute::StringAttribute lightPathAttr =
                lightGrpAttr.getChildByName("path");
        if (!lightPathAttr.isValid())
        {
            continue;
        }

        // Determine whether this location has a locally defined 'enable'
        // attribute for this light
        LightEnabledState localLightAssignment = LIGHT_ENABLE_UNDEFINED;
        if(lightListAttr.isValid())
        {
            FnAttribute::GroupAttribute localLightGrpAttr =
                                lightListAttr.getChildByName(localLightListAttr.getChildName(i));
            if(localLightGrpAttr.isValid())
            {
                // This light value has been set locally
                FnAttribute::IntAttribute localLightEnableAttr = localLightGrpAttr.getChildByName("enable");
                if (localLightEnableAttr.isValid())
                {
                    localLightAssignment = (LightEnabledState)localLightEnableAttr.getValue();
                }
            }
        }

        std::string lightLocation = lightPathAttr.getValue("", false);

        // Check if light has material, i.e. if it's fully created in PRMan
        if(sharedState->getLightHandleForLocation(lightLocation) == RI_NULL)
        {
            // Light not fully created, defer updates to later processing
            deferredLightLinks[lightLocation].push_back(update);
            continue;
        }

        // Get the updated enable attribute
        FnAttribute::IntAttribute lightEnableAttr =
                lightGrpAttr.getChildByName("enable");
        if (!lightEnableAttr.isValid())
        {
            continue;
        }
        FnAttribute::IntAttribute parentLightEnableAttr =
                            lightGrpAttr.getChildByName("parentEnable");
        bool on = lightEnableAttr.getValue(1, false);
        bool parentOn = parentLightEnableAttr.getValue(1, false);
        bool cachedLocationChanged = false;

        std::map<std::string, LocationEnableMap>::iterator lightIt =
                            lightLinkOff.find(lightLocation);
        if (lightIt != lightLinkOff.end())
        {
            LocationEnableMap::iterator locIt = lightIt->second.find(location);
            if (locIt != lightIt->second.end())
            {
                bool oldEnableValue = locIt->second;
                if(on != oldEnableValue)
                    cachedLocationChanged = true;
            }
        }

        std::map<std::string, int>::iterator lightSwitch =
            lightSwitches.find(lightLocation);
        if (lightSwitch != lightSwitches.end())
        {
            if ((*lightSwitch).second == LIGHT_OFF
                    || (*lightSwitch).second == TURN_LIGHT_OFF)
            {
                // The light has been muted so don't trigger render updates, but
                // do keep track of the light link state so that it is restored
                // correctly when the light is un-muted.
                if( localLightAssignment == LIGHT_ENABLE_UNDEFINED)
                {
                    if(lightLinkOff.count(lightLocation) && lightLinkOff[lightLocation].count(location))
                        lightLinkOff[lightLocation].erase(location);
                }
                else
                {
                    lightLinkOff[lightLocation][location] = (bool)localLightAssignment;
                }

                continue;
            }
        }

        if(!parentLightEnableAttr.isValid() || on != parentOn || cachedLocationChanged)
        {
            // The lighting at this location has changed. Either:
            //    - The local 'enable' attribute has been altered since the last
            //      update, the 'enable' attribute.
            //    - The locations 'enable' attribute is different from it's direct
            //      parent (to catch changes to parent locations).

            // Since we've already done the calculations to know if the light needs
            // updating, just store the light location and the new enable flag.
            std::string lightGroupName = localLightListAttr.getChildName(i);
            optimizedAttributesGB.set( lightGroupName + ".lightLocation", FnAttribute::StringAttribute(lightLocation) );
            optimizedAttributesGB.set( lightGroupName + ".enable", FnAttribute::IntAttribute(on) );

            // Cache light link state to be restored on mute/solo changes
            if( localLightAssignment == LIGHT_ENABLE_UNDEFINED)
            {
                if(lightLinkOff.count(lightLocation) && lightLinkOff[lightLocation].count(location))
                    lightLinkOff[lightLocation].erase(location);
            }
            else
            {
                lightLinkOff[lightLocation][location] = (bool)localLightAssignment;
            }

            // Add to custom update!
            updateIsValid = true;
        }
    }

    update.attributesAttr = optimizedAttributesGB.build();
    return updateIsValid;
}

void PRManLiveRender::applyUpdates( FnKat::FnScenegraphIterator rootIterator )
{
    // We use a predefined flush marker to avoid multiple updates (progressive or otherwise) due
    // to an arbitrary number of edit blocks that might occur from the incoming re-render filters.
    // The marker value is set beforehand by incrementing the current re-render counter with a value
    // that is high enough to cover the number of potential edit blocks.
    // We issue an empty edit block after all edits have been issued to allow a dedicated flush stream
    // marker which uses the predetermined flush marker id.
    int numberOfUpdates = getNumberOfUpdates();
    flushMarker = prmanRerenderCount + numberOfUpdates + 100;
    RicFlush( (char*) intToString( flushMarker ).c_str(), 0, RI_SUSPENDRENDERING );

    // Perform updates on update lists
    if( cameras.size() > 0 && raytrace )
    {
        updateCamera( rootIterator );
    }

    if( lights.size() > 0 )
    {
        updateLight( rootIterator );
    }

    if( materials.size() > 0 && raytrace )
    {
        updateMaterial( rootIterator );
    }

    if( coordinateSystems.size() > 0 && raytrace )
    {
        updateCoordinateSystem();
    }

    if( lightLinks.size() > 0 )
    {
        updateLightLink( rootIterator );
    }

    // We use an empty edit here to allow the flush stream marker (see above),
    // and the re-render counter is then advanced to the flush marker.
    RiArchiveRecord( "structure", (char*) getStreamMarker( intToString( flushMarker ) ).c_str(), RI_NULL );
    RiEditBegin( "instance", RI_NULL );
    RiEditEnd();
    prmanRerenderCount = flushMarker;

}

int PRManLiveRender::getNumberOfUpdates()
{
    return cameras.size() + lights.size() + materials.size() +
           coordinateSystems.size() + lightLinks.size();
}

int PRManLiveRender::updateCamera( FnKat::FnScenegraphIterator rootIterator )
{
    RiArchiveRecord( "structure", (char*) getNextStreamMarker().c_str(), RI_NULL );
    RiEditBegin( "option", RI_NULL );

    while( cameras.size() )
    {
        Update update = cameras.front();
        cameras.pop();
        std::string location = update.location;
        FnAttribute::GroupAttribute attributesAttr = update.attributesAttr;
        if ( !update.xformAttr.isValid() )
            continue;

        std::vector<FnKat::RenderOutputUtils::Transform> transforms;
        fillXFormListFromAttributes( transforms, update.xformAttr );

        // Projection or Orthographic?
        FnKat::StringAttribute projectionAttr =
          attributesAttr.getChildByName("geometry.projection");
        bool isPerspective = true;
        if(projectionAttr.isValid() && projectionAttr.getValue() == "orthographic")
          isPerspective = false;

        // FOV
        if(isPerspective)
        {
            FnKat::DoubleAttribute fovAttr =
              attributesAttr.getChildByName("geometry.fov");

            float fov = 70.0;
            if(fovAttr.isValid())
                fov = fovAttr.getValue(fov, false);

            RiProjection( RI_PERSPECTIVE, "fov", &fov, RI_NULL );
        }
        else
        {
             RiProjection( RI_ORTHOGRAPHIC, RI_NULL );
        }

        // Screen window
        float screenWindow[4] = {-1.0, 1.0, -1.0, 1.0};
        FnKat::DoubleAttribute leftAttr =
          attributesAttr.getChildByName("geometry.left");
        if(leftAttr.isValid())
            screenWindow[0] = leftAttr.getValue(screenWindow[0], false);

        FnKat::DoubleAttribute bottomAttr =
          attributesAttr.getChildByName("geometry.bottom");
        if(bottomAttr.isValid())
            screenWindow[1] = bottomAttr.getValue(screenWindow[1], false);

        FnKat::DoubleAttribute rightAttr =
          attributesAttr.getChildByName("geometry.right");
        if(rightAttr.isValid())
            screenWindow[2] = rightAttr.getValue(screenWindow[2], false);

        FnKat::DoubleAttribute topAttr =
          attributesAttr.getChildByName("geometry.top");
        if(topAttr.isValid())
            screenWindow[3] = topAttr.getValue(screenWindow[3], false);

        if(!isPerspective)
        {
            // Compute the screenWindow based on the orthographic width of the camera.
            double orthographicWidth = 30;

            FnKat::DoubleAttribute orthographicWidthAttr =
                attributesAttr.getChildByName( "geometry.orthographicWidth" );
            if (orthographicWidthAttr.isValid())
            {
                orthographicWidth = orthographicWidthAttr.getValue( orthographicWidth, false );
            }

            float screenLeft = screenWindow[0];
            float screenRight = screenWindow[2];
            float screenBottom = screenWindow[1];
            float screenTop = screenWindow[3];
            float screenWidth = screenRight - screenLeft;
            float screenHeight = screenTop - screenBottom;

            screenLeft = screenLeft * orthographicWidth / screenWidth;
            screenRight = screenRight * orthographicWidth / screenWidth;
            screenTop = screenTop * orthographicWidth / screenHeight;
            screenBottom = screenBottom * orthographicWidth / screenHeight;

            screenWindow[0] = screenLeft;
            screenWindow[1] = screenBottom;
            screenWindow[2] = screenRight;
            screenWindow[3] = screenTop;
        }
        RiScreenWindow( screenWindow[0],    // Left
                        screenWindow[2],    // Right
                        screenWindow[1],    // Bottom
                        screenWindow[3] );  // Top

        RiCamera( "world", RI_NULL );

        // Transform.
        RiTransformBegin();
        RiIdentity();
        RiScale( 1, 1, -1 );
        PRManProcedural::WriteRI_Transforms( transforms );

        RiCamera( "world", RI_NULL );
        RiTransformEnd();
    }

    RiEditEnd();

    return 0;
}

int PRManLiveRender::updateLight( FnKat::FnScenegraphIterator rootIterator )
{
    RiArchiveRecord( "structure", (char*) getNextStreamMarker().c_str(), RI_NULL );

    if( raytrace )
    {
        RiEditBegin( "attribute", RI_NULL );
    }
    else
    {
        lightsEdited = lightsEdited.substr( 0, lightsEdited.length() - 1 );
        char* editlights = (char*) lightsEdited.c_str();

        RiEditBegin( "attribute", "string editlights", &editlights, RI_NULL );
    }

    bool hasLightLinkUpdates = false;
    while( !lights.empty() )
    {
        Update update = lights.front();
        lights.pop();
        std::string location = update.location;
        FnAttribute::GroupAttribute attributesAttr = update.attributesAttr;

        if ( !attributesAttr.isValid() )
            continue;

        if ( !update.xformAttr.isValid()  )
        {
            FnKat::FnScenegraphIterator lightProducer = rootIterator.getByPath( location );
            FnAttribute::GroupAttribute xformAttr = lightProducer.getAttribute( "xform" );
            if( xformAttr.isValid() )
            {
                update.xformAttr = xformAttr;
            }
        }

        // Check if this is a partial update, i.e. a custom update in continuous containing
        // an xform update from the viewer, color information from the color picker, etc.
        // This can be useful when determining which updates are required as a partial update
        // may not necessarily contain the attributes needed to determine the light's full state.
        std::map<std::string, int>::iterator lightSwitch = lightSwitches.find( location );
        bool partialUpdate = false;
        FnAttribute::StringAttribute partialUpdateAttr = attributesAttr.getChildByName( "partialUpdate" );
        if( partialUpdateAttr.isValid() && partialUpdateAttr.getValue() == "True" )
            partialUpdate = true;

        FnAttribute::GroupAttribute materialAttr = attributesAttr.getChildByName( "material" );
        if( materialAttr.isValid() )
        {
            if( lightShaderCache.find( location ) != lightShaderCache.end() )
            {
                // Update the existing material
                FnAttribute::GroupBuilder gb;
                gb.update( lightShaderCache[location] );
                gb.update( materialAttr );
                lightShaderCache[location] = gb.build();
            }
            else
            {
                // Cache the material
                lightShaderCache[location] = materialAttr;

                if( lightSwitch == lightSwitches.end() )
                    lightSwitches[location] = TURN_LIGHT_ON;
            }
        }
        else
        {
            if( lightShaderCache.find( location ) != lightShaderCache.end() )
            {
                // Fetching a valid material from the cache
                materialAttr = lightShaderCache[location];
            }
            else
            {
                // No incoming material and no material cached under this location.
                // Fall back on the producer until an update comes through
                FnKat::FnScenegraphIterator lightProducer = rootIterator.getByPath( location );
                materialAttr = lightProducer.getAttribute( "material" );
            }
        }

        FnAttribute::IntAttribute muteAttr = attributesAttr.getChildByName( "mute" );
        if( (muteAttr.isValid() && muteAttr.getValue() == 1) || hasDeletedAttribute(attributesAttr) )
        {
            lightSwitches[location] = TURN_LIGHT_OFF;
        }
        else
        {
            // Turn on a light where it is not muted but has been turned off due to an earlier mute.
            // This only applies to a full update as a partial continuous update may omit attributes
            // such as mute.
            if( lightSwitch != lightSwitches.end() && lightSwitches[location] == LIGHT_OFF && !partialUpdate )
            {
                lightSwitches[location] = TURN_LIGHT_ON;
            }
        }

        RiTransformBegin();


        if ( update.xformAttr.isValid() )
        {
            PRManProcedural::WriteRI_Object_Transform( update.xformAttr, rootIterator, sharedState );
        }

        if( materialAttr.isValid() )
        {
            FnAttribute::GroupAttribute infoAttr = attributesAttr.getChildByName( "info" );

            PRManProcedural::WriteShader( rootIterator, materialAttr, "light", "prmanLight", infoAttr, sharedState, location );

            if(sharedState->getLightHandleForLocation(location) != RI_NULL)
            {
                // Check for deferred light link updates related to this light
                std::map<std::string, UpdateDeque>::iterator lightIt =
                        deferredLightLinks.find(location);
                if (lightIt != deferredLightLinks.end())
                {
                    if(!lightIt->second.empty())
                        hasLightLinkUpdates = true;

                    // Insert pending updates into the light link queue
                    while (!lightIt->second.empty())
                    {
                        Update update = lightIt->second.back();
                        lightIt->second.pop_back();
                        if(queueLightLinkUpdate(update))
                            lightLinks.push_front(update);
                    }

                    deferredLightLinks.erase(lightIt);
                }
            }
        }

        RiTransformEnd();
    }

    RiEditEnd();

    if( hasLightLinkUpdates )
    {
        // Update light links for this light immediately so that
        // the final mute state can be set correctly without
        // being overridden
        updateLightLink( rootIterator );
    }

    // Illumination has to be done in a separate edit block. We go through the light switches created through the mute attribute
    // and turn lights on and off accordingly. A light switch is set to a corresponding idle state after it has been used.
    for( std::map<std::string, int>::const_iterator switchIt = lightSwitches.begin(); switchIt != lightSwitches.end(); switchIt++ )
    {
        if( switchIt->second == TURN_LIGHT_ON )
        {
            updateIlluminate( switchIt->first, true );
            lightSwitches[switchIt->first] = LIGHT_ON;

            // Restore light link exceptions at the proper scope
            std::map<std::string, LocationEnableMap>::iterator lightIt =
                    lightLinkOff.find(switchIt->first);
            if (lightIt != lightLinkOff.end())
            {
                for (LocationEnableMap::iterator locIt = lightIt->second.begin(),
                        locEndIt = lightIt->second.end(); locIt != locEndIt;
                        ++locIt)
                {
                    updateIlluminate(switchIt->first, locIt->second,
                            locIt->first);
                }
            }
        }
        else if( switchIt->second == TURN_LIGHT_OFF )
        {
            updateIlluminate( switchIt->first, false );
            lightSwitches[switchIt->first] = LIGHT_OFF;
        }
    }

    return 0;
}

int PRManLiveRender::updateMaterial( FnKat::FnScenegraphIterator rootIterator )
{
    RiArchiveRecord( "structure", (char*) getNextStreamMarker().c_str(), RI_NULL );

    while( !materials.empty() )
    {
        Update update = materials.front();
        materials.pop();
        std::string location = update.location;
        FnAttribute::GroupAttribute attributesAttr = update.attributesAttr;

        if( !attributesAttr.isValid() )
            continue;

        FnAttribute::GroupAttribute materialAttr = attributesAttr.getChildByName( "material" );
        if( !materialAttr.isValid() )
            continue;

        // The material may have been assigned at a different location upstream
        FnAttribute::StringAttribute materialLocation = attributesAttr.getChildByName( "material_location" );
        if( materialLocation.isValid() )
        {
            location = std::string( materialLocation.getValue( location, false ) );
        }

        FnKat::FnScenegraphIterator locationIterator = rootIterator.getByPath(location);
        if (locationIterator.getType() == "faceset")
        {
            // Shader binding for facesets has an instance id and is saved as a
            // RiResource. The shader scope is not available for rebinding
            // when editing.
            RiEditBegin( "instance", RI_NULL );
        }
        else
        {
            std::string scopeName = std::string( "^" ) + location + std::string( "$" );
            char* scope = (char*) scopeName.c_str();
            RiEditBegin( "attribute", "string scopename", &scope, RI_NULL );
        }

        // Process any co-shaders coming from upstream
        FnAttribute::GroupAttribute upstreamMaterials = attributesAttr.getChildByName( "material_upstream" );
        FnAttribute::GroupAttribute upstreamInfoAttr = attributesAttr.getChildByName( "info_upstream" );
        if( upstreamMaterials.isValid() && upstreamInfoAttr.isValid() )
        {
            for( int i = 0; i < upstreamMaterials.getNumberOfChildren(); i++ )
            {
                FnAttribute::GroupAttribute depMaterial = upstreamMaterials.getChildByIndex( i );
                FnAttribute::GroupAttribute coshaderInfoAttr = upstreamInfoAttr.getChildByName( upstreamMaterials.getChildName( i ) );

                if( depMaterial.isValid() && coshaderInfoAttr.isValid() )
                {
                    depMaterial = PRManProcedural::ConvertNetworkMaterialToCoshaders( depMaterial );
                    updateCoshaders( depMaterial, coshaderInfoAttr );
                }
            }
        }

        // Process co-shaders
        materialAttr = PRManProcedural::ConvertNetworkMaterialToCoshaders( materialAttr );
        FnAttribute::GroupAttribute infoAttr = attributesAttr.getChildByName( "info" );
        updateCoshaders( materialAttr, infoAttr );

        std::string rendererNamespace = "prman";
        PRManProcedural::WriteShader( rootIterator, materialAttr, "surface", rendererNamespace + std::string( "Surface" ), infoAttr, sharedState, location );
        PRManProcedural::WriteShader( rootIterator, materialAttr, "displacement", rendererNamespace + std::string( "Displacement" ), infoAttr, sharedState, location );
        PRManProcedural::WriteShader( rootIterator, materialAttr, "atmosphere", rendererNamespace + std::string( "Atmosphere" ), infoAttr, sharedState, location );
        PRManProcedural::WriteShader( rootIterator, materialAttr, "interior", rendererNamespace + std::string( "Interior" ), infoAttr, sharedState, location );
        PRManProcedural::WriteShader( rootIterator, materialAttr, "exterior", rendererNamespace + std::string( "Exterior" ), infoAttr, sharedState, location );

        RiEditEnd();
    }

    return 0;
}

int PRManLiveRender::updateCoordinateSystem()
{
    while( !coordinateSystems.empty() )
    {
        Update update = coordinateSystems.front();
        coordinateSystems.pop();
        std::string location = update.location;

        if ( !update.xformAttr.isValid() )
            continue;

        std::vector<std::string> locationHierarchy;
        pystring::split( location, locationHierarchy, "/" );
        if( locationHierarchy.size() == 0 )
            continue;

        RiArchiveRecord( "structure", (char*) getNextStreamMarker().c_str(), RI_NULL );
        RiEditBegin( "attribute", "string scopename", &location, RI_NULL );

        std::vector<FnKat::RenderOutputUtils::Transform> transforms;
        fillXFormListFromAttributes( transforms, update.xformAttr, 0.0f, false );

        RiTransformBegin();
        RiIdentity();

        PRManProcedural::WriteRI_Transforms( transforms );

        RiCoordinateSystem( const_cast<char*>( locationHierarchy[locationHierarchy.size() - 1].c_str() ) );

        RiTransformEnd();
        RiEditEnd();

    }

    return 0;
}

int PRManLiveRender::updateLightLink(FnKat::FnScenegraphIterator rootIterator)
{
    RiArchiveRecord("structure", (char*)getNextStreamMarker().c_str(), RI_NULL);
    while (!lightLinks.empty())
    {
        Update update = lightLinks.front();
        lightLinks.pop_front();
        std::string location = update.location;
        FnAttribute::GroupAttribute attributesAttr = update.attributesAttr;

        if (!attributesAttr.isValid())
            continue;

        for (unsigned i = 0; i < attributesAttr.getNumberOfChildren(); ++i)
        {
            FnAttribute::GroupAttribute lightGrpAttr = attributesAttr.getChildByIndex(i);
            if (!lightGrpAttr.isValid())
            {
                continue;
            }
            FnAttribute::StringAttribute lightAttr = lightGrpAttr.getChildByName("lightLocation");
            FnAttribute::IntAttribute enableAttr = lightGrpAttr.getChildByName("enable");
            if (!lightAttr.isValid() || !enableAttr.isValid())
            {
                continue;
            }

            std::string lightLocation = lightAttr.getValue();
            updateIlluminate(lightLocation, enableAttr.getValue(), location);
        }
    }
    return 0;
}

int PRManLiveRender::updateIlluminate( const std::string& lightLocation, bool on, const std::string& scopeName )
{
    RtString scope[] = { const_cast<char*>(scopeName.c_str()) };
    RtLightHandle lightHandle = sharedState->getLightHandleForLocation(lightLocation);
    if(lightHandle == RI_NULL)
    {
        // Light has not been declared previously
        return 1;
    }

    RiEditBegin( "attribute", "string scopename", (RtPointer) scope, RI_NULL );
    if( on )
        RiIlluminate( (RtPointer) lightLocation.c_str(), RI_TRUE );
    else
        RiIlluminate( (RtPointer) lightLocation.c_str(), RI_FALSE );

    RiEditEnd();
    return 0;
}

void PRManLiveRender::updateCoshaders( FnAttribute::GroupAttribute materialAttr,
                                       FnAttribute::GroupAttribute infoAttr )
{
    PRManProcedural::CoShadersInfo coshaders;
    PRManProcedural::GetCoshaders( materialAttr, coshaders );

    // Edit co-shaders
    if( coshaders.size() > 0 )
    {
        PRManProcedural::PrepareCoshaders( "/root", coshaders, sharedState, infoAttr );
        PRManProcedural::WriteCoshaders( coshaders, 0 );
    }
}

std::string PRManLiveRender::intToString( int number )
{
    char buffer[128];
    sprintf( buffer, "%d", number );
    std::string result = buffer;
    return result;
}

std::string PRManLiveRender::getStreamMarker( std::string markerId )
{
    std::string streamMarker = std::string( RI_STREAMMARKER ) + markerId;
    prmanRerenderCount++;
    return streamMarker;
}

std::string PRManLiveRender::getNextStreamMarker()
{
    return getStreamMarker( intToString( prmanRerenderCount ) );
}


