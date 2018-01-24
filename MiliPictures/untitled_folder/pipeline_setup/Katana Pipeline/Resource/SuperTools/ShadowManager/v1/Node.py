# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import NodegraphAPI
import ScriptActions as SA
from Util import undogroup

#/////////////////////////////////////////////////////////////////////////////


# Node flavors can be added to set certain options. By uncommenting the
# following line, one can prevent the ShadowManager from showing up in the
# node list. An instance of the Super Tool can still be created directly
# inside Katana using NodegraphAPI, e.g.:
# > NodegraphAPI.CreateNode('ShadowManager')
#
# NodegraphAPI.AddNodeFlavor("ShadowManager", "_hide")


class ShadowManagerNode(NodegraphAPI.SuperTool):
    """
    The main class of the node defining node-specific functions and script
    items
    """
    def __init__(self):
        """
        Initialize the Shadow Manager node
        """
        SA.InitializeNode(self)
    
    #//////////////////////////////////////////////////////////////////////////
    
    def getRenderPasses(self):
        """
        Returns a list of RenderPassScriptItem instances representing the
        render passes defined within the node.
        """
        return SA.GetRenderPasses(self)

    def getRenderPassNames(self):
        """
        Returns a list of the render pass names.
        """
        return SA.GetRenderPassNames(self)

    @undogroup("Add Shadow Manager Pass")
    def addRenderPass(self, passName):
        """
        Defines a new render pass within the node with the specified passName.
        If necessary, the passName is altered for uniqueness (amongst peers)
        or invalid characters.

        Returns the RenderPassScriptItem created.
        """
        return SA.AddRenderPass(self, passName)
    
    class RenderPassScriptItem:
        """
        This is the scripting interface for individual render passes within
        the node.
        """        
        def __init__(self, groupNode):
            """
            Constructor used internally. There's never a need to construct
            an instance of this class manually.
            """
            self.__groupNode = groupNode
        
        def getName(self):
            """
            Returns the name of the render pass.
            """
            index = self.getIndex()
            return self.getParent().getRenderPassNames()[index]

        def getIndex(self):
            """
            Returns the index of the render pass group connected to the merge
            node. The first port is used to pass the original scene to the
            first ouput (therefore we have to subtract
            SA.GetForkPortStartIndex())
            """
            port = self.__groupNode.getInputPortByIndex(0)
            ports = port.getConnectedPorts()
            return ports[0].getIndex() - SA.GetForkPortStartIndex()

        def getParent(self):
            """
            Returns the ShadowManager node which owns this render pass.
            """
            return self.__groupNode.getParent()

        def getGroupNode(self):
            """
            Returns the group of the render pass, containing the shadow branch,
            group stack and all light passes.
            """
            return self.__groupNode

        @undogroup("Rename Shadow Manager Pass")
        def setName(self, name):
            """
            Renames the render pass. If necessary, the specified name is
            altered for uniqueness (amongst peers) or invalid characters.
            
            Returns the new name.
            """
            return SA.RenameRenderPass(self, name)
        
        @undogroup("Delete Shadow Manager Pass")
        def delete(self):
            """
            Deletes the render pass from the ShadowManager node. All internal
            nodes representing the render pass are removed. 
            
            Returns None.
            """
            SA.DeleteRenderPass(self)
        
        @undogroup("Reorder Shadow Manager Pass")
        def reorder(self, newIndex):
            """
            Reorders the render pass to a specified index amongst its peers.
            
            Returns None.
            """
            SA.ReorderRenderPass(self, newIndex)
        
        def getShadowBranchNode(self):
            """
            Returns the shadow branch node of this render pass.
            """
            groupInpNode = self.__groupNode.getInputPortByIndex(0)
            if groupInpNode == None:
                return

            groupInpName = groupInpNode.getName()
            portProducer = self.__groupNode.getSendPort(groupInpName)

            if portProducer.getNumConnectedPorts() == 0:
                return

            # GroupStack node is connected to port 1 of the render pass node
            return portProducer.getConnectedPort(1).getNode()

        def getGroupStackNode(self):
            """
            Returns the group stack node inside the render pass node
            """
            groupInpPort = self.__groupNode.getInputPortByIndex(0)
            if groupInpPort == None:
                return

            groupInpName = groupInpPort.getName()
            portProducer = self.__groupNode.getSendPort(groupInpName)

            if portProducer.getNumConnectedPorts() == 0:
                return

            # GroupStack node is connected to port 0 of the render pass node
            return portProducer.getConnectedPort(0).getNode()

        def getDepMergeNode(self):
            """
            Returns the dependency merge node of the render pass node
            """
            od = self.getGroupNode().getReturnPort('outDeps')
            return od.getConnectedPorts()[0].getNode()

    SA.RenderPassScriptItem = RenderPassScriptItem
    
    #//////////////////////////////////////////////////////////////////////////
    
    def getLightPasses(self, renderPassItem):
        """
        Returns a list of LightScriptItem instances representing the
        light passes of a given render pass
        """
        return SA.GetLightPasses(self, renderPassItem)
    
    @undogroup("Add Light/Camera")
    def addLightPass(self, lightPath, renderPassItem, atIndex=None):
        """
        Defines a new light pass for the render pass specified. The lightPath
        parameter contains the scene graph location of the light.

        Returns the LightScriptItem of the light pass created.
        """       
        return SA.AddLightPass(self, lightPath, renderPassItem, atIndex)

    class LightScriptItem:
        def __init__(self, groupNode):
            """
            Initialize the script item
            """
            self.__groupNode = groupNode
        
        def getName(self):
            """
            Returns the name of the light pass, i.e. the scene graph location
            of the light.
            """
            now = NodegraphAPI.GetCurrentTime()
            renderPassNode = self.getRenderPassItem().getGroupNode()
            passesParamGroup = renderPassNode.getParameter('lightPasses')
            passParamGroup = passesParamGroup.getChildByIndex(self.getIndex())
            lightPathParam = passParamGroup.getChild('lightPath')
            return lightPathParam.getValue(now)

        def getIndex(self):
            """
            Interally, all light pass groups are connected to a shadow branch
            and the index is determined by looking at the index of the port a
            group is connected to. Since the first port of the shadow branch
            is not used, we have to subtract GetShadowBranchPortStartIndex().

            Returns the ID of the light pass group among his peers.
            """
            port = self.getGroupNode().getInputPortByIndex(0)
            ports = port.getConnectedPorts()
            return ports[0].getIndex() - SA.GetShadowBranchPortStartIndex()

        def getParent(self):
            """
            Returns the ShadowManager node.
            """
            return self.getPassGroup().getParent()

        def getPassGroup(self):
            """
            Returns the group of the render pass this light pass belongs to.
            """
            return self.__groupNode.getParent()

        def getRenderPassItem(self):
            """
            Returns the render pass item that this light pass belongs to.
            """
            return self.getParent().RenderPassScriptItem(self.getPassGroup())

        def getGroupNode(self):
            """
            Returns the group node of this light pass
            """
            return self.__groupNode

        def getRenderSettingsNode(self):
            """
            Returns the render settings node of this light pass
            """
            # Assuming, the render settings node first in the list
            return self.__groupNode.getChildren()[0]

        def getRenderOutputDefineNode(self):
            """
            Returns the render output define node of this light pass
            """
            # Assuming, the render output define node is second in the list
            return self.__groupNode.getChildren()[1]

        def getRenderNode(self):
            """
            Returns the render  node of this light pass
            """
            # Assuming, the render node is the last in the list!
            return self.__groupNode.getChildren()[-1]

        def isDisabled(self):
            """
            Returns the bypassed status of the group node
            """
            return self.__groupNode.isBypassed()
        
        @undogroup("Change Disabled State of Shadow Manager pass")
        def setDisabled(self, state):
            """
            Sets the bypassed status of the group node
            """
            self.__groupNode.setBypassed(state)

        @undogroup("Delete Light Pass")
        def delete(self):
            """
            Deletes the light pass
            """
            SA.DeleteLightPass(self)

        @undogroup("Reorder Light Pass")
        def reorder(self, newIndex):
            """
            Reorders the light pass to a specified index amongst its peers.

            Returns None.
            """
            SA.ReorderLightPass(self, newIndex)
