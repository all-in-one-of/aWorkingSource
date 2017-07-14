// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include "arnold_render.h"
#include <pystring/pystring.h>
#include <boost/thread.hpp>

#include <FnRenderOutputUtils/FnRenderOutputUtils.h>
#include <FnAttribute/FnDataBuilder.h>
#include <FnAsset/FnDefaultAssetPlugin.h>

#include <dlfcn.h>

#include <buildCommons.h>

using namespace FnKat;

namespace
{


    void OpenOSLStub(ArnoldPluginState* sharedState)
    {
        boost::recursive_mutex::scoped_lock lock = sharedState->getScopedLock();

        if (sharedState->oslState.oslStubChecked) return;
        sharedState->oslState.oslStubChecked = true;

        std::string dsoPath;
        const char *var = ::getenv("ARNOLDTREE");
        if (var)
        {
            dsoPath = var;
        }

        dsoPath += "/tools/liboslstub.so";
        //std::cerr << dsoPath << std::endl;

        sharedState->oslState.oslStubSo = dlopen(dsoPath.c_str(), RTLD_NOW);

        if (!sharedState->oslState.oslStubSo)
        {
            AiMsgWarning("[kat] couldn't load liboslstub.so in arnold release."
                         " Using internal katana version -- which might not be compatible");

            dsoPath = "";
            const char *var = ::getenv("KATANA_ROOT");
            if (var)
            {
                dsoPath = var;
            }
            dsoPath += "/EXTERNAL_PLUGINS/DSOS/liboslstub.so";
            sharedState->oslState.oslStubSo = dlopen(dsoPath.c_str(), RTLD_NOW);
        }



        if (sharedState->oslState.oslStubSo)
        {
            sharedState->oslState.oslShaderExists = (OSLShaderExistsFnc_t)
                    dlsym(sharedState->oslState.oslStubSo, "OSLShaderExists");

            sharedState->oslState.getOslTypeStringForParameter = (GetOSLTypeStringForParameterFnc_t)
                    dlsym(sharedState->oslState.oslStubSo, "GetOSLTypeStringForParameter");
        }
        else
        {
            AiMsgError("[kat] Couldn't load liboslstub.so. This version of "
                         "arnold might not be OSL compatible");
        }
    }

    bool OSLShaderExists(const std::string & name,
            const std::string & path, ArnoldPluginState* sharedState)
    {
        boost::recursive_mutex::scoped_lock lock = sharedState->getScopedLock();

        OpenOSLStub(sharedState);
        if (sharedState->oslState.oslShaderExists)
        {
            return (*sharedState->oslState.oslShaderExists)(name, path);
        }

        return false;
    }

    std::string GetOSLTypeStringForParameter(const std::string & name,
            const std::string & param, const std::string & path, ArnoldPluginState* sharedState)
    {
        boost::recursive_mutex::scoped_lock lock = sharedState->getScopedLock();

        OpenOSLStub(sharedState);
        if (sharedState->oslState.getOslTypeStringForParameter)
        {
            return (*sharedState->oslState.getOslTypeStringForParameter)(name, param, path);
        }
        return "";
    }
}


GroupAttribute getFlattenedMaterialAttr(FnScenegraphIterator sgIterator, ArnoldPluginState* sharedState)
{
    return FnKat::RenderOutputUtils::getFlattenedMaterialAttr(sgIterator, sharedState->getTerminalNamesAttr());
}

//prototype
void makeOSLNetworkNode(
    const std::string locationName,
    RenderOutputUtils::ShadingNodeDescriptionMap & nodes,
    std::string nodeName,
    std::set<std::string> & createdNodeNames,
    std::vector<std::string> & commandsOut, ArnoldPluginState* sharedState);

namespace
{
    /* buildMatrial.cpp
     *
     * Materials in Arnold are first-class nodes, rather than just the implicit
     * set of attributes that happen to be set (as in prman).  In order to make
     * this work, we maintain a gloabl cache of location name ot material node.
     * At each location where a resolved material is found (e.g. a material group
     * attribute is present), a new material node will be constructed and put
     * into the cache.  Node pointers are represented as string attributes in
     * Katana which contain the name of the material node referenced.  In this
     * way, materials are constructed for the geometric locations where they
     * are discovered as well as for real material locations which are referenced
     * by the resolved materials.
     *
     * This system has the undesirable property that if you assign the same
     * material to a brazilian leaf nodes, then you end up with a great many
     * material nodes created (as each leaf builds a new material).  This is
     * a parallel to what happens in prman, and is a consequence of the fact
     * that materials may have per-object overrides that apply after resolution,
     * which is why we can't just build material nodes for the pre-resolved
     * material locations and have the leaf nodes all reference those.  To be
     * more specific, overrides theoretically could create a whole slew of
     * unique and otherwise anonymous materials on many leaf locations.
     *
     * It's possible we could have another level of cache, where we hash
     * the material parameters, and collapse all materials with the same
     * parameter values to the same material node, when it appears on the
     * geometry.  It's possible to do this fairly trivially by just converting
     * the material attribute to text, then hashing that for use as a cache key.
     * I'm hesitant to put in this optimization until we've got a measure
     * of performance on how important it will actually be.
     *
     * As of this writing, the applyShaderParameters call is also used in
     * buildLights, as a convenient utility for converting ScenegraphAttrs
     * into Arnold node parameters.
     *
     */


    /**
     *  Given an AtNodeEntry and a parameter name returns a AtParamEntry for
     *  the correnspondent node parameter. The paramName string can refer to a
     *  parameter component (example: Kd_Color.r), for which the correspondent
     *  parameter (Kd_Color) is returned.
     */
    const AtParamEntry* getParamFromNode(const AtNodeEntry *baseNodeEntry,
                                         const std::string& paramName, ArnoldPluginState* sharedState)
    {
        // Retrieve the name of the base parameter if it contains components
        std::string baseName = paramName;
        size_t pos = paramName.find_first_of('.');
        if (pos != std::string::npos )
        {
            baseName = paramName.substr(0, pos);
        }

        return AiNodeEntryLookUpParameter(baseNodeEntry, baseName.c_str());
    }

