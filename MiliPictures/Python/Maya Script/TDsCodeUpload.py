#coding = utf-8
__author__ = 'kai.xu'


import maya.cmds as cmds
import os,sys,shutil,datetime,json,time,chardet,shutil
import Mili.Module.utilties.MiliUser as MiliUser
import Mili.Module.utilties.lsrunas as lsrunas
import Mili.Module.utilties.Get as Get
import Mili.Module.utilties.Path as Path
reload(lsrunas)
reload(MiliUser)
reload(Get)
reload(Path)


class TDsCodeUpload():
    def __init__(self):
        self.department = MiliUser.MiliUser()['department']
        self.appointment = MiliUser.MiliUser()['appointment']
        self.moduleList = MiliUser.MiliUser().getGroupList()
        self.flag = [False, False, False, False, False, False]
        self.oldActiveMap = {}
        self.rightConfirm()
        self.showGUI()

    def showGUI(self):
        if cmds.window('TDsCodeTools', q=1, exists=1):
            cmds.deleteUI('TDsCodeTools')
        TDsCodeUploadTool = cmds.window('TDsCodeTools', title='TDsCodeTools', widthHeight=(500, 600), sizeable=False)
        cmds.rowColumnLayout('mainLayout',numberOfColumns=1, columnWidth=[(1, 500)])
        tabs = cmds.tabLayout('tabsNameSpace', innerMarginWidth=500, innerMarginHeight=600)

        child1 = cmds.rowColumnLayout(numberOfColumns=1, columnWidth=[(1, 500)])
        cmds.text('TD `s  Code  Uploader', height=30, backgroundColor=[0.5, 0.5, 0.6])
        cmds.separator(style='out')
        cmds.text(' ')
        self.identityConfirm()
        cmds.text(' ')
        cmds.separator(style='out')
        cmds.text(' ')
        self.jobList = self.jobConfirm()
        cmds.text(' ')
        cmds.separator(style='out')
        cmds.text(' ')
        
        cmds.rowColumnLayout(numberOfColumns=3, columnWidth=[(1, 100), (2, 290),(3,100)])
        cmds.text('Name your code:', align='left', enable=self.flag[2])
        cmds.textField('codeName', enable=self.flag[2])
        cmds.button(label='TestScript', command=lambda *arg: self.testScript())
        cmds.setParent('..')
        
        cmds.text(' ')
        cmds.scrollField('copyText', parent=child1, editable=True, wordWrap=False, text='Copy your code and paste here', enable=self.flag[3],height=380)
        cmds.text(' ')
        cmds.text(' ')

        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 340), (2, 150)])
        cmds.text('You can select your code in your computer too :')
        cmds.button(label='Browse Folder', enable=self.flag[4], command=lambda *arg:self.folderPathFinder())
        cmds.text(' ')
        cmds.text(' ')
        cmds.button(parent=child1, label='Upload', height=40, enable=self.flag[5], command=lambda *arg: self.codeUpload())
        cmds.setParent('..')
        
        cmds.setParent('..')
        
        child2 = cmds.rowColumnLayout(numberOfColumns=1, columnWidth=[(1, 500)])
        cmds.text('TD `s  Code  Manager', height=30, backgroundColor=[0.6, 0.5, 0.5])
        cmds.separator(style='out')

        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 250),(2,120)])
        cmds.rowColumnLayout(numberOfColumns=1, columnWidth=[(1, 250)])
        cmds.text(' ')
        self.jobList2 = self.jobConfirm2()
        cmds.text(' ')
        cmds.setParent('..')
        cmds.rowColumnLayout(numberOfColumns=1, columnWidth=[(1, 120)])
        cmds.text(' ')
        cmds.button(label='Refresh', height=40, width=100, enable=self.flag[5], command=lambda *arg: self.scrollMenuReflesh())
        cmds.text(' ')
        cmds.setParent('..')

        cmds.setParent('..')

        cmds.separator(style='out')
        cmds.text(' ')

        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 400),(2,100)])
        cmds.scrollLayout('scrollLayoutUnique', width=300, height=120)
        self.showCodeList()
        cmds.setParent('..')

        cmds.rowColumnLayout(numberOfRows=5, rowHeight=[(1, 30),(2,30),(3,30),(4,30)])
        cmds.button(label='Edit', width=100, enable=self.flag[5], command=lambda *arg: self.browseCode())
        cmds.button(label='Delete', width=100, enable=self.flag[5], command=lambda *arg: self.deleteCode())
        cmds.button(label='back up', width=100, enable=self.flag[5], command=lambda *arg: self.backUpAllFileGui())
        cmds.button(label='Active', width=100, enable=self.flag[5], command=lambda *arg: self.activeCode())
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.text(parent=child2,label=' ')
        cmds.separator(style='out')
        cmds.text(parent=child2,label=' ')

        cmds.rowColumnLayout(numberOfColumns=3, columnWidth=[(1, 45), (2, 350),(3,95)])
        cmds.text(label='Name:',align='left')
        cmds.textField('showTextName', enable=self.flag[4],width=300)
        cmds.button(label='TestScript', command=lambda *arg: self.testScript2())
        cmds.setParent('..')

        cmds.text(parent=child2,label=' ')
        cmds.scrollField('showText', parent=child2, editable=True, wordWrap=False, enable=self.flag[3],height=350)
        cmds.text(' ')
        cmds.button(label='Update',height=40, command=lambda *arg: self.updateCode())
        cmds.setParent('..')

        cmds.tabLayout(tabs, edit=True, tabLabel=((child1, 'Upload your code'), (child2, 'Manage your code')) )
        cmds.showWindow(TDsCodeUploadTool)

    def rightConfirm(self):
        #flag[0]->project right
        #flag[1]->module right
        #flag[2]->name right
        #flag[3]->copy-text right
        #flag[4]->floder right
        #flag[5]->upload right
        if self.department == 'PipelineTD':
            self.flag[0] = True
            self.flag[1] = True
            self.flag[2] = True
            self.flag[3] = True
            self.flag[4] = True
            self.flag[5] = True
        elif self.department == 'RnD':
            self.flag[0] = True
            self.flag[1] = True
            self.flag[2] = True
            self.flag[3] = True
            self.flag[4] = True
            self.flag[5] = True
        elif self.appointment == 'TD':
            self.flag[0] = True
            self.flag[1] = False
            self.flag[2] = True
            self.flag[3] = True
            self.flag[4] = True
            self.flag[5] = True
        else:
            self.flag[0] = False
            self.flag[1] = False
            self.flag[2] = False
            self.flag[3] = False
            self.flag[4] = False
            self.flag[5] = False

    def identityConfirm(self):
        if self.appointment == '':
           self.appointment = u'组员'
        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 100),(2,100)])
        cmds.text('  Your Department :',align='left')
        cmds.text(self.department)
        cmds.setParent('..')
        cmds.text(' ')
        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 100),(2,100)] )
        cmds.text('  Your Appointment :',align='left')
        cmds.text(self.appointment)
        cmds.setParent('..')

    def jobConfirm(self):
        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 70),(2,150)] )
        cmds.text('Project :',align='left')
        cmds.optionMenu('optionProject', enable=self.flag[0])
        projectList = self.projectMenuItem()
        cmds.setParent('..')
        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 70),(2,150)] )
        cmds.text('Module :',align='left')
        cmds.optionMenu('optionModule',enable = self.flag[1])
        moduleList = self.moduleMenuItem()
        cmds.setParent('..')
        return projectList,moduleList

    def projectMenuItem(self):
        Prj = Get.Get().GetActiveProject()
        ServerRoot = Path.Path().GetServerPath()
        listDir = os.listdir('%s/'%ServerRoot)
        projectList = []
        for item in listDir:
            testList = item.split('.')
            if len(testList) == 1:
                if testList[0] == 'backUp':
                    pass
                else:
                    projectList.append(item)
            else:
                pass
        for i in range(len(projectList)):
            if projectList[i] == Prj:
                tmp = projectList[0]
                projectList[0] = projectList[i]
                projectList[i] = tmp
        for item in projectList:
            cmds.menuItem(label=item)
        return projectList

    def moduleMenuItem(self):
        moduleList = self.moduleList
        for i in range(len(moduleList)):
            if moduleList[i] == self.department:
                tmp = moduleList[0]
                moduleList[0] = moduleList[i]
                moduleList[i] = tmp
        for item in moduleList:
            cmds.menuItem(label=item)
        return moduleList

    def testScript(self):
        codeName = cmds.textField('codeName', text=True, q=True)
        codeType = codeName.split('.')[-1]
        if codeType == 'py':
            self.codeType = 'python'
        elif codeType == 'mel':
            self.codeType = 'mel'
        else:
            pass

        scriptText = cmds.scrollField('copyText', q=True, text=True)
        if scriptText == 'Copy your code and paste here' or scriptText == '':
            return
        else:
            cmds.cmdScrollFieldExecuter('scriptTextTest', parent='mainLayout', text=scriptText, execute=True, sourceType=self.codeType)
            #~ cmds.deleteUI('scriptTextTest')
            cmds.deleteUI('scriptTextTest')

    def folderPathFinder(self):
        multipleFilters = "Python (*.py);;Maya mel (*.mel)"
        filePath = cmds.fileDialog2(fileMode=True, fileFilter=multipleFilters, caption='Find your code')

        if not filePath == '':
            try:
                tmpName = filePath[0].split('/')
            except:
                sys.exit()
            name = tmpName[-1]
            file = open(filePath[0], 'r').read()
            try:
                cmds.scrollField('copyText', text=file, edit=True)
            except UnicodeDecodeError:
                cmds.scrollField('copyText', text=file, edit=True)
            cmds.textField('codeName', text=name, edit=True)
            #Test the code type

    def historyCopy(self,argName,argLocalPath):
        tmp1_current_time = str(datetime.datetime.now())
        tmp2_current_time = tmp1_current_time.replace(' ', '_')
        tmp3_current_time = tmp2_current_time.split('.')
        tmp4_current_time = tmp3_current_time[0].split(':')
        current_time = tmp4_current_time[0]+'-'+tmp4_current_time[1]+'-'+tmp4_current_time[2]
        name = current_time+'&'+argName

        projectList = self.jobList[0]
        moduleList = self.jobList[1]
        projectOption=cmds.optionMenu('optionProject', q=True, select=True)
        moduleOption = cmds.optionMenu('optionModule', q=True, select=True)
        project = projectList[int(projectOption)-1]
        module = moduleList[int(moduleOption)-1]
        LocalRoot = Path.Path().GetLocalPath()
        ServerRoot = Path.Path().GetServerPath()
        LocalRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/backup/'%(LocalRoot,project, module)
        ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/backup/'%(ServerRoot,project, module)
        uploadTime = time.time()

        if os.path.isdir(ServerRootPath):
            pass
        else:
            lsrunas.lsrunas().CreateDir_1(ServerRootPath)

        if os.path.isdir(LocalRootPath):
            pass
        else:
            os.makedirs(LocalRootPath)

        shutil.copy2(argLocalPath+argName,LocalRootPath+name)
        lsrunas.lsrunas(uploadTime).UploadFile(LocalRootPath+name, ServerRootPath+name, DeleteSource=False)

    def codeUpload(self):
        projectList = self.jobList[0]
        moduleList = self.jobList[1]
        projectOption=cmds.optionMenu('optionProject', q=True, select=True)
        moduleOption = cmds.optionMenu('optionModule', q=True, select=True)
        project = projectList[int(projectOption)-1]
        module = moduleList[int(moduleOption)-1]

        fileName = cmds.textField('codeName', q=True, text=True)
        copyText = cmds.scrollField('copyText', q=True, text=True)
        name = fileName


        LocalRoot = Path.Path().GetLocalPath()
        ServerRoot = Path.Path().GetServerPath()
        LocalRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(LocalRoot,project, module)
        ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(ServerRoot,project, module)
        uploadTime = time.time()

        try:
            file = open(ServerRootPath+'activeMap.txt', 'rb')
            activeMap = json.load(file)
            file.close()
        except:
            activeMap = {}

        if os.path.isdir(ServerRootPath):
            pass
        else:
            lsrunas.lsrunas().CreateDir_1(ServerRootPath)

        if os.path.isdir(LocalRootPath):
            pass
        else:
            os.makedirs(LocalRootPath)



        #upload the copy code
        if os.path.isfile(ServerRootPath+name):
            cmds.confirmDialog(title='Oops', message='The name has existed, rename it!', button='Gotcha')
            return
        else:
            file = open(LocalRootPath+name, 'w')
            file.write(copyText)
            file.close()
            lsrunas.lsrunas(uploadTime).UploadFile(LocalRootPath+name, ServerRootPath+name, DeleteSource=False)

        cmds.textField('codeName', text='', edit=True)
        cmds.scrollField('copyText', text='', edit=True)

        cmds.confirmDialog(title='Good Job', message='Job Done!    \n\n%s'%name, button='Gotcha')

        """上传activeMap到服务器"""
        #activeMap[name] = [Active,Share]
        activeMap[name] = [True,False]
        file = open(LocalRootPath+'activeMap.txt', 'wb')
        json.dump(activeMap, file)
        file.close()
        if not os.path.isdir(os.path.dirname(ServerRootPath)):
            lsrunas.lsrunas().CreateDir_1(os.path.dirname(ServerRootPath))
        lsrunas.lsrunas(uploadTime).UploadFile(LocalRootPath+'activeMap.txt', ServerRootPath+'activeMap.txt', DeleteSource=False)

        self.historyCopy(name, LocalRootPath)

    def projectMenuItem2(self):
        Prj = Get.Get().GetActiveProject()
        listDir = os.listdir('//hnas01/data/Projects/')
        projectList = []
        for item in listDir:
            testList = item.split('.')
            if len(testList) == 1:
                if testList[0] == 'backUp':
                    pass
                else:
                    projectList.append(item)
            else:
                pass
        for i in range(len(projectList)):
            if projectList[i] == Prj:
                tmp = projectList[0]
                projectList[0] = projectList[i]
                projectList[i] = tmp
        for item in projectList:
            cmds.menuItem(label=item)
        return projectList

    def moduleMenuItem2(self):
        moduleList = self.moduleList
        for i in range(len(moduleList)):
            if moduleList[i] == self.department:
                tmp = moduleList[0]
                moduleList[0] = moduleList[i]
                moduleList[i] = tmp
        for item in moduleList:
            cmds.menuItem(label=item)
        return moduleList

    def jobConfirm2(self):
        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 70),(2,150)] )
        cmds.text('Project :',align='left')
        cmds.optionMenu('optionProject2', enable=self.flag[0], changeCommand=lambda *arg:self.scrollMenuReflesh())
        projectList = self.projectMenuItem2()
        cmds.setParent('..')
        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 70),(2,150)] )
        cmds.text('Module :',align='left')
        cmds.optionMenu('optionModule2', enable=self.flag[1], changeCommand=lambda *arg:self.scrollMenuReflesh())
        moduleList = self.moduleMenuItem2()
        cmds.setParent('..')
        return projectList,moduleList

    def scrollMenuReflesh(self):
        projectList = self.jobList2[0]
        moduleList = self.jobList2[1]
        projectOption=cmds.optionMenu('optionProject2', q=True, select=True)
        moduleOption = cmds.optionMenu('optionModule2', q=True, select=True)
        project = projectList[int(projectOption)-1]
        module = moduleList[int(moduleOption)-1]
        ServerRoot = Path.Path().GetServerPath()

        pythonCodeFilePath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(ServerRoot,project, module)

        #print pythonCodeFilePath
        try:
            tmp_listDir = os.listdir(pythonCodeFilePath)
        except:
            tmp_listDir = []
        listDir = []
        for item in tmp_listDir:
            tmp = item.split('.')
            if tmp[-1] == 'py' or tmp[-1] == 'mel':
                listDir.append(item)
            else:
                pass

        layoutList = cmds.scrollLayout('scrollLayoutUnique', childArray=True, q=True)
        if layoutList == None:
            pass
        else:
            for item in layoutList:
                cmds.deleteUI(item)
        cmds.iconTextRadioCollection('pythonCodeList', edit=True)
        self.id = -1
        for i in range(len(listDir)):
            self.id += 1
            cmds.rowColumnLayout(parent='scrollLayoutUnique', numberOfColumns=4, columnWidth=[(1, 265),(2,40),(3,20),(4,45)])
            cmds.iconTextRadioButton('id%s'%i,collection='pythonCodeList', st='iconAndTextHorizontal', visible=True, l=listDir[i], width=350, height=25)
            cmds.text('active:',enable=False)
            cmds.checkBox('id%s'%i, l='', height=25, visible=True, value=True)
            cmds.button(label='...', command=lambda *arg: self.backUpGui())
            cmds.setParent('..')



        #改变文件的列表的checkBox的显示
        self.oldActiveMap = {}

        if len(listDir) == 0:
            pass
        else:
            try:
                file = open(pythonCodeFilePath+'activeMap.txt', 'rb')
                activeMap = json.load(file)
                file.close()
            except:
                sys.exit()
            for i in range(self.id+1):
                key = cmds.iconTextRadioButton('id%s'%i, l=True, q=True)
                try:
                    mapValue = activeMap[key][0]
                except:
                    activeMap[key]=[True,False]
                    mapValue=True
                cmds.checkBox('id%s'%i, value=mapValue, e=True)
                self.oldActiveMap[key] = mapValue

    def showCodeList(self):
        projectList = self.jobList2[0]
        moduleList = self.jobList2[1]
        projectOption=cmds.optionMenu('optionProject2', q=True, select=True)
        moduleOption = cmds.optionMenu('optionModule2', q=True, select=True)
        project = projectList[int(projectOption)-1]
        module = moduleList[int(moduleOption)-1]
        ServerRoot = Path.Path().GetServerPath()

        pythonCodeFilePath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(ServerRoot,project, module)

        #print pythonCodeFilePath
        if os.path.isdir(pythonCodeFilePath):
            pass
        else:
            lsrunas.lsrunas().CreateDir_1(pythonCodeFilePath)

        tmp_listDir = os.listdir(pythonCodeFilePath)
        listDir = []
        for item in tmp_listDir:
            tmp = item.split('.')
            if tmp[-1] == 'py' or tmp[-1] == 'mel':
                listDir.append(item)
            else:
                pass
        cmds.iconTextRadioCollection('pythonCodeList')
        self.id = -1
        for i in range(len(listDir)):
            self.id += 1
            cmds.rowColumnLayout(parent='scrollLayoutUnique', numberOfColumns=4, columnWidth=[(1, 265),(2,40),(3,20),(4,45)])
            cmds.iconTextRadioButton('id%s'%i, collection='pythonCodeList', st='iconAndTextHorizontal', visible=True, l=listDir[i], width=350, height=25)
            cmds.text('active:',enable=False)
            cmds.checkBox('id%s'%i, l='', height=25, visible=True, value=True)
            cmds.button(label='...', command=lambda *arg: self.backUpGui())
            cmds.setParent('..')


        #改变文件的列表的checkBox的显示
        if len(listDir) == 0:
            pass
        else:
            file = open(pythonCodeFilePath+'activeMap.txt', 'rb')
            activeMap = json.load(file)
            file.close()
            for i in range(self.id+1):
                key = cmds.iconTextRadioButton('id%s'%i, l=True, q=True)
                try:
                    mapValue = activeMap[key][0]
                except:
                    mapValue = True
                cmds.checkBox('id%s'%i, value=bool(mapValue), e=True)
                self.oldActiveMap[key] = mapValue

    def backUpGui(self):
        projectList = self.jobList2[0]
        moduleList = self.jobList2[1]
        projectOption=cmds.optionMenu('optionProject2', q=True, select=True)
        moduleOption = cmds.optionMenu('optionModule2', q=True, select=True)
        project = projectList[int(projectOption)-1]
        module = moduleList[int(moduleOption)-1]
        ServerRoot = Path.Path().GetServerPath()
        ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/backup/'%(ServerRoot,project, module)

        ids = cmds.iconTextRadioCollection('pythonCodeList', select=True, q=True)
        try:
            codeName = cmds.iconTextRadioButton(ids, label=True, q=True)
        except RuntimeError:
            sys.exit()
        if os.path.isdir(ServerRootPath):
            pass
        else:
            lsrunas.lsrunas().CreateDir_1(ServerRootPath)

        listAllDir = os.listdir(ServerRootPath)
        listDir = []
        for item in listAllDir:
            tmp=item.split('&')
            if tmp[-1] == codeName:
                listDir.append(item)

        if cmds.window('backUpWindow', q=1, exists=1):
            cmds.deleteUI('backUpWindow')
        backUpWindow = cmds.window('backUpWindow', title='Backup', widthHeight=(300, 300), sizeable=False)
        cmds.rowColumnLayout('backUpLayout', numberOfColumns=1, columnWidth=[(1, 300)])
        cmds.text('History Code Backuper', height=30, backgroundColor=[0.5, 0.6, 0.5])
        cmds.separator(style='out')
        cmds.text(' ')
        cmds.separator(style='out')
        cmds.text(' ')
        cmds.rowColumnLayout(parent='backUpLayout', numberOfColumns=3, columnWidth=[(1, 160),(2,70),(3,50)])
        cmds.text(label='Share your code with others :', align='left')
        cmds.checkBox('shareCheckBox', l='Not now', height=25, enable=False, value=False)
        cmds.button(label='change', command=lambda *arg: self.shareChange())
        cmds.setParent('..')
        cmds.text(' ')
        cmds.separator(style='out')
        cmds.text(label='Code`s History:', align='left')
        cmds.text(' ')
        cmds.scrollLayout('historyScrollLayout', parent='backUpLayout', width=300, height=120)
        cmds.iconTextRadioCollection('historyCodeList')
        for i in range(len(listDir)):
            cmds.iconTextRadioButton(listDir[i], collection='historyCodeList', st='iconAndTextHorizontal', visible=True, l=listDir[i], width=270, height=25)
        cmds.separator(style='out')
        cmds.setParent('..')

        cmds.text(' ')
        cmds.button(label='BackUp',height=30, command=lambda *arg: self.backUpFunc())
        cmds.text(' ')
        cmds.showWindow(backUpWindow)

    def backUpFunc(self):
        projectList = self.jobList2[0]
        moduleList = self.jobList2[1]
        projectOption=cmds.optionMenu('optionProject2', q=True, select=True)
        moduleOption = cmds.optionMenu('optionModule2', q=True, select=True)
        project = projectList[int(projectOption)-1]
        module = moduleList[int(moduleOption)-1]
        ServerRoot = Path.Path().GetServerPath()
        LocalRoot = Path.Path().GetLocalPath()
        ServerBackUpPath = '%s/%s/ProjectConfig/workNodeScripts/%s/backup/'%(ServerRoot,project, module)

        ids = cmds.iconTextRadioCollection('historyCodeList', select=True, q=True)
        try:
            backup_codeName = cmds.iconTextRadioButton(ids, label=True, q=True)
        except RuntimeError:
            cmds.confirmDialog(title='Erro', message='Please select a file first!', button='Gotcha')
            sys.exit()
        pass

        codeName = backup_codeName.split('&')[-1]
        file = open(ServerBackUpPath+backup_codeName,'r').read()
        cmds.textField('showTextName', enable=self.flag[4], text=codeName, edit=True)
        cmds.scrollField('showText', enable=self.flag[3], text=file.decode('utf-8'), edit=True)
        self.textOldFileName = codeName
        cmds.deleteUI('backUpWindow')

    def backUpAllFileGui(self):
        projectList = self.jobList2[0]
        moduleList = self.jobList2[1]
        projectOption=cmds.optionMenu('optionProject2', q=True, select=True)
        moduleOption = cmds.optionMenu('optionModule2', q=True, select=True)
        project = projectList[int(projectOption)-1]
        module = moduleList[int(moduleOption)-1]
        ServerRoot = Path.Path().GetServerPath()
        ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/backup/'%(ServerRoot,project, module)

        if os.path.isdir(ServerRootPath):
            pass
        else:
            lsrunas.lsrunas().CreateDir_1(ServerRootPath)

        listAllDir = os.listdir(ServerRootPath)
        listAllDir.sort()
        if cmds.window('backUpAllFileWindow', q=1, exists=1):
            cmds.deleteUI('backUpAllFileWindow')
        backUpAllFileWindow = cmds.window('backUpAllFileWindow', title='Backup', widthHeight=(300, 300), sizeable=False)
        cmds.rowColumnLayout('backUpAllLayout', numberOfColumns=1, columnWidth=[(1, 300)])
        cmds.text('History All Code Backuper', height=30, backgroundColor=[0.5, 0.6, 0.6])
        cmds.separator(style='out')
        cmds.text(' ')
        cmds.text(' ')
        cmds.separator(style='out')
        cmds.text(label='All Code`s History:', align='left')
        cmds.text(' ')

        cmds.scrollLayout('historyAllScrollLayout', parent='backUpAllLayout', width=300, height=400)
        cmds.iconTextRadioCollection('historyAllCodeList')
        for i in range(len(listAllDir)):
            cmds.iconTextRadioButton(listAllDir[i], collection='historyAllCodeList', st='iconAndTextHorizontal', visible=True, l=listAllDir[i], width=270, height=25)
        cmds.separator(style='out')
        cmds.setParent('..')

        cmds.text(' ')
        cmds.button(label='BackUp',height=30, command=lambda *arg: self.backUpAllFunc())
        cmds.text(' ')
        cmds.showWindow(backUpAllFileWindow)

    def backUpAllFunc(self):
        projectList = self.jobList2[0]
        moduleList = self.jobList2[1]
        projectOption=cmds.optionMenu('optionProject2', q=True, select=True)
        moduleOption = cmds.optionMenu('optionModule2', q=True, select=True)
        project = projectList[int(projectOption)-1]
        module = moduleList[int(moduleOption)-1]
        ServerRoot = Path.Path().GetServerPath()
        LocalRoot = Path.Path().GetLocalPath()
        ServerBackUpPath = '%s/%s/ProjectConfig/workNodeScripts/%s/backup/'%(ServerRoot,project, module)
        LocalRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(LocalRoot,project, module)
        ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(ServerRoot,project, module)
        uploadTime = time.time()

        ids = cmds.iconTextRadioCollection('historyAllCodeList', select=True, q=True)
        try:
            backupAll_codeName = cmds.iconTextRadioButton(ids, label=True, q=True)
        except RuntimeError:
            cmds.confirmDialog(title='Erro', message='Please select a file first!', button='Gotcha')
            sys.exit()
        pass

        file = open(ServerRootPath+'activeMap.txt', 'rb')
        activeMap = json.load(file)
        file.close()

        name = backupAll_codeName.split('&')[-1]

        if os.path.isfile(ServerRootPath+name):
            cmds.confirmDialog(title='Oops', message='The name has existed, you can`t backup your code!', button='Gotcha')
            sys.exit()
        else:
            file = open(ServerBackUpPath+backupAll_codeName, 'r')
            copyText = file.readlines()
            file.close()
            file = open(LocalRootPath+name, 'w')
            file.writelines(copyText)
            file.close()
            lsrunas.lsrunas(uploadTime).UploadFile(LocalRootPath+name, ServerRootPath+name, DeleteSource=False)
            #activeMap[name] = [Active,Share]
            activeMap[name] = [True,False]
            file = open(LocalRootPath+'activeMap.txt', 'wb')
            json.dump(activeMap, file)
            file.close()
            if not os.path.isdir(os.path.dirname(ServerRootPath)):
                lsrunas.lsrunas().CreateDir_1(os.path.dirname(ServerRootPath))
            lsrunas.lsrunas(uploadTime).UploadFile(LocalRootPath+'activeMap.txt', ServerRootPath+'activeMap.txt', DeleteSource=False)

        self.historyCopy(name, LocalRootPath)
        file = open(ServerBackUpPath+backupAll_codeName,'r').read()
        cmds.textField('showTextName', enable=self.flag[4], text=name, edit=True)
        cmds.scrollField('showText', enable=self.flag[3], text=file.decode('utf-8'), edit=True)
        self.textOldFileName = name
        cmds.deleteUI('backUpAllFileWindow')
        self.scrollMenuReflesh()

    def shareChange(self):
        pass

    def testScript2(self):
        codeName = cmds.textField('showTextName', text=True, q=True)
        codeType = codeName.split('.')[-1]
        if codeType == 'py':
            self.codeType = 'python'
        elif codeType == 'mel':
            self.codeType = 'mel'
        else:
            pass


        scriptText = cmds.scrollField('showText', q=True, text=True)
        if scriptText == 'Copy your code and paste here' or scriptText == '':
            pass
        else:
            cmds.cmdScrollFieldExecuter('scriptTextTest2', parent='tabsNameSpace', text=scriptText, execute=True, sourceType=self.codeType)
            cmds.deleteUI('scriptTextTest2')
        #import maya.mel as mel
        #mel.eval("python \"%s\""%scriptText)
        #exec(scriptText)

    def changeCheckBoxMark(self):
        testMap = {}
        self.changeBoxMap = {}
        projectList = self.jobList2[0]
        moduleList = self.jobList2[1]
        projectOption=cmds.optionMenu('optionProject2', q=True, select=True)
        moduleOption = cmds.optionMenu('optionModule2', q=True, select=True)
        project = projectList[int(projectOption)-1]
        module = moduleList[int(moduleOption)-1]
        pythonCodeFilePath = '//hnas01/data/Projects/%s/ProjectConfig/workNodeScripts/%s/'%(project, module)
        tmp_listDir = os.listdir(pythonCodeFilePath)
        listDir = []
        for item in tmp_listDir:
            tmp = item.split('.')
            if tmp[-1] == 'py':
                listDir.append(item)
            else:
                pass

        for i in range(self.id+1):
            key = cmds.iconTextRadioButton('id%s'%i, l=True, q=True)
            value = cmds.checkBox('id%s'%i, value=True, q=True)
            testMap[key] = value

        print testMap
        for item in list(testMap.keys()):
            if not self.oldActiveMap[item] == None and not self.oldActiveMap[item] == testMap[item]:
                 self.changeBoxMap[item] = testMap[item]
            else:
                pass

    def browseCode(self):
        projectList = self.jobList2[0]
        moduleList = self.jobList2[1]
        projectOption=cmds.optionMenu('optionProject2', q=True, select=True)
        moduleOption = cmds.optionMenu('optionModule2', q=True, select=True)
        project = projectList[int(projectOption)-1]
        module = moduleList[int(moduleOption)-1]

        ids = cmds.iconTextRadioCollection('pythonCodeList', select=True, q=True)
        try:
            codeName = cmds.iconTextRadioButton(ids, label=True, q=True)
        except RuntimeError:
            sys.exit()
        except IOError:
            sys.exit()
        textFilePath = '//hnas01/data/Projects/%s/ProjectConfig/workNodeScripts/%s/%s'%(project, module, codeName)
        self.textOldFilePath = textFilePath
        self.textOldFileName = codeName
        file = open(textFilePath, 'r').read()
        cmds.textField('showTextName', enable=self.flag[4], text=codeName, edit=True)
        try:
            cmds.scrollField('showText', enable=self.flag[3], text=file.decode('utf-8'), edit=True)
        except UnicodeDecodeError:
            cmds.confirmDialog(title='Erro', message='Your code is not UTF-8 unicode!', button='Gotcha')

    def deleteCode(self):
        projectList = self.jobList2[0]
        moduleList = self.jobList2[1]
        projectOption=cmds.optionMenu('optionProject2', q=True, select=True)
        moduleOption = cmds.optionMenu('optionModule2', q=True, select=True)
        project = projectList[int(projectOption)-1]
        module = moduleList[int(moduleOption)-1]
        ServerRoot = Path.Path().GetServerPath()
        ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(ServerRoot,project, module)

        ids = cmds.iconTextRadioCollection('pythonCodeList', select=True, q=True)
        try:
            codeName = cmds.iconTextRadioButton(ids, label=True, q=True)
        except RuntimeError:
            sys.exit()
        except IOError:
            sys.exit()
        yourChoice = cmds.confirmDialog(title='CLOSED', message='You sure you want to delete the code?', button=['Yes', 'No'], defaultButton='Yes', cancelButton='No', dismissString='No' )
        if yourChoice == 'Yes':
            lsrunas.lsrunas().deleteFile(ServerRootPath+codeName)
            cmds.textField('showTextName',text='', e=True)
            cmds.scrollField('showText', text='', e=True)
        else:
            print 'Wise choise'
        time.sleep(2)
        self.scrollMenuReflesh()

    def activeCode(self):
        projectList = self.jobList2[0]
        moduleList = self.jobList2[1]
        projectOption=cmds.optionMenu('optionProject2', q=True, select=True)
        moduleOption = cmds.optionMenu('optionModule2', q=True, select=True)
        project = projectList[int(projectOption)-1]
        module = moduleList[int(moduleOption)-1]
        LocalRoot = Path.Path().GetLocalPath()
        ServerRoot = Path.Path().GetServerPath()
        LocalRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(LocalRoot,project, module)
        ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(ServerRoot,project, module)
        uploadTime = time.time()

        if os.path.isdir(ServerRootPath):
            pass
        else:
            lsrunas.lsrunas().CreateDir_1(ServerRootPath)

        if os.path.isdir(LocalRootPath):
            pass
        else:
            os.makedirs(LocalRootPath)

        #do active code,read and write the map
        if os.path.isfile(ServerRootPath+'activeMap.txt'):
            file = open(ServerRootPath+'activeMap.txt', 'rb')
            activeMap = json.load(file)
            file.close()
        else:
            activeMap = {}
        self.changeCheckBoxMark()
        for key in list(self.changeBoxMap.keys()):
            value = self.changeBoxMap[key]
            activeMap[key] = [value]
        print(activeMap)
        file = open(LocalRootPath+'activeMap.txt', 'wb')
        json.dump(activeMap, file)
        file.close()
        lsrunas.lsrunas(uploadTime).UploadFile(LocalRootPath+'activeMap.txt', ServerRootPath+'activeMap.txt', DeleteSource=False)
        cmds.confirmDialog(title='Good Job', message='Active completed!', button='Gotcha')

        #refresh oldActiveMap`s values
        file = open(ServerRootPath+'activeMap.txt', 'rb')
        activeMap = json.load(file)
        file.close()
        for i in range(self.id+1):
            key = cmds.iconTextRadioButton('id%s'%i, l=True, q=True)
            mapValue = activeMap[key][0]
            self.oldActiveMap[key] = mapValue

    def updateCode(self):
        projectList = self.jobList2[0]
        moduleList = self.jobList2[1]
        projectOption=cmds.optionMenu('optionProject2', q=True, select=True)
        moduleOption = cmds.optionMenu('optionModule2', q=True, select=True)
        project = projectList[int(projectOption)-1]
        module = moduleList[int(moduleOption)-1]
        LocalRoot = Path.Path().GetLocalPath()
        ServerRoot = Path.Path().GetServerPath()
        LocalRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(LocalRoot,project, module)
        ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(ServerRoot,project, module)
        uploadTime = time.time()

        if os.path.isdir(ServerRootPath):
            pass
        else:
            lsrunas.lsrunas().CreateDir_1(ServerRootPath)

        if os.path.isdir(LocalRootPath):
            pass
        else:
            os.makedirs(LocalRootPath)


        try:
            tmp_codeName = cmds.textField('showTextName',text=True, q=True)
            codeNameContent = cmds.scrollField('showText', text=True, q=True)
            codeNameList = tmp_codeName.split('.')
            codeName = codeNameList[0]+'.py'
        except RuntimeError:
            cmds.confirmDialog(title='Oops', message='Your code`s name is empty!', button='Gotcha')

        file = open(LocalRootPath+codeName, 'w')
        file.writelines(codeNameContent)
        file.close()
        lsrunas.lsrunas(uploadTime).UploadFile(LocalRootPath+codeName, ServerRootPath+codeName, DeleteSource=False)
        if self.textOldFileName == codeName:
            pass
        else:
            lsrunas.lsrunas().deleteFile(ServerRootPath+self.textOldFileName)
            time.sleep(2)

        #refresh activeMap
        file = open(ServerRootPath+'activeMap.txt', 'rb')
        activeMap = json.load(file)
        file.close()
        oldKey = self.textOldFileName
        key = codeName
        value = activeMap[oldKey][0]
        activeMap[key] = [value]
        self.oldActiveMap[key] = value
        if not oldKey == key:
            #del activeMap[oldKey]
            del self.oldActiveMap[oldKey]
        else:
            pass
        file = open(LocalRootPath+'activeMap.txt', 'wb')
        json.dump(activeMap, file)
        file.close()
        lsrunas.lsrunas(uploadTime).UploadFile(LocalRootPath+'activeMap.txt', ServerRootPath+'activeMap.txt', DeleteSource=False)

        self.scrollMenuReflesh()
        cmds.textField('showTextName', text='', e=True)
        cmds.scrollField('showText', text='', e=True)
        cmds.confirmDialog(title='Good Job', message='Update completed!', button='Gotcha')

        if not codeName == '':
            self.historyCopy(codeName,LocalRootPath)
        else:
            pass

if __name__ == '__main__':
    TDsCodeUpload()



