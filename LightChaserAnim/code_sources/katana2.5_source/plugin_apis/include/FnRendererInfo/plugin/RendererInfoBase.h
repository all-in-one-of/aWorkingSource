// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDERERINFO_RENDERERINFOBASE_H
#define FNRENDERERINFO_RENDERERINFOBASE_H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnPluginSystem/FnPlugin.h>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <FnRendererInfo/suite/RendererObjectDefinitions.h>
#include <FnRendererInfo/suite/FnRendererInfoSuite.h>

#include <FnRendererInfo/plugin/RenderMethod.h>
#include <FnRendererInfo/plugin/LiveRenderControlModule.h>
#include <FnRendererInfo/plugin/LiveRenderFilter.h>
#include <FnRendererInfo/FnRendererInfoAPI.h>

#include <iostream>
#include <string>
#include <memory>
#include <map>
#include <set>
#include <deque>
#include <utility>

namespace Foundry
{
namespace Katana
{
namespace RendererInfo
{
    /**
     * \ingroup RenderAPI
     * @{
     *
     * @brief The renderer info plug-in provides Katana with renderer specific information
     *        such as shaders and render outputs, as well as configuring how a render is
     *        launched.
     *
     * \section InteractionUI Populating renderer-specific UI parameters
     * A renderer object defines renderer-specific properties which can be configured
     * and assigned in a Katana scene. The types of renderer object are defined in
     * RendererObjectDefinitions.h and include e.g. shader, render output, output channel,
     * procedural, etc. The RendererInfoBase::buildRendererObjectInfo builds an object's
     * properties and parameters.
     *
     * Creating these renderer objects in the UI generally require selecting a particular
     * object from a list of typed renderer object names which is populated by
     * RendererInfoBase::fillRendererObjectNames. In some cases, the object names are
     * filtered based on a list of types which is populated by
     * RendererInfoBase::fillRendererObjectTypes.
     *
     * An example of this process is the use of the Material node which uses the
     * RendererObjectDefinitions::kFnRendererObjectTypeShader type:
     *  - The user clicks 'Add Shader' in the Material node which populates a list of shader types
     *    such as surface, displacement, etc. in a drop-down list. This list of shader types is
     *    populated by RendererInfoBase::fillRendererObjectTypes. A typed shader widget appears
     *    below the button where it is named '[rendererName][RendererObjectType]Shader'.
     *  - The user clicks the typed shader widget to get the list of available shader names for the
     *    corresponding renderer object type. The shader names appear in a drop-down list which is
     *    populated by RendererInfoBase::fillRendererObjectNames where the selected shader type
     *    is passed as a type tag. The co-shader type is included in the type tags if co-shaders
     *    are supported (see RendererInfoBase::getRendererCoshaderType).
     *  - Once a user has selected a shader from the drop-down list,
     *    RendererInfo::RendererInfoBase::buildRendererObjectInfo is called with the selected
     *    shader name in order to populate the UI with the corresponding shader parameters.
     */
    class FNRENDERERINFO_API RendererInfoBase
    {
    public:
        RendererInfoBase();
        virtual ~RendererInfoBase() {}

        /**
         * Configure the render method used for batch rendering. A batch render method is
         * always added automatically but here it can be customised if needed.
         *
         * @param batchRenderMethod The batch render method.
         */
        virtual void configureBatchRenderMethod(RendererInfo::DiskRenderMethod& batchRenderMethod) const = 0;

        /**
         * Advertise supported render methods.
         *
         * @param renderMethods A reference container for supported render methods.
         */
        virtual void fillRenderMethods(std::vector<RendererInfo::RenderMethod*>& renderMethods) const = 0;

