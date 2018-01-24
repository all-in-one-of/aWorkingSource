# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import NodegraphAPI
from Katana import Nodes3DAPI
from Katana import Naming
from Katana import Configuration
import logging
import os.path

#//////////////////////////////////////////////////////////////////////////////
#// Globals

HORIZONTAL_NODE_OFFSET = 200
RENDER_NODE_POSITION = -100
FORK_PORT_START_INDEX = 1
SHADOW_BRANCH_PORT_START_INDEX = 1
log = logging.getLogger("ShadowManager.ScriptActions")

#//////////////////////////////////////////////////////////////////////////////
def GetForkPortStartIndex():
    return FORK_PORT_START_INDEX

#//////////////////////////////////////////////////////////////////////////////
def GetShadowBranchPortStartIndex():
    return SHADOW_BRANCH_PORT_START_INDEX

#//////////////////////////////////////////////////////////////////////////////

def AddNodeReferenceParam(destNode, paramName, node):
    param = destNode.getParameter(paramName)
    if not param:
        param = destNode.getParameters().createChildString(paramName, '')

    param.setExpression('getNode(%r).getNodeName()' % node.getName())

#//////////////////////////////////////////////////////////////////////////////

def GetNodeReference(node, key, raiseOnError=True):
    p = node.getParameter(key)
    if not p:
        if raiseOnError:
            raise RuntimeError('internal network corrupt: %s', key)
        return

    now = NodegraphAPI.GetCurrentTime()
    node = NodegraphAPI.GetNode(p.getValue(now))

    if not node and raiseOnError:
        raise RuntimeError('internal network corrupt: %s', key)

    return node

#//////////////////////////////////////////////////////////////////////////////

def InitializeNode(smNode):
    smNode.hideNodegraphGroupControls()
    smNode.addInputPort('inputScene')
    smNode.addInputPort('sharedDeps')
    smNode.addOutputPort('originalScene')

    # Merge node exists so that we have a valid scene even with
    # nothing plugged in
    mergeNode = NodegraphAPI.CreateNode('Merge', smNode)
    mergeNode.addInputPort('i0').connect(
            smNode.getSendPort('inputScene'))


    mergeDot = NodegraphAPI.CreateNode('Dot', smNode)
    mergeDot.getInputPortByIndex(0).connect(mergeNode.getOutputPortByIndex(0))
    attr = mergeDot.getAttributes()
    attr['ns_basicDisplay'] = True
    mergeDot.setAttributes(attr)

    sceneDot1 = NodegraphAPI.CreateNode('Dot', smNode)
    sceneDot1.getInputPortByIndex(0).connect(mergeDot.getOutputPortByIndex(0))
    sceneDot1.getOutputPortByIndex(0).connect(
            smNode.getReturnPort('originalScene'))


    sharedDepDotNode = NodegraphAPI.CreateNode('Dot', smNode)
    sharedDepDotNode.getInputPortByIndex(0).connect(
            smNode.getSendPort('sharedDeps'))

    NodegraphAPI.SetNodePosition(mergeNode, (-200, 100))
    NodegraphAPI.SetNodePosition(mergeDot, (-200, 0))
    NodegraphAPI.SetNodePosition(sharedDepDotNode, (100, 100))

    NodegraphAPI.SetNodePosition(sceneDot1, (-350, RENDER_NODE_POSITION))

    AddNodeReferenceParam(smNode, 'mergeNode', mergeNode)
    AddNodeReferenceParam(smNode, 'mergeDot', mergeDot)
    AddNodeReferenceParam(smNode, 'sharedDepDotNode', sharedDepDotNode)

#//////////////////////////////////////////////////////////////////////////////

def GetUniquePassName(smNode, passName):
    existingNames = smNode.getRenderPassNames()

    return Naming.UniqueName.GetUniqueName(Naming.GetSafeIdentifier(passName),
            existingNames.__contains__)

#//////////////////////////////////////////////////////////////////////////////

