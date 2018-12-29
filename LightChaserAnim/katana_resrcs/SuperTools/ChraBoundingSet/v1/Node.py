import logging
import textwrap
log = logging.getLogger('ChraBoundingSet')
from Katana import NodegraphAPI, Nodes3DAPI, AssetAPI
from Katana import QtCore, QtGui
import ScriptActions as SA
from Katana import DrawingModule

class ChraBoundingSetNode(NodegraphAPI.SuperTool):
    def __init__(self):
        self.hideNodegraphGroupControls()
        self.setContentLocked(True)

        self.addInputPort("in")

        self.addOutputPort("out")

        ######################################################################
        # Create parameters specific to our SuperTool.
        #
        # See _ExtraHints at the bottom of this file to see how we specify how
        # these parameters should be displayed in the UI by specifying built-in
        # widgets.
        ######################################################################
        self.getParameters().createChildStringArray("rootLocations",1)


        # Switch Node
        self.switch_node = NodegraphAPI.CreateNode('Switch', self)
        self.switch_node.setName("CBS_Input_")
        self.switch_node.addInputPort("in")

        self.group_stack_node = NodegraphAPI.CreateNode('GroupStack', self)
        self.group_stack_node.setName("CBS_AttributeSet_")
        self.group_stack_node.setChildNodeType("AttributeSet")

        self.opScript_node = NodegraphAPI.CreateNode('OpScript', self)
        self.opScript_node.setName("CBS_OpScript_")
        self.opScript_node.getParameter('CEL').setValue("/root/world/geo/assets", 0)
        self.opScript_node.getParameter('script.lua').setValue(self.__getOpScript(), 0)

        self.merge_node = NodegraphAPI.CreateNode('Merge', self)
        self.merge_node.addInputPort('in')
        self.merge_node.setName('CBS_Output_')

        # We don't expose the parameters in the SuperTool's UI but you can
        # see them if you were to run something similar to:
        #
        # NodegraphAPI.GetNode('<SuperToolNodeName>').getParameters().getXML()
        SA.AddNodeReferenceParam(self, 'node_switch', self.switch_node)
        SA.AddNodeReferenceParam(self, 'node_groupStack', self.group_stack_node)
        SA.AddNodeReferenceParam(self, 'node_opScript', self.opScript_node)
        SA.AddNodeReferenceParam(self, 'node_merge', self.merge_node)

        # Connect the internal node network together and then connect it to the
        # output port of the SuperTool.
        supertools_inputport = self.getSendPort("in")
        switch_node_input = self.switch_node.getInputPort("in")
        switch_node_input.connect(supertools_inputport)
        

        switch_node_output = self.switch_node.getOutputPortByIndex(0)
        group_stack_node_input = self.group_stack_node.getInputPortByIndex(0)
        switch_node_output.connect(group_stack_node_input)

        group_stack_node_output = self.group_stack_node.getOutputPortByIndex(0)
        opScript_node_input = self.opScript_node.getInputPortByIndex(0)
        group_stack_node_output.connect(opScript_node_input)

        opScript_node_output = self.opScript_node.getOutputPortByIndex(0)
        merge_node_input = self.merge_node.getInputPortByIndex(0)
        opScript_node_output.connect(merge_node_input)

        supertools_return = self.getReturnPort("out")
        merge_node_output = self.merge_node.getOutputPortByIndex(0)
        supertools_return.connect(merge_node_output)

        # Position the nodes in the internal node network so it looks a bit
        # more organised.
        NodegraphAPI.SetNodePosition(self.switch_node, (0, 0))
        NodegraphAPI.SetNodePosition(self.group_stack_node, (0, -50))
        NodegraphAPI.SetNodePosition(self.opScript_node, (0, -100))
        NodegraphAPI.SetNodePosition(self.merge_node, (0, -150))

    def addParameterHints(self, attrName, inputDict):
        """
        This function will be called by Katana to allow you to provide hints
        to the UI to change how parameters are displayed.
        """
        inputDict.update(_ExtraHints.get(attrName, {}))

    @classmethod
    def __getOpScript(self):
        return textwrap.dedent(
            """
            local path = Interface.GetAttr("LightChaserAnimation.assembly.bodyGeometry")
            path_str = Attribute.GetStringValue(path,"")
            local bound = Interface.GetAttr("bound", path_str)
            doesBound = Interface.GetAttr("bound")
            if not doesBound then
                Interface.SetAttr("bound", bound)
            end
            """).strip()


_ExtraHints = {
    'ChraBoundingSet.rootLocations': {
        'widget': 'scenegraphLocationArray',
        'open': 'True',
        'help':
            """
            """,
    }
}