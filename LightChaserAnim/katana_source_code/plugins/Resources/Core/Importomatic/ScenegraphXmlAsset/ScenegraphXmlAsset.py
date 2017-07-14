# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import NodegraphAPI, Plugins, Utils, AssetAPI
import os
import xml.etree.ElementTree as ET
import logging
import random
import re

# Defines the logging level
LOG_LEVEL = logging.INFO

# Create a logger for this module with the given logging level
log = logging.getLogger("ScenegraphXmlAsset")
log.setLevel(LOG_LEVEL)

ImportomaticAPI = Plugins.ImportomaticAPI

g_iconBasePath = os.path.dirname(__file__)
g_iconPath = os.path.dirname(__file__)+'/ScenegraphXmlAsset.png'


#//////////////////////////////////////////////////////////////////////////////

def Register():
    ImportomaticAPI.AssetModule.RegisterCreateCallback(
            'Add ScenegraphXml', FindAndAddScenegraphXmlGeometry)
    
    ImportomaticAPI.AssetModule.RegisterBatchCreateCallback(
            'ScenegraphXml', AddScenegraphXmlGeometry)
    
    ImportomaticAPI.AssetModule.RegisterType(
            'ScenegraphXml', ScenegraphXmlModule())

AddLookFileAction = None
AddAttributeFileAction = None

def RegisterGUI():
    from Katana import QtGui, QtCore

    global AddLookFileAction
    global AddAttributeFileAction

    class AddLookFileAction(QtGui.QAction):       
        
        def __init__(self, parent, assetName, itemClass):
            QtGui.QAction.__init__(self, "Assign Look File", parent)
            self.__assetName = assetName
            self.__itemClass = itemClass

            self.connect(self, QtCore.SIGNAL('triggered(bool)'), self.__go)

        def __go(self, checked=False):
            node = NodegraphAPI.GetNode(self.__assetName)            

            from Katana import UI4
            lookFile = UI4.Util.AssetId.BrowseForAsset('', 'Select file', False,
                        {'fileTypes':'klf',
                         'acceptDir': True,
                         'context': AssetAPI.kAssetContextLookFile}
            )

            if not lookFile:
                return

            self.__itemClass.appendLookFile(lookFile, self.__assetName)

    
    class AddAttributeFileAction(QtGui.QAction):

        def __init__(self, parent, assetName, itemClass):
            QtGui.QAction.__init__(self, "Assign Attribute File", parent)
            self.__assetName = assetName
            self.__itemClass = itemClass

            self.connect(self, QtCore.SIGNAL('triggered(bool)'), self.__go)

        def __go(self, checked=False):
            node = NodegraphAPI.GetNode(self.__assetName)

            from Katana import UI4
            attributeFile = UI4.Util.AssetId.BrowseForAsset('', 'Select file', False,
                        {'fileTypes': 'xml',
                         'context': AssetAPI.kAssetContextAttributeFile}
            )

            if not attributeFile:
                return

            self.__itemClass.appendAttributeFile(attributeFile, self.__assetName)

#//////////////////////////////////////////////////////////////////////////////

#//////////////////////////////////////////////////////////////////////////////

def getHashValue(node):
    return str(hash(repr(node)))


def connectAndPlaceNodes(groupNode):
    noNodes = groupNode.getNumChildren()
    stepSize = -80

    counter = 0
    lastNode = groupNode.getChildByIndex(0)
    for n in groupNode.getChildren():
        if n.getType() == "ScenegraphXml_In":
            pass
        else:
            counter = counter + 1
            NodegraphAPI.SetNodePosition(n, (0, stepSize * counter))
            lastNode.getOutputPortByIndex(0).connect(n.getInputPortByIndex(0))
            lastNode = n

    # Connect last node to return port
    lastNode.getOutputPortByIndex(0).connect(groupNode.getReturnPort('out'))


