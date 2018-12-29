# Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.
"""
Example script that registers a layered menu for the B{Node Graph} tab, which
shows the names of available shaders and creates a ShadingNode node
with the chosen shader set on it when one of the menu entries is chosen.
"""

from Katana import NodegraphAPI, RenderingAPI, LayeredMenuAPI, os
from RenderingAPI import RenderPlugins


def PopulateCallback(layeredMenu):
    """
    Callback for the layered menu, which adds entries to the given
    C{layeredMenu} based on the available shaders.

    @type layeredMenu: L{LayeredMenuAPI.LayeredMenu}
    @param layeredMenu: The layered menu to add entries to.
    """
    # Obtain a list of names of available shaders from the renderer info plug-in
    renderer = RenderingAPI.RenderPlugins.GetDefaultRendererPluginName()
    rendererInfoPlugin = RenderPlugins.GetInfoPlugin(renderer)
    nodeTypes = rendererInfoPlugin.getRendererObjectNames(
        RenderingAPI.RendererInfo.kRendererObjectTypeShader)
    shaderType = RenderingAPI.RendererInfo.kRendererObjectTypeShader
    shaderNames = rendererInfoPlugin.getRendererObjectNames(shaderType)

    # Iterate over the names of shaders and add a menu entry for each of them
    # to the given layered menu.
    if renderer == "arnold":
        for shaderName in shaderNames:
                if shaderName.startswith("standard"):
                    layeredMenu.addEntry(shaderName, text=shaderName,
                                         color=(0.1000, 0.5000, 0.1000))
                elif shaderName.startswith("lc_"):
                    layeredMenu.addEntry(shaderName, text=shaderName,
                                         color=(0.0000, 0.3000, 0.7000))
                else:
                    layeredMenu.addEntry(shaderName, text=shaderName,
                                         color=(0.7000, 0.3000, 0.0000))
    elif renderer == "dl":
        for nodeType in nodeTypes:
            if nodeType != 'NetworkMaterial_Displacement' and nodeType != 'NetworkMaterial_Light' and nodeType != 'NetworkMaterial_Surface' and nodeType != 'uvCoord':
                layeredMenu.addEntry(nodeType, text=nodeType,
                                     color=(0.7294, 0.102, 0.1137))
    else:
        for nodeType in nodeTypes:
            layeredMenu.addEntry(nodeType, text=nodeType,
                                 color=(0.7294, 0.102, 0.1137))

