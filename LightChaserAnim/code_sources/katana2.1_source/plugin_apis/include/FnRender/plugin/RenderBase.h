// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_RENDERBASE_H
#define FNRENDER_RENDERBASE_H

#include "FnScenegraphIterator/FnScenegraphIterator.h"
#include "FnAttribute/FnAttribute.h"

#include "FnPluginSystem/FnPluginSystem.h"
#include "FnPluginSystem/FnPlugin.h"

#include "FnRender/suite/FnRenderPluginSuite.h"
#include "FnRender/plugin/DiskRenderOutputProcess.h"

#include <stdlib.h>

namespace Foundry
{
namespace Katana
{
namespace Render
{
    /**
     * \defgroup RenderAPI Renderer API
     * @{
     *
     * @brief The Renderer API allows the integration of renderers into Katana.
     *
     * \section PluginIntro What is a renderer plug-in?
     * A renderer plug-in is a collection of modules which provide Katana with renderer-specific information
     * that can be configured and declared to the renderer in the UI as well as handling the corresponding
     * Katana recipes at render time. The modules used to implement a renderer plug-in are the following:
     *  - Render::RenderBase: Translates a Katana recipe into the renderer's language using a scene graph
     *    iterator (FnScenegraphIterator), where the rendered content is typically visualised in Katana's
     *    monitor.
     *    A render plug-in represents a single render process where it is instantiated with the Katana recipe
     *    and render arguments when a render is launched and destroyed when the render is complete or cancelled.
     *    The Render plug-in can delegate the handling of scene graph location types to delegate plug-ins
     *    (see Render::ScenegraphLocationDelegate). This enables the registering and handling of new renderer-
     *    specific location types without changing the render plug-in.
     *    See \ref RecipeUtilityClasses "Recipe utility classes".
     *  - RendererInfo::RendererInfoBase: Provides Katana with renderer-specific information needed to configure a
     *    renderable scene, e.g. shaders, render outputs. If applicable, this plug-in also advertises and
     *    configures nodes such as [Renderer]ShadingNode and [Renderer]OutputChannelDefine.
     *    It is also responsible for \ref RenderMethods "advertising supported render methods" such as preview
     *    and disk renders.
     *  - \ref ConfigurationNodes "Configuration nodes": Renderer-specific nodes are useful for configuring
     *    renderable properties such as global and object settings.
     *  - \ref GafferProfiles "Gaffer profile": A Python class which is used to add and configure profiles in the
     *    Gaffer node.
     *  - Viewer manipulators: Enables the user to manipulate shader parameter values in the viewer.
     *    See the
     *    <a href="../../python_apis/UI4.Tabs.OSGViewerTab.ViewerManipulatorAPI-module.html">ViewerManipulatorAPI</a>
     *    for further information.
     *
     * \section RenderMethods Advertising render methods to start a render process
     * A renderer plug-in must advertise one or more supported render methods which become available in
     * the UI (unless explicitly hidden) and script mode. Render methods are created by adding instances of render
     * method classes to the vector passed into RendererInfo::RendererInfoBase::fillRenderMethods.
     * The base render method classes represent different ways of handling a render process in Katana:
     *  - RendererInfo::DiskRenderMethod: The render plug-in can query all the render outputs which
     *                                    have been declared using RenderOutputDefine nodes. Each output
     *                                    has a target location where it is intended to be written to disk.
     *                                    The primary output will be loaded and displayed in the monitor.
     *      - See \ref DiskImagesToKatana "Rendering one or more render outputs to disk".
     *  - RendererInfo::PreviewRenderMethod: Preview renders are sent directly from the renderer to the monitor
     *    using an inter-process communication protocol (IPC) where each bucket can be sent interactively
     *    (and progressively). Preview renders can display multiple AOVs in the monitor but only one view.
     *      - See \ref PreviewImagesToKatana "Sending rendered images to Katana using the Display Driver API".
     *  - RendererInfo::LiveRenderMethod: A live render follows the same principles as a preview render in terms of
     *    how the render is launched and how the rendered content is delivered to Katana.
     *    The difference is that the render process is kept alive to allow live attribute updates, which are sent
     *    to the render plug-in using IPC.
     *      - See the \ref RenderProcess "key stages of the render process" and how they affect live rendering.
     *      - See Render::RenderBase::queueDataUpdates.
     *
     * Each render method can \ref DebugOutput "advertise and write render debug outputs" to a file which are
     * displayed in an external editor.
     *
     * Render methods can be hidden from the UI using RendererInfo::RenderMethod::setVisible where it can
     * still be used in script mode when launching a render process with StartRender.
     *
     * A render method can customise the way a render is launched in Katana. By default, a render process
     * will create a catalog item, register the render so that it may be cancelled etc., and report any
     * render messages in the render log. These properties can be configured to launch a detached render where
     * Katana will absolve itself from any involvement beyond launching the render process with the recipe.
     * This is done by passing false into the following functions:
     * - RendererInfo::RenderMethod::setCreateCatalogItem
     * - RendererInfo::RenderMethod::setReportRenderMessages
     * - RendererInfo::RenderMethod::setRegisterRender
     *
     * Preview render methods support concurrent renders which can be switched on using
     * RendererInfo::PreviewRenderMethod::setAllowConcurrentRenders.
     *
     * \subsection BatchRendering Batch rendering
     * Katana requires a batch render method so the RendererInfoBase automatically creates a DiskRenderMethod
     * with default parameters which will be used for batch rendering. The parameters can be overridden in
     * RendererInfoBase::configureBatchRenderMethod.
     *
     * \subsection RenderMethodsUI Launching render methods in the UI
     * The render methods are grouped in Katana's render menu based on their type. The method name is passed
     * as an argument to the render process where the render plug-in can retrieve it using
     * Render::RenderBase::getRenderMethodName.
     *
     * <b>Tip:</b> It is useful to use the default name and label provided by the render method classes for
     * the most common cases as the render menu collapses items whose corresponding methods share the
     * same name and label. This is done to allow simplifying the render menu where multiple renderers
     * have equivalent render methods. Furthermore, standardising the user experience across renderers
     * makes it easier for users to find the render method they want to launch.
     *
     * \section StartingRenders Starting a render
     * A render plug-in must implement the Render::RenderBase::start function which is called at the start of
     * Katana's render process. The plug-in has immediate access to:
     *   - The Katana recipe which is traversed using a FnScenegraphIterator, where the root
     *     iterator is retrieved using Render::RenderBase::getRootIterator.
     *     Utility classes and functions are available which assist with the scene graph traversal and attribute
     *     handling (see \ref RecipeTraversal "Processing the Katana recipe").
     *   - Render arguments consisting of the render method name which was used to launch the render process,
     *     render time (frame), etc.
     *     These arguments can be retrieved either directly as strings using Render::RenderBase::findArgument,
     *     or alternatively using explicit wrapper functions such as Render::RenderBase::getRenderMethodName
     *     and Render::RenderBase::getRenderTime which returns the render time as a float value.
     *
     * \section RenderProcess Key stages of the render process
     * Implementing only the start function is sufficient for most renders.
     * More flexibility is needed for live renders where the render process is kept alive and
     * updates/commands can be sent to the render plug-in until the render is either stopped by the user,
     * or the Katana session ends.
     * This flexibility is provided by supporting optional functions which are called at different points
     * during the render process:
     *  - Render::RenderBase::start: Called to start the render process. This is the only function that must
     *                               be implemented by the render plug-in.
     *  - Render::RenderBase::pause: Pause the render/live render process. Currently it is only called during
     *                               a live render when updating regions of interest.
     *  - Render::RenderBase::stop: Always called at the end of the render process.
     *  - Render::RenderBase::queueDataUpdates: Called asynchronously in a separate thread during a live render
     *                                  with the attributes that have changed.
     *                                  The updates can either be applied directly or queued here and applied
     *                                  later in the main thread.
     *                                  When and how often this function is called depends on the live render mode:
     *     - Manual: The queueDataUpdates function is called when the user explicitly clicks on 'Update Live Render'
     *               in the Live Render Control tab.
     *     - Pen-Up: The queueDataUpdates function is called after an attribute value has changed anywhere in the
     *               scene graph.
     *     - Continuous: The queueDataUpdates function is called with a fixed frequency (every 10ms) if
     *                   attribute values have changed.
     *                   Currently this only applies to camera and light transform updates in the viewer.
     *  - Render::RenderBase::hasPendingDataUpdates: Called after live update data or command has been processed
     *                                               asynchronously to inform the render process of whether some
     *                                               or all of the data updates have not been applied in the
     *                                               queueDataUpdates function.
     *  - Render::RenderBase::applyPendingDataUpdates: Called if hasPendingDataUpdates returns true. This is used
     *                                                 to facilitate a workflow where a set of updates are queued
     *                                                 in the update thread and then flushed in the main thread.
     *  - Render::RenderBase::processControlCommand: Called when a custom live render command is executed.
     *  - Render::RenderBase::stopLiveEditing: Called when a live render is stopped by the user.
     *
     * \section RecipeTraversal Processing the Katana recipe and utility classes
     * Interpreting the Katana recipe into the renderer's language involves a deferred evaluation of the
     * scene graph. This is done by traversing the scene graph using a FnScenegraphIterator starting from the
     * root. The render plug-in can get the root iterator using Render::RenderBase::getRootIterator.
     * Each iterator corresponds to a scene graph location which contains a set of corresponding attributes.
     * Scene graph location types and attributes are based on conventions where the render plug-in can
     * selectively handle location types and attributes that are applicable to the renderer.
     *
     * \subsection SGDelegates Scene graph delegates
     * The handling of scene graph locations and their attributes can be dynamically delegated to
     * Render::ScenegraphLocationDelegate plug-ins.
     * Given an arbitrary scene graph iterator (e.g. FnScenegraphIterator sgi), we can check
     * if a corresponding delegate plug-in exists and invoke it using RenderOutputUtils::processLocation
     * \code
     *   bool pluginFound = RenderOutputUtils::processLocation(sgi, "myrenderer", sgi.getType(), 0x0, 0x0);
     * \endcode
     *
     * \subsection RecipeUtilityClasses Scene graph utility classes
     * Utility classes are provided to parse standardised attribute conventions where they can be extended to
     * provide renderer-specific behaviour through overrides:
     *  - Render::RenderSettings: Parses <b>renderSettings</b> at <b>/root</b>.
     *  - Render::CameraSettings: Parses <b>camera.geometry</b> at <b>/root/world/cam/[cameraName]</b>.
     *  - Render::GlobalSettings: Parses <b>[rendererName]GlobalStatements</b> at <b>/root</b>
     *                            (see \ref GlobalSettings "Configuring global settings").
     *  - RenderOutputUtils: A collection of useful utility functions.
     *
     *  <b>Tip:</b> Avoid using the iterator function getByPath to get an iterator for a location that has
     *              already been traversed as it will traverse the scene graph again from the root. It is
     *              generally preferable to use a cached iterator class unless the memory footprint is a
     *              critical issue.
     *
     * \section DiskImagesToKatana Disk Render: Rendering one or more render outputs to disk
     * Disk renders can only be launched from a render node and the renderer has to provide a target filename
     * for each render output (port).
     * The rendered target file for the primary render output is read and displayed in the catalog and monitor
     * when the render is complete (if the file type is supported).
     *
     * There are two steps required to configure and perform a disk render:
     *  - Configure how the render outputs are handled. This includes where they are rendered to, whether
     *    they should be displayed in the monitor, what pre- and post-processes are required, etc. This is
     *    done in Render::RenderBase::configureDiskRenderOutputProcess which is called for each output where
     *    the type of Render::RenderAction and its properties governs the workflow for the render output.
     *  - Loop over and process each render output in the render plug-in. The render outputs are parsed in
     *    Render::RenderSettings where they are retrieved using e.g. Render::RenderSettings::getRenderOutputs.
     *
     * Following these steps, a simple workflow for a <i>color</i> pass which renders to a temporary location
     * where the file is then copied to the target location could be as follows (note that the following
     * code snippet omits the Foundry::Katana namespace for simplicity):
     *
     * <b>configureDiskRenderOutputProcess</b>
     * \code
     *   // Get the render outputs from the render settings
     *   RenderSettings renderSettings(getRootIterator());
     *   RenderSettings::RenderOutput output = renderSettings.getRenderOutputByName(outputName);
     *
     *   // Generate a unique temporary local output location used by the renderer
     *   std::string tempRenderLocation = RenderOutputUtils::buildTempRenderLocation(
     *                                        rootIterator, outputName, "render", "exr", frameTime);
     *
     *   // Use the path from the location plug-in as a final target location which is displayed in the monitor
     *   std::string targetRenderLocation = outputPath;
     *
     *   // Declare the render action used for this render output
     *   std::auto_ptr<Render::RenderAction> renderAction;
     *
     *   // Determine the rendering behaviour based on the output type
     *   if (output.type == "color")
     *   {
     *       // Here we render to a temporary location and then convert and copy it to the final location
     *       renderAction.reset(new Render::CopyAndConvertRenderAction(targetRenderLocation,
     *                                                                 tempRenderLocation,
     *                                                                 output.clampOutput,
     *                                                                 output.colorConvert,
     *                                                                 output.computeStats,
     *                                                                 output.convertSettings));
     *   }
     * \endcode
     *
     * The attributes for each output are found at the root of the scene graph under
     * <i>renderSettings.outputs</i>.
     * The easiest way to get the attribute values for a particular render output is to use
     * Render::RenderSettings::getRenderOutputByName.
     *
     * <b>start (or any function which processes the render outputs)</b>
     * \code
     *   ...
     *   // Get all render outputs for a disk render
     *   RenderOutputs outputs = renderSettings.getRenderOutputs();
     *
     *   // Iterate through and process each output
     *   ...
     *   if (output.type == "color")
     *   {
     *       // Get the render location which in this case is a temporary location
     *       // as defined by the render action
     *       std::string renderLocation = output.renderLocation;
     *       ...
     *   }
     * \endcode
     *
     * \note Disk renders do not support displaying arbitrary output variables (AOVs) in the
     *       monitor.
     * \note Katana's monitor currently only supports images of type EXR, CIN, RLA, and OIIO.
     *
     * @see Render::RenderBase::configureDiskRenderOutputProcess
     * @see Render::DiskRenderOutputProcess
     * @see Render::RenderAction
     * @see Render::CopyRenderAction
     * @see Render::CopyAndConvertRenderAction
     * @see Render::TemporaryRenderAction
     * @see Render::NoOutputRenderAction
     * @see Render::RenderSettings::RenderOutputs
     * @see Render::RenderSettings::getRenderOutputs
     *
     * \section PreviewImagesToKatana Preview Render: Rendering directly to Katana's monitor
     * A preview render can be launched from any node in the UI where the renderer sends the rendered
     * content (e.g. buckets) interactively to the monitor as soon as they are ready using the
     * \ref DD "Display Driver API".
     *
     * Katana starts a listener when the UI is launched which retrieves image and ID data over a
     * communication protocol. Preview renders have no concept of render outputs but instead use
     * channel buffers that contain the names of the selected outputs and the corresponding buffer
     * IDs. The IDs represent buffers that are reserved in Katana's catalog and they are used by
     * the display driver to make sure the image channel data goes to the right buffer.
     *
     * The following exmample demonstrates how the channel buffers are queried from the render
     * settings (note that the following code snippet omits the Foundry::Katana namespace for
     * simplicity):
     *
     * \code
     *   RenderSettings::ChannelBuffers buffers;
     *   renderSettings.getChannelBuffers( buffers );
     *
     *   RenderSettings::ChannelBuffers::const_iterator it;
     *   for( it = buffers.begin(); it != buffers.end(); ++it )
     *   {
     *       RenderSettings::ChannelBuffer buffer = it->second;
     *       // Use buffer.bufferId and buffer.channelName with the Display Driver API
     *       ...
     *   }
     * \endcode
     *
     * \note Katana's monitor currently only supports images of type EXR, CIN, RLA, and OIIO.
     *
     * @see Render::RenderSettings::ChannelBuffer
     * @see Render::RenderSettings::getChannelBuffers
     * @see RendererInfo::RenderMethod
     *
     * \section DebugOutput Generating a render debug output
     * There are two steps involved in creating a render debug output:
     * - Advertise that a render method supports debug outputs. This creates the necessary UI hooks to allow
     *   the user to ask for a debug output.
     * - Check for specific render arguments in the render plug-in which indicate that the render process was
     *   triggered with the expectation that a render debug output file is created.
     *
     * \subsection DebugAdvertise Advertising debug output support
     * The most common way of requesting a debug output in Katana is through a node's context menu under
     * 'Debugging'.
     * The batch render method has to support debug outputs in order to add a menu item for the renderer.
     * This is done using RendererInfo::RenderMethod::setDebugOutputSupported. The output file type is set
     * using RendererInfo::RenderMethod::setDebugOutputFileType.
     * A menu item in the render context menu is added if support for a debug output is enabled where the
     * label is formatted as <i>"Open .[fileType] Output in [editor]"</i>.
     * The menu item launches a render process with a render argument which specifies a target location
     * for the render debug output (this is discussed more in the next section).
     *
     * Disk/batch renders do not support a partial scene graph so a previewRender method is used if
     * 'Render Only Selected Objects' is enabled in the scene graph tab. A partial scene graph here will
     * still include the root and world locations so it is conceptually identical to a full scene graph.
     *
     * A different type of partial scene graph output can be obtained using a scene graph location's
     * context menu under 'Debugging' where the menu item follows the same signature as the one above.
     * This option is enabled using RendererInfo::RenderMethod::setSceneGraphDebugOutputSupported and
     * RendererInfo::RenderMethod::setDebugOutputFileType as before. A current limitation is that only
     * one render method can support this feature.
     *
     * A debug output can be created in script mode using the following functions in the NodeDebugOutput
     * module:
     * - WriteRenderOutput(node, renderer, filename=None, expandProcedural=True, openInEditor=False,
     *                     customEditor=None, log=None)
     * - WriteRenderOutputForRenderMethod(renderMethodName, node, renderer, filename=None,
     *                                    expandProcedural=True, log=None, openInEditor=True,
     *                                    customEditor=None, sceneGraphPath=None,
     *                                    printToConsole=False)
     *
     * The former method always uses the batch render method and runs synchronously (blocking).
     * The latter one supports any render method and runs asynchronously (non-blocking).
     *
     * \subsection DebugArguments Creating a debug output in the render plug-in
     * We know that our render plug-in should create a debug output if 'renderOutputFile' was passed
     * as an argument to the render process (renderboot). The argument's value can be retrieved using
     * Render::RenderBase::getRenderOutputFile where the value specifies the target location of the
     * debug output. Katana will by default open the debug output file when the render process has
     * finished.
     *
     * Another render argument is passed which indicates whether the bounded procedurals should be
     * expanded which is queried using Render::RenderBase::isExpandProceduralActive.
     *
     * \note A debug output will not cancel a preview render which is in progress. However, a
     *       non-concurrent preview render will cancel a debug output process. A full debug
     *       output runs a synchronous disk render by default and therefore blocks the UI.
     *       A partial debug output using 'Render Only Selected Objects' runs an asynchronous
     *       preview render which does not block the UI.
     *
     * \section DefaultRenderer Setting the plug-in as the default renderer in Katana
     * Setting the default renderer in Katana is done using the environment variable
     * DEFAULT_RENDERER=[render plug-in name].
     *
     * \section LinkingPlugins Linking the plug-in to its corresponding renderer info plug-in
     * The name used to register the render plug-in should be returned in the renderer info plug-in through
     * RendererInfoBase::getRegisteredRendererName.
     * This tells Katana how to namespace attribute data provided by the renderer info plug-in (e.g. shaders),
     * in such as way that it can be easily retrieved by the render plug-in.
     *
     * \section ConfigurationNodes Adding configuration nodes
     * GenericAssign nodes are useful to expose a set of scoped attributes which can be used to specify
     * renderer-specific properties such as global settings. These nodes are created using an XML file
     * which contains the attributes and corresponding UI hints, as well as the scene graph scope.
     * The XML file has to reside in a directory called 'GenericAssign' where it will be picked up and
     * used to automatically register a node whose name matches the XML filename.
     *
     * \subsection GlobalSettings Configuring global settings ([rendererName]GlobalSettings)
     * A typical use for a configuration node is to declare global options which apply during the render
     * process. The convention in Katana is to scope these global settings at the root of the scene graph
     * under a group attribute named <b>[rendererName]GlobalStatements</b>.
     * A fixed CEL statement is used to prevent users from being able to scope the global attributes
     * arbitrarily. The following example shows how the GenericAssign node is configured to get this
     * behaviour, as well as providing a simple attribute setup which utilises widget hints and conditional
     * visibility:
     *
     * \code
     * <args format='1.0' scope='/root' fixedCEL='/root'>
     *   <group name='myrendererGlobalStatements' hideTitle='True' closed='True' groupInherit='False'>
     *     <group name='bucket' closed='True'>
     *       <string name='order' default='horizontal' widget='popup'>
     *         <hintlist name="options">
     *           <string value="horizontal"/>
     *           <string value="vertical"/>
     *           <string value="spiral"/>
     *         </hintlist>
     *         <help>Some help text</help>
     *       </string>
     *
     *       <int name='orderorigin' default='0, 0' size='2' tupleSize='2'
     *            conditionalVisOp='equalTo' conditionalVisPath='../order' conditionalVisValue='spiral'/>
     *     </group>
     *     ...
     *   </group>
     * </args>
     * \endcode
     *
     * Here, we declared a closed group which encapsulates all bucket settings where a popup list offers
     * a choice of three bucket orders and a conditional property is shown if the 'spiral' value is
     * selected.
     *
     * The Render::GlobalSettings utility class can be extended where it retrieves the global statements
     * based on Katana's naming convention. The class can then freely be used to parse the attribute data
     * and encapsulate global settings related functions.
     *
     * \subsection ObjectSettings Configuring object settings ([rendererName]ObjectSettings)
     * GenericAssign nodes can also be used to configure attributes for arbitrary scene graph locations
     * based on a CEL statement. This allows assigning attribute data on a per object basis while
     * traversing the scene graph. By convention, these attributes can be assigned to any scene graph
     * location below (and including) the world location, where the attributes are grouped under
     * <b>[rendererName]Statements</b> and can be inherited.
     * The following example shows how to configure the scope as well as setting up a page which
     * encloses a set of attributes (differently to a group attribute):
     *
     * \code
     * <args format='1.0' scope='/root/world//\* /root/world'>
     *   <group name='myrendererStatements' hideTitle='True' closed='True'>
     *     <page name="Geometry">
     *       <int name='invert_normals' default='0' widget='checkBox'>
     *         <help>
     *           Some help text.
     *         </help>
     *       </int>
     *       ...
     *     </page>
     *     ...
     *   </group>
     * </args>
     * \endcode
     *
     * \section GafferProfiles Gaffer profiles
     * A Gaffer profile class has to be provided in order to use the Gaffer node with the renderer.
     * The class should reside in a 'Plugins' directory where it extends the 'Profile' class and has
     * to provide an implementation for:
     * - getRendererName
     * - getLightShaderType
     * - getMaterialParameterName
     * - getMaterialParameterValue
     *
     * Example:
     * \code
     * from PluginAPI.BaseGafferProfile import Profile
     *
     * class MyRendererProfile(Profile):
     *     def getRendererName(self):
     *         return "myrenderer"
     *
     *     def getLightShaderType(self):
     *         return "light"
     *
     *     def getMaterialParameterName(self):
     *         return "myrendererLightShader"
     *
     *     def getMaterialParameterValue(self):
     *         return "myrenderer.light"
     * \endcode
     *
     * \section IdPasses Sending ID pass data to Katana
     * There are two aspects in terms of getting ID pass data to Katana:
     *  - Assign an integer value to a renderable scene graph location and advertise the mapping
     *    between the two to Katana.
     *  - Render the ID pass and send the single channel image to Katana.
     *
     *  The latter is naturally expressed as a channel in the \ref DD "Display Driver API" so this
     *  section will focus on the former.
     *
     *  Render::IdSenderInterface is a utility class which handles the ID communication with Katana.
     *  It is used to both get a unique integer ID from Katana as well as sending the ID values and
     *  their corresponding scene graph location names to Katana.
     *
     *  <b>Example</b>
     *  \code
     *    int64_t nextId;
     *    int64_t maxId;
     *
     *    // Get an instance of the ID sender interface from the factory
     *    Render::IdSenderInterface* idSender;
     *    idSender = Render::IdSenderFactory::getNewInstance(getKatanaHost(), bufferID);
     *
     *    // Get the next unique ID integer value
     *    idSender->getIds(nextId, maxId);
     *
     *    // Use nextId with the scene graph iterator (sgi) location
     *
     *    // Send the ID and scene graph location path
     *    idSender->send(nextId, sgi.getFullPath().c_str());
     *  \endcode
     *
     *  Here the <i>bufferID</i> is retrieved from the RenderSettings::ChannelBuffer, e.g.:
     *  \code
     *    RenderSettings::ChannelBuffers buffers;
     *    renderSettings.getInteractiveBuffers(buffers);
     *
     *    // Here we assume the first channel buffer is valid and used
     *    int64_t frameId = convertToInt(buffers[0].bufferId);
     *  \endcode
     *
     * \section RenderThreads Setting the number of render threads
     * There are two conventions used in Katana to declare the number of render threads for a
     * launched render process:
     * - Using a renderSettings.renderThreads attribute.
     *   The function Render::RenderSettings::applyRenderThreads can be used to apply the value
     *   if it has been set.
     * - UI preferences and batch command line argument. Both override values are sent to the
     *   render process as a 'threads' argument where it can be applied using
     *   Render::RenderBase::applyRenderThreadsOverride.
     *   Note that this value should always override every other thread value as the user has
     *   explicitly requested it.
     *    - UI mode: The <i>interactiveRenderThreads3D</i> value is sent to the render process
     *               if <i>interactiveRenderThreadOverride</i> is set to <i>Yes</i> in the
     *               Preferences dialog.
     *    - Batch mode: The thread count is set using the <i>--threads3d</i> argument when
     *                  launching Katana where the value is sent to the render process.
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
    class RenderBase
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
