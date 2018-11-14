# A startup file to register the ArnoldYeti Op, if this is in
# $KATANA_RESOURCES/Plugins, it will be exec'd automatically. If its
# in $KATANA_RESOURCES/Startup, it would need to be init.py, or called
# from and existing init.py script.

def registerArnoldYetiInOp():
    """
    Registers the ArnoldYeti Op using the NodeTypeBuilder.
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

        # Pass these along through the ops so that the have shutter open/close and number of samples
        systemArgs = interface.getGraphState().getOpSystemArgs()

        # Get the parameters from our node
        locationParam = node.getParameter( "location" )
        filenameParam = node.getParameter( "filename" )
        proxyParam = node.getParameter( "proxy" )
        samplesParam = node.getParameter( "samples" )
        imageSearchPathParam = node.getParameter( "imageSearchPath" )
        disableBoundingboxParam = node.getParameter( "disableBoundingbox" )
        lengthParam = node.getParameter( "length" )
        densityParam = node.getParameter( "density" )
        minPixelWidthParam = node.getParameter( "min_pixel_width" )
        modeParam = node.getParameter( "mode" )
        widthParam = node.getParameter( "width" )
        verboseParam = node.getParameter( "verbose" )
        threadsParam = node.getParameter( "threads" )
        makeInteractiveParam = node.getParameter("makeInteractive")

        if not locationParam or not filenameParam:
            raise RuntimeError( "Missing node parameters, requires 'location' and 'filename'" )
        
        # Copy array param values out to appropriate attributes
        samples = []
        for ci in range(0, samplesParam.getNumChildren()):
            samples = samples + [ samplesParam.getChildByIndex(ci).getValue(frameTime) ]
        if samples is not None and len(samples) > 0:
            opSamplesParam = FnAttribute.FloatAttribute(samples)
        imageSearchPath = ""
        if imageSearchPathParam.getNumChildren() >= 1:
            imageSearchPath = imageSearchPathParam.getChildByIndex(ci).getValue(frameTime)
        for ci in range(1, imageSearchPathParam.getNumChildren()):
            imageSearchPath = imageSearchPath + ":" + imageSearchPathParam.getChildByIndex(ci).getValue(frameTime)
        opImageSearchPathParam = FnAttribute.StringAttribute(imageSearchPath)

        # Build the Op args from our node
        argsGb = FnAttribute.GroupBuilder()
        argsGb.set( "filename", FnAttribute.StringAttribute(filenameParam.getValue(frameTime)) )
        argsGb.set( "proxy", FnAttribute.StringAttribute(proxyParam.getValue(frameTime)) )
        if samples is not None and len(samples) > 0:
            argsGb.set( "samples", opSamplesParam )
        argsGb.set( "length", FnAttribute.FloatAttribute(lengthParam.getValue(frameTime)) )
        argsGb.set( "density", FnAttribute.FloatAttribute(densityParam.getValue(frameTime)) )
        argsGb.set( "min_pixel_width", FnAttribute.FloatAttribute(minPixelWidthParam.getValue(frameTime)) )
        argsGb.set( "width", FnAttribute.FloatAttribute(widthParam.getValue(frameTime)) )
        argsGb.set( "imageSearchPath", opImageSearchPathParam )
        argsGb.set( "verbose", FnAttribute.IntAttribute(verboseParam.getValue(frameTime)) )
        argsGb.set( "threads", FnAttribute.IntAttribute(threadsParam.getValue(frameTime)) )
        argsGb.set( "frame", FnAttribute.IntAttribute(int(round(frameTime))) )
        argsGb.set( "mode", FnAttribute.IntAttribute(modeParam.getValue(frameTime)) )
        argsGb.set( "system", systemArgs )
        argsGb.set( "disableBoundingbox", FnAttribute.IntAttribute(disableBoundingboxParam.getValue(frameTime)) )
        argsGb.set( "makeInteractive", FnAttribute.StringAttribute(makeInteractiveParam.getValue(frameTime)) )

        argsGb.set( "xform", interface.getTransformAsAttribute() )
        exclusiveAttrName, exclusiveAttr = interface.getExclusiveToNameAndAttribute()
        if exclusiveAttr is not None:
            argsGb.set( "exclusiveTo", exclusiveAttr )

        # We want to use the StaticSceneCreate Op to build the parent
        # hierarchy, so that our op only has to worry about generating its
        # children. Its args are somewhat complex, but fortunately, there
        # is a helper class that makes it all much easier.

        rootLocation = locationParam.getValue( frameTime )

        sscb = FnGeolibServices.OpArgsBuilders.StaticSceneCreate()
        sscb.addSubOpAtLocation( rootLocation, "ArnoldYeti_In", argsGb.build() )

        interface.appendOp( "StaticSceneCreate", sscb.build() )

    def getScenegraphLocation(node, frameTime):
        locationParam = node.getParameter( "location" )
        return locationParam.getValue(0.0)

    # Here we need to define the parameters for the node, and register the op
    # chain creation callback function

    nodeBuilder = Nodes3DAPI.NodeTypeBuilder( "ArnoldYeti_In" )
   
    # If we wanted to merge with incoming scene, we could simply allow the
    # node to have an input. Unless you delete locations in your Op, any
    # existing locations will pass-through. It is encouraged though to avoid
    # long chains of Ops, as it makes multi-threading and caching less
    # efficient, so for 'Generator' Ops, no input is preferable.
    # nodeBuilder.setInputPortNames( ("in",) )

    # Parameters can be described by a group attribute
    paramGb = FnAttribute.GroupBuilder()
    paramGb.set( "location", FnAttribute.StringAttribute("/root/world/geo/yetiProc") )
    paramGb.set( "filename", FnAttribute.StringAttribute("") )
    paramGb.set( "proxy", FnAttribute.StringAttribute("") )
    paramGb.set( "density", FnAttribute.FloatAttribute( 1.0 ) )
    paramGb.set( "length", FnAttribute.FloatAttribute( 1.0 ) )
    paramGb.set( "imageSearchPath", FnAttribute.StringAttribute([ ], 1) )
    paramGb.set( "min_pixel_width", FnAttribute.FloatAttribute( 0.0 ) )
    paramGb.set( "width", FnAttribute.FloatAttribute( 1.0 ) )
    paramGb.set( "mode", FnAttribute.IntAttribute( MODE_RIBBON ) )
    paramGb.set( "samples", FnAttribute.FloatAttribute([ ], 1) )
    paramGb.set( "threads", FnAttribute.IntAttribute( 0 ) )
    paramGb.set( "verbose", FnAttribute.IntAttribute( 2 ) )
    paramGb.set( "disableBoundingbox", FnAttribute.IntAttribute( 1 ) )

    nodeBuilder.addTransformParameters(paramGb)
    nodeBuilder.addMakeInteractiveParameter(paramGb)
    nodeBuilder.addInteractiveTransformCallbacks(paramGb)

    nodeBuilder.setParametersTemplateAttr( paramGb.build(), forceArrayNames = ('samples', 'imageSearchPath') )

    nodeBuilder.setHintsForNode( { 'help' : 'Create an Arnold procedural node suitable for invoking Peregrine Labs\' Yeti.' } )
    
    nodeBuilder.setHintsForParameter( "location", { 'widget' : 'newScenegraphLocation' } )
    nodeBuilder.setHintsForParameter( "filename",
                                      { 'widget' : 'assetIdInput',
                                        'sequenceListing' : False,
                                        'fileTypes':'fur',
                                        'help' : 'Requred; path to Yeti fur cache file' } )
    nodeBuilder.setHintsForParameter( "proxy",
                                      { 'widget' : 'assetIdInput',
                                        'sequenceListing' : False,
                                        'fileTypes':'abc',
                                        'help' : 'Not Requred; path to Yeti fur alembic proxy file' } )
    nodeBuilder.setHintsForParameter( "density",
                                      { 'help' : 'Density scale for curve population' } )
    nodeBuilder.setHintsForParameter( "length",
                                      { 'help' : 'Length scale for curves' } )
    nodeBuilder.setHintsForParameter( "imageSearchPath",
                                      { 'widget' : 'sortableArray',
                                        'help' : 'Optional; colon-separated paths to images for curve operations' } )
    nodeBuilder.setHintsForParameter( "min_pixel_width",
                                      { 'help' : 'Arnold min-pixel-width; typically between 0 and 1, can help reduce aliasing' } )
    nodeBuilder.setHintsForParameter( "mode",
                                      { 'widget' : 'mapper',
                                        'options' : modeDict,
                                        'help' : 'Rendering mode of the curves; camera-facing ribbons, thick cylinders, or ribbons oriented by normal vectors.' } )
    nodeBuilder.setHintsForParameter( "width",
                                      { 'help' : 'Width/radius scale factor for curves' } )
    nodeBuilder.setHintsForParameter( "samples",
                                      { 'widget' : 'sortableArray',
                                        'help' : 'Optional; frame-relative motion sample times (e.g -0.25, 0.25).' } )
    nodeBuilder.setHintsForParameter( "threads",
                                      { 'help' : 'Number of threads for curve generation' } )
    nodeBuilder.setHintsForParameter( "verbose",
                                      { 'help' : 'Log level for Yeti' } )
    nodeBuilder.setHintsForParameter('disableBoundingbox',
                                      {'widget':'checkBox', 
                                      'help' : 'disable caculating Yeti fur Boundingbox'})

    # Register our Op build function
    nodeBuilder.setBuildOpChainFnc( buildOpChain )

    # Make this available for widgets and parameter expressions
    nodeBuilder.setGetScenegraphLocationFnc( getScenegraphLocation )

    # Create the new Node3D type
    nodeBuilder.build()


registerArnoldYetiInOp()

