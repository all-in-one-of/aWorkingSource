// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <ArnoldRendererInfo.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WIN32
#include <sys/wait.h>
#include <libgen.h>
#include <signal.h>
#endif //_WIN32

#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <fstream>

#include <stdio.h> // popen, pclose, fgets
#include <limits.h> // LINE_MAX

#include <FnAttribute/FnDataBuilder.h>

#include <FnLogging/FnLogging.h>


#include <FnGeolib/util/Mutex.h>
namespace FnGeolibUtil = FNGEOLIBUTIL_NAMESPACE;

FnLogSetup("ArnoldRendererInfo");

namespace // anonymous
{
    // Every use of g_cache should acquire read or write lock, as appropriate.
    FnKat::RendererInfo::ShaderInfoCache<FnAttribute::GroupAttribute> g_cache;
    FnGeolibUtil::ReadWriteLock g_readWriteLock;
}

ArnoldRendererInfo::ArnoldRendererInfo()
{
}

ArnoldRendererInfo::~ArnoldRendererInfo()
{
}

FnKat::RendererInfo::RendererInfoBase* ArnoldRendererInfo::create()
{
    return new ArnoldRendererInfo();
}

void ArnoldRendererInfo::configureBatchRenderMethod(FnKat::RendererInfo::DiskRenderMethod& batchRenderMethod) const
{
    batchRenderMethod.setDebugOutputSupported(true);
    batchRenderMethod.setDebugOutputFileType("ass");
}

void ArnoldRendererInfo::fillRenderMethods(std::vector<FnKat::RendererInfo::RenderMethod*>& renderMethods) const
{
    FnKat::RendererInfo::DiskRenderMethod*    diskRenderMethod;
    FnKat::RendererInfo::PreviewRenderMethod* previewRenderMethod;
    FnKat::RendererInfo::LiveRenderMethod*    liveRenderMethod;

    diskRenderMethod = new FnKat::RendererInfo::DiskRenderMethod();
    previewRenderMethod = new FnKat::RendererInfo::PreviewRenderMethod();
    liveRenderMethod = new FnKat::RendererInfo::LiveRenderMethod();

    previewRenderMethod->setDebugOutputSupported(true);
    previewRenderMethod->setDebugOutputFileType("ass");

    renderMethods.push_back(diskRenderMethod);
    renderMethods.push_back(previewRenderMethod);
    renderMethods.push_back(liveRenderMethod);
}

void ArnoldRendererInfo::fillRendererObjectNames(std::vector<std::string>& rendererObjectNames,
                                                 const std::string& type,
                                                 const std::vector<std::string>& typeTags) const
{
    _cacheShaderInfo();
    FnGeolibUtil::AutoReadLock readLock(g_readWriteLock);

    for (CacheIterator iter = g_cache.begin(); iter!=g_cache.end(); ++iter)
    {
        FnAttribute::GroupAttribute attr = (*iter).second;

        if (type == FnAttribute::StringAttribute(attr.getChildByName(kFnRendererObjectInfoKeyType)).getValue("", false))
        {
            if (type == kFnRendererObjectTypeShader)
            {
                if (typeTags.size())
                {
                    // We only categorize Arnold shaders as "light" or "not light".

                    // So if "light" is in the requested typeTags, then make sure
                    // it is in the shader typeTags.  If not, then make sure it is
                    // not.
                    bool foundLight = false;
                    for (std::vector<std::string>::const_iterator typeIter=typeTags.begin();
                         typeIter!=typeTags.end(); ++typeIter)
                    {
                        if ((*typeIter) == "light")
                        {
                            foundLight = true;
                            break;
                        }
                    }

                    FnAttribute::StringAttribute typeTagsAttr = attr.getChildByName(kFnRendererObjectInfoKeyTypeTags);
                    std::set<std::string> typeTagsSet;
                    if (typeTagsAttr.isValid())
                    {
                        FnAttribute::StringConstVector typeTagsValue = typeTagsAttr.getNearestSample(0);
                        typeTagsSet.insert(typeTagsValue.begin(), typeTagsValue.end());
                    }
                    if (foundLight == (bool)(typeTagsSet.find("light")!=typeTagsSet.end()))
                    {
                        rendererObjectNames.push_back((*iter).first);
                    }
                }
                else
                {
                    rendererObjectNames.push_back((*iter).first);
                }
            }
            else
            {
                rendererObjectNames.push_back((*iter).first);
            }
        }
    }
}

void ArnoldRendererInfo::fillRendererObjectTypes(std::vector<std::string>& renderObjectTypes,
                                                 const std::string& type) const
{
    renderObjectTypes.clear();

    if (type == kFnRendererObjectTypeShader)
    {
        renderObjectTypes.push_back("surface");
        renderObjectTypes.push_back("bump");
        renderObjectTypes.push_back("displacement");
        renderObjectTypes.push_back("light");
        renderObjectTypes.push_back("lightFilter");
    }
    else if (type == kFnRendererObjectTypeRenderOutput)
    {
        renderObjectTypes.push_back(kFnRendererOutputTypeColor);
        renderObjectTypes.push_back(kFnRendererOutputTypeRaw);
        renderObjectTypes.push_back(kFnRendererOutputTypeScript);
        renderObjectTypes.push_back(kFnRendererOutputTypePreScript);
        renderObjectTypes.push_back(kFnRendererOutputTypeMerge);
        renderObjectTypes.push_back(kFnRendererOutputTypeForceNone);
    }
}

void ArnoldRendererInfo::fillRendererShaderTypeTags(std::vector<std::string>& shaderTypeTags,
                                                    const std::string& shaderType) const
{
    shaderTypeTags.clear();

    // For 'surface' shaders we will want to allow connections from
    // float-compatible outputs only. All the others will be able to connect to
    // any output
    if (shaderType == "surface")
    {
        shaderTypeTags.push_back("float or rgb or rgba or vector or point or point2");
    }
}

