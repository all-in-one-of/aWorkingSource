#coding:utf-8
__author__ = 'LanHua.Zhou,edit by kai.xu'
"""
ChangeName
@version: $Id$
@author: LanHua.Zhou,edit by kai.xu
"""
import getpass,time
from functools import partial
import maya.cmds as cmds
import maya.mel as mm
from Mili.Module.utilties import animHUD
import Mili.Module.utilties.Info as Info
import Mili.Module.utilties.Get as Get
import Mili.Module.logInfo.infoReader as infoReader
reload(infoReader)
reload(Get)
reload(Info)
reload(animHUD)
from PySide import QtCore
import maya.OpenMaya as OpenMaya
import maya.OpenMayaUI as OpenMayaUI
import Mili.Module.animation.PlayBlastConvert as PlayBlastConvert
reload(PlayBlastConvert)

class playblast(animHUD.animHUD):
    def __init__(self,*args, **kwargs):
        super(playblast, self).__init__(*args, **kwargs)
        self.z_HUDCompany = "None"
        self.z_HUDProject = "None"
        self.z_HUDAniType = "None"
        self.z_HUDScene = "None"
        self.z_HUDCut = "None"
        self.z_HUDCameraSize = "None"
        self.z_HUDCameraFocal = "None"
        self.description = None

    def RefreshInfo(self):
        self.z_HUDCompany = "MiLi Pictures"
        self.z_HUDProject = Get.Get().GetActiveProject() 
        if self.z_HUDAniType:
            pass
        else:
            self.z_HUDAniType = "Animation"
        try:
            fileName = cmds.file(q=True,sn=True,shn=True)
            #get camera
            if fileName:
                sp = fileName.split("_")
                if len(sp) >= 3:
                    self.z_HUDScene = sp[0]
                    self.z_HUDCut = "CUT" + sp[1]
                else:
                    self.z_HUDScene = "None"
                    self.z_HUDCut = "None"
            else:
                self.z_HUDScene = "None"
                self.z_HUDCut = "None"
        except:
            self.z_HUDScene = "None"
            self.z_HUDCut = "None"
        self.z_HUDCameraSize = "2048x1152"
        
        self.z_HUDCameraFocal = self.getCurrentCamera()
        if self.z_HUDCameraFocal:
            pass
        else:
            self.z_HUDCameraFocal = '55'
        
        try:
            cmds.headsUpDisplay('MLHUD_Company', r = True)
            cmds.headsUpDisplay('MLHUD_Project', r = True)
            cmds.headsUpDisplay('MLHUD_CUT', r = True)
            cmds.headsUpDisplay('MLHUD_CameraInfo', r = True)
            cmds.headsUpDisplay("MLHUD_FrameInfo", r = True)
            cmds.headsUpDisplay("MLHUD_User", r = True)
            cmds.headsUpDisplay("MLHUD_Timer", r = True)
            cmds.headsUpDisplay("MLHUD_Description", r = True)
        except:
            pass

    def getCurrentCamera(self):
        try:
            '''
            CurrentPanel = cmds.getPanel(withFocus=1)
            if cmds.getPanel(to=CurrentPanel) != 'modelPanel':
                visPanel = cmds.getPanel(vis=1)
                for name in visPanel:
                    modelPanels = cmds.getPanel(type='modelPanel')
                    if cmds.getPanel(to=name) == 'modelPanel':
                        cmds.setFocus(name)
                        CurrentPanel = name
                        break
            camera = cmds.modelEditor(CurrentPanel, q = True, camera = True)
            if cmds.nodeType(camera) == "camera":
                return '%.3f'%cmds.getAttr("%s.focalLength"%camera)
            else:
                cameraShape = cmds.listRelatives(camera, shapes = True)
                if cameraShape:
                    return '%.3f'%cmds.getAttr("%s.focalLength"%cameraShape[0])
            '''
            view = OpenMayaUI.M3dView.active3dView()
            camDag = OpenMaya.MDagPath()

            view.getCamera(camDag)
            camera = camDag.fullPathName()
            return '%.3f'%cmds.getAttr("%s.focalLength"%camera)
        except:
            return False

        pass
        #getCurrentCamera()


    def setAniType(self, aniType = None):
        if aniType:
            self.z_HUDAniType = aniType
        else:
            self.z_HUDAniType = "Animation"
        return self.z_HUDAniType

    def FrameCounter(self, value = None):
        z_StartFrame = int(cmds.playbackOptions(q = True, min = True))
        z_EndFrame = int(cmds.playbackOptions(q = True, max = True))  
        z_CurrentFrame = int(cmds.currentTime(q = True))
        return "%s-%s"%(z_StartFrame,z_EndFrame) + "|" + "%s"%z_CurrentFrame

    def GetUser(self, value = None):
        #get User
        import socket
        hostName = getpass.getuser()    
        #time = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
        #return "%s"%hostName + "|" + "%s"%time
        return "%s"%hostName

    def GetTimer(self, value = None):
        import time
        time = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
        return "%s"%time

    def GetCompany(self, value = None):
        return self.z_HUDCompany

    def GetProject(self, value = None):
        return "%s"%self.z_HUDProject + "|" + "%s"%self.z_HUDAniType

    def GetAnimType(self, value = None):
        return self.z_HUDAniType

    def GetScene(self, value = None):
        return self.z_HUDScene

    def GetCut(self, value = None):
        try:
            fileName = cmds.file(q=True,sn=True,shn=True)
            #get camera
            if fileName:
                sp = fileName.split("_")
                if len(sp) >= 3:
                    self.z_HUDScene = sp[0]
                    self.z_HUDCut = "CUT" + sp[1]
                else:
                    self.z_HUDScene = "None"
                    self.z_HUDCut = "None"
            else:
                self.z_HUDScene = "None"
                self.z_HUDCut = "None"
        except:
            self.z_HUDScene = "None"
            self.z_HUDCut = "None"
        version = self.getVersion()
        return "%s"%self.z_HUDScene + "|" + "%s"%self.z_HUDCut + ' v%s'%version
        #return "Bloody World!"

    def GetCameraSize(self, value = None):
        self.z_HUDCameraFocal = self.getCurrentCamera()
        if self.z_HUDCameraFocal:
            pass
        else:
            self.z_HUDCameraFocal = '55'
        return "%s"%self.z_HUDCameraSize + "|" + "%s"%self.z_HUDCameraFocal

    def GetCameraFocal(self, value = None):
        return self.z_HUDCameraFocal

    def AnimationHUD(self):
        #mili heads up display sets
        self.RemoveHUD()
        #----- Company Name ------ 
        #left top(section 0, block 0)
        cmds.headsUpDisplay('MLHUD_Company', s = 0, b = 0, ba = "left", lfs = 'large', dfs = 'large', blockSize = 'small',c = partial(self.GetCompany),attachToRefresh=True)
        #----- Project Name -----
        #left bottom(section 5, block 0)
        cmds.headsUpDisplay('MLHUD_Project', s = 0, b = 1, ba = "center", lfs = 'large', dfs = 'large', blockSize = 'small',c = partial(self.GetProject),attachToRefresh=True)
        #----- Scene Name And Cut Name -----
        #left bottom(section 6, block 0)
        #cmds.headsUpDisplay('MLHUD_SceneCut', s = 6, b = 0, ba = "left",l = "%s"%self.z_HUDScene + " | " + "%s"%self.z_HUDCut, lfs = 'large', dfs = 'large', blockSize = 'small')
        cmds.headsUpDisplay('MLHUD_CUT',s=2 ,b=1 ,ba = "center",lfs='large',dfs='large',blockSize='small', c = partial(self.GetCut),attachToRefresh=True)
        #----- Camera Information -----
        #left bottom(section 7, block 0)GetCameraSize
        cmds.headsUpDisplay('MLHUD_CameraInfo', s = 4, b = 0, ba = "right", lfs = 'large', dfs = 'large', blockSize = 'small', c = partial(self.GetCameraSize),attachToRefresh=True)
        #----- Frame -----
        #left bottom(section 8, block 0)
        z_StartFrame = int(cmds.playbackOptions(q = True, min = True))
        z_EndFrame = int(cmds.playbackOptions(q = True, max = True))
        cmds.headsUpDisplay('MLHUD_FrameInfo',s = 4, b = 1, ba = "right", lfs = 'large', dfs = 'large', blockSize = 'small',c = partial(self.FrameCounter),attachToRefresh=True)
        #----- User -----
        #left bottom(section 8, block 0)
        cmds.headsUpDisplay('MLHUD_User',s=5,b=0,ba = "left",lfs='large',dfs='large',blockSize='small',c = partial(self.GetUser),ev = "timeChanged")

        cmds.headsUpDisplay('MLHUD_Timer',s=9,b=0,ba = "right",lfs='large',dfs='large',blockSize='small',c = partial(self.GetTimer),attachToRefresh=True)
        if self.description != None:
            self.displayDescription()

        self.HUDColor(17)

    def displayDescription(self):
        '''
        这个函数实现headsUpDisplay命令的多行显示
        '''
        textList = self.description.split('\n')
        for i in range(len(textList)):
            line = len(textList) - i
            text = textList[i]
            try:
                cmds.headsUpDisplay('MLHUD_Description_ID%s'%i,r=True)
            except:
                pass
            cmds.headsUpDisplay('MLHUD_Description_ID%s'%i,s=5,b=line,ba = "left",lfs='large',dfs='large',blockSize='small',label=text,c=lambda:'',attachToRefresh=True)

    def DisplayMode(self):
        displayKeys=['nc','pl','lt','ca','joints','ikh','df','ha','follicles','hairSystems','strokes','motionTrails','dimensions','locators']
        #Display smoothness low
        modelPanels=cmds.getPanel(typ="modelPanel")
        for currentPanel in modelPanels:
            for item in displayKeys:
                eval("cmds.modelEditor(\'"+currentPanel+"\',e=True,"+item+"=0)")

    def playblast(self, path):
        mm.eval("DisplayShaded;")
        self.DisplayMode()

        #get playStart
        import Mili.Module.utilties.ProjectConfig as ProjectConfig
        reload(ProjectConfig)
        config = ProjectConfig.ProjectConfig()

        aniCacheStart = config.get('AniStartFix', default=-8)
        aniCacheEnd = config.get('AniEndFix', default=8)

        #playblast = config.get('AniPlayblastStart', default=92)

        StartFrame = 100 + int(aniCacheStart)
        EndFrame = int(cmds.playbackOptions(q = True, max = True)) + int(aniCacheEnd)
        #cmds.playblast(startTime = StartFrame, endTime = EndFrame, filename = path, format = "avi", forceOverwrite = True, sequenceTime = 0, showOrnaments = 1, clearCache = 1, offScreen = True, percent = 100, quality = 100, widthHeight = [1024,576], viewer = 0)
        PlayBlastConvert.PlayBlastConvert(timeRange=(StartFrame,EndFrame),moviePath = path,offLine = True).getOutVideo()   # returns a list: [aviPath, movPath]

    def setDescription(self,description=None):
        self.description = description

    def getDescription(self):
        return self.description

    def getVersion(self):
        self.filename = cmds.file(q=True,expandName=True).split('/')[-1]
        assetName = self.filename.split('_')[0] + '_' + self.filename.split('_')[1]
        assetCatagory = 'Shot/%s'%self.filename.split('_')[0]
        logInfoList = infoReader.infoReader().readItem(assetCatagory,assetName,fullHistory=1)
        allVersions = []
        for logInfo in logInfoList:
            if logInfo['variant'] == 'playblast':
                version = logInfo['version']
                allVersions.append(version)
        version = max(allVersions)
        return version