def AddRenderPass(smNode, passName):
    now = NodegraphAPI.GetCurrentTime()

    # Create unique pass name and add name to main ShadowManager node class
    passName = GetUniquePassName(smNode, passName)
    smNode.getRenderPassNames().append(passName)

    # Get node references
    mergeNode = GetNodeReference(smNode, 'mergeNode')
    mergeDot = GetNodeReference(smNode, 'mergeDot')

    # Add input and output to merge dot
    iport = mergeDot.addInputPort('in1')
    oport = mergeDot.addOutputPort('out1')
    iport.connect(mergeNode.getOutputPortByIndex(0))


    # Create container group for a pass
    passGroup = NodegraphAPI.CreateNode('Group', smNode)
    passGroup.addInputPort('in')
    passGroup.setName(passName)
    passGroup.setType('ShadowManagerPassPackage')
    passGroup.getParameters().createChildString('passName', passName)
    passParamGrp = passGroup.getParameters().createChildGroup('lightPasses')

    # Connect group to dot after merge
    passGroup.getInputPortByIndex(0).connect(
            mergeDot.getOutputPortByIndex(oport.getIndex()))

    ###########################################################################
    # Create Group Stack node assigning correct shadow file to light pass
    groupStackNode = NodegraphAPI.CreateNode('GroupStack', passGroup)
    NodegraphAPI.SetNodePosition(groupStackNode, (-450, -100))

    # Connect group stack node to group
    groupStackNode.getInputPortByIndex(0).connect(
            passGroup.getSendPort(
                    passGroup.getInputPortByIndex(
                            0).getName()))

    newPort = passGroup.addOutputPort("outGroupStack")
    groupStackNode.getOutputPortByIndex(0).connect(
            passGroup.getReturnPort(
                    passGroup.getOutputPortByIndex(0).getName()))


    ###########################################################################
    depMergeNode = NodegraphAPI.CreateNode('DependencyMerge', passGroup)
    NodegraphAPI.SetNodePosition(depMergeNode, (-150, -200))

    # Link with pass group
    passGroup.addOutputPort("outDeps")
    depMergeNode.getOutputPortByIndex(0).connect(
            passGroup.getReturnPort(
                    passGroup.getOutputPortByIndex(1).getName()))


    ###########################################################################
    # Create ShadowBranch node
    shadowBranchNode = NodegraphAPI.CreateNode('ShadowBranch', passGroup)
    NodegraphAPI.SetNodePosition(shadowBranchNode, (-150, 0))

    # Connect shadow branch node to group
    shadowBranchNode.getInputPortByIndex(0).connect(
            passGroup.getSendPort(
                    passGroup.getInputPortByIndex(
                            0).getName()))


    # Position nodes and rebuild outputs
    PositionNodes(smNode)
    BuildOutputs(smNode)

    scriptItem = smNode.RenderPassScriptItem(passGroup)

    return scriptItem

#//////////////////////////////////////////////////////////////////////////////

def AddLightPass(smNode, lightName, renderPassItem, atIndex=None):
    passGroup = renderPassItem.getGroupNode()
    passName = renderPassItem.getName()
    shadowBranchNode = renderPassItem.getShadowBranchNode()

    newIndex = shadowBranchNode.getNumOutputPorts()
    shadowBranchNode.addOutputPort("out%i" % newIndex)

    # Add light pass and add dependencies
    scriptItem = smNode.RenderPassScriptItem(passGroup)
    nodeList = AddLightPassBranch(smNode, scriptItem, lightName)
    lightPassGroupNode = nodeList[0]

    # Link nodes internally
    WireInlineNodes(None, nodeList, (newIndex-1)*HORIZONTAL_NODE_OFFSET, -100)

    # Link with Shadow Branch
    lightPassGroupNode.getInputPortByIndex(0).connect(
            shadowBranchNode.getOutputPortByIndex(newIndex))

    # Create light item
    lightScriptItem = smNode.LightScriptItem(lightPassGroupNode)

    # If index to insert at it provided, reorder ports
    if atIndex != None:
        # Run ReorderLightPass that will reorder things and position nodes
        ReorderLightPass(lightScriptItem, atIndex)
    else:
        # Position nodes
        PositionNodes(smNode)


    return lightScriptItem