def addLookFileNodes(groupNode, lookRef, lookCel, ignore='False', createNode='True'):
    # -- node graph --
    noAssigns = 0

    assignNodes = []
    for childNode in groupNode.getChildren():
        if childNode.getType() == "LookFileAssign":
            assignNodes.append(childNode)
        
    if createNode == 'True':
        # - LookFileAssign -
        nodeKstdAssign = NodegraphAPI.CreateNode('LookFileAssign')
        nodeKstdAssign.setParent( groupNode )
        nodeKstdAssign.getParameter( 'CEL' ).setExpressionFlag( True )
        nodeKstdAssign.getParameter( 'CEL' ).setExpression( lookCel )
        nodeKstdAssign.getParameter( 'args.lookfile.asset.enable' ).setValue( 1, 0 )
        nodeKstdAssign.getParameter( 'args.lookfile.asset.value' ).setValue( lookRef, 0 )
        assignNodes.append(nodeKstdAssign)
    
        if ignore == 'True':
            nodeKstdAssign.setBypassed(True)
            
        return nodeKstdAssign

    return None

def addAttributeFileNodes(groupNode, attrRef, attrCel, attrCustomBrowser='',
                          groupName='attributeFile', ignore='False', createNode='True'):
    if createNode == 'True':
        # - AttributeFile_In -
        attrFileIn = NodegraphAPI.CreateNode('AttributeFile_In')
        attrFileIn.setParent( groupNode )
        attrFileIn.getParameter( 'CEL' ).setExpressionFlag( True )
        attrFileIn.getParameter( 'CEL' ).setExpression( attrCel )
        attrFileIn.getParameter( 'filepath' ).setValue( attrRef, 0 )
        attrFileIn.getParameter( 'sofilepath' ).setValue( attrCustomBrowser, 0 )
        if groupName is not None and groupName != '':
            attrFileIn.getParameter( 'groupAttr' ).setValue( groupName, 0 )
    
        if ignore == 'True':
            attrFileIn.setBypassed(True)
            
        return attrFileIn
    return None

def traverseHierarchy(node, hierarchy):
    if node.getName() != "ScenegraphXml":
        hierarchy.append(node.getName())
    
    parent = node.getParent()
    if parent is not None:
        traverseHierarchy(parent, hierarchy)

def findParentHierarchy(node):
    flatString = ""
    hierarchy = []
    traverseHierarchy(node, hierarchy)

    for item in reversed(hierarchy):
        flatString = flatString + item + "."

    return flatString


def placeAtTop(assetParam, extensionName):
    extensionParam = assetParam.getChild(extensionName)
    if extensionParam is not None:
        assetParam.reorderChild(extensionParam, 0)    


def reorderExtensions(assetParam):
    placeAtTop(assetParam, 'modifiers')
    placeAtTop(assetParam, 'looks')


def addAttributeFileParams(assetParam, attrRef, attrCel, attrCustomBrowser='',
                            groupName='attributeFile', allowDelete=True, createNode=True):
    assetAttributesParam = assetParam.getChild('modifiers')
    if not assetAttributesParam:
        assetAttributesParam = assetParam.createChildGroup('modifiers')

    assetAttr = assetAttributesParam.createChildGroup('modifier')
    name = attrRef[attrRef.rfind("/")+1:]
    assetAttr.createChildString('name', name)
    assetAttr.createChildString('path', attrRef)
    assetAttr.createChildString('cel', attrCel)
    assetAttr.createChildString('customBrowser', attrCustomBrowser)
    assetAttr.createChildString('groupName', groupName)
    assetAttr.createChildString('_type', 'attributeFile')
    assetAttr.createChildString('_allowDelete', str(allowDelete))
    assetAttr.createChildString('_ignore', 'False')
    assetAttr.createChildString('_createNode', str(createNode))

    reorderExtensions(assetParam)


def addLookFileParams(assetParam, lookRef, lookCel, allowDelete=True, createNode=True):
    assetLooksParam = assetParam.getChild('looks')
    if not assetLooksParam:
        assetLooksParam = assetParam.createChildGroup('looks')

    assetLook = assetLooksParam.createChildGroup('look')
    lookName = lookRef[lookRef.rfind("/")+1:]
    assetLook.createChildString('name', lookName)
    assetLook.createChildString('path', lookRef)
    assetLook.createChildString('cel', lookCel)
    assetLook.createChildString('_allowDelete', str(allowDelete))
    assetLook.createChildString('_ignore', 'False')
    assetLook.createChildString('_createNode', str(createNode))

    reorderExtensions(assetParam)


