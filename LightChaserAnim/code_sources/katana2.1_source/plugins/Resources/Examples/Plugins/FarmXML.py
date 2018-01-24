# Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import FarmAPI, NodegraphAPI, Callbacks
import xml.etree.ElementTree as ET
import os

# ---- SETUP ------------------------------------------------------------------

_GenerateXMLForCurrentNodeLabel = "Generate Farm XML File for Current Node..."
_GenerateXMLForSelectedNodeLabel = ("Generate Farm XML File for Selected "
                                    "Nodes...")
_GenerateXMLForAllNodesLabel = "Generate Farm XML File for All Nodes..."

def onStartup(**kwargs):
    # Add menu options
    FarmAPI.AddFarmMenuOption(_GenerateXMLForSelectedNodeLabel,
                              _InitDialogSelected)
    FarmAPI.AddFarmMenuOption(_GenerateXMLForAllNodesLabel, _InitDialogAll)

    # Add popup menu options
    FarmAPI.AddFarmPopupMenuOption(_GenerateXMLForCurrentNodeLabel,
                                   _InitDialogCurrent)
    FarmAPI.AddFarmPopupMenuOption(_GenerateXMLForSelectedNodeLabel,
                                   _InitDialogSelected)
    FarmAPI.AddFarmPopupMenuOption(_GenerateXMLForAllNodesLabel,
                                   _InitDialogAll)

    # Add custom farm settings
    FarmAPI.AddFarmSettingNumber("threadable", 1, hints={'widget': 'checkBox',
                                                         'constant': 'True'})
    FarmAPI.AddFarmSettingString("farmFileName")

Callbacks.addCallback(Callbacks.Type.onStartup, onStartup)


# -----------------------------------------------------------------------------

def _GetExistingFileLocation():
    primaryNode = FarmAPI.GetCurrentNode()
    farmFileNameParam = primaryNode.getParameter('farmSettings.farmFileName')
    if farmFileNameParam:
        return farmFileNameParam.getValue(0)
    return ''

def _InitDialog():
    _OpenDialog(fileLocation=_GetExistingFileLocation())

def _InitDialogCurrent():
    _OpenDialog(FarmAPI.NODES_SINGLE, _GetExistingFileLocation())

def _InitDialogSelected():
    _OpenDialog(FarmAPI.NODES_SELECTED)

def _InitDialogAll():
    _OpenDialog(FarmAPI.NODES_ALL)

def _OpenDialog(nodeScope=None, fileLocation=None):
    from Katana import UI4
    import AssetAPI

    enabled = FarmAPI.IsSceneValid(nodeScope=nodeScope, unwantedSymbols=['_'])
    errorText = FarmAPI.GetErrorMessages()
    if len(errorText) > 0:
        UI4.Widgets.MessageBox.Warning('Error', ' '.join(errorText))
        return

    if fileLocation is None:
        fileLocation = ''

    xmlFile = UI4.Util.AssetId.BrowseForAsset(
        fileLocation, 'Specify a filename', True,
        {'fileTypes': 'xml',
         'acceptDir': False,
         'context': AssetAPI.kAssetContextFarm})

    if not xmlFile:
        return

    nodeList = FarmAPI.GetNodeList()
    if not nodeList:
        UI4.Widgets.MessageBox.Warning('Error', 'No nodes found!')

    _GenerateFarmFile(nodeList, xmlFile)

# -----------------------------------------------------------------------------
def _GenerateFarmFile(nodeList, fileName):
    sortedDependencies = FarmAPI.GetSortedDependencies(nodeList)
    katanaFile = FarmAPI.GetKatanaFileName()

    # Generate XML
    _GenerateXML(fileName, sortedDependencies, katanaFile)

# ---- PARSE AND GENERATE XML -------------------------------------------------

