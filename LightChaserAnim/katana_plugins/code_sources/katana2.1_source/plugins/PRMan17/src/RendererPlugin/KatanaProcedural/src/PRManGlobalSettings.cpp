// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include "PRManGlobalSettings.h"

#include <vector>
#include <list>
#include <sstream>

int PRManGlobalSettings::initialise()
{
   return 0;
}

void PRManGlobalSettings::applyGlobalSettings()
{
    for ( int i = 0; i < _globalSettingsAttr.getNumberOfChildren(); i++ )
    {
        std::string name = _globalSettingsAttr.getChildName(i);

        FnAttribute::Attribute attribute = _globalSettingsAttr.getChildByIndex(i);
        if ( name == "options" )
        {
            // Options settings
            FnAttribute::GroupAttribute optionsAttr = attribute;
            if ( optionsAttr.isValid() )
            {
                for ( int option = 0; option < optionsAttr.getNumberOfChildren(); option++ )
                {
                    std::string optionName = optionsAttr.getChildName( option );

                    FnAttribute::GroupAttribute subOptions = optionsAttr.getChildByName( optionName );
                    recurseSubOptions( "options", optionName, subOptions, &RiOption );
                }
            }
        }
        else if ( name == "hider" )
        {
            // Hider settings
            FnAttribute::GroupAttribute hiderAttr = attribute;
            if ( hiderAttr.isValid() )
            {
                FnAttribute::StringAttribute hiderTypeAttr = hiderAttr.getChildByName( "hider" );

                // 'hidden' is the default hider
                std::string hiderType = "hidden";

                if ( hiderTypeAttr.isValid() )
                {
                    hiderType = hiderTypeAttr.getValue( "",false );
                }

                FnAttribute::GroupAttribute subOptions = hiderAttr.getChildByName( hiderType );
                recurseSubOptions( "hider", hiderType, subOptions, &RiHider );
            }
        }
        else if ( name == "pixelSamples" )
        {
            FnAttribute::FloatAttribute pixelSamplesAttr = attribute;

            if ( pixelSamplesAttr.isValid() )
            {
                FnAttribute::FloatConstVector pixelSamples = pixelSamplesAttr.getNearestSample( 0.f );
                RiPixelSamples( pixelSamples[0], pixelSamples[1] );
            }
        }
        else if ( name == "pixelVariance" )
        {
            FnAttribute::FloatAttribute pixelVarianceAttr = attribute;
            RiPixelVariance( pixelVarianceAttr.getValue( 0.f, false ) );
        }
        else if ( name == "shadingInterpolation" )
        {
            FnAttribute::StringAttribute shadingInterpolationAttr = attribute;
            char* shadingInterpolation = (char*)shadingInterpolationAttr.getValue( "constant", false ).c_str();
            RiShadingInterpolation( shadingInterpolation );
        }
        else if ( name == "shadingRate" )
        {
            FnAttribute::FloatAttribute shadingRateAttr = attribute;
            RiShadingRate( shadingRateAttr.getValue( 0.f, false ) );
        }
        else if ( name == "pixelFilter" )
        {
            typedef RtFloat(*RtFloatFunc)(RtFloat, RtFloat, RtFloat, RtFloat);
            std::map<std::string, RtFloatFunc> filterFuncMap;
            filterFuncMap["gaussian"] = RiGaussianFilter;
            filterFuncMap["sinc"] = RiSincFilter;
            filterFuncMap["blackman-harris"] = RiBlackmanHarrisFilter;
            filterFuncMap["mitchell"] = RiMitchellFilter;
            filterFuncMap["catmull-rom"] = RiCatmullRomFilter;
            filterFuncMap["triangle"] = RiTriangleFilter;
            filterFuncMap["box"] = RiBoxFilter;

            FnAttribute::GroupAttribute pixelFilterAttr = attribute;
            FnAttribute::StringAttribute filterFuncAttr = pixelFilterAttr.getChildByName( "filterFunc" );

            RtFloatFunc filterFunc = RiGaussianFilter;
            std::map<std::string, RtFloatFunc>::iterator iter = filterFuncMap.find( filterFuncAttr.getValue( "gaussian", false ) );
            if ( iter != filterFuncMap.end() )
            {
                filterFunc = iter->second;
            }

            FnAttribute::FloatAttribute widthAttr = pixelFilterAttr.getChildByName( "width" );
            float width = widthAttr.getValue( 2.f, false );

            FnAttribute::FloatAttribute heightAttr = pixelFilterAttr.getChildByName( "height" );
            float height = heightAttr.getValue( 2.f, false );

            RiPixelFilter( filterFunc, width, height );
        }
        else if ( name == "outputChannels" )
        {
            applyOutputChannelsGlobalSettings( attribute );
        }
        else if ( name == "camera" )
        {
            applyCameraGlobalSettings( attribute );
        }
    }

    // Set default search paths for display/procedural/shader if they were not
    // explicitly set in the global settings node
    RtString dsoPath[] = { (char*) _dsoPath.c_str() };
    RtString shaderPath[] = { (char*) _shaderPath.c_str() };

    if( !_displaySearchPathUsed )
    {
        RiOption( "searchpath", "string display", (RtPointer) dsoPath, RI_NULL );
    }

    if( !_proceduralSearchPathUsed )
    {
        RiOption( "searchpath", "string procedural", (RtPointer) dsoPath, RI_NULL );
    }

    if( !_shaderSearchPathUsed )
    {
        RiOption( "searchpath", "string shader", (RtPointer) shaderPath, RI_NULL );
    }
}