def ActionCallback(value):
    """
    Callback for the layered menu, which creates a ShadingNode node and
    sets its B{nodeType} parameter to the given C{value}, which is the name of
    a shader as set for the menu entry in L{PopulateCallback()}.

    @type value: C{str}
    @rtype: C{object}
    @param value: An arbitrary object that the menu entry that was chosen
        represents. In our case here, this is the name of a dl shader as
        passed to the L{LayeredMenuAPI.LayeredMenu.addEntry()} function in
        L{PopulateCallback()}.
    @return: An arbitrary object. In our case here, we return the created
        ShadingNode node, which is then placed in the B{Node Graph} tab
        because it is a L{NodegraphAPI.Node} instance.
    """

    # Use the entered Group node of the largest Node Graph tab as the parent
    # node for the new nodes
    import UI4
    nodeGraphTab = UI4.App.Tabs.FindTopTab('Node Graph')
    parentNode = nodeGraphTab.getEnteredGroupNode()
    nodes = []

    def CreateArnoldShadingNode(nodeType, parentNode):
        """
        Helper function to create a DlShadingNode under the given parent node
        with its nodeType parameter set to the given node type name, and its
        name set to the given node type name, possibly followed by a numeric
        index to make the name unique in the node graph document.
        """
        result = NodegraphAPI.CreateNode('ArnoldShadingNode', parentNode)
        result.getParameter('nodeType').setValue(nodeType, 0)
        result.setName(nodeType)
        result.getParameter('name').setValue(result.getName(), 0)
        return result

    def CreatePrmanShadingNode(nodeType, parentNode):
        """
        Helper function to create a DlShadingNode under the given parent node
        with its nodeType parameter set to the given node type name, and its
        name set to the given node type name, possibly followed by a numeric
        index to make the name unique in the node graph document.
        """
        result = NodegraphAPI.CreateNode('PrmanShadingNode', parentNode)
        result.getParameter('nodeType').setValue(nodeType, 0)
        result.setName(nodeType)
        result.getParameter('name').setValue(result.getName(), 0)
        return result

    def CreateDlShadingNode(nodeType, parentNode):
        """
        Helper function to create a DlShadingNode under the given parent node
        with its nodeType parameter set to the given node type name, and its
        name set to the given node type name, possibly followed by a numeric
        index to make the name unique in the node graph document.
        """
        result = NodegraphAPI.CreateNode('DlShadingNode', parentNode)
        result.getParameter('nodeType').setValue(nodeType, 0)
        result.setName(nodeType)
        result.getParameter('name').setValue(result.getName(), 0)
        return result

    renderer = RenderingAPI.RenderPlugins.GetDefaultRendererPluginName()
    if renderer == 'dl':
        shadingNodeType = 'DlShadingNode'

        # Create a shading node with its name and node type parameters set to the
        # chosen shader type name as given in `value`
        node = CreateDlShadingNode(value, parentNode)

        # Ensure that the input and output parameters are shown when the user opens
        # the input or output parameter popup by clicking the triangle on the left
        # or right side of the shading node
        node.checkDynamicParameters()

        # Define a list to collect all nodes created in this function, starting
        # with the main shading node we created above
        nodes = []
        nodes.append(node)

        # Check if the chosen type of shading node requires additional nodes in
        # order to be useful

        # First case A: surface shader connected to NetworkMaterial
        if value == 'anisotropic' or value == 'blinn' or value == 'lambert' or value == 'dl3DelightMaterial' or value == 'dlGlass' or value == 'dlMetal' or value == 'dlSkin' or value == 'dlHairAndFur' or value == 'surfaceShader':
            # Create a NetworkMaterial node
            networkMaterialNode = NodegraphAPI.CreateNode('NetworkMaterial', parentNode)
            # Make sure to call RendererInfo::fillRendererShaderTypeTags
            networkMaterialNode.addShaderInputPort('dl', 'Surface')
            nodes.append(networkMaterialNode)

            # Connect the outColor output of the value node to the
            # input port dlSurface of the networkMaterialNode
            node.getOutputPort('outColor').connect(
                networkMaterialNode.getInputPort('dlSurface'))

            # Move the networkMaterialNode node to the right of the shading node
            x, y = NodegraphAPI.GetNodePosition(node)
            NodegraphAPI.SetNodePosition(networkMaterialNode, (x + 250, y))

        # First case B: volume shader connected to NetworkMaterial
        elif value == 'dlAtmosphere':
            # Create a NetworkMaterial node
            networkMaterialNode = NodegraphAPI.CreateNode('NetworkMaterial', parentNode)
            # Make sure to call RendererInfo::fillRendererShaderTypeTags
            networkMaterialNode.addShaderInputPort('dl', 'Volume')
            nodes.append(networkMaterialNode)

            # Connect the outColor output of the value node to the
            # input port dlVolume of the networkMaterialNode
            node.getOutputPort('outColor').connect(
                networkMaterialNode.getInputPort('dlVolume'))

            # Move the networkMaterialNode node to the right of the shading node
            x, y = NodegraphAPI.GetNodePosition(node)
            NodegraphAPI.SetNodePosition(networkMaterialNode, (x + 250, y))

        # Second case: displacement shader connected to NetworkMaterial
        elif value == 'displacementShader':
            # Create a NetworkMaterial node
            networkMaterialNode = NodegraphAPI.CreateNode('NetworkMaterial', parentNode)
            # Make sure to call RendererInfo::fillRendererShaderTypeTags
            networkMaterialNode.addShaderInputPort('dl', 'Displacement')
            nodes.append(networkMaterialNode)

            # Connect the outDisplacement output of the value node to the
            # input port dlDisplacement of the networkMaterialNode
            node.getOutputPort('outDisplacement').connect(
                networkMaterialNode.getInputPort('dlDisplacement'))

            # Move the networkMaterialNode node to the right of the shading node
            x, y = NodegraphAPI.GetNodePosition(node)
            NodegraphAPI.SetNodePosition(networkMaterialNode, (x + 250, y))

        # Third case: 2D texture shader connected to place2dTexture with a single
        # connection
        elif value == 'bulge' or value == 'checker' or value == 'cloth' or value == 'fractal' or value == 'grid' or value == 'noise' or value == 'ocean' or value == 'ramp' or value == 'stencil':
            # Create a place2dTexture shading node
            place2dTextureNode = CreateDlShadingNode('place2dTexture', parentNode)
            nodes.append(place2dTextureNode)

            # Connect the UV Coordinates output of the place2dTexture node to the
            # input of the same name on the respective main shading node
            place2dTextureNode.checkDynamicParameters()
            place2dTextureNode.getOutputPort('outUV').connect(
                node.getInputPort('uvCoord'))

            # Move the place2dTexture node to the left of the main shading node
            x, y = NodegraphAPI.GetNodePosition(node)
            NodegraphAPI.SetNodePosition(place2dTextureNode, (x - 250, y))

        # Fourth case: 2D texture shader connected to place2dTexture with a lot
        # of connections (if file and psdFileTex had the correct input ports)
        elif value == 'file' or value == 'psdFileTex':
            # Create a place2dTexture shading node
            place2dTextureNode = CreateDlShadingNode('place2dTexture', parentNode)
            nodes.append(place2dTextureNode)

            # Connect the UV Coordinates output of the place2dTexture node to the
            # input of the same name on the respective main shading node
            place2dTextureNode.checkDynamicParameters()
            place2dTextureNode.getOutputPort('outUV').connect(
                node.getInputPort('uvCoord'))

            # Move the place2dTexture node to the left of the main shading node
            x, y = NodegraphAPI.GetNodePosition(node)
            NodegraphAPI.SetNodePosition(place2dTextureNode, (x - 250, y))

        # Fifth case: 3D texture shader connected to locationMatrix with single
        # connection
        elif value == 'brownian' or value == 'cloud' or value == 'granite' or value == 'leather' or value == 'marble' or value == 'rock' or value == 'snow' or value == 'solidFractal' or value == 'stucco' or value == 'volumeNoise' or value == 'wood':
            # Create a locationMatrix shading node
            locationMatrix = CreateDlShadingNode('locationMatrix', parentNode)
            nodes.append(locationMatrix)

            # Connect the output world inverse matrix of the locationMatrix node to
            # the placement matrix input on the respective main shading node
            locationMatrix.checkDynamicParameters()
            locationMatrix.getOutputPort('o_worldInverseMatrix').connect(
                node.getInputPort('placementMatrix'))

            # Move the locationMatrix node to the left of the main shading node
            x, y = NodegraphAPI.GetNodePosition(node)
            NodegraphAPI.SetNodePosition(locationMatrix, (x - 250, y))


    elif renderer == 'arnold':
        shadingNodeType = 'ArnoldShadingNode'


        closure_list = ['standard_surface', 'standard_hair', 'standard_volume', 'utility']
        
        light_list = ['cylinder_light', 'disk_light', 'distant_light', 'mesh_light', 'photometric_light', 
        'point_light', 'quad_light', 'skydome_light', 'spot_light']
        
        operater_list = ['materialx']

        # Create a shading node with its name and node type parameters set to the
        # chosen shader type name as given in `value`
        node = CreateArnoldShadingNode(value, parentNode)

        # Ensure that the input and output parameters are shown when the user opens
        # the input or output parameter popup by clicking the triangle on the left
        # or right side of the shading node
        node.checkDynamicParameters()

        # Define a list to collect all nodes created in this function, starting
        # with the main shading node we created above
        nodes.append(node)

        # surface shader connected to NetworkMaterial
        if value in closure_list:
            # Create a NetworkMaterial node
            networkMaterialNode = NodegraphAPI.CreateNode('NetworkMaterial', parentNode)
            # Make sure to call RendererInfo::fillRendererShaderTypeTags
            networkMaterialNode.addShaderInputPort('arnold', 'surface')
            nodes.append(networkMaterialNode)

            # Connect the outColor output of the value node to the
            # input port dlSurface of the networkMaterialNode
            node.getOutputPort('out').connect(
                networkMaterialNode.getInputPort('arnoldSurface'))

            # Move the networkMaterialNode node to the right of the shading node
            x, y = NodegraphAPI.GetNodePosition(node)
            NodegraphAPI.SetNodePosition(networkMaterialNode, (x+250, y))
        # light shader connected to NetworkMaterial
        elif value in light_list:
            networkMaterialNode = NodegraphAPI.CreateNode('NetworkMaterial', parentNode)
            networkMaterialNode.addShaderInputPort('arnold', 'light')
            nodes.append(networkMaterialNode)

            node.getOutputPort('out').connect(
                networkMaterialNode.getInputPort('arnoldLight'))

            x, y = NodegraphAPI.GetNodePosition(node)
            NodegraphAPI.SetNodePosition(networkMaterialNode, (x+250, y))
        # operater shader connected to NetworkMaterial
        elif value in operater_list:
            networkMaterialNode = NodegraphAPI.CreateNode('NetworkMaterial', parentNode)
            networkMaterialNode.addShaderInputPort('arnold', 'operater')
            nodes.append(networkMaterialNode)

            node.getOutputPort('out').connect(
                networkMaterialNode.getInputPort('arnoldOperater'))

            x, y = NodegraphAPI.GetNodePosition(node)
            NodegraphAPI.SetNodePosition(networkMaterialNode, (x+250, y))


    elif renderer == 'prman':
        shadingNodeType = 'PrmanShadingNode'

        # Create the node, set its shader, and set the name with the shader name
        node = CreatePrmanShadingNode(value, parentNode)
        node.checkDynamicParameters()
        nodes.append(node)
    elif renderer == 'vray':
        shadingNodeType = 'VrayShadingNode'
    elif renderer == 'Redshift':
        shadingNodeType = 'RedshiftShadingNode'
    else:
        node = NodegraphAPI.CreateNode("NetworkMaterial")

    # Check if more than one node has been created, and if so, make them move
    # along with the pointer in the Node Graph tab
    # TODO: Once it's possible to return a list of nodes by an action callback
    #       of a layered menu (instead of a single node only), we can change
    #       this function here to simply return the list of `nodes` we built
    if len(nodes) > 1:
        nodeGraphTab.floatNodes(nodes)

        # Return nothing as we're done here
        return None
    else:
        # Return the single node that was created, which is then made to move
        # along with the pointer in the Node Graph tab by the code that calls
        # this action callback
        return node



# Create and register a layered menu using the above callbacks
# This is where you can set the hotkey
layeredMenu = LayeredMenuAPI.LayeredMenu(PopulateCallback, ActionCallback,
                                         'S', alwaysPopulate=False,
                                         onlyMatchWordStart=False)
LayeredMenuAPI.RegisterLayeredMenu(layeredMenu, 'Shading_Nodes')