        /**
         * Advertise supported types for a given renderer object type. The supported renderer object
         * types are defined in RendererObjectDefinitions where the following conventions are typically
         * used:
         *
         * - RendererObjectDefinitions::kFnRendererObjectTypeShader: The advertised types populate the
         *                                'Add Shader' drop-down list in the Material node (e.g. surface,
         *                                displacement, etc.)
         * - RendererObjectDefinitions::kFnRendererObjectTypeRenderOutput: The advertised renderer types
         *                                populate the 'type' drop-down list in the RenderOutputDefine node
                                          (e.g. color, raw, etc.)
         *
         * See \ref InteractionUI "Populating renderer-specific UI parameters" for more
         * information.
         *
         * @param renderObjectTypes The advertised types for a given renderer object type.
         * @param type The renderer object type (e.g. RendererObjectDefinitions::kFnRendererObjectTypeShader).
         *
         * @see RendererInfo::RendererInfoBase::getRendererObjectDefaultType
         * @see RendererInfo::RendererInfoBase::fillRendererObjectNames
         * @see RendererInfo::RendererInfoBase::buildRendererObjectInfo
         */
        virtual void fillRendererObjectTypes(std::vector<std::string>& renderObjectTypes,
                                             const std::string& type) const = 0;

        /**
         * The default renderer value for a given object type if applicable.
         *
         * See \ref InteractionUI "Populating renderer-specific UI parameters" for more
         * information.
         *
         * @param type An object type (e.g. renderOutput)
         * @return The default renderer type (e.g. kFnRendererOutputTypeColor)
         * @see RendererInfo::RendererInfoBase::fillRendererObjectTypes
         * @see RendererInfo::RendererInfoBase::fillRendererObjectNames
         * @see RendererInfo::RendererInfoBase::buildRendererObjectInfo
         */
        virtual std::string getRendererObjectDefaultType(const std::string& type) const;

        typedef std::pair<std::string, FnAttribute::GroupAttribute> OpDefinition;
        typedef std::deque<OpDefinition> OpDefinitionQueue;

        /**
         * Populate a list of terminal Op definitions that should be applied to
         * the Op Tree during Live Rendering.
         *
         * @param terminalOps The populated terminal op deffinitions. The expected
         *                    format is a deque containing a std::pair<string, GroupAttribute>.
         *                    The string is the name of the op and the GroupAttribute is the op args.
         *
         * @param stateArgs A GroupAttribute containing some extra information.
         *                  Contains the entries defined by the constants:
         *                  - kFnTerminalOpStateArgRenderMethodType - the render
         *                      method type, which will be one of the strings
         *                      defined by the constants starting with
         *                      kFnRenderMethodType.
         *                  - kTerminalOpStateArgSystem - the Op System Args, as
         *                      defined by the GraphState. It contains
         *                      information like: currentTime, shutter info,
         *                      numSamples, etc.
         */
        virtual void fillLiveRenderTerminalOps(OpDefinitionQueue& terminalOps,
                const FnAttribute::GroupAttribute& stateArgs) const {}

        /**
         * Populate a list of terminal Op definitions that should be applied to
         * the Op Tree during specified render mode.
         *
         * @param terminalOps The populated terminal op definitions. The expected
         *                    format is a deque containing a std::pair<string, GroupAttribute>.
         *                    The string is the name of the op and the GroupAttribute is the op args.
         *
         * @param stateArgs A GroupAttribute containing some extra information.
         *                  Contains the entries defined by the constants:
         *                  - kFnTerminalOpStateArgRenderMethodType - the render
         *                      method type, which will be one of the strings
         *                      defined by the constants starting with
         *                      kFnRenderMethodType.
         *                  - kTerminalOpStateArgSystem - the Op System Args, as
         *                      defined by the GraphState. It contains
         *                      information like: currentTime, shutter info,
         *                      numSamples, etc.
         */
        virtual void fillRenderTerminalOps(OpDefinitionQueue& terminalOps,
                const FnAttribute::GroupAttribute& stateArgs) const {}


        /**
         * Advertise available names for a given renderer object type and type tags. This
         * is used to provide a list of UI options for shaders, drivers, filters, etc.
         * The type tags can be used to filter the list of available names.
         *
         * See \ref InteractionUI "Populating renderer-specific UI parameters" for more
         * information.
         *
         * @param rendererObjectNames The names of available render objects (e.g. shader names)
         * @param type The render object type (e.g. RendererObjectDefinitions::kFnRendererObjectTypeShader)
         * @param typeTags Filter tags which typically include the selected render object type
         *                 (e.g. surface) which can be used to filter the object names based
         *                 on the user's selection in the UI.
         *
         * @see RendererInfo::RendererInfoBase::fillRendererObjectTypes
         * @see RendererInfo::RendererInfoBase::getRendererObjectDefaultType
         * @see RendererInfo::RendererInfoBase::buildRendererObjectInfo
         */
        virtual void fillRendererObjectNames(std::vector<std::string>& rendererObjectNames,
                                             const std::string& type,
                                             const std::vector<std::string>& typeTags) const = 0;