std::string ArnoldRendererInfo::getRegisteredRendererName() const
{
    return "arnold";
}

std::string ArnoldRendererInfo::getRegisteredRendererVersion() const
{
    return "4.2.X.X";
}

bool ArnoldRendererInfo::isNodeTypeSupported(const std::string &nodeType) const
{
    if (nodeType == "ShadingNode" || nodeType == "OutputChannelDefine")
    {
        return true;
    }
    else
    {
        return false;
    }
}

FnAttribute::Attribute ArnoldRendererInfo::_getShaderAttr(const std::string& shaderName,
                                                    const std::string& attrName) const
{
    // Nothing to do (other than making sure that result is empty) if no shader
    // is specified
    if (shaderName.empty())  { return FnAttribute::NullAttribute(); }
    
    // Get the shader's GroupAttribute inside shaderInfoCache
    _cacheShaderInfo();
    FnAttribute::GroupAttribute groupAttr;
    {
        // groupAttr is a GroupAttribute instance rather than a reference
        // so we can release the read lock earlier.
        FnGeolibUtil::AutoReadLock readLock(g_readWriteLock);
        groupAttr = g_cache.getShaderInfo(shaderName);
    }

    if (!groupAttr.isValid())
    {
        std::ostringstream msg;
        msg << "Shader not found: \"" << shaderName << "\"";
        FnLogWarn(msg.str());
        return FnAttribute::NullAttribute();
    }

    // Get the required child Attribute
    FnAttribute::Attribute attr = groupAttr.getChildByName(attrName);
    if (!attr.isValid())
    {
        std::ostringstream msg;
        msg << "No \"" << attrName <<"\" Attribute found for shader \"" << shaderName << "\"";
        FnLogWarn(msg.str());
        return FnAttribute::NullAttribute();
    }

    return attr;
}

void ArnoldRendererInfo::_fillChildInputsOrOutputs(std::vector<std::string>& childInputsOrOutputs,
                                                   const std::string &parameterName,
                                                   const int &type) const
{
    // Add child inputs for certain input/output types. The child input is
    // defined by using the following convention: "name.childName"
    if (type == kFnRendererObjectValueTypeColor3)
    {
        childInputsOrOutputs.push_back(parameterName + ".r");
        childInputsOrOutputs.push_back(parameterName + ".g");
        childInputsOrOutputs.push_back(parameterName + ".b");
    }
    else if (type == kFnRendererObjectValueTypeColor4)
    {
        childInputsOrOutputs.push_back(parameterName + ".r");
        childInputsOrOutputs.push_back(parameterName + ".g");
        childInputsOrOutputs.push_back(parameterName + ".b");
        childInputsOrOutputs.push_back(parameterName + ".a");
    }
    else if (type == kFnRendererObjectValueTypeVector2 ||
             type == kFnRendererObjectValueTypePoint2)
    {
        childInputsOrOutputs.push_back(parameterName + ".x");
        childInputsOrOutputs.push_back(parameterName + ".y");
    }
    else if (type == kFnRendererObjectValueTypeVector3 ||
             type == kFnRendererObjectValueTypePoint3 ||
             type == kFnRendererObjectValueTypeNormal)
    {
        childInputsOrOutputs.push_back(parameterName + ".x");
        childInputsOrOutputs.push_back(parameterName + ".y");
        childInputsOrOutputs.push_back(parameterName + ".z");
    }
    else if (type == kFnRendererObjectValueTypeVector4 ||
             type == kFnRendererObjectValueTypePoint4)
    {
        childInputsOrOutputs.push_back(parameterName + ".x");
        childInputsOrOutputs.push_back(parameterName + ".y");
        childInputsOrOutputs.push_back(parameterName + ".z");
        childInputsOrOutputs.push_back(parameterName + ".w");
    }
    else if (type == kFnRendererObjectValueTypeMatrix)
    {
        for(int i = 0; i < 16; i++)
        {
            std::ostringstream childInput;
            childInput << parameterName << ".i" << i;
            childInputsOrOutputs.push_back(childInput.str());
        }
    }
}


void ArnoldRendererInfo::_fillInputTagsForType(std::vector<std::string>& inputTags,
                                               const int &type) const
{
    // All the types that can be converted between them through an integer value
    if (type == kFnRendererObjectValueTypeByte    ||
        type == kFnRendererObjectValueTypeInt     ||
        type == kFnRendererObjectValueTypeUint    ||
        type == kFnRendererObjectValueTypeBoolean)
    {
        inputTags.push_back("byte or int or uint or boolean");
    }

    // All the types that can be converted between them through a float value
    else if (type == kFnRendererObjectValueTypeFloat   ||
             type == kFnRendererObjectValueTypeColor3  ||
             type == kFnRendererObjectValueTypeColor4  ||
             type == kFnRendererObjectValueTypeVector3 ||
             type == kFnRendererObjectValueTypePoint3)
    {
        inputTags.push_back("float or rgb or rgba or vector or point or point2");
    }

    // The point2 exception
    else if (type == kFnRendererObjectValueTypePoint2)
    {
        inputTags.push_back("float or rgb or rgba or point2");
    }

    else if (type == kFnRendererObjectValueTypeString)
    {
        inputTags.push_back("string");
    }

    else if (type == kFnRendererObjectValueTypeMatrix)
    {
        inputTags.push_back("matrix");
    }

    else if (type == kFnRendererObjectValueTypeEnum)
    {
        inputTags.push_back("enum");
    }

    else if (type == kFnRendererObjectValueTypeNormal)
    {
        inputTags.push_back("normal");
    }

    else if (type == kFnRendererObjectValueTypePointer)
    {
        inputTags.push_back("pointer");
    }

    else if (type == kFnRendererObjectValueTypeShader)
    {
        inputTags.push_back("shader");
    }

    // Currently we do not set any tags for Unknown of Null, which means that
    // anything can be connected to an input of these two types.
}

