// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <PRManProcedural.h>
#include <slo.h>

using namespace PRManProcedural;

#include <pystring/pystring.h>


#include <sstream>
#include <string.h>
#include <map>

#include <FnRenderOutputUtils/FnRenderOutputUtils.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <ErrorReporting.h>

using namespace FnKat;

namespace
{
    struct eqstr
    {
        bool operator()(const char* s1, const char* s2) const
        {
            return strcmp(s1, s2) == 0;
        }
    };
    
    // Setup known attribute types for shader parameters.
    void LoadShaderParameters(const std::string & identifier, const std::string& shaderName, PRManPluginState* sharedState)
    {
        // Make sure shader paths are set
        const char* sloPath = Slo_GetPath();
        if (!sloPath)
        {
            Report_Error("Shader search path not set.", identifier);
            return;
        }
        Report_Debug(std::string("LoadShaderParameters(): ")
                     + "Shader search path: \"" + std::string(sloPath) + "\"",
                     identifier, sharedState);

        // Get the base name of the given shader, stripping the file extension
        // (PRMan takes care of stripping any path parts)
        std::string shaderBasename (shaderName);
        if (pystring::endswith(shaderBasename, ".slo"))
        {
            shaderBasename = pystring::slice(shaderBasename, 0, -4);
        }

        // Set the plug-in to the given shader
        if (Slo_SetShader((char*) shaderBasename.c_str()) != 0)
        {
            Report_Error("Shader \"" + shaderName + "\" not found in shader "
                         "path: \"" + std::string(sloPath) + "\"", identifier);
            return;
        }

        // Get the type name of the given shader
        std::string shaderType = Slo_TypetoStr(Slo_GetType());
        sharedState->setAttributeType(shaderName + ".", shaderType);

        // Iterate over the shader parameters (index is 1-based)
        int numargs = Slo_GetNArgs();
        for (int i = 1; i <= numargs; ++i)
        {
            SLO_VISSYMDEF *arg = Slo_GetArgById(i);
            if (arg)
            {
                std::string argName = shaderName + "." + arg->svd_name;
                std::ostringstream argType;
                
                // I don't know why we don't use the storage value here.
                // Brian Hall would apparently know, but he's on vacation...
                // and this code has no comment describing it at all.
                //argType << Slo_StortoStr(arg->svd_storage) << " ";
                
                argType << Slo_DetailtoStr(arg->svd_detail) << " ";
                argType << Slo_TypetoStr(arg->svd_type);
                if (arg->svd_arraylen > 0)
                {
                    argType  << "[" << arg->svd_arraylen << "]";
                }
                else if (arg->svd_arraylen == -1)
                {
                    //This indicates that it's a dynamically-sized array.
                    //We'll build the remainder of the declaration based on the size
                    //of the data provided with the material attribute.
                    argType  << "[";
                }

                sharedState->setAttributeType(argName, argType.str());
            }
        }

        Slo_EndShader();
    }

    void ProcessShadingNode(RenderOutputUtils::ShadingNodeDescriptionMap & nodeMap,
                            const std::string & name,
                            FnAttribute::GroupBuilder & gb,
                            std::set<std::string> & createdNodeNames,
                            const std::string & terminalName = "")
    {
        bool isTerminal = !terminalName.empty();
        
        if (createdNodeNames.find(name) != createdNodeNames.end())
        {
            //we've already built it
            return;
        }
        

        RenderOutputUtils::ShadingNodeDescription node = nodeMap.getShadingNodeDescriptionByName(
                name);
        
        if (!node.isValid())
        {
            //TODO, error?
            return;
        }
        
        createdNodeNames.insert(name);
        
        std::string paramPrefix;
        if (isTerminal)
        {
            paramPrefix = terminalName + "Params.";
            gb.set(terminalName + "Shader", FnAttribute::StringAttribute(node.getType()));
        }
        else
        {
            paramPrefix = "prmanCoshaders." + name + ".params.";
            gb.set("prmanCoshaders." + name + ".shader", FnAttribute::StringAttribute(
                    node.getType()));
        }
        
        for (int i = 0, e = node.getNumberOfParameterNames(); i < e; ++i)
        {
            std::string paramName = node.getParameterName(i);
            gb.set(paramPrefix + paramName, node.getParameter(paramName));
        }
        
        for (int i = 0, e = node.getNumberOfConnectionNames(); i < e; ++i)
        {
            std::string connectionName = node.getConnectionName(i);
            
            RenderOutputUtils::ShadingNodeConnectionDescription connection =
                    node.getConnection(connectionName);
            
            if (connection.isValid())
            {
                std::string connectedNodeName =
                        connection.getConnectedNodeName();

                if (!connectedNodeName.empty())
                {
                    gb.set(paramPrefix + connectionName,
                            FnAttribute::StringAttribute(connectedNodeName));
                    ProcessShadingNode(nodeMap, connectedNodeName,
                            gb, createdNodeNames);
                }
            }
        }
    }
    
