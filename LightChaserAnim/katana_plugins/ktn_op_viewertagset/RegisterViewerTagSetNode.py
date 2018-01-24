
def registerViewerTagSet():
    """
    Registers a new node type using the NodeTypeBuilder utility class.
    """

    from Katana import Nodes3DAPI
    from Katana import FnAttribute

    def buildViewerTagSetOpChain(node, interface):
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

        # Parse the color parameter
        colorRParam = node.getParameter('color.red')
        if colorRParam:
            argsGb.set('color.red', colorRParam.getValue(frameTime))
        colorGParam = node.getParameter('color.green')
        if colorGParam:
            argsGb.set('color.green', colorGParam.getValue(frameTime))
        colorBParam = node.getParameter('color.blue')
        if colorBParam:
            argsGb.set('color.blue', colorBParam.getValue(frameTime))
        colorParam = node.getParameter('color')

        # Parse the pickable parameter
        pickableParam = node.getParameter('pickable')
        if pickableParam:
            argsGb.set('pickable', FnAttribute.IntAttribute(pickableParam.getValue(frameTime)))

        # Add the Messer Op to the Ops chain
        interface.appendOp('ViewerTagSet', argsGb.build())


    # Create a NodeTypeBuilder to register the new type
    nodeTypeBuilder = Nodes3DAPI.NodeTypeBuilder('ViewerTagSet')

    # Add an input port
    nodeTypeBuilder.setInputPortNames(('in',))

    # Build the node's parameters
    gb = FnAttribute.GroupBuilder()
    gb.set('CEL', FnAttribute.StringAttribute(''))
    colorb = FnAttribute.GroupBuilder();
    colorb.set('red',FnAttribute.FloatAttribute( 0.0 ))
    colorb.set('green',FnAttribute.FloatAttribute( 0.0 ))
    colorb.set('blue',FnAttribute.FloatAttribute( 0.0 ))
    colorg = colorb.build()
    gb.set( "color", colorg )
    gb.set('pickable', FnAttribute.IntAttribute( 1 ) )
    # Set the parameters template
    nodeTypeBuilder.setParametersTemplateAttr(gb.build())
    # Set parameter hints
    nodeTypeBuilder.setHintsForParameter('CEL', {'widget':'cel'})
    nodeTypeBuilder.setHintsForParameter('color.red',{'help':"set viewer.default.annotation.color.red value"})
    nodeTypeBuilder.setHintsForParameter('color.green',{'help':"set viewer.default.annotation.color.green value"})
    nodeTypeBuilder.setHintsForParameter('color.blue',{'help':"set viewer.default.annotation.color.blue value"})
    nodeTypeBuilder.setHintsForParameter('pickable',{'widget' : 'checkBox','help' : 'Whether you can select object in viewer'})

    # Set the callback responsible to build the Ops chain
    nodeTypeBuilder.setBuildOpChainFnc(buildViewerTagSetOpChain)

    # Build the new node type
    nodeTypeBuilder.build()

# Register the node
registerViewerTagSet()