void ArnoldRendererInfo::_fillOutputTagsForType(std::vector<std::string>& outputTags,
                                                const int &type) const
{
    if (type == kFnRendererObjectValueTypeByte)
    {
        outputTags.push_back("byte");
    }

    else if (type == kFnRendererObjectValueTypeInt)
    {
        outputTags.push_back("int");
    }

    else if (type == kFnRendererObjectValueTypeUint)
    {
        outputTags.push_back("uint");
    }

    else if (type == kFnRendererObjectValueTypeBoolean)
    {
        outputTags.push_back("boolean");
    }

    else if (type == kFnRendererObjectValueTypeFloat)
    {
        outputTags.push_back("float");
    }

    else if (type == kFnRendererObjectValueTypeColor3)
    {
        outputTags.push_back("rgb");
    }

    else if (type == kFnRendererObjectValueTypeColor4)
    {
        outputTags.push_back("rgba");
    }

    else if (type == kFnRendererObjectValueTypeVector3)
    {
        outputTags.push_back("vector");
    }

    else if (type == kFnRendererObjectValueTypePoint2)
    {
        outputTags.push_back("point2");
    }

    else if (type == kFnRendererObjectValueTypePoint3)
    {
        outputTags.push_back("point");
    }

    else if (type == kFnRendererObjectValueTypeString)
    {
        outputTags.push_back("string");
    }

    else if (type == kFnRendererObjectValueTypeMatrix)
    {
        outputTags.push_back("matrix");
    }

    else if (type == kFnRendererObjectValueTypeEnum)
    {
        outputTags.push_back("enum");
    }

    else if (type == kFnRendererObjectValueTypeNormal)
    {
        outputTags.push_back("normal");
    }

    else if (type == kFnRendererObjectValueTypePointer)
    {
        outputTags.push_back("pointer");
    }

    else if (type == kFnRendererObjectValueTypeShader)
    {
        outputTags.push_back("shader");
    }

    else if (type == kFnRendererObjectValueTypeNull)
    {
        outputTags.push_back("null");
    }

    else if (type == kFnRendererObjectValueTypeUnknown)
    {
        outputTags.push_back("unknown");
    }
}

void ArnoldRendererInfo::fillShaderInputNames(std::vector<std::string>& shaderInputNames,
                                              const std::string& shaderName) const
{
    shaderInputNames.clear();

    // Get the params GroupAttribute for this shader
    FnAttribute::GroupAttribute shaderParamsAttr = _getShaderAttr(shaderName, "param");
    if (!shaderParamsAttr.isValid()) { return; }

    // For each parameter add its input and child inputs
    for(int i = 0; i < shaderParamsAttr.getNumberOfChildren(); i++)
    {
        // Get the parameter Group Attribute, name and type
        FnAttribute::GroupAttribute paramAttr = shaderParamsAttr.getChildByIndex(i);
        if (!paramAttr.isValid()) { continue; } // no param GroupAttribute found

        std::string paramName = shaderParamsAttr.getChildName(i);

        FnAttribute::IntAttribute typeAttr = paramAttr.getChildByName("type");
        if(!typeAttr.isValid()) { continue; } // no type defined for this param
        int type = typeAttr.getValue();

        // Add an input for the parameter itself
        shaderInputNames.push_back(paramName);

        // Add the child inputs
        _fillChildInputsOrOutputs (shaderInputNames, paramName, type);
    }
}