    std::string SetParamDeclaration( std::string declarationPrefix,
                                     std::string paramName,
                                     DataAttribute paramAttr,
                                     const std::string & identifier, PRManPluginState* sharedState)
    {
        // Replace "shader" in the declaration prefix with "string", because a
        // coshader has to be passed as a string
        size_t replacePos = declarationPrefix.find("shader");
        if (replacePos != std::string::npos)
        {
            declarationPrefix = declarationPrefix.replace(replacePos, 6,
                                                          "string");
        }

        // Check if the declaration prefix is for a dynamic array of values
        if (pystring::endswith(declarationPrefix, "["))
        {
            DataAttribute dataAttr = paramAttr;

            if (dataAttr.isValid())
            {
                // Set the element size to use depending on the given
                // declaration prefix
                size_t elementSize = 1;
                if (declarationPrefix == "color["
                    || declarationPrefix == "uniform color["
                    || declarationPrefix == "point["
                    || declarationPrefix == "uniform point["
                    || declarationPrefix == "vector["
                    || declarationPrefix == "uniform vector["
                    || declarationPrefix == "normal["
                    || declarationPrefix == "uniform normal[")
                {
                    elementSize = 3;
                }
                else if (declarationPrefix == "hpoint["
                         || declarationPrefix == "uniform hpoint[")
                {
                    elementSize = 4;
                }
                else if (declarationPrefix == "matrix["
                         || declarationPrefix == "uniform matrix[")
                {
                    elementSize = 16;
                }

                size_t numberOfValues = dataAttr.getNumberOfValues();
                if (numberOfValues % elementSize != 0)
                {
                    Report_Debug(std::string("SetParamDeclaration(): ")
                                 + "Skipped parameter \"" + paramName + "\" "
                                 + "because the provided attribute's number "
                                 + "of values is not a multiple of the "
                                 + "parameter type's element size.",
                                 identifier, sharedState);
                    return "";
                }

                // Build the resulting string from the (modified) declaration
                // prefix with the number of elements in square brackets
                std::ostringstream result;
                result << declarationPrefix << (numberOfValues / elementSize)
                       << "]";
                return result.str();
            }
            else
            {
                Report_Debug(std::string("SetParamDeclaration(): ")
                             + "Skipped \"" + paramName + "\" because the "
                             "provided attribute is not valid.", identifier, sharedState);
            }
        }

        return declarationPrefix;
    }
}