    AtNode* makeNetworkNode(std::string type, FnScenegraphIterator sgIterator,
                            std::vector<AtNode*>* childNodes,
                            RenderOutputUtils::ShadingNodeDescriptionMap & nodes,
                            std::string nodeName, ArnoldNodeMap_t& createdNodes, ArnoldPluginState* sharedState)
    {
        std::string uniqueNodeName = nodeName + sgIterator.getFullName()
                                   + std::string(":") + type + std::string("Shader");


        // Check if we made this not already, bail early if we can
        ArnoldNodeMap_t::iterator createdNodesI = createdNodes.find(uniqueNodeName);
        if(createdNodesI != createdNodes.end()) return (*createdNodesI).second;

        // Make node
        AtNode* arnoldShadingNode = AiNodeLookUpByName(uniqueNodeName.c_str());

        if (!arnoldShadingNode)
        {
            // Try and find the node name in the nodes map

            RenderOutputUtils::ShadingNodeDescription node = nodes.getShadingNodeDescriptionByName(nodeName);
            if (!node.isValid())
            {
                AiMsgDebug("[kat] could not find shading network node %s in ShadingNetworkNodeMap @ %s",
                    nodeName.c_str(), sgIterator.getFullName().c_str());
                return NULL;
            }

            std::string nodeType = DefaultAssetPlugin::resolveAsset(node.getType());

            // Now support the unholy union of osl and standard nodes
            // TODO: move this to a separate function so it's not duplicated
            if (pystring::startswith(nodeType, "osl:"))
            {
                std::set<std::string> createdNodeNames;
                std::vector<std::string> commandsOut;

                makeOSLNetworkNode(sgIterator.getFullName(), nodes, nodeName,
                    createdNodeNames, commandsOut, sharedState);

                if (!commandsOut.empty())
                {
                    arnoldShadingNode = AiNode("osl_shader");

                    if (!arnoldShadingNode)
                    {
                        AiMsgDebug("[kat] OSL not available.");
                        return NULL;
                    }
                    else
                    {
                        createdNodes[uniqueNodeName] = arnoldShadingNode;

                        AiNodeSetStr(arnoldShadingNode, "name", uniqueNodeName.c_str());

                        std::vector<const char *> arrayValues;

                        arrayValues.reserve(commandsOut.size());

                        for (std::vector<std::string>::const_iterator I = commandsOut.begin();
                                I!=commandsOut.end(); ++I)
                        {
                            arrayValues.push_back((*I).c_str());
                        }

                        AiNodeSetArray(arnoldShadingNode, "commands",
                                AiArrayConvert(arrayValues.size(), 1, AI_TYPE_STRING,
                                    (void*)(&arrayValues[0])));

                        if (childNodes)
                        {
                            childNodes->push_back(arnoldShadingNode);
                        }
                    }
                }

                return arnoldShadingNode;
            }
            //END TODO

            arnoldShadingNode = AiNode(nodeType.c_str());
            if(!arnoldShadingNode)
            {
                AiMsgDebug("[kat] could not create Arnold shading node %s. Node entry not found. Check shader path and/or shader.", DefaultAssetPlugin::resolveAsset(node.getType()).c_str());
                return NULL;
            }

            //AiMsgInfo("[kat] created shading network node %s", nodeName.c_str());
            AiNodeSetStr(arnoldShadingNode, "name", uniqueNodeName.c_str());

            // Push new node onto procedural's stack
            if(childNodes) childNodes->push_back(arnoldShadingNode);

            // Remember we made this node for this network already
            createdNodes[uniqueNodeName] = arnoldShadingNode;

            // Set params
            unsigned int numberOfParameterNames  = node.getNumberOfParameterNames();
            for (unsigned int i=0; i<numberOfParameterNames; ++i)
            {
                std::string paramName = node.getParameterName(i);
                applyShaderParam(type, arnoldShadingNode, paramName,
                    node.getParameter(paramName),
                    sgIterator, childNodes, true, sharedState);
            }

            // Get all connections
            // Run through the connections and walk "up"
            unsigned int numberOfConnectionNames  = node.getNumberOfConnectionNames();
            for (unsigned int i=0; i<numberOfConnectionNames; ++i)
            {
                std::string connectionName = node.getConnectionName(i);
                RenderOutputUtils::ShadingNodeConnectionDescription connection = node.getConnection(connectionName);

                if (connection.isValid())
                {
                    // Make incoming node (recursive)
                    AtNode* upConnectionNode = makeNetworkNode(type, sgIterator, childNodes, nodes,
                        connection.getConnectedNodeName(), createdNodes, sharedState);

                    // Link up new node
                    if (upConnectionNode)
                    {
                        // Need to figure out if we are a PTR param
                        const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(arnoldShadingNode);
                        const AtParamEntry* paramEntry = getParamFromNode(baseNodeEntry, connection.getName(), sharedState);

                        if (!paramEntry)
                        {
                            AiMsgInfo("[kat] No attribute '%s' found on shader node '%s'.",
                                connection.getName().c_str(), AiNodeGetStr(arnoldShadingNode, "name"));
                            continue;
                        }
                        int type = AiParamGetType(paramEntry);

                        // Remove the "out." portion if the input is a component, for which the
                        // name will be something like "out.input_component" (ex: "out.r") or leave
                        // the output name as "" if the whole output is meant to be connected
                        std::string outputName = "";
                        std::string outputPortName = connection.getConnectedPortName();
                        std::size_t dotPos = outputPortName.find('.');
                        if (dotPos != std::string::npos)
                        {
                            outputName = outputPortName.substr(dotPos + 1);
                        }



#ifdef AI_TYPE_NODE
                        if(type == AI_TYPE_POINTER || type == AI_TYPE_NODE)
#else
                        if(type == AI_TYPE_POINTER)
#endif
                        {

                            // If it's a shader and the node type of the shader is pointer,
                            // then do a node link
                            const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(upConnectionNode);
#ifdef AI_TYPE_NODE
                            if (AiNodeEntryGetType(baseNodeEntry) == AI_NODE_SHADER
                                    && (AiNodeEntryGetOutputType(baseNodeEntry) == AI_TYPE_POINTER
                                        || AiNodeEntryGetOutputType(baseNodeEntry) == AI_TYPE_NODE ))
#else
                            if (AiNodeEntryGetType(baseNodeEntry) == AI_NODE_SHADER
                                    && (AiNodeEntryGetOutputType(baseNodeEntry) == AI_TYPE_POINTER))
#endif
                            {
                                AiNodeLinkOutput(upConnectionNode, outputName.c_str(), arnoldShadingNode, connection.getName().c_str());
                            }
                            else
                            {
                                AiNodeSetPtr(arnoldShadingNode, connection.getName().c_str(), upConnectionNode);
                            }

                        }
                        else if (type == AI_TYPE_ARRAY)
                        {
                            AtArray * existingArray = AiNodeGetArray(arnoldShadingNode,
                                    connection.getName().c_str());

#ifdef AI_TYPE_NODE
                            if (existingArray->type == AI_TYPE_POINTER || existingArray->type == AI_TYPE_NODE)
#else
                            if (existingArray->type == AI_TYPE_POINTER)
#endif
                            {
                                std::vector<AtNode*> newArray;

                                newArray.insert(newArray.begin(), (AtNode**)existingArray->data,
                                        ((AtNode**)existingArray->data)+existingArray->nelements);

                                newArray.push_back(upConnectionNode);

                                AiNodeSetArray(arnoldShadingNode, connection.getName().c_str(),
                                        AiArrayConvert(newArray.size(), 1,
                                               existingArray->type , &newArray[0]));
                            }
                        }
                        else
                        {
                            AiNodeLinkOutput(upConnectionNode, outputName.c_str(), arnoldShadingNode, connection.getName().c_str());
                        }

                        //AiMsgInfo("[kat] linked upConnectioNode %s->%s", connection->getConnectedNodeName().c_str(), connection->getName().c_str());
                    }
                    else
                    {
                        AiMsgDebug("[kat] failed to create upConnectioNode %s", connection.getConnectedNodeName().c_str());
                    }
                }
            }
        }

        return arnoldShadingNode;
    }


    template <typename Attr_t>
    std::string InternalAttrAsOSLParamValue(Attr_t attr)
    {
        std::ostringstream buffer;

        ConstVector<typename Attr_t::value_type> data = attr.getNearestSample(0);
        for (typename ConstVector<typename Attr_t::value_type>::const_iterator I =
                data.begin(); I!=data.end(); ++I)
        {
            buffer << " " << (*I);
        }

        return buffer.str();
    }


    std::string AttrAsOSLParamValue(Attribute attr)
    {
        FnAttribute::StringAttribute stringAttr = attr;
        if (stringAttr.isValid())
        {
            std::ostringstream buffer;

            StringConstVector data = stringAttr.getNearestSample(0);

            for (StringConstVector::const_iterator I=data.begin(); I!=data.end(); ++I)
            {
                buffer << " \"" << (*I) << "\"";
            }

            return buffer.str();
        }

        FnAttribute::IntAttribute intAttr = attr;
        if (intAttr.isValid())
        {
            return InternalAttrAsOSLParamValue(intAttr);
        }

        FnAttribute::FloatAttribute floatAttr = attr;
        if (floatAttr.isValid())
        {
            return InternalAttrAsOSLParamValue(floatAttr);
        }

        FnAttribute::DoubleAttribute doubleAttr = attr;
        if (doubleAttr.isValid())
        {
            return InternalAttrAsOSLParamValue(doubleAttr);
        }

        return "";
    }

} //end anonymous namesacpe

    void makeOSLMonolithicCommmandString(
            const std::string & terminalName,
            const std::string & terminalShader,
            GroupAttribute paramGroup,
            std::vector<std::string> & commandsOut, ArnoldPluginState* sharedState)
    {
        if (paramGroup.isValid())
        {
            for (int i=0; i<paramGroup.getNumberOfChildren(); ++i)
            {
                std::string attrName = paramGroup.getChildName(i);
                Attribute attr = paramGroup.getChildByIndex(i);

                std::string command(GetOSLTypeStringForParameter(
                    terminalShader, attrName,
                    getenv("ARNOLD_SHADERLIB_PATH"), sharedState));
                if (command.empty()) continue;

                std::string values = AttrAsOSLParamValue(attr);
                if (values.empty()) continue;

                command += " ";
                command += attrName;
                command += values;
                command += ";";
                commandsOut.push_back(command);
            }
        }

        commandsOut.push_back("shader " + terminalShader + " " +
                 terminalName + ";");
    }



    void makeOSLNetworkNode(
            const std::string locationName,
            RenderOutputUtils::ShadingNodeDescriptionMap & nodes,
            std::string nodeName,
            std::set<std::string> & createdNodeNames,
            std::vector<std::string> & commandsOut, ArnoldPluginState* sharedState)
    {
        if (createdNodeNames.find(nodeName) != createdNodeNames.end())
        {
            return;
        }

        createdNodeNames.insert(nodeName);

        RenderOutputUtils::ShadingNodeDescription node = nodes.getShadingNodeDescriptionByName(nodeName);
        if (!node.isValid())
        {
            AiMsgDebug("[kat] could not find shading network node"
                       " %s in ShadingNetworkNodeMap @ %s",
                            nodeName.c_str(), locationName.c_str());
            return;
        }

        std::string shaderName = DefaultAssetPlugin::resolveAsset(node.getType()).c_str();
        if (!pystring::startswith(shaderName, "osl:"))
        {
            AiMsgDebug("[kat] expected OSL shader for shading network node: %s : %s",
                    nodeName.c_str(), locationName.c_str());
            return;
        }

        shaderName = pystring::slice(shaderName, 4);

        //check the OSL query to confirm that the specified shader is available
        if (!OSLShaderExists(shaderName, getenv("ARNOLD_SHADERLIB_PATH"), sharedState))
        {
            AiMsgDebug("[kat] can't load OSL Shader: %s : %s : %s", shaderName.c_str(),
                    nodeName.c_str(), locationName.c_str());
            return;
        }



        std::vector<std::string> connectionCommands;

        //write connected nodes
        unsigned int numberOfConnectionNames  = node.getNumberOfConnectionNames();
        for (unsigned int i=0; i<numberOfConnectionNames; ++i)
        {
            std::string connectionName = node.getConnectionName(i);
            RenderOutputUtils::ShadingNodeConnectionDescription connection =
                    node.getConnection(connectionName);
            if (!connection.isValid()) continue;

            makeOSLNetworkNode(locationName, nodes, connection.getConnectedNodeName(),
                createdNodeNames, commandsOut, sharedState);

            if (createdNodeNames.find(connection.getConnectedNodeName()) ==
                createdNodeNames.end())
            {
                continue;
            }

            //connection
            connectionCommands.push_back("connect " +
                    connection.getConnectedNodeName() + "." +
                    connection.getConnectedPortName() + " " +
                    nodeName + "." + connection.getName());
        }


        //write parameters
        unsigned int numberOfParameterNames  = node.getNumberOfParameterNames();
        for (unsigned int i=0; i<numberOfParameterNames; ++i)
        {
            std::string paramName = node.getParameterName(i);
            Attribute attr = node.getParameter(paramName);
            if (!attr.isValid())
            {
                continue;
            }

            std::string command = GetOSLTypeStringForParameter(shaderName,
                    paramName, getenv("ARNOLD_SHADERLIB_PATH"), sharedState);

            if (command.empty())
            {
                continue;
            }

            std::string values = AttrAsOSLParamValue(attr);

            if (values.empty())
            {
                continue;
            }

            command += " ";
            command += (paramName);
            command += values;
            command += ";";

            commandsOut.push_back(command);
        }

        //write self
        commandsOut.push_back("shader " + shaderName + " " + nodeName + ";");

        //write connections
        commandsOut.insert(commandsOut.end(), connectionCommands.begin(),
                connectionCommands.end());
    }




