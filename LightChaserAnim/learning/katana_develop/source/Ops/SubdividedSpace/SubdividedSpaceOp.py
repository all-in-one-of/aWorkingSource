# A startup file to register the SubdividedSpace Op, if this is in 
# $KATANA_RESOURCES/Plugins, it will be exec'd automatically. If its
# in $KATANA_RESOURCES/Startup, it would need to be init.py, or called
# from and existing init.py script.

def registerSubdividedSpaceOp():
    """
    Registers the SubdividedSpace Op using the NodeTypeBuilder.
    This is a helper class that takes care of registering a suitable
    node graph node type, and configuring it to call the supplied method
    in order to build the Op chain that is used to represent the
    node in the Op Graph.
    """

    from Katana import Nodes3DAPI
    from Katana import FnAttribute, FnGeolibServices

    def buildOpChain( node, interface ):
        """
        Configures the Ops to represent the current state of the node.
        The calling mechanism in Node3D takes are of ensuring that
        the underlying Op network is only updated if Op Args and types
        have changed.

        @type node: C{Nodes3DAPI.NodeTypeBuilder.SubdividedSpace}
        @type interface: C[Nodes3DAPI.NodeTypeBuilder.BuildChainInterface}
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

        # Get the parameters from our node
        maxDepthParam = node.getParameter( "maxDepth" )
        subdivParam = node.getParameter( "subdivisions" )
        locationParam = node.getParameter( "location" )

        if not locationParam or not maxDepthParam or not subdivParam:
            raise RuntimeError( "Missing node parameters, require "+
                    "'location', 'maxdepth' and 'subdivisions'" )

        # Build the Op args from our node
        argsGb = FnAttribute.GroupBuilder()
        argsGb.set( "maxDepth", FnAttribute.IntAttribute(maxDepthParam.getValue(frameTime)) )
        argsGb.set( "subdivisions", FnAttribute.IntAttribute(subdivParam.getValue(frameTime)) )

        # We want to use the StaticSceneCreate Op to build the parent
        # hierarchy, so that our op only has to worry about generating its
        # children. Its args are somewhat complex, but fortunately, there
        # is a helper class that makes it all much easier.

        rootLocation = locationParam.getValue( frameTime )

        sscb = FnGeolibServices.OpArgsBuilders.StaticSceneCreate()
        sscb.addSubOpAtLocation( rootLocation, "SubdividedSpace", argsGb.build() )

        interface.appendOp( "StaticSceneCreate", sscb.build() )


    # Here we need to define the parameters for the node, and register the op
    # chain creation callback function

    nodeBuilder = Nodes3DAPI.NodeTypeBuilder( "SubdividedSpace" )

    # If we wanted to merge with incoming scene, we could simply allow the
    # node to have an input. Unless you delete locations in your Op, any
    # existing locations will pass-through. It is encouraged though to avoid
    # long chains of Ops, as it makes multi-threading and caching less
    # efficient, so for 'Generator' Ops, no input is preferable.
    # nodeBuilder.setInputPortNames( ("in",) )

    # Parameters can be described by a group attribute
    paramGb = FnAttribute.GroupBuilder()
    paramGb.set( "location", FnAttribute.StringAttribute("/root/world/geo/dividedSpace") )
    paramGb.set( "maxDepth", FnAttribute.IntAttribute( 4 ) )
    paramGb.set( "subdivisions", FnAttribute.IntAttribute( 1 ) )

    nodeBuilder.setParametersTemplateAttr( paramGb.build() )

    nodeBuilder.setHintsForParameter( "location", { 'widget' : 'newScenegraphLocation' } )

    # Register our Op build function
    nodeBuilder.setBuildOpChainFnc( buildOpChain )

    # Create the new Node3D type
    nodeBuilder.build()


print( "Registering SubdividedSpace Op" )
registerSubdividedSpaceOp()
