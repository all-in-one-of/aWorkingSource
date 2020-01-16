__author__ = 'Kevin Tsui'
import os,sys,datetime,shutil
import maya.cmds as cmds
from PySide import QtGui,QtCore

class theDialog(QtGui.QDialog):
    def __init__(self,parent = None):
        super(theDialog,self).__init__()
        self.setParent(parent)
    def showWindows(self,title="Tips",content="Hello World!",plug=None):
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

class lightBake(QtGui.QDialog):
    def __init__(self):
        super(lightBake,self).__init__()

        dMayaLightTypeList = ['directionalLight','ambientLight','pointLight','spotLight','areaLight','volumeLight']
        dArnoldLightTypeList = ['aiAreaLight','aiSkyDomeLight','aiPhotometricLight','aiLightPortal',"aiMeshLight","aiSky"]
        dLightType = dMayaLightTypeList + dArnoldLightTypeList
        dLightList = []
        for light in dLightType:
            #print light
            tempLight = cmds.ls(type=light)
            #print tempLight
            if tempLight:
                dLightList += tempLight
        self.dLightList = dLightList

        self.showWindows()
    def showWindows(self):
        mainLayoutTabs = QtGui.QHBoxLayout()
        mainTabs = QtGui.QTabWidget()
        self.margins = QtCore.QMargins(5,0,5,0)

        tab1 = self.constructTab1()
        tab2 = self.constructTab2()
        mainTabs.addTab(tab1,"BAKE")
        mainTabs.addTab(tab2,"CREATE")
        mainLayoutTabs.addWidget(mainTabs)
        self.setLayout(mainLayoutTabs)

        flags = QtCore.Qt.WindowFlags()
        flags |= QtCore.Qt.WindowStaysOnTopHint
        self.setWindowTitle("Hoongman TD Test")
        self.setWindowFlags(flags)
        self.resize(450,500)
        self.show()
    def constructTab1(self):
        tab1 = QtGui.QWidget()
        mainLayoutTab1 = QtGui.QVBoxLayout()
        mainLayoutTab1.setContentsMargins(0,0,0,5)

        color1 = QtGui.QColor(0.5*255,0.5*255,0.6*255)
        labelTab1 = self.createTitie("Bake Lights",color1)

        labelLightList = self.createHeadLabel("Light List :")

        listWidgetLayout = QtGui.QVBoxLayout()
        listWidget = QtGui.QListView()
        listWidget.setMinimumHeight(250)
        listWidget.setSpacing(3)
        listWidgetLayout.setContentsMargins(self.margins)
        listWidget.setContentsMargins(self.margins)
        self.listModel = self.createItemWidget(listWidget)
        listWidget.setModel(self.listModel)

        layoutAddMeshButton = QtGui.QHBoxLayout()
        addMeshButton = QtGui.QPushButton()
        addMeshButton.setText("add mesh")
        addMeshButton.clicked.connect(self.addMesh)
        layoutAddMeshButton.addStretch()
        layoutAddMeshButton.addWidget(addMeshButton)

        listWidgetLayout.addWidget(listWidget)
        listWidgetLayout.addLayout(layoutAddMeshButton)


        """get the path where the file saved"""
        layoutBrowse = QtGui.QHBoxLayout()
        labelBrowse = self.createHeadLabel("Save File Path :")
        layoutBrowse.setContentsMargins(self.margins)
        self.lineEditBakeTo = QtGui.QLineEdit()
        self.lineEditBakeTo.setReadOnly(False)
        self.lineEditBakeTo.setEnabled(True)
        buttoBrowse = QtGui.QPushButton()
        buttoBrowse.clicked.connect(lambda arg=None,lineEdit=self.lineEditBakeTo,flag=0:self.browse(lineEdit,flag))
        buttoBrowse.setText("Browse")
        layoutBrowse.addWidget(self.lineEditBakeTo)
        layoutBrowse.addWidget(buttoBrowse)

        """"create Button"""
        labelButton = self.createHeadLabel("Bake Lights :")
        button = self.createButton("Bake",40,self.bake)

        mainLayoutTab1.addStretch()
        mainLayoutTab1.addWidget(labelTab1)
        mainLayoutTab1.addLayout(labelLightList)
        mainLayoutTab1.addLayout(listWidgetLayout)
        mainLayoutTab1.addSpacing(10)
        mainLayoutTab1.addLayout(labelBrowse)
        mainLayoutTab1.addLayout(layoutBrowse)
        mainLayoutTab1.addSpacing(10)
        mainLayoutTab1.addLayout(labelButton)
        mainLayoutTab1.addLayout(button)
        mainLayoutTab1.addStretch()


        tab1.setLayout(mainLayoutTab1)
        return tab1
    def constructTab2(self):
        tab2 = QtGui.QWidget()
        mainLayoutTab2 = QtGui.QVBoxLayout()
        mainLayoutTab2.setContentsMargins(0,0,0,0)

        layoutIntroduction = QtGui.QVBoxLayout()
        labelIntroduction = QtGui.QLabel()
        labelIntroduction.setText("Step 1:\n"
                                  "Selecting the lights you want to bake out\n"
                                  "\n"
                                  "Step 2:\n"
                                  "Pointing out the path your light file locate,BAKE!\n"
                                  "\n"
                                  "Step 3:\n"
                                  "Clicking the 'Light' button to pick the light file\n"
                                  "\n"
                                  "Step 4:\n"
                                  "Clicking the 'Scene' button to pick the scene file which one is no light\n"
                                  "\n"
                                  "Step 5:\n"
                                  "Clicking the 'Create' button to create the new file within light data")
        labelIntroduction.setMinimumHeight(200)
        layoutIntroduction.addWidget(labelIntroduction)
        layoutIntroduction.setContentsMargins(self.margins)

        groupBoxIntroduction = QtGui.QGroupBox("Introduction")
        groupBoxIntroduction.setLayout(layoutIntroduction)

        color2 = QtGui.QColor(0.6*255,0.5*255,0.5*255)
        labelTab2 = self.createTitie("Create New File",color2)

        labelLightIn = self.createHeadLabel("Light File Location :")
        layoutLightIn = QtGui.QHBoxLayout()
        self.lineEditLightIn = QtGui.QLineEdit()
        buttonLightIn = QtGui.QPushButton()
        buttonLightIn.setText("Light")
        buttonLightIn.clicked.connect(lambda arg=None,lineEdit=self.lineEditLightIn,flag=1:self.browse(lineEdit,flag))
        layoutLightIn.setContentsMargins(self.margins)
        layoutLightIn.addWidget(self.lineEditLightIn)
        layoutLightIn.addWidget(buttonLightIn)

        labelSceneIn = self.createHeadLabel("Scene without Light Location :")
        layoutSceneIn = QtGui.QHBoxLayout()
        self.lineEditSceneIn = QtGui.QLineEdit()
        buttonSceneIn = QtGui.QPushButton()
        buttonSceneIn.setText("Scene")
        buttonSceneIn.clicked.connect(lambda arg=None,lineEdit=self.lineEditSceneIn,flag=1:self.browse(lineEdit,flag))
        layoutSceneIn.setContentsMargins(self.margins)
        layoutSceneIn.addWidget(self.lineEditSceneIn)
        layoutSceneIn.addWidget(buttonSceneIn)

        button = self.createButton("Create",60,self.create)

        mainLayoutTab2.addWidget(labelTab2)
        mainLayoutTab2.addWidget(groupBoxIntroduction)
        mainLayoutTab2.addSpacing(20)
        mainLayoutTab2.addLayout(labelLightIn)
        mainLayoutTab2.addLayout(layoutLightIn)
        mainLayoutTab2.addSpacing(20)
        mainLayoutTab2.addLayout(labelSceneIn)
        mainLayoutTab2.addLayout(layoutSceneIn)
        mainLayoutTab2.addStretch(1)
        mainLayoutTab2.addLayout(button)
        mainLayoutTab2.addStretch(0)
        tab2.setLayout(mainLayoutTab2)
        return tab2
    def createTitie(self,InText = None,InColor = None):
        tabLabel = QtGui.QLabel()
        palette = QtGui.QPalette()
        palette.setColor(QtGui.QPalette.Background,InColor)
        color = QtGui.QColor(0,0,0)
        palette.setColor(QtGui.QPalette.WindowText,color)
        tabLabel.setAutoFillBackground(True)
        tabLabel.setText(InText)
        tabLabel.setPalette(palette)
        tabLabel.setFixedHeight(40)
        tabLabel.setAlignment(QtCore.Qt.AlignCenter)
        tabLabel.setFont(QtGui.QFont("Rome times",10,QtGui.QFont.Bold))
        return tabLabel
    def createItemWidget(self,InListWidget):
        listModel = QtGui.QStandardItemModel(InListWidget)

        lights = self.dLightList
        for light in lights:
            # create an item with a caption
            item = QtGui.QStandardItem(light)

            # add a checkbox to it
            item.setCheckable(True)
            item.setCheckState(QtCore.Qt.Checked)
            listModel.appendRow(item)
        return listModel
    def createHeadLabel(self,InText):
        label = QtGui.QLabel()
        label.setFont(QtGui.QFont("Rome times",8,QtGui.QFont.Bold))
        label.setText(InText)
        layout = QtGui.QHBoxLayout()
        layout.setContentsMargins(self.margins)
        layout.addWidget(label)
        layout.addStretch()
        return layout
    def createButton(self,InText,InHight,InFuction):
        layout = QtGui.QHBoxLayout()
        layout.setContentsMargins(self.margins)
        button = QtGui.QPushButton()
        button.setText(InText)
        button.clicked.connect(InFuction)
        button.setFixedHeight(InHight)
        layout.addWidget(button)
        return layout
    def browse(self,InLineEdit=None,flag=None):
        if flag == 0:
            fileDialog = QtGui.QFileDialog.getSaveFileName(self, "Save Light", '',
                "%s Files (*.%s);;All Files (*)" %("Maya Binary","mb"))
            filePath = fileDialog[0]
            InLineEdit.setText(filePath)

        format1 = 'ma'
        format2 = 'mb'

        if flag == 1:
            fileDialog = QtGui.QFileDialog.getOpenFileName(self, "Read File", '',
                "%s Files (*.%s);;All Files (*)" %("Maya Binary","mb"))
            filePath = fileDialog[0]
            InLineEdit.setText(filePath)
    def bake(self):
        self.backup()
        listModel = self.listModel
        aLightsExport = []
        #uncheckedState = QtCore.Qt.CheckState.Unchecked
        checkState = QtCore.Qt.CheckState.Checked

        for i in range(listModel.rowCount()):
            if listModel.item(i).checkState() == checkState:
                item = listModel.item(i).text()
                aLightsExport.append(item)
                #print listModel.item(i).text(),"Yes"

        cmds.select(aLightsExport)
        filePath = self.lineEditBakeTo.text()
        if filePath:
            cmds.file(filePath,force=True, exportSelected=True, type="mayaBinary")
            self.lineEditLightIn.setText(filePath)
            dialog = theDialog(self)
            dialog.showWindows(content="The light information have baked out!",plug="Confirm")
            print "# All Lights have bake out!!"
        else:
            print "# Please click the 'Browse' button to pass in a path!"
        cmds.select(None)
    def addMesh(self):
        selections = cmds.ls(selection=True)
        for light in selections:
            item = QtGui.QStandardItem(light)
            item.setCheckable(True)
            item.setCheckState(QtCore.Qt.Checked)
            self.listModel.appendRow(item)
    def create(self):
        self.backup()
        lightFilePath = self.lineEditLightIn.text()
        sceneFilePath = self.lineEditSceneIn.text()
        if lightFilePath and sceneFilePath:
            cmds.file(sceneFilePath,open=True,force=True)
            cmds.file(lightFilePath,i=True,force=True,namespace="LIGHT")
            dialog = theDialog(self)
            dialog.showWindows(content="The new file have created!",plug="Confirm")
            print "# Finished!"
        else:
            print "# Please click the 'Light' and 'Scene' button to pass in two paths!"
    def backup(self):
        filePath = cmds.file(sn=True,q=True)
        fileName = filePath.split("/")[-1]
        lenCut = len(fileName)
        currentTime = self.time()
        localPath = filePath[:-lenCut] + "_backup/" + currentTime + "/"
        if not os.path.isdir(localPath):
            os.makedirs(localPath)
        localFullPath = localPath + fileName
        cmds.file(save=True,force=True)
        shutil.copy2(filePath,localFullPath)
    def time(self):
        tmp1_current_time = str(datetime.datetime.now())
        tmp2_current_time = tmp1_current_time.replace('-','$',1).replace('-','-',2).replace(' ', '-')
        tmp3_current_time = tmp2_current_time.replace('$','-')
        tmp4_current_time = tmp3_current_time.split('.')[0]
        current_time = tmp4_current_time.replace(':','-',1).replace(':','-',2)+''
        return current_time

def main():
    global mainWin
    selfApp = os.path.basename(os.path.basename(sys.argv[0]))
    if selfApp == "maya.exe":
        mainWin = lightBake()
    elif selfApp.split('.')[-1] == 'py':
        app = QtGui.QApplication(sys.argv)
        mainWin = lightBake()
        sys.exit(app.exec_())
    else:
        mainWin = lightBake()

if __name__ == '__main__':
    main()


