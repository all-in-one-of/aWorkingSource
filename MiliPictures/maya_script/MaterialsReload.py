#coding = utf-8
__author__ = 'kai.xu'


import maya.cmds as cmds
import os,time,sys,glob
import Mili.Module.utilties.lsrunas as lsrunas
import Mili.Module.utilties.Path as Path
import Mili.Module.utilties.Get as Get
from PIL import Image
reload(lsrunas)
reload(Get)
reload(Path)


class materialsReload():
    def __init__(self):
        self.Prj = Get.Get().GetActiveProject()        #~ 项目名
        self.ServerRoot = Path.Path().GetServerPath()  #~ 服务器地址
        self.LocalRoot = Path.Path().GetLocalPath()    #~ 本地地址
        self.alLayerShadersInformation = {}            #~ alLayer连接的两个材质球信息，用于手动赋予材质
        self.erroCollecterList = []                    #~ 错误信息收集列表
        self.showUploadWindows()

    def showUploadWindows(self):
        if cmds.window('MaterialsReload', q=1, exists=1):
            cmds.deleteUI('MaterialsReload')
        Mytool = cmds.window('MaterialsReload',title='MaterialsReload',widthHeight=(400,500),sizeable=False)
        cmds.rowColumnLayout( numberOfColumns=1, columnWidth=[(1, 400)])
        cmds.text(label='Little  Size  Textures  Reloader',height=30, backgroundColor=[0.5, 0.5, 0.6])
        cmds.separator(style='out')
        cmds.text(label='')
        cmds.rowColumnLayout( numberOfColumns=3, columnWidth=[(1, 100),(2,130),(3,165)])
        cmds.text(label='Size of Textures:')
        cmds.optionMenu('Size_of_Textures')
        cmds.menuItem( label='Thirty-second(1/32)' )
        cmds.menuItem( label='Sixteenth       (1/16)' )
        cmds.menuItem( label='Eighth             (1/ 8)' )
        cmds.button(label='Reload', command=lambda *arg: self.uploadButton(menuItemValue=cmds.optionMenu('Size_of_Textures', q=True, select=True)))
        cmds.text(label='')
        cmds.setParent('..')
        cmds.separator(style='out')
        cmds.text(label=u'注意：',align='left')
        cmds.text(' ')
        cmds.text(label=u'如果出现灰模，请手动检查材质球，重新连接贴图')
        cmds.text(' ')
        cmds.text(label=u'下面这些情况会导致灰模或者黑色材质：',align='left')
        cmds.text(' ')
        cmds.text(label=u'1. 材质球有多张贴图叠加',align='left')
        cmds.text(label=u'2. 材质球中有阿诺德的材质节点',align='left')
        cmds.text(label=u'3. 多个材质球叠加出最终的材质',align='left')
        cmds.text(' ')
        cmds.showWindow(Mytool)

    def showMateralsWindows(self):
        if cmds.window('MaterialsReload', q=1, exists=1):
            cmds.deleteUI('MaterialsReload')
        if cmds.window('CheckOutWindows', q=1, exists=1):
            cmds.deleteUI('CheckOutWindows')
        Mytool = cmds.window('CheckOutWindows',title='CheckOutWindows',width=550,sizeable=False)
        cmds.columnLayout(adjustableColumn=True)
        cmds.text(label='Check The Materials',height=30, backgroundColor=[0.5, 0.5, 0.6])
        cmds.separator(style='out')
        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1,200),(2,350)])
        cmds.columnLayout(adjustableColumn=True)
        cmds.text('Shaders',align='center',backgroundColor=[0.5, 0.6, 0.5])
        cmds.scrollLayout(height=250)
        cmds.iconTextRadioCollection('ShadersRadioCollection')
        shaderslist=list(self.alLayerShadersInformation.keys())
        #shaderslist=['test1_sadksdknmaskfncdvsd_sasjkcnioasdjncjisdncasmcsd',
        #             'test2_skcasdojcskdmnvsjsdfn_ajscnsdjkncjksdnfcvnskjdvnsj',
        #             'test3_zxmcdnmjcvnsdij_zxcjknsdjincinsdkvjndfjv']
        for shader in shaderslist:
            cmds.iconTextRadioButton('%s'%shader,collection='ShadersRadioCollection',
                                     style='iconAndTextHorizontal',l='%s'%shader,height=25,
                                     onCommand=lambda *args:self.showInformation())
        cmds.setParent('..')
        cmds.setParent('..')

        cmds.columnLayout(adjustableColumn=True)
        cmds.text('Information',align='center',backgroundColor=[0.5, 0.6, 0.5])
        cmds.scrollLayout('mainScrollLayout',height=250)
        cmds.setParent('..')
        cmds.setParent('..')

        cmds.showWindow(Mytool)

    def showInformation(self):
        #~ 删除已有的layout
        layoutList = cmds.scrollLayout('mainScrollLayout', childArray=True, q=True)
        if layoutList == None:
            pass
        else:
            for item in layoutList:
                cmds.deleteUI(item)
        #~ 获取材质数据
        ids = cmds.iconTextRadioCollection('ShadersRadioCollection', select=True, q=True)
        self.alShader = cmds.iconTextRadioButton(ids,label=True,q=True)
        colorNode = cmds.listConnections(self.alShader+'.color',plugs=True)
        colorFileList = cmds.ls(cmds.listHistory(colorNode), type='file') #~ 找到所有的贴图文件

        #~ 创建新的layout
        cmds.columnLayout(parent='mainScrollLayout',adjustableColumn=True)
        cmds.frameLayout(label='Textures', collapsable=True, collapse=False,width=330,borderStyle='etchedIn')
        cmds.scrollLayout(height=130)
        cmds.rowColumnLayout(numberOfColumns=len(colorFileList))
        for colorFile in colorFileList:
            #image = "B:/Projects/DragonNest2/Textures/Characters/lbt/Thirty-second/c_smth_pi_1_colour.jpg"
            #image='//hnas01\data/Projects/DragonNest2/Textures/Characters/nwn_Daily/eye_texture.jpg'
            try:
                image_tamp = cmds.getAttr(colorFile+'.fileTextureName')
                image = self.globUdim(image_tamp)
                icon =  self.filePathTranslate(server=image)
            except ValueError:
                image='galaxy.png'
            cmds.iconTextButton(style='iconOnly',image=icon,width=100,height=100,command=lambda *args:self.iconTexturesButton(image))
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.text(' ')
        cmds.columnLayout(parent='mainScrollLayout',adjustableColumn=True)
        cmds.rowColumnLayout(numberOfColumns=3, columnWidth=[(1,168),(2,168)])
        cmds.button(label='Select the Shader',height=30,command=lambda *args:self.selectTheShader(self.alShader))
        cmds.button(label='Select the Face',height=30,command=lambda *args:self.selectTheFace(self.alShader))
        cmds.setParent('..')
        cmds.button(label='Assign the Materials',height=40,width=330,command=lambda *args:self.assignTheMaterials(self.alShader))
        cmds.setParent('..')
        cmds.setParent('..')

    def iconTexturesButton(self,image):
        '''
        点击贴图赋予材质
        '''
        tmpFile = cmds.shadingNode('file',asTexture=True)
        cmds.setAttr(tmpFile+'.fileTextureName',image,type="string")
        tmpShader = cmds.shadingNode('lambert', asShader=True)
        cmds.connectAttr(tmpFile+'.outColor',tmpShader+'.color',force=True)
        self.selectTheFace(self.alShader)
        cmds.hyperShade( assign=tmpShader )

    def selectTheShader(self,shader):
        cmds.select(shader)

    def selectTheFace(self,shader):
        material = self.alLayerShadersInformation[shader]['material']
        materialsMap = self.materialsMap
        faceList = self.setMaterals(material=material,materialsMap=materialsMap)
        #print faceList
        cmds.select(faceList)
        pass

    def assignTheMaterials(self,shader):
        material = self.alLayerShadersInformation[shader]['material']
        tmpShader=shader
        materialsMap = self.materialsMap
        self.setMaterals(material=material,tmpShader=tmpShader,materialsMap=materialsMap)

    def uploadButton(self,menuItemValue=None):
        self.menuItemValue = menuItemValue
        materialsList = self.importMaterials()
        self.materialsMap = self.getMaterialsMap(materialsList)
        self.cleanReferenceNode()
        self.replaceTexture(materialsList, self.materialsMap, self.menuItemValue)
        if self.alLayerShadersInformation == {}:
            cmds.confirmDialog(title='DONE', message='Operation of Reload is Completed', button='Gotcha')
        else:
            self.showMateralsWindows()

    def importMaterials(self):
        old_materialsList = cmds.ls(materials=True)

        allGroup = cmds.ls(type='transform')
        keyGroup = []
        for item in allGroup:
            test = item.split('_')
            if test[-1] == 'GRP' and test[-2] == 'model':
                if len(item.split(":")) != 1:
                    keyGroup.append(item)
                #print item
        A = cmds.getAttr(keyGroup[0]+'.Name')
        print keyGroup
        tmp_modType = cmds.getAttr(keyGroup[0]+'.Type')
        tmp_keyName = cmds.getAttr(keyGroup[0]+'.treeName')

        if tmp_modType == 'c':
            modType = 'Characters'
        elif tmp_modType == 'p':
            modType = 'Props'
        else:
            pass
        keyName = tmp_keyName
        materialsFileName = tmp_modType+'_'+tmp_keyName+'_mat.mb'
        print '%s/%s/Materials/%s/%s/default/%s'%(self.ServerRoot,self.Prj,modType,keyName,materialsFileName)

        cmds.file('%s/%s/Materials/%s/%s/default/%s'%(self.ServerRoot,self.Prj,modType,keyName,materialsFileName),i=True, type='mayaBinary')
        

        tmp_materialsList = cmds.ls(materials=True)

        materialsList = []
        for item in tmp_materialsList:
            if item  not in old_materialsList:
                materialsList.append(item)
            else:
                None
                #print('%s is old material'%item)

        return materialsList

    def getMaterialsMap(self,argMaterialsList):
        materialsList = argMaterialsList
        tmp_keysList = []
        for item in materialsList:
            if item == 'lambert1' or item == 'particleCloud1' or item == 'shaderGlow1':
                pass
                #print('Delete %s'%item)
            else:
                try:
                    tmp = cmds.getAttr(item+'.model')
                    tmp_keysList.append(tmp)
                    #print('model-------->%s'%tmp)
                except:
                    pass
        keysList = []

        for item in tmp_keysList:            
            for i in item.split(','):
                keysList.append(i)
                
        tmp_valuesList = cmds.ls(type='mesh',noIntermediate=True)
        valuesList = []
        valueMap = {}
        for item in tmp_valuesList:
            tmp = item.split(':')
            valuesList.append(tmp[-1])
            valueMap[tmp[-1]] = item
        materialsMap = {}
        trashMeshNode = []
        

        if len(keysList) > len(valuesList):
            for key in keysList:
                tmp_key = key.split('_')
                test_key = tmp_key[1:-1]
                for value in valuesList:
                    tmp_value = value.split('_')
                    test_value = tmp_value[1:-1]
                    if len(test_key)== 0 or len(test_key)== 1 :
                        pass
                    elif test_key == test_value:
                        #print 'value---:%s'%value
                        #print 'key---:%s'%key
                        longValue = valueMap[value]
                        longNameValue = cmds.ls(longValue,long=True)
                        materialsMap[key] = longNameValue
                    else:
                        pass
        else:
            for value in valuesList:
                #print 'value---:%s'%value
                tmp_value = value.split('_')
                test_value = tmp_value[1:-1]
                #print test_value
                for key in keysList:
                    tmp_key = key.split('_')
                    test_key = tmp_key[1:-1]
                    #print tmp_key
                    #print test_key
                    if len(test_key)== 0 or len(test_key)== 1 :
                        None
                        #print('Junk material')
                    elif test_key == test_value:
                        #print 'value---:%s'%value
                        #print 'key---:%s'%key
                        longValue = valueMap[value]
                        longNameValue = cmds.ls(longValue,long=True)
                        materialsMap[key] = longNameValue
                    else:
                        None
                        #print('Diffient Material')

        return materialsMap

    def replaceTexture(self,argMaterialsList,argMaterialsMap,menuItemValue):
        materialsList = argMaterialsList
        materialsMap = argMaterialsMap

        sizeSelection = menuItemValue
        if sizeSelection == 1:
            texSize = 'Thirty-second'
            texSizeArg = 0.03125
        elif sizeSelection == 2:
            texSize = 'Sixteenth'
            texSizeArg = 0.0625
        elif sizeSelection == 3:
            texSize = 'Eighth'
            texSizeArg = 0.125
        else:
            texSizeArg = 1
            texSizeArg = 0.03125

        amount = 0
        cmds.progressWindow(title='Replace Materials',progress=amount,status='Copy: %s/%s'%(0,len(materialsList)),isInterruptable=True,minValue=0,maxValue=len(materialsList))
        #seach for every material
        for material in materialsList:
            # Check if the dialog has been cancelled
            if cmds.progressWindow(query=True, isCancelled=True):
                    break
            # Check if end condition has been reached
            if cmds.progressWindow(query=True,progress=True) >= 100:
                    break
            amount += 1
            cmds.progressWindow(edit=True, progress=amount, status='Copy: %s/%s'%(amount,len(materialsList)))
            #find all file
            allFileList = cmds.ls(type='file')
            #seach for every node which type is file
            for fileItem in allFileList:
                try:
                    self.fileWork(fileItem,texSize,texSizeArg)
                except:
                    self.erroRaise()
                    continue
            self.createShader(material,materialsMap)

        cmds.progressWindow(endProgress=1)

    def fileWork(self,fileItem,texSize,texSizeArg):
        '''
        将所有的文件路径都换到三十二分之一的位置
        '''
        #get file path
        try:
            tmp0_filePath = cmds.getAttr(fileItem+'.fileTextureName')
            tmp_filePath = self.globUdim(tmp0_filePath)
        except:
            self.erroRaise()
            return

        filePathList = tmp_filePath.split('/')
        if len(filePathList) < 6:
            return
        if filePathList[2] == 'hnas01' or filePathList[3] == 'data':
            #filePath = '//hnas01/data/Projects/PPR/Textures/Characters/bitly/Head_clr.jpg'
            filePath = '%s/%s/%s/%s/%s/%s/%s'%(self.ServerRoot,self.Prj,filePathList[6],filePathList[7],filePathList[8],texSize,filePathList[-1])
        elif filePathList[0] == 'D:':
            #filePath = 'D:/Projects/PPR/Textures/Characters/bitly/Head_clr.jpg'
            if filePathList[6] == texSize:
                filePath = '%s/%s/%s/%s/%s/%s/%s'%(self.ServerRoot,self.Prj,filePathList[3],filePathList[4],filePathList[5],texSize,filePathList[-1])
            else:
                filePath = '%s/%s/%s/%s/%s/%s/%s'%(self.ServerRoot,self.Prj,filePathList[3],filePathList[4],filePathList[5],texSize,filePathList[-1])
        else:
            pass
        keyFilePath = filePath.split('/')
        imageFilePath = '%s/%s/%s/%s/%s/%s'%(self.ServerRoot,self.Prj,keyFilePath[6],keyFilePath[7],keyFilePath[8],keyFilePath[-1])
        ServerFilePath = '%s/%s/%s/%s/%s/%s/%s'%(self.ServerRoot,self.Prj,keyFilePath[6],keyFilePath[7],keyFilePath[8],keyFilePath[9],keyFilePath[-1])
        #D:\Projects\RobRabbit_Test\Textures\Characters\bitly\Half\.jpg
        myLocalFilePath = '%s/%s/%s/%s/%s/%s/%s'%(self.LocalRoot,keyFilePath[5],keyFilePath[6],keyFilePath[7],
                                                     keyFilePath[8],keyFilePath[9],keyFilePath[-1])

        ##############################################
        #上传服务器
        #~ make sure that the texture is new and create low size texture
        if os.path.exists(ServerFilePath):
            try:
                lowTexTime = os.path.getmtime(ServerFilePath)
                highTexTime = os.path.getmtime(imageFilePath)
            except:
                self.erroRaise()
        else:
            lowTexTime = 0.0
            highTexTime = 0.0
        if os.path.exists(ServerFilePath) and str(lowTexTime) == str(highTexTime):
            pass
        else:
            tmpCmd = 'imconvert.exe "%s" -resize %s%% "%s"'%(imageFilePath,100*float(texSizeArg),myLocalFilePath)
            os.popen(tmpCmd)
            targetDir = '%s/%s/%s/%s/%s/%s'%(self.LocalRoot,keyFilePath[5],keyFilePath[6],
                                                      keyFilePath[7],keyFilePath[8],keyFilePath[9])
            itemName = keyFilePath[-1]
            try:
                origTime = os.path.getmtime(imageFilePath)
            except WindowsError:
                self.erroRaise()
            if not os.path.isdir(targetDir):
                os.makedirs(targetDir)

            try:
                os.utime(targetDir+'/'+itemName, (origTime,origTime))
            except:
                self.erroRaise()
            uploadTime = time.time()
            if not os.path.isdir(os.path.dirname(ServerFilePath)):
                lsrunas.lsrunas().CreateDir_1(os.path.dirname(ServerFilePath))
            try:
                lsrunas.lsrunas(uploadTime).UploadFile(myLocalFilePath, ServerFilePath, DeleteSource=False)
            except RuntimeError:
                self.erroRaise()
        ################################################

        #~ change file path to low texture
        cmds.setAttr(fileItem +'.fileTextureName',ServerFilePath,type='string')

    def createShader(self,material,materialsMap):
        #~ create shader
        try:
            tmpShader = cmds.shadingNode('lambert', asShader=True, name='lowTex_%s'%material)
            colorNode = cmds.listConnections(material+'.diffuseColor',plugs=True)
            historyFile = cmds.ls(cmds.listHistory(colorNode), type='file')
            if len(historyFile) == 1:
                cmds.connectAttr(historyFile[0]+'.outColor',tmpShader+'.color',force=True)
            elif len(historyFile) > 1:
                cmds.connectAttr(colorNode[0],tmpShader+'.color',force=True)
            else:
                pass
            self.setMaterals(material=material,tmpShader=tmpShader,materialsMap=materialsMap)
        except:
            self.erroRaise()     #~ 获取错误信息
            #~ 修补不同材质制作方法的各种Bug
            #~ 第一种情况，材质是alLayer
            alLayerList = cmds.ls(cmds.listHistory(material), type='alLayer')
            for alLayer in alLayerList:
                #~ 根据alLayer找到连接的两个材质球，再根据这个材质球找到贴图，递推
                try:#~ layer1
                    alShader = (cmds.listConnections(alLayer+'.layer1',plugs=True,connections=True)[-1]).split('.')[0]
                    colorNode = cmds.listConnections(alShader+'.diffuseColor',plugs=True)
                    tmpShader = cmds.shadingNode('lambert', asShader=True, name='%s_lambert'%alShader)
                    cmds.connectAttr(colorNode[0],tmpShader+'.color',force=True)
                    infoMap = {'material':material}
                    self.alLayerShadersInformation[tmpShader] = infoMap
                except:
                    self.erroRaise(tmpShader)
                try:#~ layer2
                    alShader = (cmds.listConnections(alLayer+'.layer2',plugs=True,connections=True)[-1]).split('.')[0]
                    colorNode = cmds.listConnections(alShader+'.diffuseColor',plugs=True)
                    tmpShader = cmds.shadingNode('lambert', asShader=True, name='%s_lambert'%alShader)
                    cmds.connectAttr(colorNode[0],tmpShader+'.color',force=True)
                    infoMap = {'material':material}
                    self.alLayerShadersInformation[tmpShader] = infoMap
                except:
                    self.erroRaise(tmpShader)

    def setMaterals(self,material=None,tmpShader=None,materialsMap=None):
        self.faceList = []
        #~ get every face a material
        try:
            tmpModel = cmds.getAttr(material+'.model')
        except:
            self.erroRaise()
            return
        modelList = []
        for item in tmpModel.split(','):
            modelList.append(item)
        for item in modelList:
            try:
                longObjectName = materialsMap[item]
            except:
                self.erroRaise()
                continue
            tmp_faceList = item.split('.')
            if len(tmp_faceList) == 2:
                faceList = longObjectName[0] +'.'+tmp_faceList[-1]
                self.faceList.append(faceList)
            elif len(tmp_faceList) == 1:
                faceList = longObjectName[0]
                self.faceList.append(faceList)
            else:
                pass
            if tmpShader != None:
                cmds.select(faceList)
                cmds.hyperShade(assign=tmpShader)
        return self.faceList

    def cleanReferenceNode(self):
        allMesh = cmds.ls(type = 'mesh')
        for item in allMesh:
            referenceNode = cmds.listConnections(item,s=False,d=True,type='reference',connections=True,plugs=True)
            if referenceNode == None:
                pass
            else:
                if len(referenceNode) > 2:
                    pass
                nodeLink = len(referenceNode)/2
                for id in range(nodeLink):
                    i = id*2
                    cmds.disconnectAttr(referenceNode[i],referenceNode[i+1])
            cmds.select(item)
            cmds.hyperShade( assign='lambert1' )

    def erroRaise(self,info=None,kill=False):
        '''
        这个函数用于收集错误信息
        kill参数为True时会在报错时终止程序
        '''
        s=sys.exc_info()
        erro = "===Erro Report:===\n" \
               "Error : '%s' \n" \
               "Happened on line %d\n" \
               "%s\n" \
               "==================\n\n" % (s[1],s[2].tb_lineno,info)
        self.erroCollecterList.append(erro)   #~ 初始化时定义self.erroCollecterList的列表
        #~ 是否终止程序
        if kill == True:
            sys.exit()

    def globUdim(self,texPath):
        if '<udim>' in texPath.lower():
            allTextures = glob.glob(texPath.replace('<udim>','*').replace('<UDIM>','*'))
        else:
            allTextures = texPath
        return allTextures

    def filePathTranslate(self,local=None,server=None):
        '''
        转换服务器地址和本地地址
        '''
        self.ServerRoot  #~ 服务器地址
        self.LocalRoot   #~ 本地地址
        if local != None and server == None:
            #~ 将本地地址换成服务器地址
            result = local.replace(self.LocalRoot,self.ServerRoot)
        elif local == None and server != None:
            #~ 将服务器地址换成本地地址
            result = server.replace(self.ServerRoot,self.LocalRoot)
        else:
            result = None
        return result
if __name__ == '__main__':
    materialsReload()
