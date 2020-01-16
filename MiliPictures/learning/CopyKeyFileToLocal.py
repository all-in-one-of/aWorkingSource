#coding:utf-8
__author__ = 'kai.xu'
import maya.cmds as cmds
import shutil,os


class AutoCopyTool():
    def __init__(self):
        self.Prj = 'DragonNest2'
        self.ServerRootPath = '//hnas01/data/Projects/DragonNest2/'
        self.LocalRootPath = 'D:/'
        self.showGui()

    def showGui(self):
        if cmds.window('AutoCopyTool', q=1, exists=1):
            cmds.deleteUI('AutoCopyTool')
        Mytool = cmds.window('AutoCopyTool',title='Tools',widthHeight=(400,500),sizeable=False)
        cmds.rowColumnLayout( numberOfColumns=1, columnWidth=[(1, 400)])
        cmds.text(label='Copy Key File To Local',height=30, backgroundColor=[0.5, 0.5, 0.6])
        cmds.separator(style='out')
        cmds.text(label=' ')
        cmds.rowColumnLayout( numberOfColumns=2, columnWidth=[(1, 70),(2,320)])
        cmds.text(label='Name it: ',align='center',font='boldLabelFont')
        cmds.textField('name_text')
        cmds.setParent('..')
        cmds.text(label=' ')
        cmds.button(label='Get!',height=40,command=lambda *arg: self.showProgress())
        cmds.showWindow(Mytool)

    def seacher(self):
        allADsARsLists = cmds.ls(type='assemblyReference')
        allADsLists = cmds.ls(type='assemblyDefinition')
        keyADsLists = []
        for AD in allADsARsLists:
            ADsParts = AD.split('_')
            keyAD = ''
            print AD
            for i in range(1,len(ADsParts)-2):
                if i == 1:
                    keyAD += ADsParts[i]
                elif i == (len(ADsParts)-2):
                    keyAD += ADsParts[i]
                else:
                    keyAD = keyAD +'_'+ADsParts[i]
            keyADsLists.append(keyAD)
        for AD in allADsLists:
            ADsParts = AD.split('_')
            keyAD = ''
            for i in range(1,len(ADsParts)-1):
                if i == 1:
                    keyAD += ADsParts[i]
                elif i == (len(ADsParts)-1):
                    keyAD += ADsParts[i]
                else:
                    keyAD = keyAD +'_'+ADsParts[i]
            keyADsLists.append(keyAD)

        for AdName in keyADsLists:
            ModelsServerRootPath = self.ServerRootPath+'Models/'+'Scene/'+AdName
            MaterialsServerRootPath = self.ServerRootPath+'Materials/'+'Scene/'+AdName
            TexturesServerRootPath = self.ServerRootPath+'Textures/'+'Scene/'+AdName
            theName = cmds.textField('name_text',text=True,q=True)+'/'
            ModelsLocalRootPath = self.LocalRootPath+theName+self.Prj+'/Models/'+'Scene/'+AdName
            MaterialsLocalRootPath = self.LocalRootPath+theName+self.Prj+'/Materials/'+'Scene/'+AdName
            TexturesLocalRootPath = self.LocalRootPath+theName+self.Prj+'/Textures/'+'Scene/'+AdName

            try:
                shutil.copytree(ModelsServerRootPath,ModelsLocalRootPath)
            except WindowsError:
                if os.path.isdir(ModelsLocalRootPath):
                    pass
                else:
                    os.makedirs(ModelsLocalRootPath)
            try:
                shutil.copytree(MaterialsServerRootPath,MaterialsLocalRootPath)
            except WindowsError:
                if os.path.isdir(MaterialsLocalRootPath):
                    pass
                else:
                    os.makedirs(MaterialsLocalRootPath)
            try:
                shutil.copytree(TexturesServerRootPath,TexturesLocalRootPath)
            except WindowsError:
                if os.path.isdir(TexturesLocalRootPath):
                    pass
                else:
                    os.makedirs(TexturesLocalRootPath)

        cmds.confirmDialog(title='Job Done', message='Job Done!', button='Gocha!')

    def showProgress(self):
        allADsARsLists = cmds.ls(type='assemblyReference')
        allADsLists = cmds.ls(type='assemblyDefinition')
        keyADsLists = []
        for AD in allADsARsLists:
            ADsParts = AD.split('_')
            keyAD = ''
            for i in range(1,len(ADsParts)-2):
                if i == 1:
                    keyAD += ADsParts[i]
                elif i == (len(ADsParts)-2):
                    keyAD += ADsParts[i]
                else:
                    keyAD = keyAD +'_'+ADsParts[i]
            keyADsLists.append(keyAD)
        for AD in allADsLists:
            ADsParts = AD.split('_')
            keyAD = ''
            for i in range(1,len(ADsParts)-1):
                if i == 1:
                    keyAD += ADsParts[i]
                elif i == (len(ADsParts)-1):
                    keyAD += ADsParts[i]
                else:
                    keyAD = keyAD +'_'+ADsParts[i]
            keyADsLists.append(keyAD)


        amount = 0
        cmds.progressWindow(title='Doing Nothing',progress=amount,status='Copy: %s/%s'%(0,len(keyADsLists)),isInterruptable=True,minValue=0,maxValue=len(keyADsLists))
        for AdName in keyADsLists:
                # Check if the dialog has been cancelled
                if cmds.progressWindow(query=True, isCancelled=True):
                        break

                # Check if end condition has been reached
                if cmds.progressWindow(query=True,progress=True) >= 100:
                        break

                amount += 1

                ModelsServerRootPath = self.ServerRootPath+'Models/'+'Scene/'+AdName
                MaterialsServerRootPath = self.ServerRootPath+'Materials/'+'Scene/'+AdName
                TexturesServerRootPath = self.ServerRootPath+'Textures/'+'Scene/'+AdName
                theName = cmds.textField('name_text',text=True,q=True)+'/'
                ModelsLocalRootPath = self.LocalRootPath+theName+self.Prj+'/Models/'+'Scene/'+AdName
                MaterialsLocalRootPath = self.LocalRootPath+theName+self.Prj+'/Materials/'+'Scene/'+AdName
                TexturesLocalRootPath = self.LocalRootPath+theName+self.Prj+'/Textures/'+'Scene/'+AdName

                try:
                    shutil.copytree(ModelsServerRootPath,ModelsLocalRootPath)
                except WindowsError:
                    if os.path.isdir(ModelsLocalRootPath):
                        pass
                    else:
                        os.makedirs(ModelsLocalRootPath)
                try:
                    shutil.copytree(MaterialsServerRootPath,MaterialsLocalRootPath)
                except WindowsError:
                    if os.path.isdir(MaterialsLocalRootPath):
                        pass
                    else:
                        os.makedirs(MaterialsLocalRootPath)
                try:
                    shutil.copytree(TexturesServerRootPath,TexturesLocalRootPath)
                except WindowsError:
                    if os.path.isdir(TexturesLocalRootPath):
                        pass
                    else:
                        os.makedirs(TexturesLocalRootPath)

                cmds.progressWindow(edit=True, progress=amount, status='Copy: %s/%s'%(amount,len(keyADsLists)) )

        cmds.progressWindow(endProgress=1)
        cmds.confirmDialog(title='Job Done', message='The file had moved to local dish!', button='Gocha!')

if __name__ == '__main__':
    AutoCopyTool()