#//////////////////////////////////////////////////////////////////////////////

def AddLightPassBranch(smNode, renderPassItem, lightPath):
    now = NodegraphAPI.GetCurrentTime()

    # Get node references
    #depNode = GetNodeReference(smNode, 'depMergeNode')
    sharedDepDotNode = GetNodeReference(smNode, 'sharedDepDotNode')
    passGroup = renderPassItem.getGroupNode()

    ## LIGHT PASS
    lightName = lightPath.split('/')[-1]

    # Create container group for a light of a pass
    lightPassGroup = NodegraphAPI.CreateNode('Group', passGroup)
    lightPassGroup.addInputPort('in')
    lightPassGroup.addOutputPort('out')
    lightPassGroup.setName(lightName)
    lightPassGroup.setType('ShadowManagerLightPassPackage')

    # Add user parameters to pass group node
    passParamGrp = passGroup.getParameter('lightPasses')
    lightPassParam = passParamGrp.createChildGroup('lightPass')
    lightPassParamName = lightPassParam.getName()
    lightPassParam.createChildString('lightPath', lightPath)
    renderLocParam = lightPassParam.createChildString('renderLocation', '')
    location = "getenv('KATANA_TMPDIR', '/tmp') + '/' + " \
               "passName + '_%s.#.tx'" % lightName
    renderLocParam.setExpression(location)

    ###########################################################################
    # Create RenderSettings node
    renderSettingsNode = NodegraphAPI.CreateNode('RenderSettings', lightPassGroup)
    expr = 'getParent().getParent().lightPasses.' + lightPassParamName + '.lightPath'
    camStr = 'args.renderSettings.cameraName'
    renderSettingsNode.getParameter(camStr + '.value').setExpression(expr)
    renderSettingsNode.getParameter(camStr + '.enable').setValue(1, now)

    ###########################################################################
    # Create RenderOutputDefine node
    outputStr = 'args.renderSettings.outputs.outputName'
    outputDefNode = NodegraphAPI.CreateNode('RenderOutputDefine', lightPassGroup)

    ###########################################################################
    # Enable shadowType
    outputDefNode.getParameter(outputStr + '.type.value').setValue('shadow', now)
    outputDefNode.getParameter(outputStr + '.type.enable').setValue(1, now)


    # Set location type to file
    locTypeStr = outputStr + '.locationType'
    outputDefNode.getParameter( locTypeStr + '.value').setValue('file', now)
    outputDefNode.getParameter( locTypeStr + '.enable').setValue(1, now)

    # After setting the locationType, we need to check for dynamic parameters,
    # in this case locationSettings.renderLocation.
    outputDefNode.checkDynamicParameters()

    # Enable render location
    renderLocStr = outputStr + '.locationSettings.renderLocation'
    outputDefNode.getParameter(renderLocStr + '.enable').setValue(1, now)

    # Set render location as expression from pass group parameter
    exprRenderLoc = 'getParent().getParent().lightPasses.' \
                        + lightPassParamName + '.renderLocation'
    outputDefNode.getParameter(renderLocStr + '.value').setExpression(
                                                                exprRenderLoc)

    ###########################################################################
    # Create render node
    renderNode = NodegraphAPI.CreateNode('Render', lightPassGroup)
    passName = "shadow_render_" + renderPassItem.getName() + "_" + lightName
    renderNode.getParameter('passName').setValue(passName, 0)

    renderNode.addInputPort('sharedDeps').connect(
            sharedDepDotNode.getOutputPortByIndex(0))

    renderNode.addOutputPort('primary').connect(
            lightPassGroup.getReturnPort(
                    lightPassGroup.getOutputPortByIndex(0).getName()))

    lightPassGroup.getOutputPortByIndex(0).connect(
            renderPassItem.getDepMergeNode().addInputPort('i0'))

    ###########################################################################
    # Create Material node
    groupStackNode = renderPassItem.getGroupStackNode()
    materialNode = NodegraphAPI.CreateNode('Material', groupStackNode)
    materialNode.getParameter('action').setValue('override materials',0)
    exprLightPath = 'getParent().getParent().lightPasses.' \
                        + lightPassParamName + '.lightPath'
    materialNode.getParameter('overrides.CEL').setExpression(exprLightPath,0)
    materialNode.getParameter('overrides.CEL').setExpressionFlag(True)

    attrs = materialNode.getParameter('overrides.attrs')
    attrPath = 'prmanLightParams.Shadow_File'
    matOverride = attrs.createChildGroup('materialOverride')
    shadowFileGroup = matOverride.createChildGroup(attrPath)
    shadowFileGroup.createChildString('type', 'StringAttr')
    shadowFileGroup.createChildString('__path', attrPath)
    shadowFileGroup.createChildNumber('enable', 1)
    exprParam = shadowFileGroup.createChildString('value', '')
    exprParam.setExpression(exprRenderLoc)

    WireInlineNodes(groupStackNode, groupStackNode.getChildren())


    ###########################################################################
    # Link nodes internally
    internalNodes = [renderSettingsNode, outputDefNode, renderNode]
    WireInlineNodes(None, internalNodes)

    ###########################################################################
    # Link first nodes with group input
    renderSettingsNode.getInputPortByIndex(0).connect(
            lightPassGroup.getSendPort(
                    lightPassGroup.getInputPortByIndex(
                            0).getName()))

    nodeList = [lightPassGroup]
    return nodeList

