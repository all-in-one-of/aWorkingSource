// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <FnRendererInfo/FnRendererInfoPluginClient.h>

#include <iostream>
#include <map>
#include <stdexcept>
#include <sstream>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnDataBuilder.h>

#include <FnGeolib/util/AttributeKeyedCache.h>

FNRENDERERINFO_NAMESPACE_ENTER
{

FnRendererInfoPlugin::FnRendererInfoPlugin(const RendererInfoPluginSuite_v2 *suite) : _suite(suite), _handle(0x0)
{
    if (!_suite)
    {
        throw std::runtime_error("FnRendererInfoPlugin constructed with no renderer info plugin API suite.");
    }

    _handle = _suite->create();
    if (!_handle)
    {
        throw std::runtime_error("FnRendererInfoPlugin failed to create renderer info plugin instance.");
    }
}

FnRendererInfoPlugin::~FnRendererInfoPlugin()
{
    _suite->destroy(_handle);
}

void FnRendererInfoPlugin::getRendererObjectNames(const std::string &type,
    const std::vector<std::string> &typeTags, std::vector<std::string> &result)
{
    FnAttribute::StringAttribute typeTagsAttr(typeTags);
    FnAttribute::StringAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
        _suite->getRendererObjectNames(_handle, type.c_str(),
            typeTagsAttr.getHandle()));

    result.clear();
    if (resultAttr.isValid())
    {
        FnAttribute::StringConstVector resultValue =
            resultAttr.getNearestSample(0.0f);
        result.insert(result.begin(), resultValue.begin(), resultValue.end());
    }
}

FnAttribute::GroupAttribute FnRendererInfoPlugin::getRenderMethods()
{
    FnAttribute::GroupAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
            _suite->getRenderMethods(_handle));
    return resultAttr;
}

FnAttribute::GroupAttribute FnRendererInfoPlugin::getBatchRenderMethod()
{
    FnAttribute::GroupAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
        _suite->getBatchRenderMethod(_handle));
    return resultAttr;
}

void FnRendererInfoPlugin::getRendererObjectTypes(const std::string &type,
    std::vector<std::string> &result)
{
    FnAttribute::StringAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
        _suite->getRendererObjectTypes(_handle, type.c_str()));

    result.clear();
    if (resultAttr.isValid())
    {
        FnAttribute::StringConstVector resultValue =
            resultAttr.getNearestSample(0.0f);
        result.insert(result.begin(), resultValue.begin(), resultValue.end());
    }
}

void FnRendererInfoPlugin::getRendererShaderTypeTags(
    const std::string &shaderType, std::vector<std::string> &result)
{
    FnAttribute::StringAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
        _suite->getRendererShaderTypeTags(_handle, shaderType.c_str()));

    result.clear();
    if (resultAttr.isValid())
    {
        FnAttribute::StringConstVector resultValue =
            resultAttr.getNearestSample(0.0f);
        result.insert(result.begin(), resultValue.begin(), resultValue.end());
    }
}

void FnRendererInfoPlugin::getRendererCoshaderType(std::string &result)
{
    FnAttribute::StringAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
        _suite->getRendererCoshaderType(_handle));
    result = resultAttr.getValue("", false);
}

void FnRendererInfoPlugin::getRegisteredRendererName(std::string &result)
{
    FnAttribute::StringAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
        _suite->getRegisteredRendererName(_handle));
    result = resultAttr.getValue("", false);
}

void FnRendererInfoPlugin::getRegisteredRendererVersion(std::string &result)
{
    FnAttribute::StringAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
        _suite->getRegisteredRendererVersion(_handle));
    result = resultAttr.getValue("", false);
}

bool FnRendererInfoPlugin::isPresetLocalFileNeeded(const std::string &outputType)
{
    return _suite->isPresetLocalFileNeeded(_handle, outputType.c_str());
}

bool FnRendererInfoPlugin::isPolymeshFacesetSplittingEnabled()
{
    return _suite->isPolymeshFacesetSplittingEnabled(_handle);
}

bool FnRendererInfoPlugin::isNodeTypeSupported(const std::string &nodeType)
{
  return _suite->isNodeTypeSupported(_handle, nodeType.c_str());
}

void FnRendererInfoPlugin::getRendererObjectDefaultType(const std::string &type,
    std::string &result)
{
    FnAttribute::StringAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
            _suite->getRendererObjectDefaultType(_handle, type.c_str()));
    result = resultAttr.getValue("", false);
}

void FnRendererInfoPlugin::getShaderInputNames(const std::string &shader,
    std::vector<std::string> &result)
{
    FnAttribute::StringAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
        _suite->getShaderInputNames(_handle, shader.c_str()));

    result.clear();
    if (resultAttr.isValid())
    {
        FnAttribute::StringConstVector resultValue =
            resultAttr.getNearestSample(0.0f);
        result.insert(result.begin(), resultValue.begin(), resultValue.end());
    }
}