void  ArnoldRendererInfo::fillLiveRenderTerminalOps(
        FnKat::RendererInfo::RendererInfoBase::OpDefinitionQueue& terminalOps,
        const FnAttribute::GroupAttribute& stateArgs) const
{
    // Define the terminal ops to be added during live rendering
    {
        // Add a LocalizeAttribute to ensure that renderSettings is always sent
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("attributeName", FnAttribute::StringAttribute("renderSettings"));
        opArgs.set("CEL", FnAttribute::StringAttribute("/root"));
        terminalOps.push_back(std::make_pair("LocalizeAttribute", opArgs.build()));
    }

    {
        FnAttribute::GroupBuilder opArgs;
        terminalOps.push_back(std::make_pair("LiveAttribute", opArgs.build()));
    }

    {
        // Add a material hash since so that both light material and xforms are sent at once
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("type", FnAttribute::StringAttribute("light"));
        terminalOps.push_back(std::make_pair("LocalizeLiveAttributeXform", opArgs.build()));
    }

    {
        // Add a material hash since so that both light material and xforms are sent at once
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("type", FnAttribute::StringAttribute("light"));
        terminalOps.push_back(std::make_pair("LocalizeXform", opArgs.build()));
    }

    {
        // Watch for changes to renderer specific Global Statements
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("location", FnAttribute::StringAttribute("/root"));
        opArgs.set("typeAlias", FnAttribute::StringAttribute("globals"));
        opArgs.set("attributeNames", FnAttribute::StringAttribute("arnoldGlobalStatements"));
        terminalOps.push_back(std::make_pair("LiveRenderFilter", opArgs.build()));
    }

    {
        // Watch for changes to renderer specific Live Render Settings
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("location", FnAttribute::StringAttribute("/root"));
        opArgs.set("typeAlias", FnAttribute::StringAttribute("liveRenderSettings"));
        opArgs.set("attributeNames", FnAttribute::StringAttribute("liveRenderSettings.arnold"));
        terminalOps.push_back(std::make_pair("LiveRenderFilter", opArgs.build()));
    }

    {
        // Watch for changes any locations that are cameras
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("type", FnAttribute::StringAttribute("camera"));
        opArgs.set("location", FnAttribute::StringAttribute("/root/world"));

        std::string attributes[] = {"xform", "geometry"};
        opArgs.set("attributeNames", FnAttribute::StringAttribute(attributes, 2, 1));
        terminalOps.push_back(std::make_pair("LiveRenderFilter", opArgs.build()));
    }

    {
        // Watch for changes any locations that are lights
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("type", FnAttribute::StringAttribute("light"));
        opArgs.set("location", FnAttribute::StringAttribute("/root/world"));
        opArgs.set("collectUpstream", FnAttribute::IntAttribute(1));

        std::string attributes[] = {"xform", "material", "geometry", "mute", "solo"};
        opArgs.set("attributeNames", FnAttribute::StringAttribute(attributes, 5, 1));
        terminalOps.push_back(std::make_pair("LiveRenderFilter", opArgs.build()));
    }

    {
        // Watch for changes to light list attributes on any locations that are not lights
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("type", FnAttribute::StringAttribute(""));
        opArgs.set("location", FnAttribute::StringAttribute("/root/world"));
        opArgs.set("typeAlias", FnAttribute::StringAttribute("lightLink"));
        opArgs.set("exclusions", FnAttribute::StringAttribute("light"));

        terminalOps.push_back(std::make_pair("LocalizeLightListLiveRenderFilter",
                opArgs.build()));
    }

    {
        // Watch for changes to material attributes on any locations that are not lights
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("type", FnAttribute::StringAttribute(""));
        opArgs.set("location", FnAttribute::StringAttribute("/root"));
        opArgs.set("typeAlias", FnAttribute::StringAttribute("geoMaterial"));
        opArgs.set("exclusions", FnAttribute::StringAttribute("light"));
        opArgs.set("collectUpstream", FnAttribute::IntAttribute(1));

        std::string attributes[] = {"info", "material", "geometry.faces",};
        opArgs.set("attributeNames", FnAttribute::StringAttribute(attributes, 3, 1));
        terminalOps.push_back(std::make_pair("LiveRenderFilter", opArgs.build()));
    }

    {
        // Watch for changes to the xform attribute on any locations that are not lights
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("location", FnAttribute::StringAttribute("/root"));
        opArgs.set("typeAlias", FnAttribute::StringAttribute("geoXform"));

        std::string attributes[] = {"light", "camera"};
        opArgs.set("exclusions", FnAttribute::StringAttribute(attributes, 2, 1));

        opArgs.set("attributeNames", FnAttribute::StringAttribute("xform"));
        terminalOps.push_back(std::make_pair("LiveRenderFilter", opArgs.build()));
    }

    {
        // Watch /root for changes to renderSettings
        FnAttribute::GroupBuilder opArgs;
        opArgs.set("location", FnAttribute::StringAttribute("/root"));
        opArgs.set("typeAlias", FnAttribute::StringAttribute("renderSettings"));
        opArgs.set("attributeNames", FnAttribute::StringAttribute("renderSettings"));
        terminalOps.push_back(std::make_pair("LiveRenderFilter", opArgs.build()));
    }
}


void ArnoldRendererInfo::fillShaderInputTags(std::vector<std::string>& shaderInputTags,
                                             const std::string& shaderName,
                                             const std::string& inputName) const
{
    shaderInputTags.clear();

    // Get the params GroupAttribute for this shader
    FnAttribute::GroupAttribute shaderParamsAttr = _getShaderAttr(shaderName, "param");
    if (!shaderParamsAttr.isValid()) { return; }

    // Get the input type
    int type = kFnRendererObjectValueTypeNull;

    // All the components are assumed to be float - compatible
    if (inputName.find('.') != std::string::npos)
    {
        shaderInputTags.push_back("float or rgb or rgba or vector or point or point2");
        return;
    }

    // Otherwise we will use the correspondent parameter's type
    FnAttribute::GroupAttribute paramAttr = shaderParamsAttr.getChildByName(inputName);
    if (paramAttr.isValid())
    {
        FnAttribute::IntAttribute paramTypeAttr = paramAttr.getChildByName("type");
        if (paramTypeAttr.isValid())
        {
            type = paramTypeAttr.getValue();
        }
    }

    // Add the input tags for this type
    _fillInputTagsForType(shaderInputTags, type);
}

void ArnoldRendererInfo::fillShaderOutputNames(std::vector<std::string>& shaderOutputNames,
                                               const std::string& shaderName) const
{
    shaderOutputNames.clear();

    // Get the outputType Attribute for this shader and get the type
    int type = kFnRendererObjectValueTypeUnknown;
    FnAttribute::IntAttribute outputTypeAttr = _getShaderAttr(shaderName, "outputType");
    if (outputTypeAttr.isValid())
    {
        type = outputTypeAttr.getValue();
    }

    // There will always be an "out" output
    shaderOutputNames.push_back("out");

    // Add the child outputs
    _fillChildInputsOrOutputs (shaderOutputNames, "out", type);
}

void ArnoldRendererInfo::fillShaderOutputTags(std::vector<std::string>& shaderOutputTags,
                                              const std::string& shaderName,
                                              const std::string& outputName) const
{
    shaderOutputTags.clear();

    // All components will be of the type float
    if (outputName.find('.') != std::string::npos)
    {
        shaderOutputTags.push_back("float");
        return;
    }

    // Get the outputType Attribute for this shader and get the type
    FnAttribute::IntAttribute outputTypeAttr = _getShaderAttr(shaderName, "outputType");
    if (!outputTypeAttr.isValid()) { return; }

    int type = outputTypeAttr.getValue();

    // Add the input tags for this type
    _fillOutputTagsForType(shaderOutputTags, type);
}

