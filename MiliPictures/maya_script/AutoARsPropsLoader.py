#coding:utf-8
__author__ = 'kai.xu'

import json,os,sys,time,datetime
import maya.cmds as cmds
import Mili.Module.utilties.lsrunas as lsrunas
import Mili.Module.utilties.Get as Get
import Mili.Module.utilties.Path as Path
reload(lsrunas)
reload(Get)
reload(Path)

class AutoARsPropsLoader():
    def __init__(self):
        self.getNamespace()
        ServerRoot = Path.Path().GetServerPath()
        LocalRoot = Path.Path().GetLocalPath()
        Prj = Get.Get().GetActiveProject()
        #//hnas01/data/Projects/DragonNest2/
        self.ServerRootPath = ServerRoot+'/'+Prj+'/'+'Rigs'+'/'+'HugePropsSets'+'/'
        self.LocalRootPath = LocalRoot+'/'+Prj+'/'+'Rigs'+'/'+'HugePropsSets'+'/'
        self.ServerFilePath = ServerRoot+'/'+Prj+'/'+'Rigs'+'/'+'Props'+'/'

    def uploadGui(self):
        if cmds.window('HugePropsDataUpload', q=1, exists=1):
            cmds.deleteUI('HugePropsDataUpload')
        MyUploadtool = cmds.window('HugePropsDataUpload',title='Huge Props Data Upload',widthHeight=(550,500),sizeable=False)
        cmds.columnLayout(adjustableColumn=True)
        cmds.text(label='Huge Props Data Upload',height=30, backgroundColor=[0.5, 0.5, 0.6])
        cmds.separator(style='out')
        cmds.columnLayout(adjustableColumn=True)

        cmds.columnLayout(adjustableColumn=True)
        cmds.text('Name The Huge Props',align='center',backgroundColor=[0.5, 0.6, 0.5])
        cmds.columnLayout(adjustableColumn=True)
        cmds.text(' ')
        cmds.rowColumnLayout(numberOfColumns=3, columnWidth=[(1, 100),(2,200),(3,50)])
        cmds.text('Huge Props Name: ',align='right',height=25)
        cmds.textField('Huge_Props_Name',editable=False)
        cmds.button('edit_lock',label='New',command=lambda *arg: self.edit_lock())
        cmds.setParent('..')
        cmds.text(' ')
        cmds.setParent('..')
        cmds.setParent('..')


        cmds.setParent('..')

        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1,200),(2,350)])
        cmds.columnLayout(adjustableColumn=True)
        cmds.text('Huge Props List',align='center',backgroundColor=[0.5, 0.6, 0.5])
        cmds.scrollLayout('hugePropsScrollLayout', height=200)
        self.HugePropsListPlus()
        cmds.setParent('..')
        cmds.setParent('..')

        cmds.columnLayout(adjustableColumn=True)
        cmds.text('AR and Props List',align='center',backgroundColor=[0.5, 0.6, 0.5])
        cmds.scrollLayout('mainScrollLayout', height=200)
        cmds.setParent('..')
        cmds.setParent('..')

        cmds.setParent('..')
        cmds.setParent('..')

        cmds.setParent('..')
        cmds.columnLayout(adjustableColumn=True)
        cmds.separator(style='out')
        cmds.rowColumnLayout(numberOfColumns=3, columnWidth=[(1,250),(2,150),(3,150)])
        cmds.button(label='Add',height=50,command=lambda *arg: self.add())
        cmds.button(label='Upload',height=50,command=lambda *arg: self.Upload())
        cmds.button(label='Remove',height=50,command=lambda *arg: self.remove())
        cmds.setParent('..')
        cmds.separator(style='out')
        cmds.setParent('..')

        cmds.showWindow(MyUploadtool)

    def downloadGui(self):
        if cmds.window('HugePropsDownload', q=1, exists=1):
            cmds.deleteUI('HugePropsDownload')
        MyDownLoadtool = cmds.window('HugePropsDownload',title='Huge Props Download',widthHeight=(550,500),sizeable=False)
        cmds.columnLayout(adjustableColumn=True)
        cmds.text(label='Huge Props Download',height=30, backgroundColor=[0.5, 0.5, 0.6])
        cmds.separator(style='out')
        cmds.columnLayout(adjustableColumn=True)

        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1,200),(2,350)])
        cmds.columnLayout(adjustableColumn=True)
        cmds.text('Huge Props List',align='center',backgroundColor=[0.5, 0.6, 0.5])
        cmds.scrollLayout('hugePropsScrollLayout', height=200)
        self.HugePropsList()
        cmds.setParent('..')
        cmds.setParent('..')

        cmds.columnLayout(adjustableColumn=True)
        cmds.text('AR and Props List',align='center',backgroundColor=[0.5, 0.6, 0.5])
        cmds.scrollLayout('mainScrollLayout', height=200)
        cmds.setParent('..')
        cmds.setParent('..')

        cmds.setParent('..')
        cmds.setParent('..')

        cmds.setParent('..')
        cmds.columnLayout(adjustableColumn=True)
        cmds.separator(style='out')
        cmds.button(label='Download',height=50,command=lambda *arg: self.Download())
        cmds.separator(style='out')
        cmds.setParent('..')

        cmds.showWindow(MyDownLoadtool)

    def HugePropsList(self):
        try:
            hugePropsList = os.listdir(self.ServerRootPath)
        except WindowsError:
            hugePropsList = []
        cmds.columnLayout(parent='hugePropsScrollLayout',adjustableColumn=True)
        cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection')
        self.id = -1
        for i in range(len(hugePropsList)):
            self.id += 1
            cmds.iconTextRadioButton('id%s_hugePropsIconTextRadioButton'%i,collection='hugePropsIconTextRadioCollection',
                                     style='iconAndTextHorizontal',l='%s'%hugePropsList[i], width=190, height=25,
                                     onCommand=lambda *arg:self.refreshPropsList())
        cmds.setParent('..')

    def edit_lock(self):
        key = cmds.textField('Huge_Props_Name',editable=True,q=True)
        if key == False:
            cmds.textField('Huge_Props_Name',editable=True,text='',e=True)
            cmds.button('edit_lock',label='Lock',e=True)
        else:
            cmds.textField('Huge_Props_Name',editable=False,e=True)
            cmds.button('edit_lock',label='New',e=True)
        self.refresh()

    def refreshHugePropsList(self):
        #delete old
        layoutList = cmds.scrollLayout('hugePropsScrollLayout', childArray=True, q=True)
        if layoutList == None:
            pass
        else:
            for item in layoutList:
                cmds.deleteUI(item)

        #bulid new
        hugePropsList = os.listdir(self.ServerRootPath)
        cmds.columnLayout(parent='hugePropsScrollLayout',adjustableColumn=True)
        cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection')
        self.id = -1
        for i in range(len(hugePropsList)):
            self.id += 1
            cmds.iconTextRadioButton('id%s_hugePropsIconTextRadioButton'%i,collection='hugePropsIconTextRadioCollection',
                                     style='iconAndTextHorizontal',l='%s'%hugePropsList[i], width=190, height=25,
                                     onCommand=lambda *arg:self.refreshPropsList())
        cmds.setParent('..')

    def refreshPropsList(self):
        #delete old
        layoutList = cmds.scrollLayout('mainScrollLayout', childArray=True, q=True)
        if layoutList == None:
            pass
        else:
            for item in layoutList:
                cmds.deleteUI(item)

        #build new
        try:
            hugePropsName = cmds.textField('Huge_Props_Name',text=True,q=True)
            if hugePropsName == '':
                ids = cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection', select=True, q=True)
                hugePropsName = cmds.iconTextRadioButton(ids,label=True,q=True)
        except RuntimeError:
            ids = cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection', select=True, q=True)
            hugePropsName = cmds.iconTextRadioButton(ids,label=True,q=True)
        allFileList = os.listdir(self.ServerRootPath+hugePropsName)
        fileName = sorted(allFileList)[-1]
        ServerRootPath = self.ServerRootPath+hugePropsName+'/'+fileName
        try:
            file = open(ServerRootPath, 'rb')
            informationMapList = json.load(file)
            file.close()
        except:
            informationMapList = []

        cmds.columnLayout(parent='mainScrollLayout',adjustableColumn=True)
        cmds.iconTextRadioCollection('keyIconTextRadioCollection')
        for i in range(len(informationMapList)):
            text=informationMapList[i]['name']+'  --->  '+informationMapList[i]['parent']
            cmds.rowColumnLayout(numberOfColumns=3, columnWidth=[(1,260),(2,40),(3,40)])
            cmds.iconTextRadioButton('%s:%s'%(informationMapList[i]['name'],informationMapList[i]['ctrlCircle']),collection='keyIconTextRadioCollection',
                                     style='iconAndTextHorizontal',l='%s'%text, width=335, height=25,onCommand=lambda *arg:self.selectObjects())
            cmds.text('active:',enable=False)
            cmds.checkBox('id_%s'%informationMapList[i]['ctrlCircle'], l='', height=25, visible=True, value=True)
            cmds.setParent('..')
        cmds.setParent('..')
        self.checkParent()

    def refresh(self):
        #delete old propsList
        layoutList = cmds.scrollLayout('mainScrollLayout', childArray=True, q=True)
        if layoutList == None:
            pass
        else:
            for item in layoutList:
                cmds.deleteUI(item)

        #delete old hugePropsList
        layoutList = cmds.scrollLayout('hugePropsScrollLayout', childArray=True, q=True)
        if layoutList == None:
            pass
        else:
            for item in layoutList:
                cmds.deleteUI(item)

        #bulid new
        hugePropsList = os.listdir(self.ServerRootPath)
        cmds.columnLayout(parent='hugePropsScrollLayout',adjustableColumn=True)
        cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection')
        self.id = -1
        for i in range(len(hugePropsList)):
            self.id += 1
            cmds.iconTextRadioButton('id%s_hugePropsIconTextRadioButton'%i,collection='hugePropsIconTextRadioCollection',
                                     style='iconAndTextHorizontal',l='%s'%hugePropsList[i], width=190, height=25,
                                     onCommand=lambda *arg:self.refreshPropsListPlus())
        cmds.setParent('..')

    def refreshPropsListPlus(self):
        #delete old
        layoutList = cmds.scrollLayout('mainScrollLayout', childArray=True, q=True)
        if layoutList == None:
            pass
        else:
            for item in layoutList:
                cmds.deleteUI(item)

        #build new
        try:
            hugePropsName = cmds.textField('Huge_Props_Name',text=True,q=True)
            if hugePropsName == '':
                ids = cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection', select=True, q=True)
                hugePropsName = cmds.iconTextRadioButton(ids,label=True,q=True)
        except RuntimeError:
            ids = cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection', select=True, q=True)
            hugePropsName = cmds.iconTextRadioButton(ids,label=True,q=True)
        allFileList = os.listdir(self.ServerRootPath+hugePropsName)
        fileName = sorted(allFileList)[-1]
        ServerRootPath = self.ServerRootPath+hugePropsName+'/'+fileName
        try:
            file = open(ServerRootPath, 'rb')
            informationMapList = json.load(file)
            file.close()
        except:
            informationMapList = []

        cmds.columnLayout(parent='mainScrollLayout',adjustableColumn=True)
        cmds.iconTextRadioCollection('keyIconTextRadioCollection')
        for i in range(len(informationMapList)):
            text=informationMapList[i]['name']+'  --->  '+informationMapList[i]['parent']
            cmds.iconTextRadioButton('%s:%s'%(informationMapList[i]['name'],informationMapList[i]['ctrlCircle']),collection='keyIconTextRadioCollection',
                                     style='iconAndTextHorizontal',l='%s'%text, width=340, height=25,onCommand=lambda *arg:self.selectObjects())
        cmds.setParent('..')

    def HugePropsListPlus(self):
        try:
            hugePropsList = os.listdir(self.ServerRootPath)
        except WindowsError:
            hugePropsList = []
        cmds.columnLayout(parent='hugePropsScrollLayout',adjustableColumn=True)
        cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection')
        self.id = -1
        for i in range(len(hugePropsList)):
            self.id += 1
            cmds.iconTextRadioButton('id%s_hugePropsIconTextRadioButton'%i,collection='hugePropsIconTextRadioCollection',
                                     style='iconAndTextHorizontal',l='%s'%hugePropsList[i], width=190, height=25,
                                     onCommand=lambda *arg:self.refreshPropsListPlus())
        cmds.setParent('..')

    def refreshHugePropsListPlus(self):
        #delete old
        layoutList = cmds.scrollLayout('hugePropsScrollLayout', childArray=True, q=True)
        if layoutList == None:
            pass
        else:
            for item in layoutList:
                cmds.deleteUI(item)

        #bulid new
        hugePropsList = os.listdir(self.ServerRootPath)
        cmds.columnLayout(parent='hugePropsScrollLayout',adjustableColumn=True)
        cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection')
        self.id = -1
        for i in range(len(hugePropsList)):
            self.id += 1
            cmds.iconTextRadioButton('id%s_hugePropsIconTextRadioButton'%i,collection='hugePropsIconTextRadioCollection',
                                     style='iconAndTextHorizontal',l='%s'%hugePropsList[i], width=190, height=25,
                                     onCommand=lambda *arg:self.refreshPropsListPlus())
        cmds.setParent('..')

    def refreshPropsListLocal(self):
        #delete old
        layoutList = cmds.scrollLayout('mainScrollLayout', childArray=True, q=True)
        if layoutList == None:
            pass
        else:
            for item in layoutList:
                cmds.deleteUI(item)

        #build new
        hugePropsName = cmds.textField('Huge_Props_Name',text=True,q=True)
        if hugePropsName == '':
            ids = cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection', select=True, q=True)
            hugePropsName = cmds.iconTextRadioButton(ids,label=True,q=True)
        allFileList = os.listdir(self.LocalRootPath+hugePropsName)
        fileName = sorted(allFileList)[-1]
        LocalRootPath = self.LocalRootPath+hugePropsName+'/'+fileName
        try:
            file = open(LocalRootPath, 'rb')
            informationMapList = json.load(file)
            file.close()
        except:
            informationMapList = []

        cmds.columnLayout(parent='mainScrollLayout',adjustableColumn=True)
        cmds.iconTextRadioCollection('keyIconTextRadioCollection')
        for i in range(len(informationMapList)):
            text=informationMapList[i]['name']+'  --->  '+informationMapList[i]['parent']
            cmds.iconTextRadioButton('%s:%s'%(informationMapList[i]['name'],informationMapList[i]['ctrlCircle']),collection='keyIconTextRadioCollection',
                                     style='iconAndTextHorizontal',l='%s'%text, width=340, height=25,onCommand=lambda *arg:self.selectObjects())
        cmds.setParent('..')

    def selectObjects(self):
        ids = cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection', select=True, q=True)
        hugePropsName = cmds.iconTextRadioButton(ids,label=True,q=True)
        allFileList = os.listdir(self.ServerRootPath+hugePropsName)
        fileName = sorted(allFileList)[-1]
        ServerRootPath = self.ServerRootPath+hugePropsName+'/'+fileName
        try:
            file = open(ServerRootPath, 'rb')
            informationMapList = json.load(file)
            file.close()
        except:
            informationMapList = []

        ctrlCircleParts = cmds.iconTextRadioCollection('keyIconTextRadioCollection', select=True, q=True).split(':')
        ctrlCircle = ctrlCircleParts[1]+':'+ctrlCircleParts[2]
        for item in informationMapList:
            if item['ctrlCircle'] == ctrlCircle:
                try:
                    namespace = self.getNamespace()
                    cmds.select(item['ctrlCircle'],namespace+item['parent'])
                except ValueError:
                    cmds.select(None)
        pass

    def selectObjectsCheck(self,list):
        if len(list) == 0:
            cmds.confirmDialog(title='Oops', message=u'请先选择物体！', button=u'继续')
            sys.exit()
        for i in range(len(list)-1):
            print i
            if not cmds.objectType(list[i], isType='transform' ):
                print i,list[i],cmds.objectType(list[i])
                cmds.confirmDialog(title='Oops', message=u'你可能错选了物体，请检查一下！', button=u'继续')
                sys.exit()
        if not cmds.objectType(list[-1], isType='assemblyReference' ):
            print cmds.objectType(list[-1], isType='assemblyReference' ),'er'
            cmds.confirmDialog(title='Oops', message=u'你可能错选了物体，请检查一下！', button=u'继续')
            sys.exit()

    def checkParent(self):
        ids = cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection', select=True, q=True)
        hugePropsName = cmds.iconTextRadioButton(ids,label=True,q=True)
        jsonFileList = os.listdir(self.ServerRootPath+hugePropsName)
        jsonFile = sorted(jsonFileList)[-1]
        openJsonFile = self.ServerRootPath+hugePropsName +'/'+jsonFile
        file = open(openJsonFile, 'rb')
        informationMapList = json.load(file)
        file.close()
        for propInfo in informationMapList:
            #print propInfo['parent']
            try:
                #~ 试着选择有命名空间的parent物体
                namespace = self.getNamespace()
                cmds.select(namespace+propInfo['parent'])
            except ValueError:
                try:
                    #~ 试着选择没有命名空间的问题
                    cmds.select(propInfo['parent'])
                except ValueError:
                    #~ 如果这两个物体都没有，说明这个物体不存在
                    cmds.iconTextRadioButton('%s:%s'%(propInfo['name'],propInfo['ctrlCircle']),backgroundColor=[0.8,0,0],e=True)
                    cmds.select(None)

    def add(self):
        tmp1_current_time = str(datetime.datetime.now())
        tmp2_current_time = tmp1_current_time.replace(' ', '_')
        tmp3_current_time = tmp2_current_time.split('.')
        tmp4_current_time = tmp3_current_time[0].split(':')
        current_time = tmp4_current_time[0]+'-'+tmp4_current_time[1]+'-'+tmp4_current_time[2]
        hugePropsName = cmds.textField('Huge_Props_Name',text=True,q=True)
        #如果hugePropsName有值得话，就上传hugePropsName的名字
        if hugePropsName == '':
            try:
                ids = cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection', select=True, q=True)
                hugePropsName = cmds.iconTextRadioButton(ids,label=True,q=True)
            except:
                cmds.confirmDialog( title=u'注意', message=u'请输入巨型道具名称', button=u'继续')
                sys.exit()
        fileName = hugePropsName +'_'+current_time+'.json'
        LocalRootPath = self.LocalRootPath+hugePropsName+'/'+fileName
        ServerRootPath = self.ServerRootPath+hugePropsName+'/'+fileName
        #试着打开本地上的文件
        try:
            file = open(LocalRootPath, 'rb')
            informationMapList = json.load(file)
            file.close()
        except IOError:
            #本地上没有这个实时文件名的文件，就打开时期最近的文件
            try:
                allFileList = os.listdir(self.LocalRootPath+hugePropsName)
                fileName = sorted(allFileList)[-1]
                LocalRootPath = self.LocalRootPath+hugePropsName+'/'+fileName
                file = open(LocalRootPath, 'rb')
                informationMapList = json.load(file)
                file.close()
            except WindowsError:
                try:
                    allFileList = os.listdir(self.ServerRootPath+hugePropsName)
                    fileName = sorted(allFileList)[-1]
                    ServerRootPath = self.ServerRootPath+hugePropsName+'/'+fileName
                    file = open(ServerRootPath, 'rb')
                    informationMapList = json.load(file)
                    file.close()
                except WindowsError:
                    informationMapList = []
        objects = cmds.ls(selection=True)
        self.selectObjectsCheck(objects)
        #get group_model_GRP name
        allTransformNodes = cmds.ls(type='transform')
        allRootGroups = []
        for item in allTransformNodes:
            key = cmds.listRelatives(item,parent=True)
            if key == None:
                allRootGroups.append(item)
        keyMap = {}
        for i in range(len(objects)-1):
            for rootGroup in allRootGroups:
                allMembers = cmds.listRelatives(rootGroup,allDescendents=True)
                if objects[i] in allMembers:
                    for member in allMembers:
                        splitMember = member.split('_')
                        if splitMember[-1] == 'GRP' and splitMember[-2] == 'model':
                            keyMap[objects[i]] = member
                else:
                    pass

        #get object information
        for i in range(len(objects)-1):
            ctrlCircle = objects[i]
            parent = objects[-1].split(':')[-1]
            tx = cmds.getAttr(objects[i]+'.tx')
            ty = cmds.getAttr(objects[i]+'.ty')
            tz = cmds.getAttr(objects[i]+'.tz')
            rx = cmds.getAttr(objects[i]+'.rx')
            ry = cmds.getAttr(objects[i]+'.ry')
            rz = cmds.getAttr(objects[i]+'.rz')
            sx = cmds.getAttr(objects[i]+'.sx')
            sy = cmds.getAttr(objects[i]+'.sy')
            sz = cmds.getAttr(objects[i]+'.sz')
            offset = [tx,ty,tz,rx,ry,rz,sx,sy,sz]
            name = cmds.getAttr(keyMap[objects[i]]+'.treeName')
            tmpMap = {'name':name,'ctrlCircle':ctrlCircle,'parent':parent,'offset':offset}
            flag = True
            if not informationMapList == []:
                for test in informationMapList:
                    if test['ctrlCircle'] == ctrlCircle:
                        flag = False
                if flag == True:
                    informationMapList.append(tmpMap)
            else:
                informationMapList.append(tmpMap)

        if os.path.isdir(self.LocalRootPath+hugePropsName):
            pass
        else:
            os.makedirs(self.LocalRootPath+hugePropsName)
        file = open(LocalRootPath, 'wb')
        json.dump(informationMapList, file,indent=2)
        file.close()
        self.refreshPropsListLocal()
        ifNew = cmds.textField('Huge_Props_Name',text=True,q=True)
        if ifNew != '':
            self.Upload()
            self.refresh()
        cmds.textField('Huge_Props_Name',text='',e=True)

    def remove(self):
        tmp1_current_time = str(datetime.datetime.now())
        tmp2_current_time = tmp1_current_time.replace(' ', '_')
        tmp3_current_time = tmp2_current_time.split('.')
        tmp4_current_time = tmp3_current_time[0].split(':')
        current_time = tmp4_current_time[0]+'-'+tmp4_current_time[1]+'-'+tmp4_current_time[2]
        ids = cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection', select=True, q=True)
        try:
            hugePropsName = cmds.iconTextRadioButton(ids,label=True,q=True)
        except RuntimeError:
            return
        fileName = hugePropsName +'_'+current_time+'.json'
        LocalRootPath = self.LocalRootPath+hugePropsName+'/'+fileName
        ServerRootPath = self.ServerRootPath+hugePropsName+'/'+fileName
        try:
            file = open(ServerRootPath, 'rb')
            informationMapList = json.load(file)
            file.close()
        except IOError:
            allFileList = os.listdir(self.ServerRootPath+hugePropsName)
            fileName = sorted(allFileList)[-1]
            ServerRootPath = self.ServerRootPath+hugePropsName+'/'+fileName
            file = open(ServerRootPath, 'rb')
            informationMapList = json.load(file)
            file.close()
        try:
            parts = cmds.iconTextRadioCollection('keyIconTextRadioCollection', select=True, q=True).split(':')
            ctrlCircle = parts[1]+':'+parts[2]
        except IndexError:
            return
        for info in informationMapList:
            if info['ctrlCircle'] == ctrlCircle:
                informationMapList.remove(info)
        for info in informationMapList:
            print info['name']

        file = open(LocalRootPath, 'wb')
        json.dump(informationMapList, file)
        file.close()

        uploadTime = time.time()
        if not os.path.isdir(os.path.dirname(self.ServerRootPath+hugePropsName)):
            lsrunas.lsrunas().CreateDir_1(os.path.dirname(self.ServerRootPath+hugePropsName))
        lsrunas.lsrunas(uploadTime).UploadFile(LocalRootPath, ServerRootPath, DeleteSource=False)

        self.refreshPropsList()

    def Upload(self):
        hugePropsName = cmds.textField('Huge_Props_Name',text=True,q=True)
        if hugePropsName == '':
            ids = cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection', select=True, q=True)
            hugePropsName = cmds.iconTextRadioButton(ids,label=True,q=True)
        allFileList = os.listdir(self.LocalRootPath+hugePropsName)
        fileName = sorted(allFileList)[-1]
        ServerRootPath = self.ServerRootPath+hugePropsName+'/'+fileName
        LocalRootPath = self.LocalRootPath+hugePropsName+'/'+fileName

        uploadTime = time.time()
        if not os.path.isdir(os.path.dirname(self.ServerRootPath+hugePropsName)):
            lsrunas.lsrunas().CreateDir_1(os.path.dirname(self.ServerRootPath+hugePropsName))
        lsrunas.lsrunas(uploadTime).UploadFile(LocalRootPath, ServerRootPath, DeleteSource=False)
        flag = cmds.textField('Huge_Props_Name',text=True,q=True)
        if flag != '':
            self.refreshHugePropsListPlus()
        cmds.confirmDialog(title=u'完成', message=u'上传完成！', button=u'好的')

    def Download(self):
        yourChoice = cmds.confirmDialog(title=u'文件类型', message=u'请选择要加载的文件类型',
                                        button=['Rig_high','Rig_low'], defaultButton='Rig_high')
        if yourChoice == 'Rig_low':
            fileType = '_rig_low.ma'
        else:
            fileType = '_rig_high.ma'

        ids = cmds.iconTextRadioCollection('hugePropsIconTextRadioCollection', select=True, q=True)
        hugePropsName = cmds.iconTextRadioButton(ids,label=True,q=True)
        jsonFileList = os.listdir(self.ServerRootPath+hugePropsName)
        jsonFile = sorted(jsonFileList)[-1]
        openJsonFile = self.ServerRootPath+hugePropsName +'/'+jsonFile
        file = open(openJsonFile, 'rb')
        informationMapList = json.load(file)
        file.close()
        ctrlCircleList = []
        for hugePropsInfo in informationMapList:
            ctrlCircleList.append(hugePropsInfo['ctrlCircle'])
        keyCtrlCircleList = []
        for ctrlCircle in ctrlCircleList:
            checkValue = cmds.checkBox('id_%s'%ctrlCircle,value=True,q=True)
            if checkValue == True:
                keyCtrlCircleList.append(ctrlCircle)
        for ctrlCircle in keyCtrlCircleList:
            for item in informationMapList:
                if item['ctrlCircle'] == ctrlCircle:
                    name = item['name']
                    nameSpace =item['ctrlCircle'].split(':')[0]
            openPropsFile = self.ServerFilePath+name+'/p_'+name+fileType
            cmds.file(openPropsFile,reference=True, namespace=nameSpace,mergeNamespacesOnClash=False);
            for info in informationMapList:
                if info['ctrlCircle'] == ctrlCircle:
                    cmds.setAttr(info['ctrlCircle']+'.tx',info['offset'][0])
                    cmds.setAttr(info['ctrlCircle']+'.ty',info['offset'][1])
                    cmds.setAttr(info['ctrlCircle']+'.tz',info['offset'][2])
                    cmds.setAttr(info['ctrlCircle']+'.rx',info['offset'][3])
                    cmds.setAttr(info['ctrlCircle']+'.ry',info['offset'][4])
                    cmds.setAttr(info['ctrlCircle']+'.rz',info['offset'][5])
                    cmds.setAttr(info['ctrlCircle']+'.sx',info['offset'][6])
                    cmds.setAttr(info['ctrlCircle']+'.sy',info['offset'][7])
                    cmds.setAttr(info['ctrlCircle']+'.sz',info['offset'][8])
                    try:
                        parent = self.getNamespace() + info['parent']
                        parentPos = info['ctrlCircle']
                        ADPosGroup = cmds.group(empty=True,name=(parentPos+"_pos"),parent=parent)
                        cmds.parentConstraint(ADPosGroup,info['ctrlCircle'],maintainOffset=True,name='%s_parentConstraint'%info['ctrlCircle'])
                    except:
                        cmds.confirmDialog( title='ERRO', message='This object  %s  is not exsit!'%item['parent'], button='Gotcha')

    def getNamespace(self):
        filename = cmds.file(sceneName=True,q=True).split('/')[-2] #~ 获取文件的上层文件夹名字，就是镜头名
        namespace = filename + '_AD_AR_NS:' #~ namespace的名字
        return namespace

if __name__ == '__main__':
    AutoARsPropsLoader()
