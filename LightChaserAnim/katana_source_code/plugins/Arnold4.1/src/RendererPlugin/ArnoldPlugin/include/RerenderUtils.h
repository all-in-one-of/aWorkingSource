// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef RERENDERUTILS_H_
#define RERENDERUTILS_H_

#include <FnRenderOutputUtils/FnRenderOutputUtils.h>
#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnDataBuilder.h>

#include <pystring/pystring.h>

#include <string.h>
#include <buildCamera.h>

bool SetChangeParamValue(KatParamChange& change,
                        const FnKat::Attribute attr)
{
    bool submit_change = true;

    //TODO: need to deal with MB
    switch (change.param_type)
    {
    case AI_TYPE_BOOLEAN:
    {
        if (attr.getType() == kFnKatAttributeTypeInt)
        {
            FnAttribute::IntAttribute intAttr = FnAttribute::IntAttribute(attr);
            FnKat::IntConstVector data = intAttr.getNearestSample(0);
            if (change.array_type)
            {
                // std::vector<bool> is broken and won't work properly with
                // ArrayConvert, as it's specialized to bit-pack.
                // Using vector<int> instead...
                // TODO: Something more graceful.
                std::vector<int> array(data.size());
                FnKat::IntConstVector::const_iterator iter;
                for (iter = data.begin(); iter != data.end(); ++iter)
                {
                    array.push_back((int) (*iter));
                }

                change.param_value.ARRAY = AiArrayConvert(array.size(), 1,
                        AI_TYPE_BOOLEAN, &array[0]);
            }
            else
            {
                change.param_value.BOOL = (bool) data[0];
            }
        }
        else
        {
            submit_change = false;
        }
        break;
    }
    case AI_TYPE_BYTE:
    {
        if (attr.getType() == kFnKatAttributeTypeInt)
        {
            FnAttribute::IntAttribute intAttr = FnAttribute::IntAttribute(attr);
            FnKat::IntConstVector data = intAttr.getNearestSample(0);
            if (change.array_type)
            {
                std::vector<unsigned char> array;
                FnKat::IntConstVector::const_iterator iter;
                for (iter = data.begin(); iter != data.end(); ++iter)
                {
                    array.push_back((unsigned char) (*iter));
                }

                change.param_value.ARRAY = AiArrayConvert(array.size(), 1,
                        AI_TYPE_BYTE, &array[0]);
            }
            else
            {
                change.param_value.BYTE = (unsigned char) data[0];
            }
        }
        else
        {
            submit_change = false;
        }
        break;
    }
    case AI_TYPE_INT:
    case AI_TYPE_ENUM:
    {
        if (attr.getType() == kFnKatAttributeTypeInt)
        {
            FnAttribute::IntAttribute intAttr = FnAttribute::IntAttribute(attr);
            FnKat::IntConstVector data = intAttr.getNearestSample(0);
            if (change.array_type)
            {
                change.param_value.ARRAY = AiArrayConvert(data.size(), 1,
                        AI_TYPE_INT, (void*) (&data[0]));
            }
            else
            {
                change.param_value.INT = data[0];
            }
        }
        else
        {
            submit_change = false;
        }
        break;
    }
    case AI_TYPE_FLOAT:
    {
        if (attr.getType() == kFnKatAttributeTypeFloat)
        {
            FnAttribute::FloatAttribute floatAttr = FnAttribute::FloatAttribute(
                    attr);
            FnKat::FloatConstVector data = floatAttr.getNearestSample(0);
            if (change.array_type)
            {
                change.param_value.ARRAY = AiArrayConvert(data.size(), 1,
                        AI_TYPE_FLOAT, (void*) (&data[0]));
            }
            else
            {
                change.param_value.FLT = data[0];
            }
        }
        else
        {
            submit_change = false;
        }
        break;
    }
    case AI_TYPE_RGB:
    {
        if (attr.getType() == kFnKatAttributeTypeFloat)
        {
            FnAttribute::FloatAttribute floatAttr = FnAttribute::FloatAttribute(
                    attr);
            FnKat::FloatConstVector data = floatAttr.getNearestSample(0);
            if (change.array_type)
            {
                std::vector<float> array;
                for (FnKat::FloatConstVector::const_iterator iter =
                        data.begin(); iter != data.end(); ++iter)
                {
                    array.push_back((*iter));
                }

                change.param_value.ARRAY = AiArrayConvert(array.size() / 3, 1,
                        AI_TYPE_RGB, &array[0]);
            }
            else
            {
                change.param_value.RGB.r = data[0];
                change.param_value.RGB.g = data[1];
                change.param_value.RGB.b = data[2];
            }
        }
        else
        {
            submit_change = false;
        }
        break;
    }
    case AI_TYPE_RGBA:
    {
        if (attr.getType() == kFnKatAttributeTypeFloat)
        {
            FnAttribute::FloatAttribute floatAttr = FnAttribute::FloatAttribute(
                    attr);
            FnKat::FloatConstVector data = floatAttr.getNearestSample(0);
            if (change.array_type)
            {
                std::vector<float> array;
                for (FnKat::FloatConstVector::const_iterator iter =
                        data.begin(); iter != data.end(); ++iter)
                {
                    array.push_back((*iter));
                }

                change.param_value.ARRAY = AiArrayConvert(array.size() / 4, 1,
                        AI_TYPE_RGBA, &array[0]);
            }
            else
            {
                change.param_value.RGBA.r = data[0];
                change.param_value.RGBA.g = data[1];
                change.param_value.RGBA.b = data[2];
                change.param_value.RGBA.a = data[3];
            }
        }
        else
        {
            submit_change = false;
        }
        break;
    }
    case AI_TYPE_VECTOR:
    {
        if (attr.getType() == kFnKatAttributeTypeDouble)
        {
            FnAttribute::DoubleAttribute doubleAttr =
                    FnAttribute::DoubleAttribute(attr);
            FnKat::DoubleConstVector data = doubleAttr.getNearestSample(0);
            if (change.array_type)
            {
                std::vector<double> array;
                for (FnKat::DoubleConstVector::const_iterator iter =
                        data.begin(); iter != data.end(); ++iter)
                {
                    array.push_back((*iter));
                }

                change.param_value.ARRAY = AiArrayConvert(array.size() / 3, 1,
                        AI_TYPE_VECTOR, &array[0]);
            }
            else
            {
                change.param_value.VEC.x = data[0];
                change.param_value.VEC.y = data[1];
                change.param_value.VEC.z = data[2];
            }
        }
        else
        {
            submit_change = false;
        }
        break;
    }
    case AI_TYPE_POINT:
    {
        if (attr.getType() == kFnKatAttributeTypeDouble)
        {
            FnAttribute::DoubleAttribute doubleAttr =
                    FnAttribute::DoubleAttribute(attr);
            FnKat::DoubleConstVector data = doubleAttr.getNearestSample(0);
            if (change.array_type)
            {
                std::vector<double> array;
                for (FnKat::DoubleConstVector::const_iterator iter =
                        data.begin(); iter != data.end(); ++iter)
                {
                    array.push_back((*iter));
                }

                change.param_value.ARRAY = AiArrayConvert(array.size() / 3, 1,
                        AI_TYPE_POINT, &array[0]);
            }
            else
            {
                change.param_value.PNT.x = data[0];
                change.param_value.PNT.y = data[1];
                change.param_value.PNT.z = data[2];
            }
        }
        else
        {
            submit_change = false;
        }
        break;
    }
    case AI_TYPE_POINT2:
    {
        if (attr.getType() == kFnKatAttributeTypeFloat)
        {
            FnAttribute::FloatAttribute floatAttr = FnAttribute::FloatAttribute(
                    attr);
            FnKat::FloatConstVector data = floatAttr.getNearestSample(0);
            if (change.array_type)
            {
                std::vector<float> array;
                for (FnKat::FloatConstVector::const_iterator iter =
                        data.begin(); iter != data.end(); ++iter)
                {
                    array.push_back((*iter));
                }

                change.param_value.ARRAY = AiArrayConvert(array.size() / 2, 1,
                        AI_TYPE_POINT2, &array[0]);
            }
            else
            {
                change.param_value.PNT2.x = data[0];
                change.param_value.PNT2.y = data[1];
            }
        }
        else
        {
            submit_change = false;
        }
        break;
    }
    case AI_TYPE_STRING:
    case AI_TYPE_POINTER:
    {
        if (attr.getType() == kFnKatAttributeTypeString)
        {
            FnAttribute::StringAttribute stringAttr =
                    FnAttribute::StringAttribute(attr);
            FnKat::StringConstVector data = stringAttr.getNearestSample(0);
            if (change.array_type)
            {
                change.param_value.ARRAY = AiArrayConvert(data.size(), 1,
                        AI_TYPE_STRING, (void*) (&data[0]));
            }
            else
            {
                change.param_value.STR = strdup(data[0]);
            }
        }
        else
        {
            submit_change = false;
        }
        break;
    }
    default:
        submit_change = false;
    }
    return submit_change;
}