def _GenerateXML(fileName, sortedDependencies, katanaFile):
    """
    Generates XML from the given node dependencies in the given Katana file,
    and writes the XML out to the given file name.

    @type fileName: C{str}
    @type sortedDependencies: C{list} of C{FarmAPI.NodeRenderSettings}
    @type katanaFile: C{str}
    @param fileName: A path to the XML file which will be written.
    @param sortedDependencies: A list of C{FarmAPI.NodeRenderSettings}
        instances describing nodes to be rendered and their dependencies.
    @param katanaFile: The path to the Katana file from which the nodes are
        being rendered. This path is stored in the generated XML.
    """
    xmlRoot = ET.Element('farm')

    layers = dict()
    dependAll = set()

    for nodeRenderSettings in sortedDependencies:
        # Get the layer name
        nodeName = nodeRenderSettings.nodeName
        if nodeRenderSettings.renderType:
            layerName = "%s_%s" % (nodeName, nodeRenderSettings.renderType)
        else:
            layerName = nodeName

        layers[nodeName] = layerName

        xmlLayer = ET.SubElement(xmlRoot, 'layer')
        xmlLayer.attrib['enable'] = "true"
        xmlLayer.attrib['name'] = layerName

        xmlNode = ET.SubElement(xmlLayer, 'katanaFile')
        xmlNode.text = katanaFile
        xmlNode = ET.SubElement(xmlLayer, 'node')
        xmlNode.text = nodeName
        xmlNode = ET.SubElement(xmlLayer, 'renderInternalDeps')
        xmlNode.text = ("Yes"
                        if nodeRenderSettings.renderInternalDependencies
                        else "No")

        # Custom setting - 'threadable'
        xmlNode = ET.SubElement(xmlLayer, 'threadable')
        xmlNode.text = str(
            nodeRenderSettings.customFarmSettings.get('threadable', False))

        xmlNode = ET.SubElement(xmlLayer, 'service')
        xmlNode.text = nodeRenderSettings.renderService

        nodeType = nodeRenderSettings.getNodeType()
        ET.SubElement(xmlLayer, 'nodeType').text = nodeType

        if nodeType == 'RenderScript':
            renderScriptSettings = nodeRenderSettings.renderScriptSettings
            if renderScriptSettings:
                renderScriptSettings.appendXML(xmlLayer)

        # The render pass information
        outputPasses = nodeRenderSettings.outputs
        if outputPasses:
            xmlNodeOutputPasses = ET.SubElement(xmlLayer, 'outputs')
            for outputPass in outputPasses:
                xmlNodeOutputPass = ET.SubElement(xmlNodeOutputPasses,
                                                  'output')

                xmlNodeOutputPass.attrib["enabled"] = ('true'
                                                       if outputPass['enabled']
                                                       else 'false')
                xmlNodeOutputPass.attrib["name"] = outputPass['name']

                # Temp render location
                xmlNodeOutputPass_tempRenderLocation = ET.SubElement(
                    xmlNodeOutputPass, 'tempRenderLocation')
                xmlNodeOutputPass_tempRenderLocation.text = outputPass[
                    "tempRenderLocation"]

                # Output location
                xmlNodeOutputPass_outputLocation = ET.SubElement(
                    xmlNodeOutputPass, 'outputLocation')
                xmlNodeOutputPass_outputLocation.text = outputPass[
                    'outputLocation']

        outputRange = nodeRenderSettings.frameRange
        if outputRange:
            xmlRange = ET.SubElement(xmlLayer, 'range')
            xmlStart = ET.SubElement(xmlRange, 'start')
            xmlStart.text = str(int(outputRange[0]))
            xmlEnd = ET.SubElement(xmlRange, 'end')
            xmlEnd.text = str(int(outputRange[1]))

        deps = nodeRenderSettings.dependencies
        if deps:
            xmlDepsRoot = ET.SubElement(xmlLayer, 'dependencies')

            for dep in deps:
                xmlDepsNode = ET.SubElement(xmlDepsRoot, 'dependency')
                xmlDepsNode.text = layers[dep]
                xmlDepsNode.attrib['type'] = ('LayerOnLayer'
                                              if dep in dependAll
                                              else 'FrameByFrame')

        if nodeRenderSettings.dependAll:
            dependAll.add(nodeName)

    fileBase = os.path.basename(fileName)
    filePath = os.path.dirname(fileName)

    if not os.path.exists(filePath):
        os.makedirs(filePath)

    parts = os.path.splitext(fileBase)
    if len(parts) == 1 or (len(parts) == 2 and parts[1] != '.xml'):
        fileName = os.path.join(filePath, fileBase + '.xml')

    xmlTree = ET.ElementTree(xmlRoot)
    xmlTree.write(fileName)
