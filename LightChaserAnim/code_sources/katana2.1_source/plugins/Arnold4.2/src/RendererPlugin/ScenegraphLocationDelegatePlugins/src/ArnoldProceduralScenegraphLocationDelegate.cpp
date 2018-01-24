// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <buildCommons.h>
#include <ArnoldProceduralScenegraphLocationDelegate.h>
#include <vector>
#include <pystring/pystring.h>

#include <FnRenderOutputUtils/FnRenderOutputUtils.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAsset/FnDefaultAssetPlugin.h>
#include <objectSettings.h>
#include <buildCamera.h>
#include <ArnoldPluginState.h>

using namespace FnKat;

void setProceduralTextureUserData(FnScenegraphIterator sgIterator, AtNode* proceduralNode)
{
    if (!sgIterator.isValid() || !proceduralNode) return;
    
    GroupAttribute texturesAttr = sgIterator.getAttribute("textures", true);
    if (!texturesAttr.isValid()) return;
    
    //no actual textures? no problem. doesn't warrant a warning.
    if (!texturesAttr.getNumberOfChildren()) return;
    
    for (int i=0; i<texturesAttr.getNumberOfChildren(); ++i)
    {
        GroupAttribute groupAttr = texturesAttr.getChildByIndex(i);
        
        const std::string mapType = texturesAttr.getChildName(i);
        
        //means that we have per-face map description.
        //need to include idx (int array), paths (string array) attributes
        if (groupAttr.isValid())
        {
            FnAttribute::IntAttribute indicesAttr = groupAttr.getChildByName("indicies");
            if (!indicesAttr.isValid()) indicesAttr = groupAttr.getChildByName("indices");
            FnAttribute::StringAttribute mapsAttr = groupAttr.getChildByName("maps");

            //we're missing necessary information here, skip this one
            if (!indicesAttr.isValid() || !mapsAttr.isValid()) continue;
            
            //add the paths attr
            StringConstVector mapData = mapsAttr.getNearestSample(0);
            
            std::string pathsAttrName = mapType+"_paths";
            
            AiNodeDeclare(proceduralNode,
                    const_cast<char *>(pathsAttrName.c_str()),
                            "constant ARRAY STRING");
            
            AiNodeSetArray(proceduralNode, pathsAttrName.c_str(),
                    AiArrayConvert(mapData.size(), 1, AI_TYPE_STRING,
                            (void*)(&(mapData[0]))));
            
            //add the idx attr
            IntConstVector indexData = indicesAttr.getNearestSample(0);
            
            std::string idxAttrName = mapType+"_idx";
            
            AiNodeDeclare(proceduralNode,
                    const_cast<char *>(idxAttrName.c_str()),
                            "constant ARRAY INT");
            
            AiNodeSetArray(proceduralNode, idxAttrName.c_str(),
                    AiArrayConvert(indexData.size(), 1, AI_TYPE_INT,
                            (void*)(&(indexData[0]))));
        }
        //means that we have a single map, only include the paths (string) attribute
        else
        {
            //confirm that it's a string, just add the paths as an array
            FnAttribute::StringAttribute stringAttr = texturesAttr.getChildByIndex(i);
            if (!stringAttr.isValid()) continue;
            
            std::string pathsAttrName = mapType+"_paths";
            
            AiNodeDeclare(proceduralNode, const_cast<char *>(pathsAttrName.c_str()),
                     "constant ARRAY STRING");
            
            std::string stringValue = stringAttr.getValue();
            std::vector<const char*> stringVector;
            stringVector.push_back(stringValue.c_str());
            
            AiNodeSetArray(proceduralNode, pathsAttrName.c_str(),
                    AiArrayConvert(stringVector.size(), 1, AI_TYPE_STRING,
                            (void*)(&(stringVector[0]))));
        }
    }
}


void buildProceduralArrayArgument( AtNode* procedural, std::string argumentName, std::string declareString,
                                   unsigned char type, int dataSize, const void* data )
{
    AiNodeDeclare( procedural, const_cast<char *>( argumentName.c_str() ), declareString.c_str() );
    AiNodeSetArray( procedural, argumentName.c_str(), AiArrayConvert( dataSize, 1, type, data ) );
}

