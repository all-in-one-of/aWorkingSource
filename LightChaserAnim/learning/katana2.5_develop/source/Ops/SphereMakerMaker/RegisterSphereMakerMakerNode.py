# A startup file to register the SphereMakerMaker Op, if this is in
# $KATANA_RESOURCES/Plugins, it will be exec'd automatically. If its
# in $KATANA_RESOURCES/Startup, it would need to be init.py, or called
# from and existing init.py script.


def registerSphereMakerMaker():
    """
    Registers a new SphereMakerMaker node type using the NodeTypeBuilder utility
    class.
    """

    from Katana import Nodes3DAPI
    from Katana import FnAttribute

    def buildSphereMakerMakerOpChain(node, interface):
        """
        Defines the callback function used to create the Ops chain for the
        node type being registered.

        @type node: C{Nodes3DAPI.NodeTypeBuilder.SphereMakerMaker}
        @type interface: C{Nodes3DAPI.NodeTypeBuilder.BuildChainInterface}
        @param node: The node for which to define the Ops chain
        @param interface: The interface providing the functions needed to set
            up the Ops chain for the given node.
        """
         # Set the minimum number of input ports
        interface.setMinRequiredInputs(0)

        # Parse node parameters
        numberOfSpheresParam = node.getParameter("numberOfSpheres")
        locationParam = node.getParameter('location')
        numberOfSphereMakersParam = node.getParameter('numberOfSphereMakers')
        if locationParam and numberOfSphereMakersParam and numberOfSpheresParam:
            # Get the current frame time
            frameTime = interface.getGraphState().getTime()

            location = locationParam.getValue(frameTime)
            # The base location is encoded using nested group attributes
            # defining a hierarchy where the elements in the location paths
            # are interleaved with group attributes named 'c' (for child).
            # The last element will contain a group attribute, named 'a',
            # which in turn will hold an attribute defining the number of
            # sphereMakers to be generated.
            # See the Op source code for more details
            locationPaths = location[1:].split('/')[1:]
            attrsHierarchy = 'c.' + '.c.'.join(locationPaths)

            argsGb = FnAttribute.GroupBuilder()
            argsGb.set(
                attrsHierarchy + '.a.numberOfSphereMakers',
                FnAttribute.IntAttribute(
                    numberOfSphereMakersParam.getValue(frameTime)))
            argsGb.set(
                attrsHierarchy + '.a.numberOfSpheres',
                FnAttribute.IntAttribute(
                    numberOfSpheresParam.getValue(frameTime)))

        # Add the SphereMakerMaker Op to the Ops chain
        interface.appendOp('SphereMakerMaker', argsGb.build())

    # Build the node's parameters
    gb = FnAttribute.GroupBuilder()
    gb.set('numberOfSphereMakers', FnAttribute.IntAttribute(1))
    gb.set('numberOfSpheres', FnAttribute.IntAttribute(1))
    gb.set('location',
           FnAttribute.StringAttribute('/root/world/geo/sphereMakerMaker'))

    # Create a NodeTypeBuilder to register the new type
    nodeTypeBuilder = Nodes3DAPI.NodeTypeBuilder('SphereMakerMaker')

    # Set the parameters template
    nodeTypeBuilder.setParametersTemplateAttr(gb.build())

    # Set parameter hints
    nodeTypeBuilder.setHintsForParameter('location',
                                         {'widget': 'scenegraphLocation'})
    nodeTypeBuilder.setHintsForParameter('numberOfSphereMakers', {'int': True})
    nodeTypeBuilder.setHintsForParameter('numberOfSpheres', {'int': True})

    # Set the callback responsible to build the Ops chain
    nodeTypeBuilder.setBuildOpChainFnc(buildSphereMakerMakerOpChain)

    # Build the new node type
    nodeTypeBuilder.build()

# Register the node
registerSphereMakerMaker()