namespace
{
    const char * kDoNotInheritToken = "<donotinherit>";


    AtNode* getNetworkedShader(std::string type, FnScenegraphIterator sgIter, GroupAttribute m,
        std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState)
    {
        //get shader and params
        FnAttribute::StringAttribute shaderAttr = m.getChildByName("terminals."+type);
        if (!shaderAttr.isValid())
        {
            return NULL;
        }
        std::string shaderName = shaderAttr.getValue("", false);

        //allow for terminal connections to mask inheritance without error
        if (shaderName == kDoNotInheritToken)
        {
            return NULL;
        }

        //check for style "network"
        FnAttribute::StringAttribute styleAttr = m.getChildByName("style");
        if (styleAttr.getValue("", false) == "network")
        {
            RenderOutputUtils::ShadingNodeDescriptionMap nodes(m);

            RenderOutputUtils::ShadingNodeDescription node = nodes.getShadingNodeDescriptionByName(shaderName);
            if (!node.isValid())
            {
                AiMsgDebug("[kat] could not find shading network node"
                       " %s in ShadingNetworkNodeMap @ %s",
                            shaderName.c_str(), sgIter.getFullName().c_str());
                return NULL;
            }

            AtNode* terminal = NULL;
            if (pystring::startswith(DefaultAssetPlugin::resolveAsset(node.getType()), "osl:"))
            {
                std::set<std::string> createdNodeNames;
                std::vector<std::string> commandsOut;

                makeOSLNetworkNode(sgIter.getFullName(), nodes, shaderName, createdNodeNames,
                        commandsOut, sharedState);

                if (!commandsOut.empty())
                {
                    terminal = AiNode("osl_shader");

                    if (!terminal)
                    {
                        AiMsgDebug("[kat] OSL not available.");
                    }
                    else
                    {
                        std::vector<const char *> arrayValues;

                        arrayValues.reserve(commandsOut.size());

                        for (std::vector<std::string>::const_iterator I = commandsOut.begin();
                                I!=commandsOut.end(); ++I)
                        {
                            arrayValues.push_back((*I).c_str());
                        }

                        AiNodeSetArray(terminal, "commands",
                                AiArrayConvert(arrayValues.size(), 1, AI_TYPE_STRING,
                                    (void*)(&arrayValues[0])));


                        //be sure to report the terminal node
                        //in the non-osl-case, the terminal would be reported during
                        //makeNetworkNode
                        if (childNodes)
                        {
                            childNodes->push_back(terminal);
                        }
                    }
                }
            }
            else
            {
                ArnoldNodeMap_t createdNodes;
                terminal = makeNetworkNode(type, sgIter, childNodes, nodes, shaderName, createdNodes, sharedState);
            }

            if(!terminal)
            {
                AiMsgDebug("[kat] failure to assign networked material.");
                return NULL;
            }

            //Top node name does not need to have
            //shader network node name appended.
            //so rename it here to follow naming convention.
            //  "PATHNAME"+":"+"TYPE"+"Shader"
            //  ex: /root/world/lgt/lgt_key:arnoldLightShader
            std::string topNodeName = sgIter.getFullName() + std::string(":") + type + std::string("Shader");
            AiNodeSetStr(terminal, "name", topNodeName.c_str());


            return terminal;
        }

        return NULL;
    }