#//////////////////////////////////////////////////////////////////////////////

def WireInlineNodes(enclosingGroup, nodeList, x=0, y=0):
    # Position all nodes in the nodeList and connect them to each other
    for i in xrange(0, len(nodeList)):
        NodegraphAPI.SetNodePosition(nodeList[i], (x, i*-50+y))
        if i == 0:
            continue
        nodeList[i].getInputPortByIndex(0).connect(
                nodeList[i-1].getOutputPortByIndex(0))

    # Connect enclosing group, if one is specified
    if enclosingGroup:
        nodeList[0].getInputPortByIndex(0).connect(
                enclosingGroup.getSendPort(
                        enclosingGroup.getInputPortByIndex(
                                0).getName()))
        nodeList[-1].getOutputPortByIndex(0).connect(
                enclosingGroup.getReturnPort(
                        enclosingGroup.getOutputPortByIndex(0).getName()))

#//////////////////////////////////////////////////////////////////////////////

def GetRenderPasses(smNode):
    try:
        mergeDot = GetNodeReference(smNode, 'mergeDot')

        result = []
        for port in mergeDot.getOutputPorts()[FORK_PORT_START_INDEX:]:
            groupNode = port.getConnectedPorts()[0].getNode()
            result.append(groupNode)

        return [smNode.RenderPassScriptItem(n) for n in result]
    except:
        log.error("Could not get reference to merge dot.")
        return []


#//////////////////////////////////////////////////////////////////////////////

def GetRenderPassNames(smNode):
    try:
        mergeDot = GetNodeReference(smNode, 'mergeDot')

        result = []
        for port in mergeDot.getOutputPorts()[FORK_PORT_START_INDEX:]:
            passNode = port.getConnectedPorts()[0].getNode()
            nameParam = passNode.getParameter('passName')
            result.append(nameParam.getValue(0))

        return result
    except:
        log.error("Could not get reference to merge dot.")
        return []

#//////////////////////////////////////////////////////////////////////////////

def GetRenderPassIndex(renderPass):
    port = renderPass.getGroupNode().getInputPortByIndex(0)
    ports = port.getConnectedPorts()
    index = ports[0].getIndex() - FORK_PORT_START_INDEX
    return index

#//////////////////////////////////////////////////////////////////////////////

def GetLightPasses(smNode, renderPassItem):
    branchNode = renderPassItem.getShadowBranchNode()

    result = []
    for port in branchNode.getOutputPorts():
        if port.getNumConnectedPorts():
            groupNode = port.getConnectedPort(0).getNode()
            scriptItem = smNode.LightScriptItem(groupNode)
            result.append(scriptItem)

    return result