bool setChangedParam( AtNode* node, const char* attrName, FnKat::Attribute attr )
{
    /*
     * set the parameter
     * (we can nuke the stdrup() if this thread were to manage its own
     *  pool of memory for parameter names)
     */
    KatParamChange change;
    change.node = node;
    change.is_default = false;

    /*
     * look up the parameter-type
     */
    const AtNodeEntry* baseNodeEntry = AiNodeGetNodeEntry(node);
    const AtParamEntry* pentry = AiNodeEntryLookUpParameter(baseNodeEntry, attrName);

    bool submit_change = false;
    if (pentry)
    {
        submit_change = true;
        change.param_type  = AiParamGetType(pentry);
        change.array_type  = 0;

        if (change.param_type == AI_TYPE_ARRAY)
        {
            AtArray *array;
            /* We want to know the type of the elements in the array */
            array = AiParamGetDefault(pentry)->ARRAY;
            change.param_type = array->type;
            change.array_type = 1;
        }

        // If a NullAttribute is passed, then the default value should be used.
        if (attr.getType() == kFnKatAttributeTypeNull)
        {
            change.is_default = true;
            const AtParamValue *defaultValue = AiParamGetDefault(pentry);
            change.param_value = *defaultValue;
        }
        else
        {
            submit_change = SetChangeParamValue(change, attr);
        }

        if (submit_change)
        {
            change.param_name  = strdup(attrName);
            AddNodeChange(&change);
            AiRenderInterrupt();
        }
    }
    else
    {
        submit_change = true;
        change.node  = NULL;
        change.array_type  = 0;
        if (attr.getType() == kFnKatAttributeTypeInt)
        {
            FnAttribute::IntAttribute intAttr = FnAttribute::IntAttribute(attr);
            FnKat::IntConstVector data = intAttr.getNearestSample(0);
            change.param_type  = AI_TYPE_INT;
            if (data.size() > 1)
            {
                change.array_type  = 1;
                change.param_value.ARRAY = AiArrayConvert(data.size(), 1, AI_TYPE_INT, (void*)(&data[0]));
            }
            else
            {
                change.param_value.INT = (int)data[0];
            }
        }
        else if (attr.getType() == kFnKatAttributeTypeFloat)
        {
            FnAttribute::FloatAttribute floatAttr = FnAttribute::FloatAttribute(attr);
            FnKat::FloatConstVector data = floatAttr.getNearestSample(0);
            change.param_type  = AI_TYPE_FLOAT;
            if (data.size() > 1)
            {
                change.array_type  = 1;
                change.param_value.ARRAY = AiArrayConvert(data.size(), 1, AI_TYPE_FLOAT, (void*)(&data[0]));
            }
            else
            {
                change.param_value.FLT = data[0];
            }
        }
        else if (attr.getType() == kFnKatAttributeTypeString)
        {
            FnAttribute::StringAttribute stringAttr = FnAttribute::StringAttribute(attr);
            FnKat::StringConstVector data = stringAttr.getNearestSample(0);
            change.param_type  = AI_TYPE_STRING;
            if (data.size() > 1)
            {
                change.array_type  = 1;
                change.param_value.ARRAY = AiArrayConvert(data.size(), 1, AI_TYPE_STRING, (void*)(&data[0]));
            }
            else
            {
                change.param_value.STR = strdup(data[0]);
            }
        }
        else
        {
            submit_change = false;
        }

        if (submit_change)
        {
            change.param_name  = strdup(attrName);
            AddExtraChange(&change);
            AiRenderInterrupt();
        }
    }
    return submit_change;
}