    AtNode* getShaderLocal(std::string type, FnScenegraphIterator sgIter, GroupAttribute m,
        std::vector<AtNode*>* childNodes, bool hashed, bool unique, ArnoldPluginState* sharedState)
    {
        if(!sgIter.isValid() || !m.isValid()) return NULL;

        std::string hashableMaterial;

        if (hashed)
        {
            //make hashable string from entire material xml
            std::string ext;
            if (unique)
            {
                ext = sgIter.getFullName() + std::string(":") + type;
            }
            else
            {
                ext = type;
            }

            std::string inputString = sharedState->updateLastLocationPath(
                                                            sgIter.getFullName(),
                                                            m.getHash().str());
            hashableMaterial = inputString+ std::string(":") + ext;

            // Look up m(sgIter) in a shaderMap!
            AtNode* shaderNode = sharedState->getShader(hashableMaterial);
            if (shaderNode)
            {
                //std::cerr << "[kat] got hashed shader: "  << ext << " " << sgIter.getFullName() << std::endl;
                //AiMsgInfo("[kat] got hashed shader");

                return shaderNode;
            }
            else
            {
                //material not yet made
            }

        }
        else
        {
            //std::cerr << "[kat] NOT hashed: " << sgIter.getFullName() << std::endl;
            //AiMsgInfo("[kat] NOT hashed: %s ", sgIter.getFullName().c_str());
        }

        //try networked shader
        AtNode* shaderNode = getNetworkedShader(type, sgIter, m, childNodes, sharedState);

        //nope, try plain
        if (!shaderNode)
        {

            FnAttribute::StringAttribute shaderAttr;

            if (pystring::startswith(type, "arnoldCoshaders."))
            {
                 shaderAttr = m.getChildByName(type + std::string(".shader"));
            }
            else
            {
                shaderAttr = m.getChildByName(type + std::string("Shader"));
            }

            if (!shaderAttr.isValid())
            {
                //AiMsgWarning("[kat] no shader attr on %s", sgIter.getFullName().c_str());
                //sharedState->global_shadermap[hashableMaterial] = 0;

                return 0;
            }

            std::string shaderNodeName =
                DefaultAssetPlugin::resolveAsset(shaderAttr.getValue("", false));

            //Allows empty terminal connections to mask inherited values
            if (shaderNodeName == kDoNotInheritToken)
            {
                shaderNodeName = "";
            }

            if (pystring::startswith(shaderNodeName, "osl:"))
            {
                std::vector<std::string> commandsOut;
                std::string oslShaderName = pystring::slice(shaderNodeName, 4);

                if (!OSLShaderExists(oslShaderName, getenv("ARNOLD_SHADERLIB_PATH"), sharedState))
                {
                    AiMsgDebug("[kat] can't load OSL Shader: %s : %s",
                            oslShaderName.c_str(), sgIter.getFullName().c_str());
                    return 0;
                }

                makeOSLMonolithicCommmandString(type, oslShaderName,
                    m.getChildByName(type + std::string("Params")),
                    commandsOut, sharedState);

                shaderNode = AiNode("osl_shader");

                if (!shaderNode)
                {
                    AiMsgDebug("[kat] OSL not available.");
                }
                else
                {
                    std::vector<const char *> arrayValues;
                    arrayValues.reserve(commandsOut.size());
                    for (std::vector<std::string>::const_iterator I = commandsOut.begin();
                            I!=commandsOut.end(); ++I)
                    {
                        arrayValues.push_back((*I).c_str());
                    }
                    AiNodeSetArray(shaderNode, "commands",
                        AiArrayConvert(arrayValues.size(), 1, AI_TYPE_STRING,
                            (void*)(&arrayValues[0])));
                }
            }
            else
            {
                if (!shaderNodeName.empty())
                {
                    shaderNode = AiNode(shaderNodeName.c_str());
                    if (!shaderNode)
                    {
                        AiMsgWarning("[kat] could not allocate node %s",
                            shaderAttr.getValue("", false).c_str());
                    }
                }

                if (!shaderNode)
                {
                    return 0;
                }

                GroupAttribute shaderParamsAttr;
                if (pystring::startswith(type, "arnoldCoshaders."))
                {
                    shaderParamsAttr = m.getChildByName(type + std::string(".params"));
                }
                else
                {
                    shaderParamsAttr = m.getChildByName(type + std::string("Params"));

                }

                if (shaderParamsAttr.isValid())
                {
                    applyShaderParams(type, shaderNode, shaderParamsAttr, sgIter, childNodes, sharedState);
                }
            }


            if (shaderNode)
            {
                if (childNodes)
                {
                    childNodes->push_back(shaderNode);
                }

                std::string shaderName = sgIter.getFullName() + std::string(":") + type + std::string("Shader");
                AiNodeSetStr(shaderNode, "name", shaderName.c_str());
            }
        }

        if (hashed)
        {
            //AiMsgDebug("[kat] add shader %s:%s", sgIter.getFullName().c_str(),type.c_str());
            sharedState->setShader(hashableMaterial, shaderNode);
        }

        return shaderNode;
    }
}

    AtNode* getShader(std::string type, FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes,
        bool hashed, bool unique, bool suppress, ArnoldPluginState* sharedState)
    {
        //AiMsgDebug("[kat] look for shader %s in %s", sgIterator.getFullName().c_str(),type.c_str());
        // Find the material that applies to this scenegraph location.
        GroupAttribute m;
        FnScenegraphIterator sgIter = sgIterator;

        m = getFlattenedMaterialAttr(sgIterator, sharedState);

        if (!m.isValid())
        {
            GroupAttribute m2;

            bool warning = true;
            std::string type = sgIterator.getType();
            if (type == "polymesh" || type == "subdmesh")
            {
               //check faceset for warning message
               FnScenegraphIterator child = sgIterator.getFirstChild();
               if (child.isValid())
               {
                  warning = true;
                  for(; child.isValid(); child = child.getNextSibling())
                  {
                     m2 = child.getAttribute("material");
                     if (m2.isValid())
                     {
                        warning = false;
                        break;
                     }
                  }
               }
            }

            if (warning && !suppress)
            {
               AiMsgWarning("[kat] no arnold material on %s", sgIterator.getFullName().c_str());
            }
            return NULL;
        }

        return  getShaderLocal(type, sgIter, m, childNodes, hashed, unique, sharedState);
    }
    /******************************************************************************
     *
     * getShaderByPath
     *
     ******************************************************************************/

    AtNode* getShaderByPath(const std::string& pathName, FnScenegraphIterator sgIterator,
        std::vector<AtNode*>* childNodes, bool hashed, ArnoldPluginState* sharedState, const std::string stype)
    {
        AtNode* shaderNode = sharedState->getShader(pathName);
        if (shaderNode)
        {
            return shaderNode;
        }

        std::vector<std::string> token;
        pystring::split(pathName, token, ":");

        FnScenegraphIterator rootIterator = sgIterator.getRoot();
        FnScenegraphIterator shaderIterator = rootIterator.getByPath(token[0]);

        if (!shaderIterator.isValid())
        {
            AiMsgDebug("[kat] Shader node '%s' not found, referenced from '%s'.",
                token[0].c_str(), sgIterator.getFullName().c_str());
            return 0;
        }

        std::vector<std::string> typelist;
        if (pathName.rfind(":arnold") == std::string::npos)
        {
           if (stype != "")
              typelist.push_back(stype);
           typelist.push_back("arnoldSurface");
           typelist.push_back("arnoldBump");
           //typelist.push_back("arnoldSubdiv");
           typelist.push_back("arnoldDisplacement");
           typelist.push_back("arnoldGI_Quickshade");
           typelist.push_back("arnoldLight");
        }
        else
        {
           typelist.push_back(token[token.size()-1]);
        }


        for(size_t i = 0; i < typelist.size(); i++) {
            bool unique = false;
            if (typelist[i] == "arnoldLight")
               unique = true;
            AtNode* node = getShader(typelist[i], shaderIterator, childNodes, hashed,unique,i, sharedState);
            if (node)  return node;
        }
        return 0;
    }

////////////////////////////////////////

/*
 * Allocates an AtArray to be used on a node parameter of the given type and
 * size. The size is defined by the size of the attribute that contains the
 * data that will fill the Array and the number of entries in that attribute
 * per array element. The size is calculated with: attrSize / elementSize.
 */
AtArray * createParameterArray( const char *attrNameStr,
                                const unsigned int attrSize,
                                const unsigned int elementSize,
                                const unsigned char type)
{
    AtArray * array = AiArrayAllocate(attrSize / elementSize, 1, type);
    if (!array)
    {
        AiMsgWarning("[kat] Problem allocating array parameter %s", attrNameStr);
        return NULL;
    }

   return array;
}

/*
 * Gets the node for the given location. This node will either already exist
 * in the scene or will be generated by a Delegate plugin. Types of nodes that
 * cannot be generated by a Delegate or that do not exist yet in the scene will
 * not be returned.
 *
 * This is meant to be called to instantiate a node to be set on a node
 * parameter of the type AI_TYPE_NODE, hence the paramName.
 */
AtNode * getNodeForLocation( const std::string & location,
                             FnScenegraphIterator sgIterator,
                             std::vector<AtNode*>* childNodes,
                             ArnoldPluginState* sharedState)
{
    FnScenegraphIterator srcIterator = sgIterator.getRoot().getByPath(location);
    if (!srcIterator.isValid())
    {
        AiMsgWarning("[kat] No ScenegraphIterator for the location '%s'",
                     location.c_str());
        return NULL;
    }

    StringAttribute typeAttr = srcIterator.getAttribute("type");
    if (!typeAttr.isValid())
    {
        std::string msg = "[kat] No 'type' attribute found in the location '";
        msg += location;

        AiMsgWarning(msg.c_str());
        return NULL;
    }

    // Get the list of allowed location types (the ones for which there
    // is a Scenegraph Delegate
    std::vector<std::string> allowedLocations;
    RenderOutputUtils::fillDelegateHandledLocationTypesList(
            allowedLocations, "arnold");

    // Make sure the location type is one of the allowed locations
    std::string type = typeAttr.getValue();

    if ( std::find( allowedLocations.begin(),
                    allowedLocations.end(),
                    type) == allowedLocations.end())
    {
        std::string msg = "[kat] Parameter of type 'AI_NODE' can only be " \
               "connected to location types processed by a " \
               "ScenegraphLocationDelegate plugin: ";

        std::vector<std::string>::const_iterator it;
        for(it = allowedLocations.begin(); it != allowedLocations.end(); ++it)
        {
            msg += " '";
            msg += *it;
            msg += "' ";
        }

        AiMsgWarning(msg.c_str());
        return NULL;
    }

    // If there is already a node for this location, then use it
    AtNode * primNode = AiNodeLookUpByName(location.c_str());

    // Otherwise call the SG Delegate plugin to generate the node for us.
    // Nodes that are not generated
    if (primNode)
    {
        return primNode;
    }

    // Use a SG Delegate plugin to generate the other node
    ArnoldSceneGraphLocationDelegateInput sceneGraphLocationDelegateInput;
    sceneGraphLocationDelegateInput.childNodes = childNodes;
    sceneGraphLocationDelegateInput.sharedState = sharedState;
    RenderOutputUtils::processLocation(srcIterator,"arnold",type,
                                       &sceneGraphLocationDelegateInput,
                                       (void**)(&primNode));

    // Connect the other node to the current parameter
    if (!primNode)
    {
        std::string msg;
        msg += "[kat] WARNING: no node was generated for ";
        msg += location;
        msg += " while connecting it to the shader on location ";
        msg += sgIterator.getFullName();
        AiMsgWarning(msg.c_str());
    }

    return primNode;
}

/******************************************************************************
 *
 * applyShaderParam
 *
 ******************************************************************************/
