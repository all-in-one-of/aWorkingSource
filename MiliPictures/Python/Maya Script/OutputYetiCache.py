#coding:utf-8
__author__ = 'kai.xu'

import maya.cmds as cmds
import sys,os
import Mili.Module.rendering.switchIntermediateTexture as switchIntermediateTexture
import Mili.Module.utilties.Get as Get
import Mili.Module.utilties.ProjectConfig as ProjectConfig
reload(switchIntermediateTexture)
reload(ProjectConfig)
reload(Get)

class OutputYetiCache():
    def __init__(self):
        self.YetiNodesInformation = []
        self.information()
        if cmds.window('OutputYetiCache', q=1, exists=1):
            cmds.deleteUI('OutputYetiCache')
        Mytool = cmds.window('OutputYetiCache',title='OutputYetiCache',width=530,sizeable=False)
        cmds.rowColumnLayout('mainLayout', numberOfColumns=1, columnWidth=[(1, 530)])
        cmds.text(label='Yeti Cache Output',height=30, backgroundColor=[0.5, 0.5, 0.6])
        cmds.separator(style='out')
        cmds.text(' ')
        cmds.separator(style='out')
        cmds.scrollLayout('scrollLayoutUnique', height=200)

        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 330),(2,170)])

        cmds.rowColumnLayout(numberOfColumns=1, columnWidth=[(1, 330)])
        cmds.text(u'Yeti节点名称',align='left',backgroundColor=[0.5, 0.4, 0.5],font='boldLabelFont',height=30)

        #显示Yeti节点
        cmds.rowColumnLayout(numberOfColumns=3, columnWidth=[(1, 250),(2,40),(3,25)])
        allYeties = cmds.ls(type='pgYetiMaya')
        self.selectYeties = cmds.ls(sl=1,leaf=1,dag=1,type='pgYetiMaya')
        #测试勾选情况
        for YetiNode in allYeties:
            if YetiNode in self.selectYeties:
                try:
                    transformNode = cmds.listRelatives(YetiNode,parent=True)
                    if cmds.objectType(transformNode,isType='transform') == True:
                        flag = True
                    else:
                        flag = False
                except:
                    flag = False
            else:
                flag = False

            for YetiNodeInfo in self.YetiNodesInformation:
                if YetiNodeInfo['ShapeName'] == YetiNode:
                    ID = YetiNodeInfo['ID']

            cmds.text('%s'%YetiNode,align='left', height=30)
            cmds.text('active:',enable=False,height=30)
            cmds.checkBox('checkBox_%s'%ID, l='',visible=True, value=flag,height=30)
        cmds.setParent('..')

        cmds.setParent('..')

        #显示Yeti节点的参数
        cmds.rowColumnLayout(numberOfColumns=3, columnWidth=[(1, 60),(2,60),(3,60)])
        cmds.text(u'起始帧',align='center',backgroundColor=[0.6, 0.5, 0.5],font='boldLabelFont',height=30)
        cmds.text(u'结束帧',align='center',backgroundColor=[0.5, 0.6, 0.5],font='boldLabelFont',height=30)
        cmds.text(u'采样值',align='center',backgroundColor=[1, 0.7, 0.5],font='boldLabelFont',height=30)
        for YetiNodeInfo in self.YetiNodesInformation:
            ID = YetiNodeInfo['ID']
            cmds.textField('startFrame_%s'%ID, text=u'%s'%YetiNodeInfo['startFrame'],backgroundColor=[0.6, 0.5, 0.5],height=30)
            cmds.textField('endFrame_%s'%ID, text=u'%s'%YetiNodeInfo['endFrame'],backgroundColor=[0.5, 0.6, 0.5],height=30)
            cmds.textField('sampleValue_%s'%ID, text=u'%s'%YetiNodeInfo['sampleValue'],backgroundColor=[1, 0.7, 0.5],height=30)
        cmds.setParent('..')
        cmds.setParent('..')

        cmds.setParent('..')#scrollLayout
        cmds.separator(style='out')

        cmds.rowColumnLayout(numberOfColumns=4, columnWidth=[(1, 120),(2,50),(3,120),(4,50)])
        cmds.text('updateViewport_text',label='updateViewport :',align='center',font='boldLabelFont',height=25, backgroundColor=[0.5, 0.4, 0.5])
        cmds.checkBox('updateViewport', l='', align='center', value=False,backgroundColor=[0.5, 0.4, 0.5])
        cmds.text('generatePreview_text',label='generatePreview :',align='center',font='boldLabelFont',height=25, backgroundColor=[0.5, 0.4, 0.5])
        cmds.checkBox('generatePreview', l='', value=False,align='center',backgroundColor=[0.5, 0.4, 0.5])
        cmds.setParent('..')

        cmds.text(' ')

        cmds.rowColumnLayout(numberOfColumns=3, columnWidth=[(1, 70),(2,420),(3,40)])
        cmds.text(u'发布地址:',align='center',font='boldLabelFont',height=25, backgroundColor=[0.5, 0.4, 0.5])
        cmds.textField('savePath', text=' ',height=25,editable=False)
        cmds.iconTextButton(style='iconOnly',image='HIKCharacterization_ToolBar_ToggleNameMatch.png',command=lambda *arg: self.getSavePath())
        cmds.setParent('..')

        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 70),(2,460)])
        cmds.text(u'贴图池子:',align='center',font='boldLabelFont',height=25,backgroundColor=[0.5, 0.4, 0.5])
        self.Project = Get.Get().GetActiveProject()
        self.textureRoot=ProjectConfig.ProjectConfig().get('DeadlineSubmitter_osImgRoot', default=r'\\hnas01\outrender')+'\\'+self.Project+'_Texture'
        cmds.textField('textureRoot', text=self.textureRoot, height=25, editable=False)
        cmds.setParent('..')

        cmds.button(parent='mainLayout', label=u'Go!', height=45, command=lambda *arg: self.goCache())
        cmds.separator(parent='mainLayout', style='out')

        cmds.showWindow(Mytool)

    def information(self):
        #############################################################
        try:  # notify project
            from Mili.Core.ui import notifyProject
            reload(notifyProject)
            notifyProject.notifyProject()
        except: pass
        #刷Maya UI 滑条
        #############################################################
        minTime = cmds.playbackOptions(q=True, min=True)
        maxTime = cmds.playbackOptions(q=True, max=True)
        allYeties = cmds.ls(type='pgYetiMaya')
        ID = 0
        for YetiNode in allYeties:
            ID += 1
            ShapeName = YetiNode
            savePath = ''
            startFrame = minTime
            endFrame = maxTime
            sampleValue = 3
            YetiNodeMap = {'ID':ID, 'ShapeName':ShapeName,'savePath':savePath,'startFrame':startFrame,'endFrame':endFrame,'sampleValue':sampleValue}
            self.YetiNodesInformation.append(YetiNodeMap)

    def saveToMap(self):
        #在发布地址创建文件夹
        filePath = cmds.textField('savePath', text=True, q=True)
        for YetiNodeInfo in self.YetiNodesInformation:
            ID = YetiNodeInfo['ID']
            ShapeName = YetiNodeInfo['ShapeName']
            everyWord = ShapeName.split(':')
            #拆分名字建立文件夹
            if len(everyWord) == 2:
                nameSpace = everyWord[0]
                YetiShape = everyWord[1]
                fileRootPath = filePath+'/'+nameSpace+'/'+YetiShape+'/'+YetiShape+'.%04d.fur'
                fileDirsPath = filePath+'/'+nameSpace+'/'+YetiShape
                if os.path.isdir(fileDirsPath):
                    pass
                elif cmds.checkBox('checkBox_%s'%ID, value=True, q=True):
                    os.makedirs(fileDirsPath)
                else:
                    pass
                YetiNodeInfo['savePath'] = fileRootPath
            elif len(everyWord) == 1:
                nameSpace = 'noNameSpace'
                YetiShape = everyWord[0]
                fileRootPath = filePath+'/'+nameSpace+'/'+YetiShape+'/'+YetiShape+'.%04d.fur'
                fileDirsPath = filePath+'/'+nameSpace+'/'+YetiShape
                if os.path.isdir(fileDirsPath):
                    pass
                elif cmds.checkBox('checkBox_%s'%ID, value=True, q=True):
                    os.makedirs(fileDirsPath)
                else:
                    pass
                YetiNodeInfo['savePath'] = fileRootPath
            else:
                YetiNodeInfo['savePath'] = filePath + ShapeName.replace(':','_') + '.%04d.fur'

        #把信息储存到字典中
        for YetiNodeInfo in self.YetiNodesInformation:
            ID = YetiNodeInfo['ID']
            YetiNodeInfo['startFrame'] = cmds.textField('startFrame_%s'%ID, text=True, q=True)
            YetiNodeInfo['endFrame'] = cmds.textField('endFrame_%s'%ID, text=True, q=True)
            YetiNodeInfo['sampleValue'] = cmds.textField('sampleValue_%s'%ID, text=True, q=True)

    def goCache(self):
        self.saveToMap()
        ######################################
        #for YetiNode in self.YetiNodesInformation:
        #    print YetiNode['ShapeName']
        #    print YetiNode['startFrame'],YetiNode['endFrame'],YetiNode['sampleValue']
        #    print YetiNode['savePath']
        ######################################

        yourChoice = cmds.confirmDialog(title='Make Sure', message=u'你确定导出Yeti的缓存吗？', button=[u'是的！',u'点错了'], defaultButton=u'是的！', cancelButton=u'点错了', dismissString=u'点错了' )
        if yourChoice == u'是的！':
            self.getherYetiTexture()
            import maya.mel as mel
            for YetiNodeInfo in self.YetiNodesInformation:
                ID = YetiNodeInfo['ID']
                key = cmds.checkBox('checkBox_%s'%ID, value=True, q=True)
                if key == True:
                    fileName = YetiNodeInfo['savePath']
                    startFrame = YetiNodeInfo['startFrame']
                    endFrame = YetiNodeInfo['endFrame']
                    sampleValue = YetiNodeInfo['sampleValue']
                    ShapeName = YetiNodeInfo['ShapeName']

                    #发布的命令
                    updateViewport_key = cmds.checkBox('updateViewport', value=True, q=True)
                    generatePreview_key = cmds.checkBox('generatePreview', value=True, q=True)
                    if updateViewport_key == True:
                        updateViewport = 1
                    else:
                        updateViewport = 0
                    if generatePreview_key == True:
                        generatePreview = 1
                    else:
                        generatePreview = 0

                    cmd = 'pgYetiCommand -writeCache "%s" -range %s %s -samples %s -updateViewport %s -generatePreview %s "%s"'%(fileName,startFrame,endFrame,sampleValue,updateViewport,generatePreview,ShapeName)
                    mel.eval(cmd)

                else:
                    pass
        else:
            pass


    def getSavePath(self):
        filePath = cmds.fileDialog2(fileMode=3, caption='Save')
        cmds.textField('savePath', text=filePath[0], edit=True)

    def getherYetiTexture(self):
        self.Project = Get.Get().GetActiveProject()
        self.textureRoot=ProjectConfig.ProjectConfig().get('DeadlineSubmitter_osImgRoot',default=r'\\hnas01\outrender')+'\\'+self.Project+'_Texture'
        self.switcher = switchIntermediateTexture.switchIntermediateTexture(Project=self.Project, textureRoot=self.textureRoot, checkTypeList=['pgYetiMaya',])

        if self.switcher.uploadTexAssets(checkOnly=1)!={}:
            cmds.confirmDialog(message=u'Yeti节点中有丢失的贴图，如果发布.fur文件，农场无法正常渲染这个镜头\n\n请点Confirm来显示丢失的贴图，\n如果您不清楚如何修正丢失的Yeti贴图，请提醒制作Yeti的同事或TD处理')
            self.switcher.uploadTexAssets(checkOnly=2)
            sys.exit()
        if 'copyExtraTexToRelativeDir' not in self.__dict__:
            self.copyExtraTexToRelativeDir ='/Textures/RenderShots/CUT'+'_ManualCache/'
        self.switcher.uploadTexAssets(recover=1, copyExtraTexToRelativeDir=self.copyExtraTexToRelativeDir)


if __name__ == '__main__':
    OutputYetiCache()