namespace PRManProcedural
{
    FnAttribute::GroupAttribute ConvertNetworkMaterialToCoshaders( GroupAttribute materialAttr )
    {
        bool isNetwork = false;
        
        FnAttribute::StringAttribute styleAttr = materialAttr.getChildByName("style");
        if (styleAttr.isValid() && styleAttr.getValue() == "network")
        {
            isNetwork = true;
        }
        
        if ( !isNetwork )
        {
            return materialAttr;
        }

        RenderOutputUtils::ShadingNodeDescriptionMap nodeMap( materialAttr );
        std::set<std::string> createdNodeNames;
        bool atLeastOneTerminalValid = false;
        
        FnAttribute::GroupBuilder gb;

        FnAttribute::GroupAttribute terminalsAttr = materialAttr.getChildByName( "terminals" );
        
        if ( terminalsAttr.isValid() )
        {
            for ( int i = 0, e = terminalsAttr.getNumberOfChildren(); i < e; ++i )
            {
                std::string terminalName = terminalsAttr.getChildName( i );
                if ( !pystring::startswith( terminalName, "prman" ) )
                {
                    continue;
                }
                
                //special-case for a sub-group within terminals to allow for disconnected
                //nodes (and their upstream connections) to be described with arbitrary
                //terminals
                if ( terminalName == "prmanCoshaders" )
                {
                    FnAttribute::GroupAttribute coshadersAttr = terminalsAttr.getChildByIndex( i );
                    
                    if ( coshadersAttr.isValid() )
                    {
                        for ( int j = 0, e = coshadersAttr.getNumberOfChildren(); j < e; ++j )
                        {
                            FnAttribute::StringAttribute coshaderTerminalAttr = coshadersAttr.getChildByIndex( j );
                            std::string coshaderTerminalName = coshadersAttr.getChildName( i );

                            if ( !coshaderTerminalAttr.isValid() )
                            {
                                continue;
                            }
                            
                            std::string terminalNodeName = coshaderTerminalAttr.getValue();
                            
                            ProcessShadingNode( nodeMap,terminalNodeName, gb, createdNodeNames );

                            if ( createdNodeNames.find( terminalNodeName ) != createdNodeNames.end() )
                            {
                                atLeastOneTerminalValid = true;
                            }
                        }
                    }
                    
                    continue;
                }
                
                FnAttribute::StringAttribute terminalAttr = terminalsAttr.getChildByIndex( i );
                if ( !terminalAttr.isValid() )
                {
                    continue;
                }
                std::string terminalNodeName = terminalAttr.getValue();
                
                try
                {
                    ProcessShadingNode( nodeMap, terminalNodeName,
                                        gb, createdNodeNames, terminalName );
                    
                    if ( createdNodeNames.find( terminalNodeName ) != createdNodeNames.end() )
                    {
                        atLeastOneTerminalValid = true;
                    }
                }
                catch( const std::exception &e )
                {
                    std::ostringstream os;
                    os << "Error occurred processing shading node '"
                       << os << terminalNodeName << "'.\n\t"
                       << "Exception reports '" << e.what() << "'.";
                    Report_Error(os.str());
                }
            }
        }
        
        if ( atLeastOneTerminalValid )
        {
            materialAttr = gb.build();
        }
        
        return materialAttr;
    }
    
    void GetCoshaders( FnAttribute::GroupAttribute materialAttr, CoShadersInfo &coshaders )
    {
        // Loop through and add coshaders
        FnAttribute::GroupAttribute coshadersAttr = materialAttr.getChildByName( "prmanCoshaders" );
        if ( coshadersAttr.isValid() )
        {
            int numChildren = coshadersAttr.getNumberOfChildren();

            for ( int i = 0; i < numChildren; i++ )
            {
                FnAttribute::GroupAttribute attr = coshadersAttr.getChildByIndex( i );
                if ( attr.isValid() )
                {
                    std::string childName = coshadersAttr.getChildName( i );
                    CoShaderEntry entry;
                    entry.shaderName = childName;
                    entry.coshaderAttr = attr;
                    entry.used = false;
                    coshaders.insert( std::pair<std::string, CoShaderEntry>( childName, entry ) );
                }
            }
        }
    }