        /**
         * Registered renderer name that corresponds to this renderer info
         *
         * @return The renderer plug-in that corresponds to this renderer info plug-in
         */
        virtual std::string getRegisteredRendererName() const = 0;

        /**
         * Registered version of the renderer this renderer info is used with.
         *
         * @return The renderer version
         * @see getRegisteredRendererName
         */
        virtual std::string getRegisteredRendererVersion() const = 0;

        /**
         * Declares if a renderer output requires a pre-declared temporary file
         * (accessible in scene graph with implicit resolvers).
         *
         * @param outputType A render output type
         * @return true if a local file is needed, false otherwise
         */
        virtual bool isPresetLocalFileNeeded(const std::string& outputType) const { return false; }

        /**
         * Katana will call this function to determine if the renderer supports
         * specific nodes. Currently, Katana only queries whether the ShadingNode and
         * OutputChannelDefine are supported.
         *
         * @param nodeType The node type
         * @return true if the node type is supported, false otherwise
         */
        virtual bool isNodeTypeSupported(const std::string& nodeType) const { return false; }

        /**
         * Declares if polymesh faces are split into sub-meshes where each
         * mesh represents a single face-set.
         *
         * @return true if polymesh face-set splitting is enabled, false otherwise
         */
        virtual bool isPolymeshFacesetSplittingEnabled() const { return false; }

        /**
         * Populate shader input names for a given shader in a shading node.
         * This can be used to validate the shading connections.
         *
         * @param shaderInputNames The populated input names for a given shader.
         * @param shaderName The name of the shader which will be populated with
         *                   the input names.
         */
        virtual void fillShaderInputNames(std::vector<std::string>& shaderInputNames,
                                          const std::string& shaderName) const;

        /**
         * Populate shader input tags for a given input on a shader in a shading node.
         * The tags describe what kind of connections are valid when connecting to this
         * input, e.g.:
         *
         * \code
         * shaderInputTags.push_back("float or rgb or rgba or vector or point or point2");
         * \endcode
         *
         * @param shaderInputTags The populated input tags for a given input on a shader.
         * @param shaderName The name of the shader which will be populated with
         *                   the input tags.
         * @param inputName  The input name on the shader.
         */
        virtual void fillShaderInputTags(std::vector<std::string>& shaderInputTags,
                                         const std::string& shaderName,
                                         const std::string& inputName) const;

        /**
         * Populate shader output names for a given shader in a shading node.
         * This can be used to validate the shading connections.
         *
         * @param shaderOutputNames The populated output names for a given shader.
         * @param shaderName The name of the shader which will be populated with
         *                   the output names.
         */
        virtual void fillShaderOutputNames(std::vector<std::string>& shaderOutputNames,
                                           const std::string& shaderName) const;

        /**
         * Populate shader output tags for a given output on a shader in a shading node.
         * The tags describe what kind of connections are valid when connecting to this
         * output , e.g.:
         *
         * \code
         * shaderOutputTags.push_back("float");
         * \endcode
         *
         * @param shaderOutputTags The populated output tags for a given output on a shader.
         * @param shaderName The name of the shader which will be populated with
         *                   the output tags.
         * @param outputName The output name on the shader.
         */
        virtual void fillShaderOutputTags(std::vector<std::string>& shaderOutputTags,
                                          const std::string& shaderName,
                                          const std::string& outputName) const;

        virtual void fillRendererShaderTypeTags(std::vector<std::string>& shaderTypeTags,
                                                const std::string& shaderType) const;