def getXmlAttrib(node, attrName, notFoundIsNone=False):
    try:
        attr = node.attrib[attrName]
        return attr
    except:
        if notFoundIsNone:
            return None
        else:
            return ''


def checkForLookFile(instance, asmbParam, lookCel):
    lookFile = instance.find("lookFile")

    if lookFile is not None:
        lookRef = getXmlAttrib(lookFile, "ref")
        addLookFileParams(asmbParam, lookRef, lookCel, False, False)
      
    return lookFile


def checkForModifiers(instance, asmbParam, attrCel):
    modifiers = instance.find("modifiers")

    if modifiers is not None:
        attributeFiles = modifiers.findall("attributeFile")
        for attribute in attributeFiles:
            attrRef = getXmlAttrib(attribute, "ref")
            customParser = getXmlAttrib(attribute, "customParser")
            groupName = getXmlAttrib(attribute, "groupName")

            addAttributeFileParams(asmbParam, attrRef, attrCel, customParser, groupName, False, False)
  
    return modifiers


def findInstances(instanceListNode, paramNode, node, basePath, sgPath):
    if instanceListNode is None:
        return

    instances = instanceListNode.findall("instance")
    for instance in instances:
        instanceList = instance.find("instanceList")
        instanceName = instance.attrib['name']
        instanceType = instance.attrib["type"]

        asmbParam = None

        sgExtendedPath = sgPath + "/" + instanceName        
        groupType = getXmlAttrib(instance, "groupType", True)

        if groupType is not None and groupType == "assembly":
            asmbParam = paramNode.getChild(instanceName)
            if not asmbParam:
                asmbParam = paramNode.createChildGroup(instanceName)
                asmbParam.createChildString('_ignore', 'False')
                asmbParam.createChildString('_sgPath', sgExtendedPath)

            lookFile = checkForLookFile(instance, asmbParam, sgExtendedPath)
            modifiers = checkForModifiers(instance, asmbParam, sgExtendedPath)
          
            findInstances(instanceList, asmbParam, node, basePath, sgExtendedPath)
        else:
            if instanceType == "group":
                if instance[0].tag == "instanceList":
                    pseudoAsmbParam = paramNode.createChildGroup(instanceName)
                    pseudoAsmbParam.createChildString('_ignore', 'False')
                    pseudoAsmbParam.createChildString('_sgPath', sgExtendedPath)
                    findInstances(instanceList, pseudoAsmbParam, node, basePath, sgExtendedPath)             
                else:
                    findInstances(instanceList, paramNode, node, basePath, sgExtendedPath)
                
        if instanceType == "reference":
            refType = instance.attrib["refType"]
            if refType == "xml":
                asmbRefParam = None
                refFile = instance.attrib["refFile"]
                fullRef = os.path.join(basePath, refFile)
                refDir = os.path.dirname(fullRef)
                if refDir:
                    nextBasePath = os.path.join(basePath, refDir)
                else:
                    nextBasePath = basePath
                
                if asmbParam:
                    asmbRefParam = asmbParam
                else:
                    asmbRefParam = paramNode.createChildGroup(instanceName)
                    asmbRefParam.createChildString('_ignore', 'False')
                    asmbRefParam.createChildString('_sgPath', sgExtendedPath)

                checkForLookFile(instance, asmbRefParam, sgExtendedPath)
                checkForModifiers(instance, asmbRefParam, sgExtendedPath)

                xmlTree = ET.parse(fullRef)
                xmlRoot = xmlTree.getroot()
                rootInstanceList = xmlRoot.find("instanceList")                
                findInstances(rootInstanceList, asmbRefParam, node, nextBasePath, sgExtendedPath)
                
            elif refType == "procedural":
                asmbParam = paramNode.getChild(instanceName)
                if not asmbParam:
                    asmbParam = paramNode.createChildGroup(instanceName)
                    asmbParam.createChildString('_ignore', 'False')
                    asmbParam.createChildString('_sgPath', sgExtendedPath)                