int PRManGlobalSettings::applyCameraGlobalSettings( FnAttribute::GroupAttribute cameraAttr )
{
    if ( cameraAttr.isValid() )
    {
        FnAttribute::GroupAttribute depthOfFieldAttr = cameraAttr.getChildByName( "depthOfField" );
        if ( depthOfFieldAttr.isValid() )
        {

            FnAttribute::StringAttribute fStopInfiniteAttr = depthOfFieldAttr.getChildByName( "fStopInfinite" );
            std::string fStopInfinite = fStopInfiniteAttr.getValue( "Yes", false );
            if ( fStopInfinite == "No" )
            {
                FnAttribute::FloatAttribute fStopAttr = depthOfFieldAttr.getChildByName( "fStop" );
                FnAttribute::FloatAttribute focalLenAttr = depthOfFieldAttr.getChildByName( "focalLen" );
                FnAttribute::FloatAttribute focalDistAttr = depthOfFieldAttr.getChildByName( "focalDist" );

                float fStop = fStopAttr.getValue( 22.f, false );
                float focalLen = focalLenAttr.getValue( 44.f, false );
                float focalDist = focalDistAttr.getValue( 1.f, false );

                RiDepthOfField( fStop, focalLen, focalDist );
            }
            else if ( fStopInfinite == "Yes" )
            {
                RiDepthOfField( RI_INFINITY, 0.f, 0.f );
            }
        }

        FnAttribute::FloatAttribute imagerBackgroundAttr = cameraAttr.getChildByName( "imagerBackground" );
        if ( imagerBackgroundAttr.isValid() )
        {
            FnAttribute::FloatConstVector imagerBackground = imagerBackgroundAttr.getNearestSample( 0.f );
            float* color = (float*) imagerBackground.data();
            RiImager( "background", "color background", color, RI_NULL );
        }

        FnAttribute::GroupAttribute shutterOpeningGroupAttr = cameraAttr.getChildByName( "shutterOpening" );
        if ( shutterOpeningGroupAttr.isValid() )
        {
            // constant rate is the default option
            int useConstantRate = 1;
            FnAttribute::FloatAttribute shutteropeningAttr;

            FnAttribute::IntAttribute useConstantRateAttr = shutterOpeningGroupAttr.getChildByName( "useConstantRate" );
            if ( useConstantRateAttr.isValid() )
            {
                useConstantRate = useConstantRateAttr.getValue( 1, false );
            }

            if ( useConstantRate == 0 )
            {
                shutteropeningAttr = shutterOpeningGroupAttr.getChildByName( "shutteropening_curve" );
            }
            else
            {
                shutteropeningAttr = shutterOpeningGroupAttr.getChildByName( "shutteropening_const" );
            }

            if ( shutteropeningAttr.isValid() )
            {
                FnAttribute::FloatConstVector shutteropening = shutteropeningAttr.getNearestSample( 0.f );
                if ( shutteropening.size() == 2 || shutteropening.size() == 10 )
                {
                    std::stringstream buffer;
                    buffer << "float[" << shutteropening.size() << "] shutteropening";
                    RiCamera( "frame", buffer.str().c_str(), (RtPointer) shutteropening.data(), RI_NULL );
                }
            }
        }
    }

    return 0;
}