        /**
         * Specifies the shader type for coshaders if they are supported.
         *
         * @return The co-shader type if applicable. An empty string signifies
         *         that co-shaders are not supported.
         */
        virtual std::string getRendererCoshaderType() const  { return ""; }

        /**
         * Build custom live render modules for the live render tab.
         *
         * @param liveRenderControlModules Attributes describing the custom live
         *                                 render modules.
         * @deprecated All commands, data updates, and terminal Ops required
         *             should use the LiveRenderAPI Python module.
         */
        virtual void buildLiveRenderControlModules(FnAttribute::GroupBuilder& liveRenderControlModules) const {}

        /**
         * Build the attributes describing the renderer object information which includes
         * e.g. its name, type, locations, and parameters.
         *
         * See \ref InteractionUI "Populating renderer-specific UI parameters" for more
         * information.
         *
         * @param rendererObjectInfo The attributes containing the renderer object information
         *                           requested by Katana.
         * @param name The renderer object name selected in the UI (e.g. shader name)
         * @param type The render object type (e.g. RendererObjectDefinitions::kFnRendererObjectTypeShader)
         * @param inputAttr Optional input attributes.
         * @return true if the build process was successful, false otherwise.
         *
         * @see RendererInfo::RendererInfoBase::configureBasicRenderObjectInfo
         * @see RendererInfo::RendererInfoBase::addRenderObjectParam
         * @see RendererInfo::RendererInfoBase::fillRendererObjectTypes
         */
        virtual bool buildRendererObjectInfo(FnAttribute::GroupBuilder& rendererObjectInfo,
                                             const std::string& name,
                                             const std::string& type,
                                             const FnAttribute::GroupAttribute inputAttr) const = 0;

        /**
         * Flush cached data (e.g. shader information).
         */
        virtual void flushCaches() {}

        /**
         * Accessor for the path to the Render Plug-in.  In most cases this will be a directory named
         * 'Libs' within the root path of the plug-in (if you want to access the root path, you can
         * use the function getPluginRootPath instead).
         *
         * @return Full path to the directory containing the Render plug-in.
         */
        std::string getPluginPath() const;

        /**
         * Convenience function to access the parent directory of the plugin path (see getPluginPath())
         *
         * In addition to the 'Libs' directory in which the plugin's DSOs reside, the plugin's root
         * directory will commonly hold python plugins for Viewer Manipulators, nodes, etc. as well
         * as any settings or shared libraries used by the Render Plug-in.
         *
         * @return Full path to the plug-in's 'root' directory.
         */
        std::string getPluginRootPath() const;

        void setPluginPath(const std::string& pluginPath);
        void setPluginRootPath(const std::string& pluginRootPath);

        void setKatanaPath(const std::string& katana_path);
        std::string getKatanaPath() const;

        void setTmpPath(const std::string& tmp_path);
        std::string getTmpPath() const;

        // non-virtual implementations to do conversions and cache results from virtual functions above
        FnAttribute::Attribute _getRenderMethods();
        FnAttribute::Attribute _getBatchRenderMethod();
        FnAttribute::Attribute _getRendererObjectNames(const std::string& type,
                                          const std::vector<std::string>& typeTags);
        FnAttribute::Attribute _getRendererObjectTypes(const std::string& type);
        FnAttribute::Attribute _getRendererShaderTypeTags(const std::string& shaderType);
        FnAttribute::Attribute _getRendererCoshaderType();
        FnAttribute::Attribute _getRegisteredRendererName();
        FnAttribute::Attribute _getRegisteredRendererVersion();
        FnAttribute::Attribute _getRendererObjectDefaultType(const std::string& type);

        FnAttribute::Attribute _getShaderInputNames(const std::string& shaderName);
        FnAttribute::Attribute _getShaderInputTags(const std::string& shaderName,
                                      const std::string& inputName);

        FnAttribute::Attribute _getShaderOutputNames(const std::string& shaderName);
        FnAttribute::Attribute _getShaderOutputTags(const std::string& shaderName,
                                       const std::string& outputName);

        FnAttribute::Attribute _getRendererObjectInfo(const std::string& name,
                                         const std::string& typeTag,
                                         const FnAttributeHandle inputData = 0x0);

