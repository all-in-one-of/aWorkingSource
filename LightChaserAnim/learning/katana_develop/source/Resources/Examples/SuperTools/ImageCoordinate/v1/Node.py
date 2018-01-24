import logging
log = logging.getLogger('ImageCoordinateNode')

from Katana import NodegraphAPI, AssetAPI

import ScriptActions as SA


class ImageCoordinateNode(NodegraphAPI.SuperTool):
    """
    The ImageCoordinate allows a user to load an image into its user
    interface and specify a 2D-point that is then stored as attribute data
    on a scene graph location.

    The SuperTool exposes two parameters to drive its behaviour:
        - location: the scene graph location where the 2D coordinates will be
                    stored
        - filePath: the file path to the image that should be loaded and
                    displayed in the SuperTool's user interface.

    The internal node structure to support this is very simple:

        .----------------.
        | LocationCreate |
        `----------------'
                |
                |
        .----------------.
        |  AttributeSet  |
        `----------------'

    The SuperTool accepts no input and can be connected to the rest of your
    scene graph using the output port named 'output'.

    In addition to the two parameters described above, this example also
    demonstrates who you can 'steal' parameters that have been defined on
    other nodes within your SuperTool's internal network and display them
    as part of your own user interface. This is covered in Editor.py.

    Finally, this example demonstrates how to:
        - Register callbacks to receive updates when ever a specific
          parameter's value changes.
        - Push changes from a custom UI widget back to your SuperTool
          to drive scene graph attributes.
    """
    def __init__(self):
        self.hideNodegraphGroupControls()

        # Only provide an output port, this implies this SuperTool is an
        # importer node
        self.addOutputPort("output")

        ######################################################################
        # Create parameters specific to our SuperTool.
        #
        # See _ExtraHints at the bottom of this file to see how we specify how
        # these parameters should be displayed in the UI by specifying built-in
        # widgets.
        ######################################################################
        initialLocation = '/root/world/2d_image/points'
        self.getParameters().createChildString('location', initialLocation)
        self.getParameters().createChildString('filePath', '')

        ######################################################################
        # Create the SuperTool's internal node network.
        #
        # As described above, it is composed of a LocationCreate node and
        # an AttributeSet node.
        # 
        # The LocationCreate node provides a location for the AttributeSet node
        # to work on, and also defines the location's type as 'image_coord'
        # (this is an arbitrary type which you could change to whatever you
        # wish for your application's own purposes).
        #
        # This is also a convenient place to provide some default values for
        # the parameters.
        ######################################################################

        # LocationCreate setup.
        locationCreateNode = NodegraphAPI.CreateNode('LocationCreate', self)
        locationCreateNode.getParameter("type").setValue("image_coord", 0)
        locationsArrayParameter = locationCreateNode.getParameter("locations")
        locationsArrayParameter.getChildByIndex(0).setValue(initialLocation, 0)

        # AttributeSet setup.
        attributeSetNode = NodegraphAPI.CreateNode('AttributeSet', self)
        # Set the name of the attribute to create
        attributeSetNode.getParameter("attributeName").setValue("points", 0)
        # Specify a 1x2 array where we can store our 2D coordinate
        attributeSetNode.getParameter("numberValue").resizeArray(2)
        attributeSetNode.getParameter("numberValue").setTupleSize(2)
        pathsArrayParameter = attributeSetNode.getParameter("paths")
        pathsArrayParameter.getChildByIndex(0).setValue(initialLocation, 0)

        # Add two parameters to our SuperTool which will allow us to get
        # access to our internal nodes later.
        #
        # We don't expose the parameters in the SuperTool's UI but you can
        # see them if you were to run something similar to:
        #
        # NodegraphAPI.GetNode('<SuperToolNodeName>').getParameters().getXML()
        SA.AddNodeReferenceParam(self, 'node_attributes', attributeSetNode)
        SA.AddNodeReferenceParam(self, 'node_locationcreate',
                                 locationCreateNode)

        # Connect the internal node network together and then connect it to the
        # output port of the SuperTool.
        locationCreateOutputPort = locationCreateNode.getOutputPortByIndex(0)
        attributeSetInputPort = attributeSetNode.getInputPortByIndex(0)
        locationCreateOutputPort.connect(attributeSetInputPort)

        superToolReturnPort = self.getReturnPort("output")
        attributeSetOutputPort = attributeSetNode.getOutputPortByIndex(0)
        superToolReturnPort.connect(attributeSetOutputPort)

        # Position the nodes in the internal node network so it looks a bit
        # more organised.
        NodegraphAPI.SetNodePosition(locationCreateNode, (0, 0))
        NodegraphAPI.SetNodePosition(attributeSetNode, (0, -50))

    def addParameterHints(self, attrName, inputDict):
        """
        This function will be called by Katana to allow you to provide hints
        to the UI to change how parameters are displayed.
        """
        inputDict.update(_ExtraHints.get(attrName, {}))


_ExtraHints = {
    'ImageCoordinate.location': {
        'widget': 'newScenegraphLocation',
        'help':
            """
            The scene graph location to write the (x, y) coordinates selected
            by the user to.
            """,
    },
    'ImageCoordinate.filePath': {
        'widget': 'assetIdInput',
        'help':
            """
            Specify an image to load.
            """,
        'context': AssetAPI.kAssetContextImage
    },
}