def FindAndAddScenegraphXmlGeometry(importomaticNode):
    from Katana import UI4
    assetId = UI4.Util.AssetId.BrowseForAsset('', 'Select file', False,
        {'fileTypes': 'xml',
         'context': AssetAPI.kAssetContextScenegraphXml}
    )
    
    if not assetId:
        return
    
    return AddScenegraphXmlGeometry(importomaticNode, assetId)

def AddScenegraphXmlGeometry(importomaticNode, assetId, locationExpression=None):
    
    filename = ''
    assetPlugin = AssetAPI.GetDefaultAssetPlugin()
    if assetPlugin.isAssetId(assetId):
        filename = assetPlugin.resolveAsset(assetId)
    else:
        return
    
    fileBase = os.path.basename(filename)
    rootParamName = os.path.splitext(fileBase)[0]

    # Parse XML to extract relevant information
    xmlTree = ET.parse(filename)
    xmlRoot = xmlTree.getroot()

    node = None

    # Find all instances of type reference
    rootInstanceList = xmlRoot.find("instanceList")
    if rootInstanceList is not None:
        rootInstance = rootInstanceList.find("instance")
        rootInstanceName = rootInstance.attrib["name"]
        rootInstanceType = rootInstance.attrib["type"]
        
        node = NodegraphAPI.CreateNode('Group')
        node.setName(rootParamName)
        # This seems a bit odd to set the name and then retrieve it but Katana
        # will change the name to a unique name which we can then extract
        uniqueRootName = node.getName()
        node.setType('ScenegraphXml')
        node.addOutputPort('out')
        
        xmlInNode = NodegraphAPI.CreateNode('ScenegraphXml_In', node)
        xmlInNode.getOutputPortByIndex(0).connect(node.getReturnPort('out'))

        # If the location parameter isn't
        # given try to intelligently guess one
        #
        if locationExpression:
            xmlInNode.getParameter('name').setExpression(locationExpression, True)
        else:
            baseLocation = xmlInNode.getParameter("name").getValue(0)
            location = baseLocation + '/' + uniqueRootName
            xmlInNode.getParameter('name').setValue(location, 0)
            
        xmlInNode.getParameter('asset').setValue(assetId, 0)
        basePath = os.path.dirname(filename) + os.sep
        
        assetInfoParam = node.getParameters().createChildGroup('assetInfo')
        assetInfoParam.createChildString('_ignore', 'False')
        assetInfoParam.createChildString('_sgPath', '/' + uniqueRootName)
        
        findInstances(rootInstanceList, assetInfoParam, node, basePath, "")

    BuildScenegraph(node)

    return node
    

def AddPruneNode(node):
    pruneNode = NodegraphAPI.CreateNode('Prune', node)
    return pruneNode


def CleanScenegraph(node):
    for childNode in node.getChildren():
        if childNode.getType() != "ScenegraphXml_In":
            childNode.delete()