void buildProceduralArguments( AtNode* procedural, GroupAttribute argumentsAttr )
{
    int noArguments = argumentsAttr.getNumberOfChildren();

    for( int i = 0; i < noArguments; i++ )
    {
        std::string argumentName = argumentsAttr.getChildName( i );
        Attribute attr = argumentsAttr.getChildByIndex( i );

        GroupAttribute groupAttr = attr;
        FnAttribute::IntAttribute intAttr = attr;
        FnAttribute::FloatAttribute floatAttr = attr;
        FnAttribute::DoubleAttribute doubleAttr = attr;
        FnAttribute::StringAttribute stringAttr = attr;

        if( groupAttr.isValid() )
        {
            buildProceduralArguments( procedural, groupAttr );
        }
        else if( intAttr.isValid() )
        {
            IntConstVector dataVector = intAttr.getNearestSample( 0 );
            if( dataVector.size() == 1 )
            {
                AiNodeDeclare( procedural, const_cast<char *>( argumentName.c_str() ), "constant INT" );
                AiNodeSetInt( procedural, argumentName.c_str(), dataVector[0] );
            }
            else
            {
                buildProceduralArrayArgument( procedural, argumentName, "constant ARRAY INT", AI_TYPE_INT,
                                              dataVector.size(), (void*) ( &(dataVector[0]) ) );
            }
        }
        else if( floatAttr.isValid() )
        {
            FloatConstVector dataVector = floatAttr.getNearestSample( 0 );
            if( dataVector.size() == 1 )
            {
                AiNodeDeclare( procedural, const_cast<char *>( argumentName.c_str() ), "constant FLOAT" );
                AiNodeSetFlt( procedural, argumentName.c_str(), dataVector[0] );
            }
            else
            {
                buildProceduralArrayArgument( procedural, argumentName, "constant ARRAY FLOAT", AI_TYPE_FLOAT,
                                              dataVector.size(), (void*) ( &(dataVector[0]) ) );
            }
        }
        else if( doubleAttr.isValid() )
        {
            DoubleConstVector dataVector = doubleAttr.getNearestSample( 0 );
            if( dataVector.size() == 1 )
            {
                AiNodeDeclare( procedural, const_cast<char *>( argumentName.c_str() ), "constant FLOAT" );
                AiNodeSetFlt( procedural, argumentName.c_str(), dataVector[0] );
            }
            else
            {
                buildProceduralArrayArgument( procedural, argumentName, "constant ARRAY FLOAT", AI_TYPE_FLOAT,
                                              dataVector.size(), (void*) ( &(dataVector[0]) ) );
            }
        }
        else if( stringAttr.isValid() )
        {
            StringConstVector dataVector = stringAttr.getNearestSample( 0 );
            if( dataVector.size() == 1 )
            {
                AiNodeDeclare( procedural, const_cast<char *>( argumentName.c_str() ), "constant STRING" );
                AiNodeSetStr( procedural, argumentName.c_str(), dataVector[0] );
            }
            else
            {
                buildProceduralArrayArgument( procedural, argumentName, "constant ARRAY STRING", AI_TYPE_STRING,
                                              dataVector.size(), (void*) ( &(dataVector[0]) ) );
            }
        }
        else
        {
            AiMsgWarning( "[kat] Procedural argument '%s' is of unknown type!", argumentName.c_str() );
        }
    }
}