std::string PRManGlobalSettings::getFrameBeginCameraPath()
{
    FnAttribute::GroupAttribute cameraAttr = _globalSettingsAttr.getChildByName( "camera" );
    if( cameraAttr.isValid() )
    {
        FnAttribute::StringAttribute frameBeginCameraPathAttr = cameraAttr.getChildByName( "frameBeginCameraPath" );
        if ( frameBeginCameraPathAttr.isValid() )
        {
            return frameBeginCameraPathAttr.getValue();
        }
    }

    return "";
}

int PRManGlobalSettings::applyOutputChannelsGlobalSettings( FnAttribute::GroupAttribute outputChannelsAttr )
{
    if ( outputChannelsAttr.isValid() )
    {
        for ( int channelIndex = 0; channelIndex < outputChannelsAttr.getNumberOfChildren(); channelIndex++ )
        {
            // Set the name of the attribute as default value for the channel
            // name. This however cannot contain dots and symbols that are not
            // allowed in attribute names. We should update this with the
            // "name" child attribute in the channel attribute.
            std::string channelName = outputChannelsAttr.getChildName( channelIndex );

            FnAttribute::GroupAttribute channelAttr = outputChannelsAttr.getChildByIndex( channelIndex );
            if ( channelAttr.isValid() )
            {
                std::vector<RtToken> tokens;
                std::vector<std::string> tokensCache;
                std::vector<RtPointer> values;
                std::vector<FnAttribute::StringAttribute> stringAttrCache;
                // float vector list used to store values converted from DoubleAttributes.
                // Notice that floatVectorList cannot be declared locally to the DoubleAttribute
                // case as it needs to hold float values until the RiDisplayChannelV call.
                typedef std::vector<float> FloatVector;
                std::list<FloatVector> floatVectorList;

                FnAttribute::GroupAttribute paramsAttr = channelAttr.getChildByName( "params" );

                // Update the channelName with the stored name. This can
                // actually contain punctuation and symbols.
                channelName = FnAttribute::StringAttribute(
                        channelAttr.getChildByName( "name" ))
                                .getValue( channelName, false );

                for ( int paramIndex = 0; paramIndex < paramsAttr.getNumberOfChildren(); paramIndex++ )
                {

                    std::string paramName = paramsAttr.getChildName(paramIndex);
                    FnAttribute::GroupAttribute paramAttr = paramsAttr.getChildByIndex(paramIndex);
                    if ( paramAttr.isValid() )
                    {
                        FnAttribute::StringAttribute typeAttr = paramAttr.getChildByName( "type" );
                        std::string type = typeAttr.getValue( "", false );
                        std::string param = type + std::string( " " ) + paramName;

                        FnAttribute::Attribute valueAttr = paramAttr.getChildByName( "value" );
                        FnAttribute::IntAttribute intValueAttr = valueAttr;
                        if ( intValueAttr.isValid() )
                        {
                            tokensCache.push_back( param );
                            FnAttribute::IntConstVector intValue = intValueAttr.getNearestSample( 0.f );
                            values.push_back( (RtPointer) intValue.data() );
                        }

                        FnAttribute::FloatAttribute floatValueAttr = valueAttr;
                        if ( floatValueAttr.isValid() )
                        {
                            tokensCache.push_back( param );
                            FnAttribute::FloatConstVector floatValue = floatValueAttr.getNearestSample( 0.f );
                            values.push_back( (RtPointer) floatValue.data() );
                        }

                        FnAttribute::DoubleAttribute doubleValueAttr = valueAttr;
                        if ( doubleValueAttr.isValid() )
                        {
                            tokensCache.push_back( param );
                            FnAttribute::DoubleConstVector doubleValue = doubleValueAttr.getNearestSample( 0.f );
                            // PRMan only supports float parameters so here we need to convert doubles to floats.
                            floatVectorList.push_back(FloatVector());
                            FloatVector& floatVector = floatVectorList.back();
                            floatVector.reserve(doubleValue.size());
                            for (size_t i = 0; i < doubleValue.size(); ++i)
                            {
                                floatVector.push_back(doubleValue[i]);
                            }
                            values.push_back( (RtPointer) floatVector.data() );
                        }

                        FnAttribute::StringAttribute stringValueAttr = valueAttr;
                        if ( stringValueAttr.isValid() )
                        {
                            tokensCache.push_back( param );
                            FnAttribute::StringConstVector stringValue = stringValueAttr.getNearestSample( 0.f );
                            stringAttrCache.push_back( stringValueAttr );
                            values.push_back( (RtPointer) stringValue.data() );
                        }
                    }
                }

                for ( size_t tokenIndex = 0; tokenIndex < tokensCache.size(); tokenIndex++ )
                {
                    tokens.push_back( (RtToken) tokensCache[tokenIndex].c_str() );
                }

                FnAttribute::StringAttribute channelTypeAttr = channelAttr.getChildByName( "type" );
                std::string channel = channelTypeAttr.getValue( "", false ) + std::string( " " ) + channelName;
                RtInt paramSize = tokens.size();
                RiDisplayChannelV( (char*) channel.c_str(), paramSize, (RtToken*) &tokens[0], (RtPointer*) &values[0] );
            }
        }
    }

    return 0;
}