def TraverseAndBuildNodes(node, param, sgNode, pruneNode):
    if param.getName() == "modifiers":
        modifierList = param.getChildren()
        for modParam in modifierList:
            modifierType = modParam.getChild('_type').getValue(0)
            if modifierType == "attributeFile":
                attrRef = modParam.getChild("path").getValue(0)
                attrCel = "getNode('%s').name" % sgNode.getName() + " + '" + modParam.getChild("cel").getValue(0) + "//*'"
                attrCustomBrowser = modParam.getChild("customBrowser").getValue(0)
                groupName = modParam.getChild("groupName").getValue(0)
                ignore = modParam.getChild('_ignore').getValue(0)
                
                createNode = "True"
                if modParam.getChild('_createNode'):
                    createNode = modParam.getChild('_createNode').getValue(0)

                afNode = addAttributeFileNodes(node, attrRef, attrCel, attrCustomBrowser, groupName, ignore, createNode)
                
                # If no node was created, use attrCel as hash 
                if afNode == None:
                    nodeHash = attrCel
                else:
                    nodeHash = getHashValue(afNode)
                    
                if modParam.getChild('_hash'):
                    modParam.getChild('_hash').setValue(nodeHash, 0)
                else:
                    modParam.createChildString('_hash', nodeHash)

    elif param.getName() == "looks":
        lookList = param.getChildren()
        for lookParam in lookList:
            lookRef = lookParam.getChild("path").getValue(0)
            lookCel = "getNode('%s').name" % sgNode.getName() + " + '" + lookParam.getChild("cel").getValue(0) + "'"
            ignore = lookParam.getChild('_ignore').getValue(0)
            
            createNode = "True"
            if lookParam.getChild('_createNode'):
                createNode = lookParam.getChild('_createNode').getValue(0)
            
            lfNode = addLookFileNodes(node, lookRef, lookCel, ignore, createNode)
            
            # If no node was created, use lookCel as hash 
            if lfNode == None:
                nodeHash = lookCel
            else:
                nodeHash = getHashValue(lfNode)
            
            if lookParam.getChild('_hash'):
                lookParam.getChild('_hash').setValue(nodeHash, 0)
            else:
                lookParam.createChildString('_hash', nodeHash)

    else:
        paramIgnore = param.getChild("_ignore")
        paramSgPath = param.getChild("_sgPath")

        if paramIgnore is not None and paramIgnore.getValue(0) == "True":
            cel = pruneNode.getParameter('cel').getExpression()

            if cel is not "":
                cel = cel + " + ' ' + "

            if paramSgPath is not None:
                sgXmlInNode = node.getChildByIndex(0)
                if sgXmlInNode is not None:
                    expressedPath = "getNode('%s').name" % sgXmlInNode.getName() + " + '" + paramSgPath.getValue(0) + "'"
                    cel = cel + expressedPath
    
            pruneNode.getParameter('cel').setExpressionFlag( True )
            pruneNode.getParameter('cel').setExpression( cel )

    if param.getNumChildren() > 0:
        connector = "-"
        for p in param.getChildren():
            TraverseAndBuildNodes(node, p, sgNode, pruneNode)    


def BuildScenegraph(node):
    CleanScenegraph(node)
    assetParam = node.getParameter("assetInfo")
    if assetParam is not None and node is not None and len(node.getChildren()) > 0:
        pruneNode = AddPruneNode(node)
        TraverseAndBuildNodes(node, assetParam, node.getChildByIndex(0), pruneNode)

    connectAndPlaceNodes(node)

                
def findNodeTypeByHash(groupNode, hashValue, searchType):
    for childNode in groupNode.getChildren():
        if childNode.getType() == searchType and getHashValue(childNode) == hashValue:            
            return childNode
            
    BuildScenegraph(groupNode)
    from Katana import UI4
    UI4.Widgets.MessageBox.Warning('Message', 'The Importomatic node has been rebuilt due to data inconsistency. Please try again.')

    return None  

def getUniqueName(term, importomaticNode):
    groupNode = importomaticNode.getChildByIndex(0)

    children = groupNode.getChildren()
    if len(children) == 0:
        return term

    items = [c.getName() for c in children]

    findNumberedTerm = '%s\d|%s' % (term, term)
    foundTerms = [''.join(re.findall(findNumberedTerm, i)) for i in items]

    if len(foundTerms) == 1 and foundTerms[0] == '':        
        newTerm = term
    else:
        findNumbers = '\d'
        foundNumbers = [''.join(re.findall(findNumbers, i)) for i in foundTerms]

        highestNumber = max(foundNumbers)
        if highestNumber == '':
            digits = 0
        else:
            digits = int(highestNumber)

        newTerm = term + str(digits + 1)

    return newTerm


#//////////////////////////////////////////////////////////////////////////////
#///
class ScenegraphXmlModule(ImportomaticAPI.AssetModule):

    def getAssetTreeRoot(self, node):
        return ScenegraphXmlTreeRoot(node)


