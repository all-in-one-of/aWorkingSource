# Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.
"""
Example script that registers a layered menu for the B{Node Graph} tab, which
shows the names of available PRMan shaders and creates a PrmanShadingNode node
with the chosen shader set on it when one of the menu entries is chosen.
"""

from Katana import NodegraphAPI, RenderingAPI, LayeredMenuAPI, UI4
from RenderingAPI import RenderPlugins

def PopulateCallback(layeredMenu):
    """
    Callback for the layered menu, which adds entries to the given
    C{layeredMenu} based on the available PRMan shaders.

    @type layeredMenu: L{LayeredMenuAPI.LayeredMenu}
    @param layeredMenu: The layered menu to add entries to.
    """
    # Obtain a list of names of available PRMan shaders from the PRMan renderer
    # info plug-in
    rendererInfoPlugin = RenderPlugins.GetInfoPlugin('arnold')
    shaderType = RenderingAPI.RendererInfo.kRendererObjectTypeShader
    shaderNames = rendererInfoPlugin.getRendererObjectNames(shaderType)

    # Iterate over the names of shaders and add a menu entry for each of them
    # to the given layered menu, using a green-ish color
    for shaderName in shaderNames:
        layeredMenu.addEntry(shaderName, text=shaderName,
                             color=(0.3, 0.7, 0.3))

def ActionCallback(value):
    """
    Callback for the layered menu, which creates a PrmanShadingNode node and
    sets its B{nodeType} parameter to the given C{value}, which is the name of
    a PRMan shader as set for the menu entry in L{PopulateCallback()}.

    @type value: C{str}
    @rtype: C{object}
    @param value: An arbitrary object that the menu entry that was chosen
        represents. In our case here, this is the name of a PRMan shader as
        passed to the L{LayeredMenuAPI.LayeredMenu.addEntry()} function in
        L{PopulateCallback()}.
    @return: An arbitrary object. In our case here, we return the created
        PrmanShadingNode node, which is then placed in the B{Node Graph} tab
        because it is a L{NodegraphAPI.Node} instance.
    """
    # Create the node, set its shader, and set the name with the shader name
    node = NodegraphAPI.CreateNode('ArnoldShadingNode')

    node.getParameter('nodeType').setValue(value, 0)
    node.setName(value)
    node.getParameter('name').setValue(node.getName(), 0)
    NodegraphAPI.SetNodeEdited(node, True, True)
    return node


# Create and register a layered menu using the above callbacks
layeredMenu = LayeredMenuAPI.LayeredMenu(PopulateCallback, ActionCallback,
                                         '`', alwaysPopulate=False,
                                         onlyMatchWordStart=False)
LayeredMenuAPI.RegisterLayeredMenu(layeredMenu, 'ArnoldShaders')