int PRManGlobalSettings::recurseSubOptions( std::string optionsName, std::string& optionName,
                                            FnAttribute::GroupAttribute subOptions, void (*riCommand)(char *, ...) )
{
    std::map<std::string,std::string> prmanGlobalsMap;
    getGlobalTypeMap( prmanGlobalsMap );

    if ( subOptions.isValid() )
    {
        for ( int subOptionIndex = 0; subOptionIndex < subOptions.getNumberOfChildren(); subOptionIndex++ )
        {
            std::string subOption = subOptions.getChildName( subOptionIndex );

            // 'cachemode' has to be handled as a special case because the actual option's
            // name is '__cachemode', that is not supported in the Xml presentation
            if (subOption == "cachemode")
            {
                subOption = "__cachemode";
            }

            std::string subOptionName = optionsName + std::string( "." ) +
                    optionName + std::string( "." ) +
                    subOption;
            FnAttribute::Attribute subOptionAttr = subOptions.getChildByIndex( subOptionIndex );

            std::string type = "";
            std::map<std::string,std::string>::iterator typeIter = prmanGlobalsMap.find( subOptionName );
            if ( typeIter != prmanGlobalsMap.end() )
            {
                type = typeIter->second;
            }

            applyOption( optionName, subOption, type, subOptionAttr, riCommand );
        }
    }

    return 0;
}

std::string PRManGlobalSettings::getOptionParameter( std::string name, std::string type, int size )
{
    char sizeBuffer[10];
    std::string dimStr = " ";

    if( size > 1 )
    {
        sprintf( sizeBuffer, "%i", size );
        dimStr = "[" + std::string( sizeBuffer ) + "] ";
    }

    return type + dimStr + name;
}