        void _setTypeTagNameFilter(const std::string& filter,
                                   const std::string& typeTag);

        void _addObjectLocation(const std::string& type,
                                const std::string& location);

        void _clearObjectLocations(const std::string& type);

        void _flushCaches();

        FnAttribute::Attribute _getLiveRenderTerminalOps(
                const FnAttributeHandle stateArgs);
        FnAttribute::Attribute _getRenderTerminalOps(
                const FnAttributeHandle stateArgs);

        ///@cond FN_INTERNAL_DEV
        static FnPlugStatus                 setHost(FnPluginHost* host);
        static FnPluginHost*                getHost();
        static RendererInfoPluginSuite_v2   createSuite(RendererInfoPluginHandle (*create)());

        static RendererInfoPluginHandle     newRendererInfoPluginHandle(RendererInfoBase* rendererInfoPlugin);

        static unsigned int                 _apiVersion;
        static const char*                  _apiName;

        /**
         * A utility function for building render object info. It builds the attribute
         * convention for declaring the properties of render objects such as shaders.
         *
         * @param renderObjectInfo The group attribute which defines the render object
         * @param type The render object type (e.g. RendererObjectDefinitions::kFnRendererObjectTypeShader)
         * @param typeTags Tags associated with the render object type (e.g. shader type)
         * @param location The location of the render object if applicable
         * @param fullPath The full path of the render object
         * @param outputType The output type if supported, otherwise use
         *                   kFnRendererObjectValueTypeUnknown
         * @param containerHints An empty attribute represents no hints, whereas a group
         *                       attribute containing one or more StringAttribute is
         *                       interpreted as container hints which can be used to
         *                       specify help texts for pages, and whether they are hidden
         *                       or open by default.
         *
         * @see RendererInfo::RendererInfoBase::buildRendererObjectInfo
         * @see RendererInfo::RendererInfoBase::addRenderObjectParam
         */
        static void configureBasicRenderObjectInfo(FnAttribute::GroupBuilder& renderObjectInfo,
                                                   const std::string& type,
                                                   const std::vector<std::string>& typeTags,
                                                   const std::string& location,
                                                   const std::string& fullPath,
                                                   int outputType,
                                                   FnAttribute::Attribute containerHints);

    protected:
        typedef std::pair<std::string, int> EnumPair;
        typedef std::vector<EnumPair> EnumPairVector;

        /**
         * A utility function for adding a parameter to a render object when building the
         * render object info. It builds an attribute which describes parameters
         * (e.g. shader parameters) and their properties such as their type and UI hints.
         *
         * @param renderObjectInfo The renderer object info which contains the parameter
         * @param name The parameter name
         * @param type The parameter type (e.g. RendererObjectDefinitions::kFnRendererObjectValueTypeColor3)
         * @param arraySize The parameter array size (an array if the size is > 1)
         * @param defaultAttr The default parameter value stored as a typed attribute
         * @param hintsAttr UI hints for the parameter
         * @enumValues Enumerated value pairs for the parameter
         *
         * @see RendererInfo::RendererInfoBase::buildRendererObjectInfo
         * @see RendererInfo::RendererInfoBase::configureBasicRenderObjectInfo
         */
        void addRenderObjectParam(FnAttribute::GroupBuilder& renderObjectInfo,
                                  const std::string& name,
                                  int type,
                                  int arraySize,
                                  FnAttribute::Attribute defaultAttr,
                                  FnAttribute::Attribute hintsAttr,
                                  const EnumPairVector& enumValues) const;

        /**
         * A utility function that sets a container hint in the given render
         * object info group builder structure to map a shader parameter
         * attribute meta name, such as "material.meta.color", to a single
         * actual shader parameter attribute name, such as
         * "material.prmanLightParams.lightcolor".
         *
         * Attribute meta names are used in the SceneGraphView widget to
         * determine what values to show in the columns of its table.
         *
         * @param renderObjectInfo The renderer object info to modify.
         * @param metaName The meta name of the attribute to map an actual attribute to.
         * @param actualName The name of the actual attribute to map the meta attribute to.
         */
        void setShaderParameterMapping(
            FnAttribute::GroupBuilder& renderObjectInfo,
            const std::string& metaName, const std::string& actualName) const;