bool ApplyOptionAttributeChanges( FnAttribute::GroupAttribute params )
{
    AtNode *node = AiUniverseGetOptions();
    const AtNodeEntry* nentry = AiNodeGetNodeEntry(node);


    if (node == 0 || !params.isValid())
    {
        return false;
    }

    // filter node:
    //"AA_filter,AA_filter_width,AA_filter_domain,AA_filter_scalar_mode,"
    //"AA_sample_clamp,AA_filter_minimum,AA_filter_maximum,"
    std::string paramnames(
            "AA_samples,"
            "AA_sample_clamp_affects_aovs,"
            "threads,"
            "ignore_textures,"
            "ignore_shaders,"
            "ignore_atmosphere,"
            "ignore_lights,"
            "ignore_shadows,"
            "ignore_subdivision,"
            "ignore_displacement,"
            "ignore_smoothing,"
            "ignore_sss,"
            "ignore_bump,"
            "ignore_motion_blur,"
            "ignore_direct_lighting,"
            "ignore_dof,"
            "bucket_scanning,"
            "shadows_obey_light_linking,"
            "auto_transparency_depth,"
            "auto_transparency_mode,"
            "auto_transparency_threshold,"
            "GI_diffuse_depth,"
            "GI_diffuse_samples,"
            "GI_falloff_start_dist,"
            "GI_falloff_stop_dist,"
            "GI_glossy_depth,"
            "GI_glossy_samples,"
            "GI_reflection_depth,"
            "GI_refraction_depth,"
            "GI_refraction_samples,"
            "GI_total_depth,"
            "GI_refraction_samples,"
            "GI_single_scatter_samples,"
            "sss_bssrdf_samples,"
            "shader_gamma,"
            "texture_accept_untiled,"
            "texture_automip,"
            "texture_autotile,"
            "texture_conservative_lookups,"
            "texture_diffuse_blur,"
            "texture_failure_retries,"
            "texture_glossy_blur,"
            "texture_max_memory_MB,"
            "texture_max_open_files,"
            "texture_per_file_stats,"
            "texture_gamma,"
            "texture_searchpath,"
            "texture_max_sharpen,"
            "light_gamma,"
            "low_light_threshold,"
            "shader_nan_checks,"
            "volume_indirect_samples"
            );

    std::vector<std::string> optionparams;
    pystring::split(paramnames, optionparams, ",");

    /*
     * iterate over only the option parameters
     */
    for (std::vector<std::string>::iterator it = optionparams.begin();
            it != optionparams.end(); ++it)
    {
        const char* optionName = (*it).c_str();

        if (AiNodeEntryLookUpParameter(nentry, optionName) == NULL)
        {
            continue;
        }

        FnKat::Attribute attr = params.getChildByName(*it);
        if (attr.isValid())
        {
            setChangedParam(node, optionName, attr);
        }
        else
        {
            // Set the default value
            setChangedParam(node, optionName, FnAttribute::NullAttribute());
        }
    }
    return true;
}

