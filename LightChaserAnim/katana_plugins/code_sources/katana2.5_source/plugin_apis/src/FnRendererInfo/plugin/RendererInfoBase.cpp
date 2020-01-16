// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRendererInfo/plugin/RendererInfoBase.h"

#include <iostream>
#include <sstream>

#include "FnAsset/FnDefaultAssetPlugin.h"
#include "FnAsset/FnDefaultFileSequencePlugin.h"
#include "FnAttribute/FnDataBuilder.h"
#include "FnAttribute/FnGroupBuilder.h"
#include "FnLogging/FnLogging.h"
#include "FnPluginManager/FnPluginManager.h"
#include "FnRender/suite/FnRenderServicesSuite.h"
#include "FnRendererInfo/plugin/RenderMethod.h"

namespace FnKat = Foundry::Katana;

namespace  // anonymous
{
const FnRenderServicesSuite_v1* getRenderServicesSuite()
{
    static FnPluginManager::LazyHostSuite<FnRenderServicesSuite_v1> suite = {
        "RenderServices", 1
    };
    return suite.get();
}

void destroy(RendererInfoPluginHandle handle)
{
    delete handle;
}

FnAttributeHandle getRenderMethods(RendererInfoPluginHandle handle)
{
    return handle->getRendererInfoPlugin()._getRenderMethods().getRetainedHandle();
}

FnAttributeHandle getBatchRenderMethod(RendererInfoPluginHandle handle)
{
    return handle->getRendererInfoPlugin()._getBatchRenderMethod().getRetainedHandle();
}

FnAttributeHandle getRendererObjectNames(RendererInfoPluginHandle handle, const char *type,
    FnAttributeHandle typeTags)
{
    std::vector<std::string> typeTagsVec;
    FnAttribute::StringAttribute strTypeTags =
        FnAttribute::Attribute::CreateAndRetain(typeTags);
    if (strTypeTags.isValid())
    {
        FnAttribute::StringConstVector otherTypeTagsVec = strTypeTags.getNearestSample(0.0);
        typeTagsVec.insert(typeTagsVec.end(), otherTypeTagsVec.begin(), otherTypeTagsVec.end());
    }

  return handle->getRendererInfoPlugin()._getRendererObjectNames(type, typeTagsVec).getRetainedHandle();
}

FnAttributeHandle getRendererObjectTypes(RendererInfoPluginHandle handle, const char *type)
{
    return handle->getRendererInfoPlugin()._getRendererObjectTypes(type).getRetainedHandle();
}

FnAttributeHandle getRendererShaderTypeTags(RendererInfoPluginHandle handle, const char *shaderType)
{
    return handle->getRendererInfoPlugin()._getRendererShaderTypeTags(shaderType).getRetainedHandle();
}

FnAttributeHandle getRendererCoshaderType(RendererInfoPluginHandle handle)
{
    return handle->getRendererInfoPlugin()._getRendererCoshaderType().getRetainedHandle();
}

FnAttributeHandle getRegisteredRendererName(RendererInfoPluginHandle handle)
{
    return handle->getRendererInfoPlugin()._getRegisteredRendererName().getRetainedHandle();
}

FnAttributeHandle getRegisteredRendererVersion(RendererInfoPluginHandle handle)
{
    return handle->getRendererInfoPlugin()._getRegisteredRendererVersion().getRetainedHandle();
}

bool isPresetLocalFileNeeded(RendererInfoPluginHandle handle, const char *outputType)
{
    return handle->getRendererInfoPlugin().isPresetLocalFileNeeded(outputType);
}

bool isPolymeshFacesetSplittingEnabled(RendererInfoPluginHandle handle)
{
    return handle->getRendererInfoPlugin().isPolymeshFacesetSplittingEnabled();
}

bool isNodeTypeSupported(RendererInfoPluginHandle handle, const char *nodeType)
{
    return handle->getRendererInfoPlugin().isNodeTypeSupported(nodeType);
}

FnAttributeHandle getRendererObjectDefaultType(RendererInfoPluginHandle handle, const char *type)
{
    return handle->getRendererInfoPlugin()._getRendererObjectDefaultType(type).getRetainedHandle();
}

FnAttributeHandle getShaderInputNames(RendererInfoPluginHandle handle, const char *shader)
{
    return handle->getRendererInfoPlugin()._getShaderInputNames(shader).getRetainedHandle();
}

FnAttributeHandle getShaderInputTags(RendererInfoPluginHandle handle, const char *shader, const char *inputName)
{
    return handle->getRendererInfoPlugin()._getShaderInputTags(shader, inputName).getRetainedHandle();
}

FnAttributeHandle getShaderOutputNames(RendererInfoPluginHandle handle, const char *shader)
{
    return handle->getRendererInfoPlugin()._getShaderOutputNames(shader).getRetainedHandle();
}

FnAttributeHandle getShaderOutputTags(RendererInfoPluginHandle handle, const char *shader, const char *outputName)
{
    return handle->getRendererInfoPlugin()._getShaderOutputTags(shader, outputName).getRetainedHandle();
}

FnAttributeHandle getLiveRenderControlModules(RendererInfoPluginHandle handle)
{
    return NULL;
}

FnAttributeHandle getRendererObjectInfo(RendererInfoPluginHandle handle, const char *name,
  const char *type, const FnAttributeHandle inputData)
{
    return handle->getRendererInfoPlugin()._getRendererObjectInfo(
        name, type, inputData).getRetainedHandle();
}

void setTypeTagNameFilter(RendererInfoPluginHandle handle, const char *filter, const char *typeTag)
{
    handle->getRendererInfoPlugin()._setTypeTagNameFilter(filter, typeTag);
}

void setPluginPath(RendererInfoPluginHandle handle, const char *pluginPath)
{
    handle->getRendererInfoPlugin().setPluginPath(pluginPath);
}

void setPluginRootPath(RendererInfoPluginHandle handle, const char *pluginRootPath)
{
    handle->getRendererInfoPlugin().setPluginRootPath(pluginRootPath);
}

void setKatanaPath(RendererInfoPluginHandle handle, const char *katanaPath)
{
    handle->getRendererInfoPlugin().setKatanaPath(katanaPath);
}

void setTmpPath(RendererInfoPluginHandle handle, const char *tmpPath)
{
    handle->getRendererInfoPlugin().setTmpPath(tmpPath);
}

void addObjectLocation(RendererInfoPluginHandle handle, const char *type, const char *location)
{
    handle->getRendererInfoPlugin()._addObjectLocation(type, location);
}

void clearObjectLocations(RendererInfoPluginHandle handle, const char *type)
{
    std::string typeStr;
    if (type)
    {
        typeStr = type;
    }
    handle->getRendererInfoPlugin()._clearObjectLocations(typeStr);
}

/**
 * This method is deprecated and will be removed in a future version.
 * Plugin writers are advised to ensure that their code does not rely on this
 * method being called.
 */
void initialiseCaches(RendererInfoPluginHandle handle)
{
    // Deprecated.
}

void flushCaches(RendererInfoPluginHandle handle)
{
    handle->getRendererInfoPlugin().flushCaches();
    handle->getRendererInfoPlugin()._flushCaches();
}


FnAttributeHandle getLiveRenderTerminalOps(RendererInfoPluginHandle handle,
        const FnAttributeHandle stateArgs)
{
    return handle->getRendererInfoPlugin()._getLiveRenderTerminalOps(stateArgs).getRetainedHandle();
}

FnAttributeHandle getRenderTerminalOps(RendererInfoPluginHandle handle,
        const FnAttributeHandle stateArgs)
{
    return handle->getRendererInfoPlugin()._getRenderTerminalOps(stateArgs).getRetainedHandle();
}


} // namespace anonymous

