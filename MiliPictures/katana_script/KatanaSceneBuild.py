# Copyright (c) 2016 Mili Pictures. All Rights Reserved.

#coding:utf-8
__author__ = "Kevin Tsui"

import json,types,time

try:
    from Katana import NodegraphAPI
    from UI4.MainUI import QtCore,QtGui
except:
    pass

class AlembicInBase(object):
    def __init__(self,InMatJsonFile,InShadingJsonFile):
        #InMatJsonFile = "//hnas01/data/Projects/DragonNest2_test/Materials/Characters/NGJL/default/c_NGJL_mat.matJson"
        #InShadingJsonFile = "//hnas01/data/Projects/DragonNest2_test/Materials/Characters/NGJL/default/c_NGJL_mat.ShadingJson"
        self.AlembicData = []
        self.MaterialData = {}
        self.headAlebic = "/root/world/geo/asset"
        self.headMaterial = "/root/materials/"
        self.shadingJsonData = self.readJsonDocument(InShadingJsonFile)
        self.standardNode = None
        matJsonData = self.readJsonDocument(InMatJsonFile)
        for material in matJsonData:
            materialName =self.toKMate(material) + "_NetworkMaterial"
            self.MaterialData[materialName] = []
            alembicList = matJsonData[material].keys()
            for alembic in alembicList:
                alembicFullName = self.toKAbc(alembic)
                alembicName = alembicFullName.split("/")[-1]
                facesetValue = matJsonData[material][alembic].replace(":","-")
                firstFaceID = facesetValue.split(",")[0].split("-")[0]
                facesetName = alembicName+"_"+firstFaceID
                tempList = [alembicName,alembicFullName,facesetValue,materialName,facesetName]
                self.AlembicData.append(tempList)
                if facesetValue == str(-1):
                    self.MaterialData[materialName].append(alembicFullName)
                else:
                    self.MaterialData[materialName].append(alembicFullName+"/"+facesetName)
    def toKMate(self,InMayaMaterial):
        SGtoMaterial =  self.shadingJsonData["SGs"]
        Result = SGtoMaterial[InMayaMaterial]["_DefaultMat"]
        #Result = str(InMayaMaterial)[:-2]
        return Result
    def toKAbc(self,InMayaAlembic):
        Result = InMayaAlembic.replace("|","/")
        return Result
    def getOrig(self):
        return self.standardNode
    def readJsonDocument(self,InJsonFile=None):
        try:
            file = open(InJsonFile, 'rb')
            MaterialList = json.load(file)
            file.close()
        except IOError:
            MaterialList = {}
        return MaterialList
    def createAlembicNode(self):
        AlembicData = self.AlembicData
        MaterialData = self.MaterialData
        # Create Node
        modelName = "Model_In"
        nodeList = []
        standardNode = NodegraphAPI.CreateNode('Alembic_In', NodegraphAPI.GetRootNode())
        #standardNode.getParameter('name').setValue("/root/world/geo/%s"%modelName, 0)
        #standardNode.setName(modelName)
        standardNode.getParameter('abcAsset').setValue("C:/Users/mili/Desktop/c_NGJL_mod.abc", 0)
        NodegraphAPI.SetNodePosition(standardNode, (0, 0))
        nodeList.append(standardNode)

        pruneNode = NodegraphAPI.CreateNode('Prune', NodegraphAPI.GetRootNode())
        NodegraphAPI.SetNodePosition(standardNode, (0, 50))
        pruneNode.getInputPort('A').connect(standardNode.getOutputPort('out'))
        nodeList.append(pruneNode)

        facesetGroupNode = NodegraphAPI.CreateNode('GroupStack', NodegraphAPI.GetRootNode())
        facesetGroupNode.setName("FaceSetGroup")
        facesetGroupNode.setChildNodeType('FaceSetCreate')
        facesetGroupNode.getInputPort('in').connect(pruneNode.getOutputPort('out'))
        NodegraphAPI.SetNodePosition(facesetGroupNode, (0, -50))
        nodeList.append(facesetGroupNode)

        materialassignGroupNode = NodegraphAPI.CreateNode('GroupStack', NodegraphAPI.GetRootNode())
        materialassignGroupNode.setName("MaterialAssignGroup")
        materialassignGroupNode.setChildNodeType('MaterialAssign')
        materialassignGroupNode.getInputPort('in').connect(facesetGroupNode.getOutputPort('out'))
        NodegraphAPI.SetNodePosition(materialassignGroupNode, (0, -100))
        nodeList.append(materialassignGroupNode)

        nodegraphTab = UI4.App.Tabs.FindTopTab('Node Graph')

        for data in AlembicData:
            self.createFaceSet(facesetGroupNode,data)
        for material in MaterialData:
            data = (material,MaterialData[material])
            self.createMatrialAssign(materialassignGroupNode,data)
        pruneNode = NodegraphAPI.GetNode("Prune")
        pruneList = [
            '/root/world/geo/%s/front'%modelName,
            '/root/world/geo/%s/persp'%modelName,
            '/root/world/geo/%s/side'%modelName,
            '/root/world/geo/%s/top'%modelName,
        ]
        pruneNode.getParameter('cel').setValue('(' + ' '.join(pruneList) + ')', 0)

        if nodegraphTab:
            nodegraphTab.floatNodes(nodeList)
    def createFaceSet(self,InGroupStackNode,InData):
        Triger = InData[2]
        if Triger == str(-1):
            return
        MeshLocation = self.headAlebic + InData[1]
        Selection = InData[2]
        #FaceSetName = InData[0] + "_FaceSet"
        facesetNode = NodegraphAPI.CreateNode('FaceSetCreate', NodegraphAPI.GetRootNode())
        #facesetNode.setName(FaceSetName)
        facesetNode.getParameter("meshLocation").setValue(str(MeshLocation), 0)
        facesetNode.getParameter("faceSetName").setValue(str(InData[4]), 0)
        facesetNode.getParameter("selection").setValue(str(Selection), 0)
        InGroupStackNode.buildChildNode(facesetNode)
    def createMatrialAssign(self,InGroupStackNode,InData):
        MaterialAssign = self.headMaterial + InData[0]
        alembicList = []
        for alembic in InData[1]:
            alembicName = self.headAlebic + alembic
            alembicList.append(alembicName)
        materialassignNode = NodegraphAPI.CreateNode('MaterialAssign', NodegraphAPI.GetRootNode())
        materialassignNode.getParameter('CEL').setValue("("+str(' '.join(alembicList))+")", 0)
        materialassignNode.getParameter('args.materialAssign.enable').setValue(1, 0)
        materialassignNode.getParameter("args.materialAssign.value").setValue(str(MaterialAssign), 0)
        InGroupStackNode.buildChildNode(materialassignNode)
    def build(self,InPos):
        offsetX = InPos[0] - 200
        offsetY = InPos[1]
        AlembicData = self.AlembicData
        MaterialData = self.MaterialData
        #~ Create Node
        modelName = "asset"
        nodeList = []
        self.standardNode = NodegraphAPI.CreateNode('Alembic_In', NodegraphAPI.GetRootNode())
        self.standardNode.getParameter('name').setValue("/root/world/geo/%s"%modelName, 0)
        self.standardNode.setName("Asset")
        self.standardNode.getParameter('abcAsset').setValue("C:/Users/mili/Desktop/c_NGJL_mod.abc", 0)
        NodegraphAPI.SetNodePosition(self.standardNode, (0+offsetX, 150+offsetY))
        nodeList.append(self.standardNode)

        pruneNode = NodegraphAPI.CreateNode('Prune', NodegraphAPI.GetRootNode())
        NodegraphAPI.SetNodePosition(pruneNode, (0+offsetX, 100+offsetY))
        pruneNode.getInputPort('A').connect(self.standardNode.getOutputPort('out'))
        nodeList.append(pruneNode)
        pruneList = [
            '/root/world/geo/%s/front'%modelName,
            '/root/world/geo/%s/persp'%modelName,
            '/root/world/geo/%s/side'%modelName,
            '/root/world/geo/%s/top'%modelName,
        ]
        pruneNode.getParameter('cel').setValue('(' + ' '.join(pruneList) + ')', 0)

        facesetGroupNode = NodegraphAPI.CreateNode('GroupStack', NodegraphAPI.GetRootNode())
        facesetGroupNode.setName("FaceSetGroup")
        facesetGroupNode.setChildNodeType('FaceSetCreate')
        facesetGroupNode.getInputPort('in').connect(pruneNode.getOutputPort('out'))
        NodegraphAPI.SetNodePosition(facesetGroupNode, (0+offsetX, 50+offsetY))
        nodeList.append(facesetGroupNode)

        materialassignGroupNode = NodegraphAPI.CreateNode('GroupStack', NodegraphAPI.GetRootNode())
        materialassignGroupNode.setName("MaterialAssignGroup")
        materialassignGroupNode.setChildNodeType('MaterialAssign')
        materialassignGroupNode.getInputPort('in').connect(facesetGroupNode.getOutputPort('out'))
        NodegraphAPI.SetNodePosition(materialassignGroupNode, (0+offsetX, offsetY))
        nodeList.append(materialassignGroupNode)

        for data in AlembicData:
            self.createFaceSet(facesetGroupNode,data)
        for material in MaterialData:
            data = (material,MaterialData[material])
            self.createMatrialAssign(materialassignGroupNode,data)
        Result = materialassignGroupNode
        return Result