    void PrepareCoshaders( const std::string & identifier, CoShadersInfo &coshaders,
                           PRManPluginState* sharedState, FnAttribute::GroupAttribute info_attr )
    {
        std::string shaderAttrName = "shader";
        std::string paramsAttrName = "params";

        std::string coshaderName = "";
        CoShaderEntry *entry;
        FnAttribute::GroupAttribute coshaderAttr;

        for( CoShadersInfo::iterator it = coshaders.begin(); it != coshaders.end(); ++it )
        {
            coshaderName = (*it).first;
            entry = &(*it).second;
            coshaderAttr = entry->coshaderAttr;

            FnAttribute::StringAttribute shaderAttr = coshaderAttr.getChildByName( shaderAttrName );
            if ( !shaderAttr.isValid() ) 
            {
              // Move on to the next co-shader if the attribute is invalid. Bug Number: 29874, TP Number: 58398
              entry->shaderName = "";
              entry->used = true;
              continue;               
            }

            std::string shaderName = shaderAttr.getValue();
            if ( !shaderName.size() ) 
            {
              // Move on to the next co-shader if the attribute is invalid, see above.
              entry->shaderName = "";
              entry->used = true;
              continue;
            }

            entry->shaderName = shaderName;

            std::string shaderType = sharedState->getAttributeType( shaderName + ".");
            if ( shaderType.empty() )
            {
                LoadShaderParameters( identifier, shaderName, sharedState );
                shaderType = sharedState->getAttributeType( shaderName + ".");
            }

            std::string coshaderInstName = coshaderName;

            if (info_attr.isValid())
            {
                FnAttribute::StringAttribute materialAssignAttr = info_attr.getChildByName( "materialAssign" );
                FnAttribute::GroupAttribute overrideAttr = info_attr.getChildByName( "materialOverride" );

                if (materialAssignAttr.isValid() && !overrideAttr.isValid())
                {
                    coshaderInstName = materialAssignAttr.getValue();
                    coshaderInstName += ".";
                    coshaderInstName += coshaderName;
                }
            }

            // Start building a list of attributes for the current coshader
            // entry by adding the mandatory __instanceid string
            AttrList attrList;
            attrList.push_back(AttrListEntry(
                "string __instanceid", "__instanceid",
                FnAttribute::StringAttribute(std::string("shader:") + coshaderInstName),
                false));

            // Check if local attributes for custom shader parameter values are available
            FnAttribute::GroupAttribute shaderParamsAttr = coshaderAttr.getChildByName( paramsAttrName );
            if ( shaderParamsAttr.isValid() )
            {
                int numParams = shaderParamsAttr.getNumberOfChildren();

                for ( int i = 0; i < numParams; i++ )
                {
                    Attribute childAttr = shaderParamsAttr.getChildByIndex( i );
                    const std::string paramName = shaderParamsAttr.getChildName( i );

                    std::string declarationStr;
                    {
                        std::string lookupName = shaderName + "." + paramName;
                        declarationStr = sharedState->getAttributeType( lookupName );
                    }

                    if( declarationStr == "shader" ||
                        declarationStr == "uniform shader" ||
                        declarationStr == "shader[" ||
                        declarationStr == "uniform shader[" )
                    {
                        FnAttribute::StringAttribute stringAttr = childAttr;
                        if ( stringAttr.isValid() )
                        {
                            std::string paramValue = stringAttr.getNearestSample(0)[0];
                            entry->dependencies.push_back( paramValue );
                        }
                    }

                    // if it is an array, then prepare the declaration string accordingly
                    declarationStr = SetParamDeclaration( declarationStr, paramName, childAttr, identifier, sharedState );
                    if( declarationStr.empty() )
                    {
                        continue;
                    }

                    std::string entryName = declarationStr + " " + paramName;
                    attrList.push_back( AttrListEntry( entryName, paramName, childAttr, false ) );
                }
            }

            entry->attrList = attrList;
        }
    }