bool ApplyCameraAttributeChanges( const std::string  &nodeName, FnAttribute::GroupAttribute params )
{
    AtNode *node = AiNodeLookUpByName(nodeName.c_str());

    if (node == 0 || !params.isValid())
    {
        return false;
    }

    std::string paramnames("shutter_type,"
            "filtermap,"
            "focus_distance,plane_distance" //aperture_size
            );


    //init value
    setCameraDefault(node);

    std::vector<std::string> cameraparams;
    pystring::split(paramnames, cameraparams, ",");

    /*
     * iterate over only the camera parameters
     */
    for (std::vector<std::string>::iterator it = cameraparams.begin();
            it != cameraparams.end(); ++it)
    {
        FnKat::Attribute attr = params.getChildByName(*it);
        if (attr.isValid())
        {
            setChangedParam(node, (*it).c_str(), attr);
        }
    }

    return true;
}

bool ApplyAttributeChanges( const std::string  &nodeName,
                            FnAttribute::GroupAttribute params,
                            const std::set<std::string>& ignoreDefaultsSet,
                            FnKat::RenderOutputUtils::ShadingNodeDescription nodeDesc)
{
    AtNode *node = AiNodeLookUpByName(nodeName.c_str());

    if (node == 0)
    {
        return false;
    }

    // Build a set of connected parameters (including the 'base' name of array
    // parameters) that will be skipped when forcing NullAttribute() defaults
    std::set<std::string> connectedParams;
    for (int i = 0; nodeDesc.isValid()
            && i < nodeDesc.getNumberOfConnectionNames(); ++i)
    {
        std::string connectionName = nodeDesc.getConnectionName(i);
        size_t pos = connectionName.rfind(":");
        if(pos != std::string::npos)
        {
            connectionName = connectionName.substr(0, pos);
        }
        connectedParams.insert(connectionName);
    }

    // Check which parameters are not in the material Attribute and set them
    // to their default value, unless if they are connected to another node.
    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry( node );
    AtParamIterator *it =  AiNodeEntryGetParamIterator(baseNodeEntry);
    while (!AiParamIteratorFinished(it))
    {
        const AtParamEntry *pentry = AiParamIteratorGetNext(it);
        if (!pentry)
        {
            continue;
        }

        // Get the parameter name
        std::string paramName = AiParamGetName(pentry);

        // Never update the "name" node parameter during re-rendering
        if (paramName == "name")
        {
            continue;
        }

        // Ignore the parameters in the ignore set
        if (ignoreDefaultsSet.find(paramName) != ignoreDefaultsSet.end() )
        {
            continue;
        }

        // Skip parameters that are marked as connected in our node description
        if (connectedParams.find(paramName) != connectedParams.end())
        {
            continue;
        }

        // We do not want to apply any change to an attribute that has been
        // linked to another shader's output.
        if (AiNodeIsLinked(node, paramName.c_str()))
        {
            continue;
        }

        if (!params.isValid() || !params.getChildByName(paramName).isValid())
        {
            setChangedParam(node, paramName.c_str(),
                            FnAttribute::NullAttribute());
        }
    }

    AiParamIteratorDestroy(it);

    /*
     * iterate over the parameters
     */
    if (params.isValid())
    {
        for(int i=0; i<params.getNumberOfChildren(); ++i)
        {
            const std::string attrName  = params.getChildName(i);
            FnKat::Attribute attr = params.getChildByIndex(i);
            setChangedParam(node, attrName.c_str(), attr);
        }
    }
    return true;
}