namespace Foundry
{
namespace Katana
{
namespace RendererInfo
{

namespace
{
    std::string makeTypeKey(const std::string& type, const std::vector<std::string>& typeTags)
    {
        std::string typeKey = type + ":";
        for (std::vector<std::string>::const_iterator iter=typeTags.begin();
            iter!=typeTags.end(); ++iter)
        {
            typeKey += (*iter) + ",";
        }
        return typeKey;
    }

    std::string makeKey(const std::vector<std::string>& components)
    {
        std::string key = "";
        for (std::vector<std::string>::const_iterator iter=components.begin();
            iter!=components.end(); ++iter)
        {
            key += (*iter) + ":";
        }
        return key;
    }

    // Creates a key for an AttributeMap by combining a shader name with an
    // shader input or output name. This will uniquely identify a specific
    // input or output on a specific shader.
    std::string makeShaderInputOrOutputTagsKey(const std::string& shaderName,
        const std::string& inputOrOutput)
    {
        std::ostringstream key;
        key << inputOrOutput << "@" << shaderName;
        return key.str();
    }
}

RendererInfoBase::RendererInfoBase() : _liveRenderControlModulesCached(false)
{

}

FnAttribute::Attribute RendererInfoBase::_getRenderMethods()
{
    std::vector<RendererInfo::RenderMethod*> renderMethods;
    fillRenderMethods(renderMethods);

    // Compile the returned render methods into a group geter
    FnAttribute::GroupBuilder supportedRenderMethodsBuilder;
    for(unsigned int i = 0; i < renderMethods.size(); ++i)
    {
        std::ostringstream nameBuffer;
        nameBuffer << "mode_" << i;
        FnAttribute::GroupBuilder modeBuilder;
        renderMethods[i]->buildAttribute(modeBuilder);
        supportedRenderMethodsBuilder.set(nameBuffer.str(), modeBuilder.build());
    }

    return supportedRenderMethodsBuilder.build();
}

FnAttribute::Attribute RendererInfoBase::_getBatchRenderMethod()
{
    RendererInfo::DiskRenderMethod batchMethod;
    batchMethod.setName("batchRender");
    batchMethod.setCreateCatalogItem(false);
    configureBatchRenderMethod(batchMethod);
    FnAttribute::GroupBuilder modeBuilder;
    batchMethod.buildAttribute(modeBuilder);
    return modeBuilder.build();
}

FnAttribute::Attribute RendererInfoBase::_getRendererObjectNames(const std::string& type,
                                                    const std::vector<std::string>& typeTags)
{
    std::string typeKey = makeTypeKey(type, typeTags);
    AttributeMap::const_iterator iter = _objectNames.find(typeKey);
    if (iter == _objectNames.end())
    {
        std::vector<std::string> names;
        fillRendererObjectNames(names, type, typeTags);

        FnAttribute::StringBuilder sb;
        sb.set(names);
        iter = _objectNames.insert(AttributeMap::value_type(typeKey, sb.build())).first;
    }

    return (*iter).second;
}

FnAttribute::Attribute RendererInfoBase::_getRendererObjectTypes(const std::string& type)
{
    AttributeMap::iterator iter = _objectTypes.find(type);
    if (iter == _objectTypes.end())
    {
        std::vector<std::string> names;
        fillRendererObjectTypes(names, type);

        FnAttribute::StringBuilder sb;
        sb.set(names);
        iter = _objectTypes.insert(AttributeMap::value_type(type, sb.build())).first;
    }

    return (*iter).second;
}

FnAttribute::Attribute RendererInfoBase::_getRendererShaderTypeTags(const std::string& shaderType)
{
    std::vector<std::string> tags;
    fillRendererShaderTypeTags(tags, shaderType);

    FnAttribute::StringBuilder sb;
    sb.set(tags);
    return sb.build();
}

FnAttribute::Attribute RendererInfoBase::_getRendererCoshaderType()
{
    std::string name = getRendererCoshaderType();

    return FnAttribute::StringAttribute(name);
}

FnAttribute::Attribute RendererInfoBase::_getRegisteredRendererName()
{
    std::string rendererName = getRegisteredRendererName();

    return FnAttribute::StringAttribute(rendererName);
}

FnAttribute::Attribute RendererInfoBase::_getRegisteredRendererVersion()
{
    std::string rendererVersion = getRegisteredRendererVersion();

    return FnAttribute::StringAttribute(rendererVersion);
}

FnAttribute::Attribute RendererInfoBase::_getRendererObjectDefaultType(const std::string& type)
{
    std::string name = getRendererObjectDefaultType(type);
    return FnAttribute::StringAttribute(name);
}

FnAttribute::Attribute RendererInfoBase::_getShaderInputNames(const std::string& shaderName)
{
    AttributeMap::iterator iter = _shaderInputNames.find(shaderName);
    if (iter == _shaderInputNames.end())
    {
        std::vector<std::string> names;
        fillShaderInputNames(names, shaderName);
        FnAttribute::StringBuilder sb;
        sb.set(names);
        iter = _shaderInputNames.insert(AttributeMap::value_type(shaderName, sb.build())).first;
    }

    return (*iter).second;
}


FnAttribute::Attribute RendererInfoBase::_getShaderInputTags(const std::string& shaderName,
                                                const std::string& inputName)
{
    std::string key = makeShaderInputOrOutputTagsKey(shaderName, inputName);
    AttributeMap::iterator iter = _shaderInputTags.find(key);
    if (iter == _shaderInputTags.end())
    {
        std::vector<std::string> tags;
        fillShaderInputTags(tags, shaderName, inputName);
        FnAttribute::StringBuilder sb;
        sb.set(tags);
        iter = _shaderInputTags.insert(AttributeMap::value_type(key, sb.build())).first;
    }

    return (*iter).second;
}


FnAttribute::Attribute RendererInfoBase::_getShaderOutputNames(const std::string& shaderName)
{
    AttributeMap::iterator iter = _shaderOutputNames.find(shaderName);
    if (iter == _shaderOutputNames.end())
    {
        std::vector<std::string> names;
        fillShaderOutputNames(names, shaderName);
        FnAttribute::StringBuilder sb;
        sb.set(names);
        iter = _shaderOutputNames.insert(AttributeMap::value_type(shaderName, sb.build())).first;
    }

    return (*iter).second;
}


FnAttribute::Attribute RendererInfoBase::_getShaderOutputTags(const std::string& shaderName,
                                                 const std::string& outputName)
{
    std::string key = makeShaderInputOrOutputTagsKey(shaderName, outputName);
    AttributeMap::iterator iter = _shaderOutputTags.find(key);
    if (iter == _shaderOutputTags.end())
    {
        std::vector<std::string> tags;
        fillShaderOutputTags(tags, shaderName, outputName);
        FnAttribute::StringBuilder sb;
        sb.set(tags);
        iter = _shaderOutputTags.insert(AttributeMap::value_type(key, sb.build())).first;
    }

    return (*iter).second;
}

FnAttribute::Attribute RendererInfoBase::_getLiveRenderTerminalOps(
        const FnAttributeHandle stateArgs)
{
    FnAttribute::GroupAttribute stateArgsAttr =
                FnAttribute::Attribute::CreateAndRetain(stateArgs);

    OpDefinitionQueue terminalOps;
    fillLiveRenderTerminalOps(terminalOps, stateArgsAttr);
    FnAttribute::GroupBuilder gb;
    int index = 0;

    for (OpDefinitionQueue::iterator it = terminalOps.begin();
         it != terminalOps.end(); ++it)
    {
        std::ostringstream opBaseName;
        opBaseName << "op" << index++;
        gb.set(opBaseName.str()+".opType", FnAttribute::StringAttribute(it->first));
        gb.set(opBaseName.str()+".opArgs", it->second);
    }

    return gb.build();
}

FnAttribute::Attribute RendererInfoBase::_getRenderTerminalOps(
        const FnAttributeHandle stateArgs)
{
    FnAttribute::GroupAttribute stateArgsAttr =
            FnAttribute::Attribute::CreateAndRetain(stateArgs);

    OpDefinitionQueue terminalOps;
    fillRenderTerminalOps(terminalOps, stateArgsAttr);
    FnAttribute::GroupBuilder gb;
    int index = 0;

    for (OpDefinitionQueue::iterator it = terminalOps.begin();
         it != terminalOps.end(); ++it)
    {
        std::ostringstream opBaseName;
        opBaseName << "op" << index++;
        gb.set(opBaseName.str()+".opType", FnAttribute::StringAttribute(it->first));
        gb.set(opBaseName.str()+".opArgs", it->second);
    }

    return gb.build();
}


FnAttribute::Attribute RendererInfoBase::_getRendererObjectInfo(const std::string& name,
                                                   const std::string& type,
                                                   const FnAttributeHandle inputData)
{
    FnAttribute::GroupAttribute inputAttr =
        FnAttribute::Attribute::CreateAndRetain(inputData);

    if (type == kFnRendererObjectTypeRenderOutput)
    {
        // No caching for RenderOutputs
        FnAttribute::GroupBuilder gb;
        if (!buildRendererObjectInfo(gb, name, type, inputAttr)) return FnAttribute::Attribute();

        gb.set(kFnRendererObjectInfoKeyName, FnAttribute::StringAttribute(name));
        gb.set(kFnRendererObjectInfoKeyProtocolVersion, FnAttribute::IntAttribute(1));

        return gb.build();
    }
    else
    {
        // Use name, type and a hash of the inputData as key to identify items.

        // FIXME: This will need to be revisited for thread-safety.  At the
        // same time, the key can be changed to a AttributeKeyedCache-friendly attr.
        std::vector<std::string> keyComponents;
        keyComponents.push_back(name);
        keyComponents.push_back(type);
        if (inputData)
        {
            FnAttribute::Attribute inputDataAttr = FnAttribute::Attribute::CreateAndSteal(inputData);
            if (inputDataAttr.isValid())
            {
                keyComponents.push_back(inputDataAttr.getHash().str());
            }
        }
        std::string itemKey = makeKey(keyComponents);

        AttributeMap::iterator iter = _objectInfo.find(itemKey);
        if (iter == _objectInfo.end())
        {
            // No item found, add it to the map
            FnAttribute::GroupBuilder gb;
            if (!buildRendererObjectInfo(gb, name, type, inputAttr)) return FnAttribute::Attribute();

            gb.set(kFnRendererObjectInfoKeyName, FnAttribute::StringAttribute(name));
            gb.set(kFnRendererObjectInfoKeyProtocolVersion, FnAttribute::IntAttribute(1));

            iter = _objectInfo.insert(AttributeMap::value_type(itemKey, gb.build())).first;
        }
        return (*iter).second;
    }
}

void RendererInfoBase::fillRendererShaderTypeTags(std::vector<std::string>& shaderTypeTags,
                                                  const std::string& shaderType) const
{
    shaderTypeTags.clear();
}

std::string RendererInfoBase::getRendererObjectDefaultType(const std::string& type) const
{
    if (type == kFnRendererObjectTypeRenderOutput)
    {
        return kFnRendererOutputTypeColor;
    }

    return "";
}

void RendererInfoBase::fillShaderInputNames(std::vector<std::string>& shaderInputNames,
                                            const std::string& shaderName) const
{
    shaderInputNames.clear();
}

void RendererInfoBase::fillShaderInputTags(std::vector<std::string>& shaderInputTags,
                                           const std::string& shaderName,
                                           const std::string& inputName) const
{
    shaderInputTags.clear();
}

void RendererInfoBase::fillShaderOutputNames(std::vector<std::string>& shaderOutputNames,
                                             const std::string& shaderName) const
{
    shaderOutputNames.clear();
}

void RendererInfoBase::fillShaderOutputTags(std::vector<std::string>& shaderOutputTags,
                                            const std::string& shaderName,
                                            const std::string& inputName) const
{
    shaderOutputTags.clear();
}

void RendererInfoBase::_setTypeTagNameFilter(const std::string& filter,
                                             const std::string& typeTag)
{
    _typeTagFilters[filter] = typeTag;
    _objectNames.clear();
    _objectTypes.clear();
}

std::string RendererInfoBase::getPluginPath() const
{
    return _pluginPath;
}

std::string RendererInfoBase::getPluginRootPath() const
{
    return _pluginRootPath;
}

void RendererInfoBase::setPluginPath(const std::string& pluginPath)
{
    _pluginPath = pluginPath;
}

void RendererInfoBase::setPluginRootPath(const std::string& pluginRootPath)
{
    _pluginRootPath = pluginRootPath;
}

void RendererInfoBase::setKatanaPath(const std::string& katana_path)
{
    _katanaPath = katana_path;
}

std::string RendererInfoBase::getKatanaPath() const
{
    return _katanaPath;
}

void RendererInfoBase::setTmpPath(const std::string& tmp_path)
{
    _tmpPath = tmp_path;
}

std::string RendererInfoBase::getTmpPath() const
{
    return _tmpPath;
}

void RendererInfoBase::_addObjectLocation(const std::string& type,
                                          const std::string& location)
{
    _additionalObjectLocations[type].push_back(location);
    _objectNames.clear();
    _objectTypes.clear();
}

void RendererInfoBase::_clearObjectLocations(const std::string& type)
{
    if (type.empty())
    {
        _additionalObjectLocations.clear();
    }
    else
    {
        _additionalObjectLocations[type].clear();
    }
    _objectNames.clear();
    _objectTypes.clear();
}

void RendererInfoBase::_flushCaches()
{
    _objectNames.clear();
    _shaderInputNames.clear();
    _shaderInputTags.clear();
    _shaderOutputNames.clear();
    _shaderOutputTags.clear();
    _objectInfo.clear();
    _objectTypes.clear();
}

void RendererInfoBase::configureBasicRenderObjectInfo(FnAttribute::GroupBuilder& renderObjectInfo,
                                                      const std::string& type,
                                                      const std::vector<std::string>& typeTags,
                                                      const std::string& location,
                                                      const std::string& fullPath,
                                                      int outputType,
                                                      FnAttribute::Attribute containerHints)
{
    renderObjectInfo.set(kFnRendererObjectInfoKeyType, FnAttribute::StringAttribute(type));

    FnAttribute::StringBuilder sb;
    sb.set(typeTags);
    renderObjectInfo.set(kFnRendererObjectInfoKeyTypeTags, sb.build());

    renderObjectInfo.set(kFnRendererObjectInfoKeyLocation, FnAttribute::StringAttribute(location));
    renderObjectInfo.set(kFnRendererObjectInfoKeyFullPath, FnAttribute::StringAttribute(fullPath));
    renderObjectInfo.set(kFnRendererObjectInfoKeyOutputType, FnAttribute::IntAttribute(outputType));
    renderObjectInfo.set(kFnRendererObjectInfoKeyContainerHints, containerHints);
}

void RendererInfoBase::addRenderObjectParam(FnAttribute::GroupBuilder& renderObjectInfo,
                                            const std::string& name,
                                            int type,
                                            int arraySize,
                                            FnAttribute::Attribute defaultAttr,
                                            FnAttribute::Attribute hintsAttr,
                                            const EnumPairVector& enumValues) const
{
    std::string prefix = kFnRendererObjectInfoKeyParam;
    prefix += ".";
    prefix += name;
    prefix += ".";

    renderObjectInfo.set(prefix + kFnRendererObjectInfoKeyParamType,      FnAttribute::IntAttribute(type));
    renderObjectInfo.set(prefix + kFnRendererObjectInfoKeyParamArraySize, FnAttribute::IntAttribute(arraySize));
    renderObjectInfo.set(prefix + kFnRendererObjectInfoKeyParamDefault,   defaultAttr);

    if (hintsAttr.isValid())
    {
        renderObjectInfo.set(prefix + kFnRendererObjectInfoKeyParamHints, hintsAttr);
    }

    if (!enumValues.empty())
    {
        FnAttribute::GroupBuilder gb2;
        for (EnumPairVector::const_iterator iter=enumValues.begin();
            iter!=enumValues.end(); ++iter)
        {
            gb2.set(FnAttribute::DelimiterEncode((*iter).first),
                    FnAttribute::IntAttribute((*iter).second));
        }
        renderObjectInfo.set(prefix + kFnRendererObjectInfoKeyParamEnums, gb2.build());
    }
}

void RendererInfoBase::setShaderParameterMapping(
    FnAttribute::GroupBuilder& renderObjectInfo, const std::string& metaName,
    const std::string& actualName) const
{
    const std::string containerHintsKey (
        kFnRendererObjectInfoKeyContainerHints);
    renderObjectInfo.set(containerHintsKey + ".hints.meta." + metaName,
                         FnAttribute::StringAttribute(actualName));
}

void RendererInfoBase::setShaderParameterMapping(
    FnAttribute::GroupBuilder& renderObjectInfo, const std::string& metaName,
    const std::vector<std::string>& actualNames) const
{
    const std::string containerHintsKey (
        kFnRendererObjectInfoKeyContainerHints);
    renderObjectInfo.set(containerHintsKey + ".hints.meta." + metaName,
                         FnAttribute::StringAttribute(actualNames));
}

void RendererInfoBase::getTypeTagsUsingNameFilters(
    const std::string& name,
    std::set<std::string>& typeTags) const
{
    for (StringMap::const_iterator iter = _typeTagFilters.begin();
         iter != _typeTagFilters.end(); ++iter)
    {
        const char* filter = iter->first.c_str();
        if (getRenderServicesSuite()->shaderTypeTagMatchesFilter(name.c_str(),
                                                                 filter))
        {
            typeTags.insert(iter->second.c_str());
        }
    }
}

bool RendererInfoBase::findTypeTagsInObjectTypeTags(const std::vector<std::string>& typeTags,
                                                    const std::set<std::string>& objectTypeTags) const
{
// make sure every string in the specified typeTags appears in objectTypeTags
    for (std::vector<std::string>::const_iterator typeIter=typeTags.begin();
        typeIter!=typeTags.end(); ++typeIter)
    {
        if (objectTypeTags.find(*typeIter) == objectTypeTags.end())
        {
            return false;
        }
    }

    return true;
}

RendererInfoPluginHandle RendererInfoBase::newRendererInfoPluginHandle(RendererInfoBase* rendererInfoPlugin)
{
    if (!rendererInfoPlugin)
    {
        return 0x0;
    }

    RendererInfoPluginHandle h = new RendererInfoPluginStruct(rendererInfoPlugin);
    return h;
}

FnPlugStatus RendererInfoBase::setHost(FnPluginHost* host)
{
    _host = host;

    FnPluginManager::PluginManager::setHost(host);
    FnLogging::setHost(host);
    FnAttribute::GroupBuilder::setHost(host);
    FnAsset::DefaultAssetPlugin::setHost(host);
    FnAsset::DefaultFileSequencePlugin::setHost(host);
    return FnAttribute::Attribute::setHost(host);
}

FnPluginHost* RendererInfoBase::getHost()
{
    return _host;
}

RendererInfoPluginSuite_v2 RendererInfoBase::createSuite(RendererInfoPluginHandle (*create)())
{
    RendererInfoPluginSuite_v2 suite;
    suite.create                                = create;
    suite.destroy                               = ::destroy,
    suite.getRenderMethods                      = ::getRenderMethods;
    suite.getBatchRenderMethod                  = ::getBatchRenderMethod;
    suite.getRendererObjectNames                = ::getRendererObjectNames;
    suite.getRendererObjectTypes                = ::getRendererObjectTypes;
    suite.getRendererShaderTypeTags             = ::getRendererShaderTypeTags;
    suite.getRendererCoshaderType               = ::getRendererCoshaderType;
    suite.getRegisteredRendererName             = ::getRegisteredRendererName;
    suite.getRegisteredRendererVersion          = ::getRegisteredRendererVersion;
    suite.isPresetLocalFileNeeded               = ::isPresetLocalFileNeeded;
    suite.isNodeTypeSupported                   = ::isNodeTypeSupported;
    suite.isPolymeshFacesetSplittingEnabled     = ::isPolymeshFacesetSplittingEnabled;
    suite.getShaderInputNames                   = ::getShaderInputNames;
    suite.getShaderInputTags                    = ::getShaderInputTags;
    suite.getShaderOutputNames                  = ::getShaderOutputNames;
    suite.getShaderOutputTags                   = ::getShaderOutputTags;
    suite.getRendererObjectDefaultType          = ::getRendererObjectDefaultType;
    suite.getLiveRenderControlModules           = ::getLiveRenderControlModules;
    suite.getRendererObjectInfo                 = ::getRendererObjectInfo;
    suite.setTypeTagNameFilter                  = ::setTypeTagNameFilter;
    suite.setPluginPath                         = ::setPluginPath;
    suite.setPluginRootPath                     = ::setPluginRootPath;
    suite.setKatanaPath                         = ::setKatanaPath;
    suite.setTmpPath                            = ::setTmpPath;
    suite.addObjectLocation                     = ::addObjectLocation;
    suite.clearObjectLocations                  = ::clearObjectLocations;
    suite.initialiseCaches                      = ::initialiseCaches;
    suite.flushCaches                           = ::flushCaches;
    suite.getLiveRenderTerminalOps              = ::getLiveRenderTerminalOps;
    suite.getRenderTerminalOps                  = ::getRenderTerminalOps;

    return suite;
};

unsigned int RendererInfoBase::_apiVersion = 2;
const char* RendererInfoBase::_apiName = "RendererInfoPlugin";
FnPluginHost* RendererInfoBase::_host = 0x0;

}
}
}