    void WriteCoshaders( CoShadersInfo &coshaders, int unusedCoshaders = 0 )
    {
        std::string coshaderName = "";
        CoShaderEntry *entry;
        int used = 0;
        bool cyclicLink = false;

        for( CoShadersInfo::iterator it = coshaders.begin(); it != coshaders.end(); ++it )
        {
            coshaderName = (*it).first;
            entry = &(*it).second;

            if( !entry->used )
            {
                size_t dependenciesDefined = 0;
                if( entry->dependencies.size() > 0 )
                {
                    for( std::vector<std::string>::iterator depIt = entry->dependencies.begin(); depIt != entry->dependencies.end(); ++depIt )
                    {
                        if( coshaders[(*depIt)].used )
                            dependenciesDefined++;

                        // Check for cyclic links
                        std::vector<std::string> depDeps = coshaders[(*depIt)].dependencies;
                        for( std::vector<std::string>::iterator depDepsIt = depDeps.begin(); depDepsIt != depDeps.end(); ++depDepsIt )
                        {
                            if( (*depDepsIt) == coshaderName )
                            {
                                std::ostringstream os;
                                os << "Cyclic dependency link: " << coshaderName
                                   << "->" << (*depIt);
                                Report_Warning(os.str());
                                cyclicLink = true;
                                break;
                            }
                        }
                    }
                }

                if( dependenciesDefined == entry->dependencies.size() )
                {
                    AttrList_Converter converter( entry->attrList );

                    //in the case of full path shaders, support with/without .slo
                    std::string shaderName = entry->shaderName;
                    if ( pystring::endswith( shaderName, ".slo" ) )
                    {
                        shaderName = pystring::slice( shaderName, 0, -4);
                    }

                    RiShaderV( const_cast<char*>(shaderName.c_str()), const_cast<char*>( coshaderName.c_str() ),
                               converter.getSize(0),converter.getDeclarationTokens(0), converter.getParameters(0) );

                    entry->used = true;
                    used++;
                }
            }
            else
            {
                used++;
            }
        }

        int unused = coshaders.size() - used;
        if( unused > 0 )
        {
            if( unused == unusedCoshaders )
                Report_Warning("Could not continue writing co-shaders into the RIB stream: Loop without resolving co-shaders.");
            else if( cyclicLink )
                Report_Warning("Could not continue writing co-shaders into the RIB stream: Co-shaders have cyclic links.");
            else
                WriteCoshaders( coshaders, unused );
        }
    }

    /* Write info for single shader (called by WriteRIObject_Material) */
    void WriteShader( FnScenegraphIterator sgIterator,
                      FnAttribute::GroupAttribute material,
                      const std::string &type,
                      const std::string &attributeType,
                      FnAttribute::GroupAttribute info_attr,
                      PRManPluginState* sharedState,
                      std::string location)
    {
        FnAttribute::StringAttribute objectZFlipAttr = sgIterator.getAttribute("prmanStatements.light.z_flip", true);
        FnAttribute::StringAttribute globalZFlipAttr = sgIterator.getRoot().getAttribute("prmanGlobalStatements.light.z_flip");
        
        bool flipZ = true;
        
        if ( objectZFlipAttr.isValid() )
        {
            if (objectZFlipAttr.getValue() == "No")
            {
                flipZ = false;
            }
        }
        else if ( globalZFlipAttr.isValid() )
        {
            if (globalZFlipAttr.getValue() == "No")
            {
                flipZ = false;
            }
        }
        
        WriteShader(sgIterator.getFullName(), flipZ, material, type, attributeType, info_attr, sharedState, location);
    }
    
    /**
     * Adds a shader child attribute to an AttrList object.
     */
    void AddDeclarationToAttributeList(const std::string& shaderName, const std::string& childName,
                        const Attribute& childAttr, const std::string & identifier,
                        PRManPluginState* sharedState, AttrList& attrList)
    {
        std::string declarationStr;
        std::string lookupName = shaderName;
        lookupName += ".";
        lookupName += childName;
        declarationStr = sharedState->getAttributeType(lookupName);

        if (declarationStr.empty())
        {
            Report_Debug(std::string("WriteShader(): ") + "Skipped \""
                         + childName + "\" because it's not declared "
                         "for shader \"" + shaderName + "\".",
                         identifier, sharedState);
            return;
        }

        declarationStr = SetParamDeclaration(declarationStr, childName, childAttr, identifier, sharedState);
        if(declarationStr.empty())
        {
            return;
        }

        attrList.push_back( AttrListEntry( declarationStr + " " + childName, childName, childAttr, false ) );
    }