void applyShaderParam (std::string type,AtNode* node,
                       const std::string& attrName,
                       Attribute attrRef,
                       FnScenegraphIterator sgIterator,
                       std::vector<AtNode*>* childNodes,
                       bool doPointers,
                       ArnoldPluginState* sharedState)
{
    if (!attrRef.isValid())
    {
       AiMsgInfo("[kat] Cannot find shader param %s", attrName.c_str());
    }

    // Check if there is a param with that name on the arnold node
    const char* attrNameStr = attrName.c_str();
    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);
    const AtParamEntry* paramEntry = AiNodeEntryLookUpParameter(baseNodeEntry,
            attrName.c_str());
    if (!paramEntry)
    {
        AiMsgDebug("[kat] No attribute '%s' found on shader node '%s'.",
                attrNameStr, AiNodeGetStr(node, "name"));
        return;
    }

    // Get the parameter type
    int paramType = AiParamGetType(paramEntry);

    // Get the parameter value
    const AtParamValue* paramValue = AiParamGetDefault(paramEntry);

    // Only one of these attributes will be valid
    FnAttribute::IntAttribute intAttr = attrRef;
    FnAttribute::FloatAttribute floatAttr = attrRef;
    FnAttribute::DoubleAttribute doubleAttr = attrRef;
    FnAttribute::StringAttribute stringAttr = attrRef;

    // Check if this is an array parameter and init some useful variables for it
    AtArray * array = NULL;
    bool isArray = false;
    int attrSize = 0;
    if (paramType == AI_TYPE_ARRAY)
    {
        isArray = true;

        // We will adopt the type of the elements of the array
        paramType = paramValue->ARRAY->type;

        // Only one of these attributes is valid, and it will contain the
        // number of entries that we will use to know how many elements we have
        // in the array parameter
        if (intAttr.isValid())
            attrSize = intAttr.getNearestSample(0).size();
        else if (floatAttr.isValid())
            attrSize = floatAttr.getNearestSample(0).size();
        else if (doubleAttr.isValid())
            attrSize = doubleAttr.getNearestSample(0).size();
        else if (stringAttr.isValid())
            attrSize = stringAttr.getNearestSample(0).size();
        // Else: we are dealing with an empty array. attrSize will remain 0.
    }

    // Variables used for error message construction
    bool typeMismatch = false;
    std::string needed = "";

    switch (paramType)
    {
    case AI_TYPE_BYTE:
        if(!intAttr.isValid())
        {
            needed = "a int";
            typeMismatch = true;
            break;
        }

        if (!isArray)
        {
            AiNodeSetByte(node, attrNameStr,
                    static_cast<unsigned char>(intAttr.getValue(0, false)));
        }
        else
        {
            array = createParameterArray(attrNameStr, attrSize, 1, paramType);
            if (!array) break;
            IntConstVector data = intAttr.getNearestSample(0);
            for(size_t i = 0; i < attrSize ; ++i)
            {
                AiArraySetByte(array, i, static_cast<unsigned char>(data[i]));
            }
        }
        break;

    case AI_TYPE_ENUM:
        if (stringAttr.isValid())
        {
            if (!isArray)
            {
                AiNodeSetStr(node, attrNameStr,
                             stringAttr.getValue("", false).c_str());
            }
        }
        else if (intAttr.isValid())
        {
            if (!isArray)
            {
                AiNodeSetInt(node, attrNameStr, intAttr.getValue(0, false));
            }
        }
        else
        {
            needed = "an int or a string"; typeMismatch = true;
            break;
        }
        break;

    case AI_TYPE_INT:
        if(!intAttr.isValid()) { needed = "a int"; typeMismatch = true; break; }

        if (!isArray)
        {
            AiNodeSetInt(node, attrNameStr, intAttr.getValue(0, false));
        }
        else
        {
            array = createParameterArray(attrNameStr, attrSize, 1, paramType);
            if (!array) break;
            IntConstVector data = intAttr.getNearestSample(0);
            for(size_t i = 0; i < attrSize ; ++i)
            {
                AiArraySetInt(array, i, data[i]);
            }
        }
        break;

    case AI_TYPE_UINT:
        if(!intAttr.isValid()) { needed = "a int"; typeMismatch = true; break; }
        if (!isArray)
        {
            AiNodeSetUInt(node, attrNameStr,
                    static_cast<unsigned int>(intAttr.getValue(0, false)));
        }
        else
        {
            array = createParameterArray(attrNameStr, attrSize, 1, paramType);
            if (!array) break;
            IntConstVector data = intAttr.getNearestSample(0);
            for(size_t i = 0; i < attrSize ; ++i)
            {
                AiArraySetUInt(array, i, static_cast<unsigned int>(data[i]));
            }
        }
        break;

    case AI_TYPE_BOOLEAN:
        if (floatAttr.isValid())
        {
            if (!isArray)
            {
                AiNodeSetBool(node, attrNameStr,
                        static_cast<bool>(floatAttr.getValue(0, false)));
            }
            else
            {
                array = createParameterArray(attrNameStr, attrSize, 1,
                                             paramType);
                if (!array) break;
                FloatConstVector data = floatAttr.getNearestSample(0);
                for(size_t i = 0; i < attrSize ; ++i)
                {
                    AiArraySetBool(array, i, static_cast<bool>(data[i]));
                }
            }
        }
        else if( intAttr.isValid() )
        {
            if (!isArray)
            {
                AiNodeSetBool(node, attrNameStr,
                        static_cast<bool>(intAttr.getValue(0, false)));
            }
            else
            {
                array = createParameterArray(attrNameStr, attrSize, 1,
                                             paramType);
                if (!array) break;
                IntConstVector data = intAttr.getNearestSample(0);
                for(size_t i = 0; i < attrSize ; ++i)
                {
                    AiArraySetBool(array, i, static_cast<bool>(data[i]));
                }
            }
        }
        else { needed = "a int"; typeMismatch = true; break; }
        break;

    case AI_TYPE_FLOAT:
        if(!floatAttr.isValid() && !doubleAttr.isValid())
        {
            needed = "a float"; typeMismatch = true; break;
        }
        if (floatAttr.isValid())
        {
            if (!isArray)
            {
                AiNodeSetFlt(node, attrNameStr,
                             floatAttr.getValue(0.0f, false));
            }
            else
            {
                array = createParameterArray(attrNameStr, attrSize, 1,
                                             paramType);
                if (!array) break;
                FloatConstVector data = floatAttr.getNearestSample(0);
                for(size_t i = 0; i < attrSize ; ++i)
                {
                    AiArraySetFlt(array, i, data[i]);
                }
            }
        }
        else if (doubleAttr.isValid())
        {
            if (!isArray)
            {
                AiNodeSetFlt(node, attrNameStr,
                        static_cast<float>(doubleAttr.getValue(0.0, false)));
            }
            else
            {
                array = createParameterArray(attrNameStr, attrSize, 1,
                                             paramType);
                if (!array) break;
                DoubleConstVector data = doubleAttr.getNearestSample(0);
                for(size_t i = 0; i < attrSize ; ++i)
                {
                    AiArraySetFlt(array, i, static_cast<float>(data[i]));
                }
            }
        }
        break;

    case AI_TYPE_STRING:
        if(!stringAttr.isValid())
        {
            needed = "a string";
            typeMismatch = true;
            break;
        }
        if (!isArray)
        {
            AiNodeSetStr(node, attrNameStr,
                         stringAttr.getValue("", false).c_str());
        }
        else
        {
            array = createParameterArray(attrNameStr, attrSize, 1, paramType);
            if (!array) break;
            StringConstVector data = stringAttr.getNearestSample(0);
            for(size_t i = 0; i < attrSize ; ++i)
            {
                AiArraySetStr(array, i, data[i]);
            }
        }
        break;

    case AI_TYPE_RGB:
        {
            if(!floatAttr.isValid())
            {
                needed = "float";
                typeMismatch = true;
                break;
            }
            FloatConstVector data = floatAttr.getNearestSample(0);

            if (!isArray)
            {
                if(data.size() != 3)
                {
                    needed = "3 floats";
                    typeMismatch = true;
                    break;
                }
                AiNodeSetRGB(node, attrNameStr, data[0], data[1], data[2]);
            }
            else
            {
                if(data.size() % 3 != 0)
                {
                    needed = "3 floats";
                    typeMismatch = true;
                    break;
                }

                array = createParameterArray(attrNameStr, attrSize, 3,
                                             paramType);
                if (!array) break;
                for(size_t index = 0, i = 0; i < attrSize ; i += 3, ++index)
                {
                    AtRGB value;
                    value.r = data[i];
                    value.g = data[i + 1];
                    value.b = data[i + 2];
                    AiArraySetRGB(array, index, value);
                }
            }
            break;
        }

    case AI_TYPE_RGBA:
        {
            if(!floatAttr.isValid())
            {
                needed = "float";
                typeMismatch = true;
                break;
            }
            FloatConstVector data = floatAttr.getNearestSample(0);

            if (!isArray)
            {
                if(data.size() != 4)
                {
                    needed = "4 floats";
                    typeMismatch = true;
                    break;
                }
                AiNodeSetRGBA(node, attrNameStr,
                              data[0], data[1], data[2], data[3]);
            }
            else
            {
                if(data.size() % 4 != 0)
                {
                    needed = "4 floats";
                    typeMismatch = true;
                    break;
                }

                array = createParameterArray(attrNameStr, attrSize, 4, paramType);
                if (!array) break;
                for(size_t index = 0, i = 0; i < attrSize ; i += 4, ++index)
                {
                    AtRGBA value;
                    value.r = data[i];
                    value.g = data[i + 1];
                    value.b = data[i + 2];
                    value.b = data[i + 3];
                    AiArraySetRGBA(array, index, value);
                }
            }
            break;
        }

    case AI_TYPE_VECTOR:
        {
            if(doubleAttr.isValid())
            {
                DoubleConstVector data = doubleAttr.getNearestSample(0);
                if (!isArray)
                {
                    if(data.size() != 3)
                    {
                        needed = "3 doubles";
                        typeMismatch = true;
                        break;
                    }

                    AiNodeSetVec(node, attrNameStr,
                        static_cast<float>(data[0]),
                        static_cast<float>(data[1]),
                        static_cast<float>(data[2]));
                }
                else
                {
                    if(data.size() % 3 != 0)
                    {
                        needed = "3 doubles";
                        typeMismatch = true;
                        break;
                    }

                    array = createParameterArray(attrNameStr, attrSize, 3,
                            paramType);
                    if (!array) break;

                    for(size_t index = 0, i = 0; i < attrSize ; i += 3, ++index)
                    {
                        AtVector value;
                        value.x = static_cast<float>(data[i]);
                        value.y = static_cast<float>(data[i + 1]);
                        value.z = static_cast<float>(data[i + 2]);
                        AiArraySetVec(array, index, value);
                    }
                }
            }
            else if(floatAttr.isValid())
            {
                FloatConstVector data = floatAttr.getNearestSample(0);
                if (!isArray)
                {
                    if(data.size() != 3)
                    {
                        needed = "3 floats";
                        typeMismatch = true;
                        break;
                    }
                    AiNodeSetVec(node, attrNameStr, data[0], data[1], data[2]);
                }
                else
                {
                    if(data.size() % 3 != 0)
                    {
                        needed = "3 floats";
                        typeMismatch = true;
                        break;
                    }

                    array = createParameterArray(attrNameStr, attrSize, 3,
                                                 paramType);
                    if (!array) break;
                    for(size_t index = 0, i = 0; i < attrSize ; i += 3, ++index)
                    {
                        AtVector value;
                        value.x = data[i];
                        value.y = data[i + 1];
                        value.z = data[i + 2];
                        AiArraySetVec(array, index, value);
                    }
                }
            }
            else { needed = "float or double"; typeMismatch = true; break; }

            break;
        }

    case AI_TYPE_POINT:
        {
            if(doubleAttr.isValid())
            {
                DoubleConstVector data = doubleAttr.getNearestSample(0);
                if (!isArray)
                {
                    if(data.size() != 3)
                    {
                        needed = "3 doubles";
                        typeMismatch = true;
                        break;
                    }

                    AiNodeSetPnt(node, attrNameStr,
                            static_cast<float>(data[0]),
                            static_cast<float>(data[1]),
                            static_cast<float>(data[2]));
                }
                else
                {
                    if(data.size() % 3 != 0)
                    {
                        needed = "3 floats";
                        typeMismatch = true;
                        break;
                    }

                    array = createParameterArray(attrNameStr, attrSize, 3,
                                                 paramType);
                    if (!array) break;
                    for(size_t index = 0, i = 0; i < attrSize ; i += 3, ++index)
                    {
                        AtPoint value;
                        value.x = static_cast<float>(data[i]);
                        value.y = static_cast<float>(data[i + 1]);
                        value.z = static_cast<float>(data[i + 2]);
                        AiArraySetPnt(array, index, value);
                    }
                }
            }
            else if(floatAttr.isValid())
            {
                FloatConstVector data = floatAttr.getNearestSample(0);
                if (!isArray)
                {
                    if(data.size() != 3)
                    {
                        needed = "3 floats";
                        typeMismatch = true;
                        break;
                    }

                    AiNodeSetPnt(node, attrNameStr, data[0], data[1], data[2]);
                }
                else
                {
                    if(data.size() % 3 != 0)
                    {
                        needed = "3 floats";
                        typeMismatch = true;
                        break;
                    }

                    array = createParameterArray(attrNameStr, attrSize, 3,
                                                 paramType);
                    if (!array) break;
                    for(size_t index = 0, i = 0; i < attrSize ; i += 3, ++index)
                    {
                        AtPoint value;
                        value.x = data[i];
                        value.y = data[i + 1];
                        value.z = data[i + 2];
                        AiArraySetPnt(array, index, value);
                    }
                }
            }
            else { needed = "float or double"; typeMismatch = true; break; }

            break;
        }

    case AI_TYPE_POINT2:
        {
            if(doubleAttr.isValid())
            {
                DoubleConstVector data = doubleAttr.getNearestSample(0);
                if (!isArray)
                {
                    if(data.size() != 2)
                    {
                        needed = "2 doubles";
                        typeMismatch = true;
                        break;
                    }

                    AiNodeSetPnt2(node, attrNameStr,
                            static_cast<float>(data[0]),
                            static_cast<float>(data[1]));
                }
                else
                {
                    if(data.size() % 2 != 0)
                    {
                        needed = "2 doubles";
                        typeMismatch = true;
                        break;
                    }

                    array = createParameterArray(attrNameStr, attrSize, 2,
                                                 paramType);
                    if (!array) break;
                    for(size_t index = 0, i = 0; i < attrSize ; i += 2, ++index)
                    {
                        AtPoint2 value;
                        value.x = static_cast<float>(data[i]);
                        value.y = static_cast<float>(data[i + 1]);
                        AiArraySetPnt2(array, index, value);
                    }
                }
            }
            else if(floatAttr.isValid())
            {
                FloatConstVector data = floatAttr.getNearestSample(0);
                if (!isArray)
                {
                    if(data.size() != 2)
                    {
                        needed = "2 floats";
                        typeMismatch = true;
                        break;
                    }

                    AiNodeSetPnt2(node, attrNameStr, data[0], data[1]);
                }
                else
                {
                    if(data.size() % 2 != 0)
                    {
                        needed = "2 floats";
                        typeMismatch = true;
                        break;
                    }

                    array = createParameterArray(attrNameStr, attrSize, 2,
                                                 paramType);
                    if (!array) break;
                    for(size_t index = 0, i = 0; i < attrSize ; i += 2, ++index)
                    {
                        AtPoint2 value;
                        value.x = data[i];
                        value.y = data[i + 1];
                        AiArraySetPnt2(array, index, value);
                    }
                }
            }
            else { needed = "float or double"; typeMismatch = true; break; }

            break;
        }

    case AI_TYPE_MATRIX:
        {
            DoubleConstVector data = doubleAttr.getNearestSample(0);

            if(doubleAttr.isValid())
            {
                if(!isArray)
                {
                    if(data.size() != 16)
                    {
                        needed = "16 doubles";
                        typeMismatch = true;
                        break;
                    }

                    // Convert matrix to Arnold format
                    AtMatrix matrix;
                    int i = 0;
                    for(int y = 0; y < 4; ++y)
                    {
                        for(int x = 0; x < 4; ++x)
                        {
                            matrix[y][x] = static_cast<float>(data[i++]);
                        }
                    }
                    AiNodeSetMatrix(node, attrNameStr, matrix);
                }
                else
                {
                    if(data.size() % 16 != 0)
                    {
                        needed = "16 floats";
                        typeMismatch = true;
                        break;
                    }

                    array = createParameterArray(attrNameStr, attrSize, 16,
                                                 paramType);
                    if (!array) break;
                    for(size_t index = 0, i = 0; i < attrSize;)
                    {
                        // Convert matrix to Arnold format
                        AtMatrix matrix;
                        for(int y = 0; y < 4; ++y)
                        {
                            for(int x = 0; x < 4; ++x)
                            {
                                matrix[y][x] = static_cast<float>(data[i++]);
                            }
                        }
                        AiArraySetMtx(array, index++, matrix);
                    }
                }

            }
            else if(floatAttr.isValid())
            {
                FloatConstVector data = floatAttr.getNearestSample(0);

                if(!isArray)
                {
                    if(data.size() != 16)
                    {
                        needed = "16 floats";
                        typeMismatch = true;
                        break;
                    }

                    // Convert matrix to Arnold format
                    AtMatrix matrix;
                    int i = 0;
                    for(int y = 0; y < 4; ++y)
                    {
                        for(int x = 0; x < 4; ++x)
                        {
                            matrix[y][x] = data[i++];
                        }
                    }
                    AiNodeSetMatrix(node, attrNameStr, matrix);
                }
                else
                {
                    if(data.size() % 16 != 0)
                    {
                        needed = "16 floats";
                        typeMismatch = true;
                        break;
                    }

                    array = createParameterArray(attrNameStr, attrSize, 16,
                                                 paramType);
                    if (!array) break;
                    for(size_t index = 0, i = 0; i < attrSize;)
                    {
                        // Convert matrix to Arnold format
                        AtMatrix matrix;
                        for(int y = 0; y < 4; ++y)
                        {
                            for(int x = 0; x < 4; ++x)
                            {
                                matrix[y][x] = data[i++];
                            }
                        }
                        AiArraySetMtx(array, index++, matrix);
                    }
                }
            }
            else
            {
                typeMismatch = true;
            }

            if (typeMismatch)
            {
                needed = "16 float or double";
                break;
            }
            break;
        }

    case AI_TYPE_POINTER:
        {
            if (!stringAttr.isValid())
            {
                needed = "string";
                typeMismatch = true;
                break;
            }

            if(doPointers)
            {
                if (!isArray)
                {

                    std::string reference_name = stringAttr.getValue("", false);
                    AtNode* shaderNode = NULL;
                    shaderNode = getShaderByPath(reference_name,
                                                 sgIterator,
                                                 childNodes,
                                                 true,
                                                 sharedState,
                                                 type);

                    if (shaderNode)
                    {
                        AiNodeSetPtr(node, attrNameStr, shaderNode);
                    }
                    else
                    {
                        std::vector<std::string> token;
                        pystring::split(reference_name, token, ":");
                        AiMsgDebug("[kat] cannot find ptr %s",
                                   token[0].c_str());
                    }
                }
                else
                {
                   array = createParameterArray(attrNameStr, attrSize, 1,
                                                paramType);
                   if (!array) break;
                   StringConstVector data = stringAttr.getNearestSample(0);
                   for(size_t i = 0; i < attrSize ; ++i)
                   {
                       AtNode* shaderNode = NULL;
                       shaderNode = getShaderByPath(data[i],
                                                    sgIterator,
                                                    childNodes,
                                                    true,
                                                    sharedState,
                                                    type);

                       if (shaderNode)
                       {
                           AiArraySetPtr(array, i, shaderNode);
                       }
                       else
                       {
                           std::vector<std::string> token;
                           pystring::split(data[i], token, ":");
                           AiMsgDebug("[kat] cannot find ptr %s",
                                      token[0].c_str());
                       }
                   }
                }
            }

            break;
        }

    case AI_TYPE_NODE:
        {
            if (!stringAttr.isValid())
            {
                needed = "string";
                typeMismatch = true;
                break;
            }

            if(!isArray)
            {
                // Get the location path that corresponds to the node we are
                // looking for and get its iterator
                std::string location = stringAttr.getValue("", false);
                AtNode * otherNode = getNodeForLocation(location,
                                                        sgIterator,
                                                        childNodes,
                                                        sharedState);
                if (otherNode)
                {
                    AiNodeSetPtr(node, attrNameStr, otherNode);
                }
                else
                {
                    std::vector<std::string> token;
                    pystring::split(location, token, ":");
                    AiMsgDebug("[kat] cannot find node %s",
                               token[0].c_str());
                }
            }
            else
            {
                array = createParameterArray(attrNameStr, attrSize, 1,
                                             paramType);
                if (!array) break;
                StringConstVector data = stringAttr.getNearestSample(0);
                for(size_t i = 0; i < attrSize ; ++i)
                {
                    AtNode * otherNode = getNodeForLocation(data[i],
                                                            sgIterator,
                                                            childNodes,
                                                            sharedState);
                    if (otherNode)
                    {
                        AiArraySetPtr(array, i, otherNode);
                    }
                    else
                    {
                        std::vector<std::string> token;
                        pystring::split(data[i], token, ":");
                        AiMsgDebug("[kat] cannot find node %s",
                                   token[0].c_str());
                    }
                }
            }

            break;
        }

    // Cases that are likely not going to be implemented?
    case AI_TYPE_UNDEFINED:
    default:
        break;

    }

    // If an array has been initialized than set it as a parameter of the node
    if (array && array->nelements > 0)
    {
        AiNodeSetArray(node, attrNameStr, array);
    }

    if (typeMismatch)
    {
        if (NullAttribute(attrRef).isValid())
        {
          AiMsgWarning("[kat] Type mismatch on attribute '%s' found on shader "\
                       "node '%s'.", attrNameStr, AiNodeGetStr(node, "name"));

          AiMsgWarning("Param type of '%s' is '%s' and expect '%s%s' data but "\
                       "katana data type was '%d'.",
                        attrNameStr,
                        AiParamGetTypeName(paramType),
                        needed.c_str(),
                        (isArray)? " array": "",
                        attrRef.getType());
        }
    }
}

