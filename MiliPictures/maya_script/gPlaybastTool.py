#coding:utf-8
__author__ = 'edit by kai.xu'

import maya.cmds  as cmds
import maya.OpenMaya as OpenMaya
import maya.OpenMayaUI as OpenMayaUI
import os,sys,shutil,time,datetime
import Mili.Module.utilties.lsrunas as lsrunas
import Mili.Module.utilties.Get as Get
import Mili.Module.utilties.Path as Path
import Mili.Module.logInfo.logSubmitter as logSubmitter
import Mili.Module.utilties.ProjectConfig as ProjectConfig
import Mili.Module.logInfo.infoReader as infoReader
import Mili.Module.animation.playblast as playblast
reload(playblast)
reload(infoReader)
reload(ProjectConfig)
reload(logSubmitter)
reload(lsrunas)
reload(Get)
reload(Path)


class gPlaybastTool:
    def __init__(self):
        Prj = Get.Get().GetActiveProject()
        LocalRootPath = Path.Path().GetLocalPath()+'/'+Prj+'/' #~ 本地的根地址
        ServerRootPath = Path.Path().GetServerPath()+'/'       #~ 服务器地址
        tmp_current_time = str(datetime.datetime.now())
        self.date = tmp_current_time.split(' ')[0]
        self.filename = cmds.file(q=True,expandName=True).split('/')[-1]
        self.movPath = ''
        self.originalFilePath = cmds.file(q=True,expandName=True)                  #~ 原始maya文件地址
        self.LocalFilePath = LocalRootPath + "Backup/"                             #~ 本地拷贝的maya文件的路径，用于lsrunas上传类
        """遍历出服务器maya文件地址"""
        parts = self.originalFilePath.split('/')
        for i in range(2,len(parts)-1):
            ServerRootPath += parts[i]
            ServerRootPath += '/'
        self.ServerBackupPath = ServerRootPath+'Playblast/'+self.date+'/'            #~ 服务器maya文件路径，用于lsrunas上传类
        self.LocalPlayblastPath = self.movPath                                       #~ 本地拍屏文件路径，用于lsrunas上传类
        self.ServerRootPath = ProjectConfig.ProjectConfig().get('PlayblastEditRoot', #~ 服务器剪辑目录路径，用于lsrunas上传类
                                                                default='//hnas01/Data/Edit/%s/AutoImport/'%Prj)

    def Show(self):
        Window = 'SaveAsTool'
        if cmds.window(Window,exists = True) == True:
            cmds.deleteUI(Window,window = True)
        if cmds.windowPref(Window,exists = True) == True:
            cmds.windowPref(Window,remove = True)
        cmds.window(Window,sizeable = False,title = 'Playblast Tool')
        ColumnLayout = cmds.columnLayout(parent = Window,adjustableColumn = True)
        cmds.text(label='Playbast Tools',height=30, backgroundColor=[0.5, 0.5, 0.6])
        cmds.separator(style='out')
        cmds.frameLayout(label='Description :', collapsable=True, collapse=True,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.scrollField('descriptionScrollField',height=100,wordWrap=True)
        cmds.separator(style='in')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.rowColumnLayout( numberOfColumns=3, columnWidth=[(1, 50),(2,310),(3,50)])
        ShotT = cmds.text(height = 24,label = 'Mov Path:')
        ShotTF = cmds.textField(height = 24)
        browseB = cmds.button(height = 24,label = 'Browse',command = lambda *args: self.ChooseFolder())
        cmds.setParent('..')
        cmds.columnLayout(adjustableColumn=True)
        qualitySlider = cmds.intSliderGrp( field=True, label='Quality', value=50 )
        ScaleSlider = cmds.floatSliderGrp( label='Scale', field=True, minValue=0.10, maxValue=1.00, fieldMinValue=0.10, fieldMaxValue=1.00, value=0.50,precision=2 )
        playB = cmds.button(height = 32,label = 'Playblast',command = lambda *args: self.Playblastfunction())
        openB = cmds.button(height = 32,label = 'Open Mov',command = lambda *args: self.openMov())
        uploadB = cmds.button(height = 32,label = 'Upload Mov',command = lambda *args: self.uploadTheFile())
        cmds.setParent('..')
        cmds.separator(style='in')
        cmds.text(label=' ')

        self.ShotTF = ShotTF
        self.qualitySlider = qualitySlider
        self.ScaleSlider = ScaleSlider
        self.playB = playB
        self.uploadB = uploadB
        self.openB = openB
        cmds.showWindow(Window)
        self.SetDefaultFolder()
        self.setOpenB()
        self.setUploadB(False)
    def SetDefaultFolder(self):
        ShotTF = self.ShotTF
        Folder = cmds.workspace(query = True, directory = True)
        cmds.textField(ShotTF,edit = True,text = Folder)

    def ChooseFolder(self):
        ShotTF = self.ShotTF
        StartFolder = cmds.workspace(query = True, directory = True)
        Folder = cmds.fileDialog2( dialogStyle=2, startingDirectory = StartFolder,fileMode=3)
        if not Folder == None:
            cmds.textField(ShotTF,edit = True,text = Folder[0])

    def GetFolder(self):
        ShotTF = self.ShotTF
        Folder = cmds.textField(ShotTF,query = True,text = True)
        if Folder[-1] == '/':
            return Folder
        else:
            return (Folder+'/')

    def GetQuality(self):
        qualitySlider = self.qualitySlider
        quality = cmds.intSliderGrp(qualitySlider,q=1,value=1)
        return quality

    def GetScale(self):
        ScaleSlider = self.ScaleSlider
        scale = cmds.floatSliderGrp(ScaleSlider,q=1,value=1)
        return scale

    def setOpenB(self):
        openB = self.openB
        if self.movPath == '':
            cmds.button(openB,edit=1,enable=0)
        else:
            cmds.button(openB,edit=1,enable=1)

    def setUploadB(self,flag):
        uploadB = self.uploadB
        cmds.button(uploadB,enable=flag,edit=True)
    def GetCurrentCam(self):
        view = OpenMayaUI.M3dView.active3dView()
        camDag = OpenMaya.MDagPath()
        view.getCamera(camDag)
        return camDag.fullPathName()

    def openMov(self):
        movPath = self.movPath
        print movPath
        os.system(movPath)

    def Playblastfunction(self):
        self.showHUD()
        folder = self.GetFolder()
        qualityNum = self.GetQuality()
        scale = self.GetScale()
        camShape = self.GetCurrentCam().split('|')[-1]
        camTrans = cmds.listRelatives( camShape,parent=1)[0].replace(':','_')
        judgeArg = cmds.camera(camShape,q=1,overscan=1)
        cmds.camera(camShape,edit=1,overscan=1.0)
        movPath = cmds.playblast(format ='avi',filename = folder+camTrans+'.avi',forceOverwrite=1,sequenceTime=0,clearCache=1,viewer=0,showOrnaments=1,offScreen=1,fp=4,percent =scale*100, compression='none',quality=qualityNum,w=cmds.getAttr('defaultResolution.width'),h=cmds.getAttr('defaultResolution.height'))
        self.setUploadB(True)
        if judgeArg == 1.3 :
            cmds.camera(camShape,edit=1,overscan=1.3)
        self.movPath = movPath
        self.setOpenB()
        self.LocalPlayblastPath = folder      #~ 拍屏文件本地路径
        self.playblastFile = camTrans+'.avi'  #~ 拍屏文件名称
        #self.openMov() #~ 播放截屏
        #self.uploadTheFile() #~ 上传文件

    def uploadTheFile(self):
        if os.path.isdir(self.LocalFilePath):
            pass
        else:
            os.makedirs(self.LocalFilePath)
        if cmds.file(q=True,sceneName=True) == '':
            #~ 判断是否为空文件
            cmds.confirmDialog(title='Oops', message='Plsase open your file!', button='Get it')
            sys.exit()

        yourChoice = cmds.confirmDialog(title=u'上传文件', message=u'是否选择提交拍屏 ?', button=[u'提交',u'不要'], defaultButton=u'提交', cancelButton=u'不要', dismissString=u'不要' )
        if yourChoice == u'提交':
            self.ServerPlayblastPath = self.submit() #~ 登记信息
            uploadTime = time.time()
            """copy文件到项目的Backup文件夹"""
            shutil.copyfile(self.originalFilePath,self.LocalFilePath+self.filename)
            """上传maya文件"""
            if not os.path.isdir(os.path.dirname(self.ServerBackupPath)):
                lsrunas.lsrunas().CreateDir_1(os.path.dirname(self.ServerBackupPath))
            lsrunas.lsrunas(uploadTime).UploadFile(self.LocalFilePath+self.filename,self.ServerBackupPath+self.filename,DeleteSource=False)
            """上传拍屏文件到剪辑目录"""
            if not os.path.isdir(os.path.dirname(self.ServerPlayblastPath)):
                lsrunas.lsrunas().CreateDir_1(os.path.dirname(self.ServerPlayblastPath))
            lsrunas.lsrunas(uploadTime).UploadFile(self.LocalPlayblastPath+self.playblastFile, self.ServerPlayblastPath+self.playblastFile, DeleteSource=False)
            """上传拍屏文件到项目目录"""
            if not os.path.isdir(os.path.dirname(self.ServerBackupPath)):
                lsrunas.lsrunas().CreateDir_1(os.path.dirname(self.ServerBackupPath))
            lsrunas.lsrunas(uploadTime).UploadFile(self.LocalPlayblastPath+self.playblastFile, self.ServerBackupPath+self.playblastFile, DeleteSource=False)
        else:
            print 'Wise choice'
        cmds.confirmDialog(title='Job Done', message='The video has uploaded to the server', button='Get it!')

    def submit(self):
        '''
        登记信息
        '''
        assetName = self.filename.split('_')[0] + '_' + self.filename.split('_')[1]
        assetCatagory = 'Shot/%s'%self.filename.split('_')[0]
        key = self.filename.split('_')[2]
        if key == 'lay':
            workNode = 'layout_animation'
        elif key == 'blocking':
            workNode = 'blocking'
        elif key == 'ani':
            workNode = 'animation'
        else:
            pass
        """获取版本信息"""
        logInfoList = infoReader.infoReader().readItem(assetCatagory,assetName,fullHistory=1)
        allVersions = []
        for logInfo in logInfoList:
            if logInfo['variant'] == 'playblast':
                version = logInfo['version']
                allVersions.append(version)
        version = max(allVersions)
        version += 1
        ServerPlayblastPath = self.ServerRootPath+"%s/%s/%s/"%(workNode,assetName,self.date)

        logSubmitter.logSubmitter().submit('pub', assetName,assetCatagory, workNode,
                                         mainPubFile=ServerPlayblastPath+self.filename,
                                         pipelineDescription=u'上传拍屏的视频，文件有备份的哦',
                                         variant='playblast',
                                         version = version,
                                     )
        print assetCatagory,assetName
        return ServerPlayblastPath

    def showHUD(self):
        description = cmds.scrollField('descriptionScrollField',text=True,q=True)
        try:
            key = self.filename.split('_')[2]
        except IndexError:
            key = None
        if key == 'lay':
            type = 'Layout'
        elif key == 'blocking':
            type = 'Blocking'
        elif key == 'ani':
            type = 'Animation'
        else:
            type = 'None'
        pl = playblast.playblast()
        pl.setDescription(description)
        pl.setAniType(type)
        pl.AnimationHUD()
        pl.RefreshInfo()

if __name__ == '__main__':
    gPlaybastTool = gPlaybastTool()
    gPlaybastTool.Show()