    void WriteShader( const std::string & identifier,
                      bool flipZ,
                      FnAttribute::GroupAttribute material,
                      const std::string &type,
                      const std::string &attributeType,
                      FnAttribute::GroupAttribute info_attr, PRManPluginState* sharedState,
                      std::string location)
    {
        Report_Debug(std::string("WriteShader() called, type = \"") + type
                     + "\"", identifier, sharedState);

        std::string shaderAttrName = attributeType + "Shader";
        std::string paramsAttrName = attributeType + "Params";

        FnAttribute::StringAttribute shaderAttr = material.getChildByName(shaderAttrName);
        if (!shaderAttr.isValid()) return;

        std::string shaderName = shaderAttr.getValue();
        if (!shaderName.size()) return;

        // The first key we add when looking up shader arg types
        // is 'shaderName.' with the type of shader
        std::string shaderType = sharedState->getAttributeType(shaderName + ".");

        if( location == "" )
            location = identifier;

        // If we can't find shaderName.,
        // then we've not loaded any of the arguments for this shader.
        // So we should load them.
        if (shaderType.empty())
        {
            LoadShaderParameters(identifier, shaderName, sharedState);
            shaderType = sharedState->getAttributeType(shaderName + ".");
            if (shaderType.empty())
            {
                // Shader could not be found in RMAN_SHADERPATH, error already
                // reported in LoadShaderParameters()
                Report_Debug(std::string("WriteShader(): ")
                             + "Skipped shader \"" + shaderName + "\" as its "
                             "parameters could not be loaded.", identifier, sharedState);
                return;
            }
        }

        if (shaderType != type && shaderType != "shader" &&  !(shaderType=="volume" && (type=="atmosphere" || type=="interior" || type=="exterior")) )
        {
            Report_Error("Shader \"" + shaderName + "\" is type \""
                         + shaderType + "\" and we're expecting type \""
                         + type + "\".", identifier);
            return;
        }

        AttrList attrList;
        FnAttribute::GroupAttribute shaderParamsAttr = material.getChildByName(paramsAttrName);
        if (shaderParamsAttr.isValid())
        {
            int numChildren = shaderParamsAttr.getNumberOfChildren();
            std::map<std::string, std::vector<StringAttribute> > dynamicArrayAttributes;

            for (int i = 0; i < numChildren; i++)
            {
                Attribute childAttr = shaderParamsAttr.getChildByIndex(i);
                const std::string childName = shaderParamsAttr.getChildName(i);

                // In the case of ShadingNodeArrayConnectors when using co-shaders childname will
                // include a colon followed by the array index. eg. dynamicShaderArray:0
                // To handle this we search for a colon and store the element in a map to be
                // later combined into a single string array attribute.
                size_t pos = childName.rfind(":");         // position of ":" in str
                if(pos != std::string::npos)
                {
                    // This is a array element
                    std::string arrayParameterName = childName.substr(0, pos);
                    StringAttribute stringAttr = (StringAttribute)childAttr;
                    if(stringAttr.isValid())
                    {
                        // Currently only combine string arrays, as I don't believe other attr
                        // types are required
                        dynamicArrayAttributes[arrayParameterName].push_back(stringAttr);
                        continue;
                    }
                }

                AddDeclarationToAttributeList(shaderName, childName, childAttr, identifier, sharedState, attrList);
            }

            // Find any dynamic array elements and add them as a single attribute
            std::map<std::string, std::vector<StringAttribute> >::iterator it = dynamicArrayAttributes.begin();
            for(; it != dynamicArrayAttributes.end(); ++it) {
                std::string parameterName = it->first;
                std::vector<StringAttribute> elementAttributes = it->second;

                size_t size = elementAttributes.size();
                std::string* entries = new std::string[size];
                for(int i=0; i < size; ++i)
                {
                    entries[i] = elementAttributes[i].getValue("",false);
                }
                // Create a StringAttribute with multiple values
                StringAttribute stringAttr((const char**)entries, (int)size, 1);
                delete[] entries;
                AddDeclarationToAttributeList(shaderName, parameterName, stringAttr, identifier, sharedState, attrList);
            }
        }

        //in the case of full path shaders, support with/without .slo
        if ( pystring::endswith( shaderName, ".slo" ) )
        {
            shaderName = pystring::slice( shaderName, 0, -4 );
        }
        
        // Tack on the magical "__handleid" attribute to make the handle id sensible.
        if ( type == "light" )
        {
            attrList.push_back( AttrListEntry( "__handleid", "__handleid", FnAttribute::StringAttribute( location ), false ) );
        }
        else
        {
            std::string instanceName;

            if (info_attr.isValid())
            {
                FnAttribute::StringAttribute materialAssignAttr = info_attr.getChildByName( "materialAssign" );
                FnAttribute::GroupAttribute overrideAttr = info_attr.getChildByName( "materialOverride" );

                if (materialAssignAttr.isValid() && !overrideAttr.isValid())
                    instanceName = type + std::string( ":" ) + materialAssignAttr.getValue();
            }

            if (instanceName == "")
            {
                instanceName = type + std::string( ":" ) + location;
            }

            attrList.push_back( AttrListEntry( "string __instanceid", "__instanceid", FnAttribute::StringAttribute( instanceName ), false ) );
        }
        
        // Convert the attribute list to data structures expected by RenderMan
        AttrList_Converter converter( attrList );
        RtToken name = const_cast<char*>(shaderName.c_str());
        RtInt n = converter.getSize(0);
        RtToken* declarationTokens = converter.getDeclarationTokens(0);
        RtPointer* parameters = converter.getParameters(0);

        if ( type == "surface" )
        {
            Report_Debug("RiSurfaceV", name, n, declarationTokens, parameters, identifier, sharedState);
            RiSurfaceV(name, n, declarationTokens, parameters);
        }
        else if ( type == "displacement" )
        {
            Report_Debug("RiDisplacementV", name, n, declarationTokens, parameters, identifier, sharedState);
            RiDisplacementV(name, n, declarationTokens, parameters);
        }
        else if ( type == "atmosphere" )
        {
            Report_Debug("RiAtmosphereV", name, n, declarationTokens, parameters, identifier, sharedState);
            RiAtmosphereV(name, n, declarationTokens, parameters);
        }
        else if ( type == "interior" )
        {
            Report_Debug("RiInteriorV", name, n, declarationTokens, parameters, identifier, sharedState);
            RiInteriorV(name, n, declarationTokens, parameters);
        }
        else if ( type == "exterior" )
        {
            Report_Debug("RiExteriorV", name, n, declarationTokens, parameters, identifier, sharedState);
            RiExteriorV(name, n, declarationTokens, parameters);
        }
        else if ( type == "light" )
        {
            if (flipZ)
            {
                Report_Debug("RiScale(1.0f, 1.0f, -1.0f)", identifier, sharedState);
                RiScale(1.0f, 1.0f, -1.0f);
            }

            Report_Debug("RiLightSourceV", name, n, declarationTokens, parameters, identifier, sharedState);
            RtLightHandle lightHandle = RiLightSourceV(name, n, declarationTokens, parameters);
            sharedState->setLightHandleForLocation(location, lightHandle);
        }
    }

