import logging
import textwrap
log = logging.getLogger('GafferThreeRigs')
from Katana import NodegraphAPI, Nodes3DAPI, AssetAPI
from Katana import QtCore, QtGui
import ScriptActions as SA
from Katana import DrawingModule

import sys,os
api_dir = os.path.join(os.path.dirname(__file__), 'python')
sys.path.append(api_dir)

class GafferThreeRigsNode(NodegraphAPI.SuperTool):
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

        self.getParameters().createChildGroup('LightTypes', 3)
        self.getParameter('LightTypes').createChildString('KeyLight', 'SpotLight')
        self.getParameter('LightTypes').createChildString('FillLight', 'SpotLight')
        self.getParameter('LightTypes').createChildString('BackLight', 'SpotLight')
        self.getParameter('LightTypes').createChildString('RimLight', 'SpotLight')

        self.getParameters().createChildGroup('Setting', 2)
        self.getParameter('Setting').createChildNumber('useCommonKeyLight', 0)
        self.getParameter('Setting').createChildNumber('useCommonLightRig', 0)

        self.getParameters().createChildString('importRigsSet', '')
        self.getParameters().createChildString('updateParameters', '')


        # Switch Node
        self.switch_node = NodegraphAPI.CreateNode('Switch', self)
        self.switch_node.setName("GTR_Input_")
        self.switch_node.addInputPort("in")

        self.group_stack_node = NodegraphAPI.CreateNode('GroupStack', self)
        self.group_stack_node.setName("GTR_AttributeSet_")
        self.group_stack_node.setChildNodeType("AttributeSet")

        self.opScript_node = NodegraphAPI.CreateNode('OpScript', self)
        self.opScript_node.setName("GTR_OpScript_")
        self.opScript_node.getParameter('CEL').setValue("/root/world/geo/assets", 0)
        self.opScript_node.getParameter('script.lua').setValue(self.__getOpScript(), 0)

        self.merge_node = NodegraphAPI.CreateNode('Merge', self)
        self.merge_node.addInputPort('in')
        self.merge_node.setName('GTR_Output_')

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
    def importRigsSet(self):
        import GafferThreeRigsAPI
        reload(GafferThreeRigsAPI)
        GafferThreeRigsAPI.importRigsSet(self)
    def updateParameters(self):
        import GafferThreeRigsAPI
        reload(GafferThreeRigsAPI)
        GafferThreeRigsAPI.updateParameters(self)
    @classmethod
    def __getOpScript(self):
        return textwrap.dedent(
            """
            -- OpScripts
            function splitLast(inputstr, sep)
                    if sep == nil then
                            sep = "%s"
                    end
                    local last = ""
                    for str in string.gmatch(inputstr,"([^"..sep.."]+)") do
                            last = str
                    end
                    return last
            end
            local sourceLocation = Interface.GetInputLocationPath()
            local chr_name = splitLast(sourceLocation,"/")
            local light_rig = "/root/world/lgt/gaffer/chr_layer/"..chr_name
            local xform = Interface.GetAttr("xform", light_rig)
            Interface.SetAttr("LightChaserAnimation.lightRig.rigName",StringAttribute(chr_name))
            Interface.SetAttr("LightChaserAnimation.lightRig.targetRig",StringAttribute(light_rig))
            Interface.SetAttr("LightChaserAnimation.lightRig.xform",xform)
            """).strip()


_ExtraHints = {
    'GafferThreeRigs.rootLocations': {
        'widget': 'scenegraphLocationArray',
        'open': 'True',
        'help':
            """
            """,
    },
    'GafferThreeRigs.LightTypes': {
        'widget': 'group',
        'open': 'False',
        'help':
            """
            """,
    },
    'GafferThreeRigs.LightTypes.KeyLight': {
        'widget': 'popup', 
        'options': ['SpotLight','QuadLight','PointLight','DistantLight','FilterLight','GoboSpotLight'],
        'help':
            """
            """,
    },
    'GafferThreeRigs.LightTypes.FillLight': {
        'widget': 'popup', 
        'options': ['SpotLight','QuadLight','PointLight','DistantLight','FilterLight','GoboSpotLight'],
        'help':
            """
            """,
    },
    'GafferThreeRigs.LightTypes.BackLight': {
        'widget': 'popup', 
        'options': ['SpotLight','QuadLight','PointLight','DistantLight','FilterLight','GoboSpotLight'],
        'help':
            """
            """,
    },
    'GafferThreeRigs.LightTypes.RimLight': {
        'widget': 'popup',
        'options': ['SpotLight','QuadLight','PointLight','DistantLight','FilterLight','GoboSpotLight'],
        'help':
            """
            """,
    },
    'GafferThreeRigs.Setting': {
        'widget': 'group',
        'open': 'True',
        'help':
            """
            """,
    },
    'GafferThreeRigs.Setting.useCommonKeyLight': {
        'widget': 'checkBox', 
        'help':
            """
            """,
    },
    'GafferThreeRigs.Setting.useCommonLightRig': {
        'widget': 'checkBox',
        'help':
            """
            """,
    },
    'GafferThreeRigs.importRigsSet': {
        'widget': 'scriptButton',
        'scriptText': 'node.importRigsSet()',
        'buttonText': 'Import Rigs Set',
        'help':
            """
            """,
    },
    'GafferThreeRigs.updateParameters': {
        'widget': 'scriptButton',
        'scriptText': 'node.updateParameters()',
        'buttonText': 'Update Parameters',
        'help':
            """
            """,
    },
}