bool ArnoldRendererInfo::buildRendererObjectInfo(FnAttribute::GroupBuilder& rendererObjectInfo,
                                                 const std::string& name,
                                                 const std::string& type,
                                                 const FnAttribute::GroupAttribute inputAttr /*= 0x0*/) const
{
    if (type == kFnRendererObjectTypeOutputChannel)
    {
        return _buildOutputChannel(rendererObjectInfo, name);
    }
    else if (type == kFnRendererObjectTypeOutputChannelAttrHints)
    {
        return _buildOutputChannelAttrHints(rendererObjectInfo, name);
    }
    else if (type == kFnRendererObjectTypeRenderOutput)
    {
        // Check for valid input attributes before using them
        if (!inputAttr.isValid())
        {
            // Invalid inputAttr
            const char warning_message[] = "Invalid input attribute passed to buildRendererObjectInfo";
            FnLogWarn(warning_message);

            return false;
        }

        return _buildRenderOutput(rendererObjectInfo, name, inputAttr);
    }
    else
    {
        _cacheShaderInfo();
        FnAttribute::GroupAttribute groupAttr;
        {
            // groupAttr is a GroupAttribute instance rather than a reference
            // so we can release the read lock earlier.
            FnGeolibUtil::AutoReadLock readLock(g_readWriteLock);
            groupAttr = g_cache.getShaderInfo(name);
        }

        // Set up mapping of shader parameter meta names to actual attribute names
        setShaderParameterMapping(rendererObjectInfo, "shader",
                                  "arnoldLightShader");
        const std::string parameterNamePrefix = "arnoldLightParams.";

        if (groupAttr.isValid())
        {
            rendererObjectInfo.update(groupAttr);

            // Set up mapping of shader parameter meta names to actual
            // attribute names
            if (groupAttr.getChildByName("param.color").isValid())
            {
                setShaderParameterMapping(rendererObjectInfo, "color",
                                          parameterNamePrefix + "color");
            }
            if (groupAttr.getChildByName("param.intensity").isValid())
            {
                setShaderParameterMapping(rendererObjectInfo, "intensity",
                                          parameterNamePrefix + "intensity");
            }
            if (groupAttr.getChildByName("param.exposure").isValid())
            {
                setShaderParameterMapping(rendererObjectInfo, "exposure",
                                          parameterNamePrefix + "exposure");
            }
            if (groupAttr.getChildByName("param.cone_angle").isValid())
            {
                setShaderParameterMapping(rendererObjectInfo, "outerConeAngle",
                                          parameterNamePrefix + "cone_angle");
            }
            if (groupAttr.getChildByName("param.penumbra_angle").isValid())
            {
                setShaderParameterMapping(rendererObjectInfo, "innerConeAngle",
                                          parameterNamePrefix
                                          + "penumbra_angle");
            }

            if(name == "driver_exr")
            {
              //check to see if param.compression is present before setting
              //this. Otherwise, it will throw an exception when later
              //queried
              if (groupAttr.getChildByName("param.compression").isValid())
              {
                  std::string helpStrCompression =
                      "<p>The following compression settings are valid:</p>"
                      "<ul>"
                      " <li> 0 = none </li>"
                      " <li> 1 = rle </li>"
                      " <li> 2 = zips </li>"
                      " <li> 3 = zip (default) </li>"
                      " <li> 4 = piz </li>"
                      " <li> 5 = pxr24 </li>"
                      "</ul>"
                      "<p>For further information please consult the Arnold documentation."
                      "</p>";

                  rendererObjectInfo.set("param.compression.hints.help", FnAttribute::StringAttribute(helpStrCompression));
              }

              if (groupAttr.getChildByName("param.autocrop").isValid())
              {
                std::string helpStrAutoCrop =
                      "<p>Embed the DataWindow field in the EXR header.</p> <p>This "
                      "DataWindow, or ROI in Nuke, stores a tight bounding box around"
                      " non-empty pixels in the rendered image which can greatly"
                      "accelerate post-processing.</p>"
                      ""
                      "<p><b>Note: autocrop is only supported for non-tiled images"
                      "</b></p>";

                rendererObjectInfo.set("param.autocrop.hints.help", FnAttribute::StringAttribute(helpStrAutoCrop));
              }
            }

            if(name == "driver_exr" || name == "driver_tiff")
            {
              if (groupAttr.getChildByName("param.append").isValid())
              {
                std::string helpStrAppendMode =
                  "<p>If set, Arnold will preserve previously rendered tiles "
                  "and only work on the missing ones, appending them to the output "
                  "file.</p>"
                  "<p>If no image is present the render will proceed as normal "
                  "creating a new image.</p> <p>If the image specifications do not"
                  " match the render will be aborted.</p>";
              
                rendererObjectInfo.set("param.append.hints.help", FnAttribute::StringAttribute(helpStrAppendMode));
              }
            }

            return true;
        }
    }

    return false;
}

void ArnoldRendererInfo::_configureBasicRenderObjectInfo(FnAttribute::GroupBuilder &gb,
                                                         const std::string &name,
                                                         const std::string &type) const
{
    std::vector<std::string> typeTags;
    std::string location = name;
    std::string fullPath = "";
    FnAttribute::Attribute containerHintsAttr;

    configureBasicRenderObjectInfo(gb,
                                   type,                                                       /* type      */
                                   std::vector<std::string>(typeTags.begin(), typeTags.end()), /* type tags */
                                   location,                                                   /* location  */
                                   fullPath,                                                   /* full path */
                                   kFnRendererObjectValueTypeUnknown,                          /* outputType (unsupported) */
                                   containerHintsAttr);
}