    RtLightHandle GetLightHandle( std::string location, PRManPluginState* sharedState )
    {
        return sharedState->getLightHandleForLocation(location);
    }
    
    void WriteRI_Object_Material(FnScenegraphIterator sgIterator, PRManPluginState* sharedState)
    {
    
        Report_Debug("WriteRI_Object_Material() called", sharedState, sgIterator);
    
        // Write out current material settings
        FnAttribute::GroupAttribute material_attr = sgIterator.getAttribute( "material" );
        if (!material_attr.isValid())
        return;

        FnAttribute::GroupAttribute info_attr = sgIterator.getAttribute( "info" );

        //expand the network into attributes
        material_attr = ConvertNetworkMaterialToCoshaders( material_attr );

        CoShadersInfo coshaders;
        GetCoshaders( material_attr, coshaders );
        PrepareCoshaders( sgIterator.getFullName(), coshaders, sharedState, info_attr );
        WriteCoshaders( coshaders );
    
        WriteShader(sgIterator, material_attr, "surface", "prmanSurface", info_attr, sharedState);
        WriteShader(sgIterator, material_attr, "displacement", "prmanDisplacement", info_attr, sharedState);
        WriteShader(sgIterator, material_attr, "atmosphere", "prmanAtmosphere", info_attr, sharedState);
        WriteShader(sgIterator, material_attr, "interior", "prmanInterior", info_attr, sharedState);
        WriteShader(sgIterator, material_attr, "exterior", "prmanExterior", info_attr, sharedState);
        WriteShader(sgIterator, material_attr, "light", "prmanLight", info_attr, sharedState);
    }
    
