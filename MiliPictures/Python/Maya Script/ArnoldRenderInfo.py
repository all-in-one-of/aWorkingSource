#coding:utf-8
__author__ = 'kai.xu'

import sys,shiboken,os,time
import maya.cmds as cmds
import maya.mel as mel
import maya.OpenMayaUI as omui
from PySide import QtCore,QtGui

class theDialog(QtGui.QDialog):
    def __init__(self,parent = None):
        super(theDialog,self).__init__()
        self.setParent(parent)
    def showWindows(self,title="Mili Tools",content="Hello World!",plug=None):
        self.setModal(True)
        self.setWindowTitle(title)
        title_layout = QtGui.QVBoxLayout()
        title = QtGui.QLabel()
        title.setText(content)
        font = QtGui.QFont()
        font.setPixelSize(16)
        title.setFont(font)
        title.setAlignment(QtCore.Qt.AlignHCenter)
        title_layout.addStretch()
        title_layout.addWidget(title)
        title_layout.addStretch()
        button_layout = QtGui.QHBoxLayout()
        flags = QtCore.Qt.WindowFlags()
        flags |= QtCore.Qt.Tool
        flags |= QtCore.Qt.WindowStaysOnTopHint
        if plug == "Confirm":
            button = QtGui.QPushButton()
            button.setText("Confirm")
            button.clicked.connect(self.close)
            button.setFixedHeight(30)
            button_layout.addStretch()
            button_layout.addWidget(button)
            button_layout.addStretch()
        elif plug == "Choice":
            button1 = QtGui.QPushButton()
            button1.clicked.connect(lambda *args:self.yes())
            button2 = QtGui.QPushButton()
            button2.clicked.connect(lambda *args:self.noo())
            button1.setText("Yes")
            button2.setText("Noo")
            button1.setFixedHeight(30)
            button2.setFixedHeight(30)
            button_layout.addStretch()
            button_layout.addWidget(button1)
            button_layout.addWidget(button2)
            button_layout.addStretch()
        else:
            flags |= QtCore.Qt.FramelessWindowHint
        mainLayout = QtGui.QVBoxLayout()
        mainLayout.addLayout(title_layout)
        mainLayout.addLayout(button_layout)
        self.setLayout(mainLayout)
        self.setWindowFlags(flags)
        self.setFixedSize(300,150)
        self.show()
        if plug == None:
            self.timer= QtCore.QTimer()
            self.timer.timeout.connect(lambda *args: self.autoClose())
            self.timer.start(3000)
        self.exec_()

    def yes(self):
        self.value = True
        self.close()
    def noo(self):
        self.value = False
        self.close()
    def autoClose(self):
        self.timer.stop()
        self.close()

class theDockWidget(QtGui.QDockWidget):
    signal = QtCore.Signal()
    def __init__(self):
        super(theDockWidget,self).__init__()
    def closeEvent(self,event):
        self.signal.emit()