//buildProcedural
AtNode* buildProcedural(FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState)
{
    if (!isVisible(sgIterator)) return 0;
    
    std::string name = sgIterator.getFullName();

    if(AiNodeLookUpByName(name.c_str()))
    {
        AiMsgError("[kat] '%s' is already in the scene!", name.c_str());
        return 0;
    }
    
    std::string attrBaseName = "procedural";
    std::string pathAttrName = "path";
    
    if (sgIterator.getType() == "renderer procedural")
    {
        attrBaseName = "rendererProcedural";
        pathAttrName = "procedural";
    }
    
    GroupAttribute proceduralAttr = sgIterator.getAttribute(attrBaseName);
    if (!proceduralAttr.isValid())
    {
        std::string errorMessage = "[kat] arnold procedural '%s' is missing the ";
        errorMessage += attrBaseName;
        errorMessage += " attribute.";
        
        AiMsgError(errorMessage.c_str(), name.c_str());
        return 0;
    }
    
    FnAttribute::StringAttribute dsoPathAttr = proceduralAttr.getChildByName(pathAttrName);
    if (!dsoPathAttr.isValid())
    {
        std::string errorMessage = "[kat] arnold procedural '%s' is missing the ";
        errorMessage += attrBaseName;
        errorMessage += ".";
        errorMessage += pathAttrName;
        errorMessage += " attribute.";
        
        AiMsgError(errorMessage.c_str(), name.c_str());
        return 0;
    }

    AtNode* procedural = AiNode("procedural");
    if (!procedural) 
    {
        AiMsgError("[kat] unable to create arnold procedural '%s'", name.c_str());
        return 0;
    }  

    if(childNodes){
        childNodes->push_back(procedural);
    }
    AiNodeSetStr(procedural, "name", name.c_str());
    
    std::string dsoPath = FnKat::RenderOutputUtils::expandArchPath( DefaultAssetPlugin::resolveAsset(dsoPathAttr.getValue()) );
    
    AiNodeSetStr( procedural, "dso", dsoPath.c_str() );

    std::vector<double> boundValue;
    
    FnAttribute::IntAttribute useInfiniteBoundsAttr = proceduralAttr.getChildByName("useInfiniteBounds");
    
    if (useInfiniteBoundsAttr.isValid() &&
        useInfiniteBoundsAttr.getValue() == 0)
    {
        FnAttribute::DoubleAttribute boundAttr = sgIterator.getAttribute("bound", true);
        
        // Note. code borrowed from buildGroup.cpp
        if (boundAttr.isValid())
        {
            DoubleConstVector boundConstValue = boundAttr.getNearestSample(0);
            if (boundConstValue.size() != 6)
            {
                AiMsgError("[kat] FATAL: 'bound' attribute at '%s' is bogus.", sgIterator.getFullName().c_str());
                exit(-1);
            }
            
            // Transform the boundAttr into global space.
            std::map<float, Imath::M44d> xformSamples;
            getTransformSamples(&xformSamples, sgIterator, sharedState);
            Imath::M44d xform = xformSamples[0];
            
            std::vector<double> xformBoundValue;
            for (int ix = 0; ix <= 1; ++ix) for (int iy = 2; iy <= 3; ++iy) for (int iz = 4; iz <= 5; ++iz)
            {
                Imath::V3d p(boundConstValue[ix], boundConstValue[iy], boundConstValue[iz]);
                //p *= xform;
                if (xformBoundValue.empty())
                {
                    xformBoundValue.resize(6);
                    xformBoundValue[0] = p.x; xformBoundValue[1] = p.x;
                    xformBoundValue[2] = p.y; xformBoundValue[3] = p.y;
                    xformBoundValue[4] = p.z; xformBoundValue[5] = p.z;
                }
                else
                {
                    xformBoundValue[0] = std::min(p.x, xformBoundValue[0]);
                    xformBoundValue[1] = std::max(p.x, xformBoundValue[1]);
                    xformBoundValue[2] = std::min(p.y, xformBoundValue[2]);
                    xformBoundValue[3] = std::max(p.y, xformBoundValue[3]);
                    xformBoundValue[4] = std::min(p.z, xformBoundValue[4]);
                    xformBoundValue[5] = std::max(p.z, xformBoundValue[5]);
                }
            }
            boundValue = xformBoundValue;
        }
    }
    
    if (boundValue.empty())
    {
        boundValue.resize(6);
        boundValue[0] = -AI_BIG;
        boundValue[1] = AI_BIG;
        boundValue[2] = -AI_BIG;
        boundValue[3] = AI_BIG;
        boundValue[4] = -AI_BIG;
        boundValue[5] = AI_BIG;
        
        AiNodeSetBool(procedural, "load_at_init", true);
    }
    
    AiNodeSetPnt(procedural, "min", boundValue[0], boundValue[2], boundValue[4]);
    AiNodeSetPnt(procedural, "max", boundValue[1], boundValue[3], boundValue[5]);

    int frameTime = (int) GetFrameTime(sgIterator, sharedState);
    
    RenderOutputUtils::ProceduralOutputContextInfo contextInfo;
    contextInfo._frameNumber = frameTime;
    contextInfo._shutterOpen = sharedState->getShutterOpen();
    contextInfo._shutterClose = sharedState->getShutterClose();
    
    GetGlobalNormalizedCropWindow(
            contextInfo._cropWindowXMin,
            contextInfo._cropWindowXMax,
            contextInfo._cropWindowYMin,
            contextInfo._cropWindowYMax, sharedState);
    
    contextInfo._xres = sharedState->getXResolution();
    contextInfo._yres = sharedState->getYResolution();
    
    {
        GroupBuilder gb;
        gb.update(proceduralAttr);
        gb.set("systemArgs.flipV", FnAttribute::IntAttribute(1));
        proceduralAttr = gb.build();
    }
    
    
    std::string outputStyle = "classic";
    FnAttribute::StringAttribute outputStyleAttr = sgIterator.getAttribute(attrBaseName+".args.__outputStyle");
    if (outputStyleAttr.isValid())
    {
        outputStyle = outputStyleAttr.getValue();
    }
    
    RenderOutputUtils::ProceduralArgsType argType;
    if ( outputStyle == "classic" )
    {
        argType = RenderOutputUtils::kProceduralArgsType_Classic;
    }
    else if ( outputStyle == "scenegraphAttr" || outputStyle == "typedArguments" )
    {
        argType = RenderOutputUtils::kProceduralArgsType_ScenegraphAttr;
    }
    else
    {
        AiMsgError("[kat] arnold procedural '%s' has unknown output style: %s.",
                name.c_str(), outputStyle.c_str());
        return 0;
    }

    std::string dataString = 
        RenderOutputUtils::buildProceduralArgsString(sgIterator,
                                                     argType, 
                                                     attrBaseName, 
                                                     contextInfo);

    if( outputStyle == "classic" || outputStyle == "scenegraphAttr" )
    {
        //clean up args string
        dataString = pystring::replace(dataString, "\"", "'");
        dataString = pystring::replace(dataString, "\r", "");
        dataString = pystring::replace(dataString, "\n", "");
        AiNodeSetStr(procedural, "data", dataString.c_str() );
    }
    else if( outputStyle == "typedArguments" )
    {
        GroupAttribute argumentsAttr = Foundry::Katana::Attribute::parseXML( dataString.c_str() );
        buildProceduralArguments( procedural, argumentsAttr );
    }
    
    setProceduralTextureUserData(sgIterator, procedural);
    
    
    //filter geometry arbitrary for only primitive attrs, then send bogus
    //geometry attribute
    GroupAttribute arbitraryAttr = sgIterator.getAttribute("geometry.arbitrary", true);
    if (arbitraryAttr.isValid())
    {
        GroupBuilder gb;

        for (int i=0; i<arbitraryAttr.getNumberOfChildren(); ++i)
        {
            std::string name = arbitraryAttr.getChildName(i);
            GroupAttribute attr = arbitraryAttr.getChildByIndex(i);
            
            if (!attr.isValid())
            {
                continue;
            }
            
            FnAttribute::StringAttribute scopeAttr = attr.getChildByName("scope");
            
            if (scopeAttr.getValue("", false) == kFnKatArbAttrScopePrimitive)
            {
                gb.set(name, attr);
            }
        }

        GroupAttribute newAttr = gb.build();

        if (newAttr.getNumberOfChildren()!=0)
        {
            GroupBuilder gb2;
            writeArbitrary(procedural, sgIterator.getFullName(), sgIterator.getType(),
                    gb2.build(), newAttr);
        }
    }
    
    return procedural;
}