void applyMaterialDisplacement(AtNode* node, FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState);

//if there are any surface shader assigned return true
bool applyMaterialSimple(AtNode* node, FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState)
{
    //get hashed shader node
    AtNode* surfaceShaderNode = getShader("arnoldSurface", sgIterator, childNodes, true,false,true, sharedState);

    //assign it
    if(surfaceShaderNode)
    {
        AiNodeSetPtr(node, "shader", surfaceShaderNode);

        //get hashed bump shader and assign (using novel @before mechanism - don't ask)
        AtNode* bumpShaderNode = getShader("arnoldBump", sgIterator, childNodes, true,false,true, sharedState);
        if(bumpShaderNode) AiNodeLink(bumpShaderNode, "@before", surfaceShaderNode);
    }


    std::string type = sgIterator.getType();
    //Note: When we remove subdiv shader from katana material node
    //      we need to remove this if block.
    //      Then look for displacement shader from material and
    //      if object node has subdiv shader attached,
    //      plug displacement shader to it.
    if (type == "polymesh" || type == "subdmesh")
    {
        applyMaterialDisplacement(node, sgIterator, childNodes, sharedState);
    }

    return surfaceShaderNode ? true : false;
}


void applyMaterialDisplacement(AtNode* node, FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState)
{
    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);
    if (AiNodeEntryGetName(baseNodeEntry) == std::string("ginstance"))
    {
        return;
    }
    FnAttribute::IntAttribute omitDisplacementAttr = sgIterator.getAttribute(
            "arnoldStatements.omitDisplacement", true);
    if (omitDisplacementAttr.getValue(0, false))
    {
        return;
    }


    AtNode* shaderNode = getShader("arnoldDisplacement", sgIterator, childNodes, true, false, true, sharedState);
    if (shaderNode)
    {
        AiNodeSetPtr(node, "disp_map", shaderNode);
    }
}