    void WriteRI_Object_Material(const std::string & identifier, FnAttribute::GroupAttribute material_attr, bool flipZ, FnAttribute::GroupAttribute info_attr, PRManPluginState*  sharedState)
    {

        Report_Debug("WriteRI_Object_Material() called", identifier, sharedState);

        if (!material_attr.isValid())
        return;

        //expand the network into attributes
        material_attr = ConvertNetworkMaterialToCoshaders( material_attr );

        CoShadersInfo coshaders;
        GetCoshaders( material_attr, coshaders );
        PrepareCoshaders( identifier, coshaders, sharedState, info_attr );
        WriteCoshaders( coshaders );

        WriteShader(identifier, flipZ, material_attr, "surface", "prmanSurface", info_attr, sharedState);
        WriteShader(identifier, flipZ, material_attr, "displacement", "prmanDisplacement", info_attr, sharedState);
        WriteShader(identifier, flipZ, material_attr, "atmosphere", "prmanAtmosphere", info_attr, sharedState);
        WriteShader(identifier, flipZ, material_attr, "interior", "prmanInterior", info_attr, sharedState);
        WriteShader(identifier, flipZ, material_attr, "exterior", "prmanExterior", info_attr, sharedState);
        WriteShader(identifier, flipZ, material_attr, "light", "prmanLight", info_attr, sharedState);
    }
    
    void WriteRI_Object_IlluminationList(FnScenegraphIterator sgIterator, PRManPluginState* sharedState)
    {
        Report_Debug("WriteRI_Object_IlluminationList() called", sharedState, sgIterator);

        // Write out current illumination settings
        FnAttribute::GroupAttribute localLightList = sgIterator.getAttribute("lightList");
        if (!localLightList.isValid()) return;

        FnAttribute::GroupAttribute globalLightList = sgIterator.getAttribute("lightList", true);
        if (!globalLightList.isValid())
        {
            Report_Error("Global attibute 'lightList' not present", sgIterator);
            return;
        }

        // Walk through the local light list, querying local enable flags
        int numChildren = localLightList.getNumberOfChildren();
        
        for (int i = 0; i < numChildren; i++)
        {
            const std::string childName = localLightList.getChildName(i);

            // Get the locally defined lightList Attr
            FnAttribute::GroupAttribute lightGroup = localLightList.getChildByIndex(i);
            if (!lightGroup.isValid())
            {
                Report_Error("Local attribute 'lightList." + childName +"' not present", sgIterator);
                continue;
            }

            // Get the locally defined 'enable' attr
            FnAttribute::IntAttribute enableattr = lightGroup.getChildByName("enable");
            if (!enableattr.isValid())
            {
                // with the rise of non-illumination linking present in prman output,
                //this is a valid case which should be silently ignored.
                continue;
            }

            // Get the globally defined lightList Attr
            FnAttribute::GroupAttribute globalLightGroup = globalLightList.getChildByName(childName);
            if (!globalLightGroup.isValid())
            {
                Report_Error("Global attribute 'lightList." + childName +"' not present", sgIterator);
                continue;
            }

            // Get the globally defined lightList.<name>.path attribute
            FnAttribute::StringAttribute pathattr = globalLightGroup.getChildByName("path");
            if (!pathattr.isValid())
            {
                Report_Error("Global attribute 'lightList." + childName + ".path' not present", sgIterator);
                continue;
            }

            // Find light in Katana Illumination Map
            RtLightHandle lightHandle =
                sharedState->getLightHandleForLocation(pathattr.getValue());
            if(lightHandle == RI_NULL)
            {
                // If it's not in the illumination map, then it's not really a light.
                // So we don't need to do anything more here.
                // It's not an error though, so don't bug the user.
                continue;
            }

            // Make the prman call.
            RtBoolean onoff = enableattr.getValue();
            RiIlluminate(lightHandle, onoff);
        }
    }
}