void* ArnoldProceduralScenegraphLocationDelegate::process(FnScenegraphIterator sgIterator, void* optionalInput)
{
    ArnoldSceneGraphLocationDelegateInput* state = reinterpret_cast<ArnoldSceneGraphLocationDelegateInput*>(optionalInput);
    void* returnValue = buildProcedural(sgIterator, state->childNodes, state->sharedState);
    if(returnValue){
        applyObjectSettings(sgIterator, reinterpret_cast<AtNode*>(returnValue), state->childNodes, state->sharedState);
    }
    return returnValue;
}

ArnoldProceduralScenegraphLocationDelegate::~ArnoldProceduralScenegraphLocationDelegate()
{

}

void ArnoldProceduralScenegraphLocationDelegate::flush()
{

}

ArnoldProceduralScenegraphLocationDelegate::ArnoldProceduralScenegraphLocationDelegate()
{

}

ArnoldProceduralScenegraphLocationDelegate* ArnoldProceduralScenegraphLocationDelegate::create()
{
    return new ArnoldProceduralScenegraphLocationDelegate();
}

std::string ArnoldProceduralScenegraphLocationDelegate::getSupportedRenderer() const
{
    return std::string("arnold");
}

void ArnoldProceduralScenegraphLocationDelegate::fillSupportedLocationList(std::vector<std::string>& supportedLocationList) const
{
    supportedLocationList.push_back(std::string("renderer procedural"));
    supportedLocationList.push_back(std::string("arnold procedural"));
}