class ShadingNetworkBase(object):
    def __init__(self,InShadingJsonFile):
        #InShadingJsonFile = "//hnas01/data/Projects/DragonNest2_test/Materials/Characters/NGJL/default/c_NGJL_mat.ShadingJson"
        InKey = "ShadingNodes"
        self.shadingNodeMap = self.readMayaJsonDocument(InShadingJsonFile,InKey)

        self.SpecialNodeNameMap = \
            {
                "Maya:place2dTexture":"MayaPlace2DTexture",
                "offset":"offsetUV",
                "Maya:alSurfaceX":"alSurface",
                "Maya:bump2d":"mayaBump2D",
                "Maya:alRemapColor":"alRemapColor",
                "Maya:displacementShader":"MayaNormalDisplacement",
                "Maya:lambert":"lambert"
            }
        self.SpecialPortNameMap = \
            {
                "MayaFile":
                    {
                        "fileTextureName":"filename",
                    },
                "MayaPlace2DTexture":
                    {
                        "outUV":"out",
                    },
                "MayaRemapHsv":
                    {
                        "color":"input",
                    },
                "mayaBump2D":
                    {
                        "bumpValue":"bump_map",
                        "outNormal":"out",
                        "normalCamera":"shader",
                        "bumpDepth":"bump_height",
                    },
                "alRemapColor":
                    {
                    }
            }
        #~ Be careful,the key in SpecialPortValueMap was a Katana key name
        self.SpecialPortValueMap = \
            {
                "MayaNormalDisplacement":
                    {
                        "vectorSpace":{0:"world",1:"object",2:"tangent"},
                        "vectorEncoding":{0:"absolute",1:"signed"},
                    }
            }
        self.shadingNodeTypeMap = {}
        self.connectNetworkMaterialMap = {}
        self.suffixGroup = "Group"
        self.suffixNetworkMaterial = "NetworkMaterial"
        self.pos = 0
    def translateMayaType(self,InText):
        SpecialNodeNameMap = self.SpecialNodeNameMap
        NoTypeTagList = ["MayaBlinn","MayaPhong"]
        if InText in SpecialNodeNameMap.keys():
            OutText = SpecialNodeNameMap[InText]
        elif len(InText.split(":")) > 1:
            splitTemp = InText.split(":")
            tempText = splitTemp[0]+splitTemp[1].replace(splitTemp[1][0],splitTemp[1][0].upper())
            if tempText in NoTypeTagList:
                OutText = "alSurface"
            else:
                OutText = tempText
        else:
            OutText = InText
        return OutText
    def translateMayaAttr(self,InType,InName):
        constantMap = \
            {
                "outColor":"out",
                "outColorR":"out.r",
                "outColorG":"out.g",
                "outColorB":"out.b",
                "outAlpha":"out.a",
            }
        if InName in constantMap:
            result = constantMap[InName]
            return result
        SpecialPortNameMap = self.SpecialPortNameMap
        if InType in SpecialPortNameMap.keys():
            if InName in SpecialPortNameMap[InType].keys():
                result = SpecialPortNameMap[InType][InName]
                return result
            else:
                result = InName
                return result
        else:
            result = InName
            return result
    def translateMayaValue(self,InType,InName,InValue):
        SpecialPortValueMap = self.SpecialPortValueMap
        if InType in SpecialPortValueMap.keys():
            if InName in SpecialPortValueMap[InType].keys():
                result = SpecialPortValueMap[InType][InName][InValue]
                return result
            else:
                result = InValue
                return result
        else:
            result = InValue
            return result
    def readMayaJsonDocument(self,InJsonFile=None,InKey=None):
        try:
            file = open(InJsonFile, 'rb')
            activeMap = json.load(file)
            file.close()
            MaterialList = activeMap[InKey]
        except IOError:
            MaterialList = {}
        return MaterialList
    def setParameter(self,InNode,InName,InParaList):
            #~ Set Node Attribute
            for i in range(0,len(InParaList)):
                try:
                    if type(InParaList[i][1]) is bool:
                        if InParaList[i][1]:
                            value = 1
                        else:
                            value = 0
                        InNode.getParameter('parameters.%s.enable'%InParaList[i][0]).setValue(1, 0)
                        InNode.getParameter('parameters.%s.value'%InParaList[i][0]).setValue(value, 0)
                    elif type(InParaList[i][1]) is list:
                        for j in range(0,len(InParaList[i][1])):
                            InNode.getParameter('parameters.%s.enable'%InParaList[i][0]).setValue(1, 0)
                            InNode.getParameter('parameters.%s.value.i%s'%(InParaList[i][0],j)).setValue(InParaList[i][1][j], 0)
                    elif type(InParaList[i][1]) is dict:
                        temp = [InParaList[i][1]["connection"],"%s.%s"%(InName,InParaList[i][0])]
                    elif type(InParaList[i][1]) is str or unicode:
                        InNode.getParameter('parameters.%s.enable'%InParaList[i][0]).setValue(1, 0)
                        InNode.getParameter('parameters.%s.value'%InParaList[i][0]).setValue(str(InParaList[i][1]), 0)
                    else:
                        temp = self.translateMayaValue()
                        InNode.getParameter('parameters.%s.enable'%InParaList[i][0]).setValue(1, 0)
                        InNode.getParameter('parameters.%s.value'%InParaList[i][0]).setValue(InParaList[i][1], 0)
                except AttributeError:
                    pass
    def getParameterList(self,InName):
        shadingNodeMap = self.shadingNodeMap
        i = 0
        paraList = []
        for shadingNode in shadingNodeMap:
            if shadingNode == InName:
                nodeType = self.translateMayaType(shadingNodeMap[shadingNode]["__NodeType"])
                for paraName in shadingNodeMap[shadingNode]:
                    tempName = self.translateMayaAttr(nodeType,paraName)
                    tempValue = self.translateMayaValue(nodeType,tempName,shadingNodeMap[shadingNode][paraName])
                    temp = [tempName,tempValue]
                    paraList.append(temp)
                break
        result = paraList
        return result
    def depthDigger(self):
        shadingNodeMap = self.shadingNodeMap

        ###
        #~ Set connectMessageList and topNodeList
        connectMessageList = {} #~ get upstream node message
        setConnectionList = {} #~ get downstream node message
        topNodeList = [] #~ node on top
        i = 0
        for shadingNode in shadingNodeMap:
            i += 1
            currentNode = shadingNode
            isTopNode = True
            for paraName in shadingNodeMap[shadingNode]:
                paraValue = [self.translateMayaType(paraName),shadingNodeMap[shadingNode][paraName]]
                #print paraValue
                if type(paraValue[1]) is dict:
                    upstreamNode = paraValue[1]["connection"].split(".")[0]
                    connectMessageList[upstreamNode] = currentNode
                    if not setConnectionList.has_key(currentNode):
                        setConnectionList[currentNode] = [upstreamNode]
                    else:
                        if upstreamNode not in setConnectionList[currentNode]:
                            setConnectionList[currentNode].append(upstreamNode)
                    isTopNode = False
                else:
                    continue
            if isTopNode:
                setConnectionList[currentNode] = None
                topNodeList.append(currentNode)

        ###
        #~ Set baseNodeList
        baseNodeList = [] #~ lowest node
        for topNode in topNodeList:
            #~ Find all node in same material
            Flag = False
            jumpNode = topNode
            #print topNode
            while Flag == False:
                if jumpNode in connectMessageList.keys():
                    nextNode = connectMessageList[jumpNode]
                    jumpNode = nextNode
                    #print FirstNode
                else:
                    #print "Last one:",FirstNode
                    if jumpNode not in baseNodeList:
                        baseNodeList.append(jumpNode)
                    Flag = True

        ###
        #~ Set levelNodeMap
        levelNodeMap = {}
        for baseNode in baseNodeList:
            i = 1
            #print "Level%s :"%i,baseNode
            if not levelNodeMap.has_key(i):
                levelNodeMap[i] = [baseNode]
            else:
                if baseNode not in levelNodeMap[i]:
                    levelNodeMap[i].append(baseNode)
        i = 1
        Flag = True
        while Flag:
            for jumpNode in levelNodeMap[i]:
                if type(setConnectionList[jumpNode]) is list:
                    for nextNode in setConnectionList[jumpNode]:
                        #~ Add all list element into levelNodeMap
                        if not levelNodeMap.has_key((i + 1)):
                            #~ Create new key
                            levelNodeMap[(i + 1)] = [nextNode]
                        else:
                            if nextNode not in levelNodeMap[(i + 1)]:
                                levelNodeMap[(i + 1)].append(nextNode)
            #~ Is condition available
            if not levelNodeMap.has_key((i + 1)):
                Flag = False
            i += 1
        ###
        #~ Set materialNodeList,this list put the nodes in same material into one list
        materialNodeMap = {}
        for baseNode in baseNodeList:
            materialNodeMap[baseNode] = [baseNode]
            jumpList = setConnectionList[baseNode]
            Flag = True
            nextList = []
            if type(jumpList) is not list:
                continue
            else:
                materialNodeMap[baseNode] += jumpList
            while Flag:
                for jumpNode in jumpList:
                    tempList = setConnectionList[jumpNode]
                    if type(tempList) is list:
                        nextList += tempList
                jumpList = nextList
                #print "nextList:",nextList
                nextList = []
                if jumpList == []:
                    Flag = False
                else:
                    materialNodeMap[baseNode] += jumpList

        ###
        #~ Get attribute when create node
        nodeDataMap = {}
        for shadingNode in shadingNodeMap:
            i += 1
            nodeType = self.translateMayaType(shadingNodeMap[shadingNode]["__NodeType"])
            paraList = []
            for paraName in shadingNodeMap[shadingNode]:
                tempName = self.translateMayaAttr(nodeType,paraName)
                tempValue = self.translateMayaValue(nodeType,tempName,shadingNodeMap[shadingNode][paraName])
                temp = [tempName,tempValue]
                paraList.append(temp)
                #print temp
            print shadingNode,nodeType
            nodeDataMap[shadingNode] = [nodeType,paraList]

        return (materialNodeMap,levelNodeMap,nodeDataMap)
    def waitConnect(self):
        informationTuple = self.depthDigger()
        surfaceNodeList = informationTuple[0].keys()

        connectionList = []
        shadingNodeMap = self.shadingNodeMap
        for shadingNode in shadingNodeMap:
            nodeType = self.translateMayaType(shadingNodeMap[shadingNode]["__NodeType"])
            self.shadingNodeTypeMap[shadingNode] = nodeType
        """Get connectionList"""
        for shadingNode in shadingNodeMap:
            for attr in shadingNodeMap[shadingNode]:
                if type(shadingNodeMap[shadingNode][attr]) is dict:
                    attrPort = shadingNodeMap[shadingNode][attr]["connection"]
                    tempMap = {"downstream":[shadingNode,attr],"upstream":[attrPort.split(".")[0],attrPort.split(".")[-1]]}
                    connectionList.append(tempMap)
        """Connect Surface to NetworkMaterial"""
        for surface in surfaceNodeList:
            downstreamName = "%s_%s"%(surface,self.suffixNetworkMaterial)
            downstreamNode = NodegraphAPI.GetNode(str(downstreamName))
            upstreamName = surface
            upstreamNode = NodegraphAPI.GetNode(upstreamName)
            downstreamNode.getInputPort("arnoldSurface").connect(upstreamNode.getOutputPort("out"))
        """Connect attributes"""
        for connection in connectionList:
            downstream = connection["downstream"]
            upstream = connection["upstream"]
            downstreamNode = NodegraphAPI.GetNode(downstream[0])
            upstreamNode = NodegraphAPI.GetNode(upstream[0])
            input = self.translateMayaAttr(self.shadingNodeTypeMap[downstream[0]],downstream[1])
            output = self.translateMayaAttr(self.shadingNodeTypeMap[upstream[0]],upstream[1])
            "Connect normal map"
            if input == "normalCamera":
                materialName = "%s_%s"%(downstream[0],self.suffixNetworkMaterial)
                input = 'arnoldBump'
                downstreamNode = NodegraphAPI.GetNode(str(materialName))
            try:
                downstreamNode.getInputPort(input).connect(upstreamNode.getOutputPort(output))
            except AttributeError:
                #print input,output
                node = NodegraphAPI.GetNode(str(downstream[0]))
                paraList = self.getParameterList(upstream[0])
                name = downstream[0]
                self.setParameter(node,name,paraList)
                #print downstream,upstream
                #print "--------------"
        #print "############################"
        for connection in connectionList:
            downstream = connection["downstream"]
            attrName = self.translateMayaAttr(self.shadingNodeTypeMap[downstream[0]],downstream[1])
            try:
                activeNode = NodegraphAPI.GetNode(str(downstream[0]))
                activeNode.getParameter('parameters.%s.enable'%attrName).setValue(2, 0)
            except AttributeError:
                #print downstream[0],attrName
                pass
    def waitTillCompleteLoop(self):
        global ShadingNetworkBaseTimer
        isExsit = False
        informationTuple = self.depthDigger()
        surfaceNodeList = informationTuple[0].keys()
        floatNodeList = []
        for surfaceNode in surfaceNodeList:
            testNode1 = NodegraphAPI.GetNode(str("%s_%s"%(surfaceNode,self.suffixGroup)))
            testNode2 = NodegraphAPI.GetNode(str("%s_%s"%(surfaceNode,self.suffixNetworkMaterial)))
            floatNodeList.append("%s_%s"%(surfaceNode,self.suffixGroup))
            floatNodeList.append("%s_%s"%(surfaceNode,self.suffixNetworkMaterial))
            if testNode1 == None or testNode2 == None:
                isExsit = False
                break
            else:
                isExsit = True
                continue
        if isExsit == True:
            ShadingNetworkBaseTimer.stop()
            self.waitConnect()
    def createMergeNodes(self,InNodes,InParent):
        merge = NodegraphAPI.CreateNode('Merge')
        merge.setParent(InParent)

        for n in InNodes:
            output = n.getOutputPortByIndex(0)
            if not output:
                continue
            numInputs = merge.getNumInputPorts()
            inputPort = merge.addInputPort('i%d' % numInputs)
            output.connect(inputPort)

        return merge
    def createNodes(self,InName,InOffset,InType,InParaList):
        #~ Create Node
        standardNode = NodegraphAPI.CreateNode('ArnoldShadingNode', NodegraphAPI.GetRootNode())
        standardNode.getParameter('name').setValue(InName, 0)
        NodegraphAPI.SetNodePosition(standardNode, (0+InOffset[0], 0+InOffset[1]))
        standardNode.getParameter('nodeType').setValue(InType, 0)
        standardNode.checkDynamicParameters()
        #~ Set Node Attribute
        self.setParameter(standardNode,InName,InParaList)
        return standardNode
    def connectNodes(self):
        global ShadingNetworkBaseTimer
        ShadingNetworkBaseTimer= QtCore.QTimer()
        ShadingNetworkBaseTimer.timeout.connect(lambda *args: self.waitTillCompleteLoop())
        ShadingNetworkBaseTimer.start(100)
    def getPos(self):
        return self.pos
    def build(self):
        informationTuple = self.depthDigger()
        materialNodeMap = informationTuple[0]
        levelNodeMap = informationTuple[1]
        nodeDataMap = informationTuple[2]

        nodeMergeList = []
        """Reset node position"""
        i = 0
        for materialNodes in materialNodeMap:
            i += 1
            offsetX = (i-1)*500
            #~ Create a group to hold some nodes in single material
            groupNode = NodegraphAPI.CreateNode('Group', NodegraphAPI.GetRootNode())
            groupName = "%s_%s"%(materialNodes,self.suffixGroup)
            groupNode.setName(groupName)
            NodegraphAPI.SetNodePosition(groupNode, (0 + offsetX, 0))
            #~ Create a Network Material Node
            networkMaterialNode = NodegraphAPI.CreateNode('NetworkMaterial', NodegraphAPI.GetRootNode())
            NodegraphAPI.SetNodePosition(networkMaterialNode, (0 + offsetX, -250))
            materialName = "%s_%s"%(materialNodes,self.suffixNetworkMaterial)
            networkMaterialNode.getParameter('name').setValue(str(materialName), 0)
            networkMaterialNode.addInputPort('arnoldSurface')
            networkMaterialNode.addInputPort('arnoldBump')
            nodeMergeList.append(networkMaterialNode)
            tempLevelCount = {}
            for node in materialNodeMap[materialNodes]:
                for level in levelNodeMap:
                    if node in levelNodeMap[level]:
                        if tempLevelCount.has_key(level):
                            tempLevelCount[level] += 1
                        else:
                            tempLevelCount[level] = 1
            for node in materialNodeMap[materialNodes]:
                thisLevel = 1
                for level in levelNodeMap:
                    if node in levelNodeMap[level]:
                        thisLevel = level
                offsetY = (thisLevel - 1)*100
                name = str(node)
                offset = (offsetX,offsetY)
                type = str(nodeDataMap[name][0])
                paraList = nodeDataMap[name][1]
                """Create"""
                activeNode = self.createNodes(name,offset,type,paraList)
                activeNode.setParent(groupNode)
        upstreamMergeNode = self.createMergeNodes(nodeMergeList, NodegraphAPI.GetRootNode())
        NodegraphAPI.SetNodePosition(upstreamMergeNode,((len(nodeMergeList)/2)*500, -600))
        upstreamMergeNode.setName("MaterialsMerge")
        dowmstreamMergeNode = NodegraphAPI.CreateNode('Merge',NodegraphAPI.GetRootNode())
        dowmstreamMergeNode.addInputPort('i%d' % 0)
        dowmstreamMergeNode.getInputPort("i0").connect(upstreamMergeNode.getOutputPort("out"))
        NodegraphAPI.SetNodePosition(dowmstreamMergeNode,((len(nodeMergeList)/2)*500, -1000))
        dowmstreamMergeNode.setName("MergePass")
        self.pos = ((len(nodeMergeList)/2)*500, -1000)
        """Connect"""
        self.connectNodes()
        Result = dowmstreamMergeNode
        return Result