bool ArnoldRendererInfo::_buildOutputChannel(FnAttribute::GroupBuilder &gb,
                                             const std::string &name) const
{
    _configureBasicRenderObjectInfo(gb, name, kFnRendererObjectTypeOutputChannel);

    std::string attrName;
    int arraySize = 0;
    int paramType;
    EnumPairVector enums;

    // driver
    attrName = "driver";
    paramType= kFnRendererObjectValueTypeString;
    FnAttribute::GroupBuilder driverGroupBuilder;
    driverGroupBuilder.set("widget", FnAttribute::StringAttribute("shaderPopup"));
    driverGroupBuilder.set("dynamicParameters", FnAttribute::StringAttribute("driverParameters"));
    driverGroupBuilder.set("dynamicParametersType", FnAttribute::StringAttribute(kFnRendererObjectTypeDriver));
    driverGroupBuilder.set("shaderType", FnAttribute::StringAttribute(kFnRendererObjectTypeDriver));
    driverGroupBuilder.set("rendererInfoPluginName", FnAttribute::StringAttribute("ArnoldRendererInfo"));
    driverGroupBuilder.set("hideProducts", FnAttribute::StringAttribute("True"));
    driverGroupBuilder.set("flat", FnAttribute::StringAttribute("False"));
    addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute("driver_exr"),
            driverGroupBuilder.build(), enums);

    // channel
    attrName = "channel";
    paramType = kFnRendererObjectValueTypeString;
    FnAttribute::GroupBuilder channelGroupBuilder;
    channelGroupBuilder.set("conditionalVisOp", FnAttribute::StringAttribute("and"));
    channelGroupBuilder.set("conditionalVisLeft", FnAttribute::StringAttribute("conditionalVis1"));
    channelGroupBuilder.set("conditionalVisRight", FnAttribute::StringAttribute("conditionalVis2"));
    channelGroupBuilder.set("conditionalVis1Op", FnAttribute::StringAttribute("notEqualTo"));
    channelGroupBuilder.set("conditionalVis1Path", FnAttribute::StringAttribute("../driver"));
    channelGroupBuilder.set("conditionalVis1Value", FnAttribute::StringAttribute("driver_dtex"));
    channelGroupBuilder.set("conditionalVis2Op", FnAttribute::StringAttribute("notEqualTo"));
    channelGroupBuilder.set("conditionalVis2Path", FnAttribute::StringAttribute("../driver"));
    channelGroupBuilder.set("conditionalVis2Value", FnAttribute::StringAttribute("driver_vshd"));
    addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute("RGBA"),
            channelGroupBuilder.build(), enums);

    // type
    attrName = "type";
    paramType = kFnRendererObjectValueTypeString;
    FnAttribute::GroupBuilder typeGroupBuilder;
    const char *typeOptions[] = {"BYTE", "INT", "LONG", "BOOL", "FLOAT",
            "DOUBLE", "RGBA", "ABSRGB", "VECTOR", "POINT", "POINT2",
            "STRING", "POINTER", "ARRAY", "MATRIX", "ENUM"};
    typeGroupBuilder.set("options", FnAttribute::StringAttribute(typeOptions, 16, 1));
    typeGroupBuilder.set("widget", FnAttribute::StringAttribute("popup"));
    addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute("RGBA"),
            typeGroupBuilder.build(), enums);

    // filter
    attrName = "filter";
    paramType = kFnRendererObjectValueTypeString;
    FnAttribute::GroupBuilder filterGroupBuilder;
    filterGroupBuilder.set("widget", FnAttribute::StringAttribute("shaderPopup"));
    filterGroupBuilder.set("dynamicParameters", FnAttribute::StringAttribute("filterParameters"));
    filterGroupBuilder.set("dynamicParametersType", FnAttribute::StringAttribute(kFnRendererObjectTypeFilter));
    filterGroupBuilder.set("shaderType", FnAttribute::StringAttribute(kFnRendererObjectTypeFilter));
    filterGroupBuilder.set("rendererInfoPluginName", FnAttribute::StringAttribute("ArnoldRendererInfo"));
    filterGroupBuilder.set("options", FnAttribute::StringAttribute("<inherit>"));
    filterGroupBuilder.set("hideProducts", FnAttribute::StringAttribute("True"));
    filterGroupBuilder.set("flat", FnAttribute::StringAttribute("False"));
    filterGroupBuilder.set("conditionalVisOp", FnAttribute::StringAttribute("and"));
    filterGroupBuilder.set("conditionalVisLeft", FnAttribute::StringAttribute("conditionalVis1"));
    filterGroupBuilder.set("conditionalVisRight", FnAttribute::StringAttribute("conditionalVis2"));
    filterGroupBuilder.set("conditionalVis1Op", FnAttribute::StringAttribute("notEqualTo"));
    filterGroupBuilder.set("conditionalVis1Path", FnAttribute::StringAttribute("../driver"));
    filterGroupBuilder.set("conditionalVis1Value", FnAttribute::StringAttribute("driver_dtex"));
    filterGroupBuilder.set("conditionalVis2Op", FnAttribute::StringAttribute("notEqualTo"));
    filterGroupBuilder.set("conditionalVis2Path", FnAttribute::StringAttribute("../driver"));
    filterGroupBuilder.set("conditionalVis2Value", FnAttribute::StringAttribute("driver_vshd"));
    addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute("gaussian_filter"),
            filterGroupBuilder.build(), enums);

    // driverParameters
    attrName = "driverParameters";
    paramType = kFnRendererObjectValueTypeString;
    FnAttribute::GroupBuilder driverParametersAttrBuilder;
    FnAttribute::GroupBuilder driverParametersGroupBuilder;
    driverParametersGroupBuilder.set("open", FnAttribute::StringAttribute("True"));
    addRenderObjectParam(gb, attrName, paramType, arraySize, driverParametersAttrBuilder.build(),
            driverParametersGroupBuilder.build(), enums);

    // filterParameters
    attrName = "filterParameters";
    paramType = kFnRendererObjectValueTypeString;
    FnAttribute::GroupBuilder filterParametersAttrBuilder;
    FnAttribute::GroupBuilder filterParametersGroupBuilder;
    filterParametersGroupBuilder.set("open", FnAttribute::StringAttribute("True"));
    filterParametersGroupBuilder.set("conditionalVisOp", FnAttribute::StringAttribute("and"));
    filterParametersGroupBuilder.set("conditionalVisLeft", FnAttribute::StringAttribute("conditionalVis1"));
    filterParametersGroupBuilder.set("conditionalVisRight", FnAttribute::StringAttribute("conditionalVis2"));
    filterParametersGroupBuilder.set("conditionalVis1Op", FnAttribute::StringAttribute("notEqualTo"));
    filterParametersGroupBuilder.set("conditionalVis1Path", FnAttribute::StringAttribute("../driver"));
    filterParametersGroupBuilder.set("conditionalVis1Value", FnAttribute::StringAttribute("driver_dtex"));
    filterParametersGroupBuilder.set("conditionalVis2Op", FnAttribute::StringAttribute("notEqualTo"));
    filterParametersGroupBuilder.set("conditionalVis2Path", FnAttribute::StringAttribute("../driver"));
    filterParametersGroupBuilder.set("conditionalVis2Value", FnAttribute::StringAttribute("driver_vshd"));
    addRenderObjectParam(gb, attrName, paramType, arraySize, filterParametersAttrBuilder.build(),
            filterParametersGroupBuilder.build(), enums);

    // lightExpressions
    attrName = "lightExpressions";
    paramType= kFnRendererObjectValueTypeString;
    FnAttribute::GroupBuilder lightExpressionsGroupBuilder;
    lightExpressionsGroupBuilder.set("widget", FnAttribute::StringAttribute("sortableArray"));
    addRenderObjectParam(gb, attrName, paramType, 0, FnAttribute::StringAttribute(""),
            lightExpressionsGroupBuilder.build(), enums);

    return true;
}