void applyMaterialGI_Quickshade(AtNode* node, FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState)
{
    //get hashed shader node
    AtNode* shaderNode = getShader("arnoldGI_Quickshade", sgIterator, childNodes, true, false, true, sharedState);

    //assign it
    if(shaderNode) AiNodeSetPtr(node, "GI_quickshade", shaderNode);
}



ByteVectorRcPtr_t GetCachedShaderIndexArray(const std::string & path, ArnoldPluginState* sharedState)
{
    ByteVectorRcPtr_t shaderId = sharedState->getShaderIndex(path);
    if(shaderId)
    {
        return shaderId;
    }

    return ByteVectorRcPtr_t();
}


//if there are any surface shader assigned return true
bool applyMaterialFacets(AtNode* node, FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState)
{
    if (!sgIterator.isValid() || !node) return false;

    FnAttribute::IntAttribute polyStartIndexAttr = sgIterator.getAttribute("geometry.poly.startIndex", true);
    if (!polyStartIndexAttr.isValid()) return false;
    unsigned int numPolys = polyStartIndexAttr.getNumberOfTuples()-1;

    FnScenegraphIterator child = sgIterator.getFirstChild();
    if (!child.isValid()) return false;

    std::vector<AtNode*> shaders;
    std::vector<AtNode*> dispMaps;
    ByteVectorRcPtr_t shaderidxs(new std::vector<unsigned char>(numPolys, 0));

    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);

    // If the base mesh has a shader and/or disp_map, let's put that in [0] so
    // any facet that doesn't have a material will get that shader/disp_map
    // instead.

    // Check if the node already has a shader/displacement list
    AtArray* origShaderList = NULL;    // shader
    AtArray* origDispMaplist = NULL;  // disp_map
    AtArray* origShidxs = NULL;    // shidxs

    if (AiNodeEntryLookUpParameter(baseNodeEntry, "shidxs"))
          origShidxs = AiNodeGetArray(node, "shidxs");

    if (AiNodeEntryLookUpParameter(baseNodeEntry, "shader"))
        origShaderList = AiNodeGetArray(node, "shader");

    if (AiNodeEntryLookUpParameter(baseNodeEntry, "disp_map"))
        origDispMaplist = AiNodeGetArray(node, "disp_map");

    // Get the list sizes
    unsigned int nShidxs   = 0;
    unsigned int nShaders  = 0;
    unsigned int nDispMaps = 0;

    if (origShidxs)
        nShidxs = origShidxs->nelements;

    if (origShaderList)
        nShaders = origShaderList->nelements;

    if (origDispMaplist)
        nDispMaps = origDispMaplist->nelements;

    // Get the lists sizes.
    // The number of Ids needs to be the same as the number of polys.
    bool hasShidxs = (origShidxs && origShidxs->data && nShidxs && nShidxs == numPolys);
    bool hasShaderlist = (origShaderList && origShaderList->data && nShaders > 0);
    bool hasDispMapList = (origDispMaplist && origDispMaplist->data && nDispMaps > 0);

    //AiMsgInfo("[kat] %d %d %d %d %d %d %d",
    //    (int)origShlist, (int)origShlist->data, origShlist->nelements,
    //    (int)origShidxs, (int)origShidxs->data, origShidxs->nelements, numPolys);

    // If we have an shidx list and a shader and/or disp_map arrays

    if (hasShidxs && (hasShaderlist || hasDispMapList))
    {
        //TODO: shouldn't shaders and dispMaps have the same size?

        // Populate the shaders list
        if (hasShaderlist)
        {
            AtNode** shaderListData = (AtNode**) origShaderList->data;
            for(unsigned int i = 0; i < origShaderList->nelements; i++)
            {
                //AiMsgInfo("[kat] %s", AiNodeGetStr(shlistData[i], "name"));
                shaders.push_back( shaderListData[i] );
            }
        }

        // Make sure that if there is a disp_map array then it has the same
        // size as the shaders list
        if (hasDispMapList)
        {
            AtNode** dispMapData = (AtNode**) origDispMaplist->data;
            for(unsigned int i = 0; i < origDispMaplist->nelements; i++)
            {
                dispMaps.push_back( dispMapData[i] );
            }
        }

        // Populate the indices list
        for(unsigned int i = 0; i < origShidxs->nelements; i++)
        {
            (*shaderidxs)[i] = ((unsigned char*)origShidxs->data)[i];
        }

    }
    else
    {

        // If there is a shader or a disp_map attached to the node then make
        // sure we add it to shaders and dispMaps. If one of them is NULL then
        // add a NULL to the correspondent list anyway, so that both lists end
        // up with the same number of members.
        AtNode* baseShader = (AtNode*) AiNodeGetPtr(node, "shader");
        AtNode* baseDispMap = (AtNode*) AiNodeGetPtr(node, "disp_map");

        // for some reason Arnold has this weird state where there is a dummy shader attached
        // we don't really want that one...
        if ( baseShader && AiNodeGetStr(baseShader, "name") ==  std::string("ai_default_reflection_shader"))
        {
            baseShader = NULL;
        }

        if (baseShader || baseDispMap)
        {
            shaders.push_back(baseShader);
            dispMaps.push_back(baseDispMap);
        }

    }

    for(; child.isValid(); child = child.getNextSibling())
    {
        //TODO, confirm that it's a "faceset" location just to be sure
        FnAttribute::IntAttribute facesAttr = child.getAttribute("geometry.faces", true);
        if (!facesAttr.isValid())
        {
            continue;
        }
        IntConstVector faces = facesAttr.getNearestSample(0);

        //check for a local material attribute on the faceset. If it's there,
        //go ahead and query for the fully flattened one
        GroupAttribute m = child.getAttribute("material");
        if (!m.isValid()) continue;

        //since this will mask anything assigned at the shape level, we need to incorporate
        //the full material hierarchy
        m = getFlattenedMaterialAttr(child, sharedState);
        if (!m.isValid()) continue;

        //make surface and displacement shader (this also evaluates any network we might have)
        AtNode* shaderNode = getShaderLocal("arnoldSurface", child, m, childNodes, true,false, sharedState);
        if (!shaderNode) continue;

        AtNode* bumpNode = getShaderLocal("arnoldBump", child, m, childNodes, true, false, sharedState);
        if (bumpNode) AiNodeLink(bumpNode, "@before", shaderNode);

        //store pointers for shdixs
        unsigned char shaderID = (unsigned char) shaders.size();
        shaders.push_back(shaderNode);

        // Check if it has a displacement shader attached and add it to dispMaps.
        // If the disp_map node doesn't exist (if it is NULL), then we add NULL
        // anyway to keep the dispMaps in sync with the shaders list
        AtNode* dispMapNode = getShaderLocal("arnoldDisplacement", child, m, childNodes, true, false, sharedState);
        dispMaps.push_back(dispMapNode);

        //set idxs
        int shidxSize = (*shaderidxs).size();
        bool outofrange = false;
        for(unsigned int i = 0; i < faces.size(); i++)
        {
            if (faces[i] < shidxSize)
               (*shaderidxs)[faces[i]] = shaderID;
            else
               outofrange = true;
        }

        if (outofrange)
            AiMsgWarning("[kat] faceset on '%s' had out of bound value(s).", sgIterator.getFullName().c_str());
    }

    //if we have more than just the base shader
    if(shaders.size())
    {
        //check if all shaders are the same, don't want to waste efforts here...
        std::set<AtNode*> uniqueShaders;
        uniqueShaders.insert(shaders.begin(), shaders.end());

        std::set<AtNode*> uniqueDispMaps;
        uniqueDispMaps.insert(dispMaps.begin(), dispMaps.end());

        if (uniqueDispMaps.size() == 1)
        {
            AiNodeSetPtr(node, "disp_map", dispMaps[0]);
        }

        //just one unique shader, set simple style
        if(uniqueShaders.size() == 1)
        {
            AiNodeSetPtr(node, "shader", shaders[0]);
        }
        else //multiple shaders, need to use idxs
        {
            //assign shader array to mesh node
            AiNodeSetArray(node, "shader",
                           AiArrayConvert(shaders.size(), 1,
                           AI_TYPE_POINTER, &shaders[0]));

            // Make sure that if there are any disp_map entries then they are
            // as many as the shaders
            if (shaders.size() == dispMaps.size())
            {
                AiNodeSetArray(node, "disp_map",
                               AiArrayConvert(dispMaps.size(), 1,
                               AI_TYPE_POINTER, &dispMaps[0]));
            }
            else if (uniqueDispMaps.size() > 1)
            {
                //TODO: warn
            }


            if (AiNodeEntryLookUpParameter(baseNodeEntry, "shidxs"))
            {
               //convert and set idxs
               AiNodeSetArray(node, "shidxs",
                              AiArrayConvert((*shaderidxs).size(), 1,
                              AI_TYPE_BYTE, &(*shaderidxs)[0]));

               //if in rerendering mode, we'll need to store this ourselves
               //because arnold will eat it if we don't.
               if (sharedState->isRerendering())
               {
                    sharedState->setShaderIndex(sgIterator.getFullName(), shaderidxs);
               }
            }
        }
    }

    return shaders.size() ? true : false;
}

void applyMaterial(AtNode* node, FnScenegraphIterator sgIterator, std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState)
{
    std::string type = sgIterator.getType();

    //TODO: these two should be rolled into one
    //apply shaders directly assigned to shapes
    bool found = applyMaterialSimple(node, sgIterator, childNodes, sharedState);

    if (type == "polymesh" || type == "subdmesh")
    {
        bool appliedFacesets = applyMaterialFacets(node, sgIterator, childNodes, sharedState);

        //apply shaders that might be applied to facesets
        if (!found && !appliedFacesets)
        {
            AiMsgWarning("[kat] no arnold material on %s", sgIterator.getFullName().c_str());
        }
    }

    //gi quickshade
    applyMaterialGI_Quickshade(node, sgIterator, childNodes, sharedState);
}

//applyShaderParams
void applyShaderParams(std::string type, AtNode* shaderNode,
        GroupAttribute shaderParamsAttr, FnScenegraphIterator sgIterator,
                std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState)
{
    for (int i=0; i<shaderParamsAttr.getNumberOfChildren(); ++i)
    {
        applyShaderParam(type, shaderNode, shaderParamsAttr.getChildName(i),
            shaderParamsAttr.getChildByIndex(i), sgIterator, childNodes, true, sharedState);
    }
}