#//////////////////////////////////////////////////////////////////////////////

def RenameRenderPass(renderPass, newName):
    now = NodegraphAPI.GetCurrentTime()
    smNode = renderPass.getParent()
    oldName = renderPass.getName()
    
    # Return early if the name was not changed
    if oldName == newName:
        return oldName 

    # Make sure new name is unique
    newName = GetUniquePassName(smNode, newName)

    # Update parameter
    renderPass.getGroupNode().setName(newName)

    # Update group name
    renderPass.getGroupNode().getParameter('passName').setValue(newName, now)

    # Rebuild outputs of Shadow Manager node
    BuildOutputs(smNode)

    return newName

#//////////////////////////////////////////////////////////////////////////////

def DeleteRenderPass(renderPass):
    smNode = renderPass.getParent()
    index = renderPass.getIndex()
    groupNode = renderPass.getGroupNode()

    # Remove output port of the fork
    mergeDot = GetNodeReference(smNode, 'mergeDot')
    inPort = mergeDot.getInputPortByIndex(index+FORK_PORT_START_INDEX)
    outPort = mergeDot.getOutputPortByIndex(index+FORK_PORT_START_INDEX)
    mergeDot.removeInputPort(inPort.getName())
    mergeDot.removeOutputPort(outPort.getName())

    # Delete sub groups (passes)
    for lightPassItem in smNode.getLightPasses(renderPass):
        DeleteLightPass(lightPassItem)

    # Remove group node
    groupNode.delete()

    # Delete unused output ports of ShadowManager node
    for op in smNode.getOutputPorts():
        name = op.getName()
        rp = smNode.getReturnPort(name)
        if rp.getNumConnectedPorts() == 0:
            smNode.removeOutputPort(name)


    # Rebuild outputs of Shadow Manager node and position nodes
    BuildOutputs(smNode)
    PositionNodes(smNode)

#//////////////////////////////////////////////////////////////////////////////

def DeleteLightPass(lightPass):
    smNode = lightPass.getParent()
    groupNode = lightPass.getGroupNode()
    renderPassItem = lightPass.getRenderPassItem()
    depNode = renderPassItem.getDepMergeNode()
    passGroup = lightPass.getPassGroup()

    # Delete parameters group from pass group node
    params = passGroup.getParameter('lightPasses')
    grp = params.getChildByIndex(lightPass.getIndex())
    params.deleteChild(grp)

    # Delete Material node from group stack
    groupStackNode = renderPassItem.getGroupStackNode()
    matNode = groupStackNode.getChildByIndex(lightPass.getIndex())
    groupStackNode.deleteChildNode(matNode)
    if groupStackNode.getChildren():
        WireInlineNodes(groupStackNode, groupStackNode.getChildren())

    # Remove output port of the fork
    inputPortPassGroup = groupNode.getInputPorts()[0]
    shadowBranchPort = inputPortPassGroup.getConnectedPort(0)
    sbNode = shadowBranchPort.getNode()
    sbNode.removeOutputPort(shadowBranchPort.getName())

    # Delete output port from pass group to dep merge
    groupOutPort = groupNode.getOutputPortByIndex(0).getConnectedPort(0)
    portName = groupOutPort.getName()
    depNode.removeInputPort(portName)
    passGroup.removeOutputPort(portName)

    # Remove group node
    groupNode.delete()

    PositionNodes(smNode)

#//////////////////////////////////////////////////////////////////////////////

def PositionNodes(smNode):
    for i, passItem in enumerate(GetRenderPasses(smNode)):
        node = passItem.getGroupNode()
        NodegraphAPI.SetNodePosition(node,
                (HORIZONTAL_NODE_OFFSET * (i-1), RENDER_NODE_POSITION))

        # Organize light passes inside a render pass
        for j, lightPassItem in enumerate(GetLightPasses(smNode, passItem)):
            subnode = lightPassItem.getGroupNode()
            NodegraphAPI.SetNodePosition(subnode,
                    (HORIZONTAL_NODE_OFFSET * (j-1), RENDER_NODE_POSITION))


    return

