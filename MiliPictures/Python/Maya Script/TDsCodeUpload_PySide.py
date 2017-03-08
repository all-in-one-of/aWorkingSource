#coding:utf-8
__author__ = 'kai.xu'

from PySide import QtCore,QtGui
import os,sys,json,time,ConfigParser

config = ConfigParser.ConfigParser()
config.read("Environment.ini")
LOCALROOTPATH = config.get("filepath","LOCALROOTPATH")
SERVERROOTPATH = config.get("filepath","SERVERROOTPATH")
DEPARTMENT = config.get("filepath","DEPARTMENT")
APPOINTMENT = config.get("filepath","APPOINTMENT")
PROJECT = config.get("filepath","PROJECT")
MODULELIST = config.get("filepath","MODULELIST")
PROJECTLIST = config.get("filepath","PROJECTLIST")
IMAGEPATH = config.get("imagepath","IMAGEPATH")

class theButton(QtGui.QPushButton):
    signal = QtCore.Signal()
    def __init__(self):
        self.value = False
        super(theButton,self).__init__()

    def mouseDoubleClickEvent(self,event):
        self.signal.emit()

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

class TDsCodeUpload(QtGui.QDialog):
    def __init__(self,env=None):
        super(TDsCodeUpload,self).__init__()
        self.env = env
        self.image_path = IMAGEPATH
        if self.env == "maya.exe":
            import Mili.Module.utilties.MiliUser as MiliUser
            import Mili.Module.utilties.lsrunas as lsrunas
            import Mili.Module.utilties.Get as Get
            import Mili.Module.utilties.Path as Path
            reload(lsrunas)
            reload(MiliUser)
            reload(Get)
            reload(Path)

            self.department = MiliUser.MiliUser()['department']
            self.appointment = MiliUser.MiliUser()['appointment']
            self.moduleList = MiliUser.MiliUser().getGroupList()
            self.project = Get.Get().GetActiveProject()
            self.projectList = []
            self.LocalRoot = Path.Path().GetLocalPath()
            self.ServerRoot = Path.Path().GetServerPath()
            listDir = os.listdir('%s/'%self.ServerRoot)
            for item in listDir:
                testList = item.split('.')
                if len(testList) == 1:
                    if testList[0] == 'backUp':
                        pass
                    else:
                        self.projectList.append(item)
        else:
            self.department = DEPARTMENT
            self.appointment = APPOINTMENT
            self.project = PROJECT
            self.moduleList = eval(MODULELIST)
            self.projectList = eval(PROJECTLIST)
            self.LocalRoot = LOCALROOTPATH
            self.ServerRoot = SERVERROOTPATH

        self.showWindows()
        self.setInitValue()
        self.setUserRights()
    def showWindows(self):
        self.main_tabs = QtGui.QTabWidget()
        self.main_tabs.currentChanged.connect(self.refreshList)
        """Setting tab1 widget"""
        self.tab1 = QtGui.QWidget()
        main_layout_tabs1 = QtGui.QVBoxLayout()
        main_layout_tabs1.setContentsMargins(0,0,0,5)
        label_tab1 = QtGui.QLabel()
        palette1 = QtGui.QPalette()
        color1 = QtGui.QColor(0.5*255,0.5*255,0.6*255)
        palette1.setColor(QtGui.QPalette.Background,color1)
        color = QtGui.QColor(0,0,0)
        palette1.setColor(QtGui.QPalette.WindowText,color)
        label_tab1.setAutoFillBackground(True)
        label_tab1.setText("Codes Uploader")
        label_tab1.setPalette(palette1)
        label_tab1.setFixedHeight(30)
        label_tab1.setAlignment(QtCore.Qt.AlignCenter)
        label_tab1.setFont(QtGui.QFont("Rome times",10,QtGui.QFont.Bold))
        layout_HBox = QtGui.QHBoxLayout()
        layout_HBox.setContentsMargins(10,0,10,0)
        """layout_selection1"""
        layout_selection1 = QtGui.QGridLayout()
        label1_selection1 = QtGui.QLabel("Project :")
        label1_selection1.setFont(QtGui.QFont("Helvetica",8,QtGui.QFont.DemiBold))
        self.comboBox1_selection1 = QtGui.QComboBox()
        self.comboBox1_selection1.setFixedWidth(120)
        self.comboBox2_selection1 = QtGui.QComboBox()
        self.comboBox2_selection1.setFixedWidth(120)
        label2_selection1 = QtGui.QLabel("Module :")
        label2_selection1.setFont(QtGui.QFont("Helvetica",8,QtGui.QFont.DemiBold))
        layout_selection1.addWidget(label1_selection1,1,0)
        layout_selection1.addWidget(self.comboBox1_selection1,1,1)
        layout_selection1.addWidget(label2_selection1,2,0)
        layout_selection1.addWidget(self.comboBox2_selection1,2,1)
        """layout_information"""
        layout_information = QtGui.QGridLayout()
        label1_information = QtGui.QLabel("Department :")
        label1_information.setFont(QtGui.QFont("Helvetica",8,QtGui.QFont.DemiBold))
        self.label2_information = QtGui.QLabel("PipelineTD")
        label3_information = QtGui.QLabel("Appointment :")
        label3_information.setFont(QtGui.QFont("Helvetica",8,QtGui.QFont.DemiBold))
        self.label4_information = QtGui.QLabel(u"组员")
        layout_information.addWidget(label1_information,1,0)
        layout_information.addWidget(self.label2_information,1,1)
        layout_information.addWidget(label3_information,2,0)
        layout_information.addWidget(self.label4_information,2,1)
        group_box = QtGui.QGroupBox(u'User')
        group_box.setLayout(layout_information)
        """set layout_HBox"""
        layout_HBox.addLayout(layout_selection1)
        layout_HBox.addStretch()
        layout_HBox.addWidget(group_box)
        layout_HBox.addStretch()
        """layout_separation1"""
        layout_separation1 = QtGui.QHBoxLayout()
        layout_separation1.setContentsMargins(0,0,0,0)
        layout_separation2 = QtGui.QHBoxLayout()
        layout_separation2.setContentsMargins(0,0,0,0)
        label_separation1 = QtGui.QLabel()
        label_separation1.setAutoFillBackground(True)
        label_separation1.setFixedHeight(5)
        label_separation1.setPalette(palette1)
        label_separation2 = QtGui.QLabel()
        label_separation2.setAutoFillBackground(True)
        label_separation2.setFixedHeight(5)
        label_separation2.setPalette(palette1)
        layout_separation1.addWidget(label_separation1)
        layout_separation2.addWidget(label_separation2)
        """set layout_namefile"""
        layout_namefile = QtGui.QHBoxLayout()
        layout_namefile.setContentsMargins(10,10,10,10)
        label_namefile = QtGui.QLabel()
        label_namefile.setText("Name your code :")
        self.lineEdit_namefile = QtGui.QLineEdit()
        layout_namefile.addWidget(label_namefile)
        layout_namefile.addWidget(self.lineEdit_namefile)
        """layout_code_edit"""
        layout_code_edit = QtGui.QVBoxLayout()
        layout_code_edit.setContentsMargins(10,0,10,0)
        self.textEdit_code_edit = QtGui.QTextEdit()
        self.textEdit_code_edit.setLineWrapMode(QtGui.QTextEdit.NoWrap)
        self.textEdit_code_edit.setMinimumHeight(400)
        layout_code_edit.addWidget(self.textEdit_code_edit)
        """layout_tool_button"""
        layout_tool_button = QtGui.QHBoxLayout()
        layout_tool_button.setContentsMargins(10,0,10,0)
        button_tool_button = QtGui.QPushButton()
        button_tool_button.setIcon(QtGui.QIcon(self.image_path+"lightbulb.png"))
        button_tool_button.setText("Test Script")
        button_tool_button.clicked.connect(lambda *args:self.testScript(1))
        layout_tool_button.addWidget(button_tool_button)
        layout_tool_button.addStretch()
        """layout_file_browse"""
        layout_file_browse = QtGui.QHBoxLayout()
        layout_file_browse.setContentsMargins(10,0,10,0)
        self.lineEdit_file_browse = QtGui.QLineEdit()
        self.lineEdit_file_browse.setReadOnly(True)
        self.lineEdit_file_browse.setEnabled(False)
        button_file_browse = QtGui.QPushButton()
        button_file_browse.clicked.connect(self.browse)
        button_file_browse.setText("Browse")
        button_file_browse.setIcon(QtGui.QIcon(self.image_path+"folder.png"))
        layout_file_browse.addWidget(self.lineEdit_file_browse)
        layout_file_browse.addWidget(button_file_browse)
        """layout_upload_button"""
        layout_upload_button = QtGui.QVBoxLayout()
        layout_upload_button.setContentsMargins(10,0,10,0)
        self.button_upload_button = QtGui.QPushButton()
        self.button_upload_button.clicked.connect(self.upload)
        self.button_upload_button.setIcon(QtGui.QIcon(self.image_path+"rocket.png"))
        self.button_upload_button.setIconSize(QtCore.QSize(40,40))
        self.button_upload_button.setText("Upload")
        self.button_upload_button.setFont(QtGui.QFont("Roman times",9,QtGui.QFont.Bold))
        layout_upload_button.addWidget(self.button_upload_button)
        """set main_layout_tabs1"""
        main_layout_tabs1.addWidget(label_tab1)
        main_layout_tabs1.addLayout(layout_HBox)
        main_layout_tabs1.addLayout(layout_separation1)
        main_layout_tabs1.addLayout(layout_namefile)
        main_layout_tabs1.addLayout(layout_code_edit)
        main_layout_tabs1.addLayout(layout_tool_button)
        main_layout_tabs1.addLayout(layout_file_browse)
        main_layout_tabs1.addSpacing(20)
        main_layout_tabs1.addLayout(layout_upload_button)

        self.tab1.setLayout(main_layout_tabs1)

        """Setting tab2 widget"""
        self.tab2 = QtGui.QWidget()
        main_layout_tabs2 = QtGui.QVBoxLayout()
        main_layout_tabs2.setContentsMargins(0,0,0,5)
        label_tab2 = QtGui.QLabel()
        palette2 = QtGui.QPalette()
        color2 = QtGui.QColor(0.6*255,0.5*255,0.5*255)
        color = QtGui.QColor(0,0,0)
        palette2.setColor(QtGui.QPalette.WindowText,color)
        palette2.setColor(QtGui.QPalette.Background,color2)
        label_tab2.setAutoFillBackground(True)
        label_tab2.setText("Codes Manager")
        label_tab2.setPalette(palette2)
        label_tab2.setFixedHeight(30)
        label_tab2.setAlignment(QtCore.Qt.AlignCenter)
        label_tab2.setFont(QtGui.QFont("Rome times",10,QtGui.QFont.Bold))
        """layout_selection2"""
        layout_selection2 = QtGui.QGridLayout()
        label1_selection2 = QtGui.QLabel("Project :")
        label1_selection2.setFont(QtGui.QFont("Helvetica",8,QtGui.QFont.DemiBold))
        self.comboBox1_selection2 = QtGui.QComboBox()
        self.comboBox1_selection2.currentIndexChanged.connect(self.refreshList)
        self.comboBox1_selection2.setFixedWidth(120)
        self.comboBox2_selection2 = QtGui.QComboBox()
        self.comboBox2_selection2.currentIndexChanged.connect(self.refreshList)
        self.comboBox2_selection2.setFixedWidth(120)
        label2_selection2 = QtGui.QLabel("Module :")
        label2_selection2.setFont(QtGui.QFont("Helvetica",8,QtGui.QFont.DemiBold))
        layout_selection2.addWidget(label1_selection2,1,0)
        layout_selection2.addWidget(self.comboBox1_selection2,1,1)
        layout_selection2.addWidget(label2_selection2,2,0)
        layout_selection2.addWidget(self.comboBox2_selection2,2,1)
        pixmap_logo = QtGui.QLabel()
        image = QtGui.QImage(self.image_path+"mili.png")
        image = image.scaled(60,60)
        pixmap_logo.setPixmap(QtGui.QPixmap.fromImage(image))
        layout_HBox_manage = QtGui.QHBoxLayout()
        layout_HBox_manage.setContentsMargins(10,0,10,0)
        layout_HBox_manage.addLayout(layout_selection2)
        layout_HBox_manage.addStretch(3)
        layout_HBox_manage.addWidget(pixmap_logo)
        layout_HBox_manage.addStretch(1)
        """layout_separation2"""
        layout_separation2 = QtGui.QHBoxLayout()
        layout_separation2.setContentsMargins(0,0,0,0)
        label_separation2 = QtGui.QLabel()
        label_separation2.setAutoFillBackground(True)
        label_separation2.setFixedHeight(5)
        label_separation2.setPalette(palette2)
        layout_separation2.addWidget(label_separation2)
        """layout_proj_list"""
        layout_proj_list = QtGui.QHBoxLayout()
        layout_proj_list.setContentsMargins(10,0,10,0)
        self.listWidget = QtGui.QListWidget()
        self.listWidget.setMaximumHeight(150)
        layout_button_list = QtGui.QVBoxLayout()
        layout_button_list.setContentsMargins(0,0,0,0)
        button1_list = QtGui.QPushButton()
        button1_list.setFixedSize(100,30)
        button1_list.setIcon(QtGui.QIcon(self.image_path+"compose.png"))
        button1_list.setText("Edit   ")
        button1_list.clicked.connect(self.edit)
        self.button2_list = QtGui.QPushButton()
        self.button2_list.setFixedSize(100,30)
        self.button2_list.setIcon(QtGui.QIcon(self.image_path+"bomb.png"))
        self.button2_list.clicked.connect(self.delete)
        self.button2_list.setText("Delete ")
        self.button3_list = QtGui.QPushButton()
        self.button3_list.setFixedSize(100,30)
        self.button3_list.setIcon(QtGui.QIcon(self.image_path+"colorwheel.png"))
        self.button3_list.setText("Back up")
        self.button3_list.clicked.connect(self.backup)
        self.button4_list = theButton()
        self.button4_list.signal.connect(lambda *args:self.active("caution"))
        self.button4_list.clicked.connect(lambda *args:self.active("check"))
        self.button4_list.setFixedSize(100,30)
        self.button4_list.setIcon(QtGui.QIcon(self.image_path+"brightness.png"))
        self.button4_list.setText("Active ")
        layout_button_list.addWidget(button1_list)
        layout_button_list.addWidget(self.button2_list)
        layout_button_list.addWidget(self.button3_list)
        layout_button_list.addWidget(self.button4_list)
        layout_proj_list.addWidget(self.listWidget)
        layout_proj_list.addLayout(layout_button_list)
        """layout_manage_name"""
        layout_manage_name = QtGui.QHBoxLayout()
        layout_manage_name.setContentsMargins(10,0,10,0)
        label_manage_name = QtGui.QLabel("Name :")
        self.lineEdit_manage_name = QtGui.QLineEdit()
        layout_manage_name.addWidget(label_manage_name)
        layout_manage_name.addWidget(self.lineEdit_manage_name)
        """layout_code_edit_manage"""
        layout_code_edit_manage = QtGui.QVBoxLayout()
        layout_code_edit_manage.setContentsMargins(10,0,10,0)
        self.textEdit_code_edit_manage = QtGui.QTextEdit()
        self.textEdit_code_edit_manage.setLineWrapMode(QtGui.QTextEdit.NoWrap)
        self.textEdit_code_edit_manage.setMinimumHeight(280)
        layout_code_edit_manage.addWidget(self.textEdit_code_edit_manage)
        """layout_tool_button_manage"""
        layout_tool_button_manage = QtGui.QHBoxLayout()
        layout_tool_button_manage.setContentsMargins(10,0,10,0)
        button1_tool_button_manage = QtGui.QPushButton()
        button1_tool_button_manage.setText("Run")
        button1_tool_button_manage.clicked.connect(lambda *args:self.testScript(2))
        button1_tool_button_manage.setIcon(QtGui.QIcon(self.image_path+"lightbulb.png"))
        button2_tool_button_manage = QtGui.QPushButton()
        button2_tool_button_manage.setText("Clear")
        button2_tool_button_manage.clicked.connect(self.clear)
        button2_tool_button_manage.setIcon(QtGui.QIcon(self.image_path+"magicwand.png"))
        layout_tool_button_manage.addWidget(button1_tool_button_manage)
        layout_tool_button_manage.addWidget(button2_tool_button_manage)
        layout_tool_button_manage.addStretch()
        """layout_update_button"""
        layout_update_button = QtGui.QVBoxLayout()
        layout_update_button.setContentsMargins(10,0,10,0)
        self.button_update_button = QtGui.QPushButton()
        self.button_update_button.setIcon(QtGui.QIcon(self.image_path+"spaceshuttle.png"))
        self.button_update_button.setIconSize(QtCore.QSize(40,40))
        self.button_update_button.setText("Update")
        self.button_update_button.clicked.connect(self.update)
        self.button_update_button.setFont(QtGui.QFont("Roman times",9,QtGui.QFont.Bold))
        layout_update_button.addWidget(self.button_update_button)
        """set main_layout_tabs2"""
        main_layout_tabs2.addWidget(label_tab2)
        main_layout_tabs2.addLayout(layout_HBox_manage)
        main_layout_tabs2.addLayout(layout_separation2)
        main_layout_tabs2.addLayout(layout_proj_list)
        main_layout_tabs2.addSpacing(10)
        main_layout_tabs2.addLayout(layout_manage_name)
        main_layout_tabs2.addLayout(layout_code_edit_manage)
        main_layout_tabs2.addLayout(layout_tool_button_manage)
        main_layout_tabs2.addSpacing(20)
        main_layout_tabs2.addLayout(layout_update_button)

        self.tab2.setLayout(main_layout_tabs2)
        """Setting main tab widget"""
        self.main_tabs.addTab(self.tab1,"Upload your codes")
        self.main_tabs.addTab(self.tab2,"Manage your codes")
        main_layout_tabs = QtGui.QHBoxLayout()
        main_layout_tabs.addWidget(self.main_tabs)
        self.setWindowTitle("Codes Upload For TD")
        self.setWindowIcon(QtGui.QIcon(self.image_path+'mililogo.bmp'))
        self.setLayout(main_layout_tabs)
        flags = QtCore.Qt.WindowFlags()
        #flags |= QtCore.Qt.Tool
        flags |= QtCore.Qt.WindowStaysOnTopHint
        self.setWindowFlags(flags)
        self.resize(450,700)
        self.show()

    def refreshList(self):
        self.listWidget.clear()
        ServerRoot = self.ServerRoot
        project = self.comboBox1_selection2.currentText()
        module = self.comboBox2_selection2.currentText()
        ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(ServerRoot,project, module)
        #get listDir
        try:
            listDir = os.listdir('%s/'%ServerRootPath)
        except WindowsError:
            listDir = []
        #open activeMap.txt and get information
        try:
            file = open(ServerRootPath+'activeMap.txt', 'rb')
            activeMap = json.load(file)
            file.close()
        except IOError:
            activeMap = {}

        fileList = []
        for item in listDir:
            testList = item.split('.')
            if testList[-1] == 'py' or testList[-1] == 'mel' or testList[-1] == 'cpp' or testList[-1] == 'xml':
                fileList.append(item)
        self.listWidget.addItems(fileList)
        self.listWidget.setSpacing(1)
        for i in range(self.listWidget.count()):
            listWidgetItem = self.listWidget.item(i)
            try:
                flag = activeMap[listWidgetItem.text()][0]
            except KeyError:
                flag = None
            if flag == True:
                listWidgetItem.setIcon(QtGui.QIcon(self.image_path+"check.png"))
            elif flag == False:
                listWidgetItem.setIcon(QtGui.QIcon(self.image_path+"caution.png"))
            else:
                listWidgetItem.setIcon(QtGui.QIcon(self.image_path+"denied.png"))
            listWidgetItem.setSizeHint(QtCore.QSize(20,20))

    def setInitValue(self):
        if self.appointment == '':
            self.appointment = u"组员"
        self.label2_information.setText(self.department)
        self.label4_information.setText(self.appointment)
        self.comboBox1_selection1.addItems(self.projectList)
        self.comboBox2_selection1.addItems(self.moduleList)
        index1 = self.projectList.index(self.project)
        self.comboBox1_selection1.setCurrentIndex(index1)
        index2 = self.moduleList.index(self.department)
        self.comboBox2_selection1.setCurrentIndex(index2)
        self.comboBox1_selection2.addItems(self.projectList)
        self.comboBox2_selection2.addItems(self.moduleList)
        self.comboBox1_selection2.setCurrentIndex(index1)
        self.comboBox2_selection2.setCurrentIndex(index2)
        self.refreshList()

    def setMayaStyle(self):
        if self.env == "maya.exe":
            palette = QtGui.QPalette()
            colorTab = QtGui.QColor(0.3*255,0.3*255,0.3*255)
            palette.setColor(QtGui.QPalette.Background,colorTab)
            self.tab1.setAutoFillBackground(True)
            self.tab1.setPalette(palette)
            self.tab2.setAutoFillBackground(True)
            self.tab2.setPalette(palette)
        else:
            pass

    def browse(self):
        format1 = 'py'
        format2 = 'mel'
        format3 = 'cpp'
        format4 = 'txt'
        format5 = 'xml'
        FileDialog = QtGui.QFileDialog.getOpenFileName(self, "Find File", 'D:/',
                                                            "%s Files (*.%s);;%s Files (*.%s);;%s Files (*.%s);;%s Files (*.%s);;%s Files (*.%s);;All Files (*)" %
                                                            (format1.upper(), format1,format2.upper(), format2,format3.upper(), format3,format4.upper(), format4,format5.upper(), format5))
        file_path = FileDialog[0]
        if file_path:
            file_name = file_path.split('/')[-1]
            self.lineEdit_namefile.setText(file_name)
            file = open(file_path,'rb')
            file_content = file.read()
            file.close()
            """content decode"""
            try:
                file_content = file_content.decode("utf-8")
            except LookupError:
                try:
                    file_content = file_content.decode("ascii")
                except LookupError:
                    file_content = file_content.decode("ANSI")
            except UnicodeDecodeError:
                self.erroRaise()
            self.textEdit_code_edit.setText(file_content)
            self.lineEdit_file_browse.setText(file_path)

    def testScript(self,flag = 0):
        if flag == 1:
            if self.env == "maya.exe":
                import maya.cmds as cmds
                name = self.lineEdit_namefile.text()
                scriptText = self.textEdit_code_edit.toPlainText()
                type = name.split(".")[-1]
                if type == 'mel':
                    type = "mel"
                else:
                    type = 'python'
                if scriptText:
                    print "Hello World"
                    cmds.window("maya_script_windows")
                    cmds.columnLayout(adjustableColumn=True )
                    cmds.cmdScrollFieldExecuter('scriptTextTest',text=scriptText, execute=True, sourceType=type)
                    cmds.showWindow()
                    cmds.deleteUI('maya_script_windows')
            else:
                print self.textEdit_code_edit.toPlainText()
        elif flag == 2:
            if self.env == "maya.exe":
                import maya.cmds as cmds
                name = self.lineEdit_manage_name.text()
                scriptText = self.textEdit_code_edit_manage.toPlainText()
                type = name.split(".")[-1]
                if type == 'mel':
                    type = "mel"
                else:
                    type = 'python'
                if scriptText:
                    cmds.window("maya_script_windows")
                    cmds.columnLayout(adjustableColumn=True )
                    cmds.cmdScrollFieldExecuter('scriptTextTest',text=scriptText, execute=True, sourceType=type)
                    cmds.showWindow()
                    cmds.deleteUI('maya_script_windows')
            else:
                print self.textEdit_code_edit_manage.toPlainText()
        else:
            pass

    def time(self):
        import datetime
        tmp1_current_time = str(datetime.datetime.now())
        tmp2_current_time = tmp1_current_time.replace('-','y$',1).replace('-','m-',2).replace(' ', 'd-')
        tmp3_current_time = tmp2_current_time.replace('$','-')
        tmp4_current_time = tmp3_current_time.split('.')[0]
        current_time = tmp4_current_time.replace(':','h-',1).replace(':','m-',2)+'s'
        return current_time

    def clear(self):
        self.lineEdit_manage_name.clear()
        self.textEdit_code_edit_manage.clear()
    def lsrunasPlus(self,local,server,name=None,delete=False):
        if self.env == "maya.exe":
            import Mili.Module.utilties.lsrunas as lsrunas
            reload(lsrunas)
            if not os.path.isdir(server):
                lsrunas.lsrunas().CreateDir_1(server)
            if delete == True:
                lsrunas.lsrunas().deleteFile(server+name)
            else:
                lsrunas.lsrunas(time.time()).UploadFile(local+name, server+name, DeleteSource=False)
        else:
            import shutil
            if not os.path.isdir(server):
                os.makedirs(server)
            if not os.path.isdir(server):
                os.makedirs(server)
            if delete == True:
                os.remove(server+name)
            else:
                shutil.copyfile(local+name,server+name)

    def upload(self):
        current_time = self.time()
        LocalRoot = self.LocalRoot
        ServerRoot = self.ServerRoot
        project = self.comboBox1_selection1.currentText()
        module = self.comboBox2_selection1.currentText()
        LocalRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(LocalRoot,project, module)
        ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(ServerRoot,project, module)
        ServerBackUpPath = '%s/%s/ProjectConfig/workNodeScripts/%s/backup/%s/'%(ServerRoot,project, module,current_time)
        name = self.lineEdit_namefile.text()
        content = self.textEdit_code_edit.toPlainText()
        if name == '' or content == '':
            return
        #create LocalRootPath when it does not exsit.
        if not os.path.isdir(LocalRootPath):
            os.makedirs(LocalRootPath)

        #create local file
        if os.path.isfile(ServerRootPath+name):
            dialog = theDialog(self)
            dialog.showWindows(title="Warming",content="The file is exsited!",plug="Confirm")
            return
        else:
            file = open(LocalRootPath+name,'wb')
            file.writelines(content.encode("utf-8"))
            file.close()

        #write activeMap`s information
        try:
            file = open(ServerRootPath+'activeMap.txt', 'rb')
            activeMap = json.load(file)
            file.close()
        except IOError:
            activeMap = {}
        activeMap[name] = [True,False]
        file = open(LocalRootPath+'activeMap.txt', 'wb')
        json.dump(activeMap, file)
        file.close()

        #do upload
        self.lsrunasPlus(LocalRootPath,ServerRootPath,name=name)
        self.lsrunasPlus(LocalRootPath,ServerBackUpPath,name=name)
        self.lsrunasPlus(LocalRootPath,ServerRootPath,name='activeMap.txt')

        self.lineEdit_namefile.clear()
        self.textEdit_code_edit.clear()
        self.lineEdit_file_browse.clear()
        dialog = theDialog(self)
        dialog.showWindows(title="Completing",content="The file is uploaded",plug="Confirm")

    def edit(self):
        listWidgetItem = self.listWidget.currentItem()
        if not listWidgetItem:
            return
        name = listWidgetItem.text()
        ServerRoot = self.ServerRoot
        project = self.comboBox1_selection2.currentText()
        module = self.comboBox2_selection2.currentText()
        ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(ServerRoot,project, module)

        file = open(ServerRootPath+name,'rb')
        content = file.read()
        file.close()
        self.lineEdit_manage_name.setText(name)
        self.textEdit_code_edit_manage.setText(content.decode("utf-8"))

    def delete(self):
        listWidgetItem = self.listWidget.currentItem()
        if not listWidgetItem:
            return
        dialog = theDialog(self)
        dialog.showWindows(title="Warming",content="You sure to delete the file?",plug="Choice")
        try:
            feedback = dialog.value
        except AttributeError:
            feedback = False
        if feedback:
            name = listWidgetItem.text()
            ServerRoot = self.ServerRoot
            project = self.comboBox1_selection2.currentText()
            module = self.comboBox2_selection2.currentText()
            ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(ServerRoot,project, module)

            self.lsrunasPlus(None,ServerRootPath,name=name,delete=True)
            self.refreshList()
        self.clear()

    def backup(self):
        ServerRoot = self.ServerRoot
        project = self.comboBox1_selection2.currentText()
        module = self.comboBox2_selection2.currentText()
        ServerBackUpPath = '%s/%s/ProjectConfig/workNodeScripts/%s/backup/'%(ServerRoot,project, module)

        format1 = 'py'
        format2 = 'mel'
        format3 = 'cpp'
        format4 = 'txt'
        format5 = 'xml'
        FileDialog = QtGui.QFileDialog.getOpenFileName(self, "Find File", ServerBackUpPath,
                                                            "%s Files (*.%s);;%s Files (*.%s);;%s Files (*.%s);;%s Files (*.%s);;%s Files (*.%s);;All Files (*)" %
                                                            (format1.upper(), format1,format2.upper(), format2,format3.upper(), format3,format4.upper(), format4,format5.upper(), format5))
        file_path = FileDialog[0]
        if file_path:
            file_name = file_path.split('/')[-1]
            self.lineEdit_manage_name.setText(file_name)
            file = open(file_path,'rb')
            file_content = file.read()
            file.close()
            """content decode"""
            try:
                file_content = file_content.decode("utf-8")
            except LookupError:
                try:
                    file_content = file_content.decode("ascii")
                except LookupError:
                    file_content = file_content.decode("ANSI")
            except UnicodeDecodeError:
                self.erroRaise()
            self.textEdit_code_edit_manage.setText(file_content)

    def active(self,plug=None):
        listWidgetItem = self.listWidget.currentItem()
        if not listWidgetItem:
            return
        LocalRoot = self.LocalRoot
        ServerRoot = self.ServerRoot
        project = self.comboBox1_selection2.currentText()
        module = self.comboBox2_selection2.currentText()
        LocalRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(LocalRoot,project, module)
        ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(ServerRoot,project, module)
        try:
            file = open(ServerRootPath+'activeMap.txt', 'rb')
            activeMap = json.load(file)
            file.close()
        except IOError:
            activeMap = {}
        if plug == "check":
            listWidgetItem.setIcon(QtGui.QIcon(self.image_path+"check.png"))
            activeMap[listWidgetItem.text()] = [True,False]
        elif plug == "caution":
            listWidgetItem.setIcon(QtGui.QIcon(self.image_path+"caution.png"))
            activeMap[listWidgetItem.text()] = [False,False]
        else:
            return
        file = open(LocalRootPath+'activeMap.txt', 'wb')
        json.dump(activeMap, file)
        file.close()
        self.lsrunasPlus(LocalRootPath,ServerRootPath,'activeMap.txt')

    def update(self):
        current_time = self.time()
        LocalRoot = self.LocalRoot
        ServerRoot = self.ServerRoot
        project = self.comboBox1_selection2.currentText()
        module = self.comboBox2_selection2.currentText()
        LocalRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(LocalRoot,project, module)
        ServerRootPath = '%s/%s/ProjectConfig/workNodeScripts/%s/'%(ServerRoot,project, module)
        ServerBackUpPath = '%s/%s/ProjectConfig/workNodeScripts/%s/backup/%s/'%(ServerRoot,project, module,current_time)
        name = self.lineEdit_manage_name.text()
        content = self.textEdit_code_edit_manage.toPlainText()
        if name == '' or content == '':
            return
        dialog = theDialog(self)
        dialog.showWindows(title="Notice",content="You sure to update this file?",plug="Choice")
        try:
            feedback = dialog.value
        except AttributeError:
            feedback = False
        if feedback == False:
            return

        #create LocalRootPath when it does not exsit.
        if not os.path.isdir(LocalRootPath):
            os.makedirs(LocalRootPath)

        #create local file
        file = open(LocalRootPath+name,'wb')
        file.writelines(content.encode("utf-8"))
        file.close()

        #write activeMap`s information
        try:
            file = open(ServerRootPath+'activeMap.txt', 'rb')
            activeMap = json.load(file)
            file.close()
        except IOError:
            activeMap = {}

        try:
            activeMap[name]
        except KeyError:
            activeMap[name] = [True,False]
        file = open(LocalRootPath+'activeMap.txt', 'wb')
        json.dump(activeMap, file)
        file.close()

        #do update
        self.lsrunasPlus(LocalRootPath,ServerRootPath,name=name)
        self.lsrunasPlus(LocalRootPath,ServerBackUpPath,name=name)
        self.lsrunasPlus(LocalRootPath,ServerRootPath,name='activeMap.txt')
        self.clear()
        self.refreshList()

    def erroRaise(self,info="No information",kill=False):
        """
        这个函数用于收集错误信息
        kill参数为True时会在报错时终止程序
        """
        s=sys.exc_info()
        erro = "===Erro Report:===\n" \
               "Error : '%s' \n" \
               "Happened on line %d\n" \
               "%s\n" \
               "==================\n\n" % (s[1],s[2].tb_lineno,info)
        print erro
        #~ 是否终止程序
        if kill == True:
            sys.exit()

    def setUserRights(self):
        if self.department == "PipelineTD" or self.department == "RnD" or self.appointment == "TD":
            pass
        else:
            self.button_upload_button.setEnabled(False)
            self.button2_list.setEnabled(False)
            self.button3_list.setEnabled(False)
            self.button4_list.setEnabled(False)
            self.button_update_button.setEnabled(False)

def main():
    global mainWin
    selfApp = os.path.basename(os.path.basename(sys.argv[0]))
    if selfApp == "maya.exe":
        mainWin = TDsCodeUpload(selfApp)
        mainWin.setMayaStyle()
    elif selfApp.split('.')[-1] == 'py':
        app = QtGui.QApplication(sys.argv)
        mainWin = TDsCodeUpload(selfApp)
        sys.exit(app.exec_())
    else:
        mainWin = TDsCodeUpload(selfApp)


if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    mainWin = TDsCodeUpload(None)
    sys.exit(app.exec_())