bool ApplyAttributeChanges( const std::string  &nodeName,
                            FnAttribute::GroupAttribute params,
                            const std::set<std::string>& ignoreDefaultsSet)
{
    FnKat::RenderOutputUtils::ShadingNodeDescription nodeDesc(0x0);
    return ApplyAttributeChanges(nodeName, params, ignoreDefaultsSet, nodeDesc);
}

bool ApplyAttributeChanges( const std::string  &nodeName,
                            FnAttribute::GroupAttribute params)
{
    std::set<std::string> emptySet;
    return ApplyAttributeChanges(nodeName, params, emptySet);
}


void ApplyNetworkedShaderChanges( AtNode * shaderNode,
                                  FnKat::RenderOutputUtils::ShadingNodeDescription nodeDesc,
                                  FnKat::RenderOutputUtils::ShadingNodeDescriptionMap & nodes,
                                  std::set<AtNode *> & alreadyWalkedSet,
                                  const std::set<std::string>& ignoreDefaultsSet )
{
    unsigned int numberOfParameterNames = nodeDesc.getNumberOfParameterNames();
    unsigned int numberOfConnectionNames = nodeDesc.getNumberOfConnectionNames();

    std::set<std::string> connectionNameSet;
    for (unsigned int i=0; i<numberOfConnectionNames; ++i)
    {
        connectionNameSet.insert(nodeDesc.getConnectionName(i));
    }

    FnAttribute::GroupBuilder gb;

    for (unsigned int i=0; i<numberOfParameterNames; ++i)
    {
        std::string name = nodeDesc.getParameterName(i);

        //skip the connected ones
        if (connectionNameSet.find(name) != connectionNameSet.end())
        {
            continue;
        }

        gb.set(name, nodeDesc.getParameter(name));
    }
    gb.setGroupInherit(false);

    ApplyAttributeChanges(AiNodeGetStr(shaderNode, "name"), gb.build(),
                          ignoreDefaultsSet, nodeDesc);

    alreadyWalkedSet.insert(shaderNode);

    //now go through the connections
    for (unsigned int i=0; i<numberOfConnectionNames; ++i)
    {
        const std::string name = nodeDesc.getConnectionName(i);

        FnKat::RenderOutputUtils::ShadingNodeConnectionDescription connection = nodeDesc.getConnection(name);
        if (!connection.isValid()) continue;

        //confirm that the parameter exists
        const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(shaderNode);
        const AtParamEntry* paramEntry = AiNodeEntryLookUpParameter(baseNodeEntry, connection.getName().c_str());
        if (!paramEntry) continue;

        FnKat::RenderOutputUtils::ShadingNodeDescription connectedNodeDesc = nodes.getShadingNodeDescriptionByName(
            connection.getConnectedNodeName());

        if (!connectedNodeDesc.isValid())
        {
            continue;
        }

        AtNode * connectedNode = NULL;

        int type = AiParamGetType(paramEntry);
        if (type == AI_TYPE_POINTER)
        {
            connectedNode = (AtNode *) AiNodeGetPtr(shaderNode, name.c_str());
        }
        else if (type == AI_TYPE_ARRAY)
        {
            size_t pos = name.rfind(":");
            if(pos != std::string::npos)
            {
                // This is a array element
                std::string filterName = name.substr(0, pos);
                // Parse the remainder of the string to get the array index
                int filterIndex = atoi(name.substr(pos+1).c_str());

                if(!filterName.empty())
                {
                    AtArray* connectionArray = AiNodeGetArray(
                        shaderNode, filterName.c_str());

                    if(connectionArray
                            && filterIndex < connectionArray->nelements)
                    {
                        connectedNode =
                            ((AtNode**) connectionArray->data)[filterIndex];
                    }
                }
            }
        }
        else
        {
            connectedNode = AiNodeGetLink(shaderNode, name.c_str());
        }

        if (!connectedNode)
        {
            continue;
        }
        if (alreadyWalkedSet.find(connectedNode) != alreadyWalkedSet.end())
        {
            continue;
        }

        ApplyNetworkedShaderChanges(connectedNode,
                connectedNodeDesc, nodes, alreadyWalkedSet, ignoreDefaultsSet);
    }
}

