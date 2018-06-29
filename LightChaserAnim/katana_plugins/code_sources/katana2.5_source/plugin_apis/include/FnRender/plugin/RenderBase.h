// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_RENDERBASE_H
#define FNRENDER_RENDERBASE_H

#include <stdlib.h>

#include "FnAttribute/FnAttribute.h"

#include "FnPluginSystem/FnPlugin.h"
#include "FnPluginSystem/FnPluginSystem.h"

#include "FnRender/FnRenderAPI.h"
#include "FnRender/plugin/DiskRenderOutputProcess.h"
#include "FnRender/suite/FnRenderPluginSuite.h"

#include "FnScenegraphIterator/FnScenegraphIterator.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{
    /**
     * \defgroup RenderAPI Renderer API
     * @{
     */

    /**
     * @brief The base class for the render plug-in which takes a Katana recipe and converts
     *        it into the renderer's language.
     *
     * A render plug-in represents a single render process (for a single frame) where the
     * plug-in is instantiated when a render is launched and destroyed when the render is
     * complete or cancelled.
     * The Katana recipe is presented to the render plug-in in the form of a FnScenegraphIterator
     * which allows traversing the scene graph in a deferred manner. The root scene graph
     * iterator is passed to the plug-in's constructor where it is accessible anywhere in
     * the plug-in through Render::RenderBase::getRootIterator.
     * The render arguments are also passed to the constructor but we recommend using the
     * wrapper functions to retrieve the standard arguments.
     *
     * The plug-in has to extend the Render abstract class and register itself using the
     * <i>DEFINE_RENDER_PLUGIN(class)</i> and
     * <i>REGISTER_PLUGIN(class, name, major_version, minor_version)</i>
     * macros.
     * This will ensure the plug-in is exposed in the UI by its registered name as well as
     * making it available to Katana's render process.
     */
    class FNRENDER_API RenderBase
    {
    public:
        /**
         * The render plug-in class is instantiated by the renderboot process which is launched when
         * a render is started through the UI or using the RenderManager.StartRender script function.
         *
         * @param rootIterator  The root FnScenegraphIterator used to traverse the Katana recipe
         * @param arguments     Render arguments (e.g. render time)
         *
         * @see Render::RenderBase::getRootIterator
         */
        RenderBase(FnScenegraphIterator rootIterator, FnAttribute::GroupAttribute arguments) : _rootIterator(rootIterator),
                                                                                               _arguments(arguments) {}
        virtual ~RenderBase() {}

        /**
         * Start a render by traversing the scene graph using FnScenegraphIterator and interpret the
         * locations and attributes into the renderer's language. This function is called at the start
         * of the render process.
         *
         * @return A zero value if successful, a non-zero value otherwise.
         */
        virtual int start() = 0;

        /**
         * Interrupt and pause the render process but does not terminate it.
         *
         * \note This is only used during live render when updating regions of interest.
         *
         * @return A zero value if successful, a non-zero value otherwise.
         */
        virtual int pause() { return 0; }

        /**
         * Resume a paused render.
         *
         * \note This is currently never called.
         *
         * @return A zero value if successful, a non-zero value otherwise.
         */
        virtual int resume() { return 0; }

        /**
         * Called at the end of the render process.
         *
         * @return A zero value if successful, a non-zero value otherwise.
         */
        virtual int stop() { return 0; }

        /**
         * Start the live render process.  Data updates and control commands will follow.
         *
         * @return A zero value if successful, a non-zero value otherwise.
         */
        virtual int startLiveEditing() { return 0; }

        /**
         * Stop the live render process.  No further data updates or control commands should be expected.
         *
         * @return A zero value if successful, a non-zero value otherwise.
         */
        virtual int stopLiveEditing() { return 0; }

        /**
         * Process a custom live render control command which has been triggered by the user in the live
         * update tab.
         *
         * @param command   Live render control command
         *
         * @return A zero value if successful, a non-zero value otherwise.
         */
        virtual int processControlCommand(const std::string & command) { return 0; }

        /**
         * Process a data update during live render.
         * The update is based on an attribute convention where an update for a single scene graph location
         * is encapsulated by a GroupAttribute. Each attribute update contains the following:
         *  - type: root, globals, camera, light, geoMaterial, or a custom type declared as
         *          a LiveRenderAPI plugin.
         *  - location: The full scene graph location name
         *  - attributes: A GroupAttribute containing the update attributes which vary based on the type:
         *     - root: camera (StringAttribute containing the selected camera)
         *     - globals: [rendererName]GlobalStatements
         *     - camera: xform, geometry
         *     - light: xform, material, geometry, mute, info
         *     - geoMaterial: material, info
         *     - (custom): Depends on what attributes were declared for the custom type.
         *
         * \note
         * A useful trick while debugging is to print the XML string for the update attribute
         * to see its contents: updateAttribute.getXML();
         *
         * \note
         * This function is called from a separate update thread whereas hasPendingDataUpdates
         * and applyPendingDataUpdates are called in the main render process thread.
         *
         * @param updateAttribute   A group attribute containing one or more update attributes.
         *
         * @return A zero value if successful, a non-zero value otherwise.
         *
         * @see Render::RenderBase::hasPendingDataUpdates
         * @see Render::RenderBase::applyPendingDataUpdates
         */
        virtual int queueDataUpdates(FnAttribute::GroupAttribute updateAttribute) { return 0; }

        /**
         * Apply one or more live data updates which have been processed by queueDataUpdates.
         * This provides an opportunity to process and queue live update attributes in the
         * update thread and then flush them in the main thread.
         *
         * This function is called if hasPendingDataUpdates returns true.
         *
         * @return A zero value if successful, a non-zero value otherwise.
         *
         * @see Render::RenderBase::hasPendingDataUpdates
         * @see Render::RenderBase::queueDataUpdates
         */
        virtual int applyPendingDataUpdates() { return 0; }

        /**
         * Inform the render process whether it has to flush and apply live updates.
         *
         * @return true if there are updates which have to be applied, false otherwise
         *
         * @see Render::RenderBase::applyPendingDataUpdates
         * @see Render::RenderBase::queueDataUpdates
         */
        virtual bool hasPendingDataUpdates() const { return false; }

        /**
         * Provide Katana with information on how to process a given render output which has been defined
         * in a Katana scene using the <i>RenderOutputDefine</i> node.
         * This is only applicable during disk render where the function is called for each render output
         * (port) on the render node.
         *
         * \note
         * The list of available output types is provided by
         * RendererInfo::RendererInfoBase::buildRendererObjectInfo
         * when Katana calls the function using <i>kFnRendererObjectTypeRenderOutput</i> as an
         * object type.
         *
         * @param diskRenderOutputProcess Defines the render action for a given render output as
         *                                well as pre- and post-commands used in the render process.
         * @param outputName              The name of the render output
         *                                (as defined in the corresponding
         *                                <i>RenderOutputDefine</i> node).
         * @param outputPath              The target location for the render output.
         * @param renderMethodName        The render method used to launch the render.
         * @param frameTime               The current frame time.
         *
         * @see Render::DiskRenderOutputProcess
         * @see Render::RenderAction
         * @see Render::CopyRenderAction
         * @see Render::CopyAndConvertRenderAction
         * @see Render::TemporaryRenderAction
         * @see Render::NoOutputRenderAction
         * @see Render::RenderSettings::RenderOutputs
         * @see Render::RenderSettings::getRenderOutputs
         * @see RendererInfo::RendererInfoBase::buildRendererObjectInfo
         */
        virtual void configureDiskRenderOutputProcess(DiskRenderOutputProcess& diskRenderOutputProcess,
                                                      const std::string& outputName,
                                                      const std::string& outputPath,
                                                      const std::string& renderMethodName,
                                                      const float& frameTime) const {}

        // non-virtual implementations to do conversions from virtual functions above
        FnAttribute::Attribute _configureDiskRenderOutputProcess(const std::string& outputName,
                                                    const std::string& outputPath,
                                                    const std::string& renderMethodName,
                                                    const float& frameTime) const;

        static FnPlugStatus setHost(FnPluginHost* host);
        static FnPluginHost* getHost();

        static FnRenderPluginSuite_v1 createSuite(FnRenderPluginHandle (
                *create)(FnSgIteratorHandle, FnAttributeHandle));
        static FnRenderPluginHandle newRenderHandle(RenderBase* render);

        static unsigned int _apiVersion;
        static const char*  _apiName;

    protected:
        /**
         * The scene graph iterator is used to traverse the scene graph and
         * author data to the renderer based on the scene graph type and
         * attributes. The root iterator provides a starting point for the
         * traversal as it contains the entire scene graph as well as any
         * global scene attributes which are by convention assigned to the
         * root location.
         *
         * @return The iterator for the root scene graph location
         */
        FnScenegraphIterator getRootIterator() const { return _rootIterator; }

        /**
         * Retrieve the render method used to launch the render from the render arguments.
         * The method name is passed to the RenderManager.StartRender function where the list
         * of available methods is declared by RendererInfo::RendererInfoBase::fillRenderMethods
         * in UI mode, and by configureBatchRenderMethod in batch mode, when creating debug outputs,
         * and when rendering dependencies.
         *
         * @return The render method name used to launch the render process
         *
         * @see RendererInfo::RendererInfoBase::fillRenderMethods
         * @see RendererInfo::RendererInfoBase::configureBatchRenderMethod
         */
        std::string getRenderMethodName() const;

        /**
         * Retrieve a render argument by name. The standard render arguments are accessible through
         * explicit functions but custom live render arguments which have been advertised in the
         * live render tab using the startConfiguration policy type.
         *
         * @param argumentName  The name of the render argument that should be retrieved
         *
         * @return The render argument value
         */
        std::string findArgument(const std::string& argumentName, const std::string & defaultValue="") const;

        /**
         * Update the number of render threads if a 'threads' argument is passed
         * as a render argument. The thread value is passed to the render process in the following cases:
         *  - UI mode: The 'interactiveRenderThreads3D' value is used if 'interactiveRenderThreadOverride'
         *             is set to 'Yes' in the Preferences dialog under 'application->rendering'.
         *  - Batch mode: The thread count is set using the '--threads3d' argument when launching Katana.
         *
         * @param numberOfRenderThreads The render thread variable which is subject to the thread override
         * @return true if an override was applied, false otherwise
         */
        bool applyRenderThreadsOverride(int& numberOfRenderThreads) const;

        /**
         * @return The base path for Katana
         */
        std::string getKatanaBasePath() const { return findArgument("katanaRoot"); }

        /**
         * Get the name of the temporary directory Katana creates at startup which is used
         * throughout the session.
         *
         * @return Katana's temporary directory
         */
        std::string getKatanaTempDirectory() const { return findArgument("tempDir"); }

        /**
         * Get the host and port of Katana's listener which is started when Katana is launched
         * in UI mode. The listener is used to retrieve image and id data from the renderer
         * plug-in.
         *
         * @return The host name for Katana's image and id listener
         */
        std::string getKatanaHost() const { return findArgument("host"); }

        /**
         * Get the render time which is either a value retrieved from the timeline (UI mode),
         * or a single frame from the interval provided by the -t command line argument when
         * launching a batch render.
         *
         * @return The render time (frame)
         */
        float getRenderTime() const;

        /**
         * Specifies whether an ID pass should be created. The user guide explains
         * how to activate an ID pass in the monitor and how to use the picker tool.
         *
         * See \ref IdPasses "Sending ID pass data to Katana" for further information.
         *
         * @return true if an ID pass should be created, false otherwise
         *
         * @see Render::IdSenderFactory::getNewInstance
         * @see Render::IdSenderInterface
         */
        bool useRenderPassID() const;

        /**
         * Returns the name of a file which Katana expects the render plug-in to
         * create and populate with scene description data and/or debug information.
         * See \ref DebugOutput "Generating a render debug output".
         *
         * @return The name of the the render debug output file
         */
        std::string getRenderOutputFile() const { return findArgument("renderOutputFile"); }

        /**
         * This function is used in conjunction with getRenderOutputFile where if a render
         * output file is expected to be generated, and if the renderer supports recursive
         * procedurals, then this determines whether the procedurals should be expanded
         * or not.
         *
         * @return true if the procedurals should be expanded, false otherwise
         *
         * @see getRenderOutputFile
         */
        bool isExpandProceduralActive(bool defaultValue=false) const;

        /**
         * Returns the directory name where the render plug-in resides on disk.
         *
         * @return The render plug-in directory
         */
        std::string getRenderPluginDirectory() const { return findArgument("pluginDsoPath"); }

        /**
         * Looks up and returns an environment variable by name.
         *
         * @return The value of the given environment variable name
         */
        std::string getEnvironmentVariable(const std::string& environmentVariableName) const;

        /**
         * A helper function which retrieves and converts the value of an Attribute
         * to a desired standard type. This is useful for instance when collecting
         * custom <i>rendererSettings</i> values from the render outputs, e.g.:
         *
         * \code
         *   int mySettings = getAttrValue<int, IntAttribute>(output.rendererSettings["mySettings"], 0);
         * \endcode
         *
         * @param attr          The Attribute that we want to retrieve the value from
         * @param defaultValue  Default value if the attribute is not valid
         *
         * @return The typed attribute value or the default value if the attribute is not valid
         */
        template <typename T, typename A>
        T getAttrValue(const FnAttribute::Attribute& attr, const T& defaultValue) const
        {
            A typedAttr = attr;
            if(typedAttr.isValid())
                return typedAttr.getValue();

            return defaultValue;
        }

        /**
         * A convenience function for retrieving the value of StringAttribute
         *
         * @param attr          The StringAttribute that we want to retrieve the value from
         * @param defaultValue  Default string value if the attribute is not valid
         *
         * @return The string value contained in the attribute or the default value if the attribute is not valid
         */
        inline std::string getStringAttrValue(const FnAttribute::Attribute& attr, const std::string & defaultValue = "") const
        {
            return getAttrValue<std::string, FnAttribute::StringAttribute>(attr, defaultValue);
        }

    private:
        FnScenegraphIterator _rootIterator;
        FnAttribute::GroupAttribute _arguments;

        static FnPluginHost* _host;
    };
    /**
     * @}
     */
}
}
}

namespace FnKat = Foundry::Katana;

// Plugin Registering Macro.
#define DEFINE_RENDER_PLUGIN(RENDER_CLASS)                               \
                                                                         \
    FnPlugin RENDER_CLASS##_plugin;                                      \
                                                                         \
    FnRenderPluginHandle RENDER_CLASS##_create(FnSgIteratorHandle sg,    \
                                               FnAttributeHandle args)   \
    {                                                                    \
        return Foundry::Katana::Render::RenderBase::newRenderHandle(     \
                RENDER_CLASS::create(                                    \
                        Foundry::Katana::FnScenegraphIterator(sg),       \
                        FnAttribute::Attribute::CreateAndRetain(args))); \
    }                                                                    \
                                                                         \
    FnRenderPluginSuite_v1 RENDER_CLASS##_suite =                        \
            Foundry::Katana::Render::RenderBase::createSuite(            \
                    RENDER_CLASS##_create);                              \
                                                                         \
    const void* RENDER_CLASS##_getSuite()                                \
    {                                                                    \
        return &RENDER_CLASS##_suite;                                    \
    }

#endif