class ScenegraphXmlTreeRoot(ImportomaticAPI.AssetTreeChild):
    
    def __init__(self, node):
        self.__node = node
        self.__sgNode = None
        self.__assetInfoParam = self.__node.getParameter('assetInfo')
        
        for childNode in self.__node.getChildren():
            if childNode.getType() == "ScenegraphXml_In":
                self.__sgNode = childNode
                
   
    def setItemState(self, item):
        from Katana import UI4
        ScenegraphIconManager = UI4.Util.ScenegraphIconManager
        IconManager = UI4.Util.IconManager

        #item.setText(ImportomaticAPI.NAME_COLUMN, 'ScenegraphXml')
        item.setText(ImportomaticAPI.NAME_COLUMN, self.__node.getName())
        item.setIcon(ImportomaticAPI.NAME_COLUMN, IconManager.GetIcon(g_iconPath))
        
    def getAssetId(self):
        assetId = self.__sgNode.getParameterValue('asset', 0)
        return assetId
    
    def setAssetId(self, assetId):
        assetParam = self.__sgNode.getParameter('asset')
        assetParam.setValue(assetId, 0)
                

    def getChildren(self):
        children = []
                
        if self.__assetInfoParam is not None:
            if self.__assetInfoParam.getNumChildren() > 0:
                for p in self.__assetInfoParam.getChildren():
                    if p.getName() == 'looks':
                        for look in p.getChildren():
                            children.append(LookFileTreeHandler(self.__node, look))
                    elif p.getName() == 'modifiers':
                        children.append(ModifierRootHandler(self.__node, p, 'assetInfo'))
                    elif p.getName()[0:1] == '_': # Ignore all hidden parameters
                        pass
                    else:
                        children.append(AssetTreeHandler(self.__node, self.__sgNode, p))

        return children
    
    def getItemKey(self):
        return self.__node

    def getEditor(self, widgetParent):
        handler = ImportomaticAPI.AssetModule.GetHandlerForType(self.__node)
        return handler.getEditor(self.__node.getChildByIndex(0), widgetParent) # The ScenegraphXml_In is always the first child node

    def isDeletable(self):
        return True
    
    def isSelectable(self):
        return True

    def delete(self):
        self.__node.delete()

    def isIgnorable(self):
        return True
    
    def isIgnored(self):
        return self.__node.isBypassed()

    def setIgnored(self, state):
        self.__node.setBypassed(state)

    def addNodeObservers(self, callback):
        callback(self.__node)

    def addToContextMenu(self, menu, importomaticNode):
        global AddLookFileAction
        global AddAttributeFileAction
        
        menu.addSeparator()
        menu.addAction(AddLookFileAction(menu, 'assetInfo', self))
        menu.addAction(AddAttributeFileAction(menu, 'assetInfo', self))

    def appendLookFile(self, lookFileName, assetName):
        celValue = self.__assetInfoParam.getChild("_sgPath").getValue(0)
        addLookFileParams(self.__assetInfoParam, lookFileName, celValue)

        BuildScenegraph(self.__node)

    def appendAttributeFile(self, attributeRef, assetName):
        celValue = self.__assetInfoParam.getChild("_sgPath").getValue(0)
        addAttributeFileParams(self.__assetInfoParam, attributeRef, celValue)            

        BuildScenegraph(self.__node)
        
    def getDefaultOpenState(self):
        return False

class ScenegraphXmlTreeHandlerBase(ImportomaticAPI.AssetTreeChild):

    def isSelectable(self):
        return True
    
    def getDefaultOpenState(self):
        return False


