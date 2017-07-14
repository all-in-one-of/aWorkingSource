// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef INCLUDED_RENDERING_FNRENDERERINFOPLUGIN_H
#define INCLUDED_RENDERING_FNRENDERERINFOPLUGIN_H

#include <FnRendererInfo/suite/FnRendererInfoSuite.h>
#include <FnRendererInfo/suite/RendererObjectDefinitions.h>

#include <string>
#include <vector>
#include <map>

#include <tr1/memory>
#define PLUGIN_SHARED_PTR std::tr1::shared_ptr

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include "ns.h"

FNRENDERERINFO_NAMESPACE_ENTER
{
  class FnRendererObjectInfo
  {
    public:
      FnRendererObjectInfo(const FnAttribute::GroupAttribute & infoAttr);
      ~FnRendererObjectInfo();

      typedef PLUGIN_SHARED_PTR<FnRendererObjectInfo> Ptr;

      struct ParamInfo
      {
        std::string name;
        int type;
        int arraySize;
        FnAttribute::Attribute defaultAttr;
        FnAttribute::Attribute hintsAttr;
        FnAttribute::Attribute enumsAttr;
      };

      const std::string & getName() const {return _name;}
      const std::string & getType() const {return _type;}
      const std::vector<std::string> & getTypeTags() const {return _typeTags;}
      const std::string & getLocation() const {return _location;}
      const std::string & getFullPath() const {return _fullPath;}
      const std::vector<ParamInfo> &getParams() const {return _params;}
      int getOutputType() const {return _outputType;}
      FnAttribute::Attribute getContainerHints() const {return _containerHints;}

      int getParamIndex(const std::string & name) const
      {
        std::map<std::string, size_t>::const_iterator I =
          _paramIndexMap.find(name);
        if (I != _paramIndexMap.end())
        {
            return (int)(*I).second;
        }
        return -1;
      }



    private:
      void _parseInfo(const FnAttribute::GroupAttribute & infoAttr);
      void _parseProtocol1(const FnAttribute::GroupAttribute & infoAttr);
      std::string _stringFromChildAttr(const FnAttribute::GroupAttribute & infoAttr,
        const std::string &childName, const std::string &parentStr);
      int _intFromChildAttr(const FnAttribute::GroupAttribute & infoAttr,
        const std::string &childName, const std::string &parentStr);

      std::string _name;
      std::string _type;
      std::vector<std::string> _typeTags;
      std::string _location;
      std::string _fullPath;
      std::vector<ParamInfo> _params;
      std::map<std::string, size_t> _paramIndexMap;
      int _outputType;
      FnAttribute::Attribute _containerHints;
  };

  // Class that wraps a specific instance of RendererInfoPluginSuite_v2
  class FnRendererInfoPlugin
  {
  public:
    FnRendererInfoPlugin(const RendererInfoPluginSuite_v2 *suite);
    ~FnRendererInfoPlugin();

    typedef PLUGIN_SHARED_PTR<FnRendererInfoPlugin> Ptr;

    FnAttribute::GroupAttribute getRenderMethods();
    FnAttribute::GroupAttribute getBatchRenderMethod();

    void getRendererObjectNames(const std::string &type,
        const std::vector<std::string> &typeTags, std::vector<std::string> &result);

    void getRendererObjectTypes(const std::string &type, std::vector<std::string> &result);
    void getRendererShaderTypeTags(const std::string &shaderType, std::vector<std::string> &result);
    void getRendererCoshaderType(std::string &result);
    void getRegisteredRendererName(std::string &result);
    void getRegisteredRendererVersion(std::string &result);
    bool isPresetLocalFileNeeded(const std::string &outputType);
    bool isPolymeshFacesetSplittingEnabled();
    void getRendererObjectDefaultType(const std::string &type, std::string &result);

    bool isNodeTypeSupported(const std::string &nodeType);

    // Shader Inputs / Outputs
    void getShaderInputNames(const std::string &shader,
                             std::vector<std::string> &result);

    void getShaderInputTags(const std::string &shader,
                            const std::string &inputName,
                            std::vector<std::string> &result);

    void getShaderOutputNames(const std::string &shader,
                              std::vector<std::string> &result);

    void getShaderOutputTags(const std::string &shader,
                             const std::string &outputName,
                             std::vector<std::string> &result);

    FnRendererObjectInfo::Ptr getRendererObjectInfo(const std::string &name,
        const std::string &type, const FnAttribute::Attribute & inputData);

    void setTypeTagNameFilter(const std::string &filter, const std::string &typeTag);

    void addObjectLocation(const std::string &type, const std::string &location);
    void clearObjectLocations(const std::string &type);

    void setPluginPath(const std::string &plugin_path);
    void setPluginRootPath(const std::string &plugin_path);

    void setKatanaPath(const std::string &katana_path);

    void setTmpPath(const std::string &tmp_path);

    void flushCaches();

    void getLiveRenderTerminalOps(FnAttribute::GroupAttribute& terminalOps,
                                  const FnAttribute::GroupAttribute& stateArgs);
    void getRenderTerminalOps(FnAttribute::GroupAttribute& terminalOps,
                              const FnAttribute::GroupAttribute& stateArgs);

    const RendererInfoPluginSuite_v2 *getRendererInfoSuite() {return _suite;}
    RendererInfoPluginHandle getHandle() {return _handle;}

  private:
    const RendererInfoPluginSuite_v2 *_suite;
    RendererInfoPluginHandle _handle;

    // no copy/assign
    FnRendererInfoPlugin(const FnRendererInfoPlugin &rhs);
    FnRendererInfoPlugin &operator=(const FnRendererInfoPlugin &rhs);
  };
}
FNRENDERERINFO_NAMESPACE_EXIT

#endif // INCLUDED_RENDERING_FNRENDERERINFOPLUGIN_H