        /**
         * A utility function that sets a container hint in the given render
         * object info group builder structure to map a shader parameter
         * attribute meta name, such as "material.meta.color", to a list of
         * actual shader parameter attribute names, such as
         * "material.prmanLightParams.lightcolor" and
         * "material.prmanLightParams.Color".
         *
         * Attribute meta names are used in the SceneGraphView widget to
         * determine what values to show in the columns of its table.
         *
         * @param renderObjectInfo The renderer object info to modify.
         * @param metaName The meta name of the attribute to map a list of actual attributes to.
         * @param actualNames A list of names of the actual attributes to map the meta attribute to.
         */
        void setShaderParameterMapping(
            FnAttribute::GroupBuilder& renderObjectInfo,
            const std::string& metaName,
            const std::vector<std::string>& actualNames) const;

        void getTypeTagsUsingNameFilters(const std::string& name,
                                         std::set<std::string>& typeTags) const;

        bool findTypeTagsInObjectTypeTags(const std::vector<std::string>& typeTags,
                                          const std::set<std::string>& objectTypeTags) const;

        typedef std::map<std::string, std::vector<std::string> > ObjectLocationsMap;

        const ObjectLocationsMap& getAdditionalObjectLocations() const {return _additionalObjectLocations;}
        const std::vector<std::string>& getAdditionalObjectLocations(const std::string& type) const {return _additionalObjectLocations[type];}

    private:

        static FnPluginHost* _host;

        typedef std::map<std::string, std::string> StringMap;
        typedef std::map<std::string, FnAttribute::Attribute> AttributeMap;

        AttributeMap _objectNames;
        AttributeMap _objectTypes;
        AttributeMap _shaderInputNames;
        AttributeMap _shaderInputTags;
        AttributeMap _shaderOutputNames;
        AttributeMap _shaderOutputTags;
        AttributeMap _objectInfo;

        bool _liveRenderControlModulesCached;
        FnAttribute::GroupAttribute _liveRenderControlModules;

        mutable ObjectLocationsMap _additionalObjectLocations;  // (locations by primary type)

        StringMap _typeTagFilters;

        std::string _pluginPath;
        std::string _pluginRootPath;
        std::string _katanaPath;
        std::string _tmpPath;

        ///@endcond
    };

    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

///@cond FN_INTERNAL_DEV

#define DEFINE_RENDERERINFO_PLUGIN(RENDERERINFO_CLASS)                                          \
                                                                                                \
  FnPlugin RENDERERINFO_CLASS##_plugin;                                                         \
                                                                                                \
  RendererInfoPluginHandle RENDERERINFO_CLASS##_create()                                        \
  {                                                                                             \
        return Foundry::Katana::RendererInfo::RendererInfoBase::newRendererInfoPluginHandle(    \
            RENDERERINFO_CLASS::create());                                                      \
  }                                                                                             \
                                                                                                \
  RendererInfoPluginSuite_v2 RENDERERINFO_CLASS##_suite =                                       \
    Foundry::Katana::RendererInfo::RendererInfoBase::createSuite(RENDERERINFO_CLASS##_create);  \
                                                                                                \
  const void* RENDERERINFO_CLASS##_getSuite()                                                   \
  {                                                                                             \
        return &RENDERERINFO_CLASS##_suite;                                                     \
  }                                                                                             \

struct FNRENDERERINFO_API RendererInfoPluginStruct
{
public:
    RendererInfoPluginStruct(Foundry::Katana::RendererInfo::RendererInfoBase* rendererInfoPlugin) :
    _rendererInfoPlugin(rendererInfoPlugin) { /* Empty */ }
    ~RendererInfoPluginStruct() { /* Empty */ }

    Foundry::Katana::RendererInfo::RendererInfoBase& getRendererInfoPlugin()
    {
        return *_rendererInfoPlugin;
    }

private:
    std::auto_ptr<Foundry::Katana::RendererInfo::RendererInfoBase> _rendererInfoPlugin;
};

///@endcond

#endif
