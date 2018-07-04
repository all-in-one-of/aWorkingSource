
def registerGeoScaler():
    """
    Registers a new GeoScaler node type using the NodeTypeBuilder utility
    class.
    """

    from Katana import Nodes3DAPI
    from Katana import FnAttribute

    def buildGeoScalerOpChain(node, interface):
        """
        Defines the callback function used to create the Ops chain for the
        node type being registered.

        @type node: C{Nodes3DAPI.NodeTypeBuilder.GeoScaler}
        @type interface: C{Nodes3DAPI.NodeTypeBuilder.BuildChainInterface}
        @param node: The node for which to define the Ops chain
        @param interface: The interface providing the functions needed to set
            up the Ops chain for the given node.
        """
        # Get the current frame time
        frameTime = interface.getGraphState().getTime()

        # Set the minimum number of input ports
        interface.setMinRequiredInputs(1)

        argsGb = FnAttribute.GroupBuilder()

        # Parse node parameters
        CELParam = node.getParameter("CEL")
        if CELParam:
            CEL = CELParam.getValue(frameTime)
            argsGb.set("CEL", CEL)

        scaleParam = node.getParameter('scale')
        if scaleParam:
            scale = scaleParam.getValue(frameTime)
            argsGb.set("scale", scale)

        # Add the GeoScaler Op to the Ops chain
        interface.appendOp('GeoScaler', argsGb.build())

    # Create a NodeTypeBuilder to register the new type
    nodeTypeBuilder = Nodes3DAPI.NodeTypeBuilder('GeoScaler')

    # Add input port
    nodeTypeBuilder.setInputPortNames(("in",))

    # Build the node's parameters
    gb = FnAttribute.GroupBuilder()
    gb.set('CEL', FnAttribute.StringAttribute(''))
    gb.set('scale', FnAttribute.FloatAttribute(1.0))

    # Set the parameters template
    nodeTypeBuilder.setParametersTemplateAttr(gb.build())

    # Set parameter hints
    nodeTypeBuilder.setHintsForParameter('CEL', {'widget': 'scenegraphLocation'})

    # Set the callback responsible to build the Ops chain
    nodeTypeBuilder.setBuildOpChainFnc(buildGeoScalerOpChain)

    # Build the new node type
    nodeTypeBuilder.build()

# Register the node
registerGeoScaler()