class KatanaSceneBuild():
    def __init__(self,InMatJsonFile,InShadingJsonFile):
        #MatJsonFile = "//hnas01/data/Projects/DragonNest2_test/Materials/Characters/NGJL/default/c_NGJL_mat.matJson"
        #ShadingJsonFile = "//hnas01/data/Projects/DragonNest2_test/Materials/Characters/NGJL/default/c_NGJL_mat.ShadingJson"

        self.ShadingNetworkBase = ShadingNetworkBase(InShadingJsonFile)
        self.AlembicInBase = AlembicInBase(InMatJsonFile,InShadingJsonFile)
    def create(self,InData):
        global KatanaSceneBuildTimer
        KatanaSceneBuildTimer= QtCore.QTimer()
        KatanaSceneBuildTimer.timeout.connect(lambda *args: self.waitCreateLoop(InData))
        KatanaSceneBuildTimer.start(100)
    def waitCreateLoop(self,InData):
        global KatanaSceneBuildTimer
        try:
            InData[0].getName()
            InData[1].getName()
        except AttributeError:
            pass
        else:
            KatanaSceneBuildTimer.stop()
            self.createRestParts(InData)
    def createRestParts(self,InData):
        root = (InData[2][0]-100, InData[2][1]-100)
        mergeNode = NodegraphAPI.CreateNode('Merge',NodegraphAPI.GetRootNode())
        NodegraphAPI.SetNodePosition(mergeNode, root)
        mergeNode.addInputPort('i%d' % 0)
        mergeNode.addInputPort('i%d' % 1)
        mergeNode.getInputPort("i0").connect(InData[0].getOutputPort("out"))
        mergeNode.getInputPort("i1").connect(InData[1].getOutputPort("out"))

        upstreamDot = NodegraphAPI.CreateNode('Dot',NodegraphAPI.GetRootNode())
        downstreamDot = NodegraphAPI.CreateNode('Dot',NodegraphAPI.GetRootNode())
        NodegraphAPI.SetNodePosition(downstreamDot, (root[0]-400,root[1]))
        NodegraphAPI.SetNodePosition(upstreamDot, (root[0]-400,root[1]+200))
        downstreamDot.getInputPort("input").connect(upstreamDot.getOutputPort("output"))
        origNode = InData[3]
        upstreamDot.getInputPort("input").connect(origNode.getOutputPort("out"))

        LookFileBakeNode = NodegraphAPI.CreateNode('LookFileBake',NodegraphAPI.GetRootNode())
        NodegraphAPI.SetNodePosition(LookFileBakeNode, (root[0]-200,root[1]-100))
        LookFileBakeNode.getInputPort("orig").connect(downstreamDot.getOutputPort("output"))
        LookFileBakeNode.getInputPort("default").connect(mergeNode.getOutputPort("out"))
    def build(self):
        ModelAsset = self.ShadingNetworkBase.build()
        pos = self.ShadingNetworkBase.getPos()
        MaterialAsset = self.AlembicInBase.build(pos)
        orig = self.AlembicInBase.getOrig()
        data = (MaterialAsset,ModelAsset,pos,orig)
        self.create(data)

def execute():
    #MatJsonFile = "//hnas01/data/Projects/DragonNest2_test/Materials/Characters/NGJL/default/c_NGJL_mat.matJson"
    ShadingJsonFile = "//hnas01/data/Projects/DragonNest2_test/Materials/Characters/NGJL/default/c_NGJL_mat.ShadingJson"
    app = KatanaSceneBuild(MatJsonFile,ShadingJsonFile)
    app.build()

if __name__ == "__main__":
    app = AlembicInBase()
    for item in app.MaterialData:
        print item,app.MaterialData[item]
    print "----------"
    for item in app.AlembicData:
        print item