bool ArnoldRendererInfo::_buildOutputChannelAttrHints(FnAttribute::GroupBuilder &gb,
                                                      const std::string &name) const
{
    _configureBasicRenderObjectInfo(gb, name, kFnRendererObjectTypeOutputChannelAttrHints);

    std::string attrName;
    int arraySize = 0;
    int paramType;
    EnumPairVector enums;

    // outputChannels
    attrName = "outputChannels";
    paramType= kFnRendererObjectValueTypeString;
    FnAttribute::GroupBuilder outputChannelsGroupBuilder;
    outputChannelsGroupBuilder.set("closed", FnAttribute::StringAttribute("True"));
    outputChannelsGroupBuilder.set("page", FnAttribute::StringAttribute("Channel Definitions"));
    addRenderObjectParam(gb, attrName, paramType, arraySize, FnAttribute::StringAttribute(""),
            outputChannelsGroupBuilder.build(), enums);

    return true;
}

bool ArnoldRendererInfo::_buildRenderOutput(FnAttribute::GroupBuilder &gb,
                                            const std::string &name,
                                            const FnAttribute::GroupAttribute inputAttr = FnAttribute::GroupAttribute()) const
{
    _configureBasicRenderObjectInfo(gb, name, kFnRendererObjectTypeRenderOutput);

    FnAttribute::GroupAttribute globalStatementsAttr = inputAttr.getChildByName("arnoldGlobalStatements");
    std::string attrName;
    std::string helpStr;
    int arraySize = 0;
    int paramType;
    EnumPairVector enums;

    if(name == kFnRendererOutputTypeRaw)
    {
        attrName = "tempRenderLocation";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder tempRenderLocationGroupBuilder;
        helpStr = "<p><b>For normal usage, this field should be left blank.</b></p>"
                "<p>In the rare occasion that you need to specify the temp file "
                "path that the renderer will initially write, enter it here. "
                "This file path is only relevant during the render process. "
                "When the render is complete, Katana converts or copies the temp "
                "file to its final location and removes the temp file.</p>"
                "<p>Use <code>$KATANA_TMPDIR</code> for a session-specific local "
                "directory.</p>"
                "<p>Example: <code>$KATANA_TMPDIR/my_unique_filename.#.exr</code></p>";
        tempRenderLocationGroupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
        addRenderObjectParam(gb, attrName, paramType, arraySize, FnAttribute::StringAttribute(""),
                tempRenderLocationGroupBuilder.build(), enums);
    }

    if (name == kFnRendererOutputTypeColor || name == kFnRendererOutputTypeRaw)
    {
        std::vector<std::string> outputChannels;
        outputChannels.push_back("rgba");
        FnAttribute::GroupAttribute outputGroup = globalStatementsAttr.getChildByName("outputChannels");
        if (outputGroup.isValid())
        {
            for (int i=0; i < outputGroup.getNumberOfChildren(); ++i)
            {
                // We want to get the name from the nested StringAttibute
                // "name", this can correctly contain symbols and punctuation.
                FnAttribute::GroupAttribute child =
                        outputGroup.getChildByIndex(i);

                // If the "name" attribute is not there we can still try to
                // use the groupAttr name.
                FnAttribute::StringAttribute channelNameAttr =
                        child.getChildByName("name");
                const std::string defaultChannelName =
                        outputGroup.getChildName(i);

                outputChannels.push_back(
                    channelNameAttr.getValue(defaultChannelName, false));
            }
        }

        attrName = "channel";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder channelGroupBuilder;
        channelGroupBuilder.set("widget", FnAttribute::StringAttribute("popup"));
        channelGroupBuilder.set("options", FnAttribute::StringAttribute(outputChannels));

        addRenderObjectParam(gb, attrName, paramType, arraySize,
                FnAttribute::StringAttribute("rgba"),
                channelGroupBuilder.build(), enums);
    }

    if (name == kFnRendererOutputTypeColor)
    {
        attrName = "isolateLight";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder isolateLightGroupBuilder;
        helpStr = "Add scenegraph locations of lights and/or the GI background material "
                "(from ArnoldGlobalSettings.background).  Resulting image is the "
                "standard output for the selected display channel with only the "
                "specified lights active.";
        isolateLightGroupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
        isolateLightGroupBuilder.set("widget", FnAttribute::StringAttribute("sortableArray"));
        isolateLightGroupBuilder.set("open", FnAttribute::StringAttribute("False"));
        isolateLightGroupBuilder.set("conditionalVisOp", FnAttribute::StringAttribute("equalTo"));
        isolateLightGroupBuilder.set("conditionalVisPath", FnAttribute::StringAttribute("../lightAgnostic"));
        isolateLightGroupBuilder.set("conditionalVisValue", FnAttribute::StringAttribute("0"));

        FnAttribute::GroupBuilder childHintsGroupBuilder;
        childHintsGroupBuilder.set("widget", FnAttribute::StringAttribute("scenegraphLocation"));
        isolateLightGroupBuilder.set("childHints", childHintsGroupBuilder.build());
        addRenderObjectParam(gb, attrName, paramType, arraySize,
                FnAttribute::StringAttribute(""),
                isolateLightGroupBuilder.build(), enums);


        attrName = "lightAgnostic";
        paramType = kFnRendererObjectValueTypeInt;
        FnAttribute::IntBuilder lightAgnosticAttrBuilder;
        lightAgnosticAttrBuilder.push_back(0);
        FnAttribute::GroupBuilder lightAgnosticGroupBuilder;
        helpStr = "Set this to <i>Yes</i> if this output does not depend on which lights "
                "are active.  If set to <i>Yes</i>, this pass will render with the first "
                "available Arnold pass that has a matching render camera, regardless of "
                "its <i>isolateLight</i> settings.";
        lightAgnosticGroupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
        lightAgnosticGroupBuilder.set("widget", FnAttribute::StringAttribute("boolean"));
        addRenderObjectParam(gb, attrName, paramType, arraySize, lightAgnosticAttrBuilder.build(),
                lightAgnosticGroupBuilder.build(), enums);


        attrName = "cameraName";
        paramType = kFnRendererObjectValueTypeString;
        FnAttribute::GroupBuilder cameraNameGroupBuilder;
        helpStr = "Scenegraph location of camera to render from.  If empty, render from camera "
                "specified at <i>renderSettings.cameraName</i> on <i>/root</i>.</p>";
        cameraNameGroupBuilder.set("help", FnAttribute::StringAttribute(helpStr));
        cameraNameGroupBuilder.set("widget", FnAttribute::StringAttribute("scenegraphLocation"));
        addRenderObjectParam(gb, attrName, paramType, arraySize,
            FnAttribute::StringAttribute(""),
            cameraNameGroupBuilder.build(), enums);

    }

    return true;
}

