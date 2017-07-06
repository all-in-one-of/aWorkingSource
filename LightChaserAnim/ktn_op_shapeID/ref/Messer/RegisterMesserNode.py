
def registerMesser():
    """
    Registers a new Messer node type using the NodeTypeBuilder utility class.
    """

    from Katana import Nodes3DAPI
    from Katana import FnAttribute

    def buildMesserOpChain(node, interface):
        """
        Defines the callback function used to define the Ops chain for the
        node type being registered.

        @type node: C{Nodes3DAPI.NodeTypeBuilder.Messer}
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

        # Parse the CEL parameter
        celParam = node.getParameter('CEL')
        if celParam:
            argsGb.set('CEL', celParam.getValue(frameTime))

        # Parse the displacement parameter
        dispParam = node.getParameter('displacement')
        if dispParam:
            argsGb.set('displacement', dispParam.getValue(frameTime))

        # Add the Messer Op to the Ops chain
        interface.appendOp('Messer', argsGb.build())


    # Create a NodeTypeBuilder to register the new type
    nodeTypeBuilder = Nodes3DAPI.NodeTypeBuilder('Messer')

    # Add an input port
    nodeTypeBuilder.setInputPortNames(('in',))

    # Build the node's parameters
    gb = FnAttribute.GroupBuilder()
    gb.set('CEL', FnAttribute.StringAttribute(''))
    gb.set('displacement', FnAttribute.FloatAttribute(0.0))

    # Set the parameters template
    nodeTypeBuilder.setParametersTemplateAttr(gb.build())
    # Set parameter hints
    nodeTypeBuilder.setHintsForParameter('CEL', {'widget':'cel'})
    # Set the callback responsible to build the Ops chain
    nodeTypeBuilder.setBuildOpChainFnc(buildMesserOpChain)

    # Build the new node type
    nodeTypeBuilder.build()

# Register the node
registerMesser()
