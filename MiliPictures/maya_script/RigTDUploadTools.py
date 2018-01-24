#coding:utf-8
__author__ = 'kai.xu'


import maya.cmds as cmds
import os,time,sys
import Mili.Module.utilties.lsrunas as lsrunas
import Mili.Module.utilties.Path as Path
import Mili.Module.utilties.Get as Get
reload(lsrunas)
reload(Get)
reload(Path)


class RigTDUploadTools():
    def __init__(self):
        self.ServerRootPath = ''

        if cmds.window('RigTDUploadTools', q=1, exists=1):
            cmds.deleteUI('RigTDUploadTools')
        Mytool = cmds.window('RigTDUploadTools',title='RigTDUploadTools',widthHeight=(400,500),sizeable=False)
        cmds.rowColumnLayout( numberOfColumns=1, columnWidth=[(1, 400)])
        cmds.text(label='RigTD   Upload   Tools',height=30, backgroundColor=[0.5, 0.5, 0.6])
        cmds.separator(style='out')
        cmds.text(label=' ')
        cmds.rowColumnLayout( numberOfColumns=2, columnWidth=[(1, 70),(2,320)])
        cmds.text(label='Server Path:', align='left')
        cmds.textField('textServer', editable=False)
        cmds.text(label=' ')
        cmds.setParent('..')
        cmds.separator(style='out')
        cmds.text(label=' ')
        cmds.rowColumnLayout( numberOfColumns=1, columnWidth=[(1, 400)])
        cmds.button(label='Upload', height=40,align='right', command=lambda *arg: self.uploadFile())
        cmds.setParent('..')
        cmds.text(label=' ')
        cmds.separator(style='out')

        cmds.showWindow(Mytool)

        self.getFilePath()

    def getFilePath(self):
        #ServerProjectsPath\%ProjectName%\Rigs\%Type%\%Name%\TDAssets\
        Prj = Get.Get().GetActiveProject()
        ServerRoot = Path.Path().GetServerPath()

        allGroup = cmds.ls(type='transform')
        keyGroup = []
        for item in allGroup:
            test = item.split('_')
            if test[-1] == 'GRP' and test[-2] == 'model':
                keyGroup.append(item)
        try:
            tmp_modType = cmds.getAttr(keyGroup[0]+'.Type')
            keyName = cmds.getAttr(keyGroup[0]+'.treeName')
        except:
            #cmds.confirmDialog(title='Oops', message='Plsase open your Rig file first!', button='Gotcha')
            cmds.textField('textServer', text=u'请打开绑定文件，再重新打开此插件,以获取上传路径', edit=True)
            return
        if tmp_modType == 'c':
            modType = 'Characters'
        elif tmp_modType == 'p':
            modType = 'Props'
        else:
            print 'Erro modType:%s'%tmp_modType

        self.ServerRootPath = '%s/%s/Rigs/%s/%s/TDAssets/'%(ServerRoot,Prj,modType,keyName) #'ServerProjectsPath/%ProjectName%/Rigs/%Type%/%Name%/TDAssets'
        #print self.ServerRootPath
        textServer = self.ServerRootPath.replace('/','\\')
        cmds.textField('textServer', text=textServer, edit=True)
    '''
    def browseFile(self):
        FileDialog = cmds.fileDialog2(fileMode=True, caption='Find your file')
        try:
            self.filePath = FileDialog[0]
            self.fileName = FileDialog[0].split('/')[-1]
        except:
            self.filePath = '...'
            self.fileName = '...'
        cmds.text('textName', label=self.fileName, edit=True)
        cmds.text('textPath', label=self.filePath, edit=True)
    '''
    def uploadFile(self):
        if cmds.file(q=True,sceneName=True) == '':
            cmds.confirmDialog(title='Oops', message='Plsase open your file!', button='Gotcha')
            sys.exit()
        else:
            self.fileName = cmds.file(q=True,expandName=True).split('/')[-1]
            self.filePath = cmds.file(q=True,expandName=True)

        LocalRootPath = self.filePath
        uploadTime = time.time()
        if self.ServerRootPath == '':
            cmds.confirmDialog(title='Oops', message='Please open rig file first!', button='Gotcha')
            sys.exit()
        elif os.path.isfile(self.ServerRootPath+self.fileName):
            lsrunas.lsrunas().deleteFile(self.ServerRootPath+self.fileName)
            time.sleep(3)
        else:
            pass

        if not os.path.isdir(os.path.dirname(self.ServerRootPath)):
            lsrunas.lsrunas().CreateDir_1(os.path.dirname(self.ServerRootPath))
        print LocalRootPath
        print self.ServerRootPath+self.fileName
        lsrunas.lsrunas(uploadTime).UploadFile(LocalRootPath, self.ServerRootPath+self.fileName, DeleteSource=False)
        cmds.confirmDialog(title='DONE', message='Job Done!!!', button='Gotcha')






if __name__ == '__main__':
    RigTDUploadTools()