class AssetTreeHandler(ScenegraphXmlTreeHandlerBase):
    
    def __init__(self, node, xmlInNode, param, fullRef=""):
        self.__node = node
        self.__xmlInNode = xmlInNode
        self.__param = param
        self.__fullRef = fullRef + "." + param.getName()

    def setItemState(self, item):
        from Katana import UI4
        ScenegraphIconManager = UI4.Util.ScenegraphIconManager
        IconManager = UI4.Util.IconManager

        icon = g_iconBasePath + "/assembly16.png"
        item.setText(ImportomaticAPI.NAME_COLUMN, self.__param.getName())
        item.setIcon(ImportomaticAPI.NAME_COLUMN, IconManager.GetIcon(icon))
        item.setText(ImportomaticAPI.TYPE_COLUMN, 'assembly')

    def getItemKey(self):
        return str(self.__xmlInNode) + self.__fullRef

    def getChildren(self):
        children = []

        if self.__param.getNumChildren() > 0:
            for p in self.__param.getChildren():
                if p.getName() == 'looks':
                    for look in p.getChildren():
                        children.append(LookFileTreeHandler(self.__node, look))
                elif p.getName() == 'modifiers':
                    children.append(ModifierRootHandler(self.__node, p, self.__fullRef))
                elif p.getName()[0:1] == '_': # Ignore all hidden parameters
                    pass
                else:
                    children.append(AssetTreeHandler(self.__node, self.__xmlInNode, p, self.__fullRef))

        return children

    def addToContextMenu(self, menu, importomaticNode):
        global AddLookFileAction
        global AddAttributeFileAction
        menu.addSeparator()
        menu.addAction(AddLookFileAction(menu, self.__param.getName(), self))
        menu.addAction(AddAttributeFileAction(menu, self.__param.getName(), self))

    def appendLookFile(self, lookFileName, assetName):
        assetParam = self.__node.getParameter('assetInfo' + self.__fullRef)
        celValue = assetParam.getChild("_sgPath").getValue(0) # Abstract level to allow CEL modifications
        
        if assetParam is not None:
            addLookFileParams(assetParam, lookFileName, celValue)

        BuildScenegraph(self.__node)

    def appendAttributeFile(self, attributeRef, assetName):
        assetParam = self.__node.getParameter('assetInfo' + self.__fullRef)
        celValue = assetParam.getChild("_sgPath").getValue(0) # Abstract level to allow CEL modifications
 
        if assetParam is not None:
            addAttributeFileParams(assetParam, attributeRef, celValue)            

        BuildScenegraph(self.__node)

    def isIgnorable(self):
        return True
    
    def isIgnored(self):
        return self.__param.getChild('_ignore').getValue(0) == "True"

    def setIgnored(self, state):
        if state:
            self.__param.getChild('_ignore').setValue("True", 0)
        else:
            self.__param.getChild('_ignore').setValue("False", 0)

        BuildScenegraph(self.__node)


class ModifierRootHandler(ScenegraphXmlTreeHandlerBase):

    def __init__(self, node, param, parent):
        self.__node = node
        self.__param = param
        self.__parent = parent

    def setItemState(self, item):        
        from Katana import UI4
        ScenegraphIconManager = UI4.Util.ScenegraphIconManager
        item.setText(ImportomaticAPI.NAME_COLUMN, 'modifiers')
        item.setText(ImportomaticAPI.TYPE_COLUMN, '')

    def getChildren(self):
        children = []
        
        for attr in self.__param.getChildren():
            children.append(AttributeFileTreeHandler(self.__node, attr, self.__parent))        
        
        return children