void FnRendererInfoPlugin::getShaderInputTags(const std::string &shader,
    const std::string &inputName, std::vector<std::string> &result)
{
    FnAttribute::StringAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
        _suite->getShaderInputTags(_handle, shader.c_str(), inputName.c_str()));

    result.clear();
    if (resultAttr.isValid())
    {
        FnAttribute::StringConstVector resultValue =
            resultAttr.getNearestSample(0.0f);
        result.insert(result.begin(), resultValue.begin(), resultValue.end());
    }
}

void FnRendererInfoPlugin::getShaderOutputNames(const std::string &shader,
                                                std::vector<std::string> &result)
{
    FnAttribute::StringAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
        _suite->getShaderOutputNames(_handle, shader.c_str()));

    result.clear();
    if (resultAttr.isValid())
    {
        FnAttribute::StringConstVector resultValue =
            resultAttr.getNearestSample(0.0f);
        result.insert(result.begin(), resultValue.begin(), resultValue.end());
    }
}

void FnRendererInfoPlugin::getShaderOutputTags(const std::string &shader,
                                               const std::string &outputName,
                                               std::vector<std::string> &result)
{
    FnAttribute::StringAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
        _suite->getShaderOutputTags(_handle, shader.c_str(),
            outputName.c_str()));

    result.clear();
    if (resultAttr.isValid())
    {
        FnAttribute::StringConstVector resultValue =
            resultAttr.getNearestSample(0.0f);
        result.insert(result.begin(), resultValue.begin(), resultValue.end());
    }
}



namespace
{
    class RendererObjectInfoCache :
        public FnGeolibUtil::AttributeKeyedCache<FnRendererObjectInfo>
    {
    public:
        RendererObjectInfoCache() :
            FnGeolibUtil::AttributeKeyedCache<FnRendererObjectInfo>(100, 1000)
        {
        }

    private:
        IMPLPtr createValue(const FnAttribute::Attribute & iAttr)
        {
            return IMPLPtr(new FnRendererObjectInfo(iAttr));
        }
    };

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif  // __clang__
    RendererObjectInfoCache g_rendererObjectInfoCache;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif  // __clang__
}


FnRendererObjectInfo::Ptr FnRendererInfoPlugin::getRendererObjectInfo(
    const std::string &name, const std::string &type,
    const FnAttribute::Attribute & inputData)
{
    FnAttribute::GroupAttribute resultAttr=
        FnAttribute::Attribute::CreateAndSteal(
        _suite->getRendererObjectInfo(_handle, name.c_str(), type.c_str(),
            inputData.getHandle()));
    if (resultAttr.isValid())
    {
        return g_rendererObjectInfoCache.getValue(resultAttr);
    }
    else
    {
        return FnRendererObjectInfo::Ptr();
    }
}

void FnRendererInfoPlugin::setTypeTagNameFilter(const std::string &filter, const std::string &typeTag)
{
  _suite->setTypeTagNameFilter(_handle, filter.c_str(), typeTag.c_str());
}

void FnRendererInfoPlugin::addObjectLocation(const std::string &type, const std::string &location)
{
  _suite->addObjectLocation(_handle, type.c_str(), location.c_str());
}

void FnRendererInfoPlugin::clearObjectLocations(const std::string &type)
{
  _suite->clearObjectLocations(_handle, type.c_str());
}


void FnRendererInfoPlugin::setPluginPath(const std::string &plugin_path)
{
  _suite->setPluginPath(_handle, plugin_path.c_str());
}

void FnRendererInfoPlugin::setPluginRootPath(const std::string &plugin_path)
{
  _suite->setPluginRootPath(_handle, plugin_path.c_str());
}

void FnRendererInfoPlugin::setKatanaPath(const std::string &katana_path)
{
  _suite->setKatanaPath(_handle, katana_path.c_str());
}

void FnRendererInfoPlugin::setTmpPath(const std::string &tmp_path)
{
  _suite->setTmpPath(_handle, tmp_path.c_str());
}

void FnRendererInfoPlugin::flushCaches()
{
  g_rendererObjectInfoCache.clear();
  _suite->flushCaches(_handle);
}

void FnRendererInfoPlugin::getLiveRenderTerminalOps(
        FnAttribute::GroupAttribute& terminalOps,
        const FnAttribute::GroupAttribute& stateArgs)
{
    FnAttribute::GroupAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
            _suite->getLiveRenderTerminalOps(_handle, stateArgs.getHandle()));

    terminalOps = resultAttr;
}

void FnRendererInfoPlugin::getRenderTerminalOps(
        FnAttribute::GroupAttribute& terminalOps,
        const FnAttribute::GroupAttribute& stateArgs)
{
    FnAttribute::GroupAttribute resultAttr =
        FnAttribute::Attribute::CreateAndSteal(
            _suite->getRenderTerminalOps(_handle, stateArgs.getHandle()));

    terminalOps = resultAttr;
}

// --- FnRendererObjectInfo --------------------------------------------------------------