int PRManGlobalSettings::applyOption( std::string name, std::string parameterName, std::string parameterType,
                                      FnAttribute::Attribute parameterAttr, void (*riCommand)(char *, ...) )
{
    // Check if we have a valid type or not
    std::string parameter = parameterName;
    if ( parameterType != "" )
    {
        parameter = parameterType + std::string(" ") + parameterName;
    }

    // Int data, can contain bools as well
    FnAttribute::IntAttribute intSubOptionAttr = parameterAttr;
    if ( intSubOptionAttr.isValid() )
    {
        FnAttribute::IntConstVector subOptionValue = intSubOptionAttr.getNearestSample( 0.f );
        if ( parameterType == "" )
        {
            parameter = getOptionParameter( parameterName, "int", subOptionValue.size() );
        }
        riCommand( (char*) name.c_str(), (char*) parameter.c_str(), subOptionValue.data(), RI_NULL );
    }

    // Float data, these can also contain colors
    FnAttribute::FloatAttribute floatSubOptionAttr = parameterAttr;
    if ( floatSubOptionAttr.isValid() )
    {
        FnAttribute::FloatConstVector subOptionValue = floatSubOptionAttr.getNearestSample( 0.f );
        if ( parameterType == "" )
        {
            parameter = getOptionParameter( parameterName, "float", subOptionValue.size() );
        }
        riCommand( (char*) name.c_str(), (char*) parameter.c_str(), subOptionValue.data(), RI_NULL );
    }

    // Double data
    FnAttribute::DoubleAttribute doubleSubOptionAttr = parameterAttr;
    if ( doubleSubOptionAttr.isValid() )
    {
        // Since 'double' is not supported by PRMan, convert to a float
        std::vector<float> subOptionValueFloat;
        FnAttribute::DoubleConstVector subOptionValue = doubleSubOptionAttr.getNearestSample( 0.f );
        for ( unsigned int i = 0; i < subOptionValue.size(); i++ )
        {
            subOptionValueFloat.push_back( (float)subOptionValue.at( i ) );
        }
        if ( parameterType == "" )
        {
            parameter = getOptionParameter( parameterName, "float", subOptionValue.size() );
        }
        riCommand( (char*) name.c_str(), (char*) parameter.c_str(), subOptionValueFloat.data(), RI_NULL );
    }

    // String data
    FnAttribute::StringAttribute stringSubOptionAttr = parameterAttr;
    if ( stringSubOptionAttr.isValid() )
    {
        FnAttribute::StringConstVector subOptionValue = stringSubOptionAttr.getNearestSample( 0.f );
        if ( parameterType == "" )
        {
            parameter = getOptionParameter( parameterName, "string", subOptionValue.size() );
        }
        
        if ( parameterType == "uniform int" || parameterType == "constant int" )
        {
            const int value = ( stringSubOptionAttr.getValue( "No", false ) == "Yes" ) ? 1 : 0;
            riCommand( (char*) name.c_str(), (char*) parameter.c_str(), &value, RI_NULL );
        }
        else if ( parameterType == "uniform float" || parameterType == "constant float" )
        {
            const float value = ( stringSubOptionAttr.getValue( "No", false ) == "Yes" ) ? 1.0f : 0.0f;
            riCommand( (char*) name.c_str(), (char*) parameter.c_str(), &value, RI_NULL );
        }
        else
        {
            // We have special handling for certain search paths as we set their values implicitly
            if( name == "searchpath" )
            {
                std::string value = subOptionValue.at( 0 );

                if( parameterName == "display" )
                {
                    if ( !_dsoPath.empty() )
                    {
                        value += std::string( ":" ) + _dsoPath;
                    }                    
                    _displaySearchPathUsed = true;
                }
                else if( parameterName == "procedural" )
                {
                    if ( !_dsoPath.empty() )
                    {
                        value += std::string( ":" ) + _dsoPath;
                    }   
                    _proceduralSearchPathUsed = true;
                }
                else if( parameterName == "shader" )
                {
                    if ( !_shaderPath.empty() )
                    {
                        value += std::string( ":" ) + _shaderPath;
                    }   
                    _shaderSearchPathUsed = true;
                    _shaderPath = value;
                }

                RtString searchPath[] = { (char*) value.c_str() };
                riCommand( (char* ) name.c_str(), (char*) parameter.c_str(), (RtPointer) searchPath, RI_NULL );
            }
            else
            {
                riCommand( (char* ) name.c_str(), (char*) parameter.c_str(), subOptionValue.data(), RI_NULL );
            }
        }
    }

    return 0;
}