#//////////////////////////////////////////////////////////////////////////////

def ReorderRenderPass(renderPass, toIndex):
    smNode = renderPass.getParent()
    fromIndex = renderPass.getIndex()

    if toIndex == fromIndex:
        return

    # Reorder output ports of merge node
    mergeDot = GetNodeReference(smNode, 'mergeDot')
    ReorderPort(mergeDot.getOutputPortByIndex(fromIndex +
        GetForkPortStartIndex()), toIndex + GetForkPortStartIndex())

    PositionNodes(smNode)
    BuildOutputs(smNode)

#//////////////////////////////////////////////////////////////////////////////

def ReorderLightPass(lightPass, toIndex):
    smNode = lightPass.getParent()
    fromIndex = lightPass.getIndex()
    renderPass = lightPass.getRenderPassItem()
    passGroup = renderPass.getGroupNode()

    if toIndex == fromIndex:
        return

    # Reorder output ports of merge node
    shadowBranch = renderPass.getShadowBranchNode()
    ReorderPort(shadowBranch.getOutputPortByIndex(fromIndex +
        GetForkPortStartIndex()), toIndex + GetForkPortStartIndex())

    # Reorder parameter groups on pass group node
    lightPassParam = passGroup.getParameter('lightPasses')
    paramGroup = lightPassParam.getChildByIndex(fromIndex)
    lightPassParam.reorderChild(paramGroup, toIndex)

    # Reorder dep merge inputs
    depMerge = renderPass.getDepMergeNode()
    ReorderPort(depMerge.getInputPortByIndex(fromIndex), toIndex)

    PositionNodes(smNode)

#//////////////////////////////////////////////////////////////////////////////

def BuildOutputs(smNode):
    # Rename all outputs except first (originalScene)
    # This will prevent name clashes when reordering ports
    for port in smNode.getOutputPorts()[GetForkPortStartIndex():]:
        smNode.renameOutputPort(port.getName(), "_tmpname_")

    # Create output ports for every pass
    for renderPass in smNode.getRenderPasses():
        passGroup = renderPass.getGroupNode()
        passName = renderPass.getName()

        # Determine index of the output node
        ind = renderPass.getIndex() * 2 + GetForkPortStartIndex()

        # Get current ports
        portScene = smNode.getOutputPortByIndex(ind)
        portDeps = smNode.getOutputPortByIndex(ind+1)
        ports = [portScene, portDeps]
        portSuffixes = ["Scene", "Deps"]

        for n, port in enumerate(ports):
            # If port does not exists, create it.
            if port == None:
                port = smNode.addOutputPort("_tmpname_")

            # Rename output ports
            portName = smNode.renameOutputPort(port.getName(),
                                                passName + portSuffixes[n])

            # Conncet output ports
            passGroup.getOutputPortByIndex(n).connect(
                                                smNode.getReturnPort(portName))

#//////////////////////////////////////////////////////////////////////////////

def ReorderPort(port, toIndex):
    portName = port.getName()
    fromIndex = port.getIndex()

    node = port.getNode()

    connections = port.getConnectedPorts()
    internalConnections = None

    if isinstance(node, NodegraphAPI.GroupNode):
        if port.getType() == port.TYPE_CONSUMER:
            internalPort = node.getSendPort(portName)
        else:
            internalPort = node.getReturnPort(portName)

        internalConnections = internalPort.getConnectedPorts()

    if port.getType() == port.TYPE_CONSUMER:
        node.removeInputPort(portName)
        port = node.addInputPortAtIndex(portName, toIndex)
    else:
        node.removeOutputPort(portName)
        port = node.addOutputPortAtIndex(portName, toIndex)

    for connectedPort in connections:
        port.connect(connectedPort)

    if internalConnections is not None:
        if port.getType() == port.TYPE_CONSUMER:
            internalPort = node.getSendPort(portName)
        else:
            internalPort = node.getReturnPort(portName)

        for connectedPort in internalConnections:
            internalPort.connect(connectedPort)

#//////////////////////////////////////////////////////////////////////////////
