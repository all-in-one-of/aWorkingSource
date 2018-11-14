import logging
import textwrap
log = logging.getLogger('GafferGhost')
from Katana import NodegraphAPI, Nodes3DAPI, AssetAPI
from Katana import QtCore, QtGui
import ScriptActions as SA
from Katana import DrawingModule

class GafferGhostNode(NodegraphAPI.SuperTool):
    def __init__(self):
        self.hideNodegraphGroupControls()
        self.setContentLocked(True)

        self.addInputPort("input")

        self.addOutputPort("output")

        ######################################################################
        # Create parameters specific to our SuperTool.
        #
        # See _ExtraHints at the bottom of this file to see how we specify how
        # these parameters should be displayed in the UI by specifying built-in
        # widgets.
        ######################################################################
        self.getParameters().createChildGroup('tagets', 2)
        self.getParameter('tagets').createChildString('tagetObject', '')
        self.getParameter('tagets').createChildString('tagetLight', '')
        self.getParameters().createChildGroup('tuning', 1)
        self.getParameters().createChildNumberArray('translate', 3)
        self.getParameters().createChildNumberArray('rotatexyz', 3)
        self.getParameters().createChildNumberArray('scaleback', 3)
        # Switch Node
        self.switch_node = NodegraphAPI.CreateNode('Switch', self)
        self.switch_node.setName("LookSwitch")
        self.switch_node.addInputPort("input")

        self.opScript_node = NodegraphAPI.CreateNode('OpScript', self)
        self.opScript_node.setName("MaterialAssignedAttr")
        self.opScript_node.getParameter('CEL').setExpression(
            """'/root/world//*{ attr( "type" ) == "subdmesh"  or attr( "type" ) == "polymesh"}'""")
        self.opScript_node.getParameter('script.lua').setValue(self.__getMaterialAssignedOpScript(), 0)

        self.merge_node = NodegraphAPI.CreateNode('Merge', self)
        self.merge_node.addInputPort('in')
        self.merge_node.setName('MaterialXOutput')

        # We don't expose the parameters in the SuperTool's UI but you can
        # see them if you were to run something similar to:
        #
        # NodegraphAPI.GetNode('<SuperToolNodeName>').getParameters().getXML()
        SA.AddNodeReferenceParam(self, 'node_switch', self.switch_node)
        SA.AddNodeReferenceParam(self, 'node_opScript', self.opScript_node)
        SA.AddNodeReferenceParam(self, 'node_merge', self.merge_node)

        # Connect the internal node network together and then connect it to the
        # output port of the SuperTool.
        supertools_inputport = self.getSendPort("input")
        switch_node_input = self.switch_node.getInputPort("input")
        switch_node_input.connect(supertools_inputport)
        

        switch_node_output = self.switch_node.getOutputPortByIndex(0)
        opScript_node_input = self.opScript_node.getInputPortByIndex(0)
        switch_node_output.connect(opScript_node_input)

        opScript_node_output = self.opScript_node.getOutputPortByIndex(0)
        merge_node_input = self.merge_node.getInputPortByIndex(0)
        opScript_node_output.connect(merge_node_input)

        supertools_return = self.getReturnPort("output")
        merge_node_output = self.merge_node.getOutputPortByIndex(0)
        supertools_return.connect(merge_node_output)

        # Position the nodes in the internal node network so it looks a bit
        # more organised.
        NodegraphAPI.SetNodePosition(self.switch_node, (0, 0))
        NodegraphAPI.SetNodePosition(self.opScript_node, (0, -50))
        NodegraphAPI.SetNodePosition(self.merge_node, (0, -120))

    def addParameterHints(self, attrName, inputDict):
        """
        This function will be called by Katana to allow you to provide hints
        to the UI to change how parameters are displayed.
        """
        inputDict.update(_ExtraHints.get(attrName, {}))

    @classmethod
    def __getMaterialAssignedOpScript(self):
        return textwrap.dedent(
            """
            -- Get the current time
            local time = Interface.GetCurrentTime()

            -- Get the 'sourceLocation' user parameter
            local sourceLocation = Interface.GetInputLocationPath()

            -- Get the source's bounding box
            local bound = Interface.GetBoundAttr(sourceLocation)

            -- Retrieve the source global transformation, then flatten it
            local xform1 = Interface.GetGlobalXFormGroup(sourceLocation, 0)
            local xform2 = XFormUtils.CalcTransformMatrixAtExistingTimes(xform1):getNearestSample(time)

            -- Transform the bounding box
            local p = Interface.GetTransformedBoundAttrPoints(bound, time, xform2)
            local p_min,p_max = Interface.GetTransformedBoundAttrMinMax(bound, time, xform2)

            -- Construct new points attribute. The order of points returned by 
            -- `GetTransformedBoundAttrPoints` differs from the default cube, so we re-order
            -- them to preserve faces.
            local p0 = {
                (p_min[1]+p_max[1])/2,
                (p_min[2]+p_max[2])/2,
                (p_min[3]+p_max[3])/2
            }
            local p1 = {
                p_min[1],p_min[2],
                p_min[3],p_max[1],
                p_max[2],p_max[3]
            }
            Interface.SetAttr("GafferGhost.position.pivot", DoubleAttribute(p0, 3))
            Interface.SetAttr("GafferGhost.bound", FloatAttribute(p1, 2))
            """).strip()

    def widget(self):
        pass
    def bake(self):
        pass

_ExtraHints = {
    'GafferGhost.tagets': {
        'widget': 'group',
        'open': 'True',
        'help':
            """
            """,
    },
    'GafferGhost.tagets.tagetObject': {
        'widget': 'scenegraphLocation',
        'help':
            """
            """,
    },
    'GafferGhost.tagets.tagetLight': {
        'widget': 'scenegraphLocation',
        'help':
            """
            """,
    },
    'GafferGhost.tuning': {
        'widget': 'group',
        'open': 'True',
        'help':
            """
            """,
    },
    'GafferGhost.translate': {
        'help':
            """
            """,
    },
    'GafferGhost.rotatexyz': {
        'help':
            """
            """,
    },
    'GafferGhost.scaleback': {
        'help':
            """
            """,
    },
}