void PRManGlobalSettings::getGlobalTypeMap( std::map<std::string,std::string> &prmanGlobalsMap )
{
    prmanGlobalsMap["options.limits.bucketsize"] = "constant int[2]";
    prmanGlobalsMap["options.limits.gridsize"] = "constant int";
    prmanGlobalsMap["options.limits.gridmemory"] = "constant int";
    prmanGlobalsMap["options.limits.texturememory"] = "constant int";
    prmanGlobalsMap["options.limits.threads"] = "constant int";
    prmanGlobalsMap["options.limits.zthreshold"] = "color";
    prmanGlobalsMap["options.limits.othreshold"] = "color";
    // 'extremedisplacement' deprecated - kept for backward compatibility
    prmanGlobalsMap["options.limits.extremedisplacement"] = "constant int";
    prmanGlobalsMap["options.limits.deepshadowerror"] = "constant float";
    prmanGlobalsMap["options.limits.deepshadowsimplifyerror"] = "constant float";
    prmanGlobalsMap["options.limits.deepshadowtiles"] = "constant int";
    prmanGlobalsMap["options.limits.geocachememory"] = "constant int";
    prmanGlobalsMap["options.limits.brickmemory"] = "constant int";
    prmanGlobalsMap["options.limits.hemispheresamplememory"] = "constant int";
    prmanGlobalsMap["options.limits.octreememory"] = "constant int";
    prmanGlobalsMap["options.limits.pointmemory"] = "constant int";
    prmanGlobalsMap["options.limits.proceduralmemory"] = "constant int";
    prmanGlobalsMap["options.limits.deepshadowmemory"] = "constant int";
    prmanGlobalsMap["options.limits.radiositycachememory"] = "constant int";
    prmanGlobalsMap["options.limits.vpdepthshadingrate"] = "constant float";
    prmanGlobalsMap["options.limits.vprelativeshadingrate"] = "constant float";
    prmanGlobalsMap["options.limits.vpvolumeintersections"] = "uniform float";
    prmanGlobalsMap["options.limits.deepshadowdeptherror"] = "constant float";
    prmanGlobalsMap["options.limits.ptexturememory"] = "constant int";
    prmanGlobalsMap["options.limits.ptexturemaxfiles"] = "constant int";
    prmanGlobalsMap["options.limits.vpinteriorheuristic"] = "uniform int";

    prmanGlobalsMap["options.shade.__cachemode"] = "constant int";

    prmanGlobalsMap["options.shading.debug"] = "constant int";
    prmanGlobalsMap["options.shading.derivsfollowdicing"] = "constant int";
    prmanGlobalsMap["options.shading.checknans"] = "constant int";
    prmanGlobalsMap["options.shading.defcache"] = "constant float";
    prmanGlobalsMap["options.shading.objectcache"] = "constant float";

    prmanGlobalsMap["options.bucket.order"] = "constant string";
    prmanGlobalsMap["options.bucket.orderorigin"] = "constant int[2]";

    prmanGlobalsMap["options.statistics.endofframe"] = "constant int";
    prmanGlobalsMap["options.statistics.filename"] = "constant string";
    prmanGlobalsMap["options.statistics.xmlfilename"] = "constant string";
    prmanGlobalsMap["options.statistics.shaderprofile"] = "constant string";
    prmanGlobalsMap["options.statistics.displace_ratios"] = "constant float[2]";
    prmanGlobalsMap["options.statistics.maxdispwarnings"] = "constant int";
    prmanGlobalsMap["options.statistics.stylesheet"] = "constant string";

    prmanGlobalsMap["options.shadow.bias"] = "constant float";
    prmanGlobalsMap["options.texture.enable_gaussian"] = "constant float";
    prmanGlobalsMap["options.texture.enable_lerp"] = "constant float";
    prmanGlobalsMap["options.texture.texturefilter"] = "constant string";

    prmanGlobalsMap["options.shutter.clampmotion"] = "constant int";
    prmanGlobalsMap["options.shutter.offset"] = "constant float";

    prmanGlobalsMap["options.trace.maxdepth"] = "uniform int";
    prmanGlobalsMap["options.trace.samplefactor"] = "uniform float";
    // 'specularthreshold' deprecated - kept for backward compatibility
    prmanGlobalsMap["options.trace.specularthreshold"] = "uniform float";
    prmanGlobalsMap["options.trace.decimationrate"] = "int";
    // 'continuationbydefault' deprecated - kept for backward compatibility
    prmanGlobalsMap["options.trace.continuationbydefault"] = "constant int";

    prmanGlobalsMap["options.photon.emit"] = "constant int";
    prmanGlobalsMap["options.photon.lifetime"] = "constant string";

    prmanGlobalsMap["options.render.rerenderbake"] = "constant int";
    prmanGlobalsMap["options.render.rerenderbakedbdir"] = "constant string";
    prmanGlobalsMap["options.render.rerenderbakedbname"] = "constant string";
    prmanGlobalsMap["options.rerender.lodrange"] = "constant int[2]";

    prmanGlobalsMap["options.searchpath.shader"] = "string";
    prmanGlobalsMap["options.searchpath.textures"] = "string";
    prmanGlobalsMap["options.searchpath.display"] = "string";
    prmanGlobalsMap["options.searchpath.archive"] = "string";
    prmanGlobalsMap["options.searchpath.procedural"] = "string";
    prmanGlobalsMap["options.searchpath.resource"] = "string";
    prmanGlobalsMap["options.searchpath.servershader"] = "string";
    prmanGlobalsMap["options.searchpath.servertexture"] = "string";
    prmanGlobalsMap["options.searchpath.serverdisplay"] = "string";
    prmanGlobalsMap["options.searchpath.serverarchive"] = "string";
    prmanGlobalsMap["options.searchpath.serverresource"] = "string";
    prmanGlobalsMap["options.searchpath.dirmap"] = "string";

    prmanGlobalsMap["options.curve.orienttotransform"] = "int";
    prmanGlobalsMap["options.dice.maxhairlength"] = "int";
    prmanGlobalsMap["options.hair.minwidth"] = "float";

    prmanGlobalsMap["hider.hidden.jitter"] = "uniform float";
    prmanGlobalsMap["hider.hidden.mpcache"] = "uniform int";
    prmanGlobalsMap["hider.hidden.samplemotion"] = "uniform int";
    prmanGlobalsMap["hider.hidden.mpmemory"] = "uniform int";

    prmanGlobalsMap["hider.hidden.depthfilter"] = "uniform string";
    prmanGlobalsMap["hider.photon.emit"] = "uniform int";

    prmanGlobalsMap["hider.hidden.sigma"] = "constant int";
    prmanGlobalsMap["hider.hidden.sigmablur"] = "constant float";
    // 'shutteropening' deprecated - kept for backward compatibility
    prmanGlobalsMap["hider.hidden.shutteropening"] = "constant float[2]";
    prmanGlobalsMap["hider.hidden.aperture"] = "constant float[4]";
    prmanGlobalsMap["hider.hidden.maxvpdepth"] = "constant int";
    prmanGlobalsMap["hider.hidden.dofaspect"] = "constant float";
    prmanGlobalsMap["hider.hidden.mattefile"] = "constant string";
    prmanGlobalsMap["hider.hidden.mpcachedir"] = "constant string";
    prmanGlobalsMap["hider.hidden.subpixel"] = "constant int";
    prmanGlobalsMap["hider.hidden.extrememotiondof"] = "constant int";
    prmanGlobalsMap["hider.hidden.pointfalloffpower"] = "constant float";
    // 'occlusionbound' deprecated - kept for backward compatibility
    prmanGlobalsMap["hider.hidden.occlusionbound"] = "constant float";

    prmanGlobalsMap["hider.raytrace.samplemode"] = "string";
    prmanGlobalsMap["hider.raytrace.minsamples"] = "int";
    prmanGlobalsMap["hider.raytrace.aperture"] = "constant float[4]";
    prmanGlobalsMap["hider.raytrace.dofaspect"] = "constant float";
    prmanGlobalsMap["hider.raytrace.samplemotion"] = "uniform int";
    prmanGlobalsMap["hider.raytrace.jitter"] = "uniform int";
}