void ApplyNetworkedShaderChanges( AtNode * shaderNode,
                                  FnKat::RenderOutputUtils::ShadingNodeDescription nodeDesc,
                                  FnKat::RenderOutputUtils::ShadingNodeDescriptionMap & nodes,
                                  std::set<AtNode *> & alreadyWalkedSet)
{
    std::set<std::string> emptySet;
    ApplyNetworkedShaderChanges(shaderNode, nodeDesc, nodes, alreadyWalkedSet,
                                emptySet);
}

namespace
{
    template <typename Attr_t>
    FnKat::Attribute build_attribute(const void* attrDataStart,
                              const void* attrDataEnd,
                              unsigned long elementSize,
                              Attr_t* dummy = 0)
    {
        const typename Attr_t::value_type *typedStart = reinterpret_cast<const typename Attr_t::value_type*>(attrDataStart);
        const typename Attr_t::value_type *typedEnd = reinterpret_cast<const typename Attr_t::value_type*>(attrDataEnd);
        return Attr_t(typedStart, (typedEnd-typedStart), elementSize);
    }
}

// Attr Queue Callbacks
namespace
{
    void SetOSLLightXformAttrCallback(AtNode * node, FnKat::Attribute data, ArnoldPluginState* sharedState)
    {
        FnAttribute::FloatAttribute floatAttr = data;
        if (floatAttr.isValid())
        {
            FnKat::FloatConstVector m = floatAttr.getNearestSample(0);

            if (m.size() == 16)
            {
                std::map<float, Imath::M44d> xformSamples;

                xformSamples[0] = Imath::M44d(m[0], m[1], m[2], m[3],
                        m[4], m[5], m[6], m[7],
                        m[8], m[9], m[10], m[11],
                        m[12], m[13], m[14], m[15]);

                setLightXform(node, xformSamples);
            }
        }
    }