FnRendererObjectInfo::FnRendererObjectInfo(
    const FnAttribute::GroupAttribute & infoAttr) :
    _outputType(kFnRendererObjectValueTypeUnknown)
{
    if (!infoAttr.isValid())
    {
        throw std::runtime_error("FnRendererObjectInfo constructed with no info attribute.");
    }

    _parseInfo(infoAttr);
}

FnRendererObjectInfo::~FnRendererObjectInfo()
{
}


void FnRendererObjectInfo::_parseInfo(const FnAttribute::GroupAttribute & infoAttr)
{
    // check protocol and dispatch
    FnAttribute::IntAttribute protocolAttr(
        infoAttr.getChildByName(kFnRendererObjectInfoKeyProtocolVersion));
    if (protocolAttr.isValid() && protocolAttr.getNumberOfValues() >= 1)
    {
        int protocolVersion = protocolAttr.getValue();
        switch (protocolVersion)
        {
        case 1: _parseProtocol1(infoAttr); break;
        default:
        {
            std::ostringstream errmsg;
            errmsg << "Unknown info protocol version: " << protocolVersion;
            throw std::runtime_error(errmsg.str());
        }
        }
    }
}

void FnRendererObjectInfo::_parseProtocol1(
    const FnAttribute::GroupAttribute & infoAttr)
{
    _name = _stringFromChildAttr(infoAttr, kFnRendererObjectInfoKeyName, "info");
    _type = _stringFromChildAttr(infoAttr, kFnRendererObjectInfoKeyType, "info");
    _location = _stringFromChildAttr(infoAttr, kFnRendererObjectInfoKeyLocation, "info");
    _fullPath = _stringFromChildAttr(infoAttr, kFnRendererObjectInfoKeyFullPath, "info");
    _outputType = _intFromChildAttr(infoAttr, kFnRendererObjectInfoKeyOutputType, "info");
    _containerHints = infoAttr.getChildByName(
        kFnRendererObjectInfoKeyContainerHints);

    FnAttribute::StringAttribute typeTagsAttr(
        infoAttr.getChildByName(kFnRendererObjectInfoKeyTypeTags));
    if (typeTagsAttr.isValid())
    {
        FnAttribute::StringConstVector typeTagsValue =
            typeTagsAttr.getNearestSample(0.0f);
        _typeTags.clear();
        _typeTags.insert(_typeTags.begin(),
            typeTagsValue.begin(), typeTagsValue.end());
    }

    FnAttribute::GroupAttribute paramAttr(
        infoAttr.getChildByName(kFnRendererObjectInfoKeyParam));
    if (paramAttr.isValid())
    {
        int64_t numParamChildren = paramAttr.getNumberOfChildren();
        for (int64_t i=0; i<numParamChildren; ++i)
        {
            ParamInfo paramInfo;
            paramInfo.name = paramAttr.getChildName(i);

            FnAttribute::GroupAttribute paramGroupAttr(
                paramAttr.getChildByIndex(i));

            if (!paramGroupAttr.isValid())
            {
                throw std::runtime_error("Param info attr '" + paramInfo.name + "' is not a group.");
            }

            paramInfo.type = _intFromChildAttr(paramGroupAttr, kFnRendererObjectInfoKeyParamType,
                    "param '"+paramInfo.name+"'");
            paramInfo.arraySize = _intFromChildAttr(paramGroupAttr, kFnRendererObjectInfoKeyParamArraySize,
                    "param '"+paramInfo.name+"'");
            paramInfo.defaultAttr = paramGroupAttr.getChildByName(kFnRendererObjectInfoKeyParamDefault);
            paramInfo.hintsAttr = paramGroupAttr.getChildByName(kFnRendererObjectInfoKeyParamHints);
            paramInfo.enumsAttr = paramGroupAttr.getChildByName(kFnRendererObjectInfoKeyParamEnums);

            _paramIndexMap[paramInfo.name] = _params.size();
            _params.push_back(paramInfo);
        }
    }
}

std::string FnRendererObjectInfo::_stringFromChildAttr(
    const FnAttribute::GroupAttribute & infoAttr,
    const std::string & childName, const std::string & parentStr)
{
    FnAttribute::StringAttribute stringAttr(
        infoAttr.getChildByName(childName));
    try
    {
        return stringAttr.getValue();
    }
    catch (std::exception & e)
    {
        throw std::runtime_error("Error getting '" + childName + "' from " + parentStr + " attribute.");
    }
}

int FnRendererObjectInfo::_intFromChildAttr(
    const FnAttribute::GroupAttribute & infoAttr,
    const std::string &childName, const std::string &parentStr)
{
    FnAttribute::IntAttribute intAttr(
        infoAttr.getChildByName(childName));
    try
    {
        return intAttr.getValue();
    }
    catch (std::exception & e)
    {
        throw std::runtime_error("Error getting '" + childName + "' from " + parentStr + " attribute.");
    }
}

}
FNRENDERERINFO_NAMESPACE_EXIT
