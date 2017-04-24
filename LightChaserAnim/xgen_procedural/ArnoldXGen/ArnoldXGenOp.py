# A startup file to register the ArnoldXGen Op, if this is in
# $KATANA_RESOURCES/Plugins, it will be exec'd automatically. If its
# in $KATANA_RESOURCES/Startup, it would need to be init.py, or called
# from and existing init.py script.

def registerArnoldXGenOp():
    """
    Registers the ArnoldXGen Op using the NodeTypeBuilder.
    This is a helper class that takes care of registering a suitable
    node graph node type, and configuring it to call the supplied method
    in order to build the Op chain that is used to represent the
    node in the Op Graph.
    """

    from Katana import Nodes3DAPI
    from Katana import FnAttribute, FnGeolibServices

    MODE_RIBBON   = 0
    MODE_THICK    = 1
    MODE_ORIENTED = 2
    modeDict = { 'ribbon'   : MODE_RIBBON,
                 'thick'    : MODE_THICK,
                 'oriented' : MODE_ORIENTED }

    XGEN_FILE = 0
    XGEN_ROOT = 1
    xgenSpecStyle = { 'xgen_file' : XGEN_FILE,
                      'xgen_root' : XGEN_ROOT }

    FROM_ENVIRONMENT = 0
    CUSTOM_PATHS = 1
    customPathsSetting = { 'from environment' : FROM_ENVIRONMENT,
                           'customize paths'  : CUSTOM_PATHS }

    def buildOpChain( node, interface ):
        """
        Configures the Ops to represent the current state of the node.
        The calling mechanism in Node3D takes are of ensuring that
        the underlying Op network is only updated if Op Args and types
        have changed.

        @type node: C{Nodes3DAPI.NodeTypeBuilder.SubdividedSpace}
        @type interface: C{Nodes3DAPI.NodeTypeBuilder.BuildChainInterface}
        @param node: The node to build the Op and OpArgs from (ie: an instance
        of our node).
        @param interface: The interface that will configure the Ops in the
        underlying Op network.
        """

        # Ensure the runtime doesn't error for us if there are no inputs on the
        # Node - as we want to allow it to exist in isolation. The default is 1
        interface.setMinRequiredInputs( 0 )

        # We first need the current time for parameter value queries
        frameTime = interface.getGraphState().getTime()

        # TODO: figure this out so it's automatic
        #fps = interface.getGraphState().getFramesPerSecond()

        # Get the parameters from our node
        locationParam = node.getParameter( "location" )
        xgenRootParam = node.getParameter( "xgen_root" )
        xgenFileParam = node.getParameter( "xgen_file" )
        pathsParam    = node.getParameter( "paths" )
        mayaPathParam = node.getParameter( "maya_path" )
        mtoaPathParam = node.getParameter( "mtoa_path" )
        xgenLocParam  = node.getParameter( "xgen_location" )
        sceneParam    = node.getParameter( "scene" )
        paletteParam  = node.getParameter( "collection" )
        descParam     = node.getParameter( "description" )
        schemeParam   = node.getParameter( "name_scheme" )
        cameraParam   = node.getParameter( "camera" )
        samplesParam  = node.getParameter( "samples" )
        fpsParam      = node.getParameter( "fps" )
        mpwParam      = node.getParameter( "min_pixel_width" )
        modeParam     = node.getParameter( "mode" )
        patchesParam  = node.getParameter( "patches" )
        verboseParam  = node.getParameter( "verbose" )
        specParam     = node.getParameter( "specification" )

        if not locationParam or not (xgenRootParam or xgenFileParam):
            raise RuntimeError( "Missing node parameters, requires 'location' and one of 'xgen_root' or 'xgen_scene' to be set" )

        # copy array param values out to appropriate attributes
        samples = []
        for ci in range(0, samplesParam.getNumChildren()):
            samples = samples + [ samplesParam.getChildByIndex(ci).getValue(frameTime) ]
        if not samples:
            samples = [ 0.0 ]
        opSamplesParam = FnAttribute.FloatAttribute(samples)

        patches = []
        for ci in range(0, patchesParam.getNumChildren()):
            patches = patches + [ patchesParam.getChildByIndex(ci).getValue(frameTime) ]
        if not patches:
            opPatchesParam = FnAttribute.StringAttribute([], 1)
        else:
            opPatchesParam = FnAttribute.StringAttribute(patches)

        # Build the Op args from our node
        argsGb = FnAttribute.GroupBuilder()
        argsGb.set( "patch",         opPatchesParam )
        argsGb.set( "samples",       opSamplesParam )
        argsGb.set( "specification", FnAttribute.IntAttribute(specParam.getValue(frameTime)) )
        argsGb.set( "xgen_root",     FnAttribute.StringAttribute(xgenRootParam.getValue(frameTime)) )
        argsGb.set( "xgen_file",     FnAttribute.StringAttribute(xgenFileParam.getValue(frameTime)) )
        if pathsParam.getValue(frameTime) == CUSTOM_PATHS:
            argsGb.set( "maya_path",     FnAttribute.StringAttribute(mayaPathParam.getValue(frameTime)) )
            argsGb.set( "mtoa_path",     FnAttribute.StringAttribute(mtoaPathParam.getValue(frameTime)) )
            argsGb.set( "xgen_location", FnAttribute.StringAttribute(xgenLocParam.getValue(frameTime)) )
        else:
            argsGb.set( "maya_path",     FnAttribute.StringAttribute('') )
            argsGb.set( "mtoa_path",     FnAttribute.StringAttribute('') )
            argsGb.set( "xgen_location", FnAttribute.StringAttribute('') )
        argsGb.set( "scene",           FnAttribute.StringAttribute(sceneParam.getValue(frameTime)) )
        argsGb.set( "palette",         FnAttribute.StringAttribute(paletteParam.getValue(frameTime)) )
        argsGb.set( "description",     FnAttribute.StringAttribute(descParam.getValue(frameTime)) )
        argsGb.set( "name_scheme",     FnAttribute.StringAttribute(schemeParam.getValue(frameTime)) )
        argsGb.set( "camera",          FnAttribute.StringAttribute(cameraParam.getValue(frameTime)) )
        argsGb.set( "min_pixel_width", FnAttribute.FloatAttribute(mpwParam.getValue(frameTime)) )
        argsGb.set( "fps",             FnAttribute.FloatAttribute(fpsParam.getValue(frameTime)) )
        argsGb.set( "verbose",         FnAttribute.IntAttribute(verboseParam.getValue(frameTime)) )
        argsGb.set( "frame",           FnAttribute.IntAttribute(int(frameTime)) )
        argsGb.set( "mode",            FnAttribute.IntAttribute(modeParam.getValue(frameTime)) )

        # We want to use the StaticSceneCreate Op to build the parent
        # hierarchy, so that our op only has to worry about generating its
        # children. Its args are somewhat complex, but fortunately, there
        # is a helper class that makes it all much easier.

        rootLocation = locationParam.getValue( frameTime )

        sscb = FnGeolibServices.OpArgsBuilders.StaticSceneCreate()
        sscb.addSubOpAtLocation( rootLocation, "ArnoldXGen", argsGb.build() )

        interface.appendOp( "StaticSceneCreate", sscb.build() )


    # Here we need to define the parameters for the node, and register the op
    # chain creation callback function

    nodeBuilder = Nodes3DAPI.NodeTypeBuilder( "ArnoldXGen" )

    # Merge with incoming scene, simply allow the node to have an input. Unless
    # you delete locations in your Op, any existing locations will pass-through.
    # We need the input to allow the camera to be used
    nodeBuilder.setInputPortNames( ("in",) )

    # Parameters can be described by a group attribute
    paramGb = FnAttribute.GroupBuilder()
    paramGb.set( "location", FnAttribute.StringAttribute("/root/world/geo/xgen") )
    # Can specify these paths, but if empty they will fall back on MAYA_PATH and MTOA_PATH environment vars
    paramGb.set( "paths", FnAttribute.IntAttribute(FROM_ENVIRONMENT) )
    paramGb.set( "maya_path", FnAttribute.StringAttribute("") )
    paramGb.set( "mtoa_path", FnAttribute.StringAttribute("") )
    paramGb.set( "xgen_location", FnAttribute.StringAttribute("") )
    paramGb.set( "specification", FnAttribute.IntAttribute(XGEN_FILE) )
    paramGb.set( "xgen_root", FnAttribute.StringAttribute("") )  # hidden for XGEN_FILE
    paramGb.set( "xgen_file", FnAttribute.StringAttribute("") )  # hidden for XGEN_ROOT
    paramGb.set( "scene", FnAttribute.StringAttribute("") )      # hidden for XGEN_FILE
    paramGb.set( "collection", FnAttribute.StringAttribute("") ) # hidden for XGEN_FILE
    paramGb.set( "description", FnAttribute.StringAttribute("") )
    paramGb.set( "patches", FnAttribute.StringAttribute([ ], 1) )
    paramGb.set( "name_scheme", FnAttribute.StringAttribute("<scene>_<collection>_<description>_<patch>") )
    paramGb.set( "camera", FnAttribute.StringAttribute( "/root/world/cam/camera" ) )
    paramGb.set( "fps", FnAttribute.FloatAttribute( 24.0 ) )
    paramGb.set( "samples", FnAttribute.FloatAttribute([ ], 1) )
    paramGb.set( "min_pixel_width", FnAttribute.FloatAttribute( 0.0 ) )
    paramGb.set( "mode", FnAttribute.IntAttribute( MODE_RIBBON ) )
    paramGb.set( "verbose", FnAttribute.IntAttribute( 1 ) )

    nodeBuilder.setParametersTemplateAttr( paramGb.build(), forceArrayNames = ('samples', 'patches') )

    nodeBuilder.setHintsForNode( { 'help' : '<p>Create Arnold procedurals suitable for invoking Autodesk XGen (bundled with Maya).</p>' +
                                            '<p>It is recommended that you turn on "compatible_motion_blur" in an ArnoldGlobalSettings node if you use ArnoldXGen. ' +
                                            'Also, if you wish to use camera-guided data (such as LOD), feed the scene with a camera to the input.</p>' +
                                            '<p><b>NOTE:</b> this node is influenced by the following environment variables:</p><ul><li><b>MAYA_PATH</b> root of Maya installation</li><li><b>MTOA_PATH</b> root of MtoA installation</li><li><b>XGEN_LOCATION</b> xgen location within Maya, usually ${MAYA_PATH}/plug-ins/xgen</li></ul>' } )
    
    nodeBuilder.setHintsForParameter( "location", { 'widget' : 'newScenegraphLocation' } )
    nodeBuilder.setHintsForParameter( "camera", { 'widget' : 'scenegraphLocation', 'help' : 'The camera used for LOD and other effects; can be blank to disable camera-related effects' } )
    nodeBuilder.setHintsForParameter( "specification",
                                      { 'widget' : 'mapper',
                                        'options' : xgenSpecStyle,
                                        'help' : 'How to find the xgen data; either directly specify a .xgen file, or else specify a maya project directory and the .xgen files will be searched for.' } )
    nodeBuilder.setHintsForParameter( "xgen_root",
                                      { 'widget' : 'fileInput',
                                        'dirsOnly' : 'True',
                                        'help' : 'Requred; path to maya project directory',
                                        'conditionalVisOp' : 'equalTo',
                                        'conditionalVisPath' : '../specification',
                                        'conditionalVisValue' : XGEN_ROOT } )
    nodeBuilder.setHintsForParameter( "xgen_file",
                                      { 'widget' : 'fileInput',
                                        'help' : 'Requred; path to .xgen file',
                                        'conditionalVisOp' : 'equalTo',
                                        'conditionalVisPath' : '../specification',
                                        'conditionalVisValue' : XGEN_FILE } )

    nodeBuilder.setHintsForParameter( "paths",
                                      { 'widget' : 'mapper',
                                        'options' : customPathsSetting,
                                        'help' : 'Whether to customize paths to Maya/MtoA/XGen, or just use MAYA_PATH, MTOA_PATH, and XGEN_LOCATION environment variables.' } )
    nodeBuilder.setHintsForParameter( "maya_path",
                                      { 'widget' : 'fileInput',
                                        'help' : 'Optional; path to Maya installation (to find xgen libraries).  If empty, KtoA will use the MAYA_PATH environment variable to locate Maya.',
                                        'conditionalVisOp' : 'equalTo',
                                        'conditionalVisPath' : '../paths',
                                        'conditionalVisValue' : CUSTOM_PATHS } )
    nodeBuilder.setHintsForParameter( "mtoa_path",
                                      { 'widget' : 'fileInput',
                                        'help' : 'Optional; path to MtoA installation (to find xgen_procedural for Arnold).  If empty, KtoA will use the MTOA_PATH environment variable to locate MtoA.',
                                        'conditionalVisOp' : 'equalTo',
                                        'conditionalVisPath' : '../paths',
                                        'conditionalVisValue' : CUSTOM_PATHS } )
    nodeBuilder.setHintsForParameter( "xgen_location",
                                      { 'widget' : 'fileInput',
                                        'help' : 'Optional; path to XGen installation (to find presets, scripts, etc). Usually this is "${MAYA_PATH}/plug-ins/xgen/"  If empty, KtoA will use the XGEN_LOCATION environment variable or infer it from the Maya path.',
                                        'conditionalVisOp' : 'equalTo',
                                        'conditionalVisPath' : '../paths',
                                        'conditionalVisValue' : CUSTOM_PATHS } )
    nodeBuilder.setHintsForParameter( "patches",
                                      { 'widget' : 'sortableArray',
                                        'help' : 'Optional; Maya shapes serving as base surfaces from which xgen curves/shapes are populated.  Please include namespaces.  If left empty, patches will be found from the xgen data.' } )
    nodeBuilder.setHintsForParameter( "name_scheme",
                                      { 'help' : 'Naming scheme for child locations of unique xgen procedurals.  It honors the following tags (enclosed in angle braces): scene, collection, description, patch' } )
    nodeBuilder.setHintsForParameter( "scene",
                                      { 'help' : 'Optional; Maya scene file name (without file path or extension) in which the xgen data lives.  If left empty, all scenes with xgen data will be found.',
                                        'conditionalVisOp' : 'equalTo',
                                        'conditionalVisPath' : '../specification',
                                        'conditionalVisValue' : XGEN_ROOT
                                        } )
    nodeBuilder.setHintsForParameter( "collection",
                                      {  'help' : 'XGen collection to render.  Please include namespace.  If left empty, collections will be found from the xgen data.',
                                        'conditionalVisOp' : 'equalTo',
                                        'conditionalVisPath' : '../specification',
                                        'conditionalVisValue' : XGEN_ROOT } )
    nodeBuilder.setHintsForParameter( "description", {  'help' : 'XGen description to render.  If left empty, descriptions will be found from the xgen data.' } )
    nodeBuilder.setHintsForParameter( "fps",
                                      { 'help' : 'Frames per second animation is proceeding at; 24, 30, 48 and 60 are typical' } )
    nodeBuilder.setHintsForParameter( "min_pixel_width",
                                      { 'help' : 'Arnold min-pixel-width; typically between 0 and 1, can help reduce aliasing' } )
    nodeBuilder.setHintsForParameter( "mode",
                                      { 'widget' : 'mapper',
                                        'options' : modeDict,
                                        'help' : 'Rendering mode of the curves; camera-facing ribbons, thick cylinders, or ribbons oriented by normal vectors.' } )
    nodeBuilder.setHintsForParameter( "samples",
                                      { 'widget' : 'sortableArray',
                                        'help' : 'Required; frame-relative motion sample times (e.g -0.25, 0.25).  It is recommended that you turn on "compatible_motion_blur" in an ArnoldGlobalSettings node if you use ArnoldXGen.' } )
    nodeBuilder.setHintsForParameter( "verbose",
                                      { 'help' : 'Log level for XGen' } )

    # Register our Op build function
    nodeBuilder.setBuildOpChainFnc( buildOpChain )

    # Create the new Node3D type
    nodeBuilder.build()


registerArnoldXGenOp()