class ArnoldRenderInfo(object):
    def __init__(self):
        self.renderInfo = {}#储存渲染参数的字典
        self.getInformation()
        self.showWindows()
        self.width = self.showGui.width()
        self.height = self.showGui.height()

    def getInformation(self):

        #get render information
        ImageFormat = mel.eval("getImfImageType();")
        CameraAA = cmds.getAttr("defaultArnoldRenderOptions.AA_samples")
        Diffuse = cmds.getAttr("defaultArnoldRenderOptions.GI_diffuse_samples")
        Glossy = cmds.getAttr("defaultArnoldRenderOptions.GI_glossy_samples")
        Refraction = cmds.getAttr("defaultArnoldRenderOptions.GI_refraction_samples")
        SSS = cmds.getAttr("defaultArnoldRenderOptions.sssBssrdfSamples")
        VolumeIndirect = cmds.getAttr("defaultArnoldRenderOptions.volumeIndirectSamples")
        MotionBlurEnable = cmds.getAttr("defaultArnoldRenderOptions.motion_blur_enable")

        #get AOVs`s information
        ObjectID = 0
        LightID = 0
        AOVsList = cmds.ls(type='aiAOV')
        if AOVsList == None or AOVsList == []:
            CommontAOVs = 'None'
            ObjectID = 0
            LightID = 0
        else:
            CommontAOVs = 'Off'
            for AOV in AOVsList:
                AOVname = cmds.getAttr(AOV+'.name')
                key = AOVname.split('_')[0]
                if key == 'Depth' or key == 'OCC':
                    CommontAOVs = 'On'
                if key == 'ID':
                    ObjectID += 1
                elif key == 'light':
                    LightID += 1
                else:
                    pass
        self.renderInfo['CommontAOVs'] = CommontAOVs
        self.renderInfo['ObjectID'] = ObjectID
        self.renderInfo['LightID'] = LightID

        self.renderInfo['ImageFormat'] = ImageFormat
        self.renderInfo['CameraAA'] = CameraAA
        self.renderInfo['Diffuse'] = Diffuse
        self.renderInfo['Glossy'] = Glossy
        self.renderInfo['Refraction'] = Refraction
        self.renderInfo['SSS'] = SSS
        self.renderInfo['VolumeIndirect'] = VolumeIndirect
        self.renderInfo['MotionBlurEnable'] = MotionBlurEnable

    def isTD(self):
        import Mili.Module.utilties.MiliUser as MiliUser
        reload(MiliUser)
        self.department = MiliUser.MiliUser()['department']
        self.appointment = MiliUser.MiliUser()['appointment']
        if self.department == "PipelineTD" or self.department == "RnD" or self.appointment == "TD":
            return True
        else:
            return False

    def setColor(self):
        VolumeIndirectValueT = self.VolumeIndirectValueT
        palette_red = QtGui.QPalette()
        palette_red.setColor(QtGui.QPalette.Background,QtGui.QColor(0.8*255,0.2*255,0.2*255))
        palette_green = QtGui.QPalette()
        palette_green.setColor(QtGui.QPalette.Background,QtGui.QColor(0.3*255,0.8*255,0.4*255))
        palette_yellow = QtGui.QPalette()
        palette_yellow.setColor(QtGui.QPalette.Background,QtGui.QColor(0.9*255,0.8*255,0.1*255))
        ImageFormatPalette = palette_red
        CameraPalette = palette_green
        DiffusePalette = palette_green
        GlossyPalette = palette_green
        RefractionPalette = palette_green
        sssPalette = palette_green
        VolumeIndirectPalette = palette_green
        MotionBlurEnablePalette = palette_green
        commontAOVsPalette = palette_red
        objectIDPalette = palette_green
        lightIDPalette = palette_green

        """set groupBox widget`s color to get user`s attention"""
        for ImageFormatValueT in self.ImageFormatValueT:
            if eval(ImageFormatValueT[1].replace("$i","self.renderInfo['ImageFormat']")):
                ImageFormatPalette = eval("palette_%s"%ImageFormatValueT[0])
        for CameraValueT in self.CameraValueT:
            if eval(CameraValueT[1].replace("$i","self.renderInfo['CameraAA']")):
                CameraPalette = eval("palette_%s"%CameraValueT[0])
        for DiffuseValueT in self.DiffuseValueT:
            if eval(DiffuseValueT[1].replace("$i","self.renderInfo['Diffuse']")):
                DiffusePalette = eval("palette_%s"%DiffuseValueT[0])
        for GlossyValueT in self.GlossyValueT:
            if eval(GlossyValueT[1].replace("$i","self.renderInfo['Glossy']")):
                GlossyPalette = eval("palette_%s"%GlossyValueT[0])
        for RefractionValueT in self.RefractionValueT:
            if eval(RefractionValueT[1].replace("$i","self.renderInfo['Refraction']")):
                RefractionPalette = eval("palette_%s"%RefractionValueT[0])
        for sssValueT in self.sssValueT:
            if eval(sssValueT[1].replace("$i","self.renderInfo['SSS']")):
                sssPalette = eval("palette_%s"%sssValueT[0])
        VolumeIndirectPalette = palette_green
        for MotionBlurEnableValueT in self.MotionBlurEnableValueT:
            if eval(MotionBlurEnableValueT[1].replace("$i","str(self.renderInfo['MotionBlurEnable'])")):
                MotionBlurEnablePalette = eval("palette_%s"%MotionBlurEnableValueT[0])
        for commontAOVsValueT in self.commontAOVsValueT:
            if eval(commontAOVsValueT[1].replace("$i","self.renderInfo['CommontAOVs']")):
                commontAOVsPalette = eval("palette_%s"%commontAOVsValueT[0])
        for objectIDValueT in self.objectIDValueT:
            if eval(objectIDValueT[1].replace("$i","self.renderInfo['ObjectID']")):
                objectIDPalette = eval("palette_%s"%objectIDValueT[0])
        for lightIDValueT in self.lightIDValueT:
            if eval(lightIDValueT[1].replace("$i","self.renderInfo['LightID']")):
                lightIDPalette = eval("palette_%s"%lightIDValueT[0])

        self.imageformat.setPalette(ImageFormatPalette)
        self.ImageFormatValue.setPalette(ImageFormatPalette)
        self.Camera.setPalette(CameraPalette)
        self.CameraValue.setPalette(CameraPalette)
        self.Diffuse.setPalette(DiffusePalette)
        self.DiffuseValue.setPalette(DiffusePalette)
        self.Glossy.setPalette(GlossyPalette)
        self.GlossyValue.setPalette(GlossyPalette)
        self.Refraction.setPalette(RefractionPalette)
        self.RefractionValue.setPalette(RefractionPalette)
        self.sss.setPalette(sssPalette)
        self.sssValue.setPalette(sssPalette)
        self.VolumeIndirect.setPalette(VolumeIndirectPalette)
        self.VolumeIndirectValue.setPalette(VolumeIndirectPalette)
        self.MotionBlurEnable.setPalette(MotionBlurEnablePalette)
        self.MotionBlurEnableValue.setPalette(MotionBlurEnablePalette)
        self.commontAOVs.setPalette(commontAOVsPalette)
        self.commontAOVsValue.setPalette(commontAOVsPalette)
        self.objectID.setPalette(objectIDPalette)
        self.objectIDValue.setPalette(objectIDPalette)
        self.lightID.setPalette(lightIDPalette)
        self.lightIDValue.setPalette(lightIDPalette)

    def showWindows(self):
        image = os.environ['PIPELINE_PATH'] + "/Maya/Icon/Lib/"
        try:
            mayaWindowParent = shiboken.wrapInstance(long(omui.MQtUtil.mainWindow()), QtGui.QWidget)
            prt = mayaWindowParent
        except:
            prt = None
        palette = QtGui.QPalette()
        color = QtGui.QColor(0.4*255,0.5*255,0.5*255)
        palette.setColor(QtGui.QPalette.Background,color)
        palette.setColor(QtGui.QPalette.WindowText,QtGui.QColor(0,0,0))
        palette_white = QtGui.QPalette()
        palette_white.setColor(QtGui.QPalette.Background,QtGui.QColor(255,255,255))
        """set windows widget"""
        self.showGui = QtGui.QDialog(parent=prt)
        self.showGui.setModal(True)
        self.showGui.setWindowTitle('Mili')
        self.showGui.setAutoFillBackground(True)
        #self.showGui.setPalette(palette)
        self.imageformat = QtGui.QLabel('Image Format :')
        self.imageformat.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        self.imageformat.setMinimumHeight(20)
        self.imageformat.setAutoFillBackground(True)
        self.ImageFormatValue = QtGui.QLabel(self.renderInfo['ImageFormat'])
        self.ImageFormatValue.setAlignment(QtCore.Qt.AlignCenter)
        self.ImageFormatValue.setAutoFillBackground(True)
        self.Camera = QtGui.QLabel('Camera(AA) :')
        self.Camera.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        self.Camera.setAutoFillBackground(True)
        self.Camera.setMinimumHeight(20)
        self.CameraValue = QtGui.QLabel(str(self.renderInfo['CameraAA']))
        self.CameraValue.setAlignment(QtCore.Qt.AlignCenter)
        self.CameraValue.setAutoFillBackground(True)
        self.Diffuse = QtGui.QLabel('Diffuse :')
        self.Diffuse.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        self.Diffuse.setAutoFillBackground(True)
        self.Diffuse.setMinimumHeight(20)
        self.DiffuseValue = QtGui.QLabel(str(self.renderInfo['Diffuse']))
        self.DiffuseValue.setAlignment(QtCore.Qt.AlignCenter)
        self.DiffuseValue.setAutoFillBackground(True)
        self.Glossy = QtGui.QLabel('Glossy :')
        self.Glossy.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        self.Glossy.setAutoFillBackground(True)
        self.Glossy.setMinimumHeight(20)
        self.GlossyValue = QtGui.QLabel(str(self.renderInfo['Glossy']))
        self.GlossyValue.setAlignment(QtCore.Qt.AlignCenter)
        self.GlossyValue.setAutoFillBackground(True)
        self.Refraction = QtGui.QLabel('Refraction :')
        self.Refraction.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        self.Refraction.setAutoFillBackground(True)
        self.Refraction.setMinimumHeight(20)
        self.RefractionValue = QtGui.QLabel(str(self.renderInfo['Refraction']))
        self.RefractionValue.setAlignment(QtCore.Qt.AlignCenter)
        self.RefractionValue.setAutoFillBackground(True)
        self.sss = QtGui.QLabel('SSS :')
        self.sss.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        self.sss.setAutoFillBackground(True)
        self.sss.setMinimumHeight(20)
        self.sssValue = QtGui.QLabel(str(self.renderInfo['SSS']))
        self.sssValue.setAlignment(QtCore.Qt.AlignCenter)
        self.sssValue.setAutoFillBackground(True)
        self.VolumeIndirect = QtGui.QLabel('Volume Indirect :')
        self.VolumeIndirect.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        self.VolumeIndirect.setAutoFillBackground(True)
        self.VolumeIndirect.setMinimumHeight(20)
        self.VolumeIndirectValue = QtGui.QLabel(str(self.renderInfo['VolumeIndirect']))
        self.VolumeIndirectValue.setAlignment(QtCore.Qt.AlignCenter)
        self.VolumeIndirectValue.setAutoFillBackground(True)
        self.MotionBlurEnable = QtGui.QLabel('Motion Blur Enable :')
        self.MotionBlurEnable.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        self.MotionBlurEnable.setAutoFillBackground(True)
        self.MotionBlurEnable.setMinimumHeight(20)
        self.MotionBlurEnableValue = QtGui.QLabel(str(self.renderInfo['MotionBlurEnable']))
        self.MotionBlurEnableValue.setAlignment(QtCore.Qt.AlignCenter)
        self.MotionBlurEnableValue.setAutoFillBackground(True)
        self.MotionBlurEnableValue.setMinimumHeight(20)
        self.commontAOVs = QtGui.QLabel('Commont AOVs :')
        self.commontAOVs.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        self.commontAOVs.setAutoFillBackground(True)
        self.commontAOVs.setMinimumHeight(20)
        self.commontAOVsValue = QtGui.QLabel(self.renderInfo['CommontAOVs'])
        self.commontAOVsValue.setAlignment(QtCore.Qt.AlignCenter)
        self.commontAOVsValue.setAutoFillBackground(True)
        self.commontAOVsValue.setMinimumHeight(20)
        self.objectID = QtGui.QLabel('   Number of ObjectIDs :')
        self.objectID.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        self.objectID.setAutoFillBackground(True)
        self.objectID.setMinimumHeight(20)
        self.objectIDValue = QtGui.QLabel(str(self.renderInfo['ObjectID']))
        self.objectIDValue.setAlignment(QtCore.Qt.AlignCenter)
        self.objectIDValue.setAutoFillBackground(True)
        self.objectIDValue.setMinimumHeight(20)
        self.lightID = QtGui.QLabel('Number of LightIDs :')
        self.lightID.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        self.lightID.setAutoFillBackground(True)
        self.lightID.setMinimumHeight(20)
        self.lightIDValue = QtGui.QLabel(str(self.renderInfo['LightID']))
        self.lightIDValue.setAlignment(QtCore.Qt.AlignCenter)
        self.lightIDValue.setAutoFillBackground(True)
        self.lightIDValue.setMinimumHeight(20)
        gridLayout = QtGui.QGridLayout()
        gridLayout.setSpacing(1)
        gridLayout.addWidget(self.imageformat,1,0)
        gridLayout.addWidget(self.ImageFormatValue,1,1)
        gridLayout.addWidget(self.Camera,2,0)
        gridLayout.addWidget(self.CameraValue,2,1)
        gridLayout.addWidget(self.Diffuse,3,0)
        gridLayout.addWidget(self.DiffuseValue,3,1)
        gridLayout.addWidget(self.Glossy,4,0)
        gridLayout.addWidget(self.GlossyValue,4,1)
        gridLayout.addWidget(self.Refraction,5,0)
        gridLayout.addWidget(self.RefractionValue,5,1)
        gridLayout.addWidget(self.sss,6,0)
        gridLayout.addWidget(self.sssValue,6,1)
        gridLayout.addWidget(self.VolumeIndirect,7,0)
        gridLayout.addWidget(self.VolumeIndirectValue,7,1)
        gridLayout.addWidget(self.MotionBlurEnable,8,0)
        gridLayout.addWidget(self.MotionBlurEnableValue,8,1)
        gridLayout.addWidget(self.commontAOVs,9,0)
        gridLayout.addWidget(self.commontAOVsValue,9,1)
        gridLayout.addWidget(self.objectID,10,0)
        gridLayout.addWidget(self.objectIDValue,10,1)
        gridLayout.addWidget(self.lightID,11,0)
        gridLayout.addWidget(self.lightIDValue,11,1)
        groupBox = QtGui.QGroupBox('Informations')
        groupBox.setLayout(gridLayout)
        groupBox.setPalette(palette)
        self.showGui.setContentsMargins(0,0,0,0)#内容完全充满窗口
        title_layout = QtGui.QVBoxLayout()
        title = QtGui.QLabel('  Arnold Render Information  ')
        title.setMinimumHeight(30)
        title.setAlignment(QtCore.Qt.AlignCenter)
        title.setFont(QtGui.QFont("Rome times",10,QtGui.QFont.Bold))
        title.setAutoFillBackground(True)
        title.setPalette(palette)
        title_layout.addWidget(title)
        button = QtGui.QPushButton(u'确定')
        button.clicked.connect(self.makeSure)
        button.setMinimumHeight(30)
        button.setFont(QtGui.QFont("Rome times",10,QtGui.QFont.Bold))
        information_layout = QtGui.QVBoxLayout()
        information_layout.addWidget(groupBox)
        information_layout.setContentsMargins(10,10,10,0)
        edit_button = QtGui.QToolButton()
        edit_button.clicked.connect(self.showScriptEditGui)
        edit_button.setIcon(QtGui.QIcon(image+"gear.png"))
        edit_button.setIconSize(QtCore.QSize(21,21))
        button_layout = QtGui.QHBoxLayout()
        button_layout.addWidget(button)
        if self.isTD():
            button_layout.addWidget(edit_button)
        button_layout.setContentsMargins(10,0,10,10)
        button_layout.setSpacing(0)
        infoLayout = QtGui.QVBoxLayout()
        infoLayout.setSpacing(10)
        infoLayout.addLayout(title_layout)
        infoLayout.addLayout(information_layout)
        infoLayout.addStretch()
        infoLayout.addSpacing(30)
        infoLayout.addLayout(button_layout)
        infoLayout.addSpacing(10)
        self.mainLayout = QtGui.QHBoxLayout()
        self.mainLayout.setContentsMargins(0,0,0,0)
        self.mainLayout.setSpacing(10)
        self.mainLayout.addLayout(infoLayout)
        self.showGui.setLayout(self.mainLayout)
        flags = QtCore.Qt.WindowFlags()
        flags |= QtCore.Qt.Tool
        flags |= QtCore.Qt.WindowStaysOnTopHint
        try:
            self.setTestValue("Server")
            self.setColor()
        except:
            pass
        self.showGui.setWindowFlags(flags)
        self.showGui.show()

    def showConfigGui(self):
        self.dock = theDockWidget()
        self.dock.signal.connect(self.waitTimer)
        ImageFormat = QtGui.QLabel('Image Format :')
        ImageFormat.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        ImageFormat.setMinimumHeight(20)
        self.ImageFormatValueE = QtGui.QLineEdit(self.ImageFormatValueT)
        self.ImageFormatValueE.setAlignment(QtCore.Qt.AlignCenter)
        Camera = QtGui.QLabel('Camera(AA) :')
        Camera.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        Camera.setMinimumHeight(20)
        self.CameraValueE = QtGui.QLineEdit(self.CameraValueT)
        self.CameraValueE.setAlignment(QtCore.Qt.AlignCenter)
        Diffuse = QtGui.QLabel('Diffuse :')
        Diffuse.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        Diffuse.setMinimumHeight(20)
        self.DiffuseValueE = QtGui.QLineEdit(self.DiffuseValueT)
        self.DiffuseValueE.setAlignment(QtCore.Qt.AlignCenter)
        Glossy = QtGui.QLabel('Glossy :')
        Glossy.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        Glossy.setMinimumHeight(20)
        self.GlossyValueE = QtGui.QLineEdit(self.GlossyValueT)
        self.GlossyValueE.setAlignment(QtCore.Qt.AlignCenter)
        Refraction = QtGui.QLabel('Refraction :')
        Refraction.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        Refraction.setMinimumHeight(20)
        self.RefractionValueE = QtGui.QLineEdit(self.RefractionValueT)
        self.RefractionValueE.setAlignment(QtCore.Qt.AlignCenter)
        sss = QtGui.QLabel('SSS :')
        sss.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        sss.setMinimumHeight(20)
        self.sssValueE = QtGui.QLineEdit(self.sssValueT)
        self.sssValueE.setAlignment(QtCore.Qt.AlignCenter)
        VolumeIndirect = QtGui.QLabel('Volume Indirect :')
        VolumeIndirect.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        VolumeIndirect.setMinimumHeight(20)
        self.VolumeIndirectValueE = QtGui.QLineEdit(self.VolumeIndirectValueT)
        self.VolumeIndirectValueE.setAlignment(QtCore.Qt.AlignCenter)
        self.VolumeIndirectValueE.setMinimumHeight(20)
        MotionBlurEnable = QtGui.QLabel('Motion Blur Enable :')
        MotionBlurEnable.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        MotionBlurEnable.setMinimumHeight(20)
        self.MotionBlurEnableValueE = QtGui.QLineEdit(self.MotionBlurEnableValueT)
        self.MotionBlurEnableValueE.setMinimumWidth(40)
        self.MotionBlurEnableValueE.setAlignment(QtCore.Qt.AlignCenter)
        self.MotionBlurEnableValueE.setMinimumHeight(20)
        commontAOVs = QtGui.QLabel('Commont AOVs :')
        commontAOVs.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        commontAOVs.setMinimumHeight(20)
        self.commontAOVsValueE = QtGui.QLineEdit(self.commontAOVsValueT)
        self.commontAOVsValueE.setAlignment(QtCore.Qt.AlignCenter)
        self.commontAOVsValueE.setMinimumHeight(20)
        objectID = QtGui.QLabel('   Number of ObjectIDs :')
        objectID.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        objectID.setMinimumHeight(20)
        self.objectIDValueE = QtGui.QLineEdit(self.objectIDValueT)
        self.objectIDValueE.setAlignment(QtCore.Qt.AlignCenter)
        self.objectIDValueE.setMinimumHeight(20)
        lightID = QtGui.QLabel('Number of LightIDs :')
        lightID.setAlignment(QtCore.Qt.AlignVCenter|QtCore.Qt.AlignRight)
        lightID.setMinimumHeight(20)
        self.lightIDValueE = QtGui.QLineEdit(self.lightIDValueT)
        self.lightIDValueE.setAlignment(QtCore.Qt.AlignCenter)
        self.lightIDValueE.setMinimumHeight(20)
        gridLayout = QtGui.QGridLayout()
        gridLayout.setSpacing(1)
        gridLayout.addWidget(ImageFormat,1,0)
        gridLayout.addWidget(self.ImageFormatValueE,1,1)
        gridLayout.addWidget(Camera,2,0)
        gridLayout.addWidget(self.CameraValueE,2,1)
        gridLayout.addWidget(Diffuse,3,0)
        gridLayout.addWidget(self.DiffuseValueE,3,1)
        gridLayout.addWidget(Glossy,4,0)
        gridLayout.addWidget(self.GlossyValueE,4,1)
        gridLayout.addWidget(Refraction,5,0)
        gridLayout.addWidget(self.RefractionValueE,5,1)
        gridLayout.addWidget(sss,6,0)
        gridLayout.addWidget(self.sssValueE,6,1)
        gridLayout.addWidget(VolumeIndirect,7,0)
        gridLayout.addWidget(self.VolumeIndirectValueE,7,1)
        gridLayout.addWidget(MotionBlurEnable,8,0)
        gridLayout.addWidget(self.MotionBlurEnableValueE,8,1)
        gridLayout.addWidget(commontAOVs,9,0)
        gridLayout.addWidget(self.commontAOVsValueE,9,1)
        gridLayout.addWidget(objectID,10,0)
        gridLayout.addWidget(self.objectIDValueE,10,1)
        gridLayout.addWidget(lightID,11,0)
        gridLayout.addWidget(self.lightIDValueE,11,1)
        button1 = QtGui.QPushButton(u"测试")
        button1.clicked.connect(self.test)
        button2 = QtGui.QPushButton(u"提交")
        button2.clicked.connect(self.upload)
        buttonLayout = QtGui.QHBoxLayout()
        buttonLayout.addWidget(button1)
        buttonLayout.addWidget(button2)
        mainLayout = QtGui.QVBoxLayout()
        mainLayout.addStretch()
        mainLayout.addLayout(gridLayout)
        mainLayout.addStretch()
        mainLayout.addLayout(buttonLayout)
        self.widget = QtGui.QWidget()
        self.widget.setLayout(mainLayout)
        palette = QtGui.QPalette()
        color = QtGui.QColor(0.4*255,0.5*255,0.5*255)
        palette.setColor(QtGui.QPalette.Background,color)
        palette.setColor(QtGui.QPalette.WindowText,QtGui.QColor(0,0,0))
        colorLine = QtGui.QLabel()
        colorLine.setAutoFillBackground(True)
        colorPalette = QtGui.QPalette()
        colorPalette.setColor(QtGui.QPalette.Background,QtGui.QColor(255,255,255))
        colorLine.setPalette(colorPalette)
        self.widget.setAutoFillBackground(True)
        self.widget.setContentsMargins(0,0,0,5)
        self.dock.setWindowTitle("Set Config")
        self.dock.setWidget(self.widget)
        #self.dock.setPalette(palette)
        self.dock.setContentsMargins(0,0,5,0)
        dockLayout = QtGui.QHBoxLayout()
        dockLayout.addWidget(self.dock)
        dockLayout.setContentsMargins(0,0,0,0)
        self.mainLayout.addLayout(dockLayout)

    def initScript(self):
        try:
            import Mili.Module.utilties.ProjectConfig as ProjectConfig
            reload(ProjectConfig)
            text = ProjectConfig.ProjectConfig().get('DefaultRenderConfig')
            configMap = eval("{"+text+"}")
            Script = \
            """'ImageFormat' : %s,\n"""\
            """'CameraAA' : %s,\n"""\
            """'Diffuse' : %s,\n"""\
            """'Glossy' : %s,\n"""\
            """'Refraction' : %s,\n"""\
            """'SSS' : %s,\n"""\
            """'VolumeIndirect' : %s,\n"""\
            """'MotionBlurEnable' : %s,\n"""\
            """'CommontAOVs' : %s,\n"""\
            """'ObjectID' : %s,\n"""\
            """'LightID' : %s,\n"""%(
            configMap['ImageFormat'],
            configMap['CameraAA'],
            configMap['Diffuse'],
            configMap['Glossy'],
            configMap['Refraction'],
            configMap['SSS'],
            configMap['VolumeIndirect'],
            configMap['MotionBlurEnable'],
            configMap['CommontAOVs'],
            configMap['ObjectID'],
            configMap['LightID'],)
        except:
            Script = self.printTo(script=True)
        self.scriptEdit.setText(Script)
        self.scriptEdit.setFont(QtGui.QFont("times",10,QtGui.QFont.DemiBold))

    def showScriptEditGui(self):
        self.dock = theDockWidget()
        self.dock.signal.connect(self.waitTimer)
        scriptLayout = QtGui.QVBoxLayout()
        self.scriptEdit = QtGui.QTextEdit()
        self.scriptEdit.setLineWrapMode(QtGui.QTextEdit.NoWrap)
        self.scriptEdit.setMinimumSize(300,300)
        self.initScript()
        self.translateTo()
        scriptLayout.addWidget(self.scriptEdit)
        button1 = QtGui.QPushButton(u"测试")
        button1.clicked.connect(self.test)
        button2 = QtGui.QPushButton(u"提交")
        button2.clicked.connect(self.upload)
        buttonLayout = QtGui.QHBoxLayout()
        buttonLayout.addWidget(button1)
        buttonLayout.addWidget(button2)
        mainLayout = QtGui.QVBoxLayout()
        mainLayout.addStretch()
        mainLayout.addLayout(scriptLayout)
        mainLayout.addStretch()
        mainLayout.addLayout(buttonLayout)
        self.widget = QtGui.QWidget()
        self.widget.setLayout(mainLayout)
        palette = QtGui.QPalette()
        color = QtGui.QColor(0.4*255,0.5*255,0.5*255)
        palette.setColor(QtGui.QPalette.Background,color)
        palette.setColor(QtGui.QPalette.WindowText,QtGui.QColor(0,0,0))
        colorLine = QtGui.QLabel()
        colorLine.setAutoFillBackground(True)
        colorPalette = QtGui.QPalette()
        colorPalette.setColor(QtGui.QPalette.Background,QtGui.QColor(255,255,255))
        colorLine.setPalette(colorPalette)
        self.widget.setAutoFillBackground(True)
        self.widget.setContentsMargins(0,0,0,5)
        self.dock.setWindowTitle("Set Config")
        self.dock.setWidget(self.widget)
        #self.dock.setPalette(palette)
        self.dock.setContentsMargins(0,0,5,0)
        dockLayout = QtGui.QHBoxLayout()
        dockLayout.addWidget(self.dock)
        dockLayout.setContentsMargins(0,0,0,0)
        self.mainLayout.addLayout(dockLayout)

    def makeSure(self):
        self.showGui.accept()
        self.showGui.close()

    def waitTimer(self):
        self.timer= QtCore.QTimer()
        self.timer.start(10)
        self.timer.timeout.connect(lambda *args: self.resizeTo())

    def printTo(self,template=True,script=False):
        if template == True and script == False:
            Template = \
            u"""脚本配置模板:\n"""\
            u"""'参数名' : [('颜色1', '判定语句1')，('颜色2', '判定语句2')],\n"""
            print Template
            return Template
        if script == True:
            Script = \
            """'ImageFormat' : [('green', '$i == "exr" or $i == "deepexr"')],\n"""\
            """'CameraAA' : [('red', '$i < 5'),('red', '$i > 10')],\n"""\
            """'Diffuse' : [('yellow', '$i < 2')],\n"""\
            """'Glossy' : [('yellow', '$i < 2')],\n"""\
            """'Refraction' : [('yellow', '$i < 2')],\n"""\
            """'SSS' : [('yellow', '$i < 2')],\n"""\
            """'VolumeIndirect' : [('yellow', '$i < 2')],\n"""\
            """'MotionBlurEnable' : [('green', '$i == "True"')],\n"""\
            """'CommontAOVs' : [('green', '$i == "On"')],\n"""\
            """'ObjectID' : [('yellow', '$i == 0')],\n"""\
            """'LightID' : [('yellow', '$i == 0')],\n"""
            return Script

    def resizeTo(self):
        self.showGui.resize(self.width,self.height)
        self.timer.stop()

    def translateTo(self):
        text = self.scriptEdit.toPlainText()
        self.scriptMap = eval("{"+text+"}")

    def setTestValue(self,plug):
        if plug == "Server":
            import Mili.Module.utilties.ProjectConfig as ProjectConfig
            reload(ProjectConfig)
            text = ProjectConfig.ProjectConfig().get('DefaultRenderConfig')
            try:
                if text:
                    configMap = eval("{"+text+"}")
                else:
                    text = self.printTo(script=True)
                    configMap = eval("{"+text+"}")
            except:
                text = self.printTo(script=True)
                configMap = eval("{"+text+"}")
            self.ImageFormatValueT = configMap['ImageFormat']
            self.CameraValueT = configMap['CameraAA']
            self.DiffuseValueT = configMap['Diffuse']
            self.GlossyValueT = configMap['Glossy']
            self.RefractionValueT = configMap['Refraction']
            self.sssValueT = configMap['SSS']
            self.VolumeIndirectValueT = configMap['VolumeIndirect']
            self.MotionBlurEnableValueT = configMap['MotionBlurEnable']
            self.commontAOVsValueT = configMap['CommontAOVs']
            self.objectIDValueT = configMap['ObjectID']
            self.lightIDValueT = configMap['LightID']
        elif plug == "Local":
            self.ImageFormatValueT = self.scriptMap["ImageFormat"]
            self.CameraValueT = self.scriptMap["CameraAA"]
            self.DiffuseValueT = self.scriptMap["Diffuse"]
            self.GlossyValueT = self.scriptMap["Glossy"]
            self.RefractionValueT = self.scriptMap["Refraction"]
            self.sssValueT = self.scriptMap["SSS"]
            self.VolumeIndirectValueT = self.scriptMap["VolumeIndirect"]
            self.MotionBlurEnableValueT = self.scriptMap["MotionBlurEnable"]
            self.commontAOVsValueT = self.scriptMap["CommontAOVs"]
            self.objectIDValueT = self.scriptMap["ObjectID"]
            self.lightIDValueT = self.scriptMap["LightID"]
        else:
            print "####################\n#Wrong arguments\n####################\n"

    def test(self):
        try:
            self.translateTo()
        except:
            dialog = theDialog(self.showGui)
            dialog.showWindows(title="Erro",content=self.erroRaise(u"脚本格式错误",line="0~11"),plug="Confirm")
            return
        self.setTestValue("Local")
        try:
            self.setColor()
        except:
            dialog = theDialog(self.showGui)
            dialog.showWindows(title="Erro",content=self.erroRaise(u"逻辑脚本错误",line=1),plug="Confirm")
            return

    def upload(self):
        try:
            try:
                self.translateTo()
            except:
                dialog = theDialog(self.showGui)
                dialog.showWindows(title="Erro",content=self.erroRaise(u"脚本格式错误",line="0~11"),plug="Confirm")
                return
            self.setTestValue("Local")
            try:
                self.setColor()
            except:
                dialog = theDialog(self.showGui)
                dialog.showWindows(title="Erro",content=self.erroRaise(u"逻辑脚本错误",line=1),plug="Confirm")
                return
        except:
            dialog = theDialog(self.showGui)
            dialog.showWindows(title="Erro",content=self.erroRaise(u"代码逻辑错误"),plug="Confirm")
            return
        import Mili.Module.utilties.ProjectConfig as ProjectConfig
        reload(ProjectConfig)
        config = ProjectConfig.ProjectConfig()
        config['DefaultRenderConfig'] = str(self.scriptEdit.toPlainText())
        config.save()
        self.dock.close()

    def erroRaise(self,info="No information",line=1,kill=False):
        """
        这个函数用于收集错误信息
        kill参数为True时会在报错时终止程序
        """
        s=sys.exc_info()
        erro = "===Erro Report:===\n" \
               "Error : '%s' \n" \
               "Happened on line %s\n" \
               "%s\n" \
               "==================\n\n" % (s[1],line,info)
        print erro
        self.printTo()
        #~ 是否终止程序
        if kill == True:
            sys.exit()
        return erro

if __name__ == '__main__':
    #app = QtGui.QApplication(sys.argv)
    ARI = ArnoldRenderInfo()
    ARI.showGui.exec_()
    #sys.exit(app.exec_())


