# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import NodegraphAPI

# Functions used by both the node and editor, but not part of the public API.

# We use hidden parameters on the SuperTool node to reference internal nodes
# using an expression.  Since expressions automatically track node renames,
# evaluating the parameter value will always give you the correct name for
# the internal node.
def GetRefNode(gnode, key):
    p = gnode.getParameter('node_'+key)
    if not p:
        return None
    
    return NodegraphAPI.GetNode(p.getValue(0))

def GetMergeNode(gnode):
    return GetRefNode(gnode, 'merge')

def AddNodeReferenceParam(destNode, paramName, node):
    param = destNode.getParameter(paramName)
    if not param:
        param = destNode.getParameters().createChildString(paramName, '')
    
    param.setExpression('getNode(%r).getNodeName()' % node.getName())

def GetPonyNode(gnode, index):
    mergeNode = GetMergeNode(gnode)
    port = mergeNode.getInputPortByIndex(index)
    if port and port.getNumConnectedPorts():
        return port.getConnectedPort(0).getNode()
    return None

def CleanupInputPorts(node):
    for i in xrange(node.getNumInputPorts()-1, -1, -1):
        port = node.getInputPortByIndex(i)
        if port.getNumConnectedPorts() == 0:
            node.removeInputPort(port.getName())

def LayoutInputNodes(node):
    spacing = (200, 100)
    pos = NodegraphAPI.GetNodePosition(node)
    x = pos[0] - (spacing[0] * (node.getNumInputPorts()-1))/2
    y = pos[1] + spacing[1]

    for port in node.getInputPorts():
        if port.getNumConnectedPorts():
            inputNode = port.getConnectedPort(0).getNode()
            NodegraphAPI.SetNodePosition(inputNode, (x, y))
        x += spacing[0]
