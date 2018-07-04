# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import NodegraphAPI, Utils, UniqueName

from Upgrade import Upgrade
import ScriptActions as SA
import logging

log = logging.getLogger("PonyStack.Node")


class PonyStackNode(NodegraphAPI.SuperTool):
    def __init__(self):
        self.hideNodegraphGroupControls()
        self.addOutputPort('out')
        
        # Hidden version parameter to detect out-of-date internal networks and upgrade it.
        self.getParameters().createChildNumber('version', 1)

        # Parameter for parent location of all the ponies.
        self.getParameters().createChildString('location', '/root/world/geo')

        self.__buildDefaultNetwork()

    def __buildDefaultNetwork(self):
        mergeNode = NodegraphAPI.CreateNode('Merge', self)
        SA.AddNodeReferenceParam(self, 'node_merge', mergeNode)

        self.getReturnPort(self.getOutputPortByIndex(0).getName()).connect(
            mergeNode.getOutputPortByIndex(0))

    # Public API - used by the UI, but can also be called directly by scripts
    def addPony(self, desiredName, index=-1):
        Utils.UndoStack.OpenGroup('Add pony to node "%s"' % self.getName())
        try:
            ponyName = desiredName.replace('/', '_')
            ponyName = UniqueName.GetUniqueName(ponyName, self.getPonyNames().__contains__)

            mergeNode = SA.GetMergeNode(self)

            ponyNode = NodegraphAPI.CreateNode('PonyCreate', self)
            if index>=0:
                ponyPort = mergeNode.addInputPortAtIndex('pony', index)
            else:
                ponyPort = mergeNode.addInputPort('pony')
            ponyNode.getOutputPortByIndex(0).connect(ponyPort)
            nameParam = ponyNode.getParameter('name')
            nameParam.setExpression('getParent().location + \'/\' + %r' % ponyName)
            nameParam.setExpressionFlag(True)

            self.cleanupInternalNetwork()

            return (ponyName, ponyPort.getIndex())
        finally:
            Utils.UndoStack.CloseGroup()

    def deletePony(self, index):
        Utils.UndoStack.OpenGroup('Delete pony from node "%s"' % self.getName())
        try:
            mergeNode = SA.GetMergeNode(self)

            ponyPort = mergeNode.getInputPortByIndex(index)
            if not ponyPort: return
            if not ponyPort.getNumConnectedPorts(): return

            ponyNode = ponyPort.getConnectedPort(0).getNode()
            ponyNode.delete()

            self.cleanupInternalNetwork()
        finally:
            Utils.UndoStack.CloseGroup()

    def reorderPony(self, oldIndex, newIndex):
        Utils.UndoStack.OpenGroup('Reorder pony in node "%s"' % self.getName())
        try:
            mergeNode = SA.GetMergeNode(self)
            NodegraphAPI.Util.ReorderInputPorts(mergeNode, oldIndex, newIndex)
            self.cleanupInternalNetwork()
        finally:
            Utils.UndoStack.CloseGroup()

    def cleanupInternalNetwork(self):
        mergeNode = SA.GetMergeNode(self)
        SA.CleanupInputPorts(mergeNode)
        SA.LayoutInputNodes(mergeNode)
    
    def getPonyNames(self):
        ponyList = []
        rootLocation = self.getRootLocation()

        mergeNode = SA.GetMergeNode(self)
        for port in mergeNode.getInputPorts():
            if port.getNumConnectedPorts():
                ponyNode = port.getConnectedPort(0).getNode()
                ponyPath = ponyNode.getParameter('name').getValue(0)
                if ponyPath.startswith(rootLocation+'/'):
                    ponyList.append(ponyPath[len(rootLocation+'/'):])

        return ponyList

    def getRootLocation(self):
        return self.getParameter('location').getValue(0)

    def setRootLocation(self, rootLocation):
        self.getParameter('location').setValue(rootLocation, 0)

    def addParameterHints(self, attrName, inputDict):
        inputDict.update(_ExtraHints.get(attrName, {}))

    def upgrade(self):
        if not self.isLocked():
            Upgrade(self)
        else:
            log.warning('Cannot upgrade locked PonyStack node "%s".'
                        % self.getName())



_ExtraHints = {
    'PonyStack.location':{
        'widget':'newScenegraphLocation',
    },
}
