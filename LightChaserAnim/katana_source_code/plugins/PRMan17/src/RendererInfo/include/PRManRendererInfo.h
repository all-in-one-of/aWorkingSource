// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#ifndef PRMANRENDERERINFO__H
#define PRMANRENDERERINFO__H

#include <FnRendererInfo/plugin/RendererInfoBase.h>
#include <FnRendererInfo/plugin/ShaderInfoCache.h>

#include <FnAttribute/FnGroupBuilder.h>
#include <FnLogging/FnLogging.h>

/*Taken from /usr/src/kernels/2.6.18-164.el5-x86_64/include/linux/limits.h*/
#define PATH_MAX 4096 /* # max length of a path name in bytes including nul. 4kb */
#define OutputTypePtc "ptc"

class TrackPaths;

class PRManRendererInfo : public FnKat::RendererInfo::RendererInfoBase
{
public:
    PRManRendererInfo();
    virtual ~PRManRendererInfo();

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

    bool isPresetLocalFileNeeded(const std::string& outputType) const;

    bool isNodeTypeSupported(const std::string& nodeType) const;

    bool isPolymeshFacesetSplittingEnabled() const;

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

    std::string getRendererCoshaderType() const;

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

    void flushCaches() ;

    // flush per plugin cache
    static void flush() {};

    typedef std::pair<std::string, std::string> DirFilePair;

private:
    void _cacheShaderInfo() const;

    bool _recursiveCacheShaderInfo(
          TrackPaths&  trackPaths,
          const std::string& rootPath,
          const std::string& relpath,
          const std::string& fullPath,
          const std::string& d_name) const;    

    bool _getDirAndFileFromLocation(DirFilePair& dirFilePair,
                                    std::string& objectName,
                                    const std::string& location) const;
    void _fillTypeTagsFromSloFile(std::set<std::string>& typeTags,
                                  const std::string& dirName, 
                                  const std::string& shaderName) const;
    void _fillParamsFromSloFile(FnAttribute::GroupBuilder& params, 
                                const std::string& dirName, 
                                const std::string& shaderName) const;
    void _configureBasicRenderObjectInfo(FnAttribute::GroupBuilder& gb, 
                                         const std::string& name,
                                         const std::string& type) const;
    bool _buildChannelCustomParam(FnAttribute::GroupBuilder& gb, 
                                  const std::string& name) const;
    bool _buildOutputChannel(FnAttribute::GroupBuilder& gb, 
                             const std::string& name) const;
    bool _buildRenderOutput(FnAttribute::GroupBuilder& gb, 
                            const std::string& name,
                            const FnAttribute::GroupAttribute inputAttr) const;


    static FnKat::FnLogging::FnLog _log;

    typedef FnKat::RendererInfo::ShaderInfoCache<DirFilePair> Cache;
    typedef Cache::Iterator CacheIterator;
};

#endif  // PRMANRENDERERINFO__H