class LookFileTreeHandler(ScenegraphXmlTreeHandlerBase):
    
    def __init__(self, node, lookParam):
        self.__node = node
        self.__lookParam = lookParam
        self.__lookName = lookParam.getChild('name').getValue(0)
        self.__hash = lookParam.getChild('_hash').getValue(0)
        self.__allowDelete = self.__lookParam.getChild('_allowDelete').getValue(0)

        # Networks created with the old Importomatic node might not have the
        # _createNode child attribute, so check it
        self.__createNode = 'True'
        if lookParam.getChild('_createNode'):
            self.__createNode = lookParam.getChild('_createNode').getValue(0)

    def setItemState(self, item):
        from Katana import UI4
        item.setText(ImportomaticAPI.NAME_COLUMN, self.__lookName)
        if self.__allowDelete == "True":
            item.setText(ImportomaticAPI.TYPE_COLUMN, 'look file override')
        else:
            item.setText(ImportomaticAPI.TYPE_COLUMN, 'look file')
        item.setText(ImportomaticAPI.STATUS_COLUMN, '')    
            
    def getItemKey(self):
        return self.__hash

    def getEditor(self, widgetParent):
        handler = ImportomaticAPI.AssetModule.GetHandlerForType(self.__node)

        # Don't try to get node editor if look file didn't create a node
        if self.__createNode == 'False':
            return None

        lookFileAssignNode = findNodeTypeByHash(self.__node, self.__hash, "LookFileAssign")
        if lookFileAssignNode is None:
            return None

        editor = handler.getEditor(lookFileAssignNode, widgetParent)
        return editor

    def isDeletable(self):
        return self.__allowDelete == "True"

    def delete(self):
        parent = self.__lookParam.getParent()
        parent.deleteChild(self.__lookParam)

        BuildScenegraph(self.__node)

    def isIgnorable(self):
        # Bypassing assignment coming directly from SGXML_In is currently not
        # supported 
        return (self.__createNode == 'True')
    
    def isIgnored(self):
        return self.__lookParam.getChild('_ignore').getValue(0) == "True"

    def setIgnored(self, state):
        if state:
            self.__lookParam.getChild('_ignore').setValue("True", 0)
        else:
            self.__lookParam.getChild('_ignore').setValue("False", 0)

        BuildScenegraph(self.__node)


class AttributeFileTreeHandler(ScenegraphXmlTreeHandlerBase):
    
    def __init__(self, node, attrParam, parent):
        self.__node = node
        self.__attrParam = attrParam
        self.__parent = parent
        self.__attrName = attrParam.getChild('name').getValue(0)
        self.__hash = attrParam.getChild('_hash').getValue(0)
        self.__allowDelete = self.__attrParam.getChild('_allowDelete').getValue(0)

        # Networks created with the old Importomatic node might not have the
        # _createNode child attribute, so check it
        self.__createNode = 'True'
        if attrParam.getChild('_createNode'):
            self.__createNode = attrParam.getChild('_createNode').getValue(0)

    def setItemState(self, item):
        from Katana import UI4
        ScenegraphIconManager = UI4.Util.ScenegraphIconManager
        IconManager = UI4.Util.IconManager

        item.setText(ImportomaticAPI.NAME_COLUMN, self.__attrName)
        if self.__allowDelete == "True":    
            item.setText(ImportomaticAPI.TYPE_COLUMN, 'attribute file override')
        else:
            item.setText(ImportomaticAPI.TYPE_COLUMN, 'attribute file')
        item.setText(ImportomaticAPI.STATUS_COLUMN, '')    
            
    def getItemKey(self):
        return self.__hash

    def getEditor(self, widgetParent):
        handler = ImportomaticAPI.AssetModule.GetHandlerForType(self.__node)

        # Don't try to get node editor if look file didn't create a node
        if self.__createNode == 'False':
            return None

        attrFileInNode = findNodeTypeByHash(self.__node, self.__hash, "AttributeFile_In")
        if attrFileInNode is None:
            return None        

        editor = handler.getEditor(attrFileInNode, widgetParent)
        return editor

    def isDeletable(self):
        return self.__allowDelete == "True"

    def delete(self):
        modifiersParam = self.__attrParam.getParent()
        modifiersParam.deleteChild(self.__attrParam)
        if modifiersParam.getNumChildren() == 0:
            modifiersParam.getParent().deleteChild(modifiersParam)

        BuildScenegraph(self.__node)

    def isIgnorable(self):
        # Bypassing assignment coming directly from SGXML_In is currently not
        # supported 
        return (self.__createNode == 'True')
    
    def isIgnored(self):
        return self.__attrParam.getChild('_ignore').getValue(0) == "True"

    def setIgnored(self, state):
        if state:
            self.__attrParam.getChild('_ignore').setValue("True", 0)
        else:
            self.__attrParam.getChild('_ignore').setValue("False", 0)

        BuildScenegraph(self.__node)