    void SetOSLMonolithicMaterialCallback(AtNode * node, FnKat::Attribute data, ArnoldPluginState* sharedState)
    {
        FnAttribute::GroupAttribute m = data;
        if (!m.isValid()) return;

        FnAttribute::StringAttribute terminalNameAttr = m.getChildByName("terminalName");
        if (!terminalNameAttr.isValid()) return;

        const std::string terminalName = terminalNameAttr.getValue();

        FnAttribute::StringAttribute shaderAttr = m.getChildByName(terminalName+"Shader");

        if (!shaderAttr.isValid()) return;

        FnAttribute::GroupAttribute paramsAttr = m.getChildByName(terminalName+"Params");

        std::vector<std::string> commandsOut;

        std::string shader = shaderAttr.getValue();
        if (pystring::startswith(shader, "osl:"))
        {
            shader = pystring::slice(shader, 4);
        }


        makeOSLMonolithicCommmandString(terminalName, shader, paramsAttr, commandsOut, sharedState);

        std::vector<const char *> arrayValues;
        arrayValues.reserve(commandsOut.size());

        //BEGIN duplicated from buildMaterial.cpp
        for (std::vector<std::string>::const_iterator I = commandsOut.begin();
             I!=commandsOut.end(); ++I)
        {
            arrayValues.push_back((*I).c_str());
        }
        AiNodeSetArray( node, "commands",
                        AiArrayConvert(arrayValues.size(), 1, AI_TYPE_STRING,
                        (void*)(&arrayValues[0])) );
    }

    void SetOSLGeneralNetworkCallback(AtNode * node, FnKat::Attribute data,
        const std::string & terminalName, ArnoldPluginState* sharedState)
    {
        FnAttribute::GroupAttribute m = data;
        if (!m.isValid()) return;
        if (!node) return;

        FnAttribute::StringAttribute terminalNodeAttr = m.getChildByName("terminals."+terminalName);
        if (!terminalNodeAttr.isValid()) return;

        std::string terminalNodeName = terminalNodeAttr.getValue();

        //parse the graph
        FnKat::RenderOutputUtils::ShadingNodeDescriptionMap nodes(m);

        std::set<std::string> createdNodeNames;
        std::vector<std::string> commandsOut;

        makeOSLNetworkNode(AiNodeGetStr(node, "name"), nodes, terminalNodeName,
                createdNodeNames, commandsOut, sharedState);

        std::vector<const char *> arrayValues;
        arrayValues.reserve(commandsOut.size());

        //BEGIN duplicated from buildMaterial.cpp
        for (std::vector<std::string>::const_iterator I = commandsOut.begin();
                I!=commandsOut.end(); ++I)
        {
            arrayValues.push_back((*I).c_str());
        }

        AiNodeSetArray( node, "commands",
                        AiArrayConvert(arrayValues.size(), 1, AI_TYPE_STRING,
                        (void*)(&arrayValues[0])) );
        //END duplicated
    }

    void SetOSLTerminalNameShaderNetworkCallback(AtNode * node, FnKat::Attribute data, ArnoldPluginState* sharedState)
    {
        FnAttribute::GroupAttribute m = data;
        if (!m.isValid()) return;

        FnAttribute::StringAttribute terminalNameAttr = m.getChildByName("terminalName");
        if (!terminalNameAttr.isValid()) return;

        const std::string terminalName = terminalNameAttr.getValue();

        SetOSLGeneralNetworkCallback(node, data, terminalName, sharedState);
    }

    void SetOSLLightGeometry(AtNode * node, FnKat::Attribute data)
    {
        setLightGeometry(node, data);
    }

    void ProcessXFormMatrix(
        const FnKat::DoubleConstVector & data,
        AtPoint & pos,
        AtPoint & look_at,
        AtVector & up)
    {
        AiV3Create( pos, data[12], data[13], data[14] );
        AiV3Create( look_at, data[12] - data[8],
                    data[13] - data[9],
                    data[14] - data[10] );
        AiV3Create( up, data[4], data[5], data[6] );
    }
}

#endif /* RERENDERUTILS_H_ */
