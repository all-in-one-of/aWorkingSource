// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************
#ifndef ArnoldRendererInfo__H
#define ArnoldRendererInfo__H

#include <FnRendererInfo/plugin/RendererInfoBase.h>
#include <FnRendererInfo/plugin/RenderMethod.h>
#include <FnRendererInfo/plugin/ShaderInfoCache.h>

#include <FnAttribute/FnGroupBuilder.h>
#include <FnLogging/FnLogging.h>


class ArnoldRendererInfo : public FnKat::RendererInfo::RendererInfoBase
{
public:
    ArnoldRendererInfo();
    virtual ~ArnoldRendererInfo();

    static FnKat::RendererInfo::RendererInfoBase* create();

    void fillRenderMethods(std::vector<FnKat::RendererInfo::RenderMethod*>& renderMethods) const;

    void fillRendererObjectNames(std::vector<std::string>& rendererObjectNames,
                                 const std::string& type,
                                 const std::vector<std::string>& typeTags) const;

    void fillRendererObjectTypes(std::vector<std::string>& renderObjectTypes,
                                 const std::string& type) const;

    void configureBatchRenderMethod(FnKat::RendererInfo::DiskRenderMethod& batchRenderMethod) const;

    std::string getRegisteredRendererName() const;

    std::string getRegisteredRendererVersion() const;

    bool isNodeTypeSupported(const std::string& nodeType) const;

    void fillShaderInputNames(std::vector<std::string>& shaderInputNames,
                              const std::string& shaderName) const;

    void fillShaderInputTags(std::vector<std::string>& shaderInputTags,
                             const std::string& shaderName,
                             const std::string& inputName) const;

    void fillShaderOutputNames(std::vector<std::string>& shaderOutputNames,
                               const std::string& shaderName) const;

    void fillShaderOutputTags(std::vector<std::string>& shaderOutputTags,
                              const std::string& shaderName,
                              const std::string& outputName) const;

    void fillRendererShaderTypeTags(std::vector<std::string>& shaderTypeTags,
                                    const std::string& shaderType) const;

    bool buildRendererObjectInfo(FnAttribute::GroupBuilder& rendererObjectInfo,
                                 const std::string& name,
                                 const std::string& type,
                                 const FnAttribute::GroupAttribute inputAttr = 0x0) const;

    void fillRenderTerminalOps(
            FnKat::RendererInfo::RendererInfoBase::OpDefinitionQueue& terminalOps,
            const FnAttribute::GroupAttribute& stateArgs) const {};

    void fillLiveRenderTerminalOps(
            FnKat::RendererInfo::RendererInfoBase::OpDefinitionQueue& terminalOps,
            const FnAttribute::GroupAttribute& stateArgs) const;

    // build instance cache using available shaders
    void initialiseCaches();

    // flush instance cache
    void flushCaches();

    // flush per plugin cache
    static void flush() {};

private:
    static FnKat::FnLogging::FnLog _log;

    void _cacheShaderInfo() const;

    FnKat::Attribute _getShaderAttr(const std::string& shaderName,
                                    const std::string& attrName) const;

    void _fillChildInputsOrOutputs(std::vector<std::string>& childInputsOrOutputs,
                                   const std::string& parameterName,
                                   const int& type) const;

    void _fillInputTagsForType(std::vector<std::string>& inputTags,
                               const int& type) const;

    void _fillOutputTagsForType(std::vector<std::string>& outputTags,
                                const int& type) const;

    void _configureBasicRenderObjectInfo(FnAttribute::GroupBuilder& gb, 
                                         const std::string& name,
                                         const std::string& type) const;

    bool _buildOutputChannel(FnAttribute::GroupBuilder& gb, 
                             const std::string& name) const;

    bool _buildOutputChannelAttrHints(FnAttribute::GroupBuilder& gb, 
                                      const std::string& name) const;

    bool _buildRenderOutput(FnAttribute::GroupBuilder& gb, 
                            const std::string& name,
                            const FnAttribute::GroupAttribute inputAttr) const;

    typedef FnKat::RendererInfo::ShaderInfoCache<FnAttribute::GroupAttribute> Cache;
    typedef Cache::Iterator CacheIterator;
};

#endif  // ArnoldRendererInfo__H
