// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef COMPANYNAMEKATANA_TEMPLATERENDERERINFOPLUGIN_H
#define COMPANYNAMEKATANA_TEMPLATERENDERERINFOPLUGIN_H

#include <FnRendererInfo/plugin/RendererInfoBase.h>


namespace CompanyName
{
namespace Katana
{
    /**
     * \ingroup TemplateRenderPlugin
     */

    /**
     * @brief Template RendererInfo Plugin
     */
    class TemplateRendererInfoPlugin : public FnKat::RendererInfo::RendererInfoBase
    {
    public:

        TemplateRendererInfoPlugin();
        virtual ~TemplateRendererInfoPlugin();

        /**
        * fillRenderMethods
        */
        void fillRenderMethods(std::vector<FnKat::RendererInfo::RenderMethod*>& renderMethods) const;

        /**
        * fillRendererObjectNames
        */
        void fillRendererObjectNames(std::vector<std::string>& rendererObjectNames,
                                     const std::string& type,
                                     const std::vector<std::string>& typeTags) const;

        /**
        * fillRendererObjectTypes
        */
       void fillRendererObjectTypes(std::vector<std::string>& renderObjectTypes,
                                    const std::string& type) const;

        /**
        * configureBatchRenderMethod
        */
        void configureBatchRenderMethod(FnKat::RendererInfo::DiskRenderMethod& batchRenderMethod) const;

        /**
        * Registered renderer name that corresponds to this renderer info
        *
        * @param result The renderer plug-in that corresponds to this renderer info plug-in
        */
        std::string getRegisteredRendererName() const;

        /**
        * Registered version of the renderer this renderer info is used with.
        *
        * @param result The renderer version
        * @see getRegisteredRendererName
        */
        std::string getRegisteredRendererVersion() const;

        /**
        * getRendererObjectDefaultType
        */
        std::string getRendererObjectDefaultType(const std::string& type) const;

        /**
        * Declares if a renderer output requires a pre-declared temp file
        * (accessible in scene graph with implicit resolvers).
        *
        * @param outputType A render output type
        * @return true if a local file is needed, false otherwise
        */
        bool isPresetLocalFileNeeded(const std::string& outputType) const;

        /**
        * Katana will call this function to determine if the renderer supports
        * specific nodes. Currently ShadingNode and OutputChannelDefine will be
        * queried. True should be returned if the renderer supports this node type.
        *
        * @return true if the node type is supported, false otherwise
        */
        bool isNodeTypeSupported(const std::string& nodeType) const;

        /**
        * Declares if polymesh faces are split into sub-meshes where each
        * mesh represents a single face set as required by some renderers.
        *
        * @return true if splitting is enabled, false otherwise
        */
        bool isPolymeshFacesetSplittingEnabled() const;

        /**
        * Shader Inputs / Outputs
        *
        */
        void fillShaderInputNames(std::vector<std::string>& shaderInputNames,
                                  const std::string& shaderName) const;

        /**
        * fillShaderInputTags
        */
        void fillShaderInputTags(std::vector<std::string>& shaderInputTags,
                                 const std::string& shaderName,
                                 const std::string& inputName) const;

        /**
        * fillShaderOutputNames
        */
        void fillShaderOutputNames(std::vector<std::string>& shaderOutputNames,
                                   const std::string& shaderName) const;

        /**
        * fillShaderOutputTags
        */
        void fillShaderOutputTags(std::vector<std::string>& shaderOutputTags,
                                  const std::string& shaderName,
                                  const std::string& outputName) const;

        /**
        * fillRendererShaderTypeTags
        */
        void fillRendererShaderTypeTags(std::vector<std::string>& shaderTypeTags,
                                        const std::string& shaderType) const;

        /**
        * getRendererCoshaderType
        */
        std::string getRendererCoshaderType() const;

        /**
        * buildRendererObjectInfo
        */
        bool buildRendererObjectInfo(FnKat::GroupBuilder& rendererObjectInfo,
                                     const std::string& name,
                                     const std::string& type,
                                     const FnKat::GroupAttribute inputAttr) const;

        /**
        * flushCaches
        */
        void flushCaches();

        static FnKat::RendererInfo::RendererInfoBase* create()
        {
            return new TemplateRendererInfoPlugin();
        }

        static void flush()
        {

        }
    };

    /**
     * @}
     */
}
}

#endif
