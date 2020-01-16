#coding: utf-8
'''
udimConvert

gong.huang
edit by kai.xu

usage:
import Mili.Module.modeling.udimConvert as udimConvert
reload(udimConvert)
udimConvert.udimConvert().showUI()


'''

import maya.cmds as cmds
import os,sys,glob,json
import Mili.Module.utilties.lsrunas as lsrunas
import Mili.Module.utilties.Path as Path
reload(Path)
reload(lsrunas)

class util():
    def intsToStrExpr(self, intList):
        fList = sorted(intList)
        pairs = []
        if len(fList)>0:
            pairs=[  [fList[0],]  ]
            for i in fList[1:]:
                if i == pairs[-1][-1]+1:
                    pairs[-1].append(i)
                else:
                    pairs.append( [i,])
        outStr = ''
        outList = []
        for i in pairs:
            print i
            if len(i)==1:
                outList.append('%i'%(i[0]))
            else:
               outList.append('%i-%i'%(i[0],i[-1]))
        return '  '.join(outList)


class udimConvert():
    def __init__(self):
        #self.uvKeyInfo = self.getUVInformation(clearInfo=True) #~ 遍历一遍需要很多时间，所以只在初始化时执行一次
        pass

    def showUI(self,checkOffList=[]):
        if cmds.window('udimConvertWindow',q=1,exists=1):
            cmds.deleteUI('udimConvertWindow')
        cmds.window('udimConvertWindow', title='Convert UDIM Textures')
        self.mainScroll = cmds.scrollLayout()
        mainCol = cmds.rowColumnLayout(nc=1)
        
        cmds.rowColumnLayout(nc=4)
        cmds.button(l='Extract UDIM to Chain',h=30, c=lambda *args: self.convertNodes())
        self.createContainerCB = cmds.checkBox(l='Create Container',v=0)
        #~ cmds.button(l='Convert back to UDIM',visible=0)
        cmds.button(l='Delete The Trash', c=lambda *args: self.deleteSuperfluousTextures())
        cmds.button(l='Refresh', c=lambda *args: self.refresh())
        cmds.setParent('..')
        
        self.checkDict = {}
        
        self.getTextures()  # get self.udimTextures, self.bigTextures
        cmds.frameLayout(collapsable=1,label='Udim Textures')
        udimRC = cmds.rowColumnLayout(nc=1)
        for i in sorted(self.udimTextures.keys(), key=lambda x: os.path.basename(x).lower()):
            texPath = self.udimTextures[i][0]
            numList = self.udimTextures[i][1]
            fileNodes= self.udimTextures[i][2:]
            
            cmds.text(l='',h=5, bgc=[0.2]*3)
            tmpFL = cmds.formLayout()
            tmpBtn = cmds.iconTextButton(l='    '+os.path.basename(texPath)+u'             (%i节点, %i纹理)'%(len(fileNodes),len(numList)), 
                                                                ann=texPath.replace('/','\\'),
                                                                w=350, h=20, flat=0,st='iconAndTextHorizontal')
            tmpCB = cmds.checkBox(l='',h=20, v=1)
            cmds.setParent(udimRC)
            tmpFrm = cmds.frameLayout(labelVisible=0,collapsable=1,collapse=1)
            cmds.rowColumnLayout(nc=2)
            cmds.text(l='',w=15)
            cmds.textField(text=texPath.replace('/','\\'), editable=0,w=310)
            cmds.text(l='',w=15)
            cmds.textField(text=util().intsToStrExpr([x%1000 for x in numList]), editable=0,w=310)
            
            cbList=[]
            for j in fileNodes:
                cmds.text(l='',w=15)
                if j in checkOffList:
                    cmds.checkBox(tmpCB,e=1,v=0)
                    
                cbList.append(cmds.checkBox(l=j,ann=j,v=(j not in checkOffList), ofc=lambda a=1,b=tmpCB,c=1: cmds.checkBox(b,e=1,v=0)))
            self.checkDict[texPath] = cbList
            for j in cbList:
                cmds.checkBox(j,e=1, onc=lambda a=1,b=cbList,c=tmpCB,d=1: cmds.checkBox(c,e=1, v=min([1]+[int(cmds.checkBox(x,q=1,v=1)) for x in b])))
            cmds.setParent(udimRC)
            cmds.iconTextButton(tmpBtn, e=1, 
                                            c=lambda a=1,b=tmpFrm, c=1: cmds.frameLayout(b,e=1,collapse=not cmds.frameLayout(b,q=1,collapse=1))
                                            )
            cmds.checkBox(tmpCB,e=1, 
                                        onc= lambda a=1,b=cbList,c=1: [cmds.checkBox(x,e=1,v=1) for x in b],
                                        ofc= lambda a=1,b=cbList,c=1: [cmds.checkBox(x,e=1,v=0) for x in b],
                                    )
            
        cmds.setParent(mainCol)
        cmds.text(l='', h=8)
        
        cmds.frameLayout(collapsable=1,label='Big Textures', visible=0)
        
        cmds.setParent('..')
        
        cmds.showWindow('udimConvertWindow')
    
    def getTextures(self):
        udimTextures = {}
        bigTextures = {}
        self.coordNodes={}
        
        for i in cmds.ls(type=['file']):
            texPath = cmds.getAttr(i+'.fileTextureName')
            texName = os.path.basename(texPath).lower()
            texPathL = texPath.lower()
            
            haveOutConn=0
            for attr in ['outColor','outColorR','outColorG','outColorB','outTransparency','outTransparencyR','outTransparencyG','outTransparencyB','outAlpha']:
                conn = cmds.listConnections(i+'.'+attr, s=0,d=1)
                if conn:
                    haveOutConn=1
                    break
            if not haveOutConn:
                continue
            
            if '<udim>' in texName:
                if texPathL not in udimTextures:
                    udimTextures[texPathL] = [texPath,[]] #第一个元素是原路径，第二个将是图号List，后面元素将是file节点名
                udimTextures[texPathL].append(i)
            
            conn = cmds.listConnections(i+'.uvCoord',type='place2dTexture',s=1,d=0)
            coordNode=None
            if conn:
                coordNode = conn[0]
            self.coordNodes[i]=coordNode
        for texPathL in udimTextures:
            #~ print udimTextures[texPathL]
            allSubTex = glob.glob(texPathL.replace('<udim>', '*[0-9]'))
            prefix,suffix = texPathL.split('<udim>')[:2]
            imgNumList = udimTextures[texPathL][1]
            for j in allSubTex:
                imgNumList.append( int(j[len(prefix):-len(suffix)]) )
            #~ print udimTextures[texPathL][1]
        
        self.udimTextures = udimTextures
        self.bigTextures = bigTextures
            
    def convertNodes(self):
        for texPath in self.checkDict:
            prefix,suffix = texPath.lower().split('<udim>')[0:2]
            cbList = self.checkDict[texPath]
            """遍历每个贴图文件，生成贴图串"""
            for cb in cbList:
                if not cmds.checkBox(cb,q=1,v=1):
                    continue
                fileNode = cmds.checkBox(cb,q=1,label=1)
                newNodes = []

                #print fileNode
                #print self.udimTextures[texPath.lower()][1]
                #continue
                lastNode=None
                lastP2d=None
                for i in self.udimTextures[texPath.lower()][1]:
                    num = int(i%1000)
                    x = (num-1)%10
                    y = int((num-1)/10)

                    tmpPath = texPath[:len(prefix)]+str(i)+texPath[-len(suffix):]

                    #~ print os.path.isfile(tmpPath), tmpPath
                    newFN = cmds.duplicate(fileNode, name=self.uniqName(fileNode.split(':')[-1]+'_%4d_'%i), smartTransform=1)[0]
                    newP2d = cmds.createNode('place2dTexture', name=self.uniqName('p2d_'+fileNode.split(':')[-1]+'_%4d_'%i))
                    cmds.setAttr(newP2d+'.wrapU',0)
                    cmds.setAttr(newP2d+'.wrapV',0)
                    cmds.setAttr(newP2d+'.translateFrameU',x)
                    cmds.setAttr(newP2d+'.translateFrameV',y)
                    
                    newNodes += [newFN, newP2d]
                    
                    for p2dCon in [('.outUV', '.uvCoord'), ('.outUvFilterSize', '.uvFilterSize'), ('.coverage', '.coverage'), ('.translateFrame', '.translateFrame'), ('.rotateFrame', '.rotateFrame'), ('.mirrorU', '.mirrorU'), ('.mirrorV', '.mirrorV'), ('.stagger', '.stagger'), ('.wrapU', '.wrapU'), ('.wrapV', '.wrapV'), ('.repeatUV', '.repeatUV'), ('.vertexUvOne', '.vertexUvOne'), ('.vertexUvTwo', '.vertexUvTwo'), ('.vertexUvThree', '.vertexUvThree'), ('.vertexCameraOne', '.vertexCameraOne'), ('.noiseUV', '.noiseUV'), ('.offset', '.offset'), ('.rotateUV', '.rotateUV')]:
                        cmds.connectAttr(newP2d+p2dCon[0], newFN+p2dCon[1], f=1)
                    #~ print newFN
                    cmds.setAttr(newFN+'.fileTextureName', tmpPath, type='string')
                    
                    if not lastNode:
                        conn = cmds.listConnections(fileNode, s=0,d=1,connections=1,plugs=1)
                        #~ print conn
                        if conn:
                            for j in range(0,len(conn),2):
                                cmds.connectAttr(newFN+'.'+conn[j].split('.',1)[-1], conn[j+1], f=1)
                        
                        #mark attr on old fileNode
                        #~ cmds.addAttr(fileNode, ln='udimTrailHead', at='bool')
                        #~ cmds.connectAttr(newFN+'.disableFileLoad', fileNode+'.udimTrailHead')
                        
                    else:
                        cmds.connectAttr(newFN+'.outColor', lastNode+'.defaultColor')
                        pass
                    if lastP2d:
                        cmds.connectAttr(newP2d+'.fast', lastP2d+'.fast')
                    lastNode = newFN
                    lastP2d = newP2d


                if cmds.checkBox(self.createContainerCB ,q=1,v=1):
                    self.createContainer(newNodes,name=fileNode+'_udim')
                # mel: hyperShadePanelGraphCommand("hyperShadePanel1", "createContainer");
        #~ global superfluousFileList
        #~ self.jsonData(list=superfluousFileList,save=True)
        self.refresh()
    
    def combineTexture(self,textureList=[]):
        pass
    
    def createContainer(self,nodeList,name='myFile_udim'):
        if nodeList!=[]:
            containerName = self.uniqName(name)
            cmds.container(name=containerName, includeShapes=1, includeTransform=1, force=1, addNode=nodeList)
            for i in cmds.ls(nodeList, type='file'):
                cmds.container(containerName, e=1, publishAndBind=[i+".fileTextureName", "texture_%s"%(i.split('_')[-2])])

    def refresh(self):
        offCheckList = []
        for texPath in self.checkDict:
            cbList = self.checkDict[texPath]
            for cb in cbList:
                fileNode = cmds.checkBox(cb,q=1,label=1)
                if not cmds.checkBox(cb,q=1,v=1):
                    offCheckList.append(fileNode)
        udimConvert().showUI(checkOffList = offCheckList)

    def uniqName(self,name):
        name0 = name
        i=1
        while cmds.objExists(name0):
            name0 = name+'_%i'%i
            i+=1
        return name0

    def deleteSuperfluousTextures(self):
        uvInformationList = self.getUVInformation()
        deleteNodeList = []
        """遍历每个mesh中的信息"""
        for meshInfo in uvInformationList:
            meshVtxUVList = meshInfo['meshVtxUVList']
            """遍历mesh中file节点的UV信息"""
            for filep2dNode in meshInfo['meshFileP2dList']:
                p2d = filep2dNode[1]
                x = cmds.getAttr(p2d+'.translateFrameU')
                y = cmds.getAttr(p2d+'.translateFrameV')
                uv = [int(x),int(y)]
                wrapU = cmds.getAttr(p2d+'.wrapU')
                wrapV = cmds.getAttr(p2d+'.wrapV')
                #if filep2dNode[0] == 's_FISH_WG_CB_mat_1442913554672_file67_1001_':
                if wrapU==False and wrapV==False:
                    if not uv in meshVtxUVList:
                        deleteNodeList.append(filep2dNode)


        self.deleteNodeList(deleteNodeList)
        cmds.confirmDialog(title='Job Done', message='All trash had removed!', button='Gocha!')

    def deleteNodeList(self,fileList):
        #fileList = self.getAllUdimNode()
        deleteList = []
        for file in fileList:
            """连接file文件"""
            trash = ['dead','','']
            #cmds.connectAttr(newFN+'.outColor', lastNode+'.defaultColor')
            outputFileAttr = cmds.listConnections(file[0],s=False,d=True)
            inputFileAttr = cmds.listConnections(file[0],s=True,d=False)
            firstArg = None
            for key in inputFileAttr:
                if cmds.objectType(key,isType='file'):
                    firstArg = key
                    continue
            if outputFileAttr != None:
                secondArg = outputFileAttr[0]
            else:
                secondArg = None
            #print '------------'
            #print file[0]
            #print firstArg
            #print secondArg
            """重新连接file属性"""
            if outputFileAttr != None and inputFileAttr!= None and firstArg != None:
                if cmds.objectType(firstArg,isType='file') and cmds.objectType(secondArg,isType='file'):
                        cmds.connectAttr(firstArg+'.outColor', secondArg+'.defaultColor',f=True)
                else:
                    trash[0] = 'mercy'
                    '''
                    plugsAttr = cmds.listConnections(file[0],s=False,d=True,connections=True,plugs=True)
                    print '--------'
                    for i in range(len(plugsAttr)/2):
                        key = 2*i
                        firstAttr = firstArg + '.' +plugsAttr[key].split('.')[-1]
                        secondAttr = plugsAttr[key+1]
                        print 'firstAttr:%s'%firstAttr
                        print 'secondAttr:%s'%secondAttr
                        cmds.connectAttr(firstAttr, secondAttr,f=True)
                    '''
            trash[1] = file[0]
            """连接place2dTexture文件"""
            outputP2dAttr = cmds.listConnections(file[1],s=False,d=True)
            inputP2dAttr = cmds.listConnections(file[1],s=True,d=False)
            secondArg = None
            for key in outputP2dAttr:
                if cmds.objectType(key,isType='place2dTexture'):
                    secondArg = key
                    continue
            if inputP2dAttr != None:
                firstArg = inputP2dAttr[0]
            else:
                firstArg = None
            #print '------------'
            #print file[1]
            #print firstArg
            #print secondArg
            #print outputP2dAttr
            #print inputP2dAttr
            """重新连接place2dTexture属性"""
            if outputP2dAttr != None and inputP2dAttr!= None and firstArg != None and secondArg != None:
                cmds.connectAttr(firstArg+'.fast', secondArg+'.fast',f=True)
            trash[2] = file[1]
            deleteList.append(trash)

        for item in deleteList:
            if item[0] != 'mercy':
                try:
                    cmds.delete(item[1])
                    print "//Delete %s"%item[1]
                    cmds.delete(item[2])
                    print "//Delete %s"%item[2]
                except ValueError:
                    print "//Delete %s"%item[1]
                    print "//Delete %s"%item[2]
        """将材质球之后第一个file文件留到最后删除，这样可以少连接很多次，避免出错，虽然出的什么错我也不知道"""
        for item in deleteList:
            if item[0] == 'mercy':
                inputFileAttr = cmds.listConnections(item[1],s=True,d=False)
                firstArg = None
                for key in inputFileAttr:
                    if cmds.objectType(key,isType='file'):
                        firstArg = key
                        continue
                if firstArg != None:
                    plugsAttr = cmds.listConnections(item[1],s=False,d=True,connections=True,plugs=True)
                    for i in range(len(plugsAttr)/2):
                        key = 2*i
                        firstAttr = firstArg + '.' +plugsAttr[key].split('.')[-1]
                        secondAttr = plugsAttr[key+1]
                        cmds.connectAttr(firstAttr, secondAttr,f=True)
                """删除节点"""
                try:
                    cmds.delete(item[1])
                    print "//Delete %s"%item[1]
                    cmds.delete(item[2])
                    print "//Delete %s"%item[2]
                except ValueError:
                    print "//Delete %s"%item[1]
                    print "//Delete %s"%item[2]

    def getUVInformation(self):
        allMeshList = cmds.ls(type='mesh')
        noUVMeshList = []
        uvInformationList = []
        oldMeshList = []
        amount = 0
        cmds.progressWindow(title='Get UV Information',progress=amount,status='Reading: %s/%s'%(0,len(allMeshList)),isInterruptable=True,minValue=0,maxValue=len(allMeshList))
        for mesh in allMeshList:
            if not mesh in oldMeshList: #~ 检验模型是否已经检查过
                meshFileP2dList = []
                meshVtxUVList = []
                """找到所有的file和p2d"""
                shadingEngine = cmds.listConnections(mesh,s=False,d=True,type='shadingEngine')
                if shadingEngine == None:
                    continue
                p2dList = cmds.ls(cmds.listHistory(shadingEngine),type='place2dTexture')
                for p2d in p2dList:
                    outputP2dAttr = cmds.listConnections(p2d,s=False,d=True)
                    file = None
                    for item in outputP2dAttr:
                        if file == None and cmds.objectType(item,isType='file'):
                            file = item
                            break
                    tmp = [file,p2d]
                    meshFileP2dList.append(tmp)

                """获取模型的shadingEngine连接的所有模型信息"""
                allShaderMesh = []
                allMeshTrans = cmds.listConnections(shadingEngine,s=True,d=False,type='mesh')
                for meshTrans in allMeshTrans:
                    shaderMesh = cmds.listRelatives(meshTrans)[0]
                    allShaderMesh.append(shaderMesh)
                """遍历这些模型对应的UV信息"""
                for shaderMesh in allShaderMesh:
                    """进度条"""
                    # Check if the dialog has been cancelled
                    if cmds.progressWindow(query=True, isCancelled=True):
                            break
                    amount += 1
                    cmds.progressWindow(edit=True, progress=amount, status='Reading: %s/%s'%(amount,len(allMeshList)) )

                    oldMeshList.append(shaderMesh)
                    vtxIndexList = cmds.getAttr(shaderMesh+'.vrts',multiIndices=True)

                    for vtx in vtxIndexList:
                        cmds.select(shaderMesh+'.vtx[%s]'%vtx)
                        uvMap = cmds.polyListComponentConversion(fv=True,tuv=True)
                        if uvMap != []:
                            uvInfo = cmds.polyEditUV(uvMap,q=True)
                            uValue = int(uvInfo[0])
                            vValue = int(uvInfo[1])
                            uvBlock = [uValue,vValue]
                            if not uvBlock in meshVtxUVList:
                                meshVtxUVList.append(uvBlock)
                        else:
                            noUVMeshList.append(shaderMesh)

                tmpMeshMap = {'meshFileP2dList':meshFileP2dList,'meshVtxUVList':meshVtxUVList}
                uvInformationList.append(tmpMeshMap)
        cmds.progressWindow(endProgress=1)
        return uvInformationList

    def filePathTranslate(self,local=None,server=None):
        '''
        转换服务器地址和本地地址
        '''
        self.ServerRoot = Path.Path().GetServerPath()  #~ 服务器地址
        self.LocalRoot = Path.Path().GetLocalPath()    #~ 本地地址
        if local != None and server == None:
            #~ 将本地地址换成服务器地址
            result = local.replace(self.LocalRoot,self.ServerRoot)
        elif local == None and server != None:
            #~ 将服务器地址换成本地地址
            result = server.replace(self.ServerRoot,self.LocalRoot)
        else:
            result = None
        return result

    def jsonData(self,list=None,save=False,load=False):
        if list != None and save == True:
            file = open('D:/activeMap.txt', 'wb')
            json.dump(list, file)
            file.close()
        if list==None and load==True:
            file = open('D:/activeMap.txt', 'rb')
            activeMap = json.load(file)
            file.close()
            return activeMap

if __name__=='__main__':
    udimConvert().showUI()