void ArnoldRendererInfo::initialiseCaches()
{
}

void ArnoldRendererInfo::flushCaches()
{
    FnGeolibUtil::AutoWriteLock writeLock(g_readWriteLock);
    g_cache.flush();
}

//we need to load shader dso's into Arnold before we can interrogate them
void ArnoldRendererInfo::_cacheShaderInfo() const
{
    // scope read lock to avoid interfering with write lock below
    {
        FnGeolibUtil::AutoReadLock readLock(g_readWriteLock);
        if (!g_cache.isEmpty()) return;
    }

    std::string pluginPath = getPluginPath();
    std::string dumpExe = pluginPath + "/../RenderBin/ArnoldRendererInfoDump";

    std::string tmpDir = getTmpPath();
    if (tmpDir == "")
    {
        FnLogError("Error running child Arnold info process, no tmp directory specified.");
        return;
    }
    std::string tmpFile = std::string(tmpDir) + "/ArnoldRendererInfoDump.txt";

    // If ARNOLD_LD_LIBRARY_PATH is set, then make sure it is passed to the
    // ArnoldRendererInfoDump executable.
    char * ldPath = ::getenv("LD_LIBRARY_PATH");
    char * arnoldLdPath = ::getenv("ARNOLD_LD_LIBRARY_PATH");

    std::ostringstream cmd;

    // Add KATANA_ROOT to the environment of the child Process
    std::string katanaRoot = getKatanaPath();
    cmd << "env KATANA_ROOT=" << katanaRoot << " ";

    // Add LD_LIBRARY_PATH to the environment of the child Process
    cmd << "LD_LIBRARY_PATH=";
    if(arnoldLdPath != 0x0)
    {
        cmd << arnoldLdPath << ":";
    }
    if(ldPath != 0x0)
    {
        cmd << ldPath << ":";
    }
    cmd << katanaRoot << "/bin";

    cmd << " " << dumpExe << " " << tmpFile << " " << pluginPath;

    if (::system(cmd.str().c_str()) != 0)
    {
        FnLogError("Error running child Arnold info process, no shader info can be loaded.");
        return;
    }

    std::ifstream infile(tmpFile.c_str());
    if (!infile)
    {
        FnLogError("Error loading output file from child Arnold info process, no shader info can be loaded.");
        return;
    }

    std::ostringstream xmlStream;
    xmlStream << infile.rdbuf();
    infile.close();

    ::unlink(tmpFile.c_str());
    
    FnAttribute::GroupAttribute groupAttr;
    try
    {
        groupAttr = FnAttribute::Attribute::parseXML(xmlStream.str().c_str());
    }
    catch (std::exception& e)
    {
        FnLogError(std::string("Error parsing XML output from Arnold info process: ") + e.what());
    }
    catch (...)
    {
        // empty
    }
    
    if (!groupAttr.isValid())
    {
        FnLogError("Error parsing output file from Arnold info process, no shader info can be loaded.");
        return;
    }

    // scope write lock to just the loop that sets shader info
    {
        FnGeolibUtil::AutoWriteLock writeLock(g_readWriteLock);

        for (int i=0; i<groupAttr.getNumberOfChildren(); ++i)
        {
            g_cache.addShaderInfo(groupAttr.getChildName(i), groupAttr.getChildByIndex(i));
        }
    }
}
