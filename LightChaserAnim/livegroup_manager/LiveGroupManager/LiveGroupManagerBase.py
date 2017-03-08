# Copyright (c) 2016 Light Chaser Animation. All Rights Reserved.
#coding = utf-8
__author__ = 'Kevin Tsui'
import os,sys,json
from PyQt4 import QtGui,QtCore
try:
    from Katana import NodegraphAPI
except:
    pass

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



class LiveGroupManager(QtGui.QDialog):
    def __init__(self,parent=None):
        super(LiveGroupManager,self).__init__(parent)
        ###########################################################################
        # the dLiveGroupList hold all livegroup name in current Katana file
        # the dLiveGroupSourceMap hold all livegroup source in current Katana file
        # the dRememberOpData will remember the version user saved
        # the dLiveGroupData hold all information that Lighting leader update!
        self.primission = self.getPrimission() # Artist-Lead
        self.configPath = ""
        self.dLiveGroupList = []
        self.dLiveGroupSourceMap = {}
        self.dRememberOpData = {}
        self.dLiveGroupData = {}
        self.aRememberOpData = {}
        self.initialize()
    def initialize(self):
        for item in NodegraphAPI.GetAllNodesByType("LiveGroup", ):
            itemSource = item.getParameterValue("source", NodegraphAPI.GetCurrentTime())
            if itemSource:
                if itemSource[0] != "$":
                    itemName = item.getName()
                    self.dLiveGroupList.append(str(itemName))
                    self.dLiveGroupSourceMap[str(itemName)] = str(itemSource)
            else:
                itemName = item.getName()
                self.dLiveGroupList.append(str(itemName))
                self.dLiveGroupSourceMap[str(itemName)] = str(itemSource)

        ###############################################################
        # tmpLiveGroupSource keeps all source value from LiveGroup Node
        tmpLiveGroupSource = self.dLiveGroupSourceMap.values()
        aLiveGroupSource = []
        for item in tmpLiveGroupSource:
            if item:
                Len = len(item.split("/")[-1])
                aLiveGroupSource.append(item[:-Len])
        if aLiveGroupSource:
            self.configPath = self.getMostCount(aLiveGroupSource)[0] + "config.json"

        #print "@@@@@@@@@@@@@@@@@@@@@"
        #print self.configPath
        #print os.path.abspath(self.configPath)
        if os.path.isfile(self.configPath):
            file = open(self.configPath, "rb")
            self.dLiveGroupData = json.load(file)
            file.close()
        else:
            self.configPath = ""
    def showWindows(self):
        mainLayoutTabs = QtGui.QHBoxLayout()
        self.margins = (5, 5, 5, 5)
        mainWidget = self.constructWidget()

        mainLayoutTabs.setContentsMargins(0,0,0,0)
        mainLayoutTabs.addWidget(mainWidget)

        flags = QtCore.Qt.WindowFlags()
        flags |= QtCore.Qt.WindowStaysOnTopHint

        self.setLayout(mainLayoutTabs)
        self.setWindowFlags(flags)
        self.setWindowTitle("Light Chaser Animation Tools")
        self.resize(300,300)
        #self.center()
        self.setEnabledWidget()
        self.show()
    def constructWidget(self):
        tab = QtGui.QWidget()
        mainLayout = QtGui.QVBoxLayout()
        mainLayout.setContentsMargins(0,0,0,5)

        # add "LiveGroup Manager" title
        colorB = QtGui.QColor(0.5*255,0.5*255,0.6*255)
        tabLabel = QtGui.QLabel()
        palette = QtGui.QPalette()
        palette.setColor(QtGui.QPalette.Background,colorB)
        colorF = QtGui.QColor(0,0,0)
        palette.setColor(QtGui.QPalette.WindowText,colorF)
        tabLabel.setAutoFillBackground(True)
        tabLabel.setText("LiveGroup Manager")
        tabLabel.setPalette(palette)
        tabLabel.setFixedHeight(40)
        tabLabel.setAlignment(QtCore.Qt.AlignCenter)
        tabLabel.setFont(QtGui.QFont("Rome times",10,QtGui.QFont.Bold))

        listWidgetLayout = QtGui.QHBoxLayout()
        self.listWidget = QtGui.QListView()
        self.listWidget.setMinimumSize(150,250)
        self.listWidget.setSpacing(1)

        listWidgetLayout.setContentsMargins(self.margins[0],self.margins[1],self.margins[2],self.margins[3])
        self.listWidget.setContentsMargins(self.margins[0],self.margins[1],self.margins[2],self.margins[3])
        self.listModel = self.createItemWidget(self.listWidget)
        self.listWidget.clicked.connect(self.listChange)
        self.listWidget.setModel(self.listModel)
        infoGroupBox = QtGui.QGroupBox("information")
        infoLayout = QtGui.QVBoxLayout()
        infoLayout.setContentsMargins(self.margins[0],self.margins[1],self.margins[2],self.margins[3])
        versionLayout = QtGui.QHBoxLayout()
        versionLabel = QtGui.QLabel("Version :")
        self.version = QtGui.QComboBox()
        self.version.currentIndexChanged.connect(self.versionChange)
        versionLayout.addWidget(versionLabel)
        versionLayout.addWidget(self.version)
        publishPathLayout = QtGui.QHBoxLayout()
        publishPathLabel = QtGui.QLabel("Source :")
        self.publishPath = QtGui.QLineEdit()
        self.publishPath.setMinimumWidth(250)
        self.browseButton = QtGui.QPushButton()
        self.browseButton.clicked.connect(self.browse)
        self.browseButton.setIcon(QtGui.QIcon(os.path.join(os.path.dirname(__file__))+"/folder.png"))
        publishPathLayout.addWidget(publishPathLabel)
        publishPathLayout.addWidget(self.publishPath)
        publishPathLayout.addWidget(self.browseButton)
        self.info = QtGui.QTextEdit()
        self.info.setReadOnly(True)
        infoButtonLayout = QtGui.QHBoxLayout()
        self.editButton = self.createButton("Edit",InFuction=self.edit)
        self.saveButton = self.createButton("Save",InFuction=self.save)
        infoButtonLayout.addWidget(self.editButton)
        infoButtonLayout.addWidget(self.saveButton)
        infoLayout.addLayout(versionLayout)
        infoLayout.addLayout(publishPathLayout)
        infoLayout.addWidget(self.info)
        infoLayout.addLayout(infoButtonLayout)
        infoGroupBox.setLayout(infoLayout)
        listWidgetLayout.addWidget(self.listWidget)
        listWidgetLayout.addWidget(infoGroupBox)

        livegroupLayout = QtGui.QVBoxLayout()
        livegroupLayout.addLayout(listWidgetLayout)

        opButtonLayout = QtGui.QVBoxLayout()
        opButtonLayout.setContentsMargins(self.margins[0],self.margins[1],self.margins[2],self.margins[3])
        self.publishButton = self.createButton("Publish All",40,self.publish)
        self.reloadButton = self.createButton("Reload All",40,self.reload)
        opButtonLayout.addWidget(self.publishButton)
        opButtonLayout.addWidget(self.reloadButton)


        mainLayout.addStretch()
        mainLayout.addWidget(tabLabel)
        mainLayout.addLayout(livegroupLayout)
        mainLayout.addLayout(opButtonLayout)


        mainLayout.addStretch()


        tab.setLayout(mainLayout)
        self.setLocked(True)
        return tab
    def center(self):
        screen = QtGui.QDesktopWidget().screenGeometry()
        size = self.geometry()
        self.move((screen.width() - size.width()) / 2, (screen.height() - size.height()) / 2)
    def createItemWidget(self,InListWidget):
        listModel = QtGui.QStandardItemModel(InListWidget)

        allLiveGroups = self.dLiveGroupList
        for liveGroup in allLiveGroups:
            # create an item with a caption
            item = QtGui.QStandardItem(liveGroup)
            #font = QtGui.QFont()
            #item.setFont(QtGui.QFont(QtGui.QFont.defaultFamily,12,QtGui.QFont.DemiBold))
            # add a checkbox to it
            item.setCheckable(True)
            if self.primission == "Lead":
                item.setCheckState(QtCore.Qt.Unchecked)
            else:
                item.setCheckState(QtCore.Qt.Checked)
            item.setEditable(False)
            listModel.appendRow(item)
        return listModel
    def createHeadLabel(self,InText):
        label = QtGui.QLabel()
        label.setFont(QtGui.QFont("Rome times",8,QtGui.QFont.Bold))
        label.setText(InText)
        layout = QtGui.QHBoxLayout()
        layout.setContentsMargins(self.margins[0],self.margins[1],self.margins[2],self.margins[3])
        layout.addWidget(label)
        layout.addStretch()
        return layout
    def createButton(self,InText,InHight=20,InFuction=None):
        #layout.setContentsMargins(self.margins[0],self.margins[1],self.margins[2],self.margins[3])
        button = QtGui.QPushButton()
        button.setText(InText)
        if InFuction !=None:
            button.clicked.connect(InFuction)
        button.setFixedHeight(InHight)
        return button
    def getCurrentItem(self):
        index = self.listWidget.selectedIndexes()
        if index:
            currentItem = index[0].data().toString()
        else:
            currentItem = None
        return currentItem
    def listChange(self):
        currentListItem = str(self.getCurrentItem())
        if currentListItem:
            if currentListItem in self.dLiveGroupData or currentListItem in self.aRememberOpData:
                if currentListItem in self.aRememberOpData and currentListItem not in self.dLiveGroupData:
                    self.version.clear()
                    self.version.addItem("v001")
                    currentData = self.aRememberOpData[currentListItem]
                    self.publishPath.setText(currentData[1])
                    self.info.setText(currentData[2])
                else:
                    currentData = self.dLiveGroupData[currentListItem]
                    versionList = currentData.keys()
                    versionList.sort(reverse=True)
                    self.version.clear()
                    self.version.addItems(versionList)
                    currentVersionItem = versionList[0]
                    internData = self.dLiveGroupData[currentListItem][currentVersionItem]

                    self.publishPath.setText(internData[0])
                    self.info.setText(internData[1])
                    if currentListItem in self.aRememberOpData:
                        currentData = self.aRememberOpData[currentListItem]
                        currentVersionItem = currentData[0]
                        currentIndex = 0
                        for i in range(self.version.count()):
                            if currentVersionItem == self.version.itemText(i):
                                currentIndex = i
                        self.version.setCurrentIndex(currentIndex)
                        self.publishPath.setText(currentData[1])
                        self.info.setText(currentData[2])

                self.setLocked(True)
            elif currentListItem in self.dLiveGroupSourceMap:
                #############################################################################
                # those code is work with the situation that the LiveGroup info can`t be find
                # in self.dLiveGroupData,so we just show the publishPath,
                # and set other thing clean!
                self.publishPath.setText(self.dLiveGroupSourceMap[currentListItem])
                self.version.clear()
                self.info.setText("")
            else:
                #############################################################
                # currentListItem is not in the self.dLiveGroupSourceMap,
                # Bur,this situation won`t happen in Katana,only when we test
                # in standalone IDE!
                self.clean()
    def clean(self):
        self.version.clear()
        self.publishPath.setText('')
        self.info.setText('')
    def versionChange(self):
        currentListItem = str(self.getCurrentItem())
        currentVersionItem = str(self.version.currentText())
        if currentListItem and currentVersionItem and currentListItem in self.dLiveGroupData and currentVersionItem in self.dLiveGroupData[currentListItem]:
            #################################################################################
            # currentListItem exist and also currentVersionItem exist,this means that version
            # Combobox is work fine
            internData = self.dLiveGroupData[currentListItem][currentVersionItem]
            self.publishPath.setText(internData[0])
            self.info.setText(internData[1])
        elif currentListItem and currentListItem in self.dLiveGroupData:
            ###################################################################
            # currentListItem is existed but currentVersionItem could be the ''
            currentData = self.dLiveGroupData[currentListItem]
            versionList = currentData.keys()
            versionList.sort(reverse=True)
            currentVersionItem = versionList[0]
            internData = self.dLiveGroupData[currentListItem][currentVersionItem]

            self.publishPath.setText(internData[0])
            self.info.setText(internData[1])
        #self.dRememberOpData[currentListItem] = currentVersionItem
        self.setLocked(True)
    def setLocked(self,InState):
        if InState == True:
            stateTuple = (True,True,False,True)
        else:
            stateTuple = (True,False,True,False)
        self.version.setUpdatesEnabled(stateTuple[0])
        self.publishPath.setReadOnly(stateTuple[1])
        self.browseButton.setEnabled(stateTuple[2])
        self.info.setReadOnly(stateTuple[3])
    def setLockVersion(self,InState):
        if InState == True:
            stateTuple = (False,False,False,False)
        else:
            stateTuple = (True,False,True,True)

        self.version.enabledChange(stateTuple[0])
        self.version.setEditable(stateTuple[0])
        self.version.setUpdatesEnabled(stateTuple[0])
        self.version.setEnabled(stateTuple[0])
    def browse(self):
        defalutPath = "/mnt/proj/resource/lgt/"
        publishPath = self.publishPath.text()
        if publishPath:
            Len = len(publishPath.split("/")[-1])
            path = publishPath[:-Len]
        else:
            path = defalutPath
        fileDialog = QtGui.QFileDialog.getSaveFileName(self, "Save LiveGroup",path,"%s Files (*.%s);;All Files (*)" %("LiveGroup","livegroup"))

        if fileDialog:
            filePath = fileDialog
            self.publishPath.setText(filePath)
    def edit(self):
        currentListView = self.getCurrentItem()
        #print "@@@@@@@@@@@@@@@@@@@@"
        #print currentListView
        if currentListView == None:
            return
        self.setLocked(False)
    def save(self):
        def tr(t):
            #return unicode(t).encode('utf-8')
            return unicode(t)

        currentListView = self.getCurrentItem()
        if currentListView == None:
            return

        self.setLocked(True)
        ##################################################################
        # Be Careful,we should convert all type to string when we use JSON
        currentListItem = str(self.getCurrentItem())
        currentVersionItem = str(self.version.currentText())
        currentData = [str(self.publishPath.text()),tr(self.info.toPlainText())]
        if currentListItem and currentVersionItem and currentListItem in self.dLiveGroupData and currentVersionItem in self.dLiveGroupData[currentListItem]:
            self.aRememberOpData[currentListItem] = [currentVersionItem, str(self.publishPath.text()), tr(self.info.toPlainText())]
        else:
            self.aRememberOpData[currentListItem] = ["v001", str(self.publishPath.text()), tr(self.info.toPlainText())]
            self.version.addItem("v001")
        #self.dRememberOpData[currentListItem] = str(currentVersionItem)
    def getNewVersionKey(self, InKey):
        count = int(InKey[1:])
        count += 1
        result = "v" + "%03d"%count
        return result
    def getNewVersionPath(self,InVersion,InPath):
        tmpPath1 = InPath.split(".")
        if len(tmpPath1) > 0 and tmpPath1[-1] == "livegroup":
            tmpPath2 = InPath[0:-len(".livegroup")]
            tmpPath3 = tmpPath2.split("_")
            if len(tmpPath3) > 0:
                key = tmpPath3[-1]
                if key[0] == 'v' and len(key) == 4:
                    result = tmpPath2[0:-4] + InVersion + ".livegroup"
                else:
                    result = tmpPath2 + "_" + InVersion + ".livegroup"
            else:
                result = tmpPath2 + InVersion
        else:
            result = InPath + "_" + InVersion + ".livegroup"
        return result
    def publish(self):
        if self.primission == "Artist":
            return
        listModel = self.listModel
        uploadList = []
        for i in range(listModel.rowCount()):
            if listModel.item(i).checkState() == 0:
                "Do Nothing"
            else:
                item = listModel.item(i).text()
                uploadList.append(str(item))
        for uploadItem in uploadList:
            if uploadItem in self.dLiveGroupData:
                versionList = self.dLiveGroupData[uploadItem].keys()
                versionList.sort(reverse=True)
                newVersion = self.getNewVersionKey(versionList[0])
            else:
                newVersion = "v001"
            if uploadItem in self.aRememberOpData:
                tmpPath = self.aRememberOpData[uploadItem][1]
                info = self.aRememberOpData[uploadItem][2]
                self.aRememberOpData[uploadItem][0] = newVersion
            else:
                tmpPath = self.dLiveGroupSourceMap[uploadItem]
                info = ''
            path = self.getNewVersionPath(newVersion, tmpPath)
            if uploadItem in self.dLiveGroupData:
                self.dLiveGroupData[uploadItem][newVersion] = [path, info]
            else:
                self.dLiveGroupData[uploadItem] = {newVersion: [path, info]}


            currentLiveGroup = NodegraphAPI.GetNode(uploadItem)
            ##################################################################
            # Reload the LiveGroup at first to publish a old version as latest
            currentLiveGroup.getParameter("source").setValue(str(tmpPath), 0)
            currentLiveGroup.reloadFromSource()
            currentLiveGroup.publishAssetAndFinishEditingContents(path)
        
        self.clean()

        if self.configPath == "":
            aLiveGroupSource = []
            for item in self.aRememberOpData:
                tmp = self.aRememberOpData[item][1]
                Len = len(tmp.split("/")[-1])
                aLiveGroupSource.append(tmp[:-Len])
            for item in self.dLiveGroupSourceMap:
                tmp = self.dLiveGroupSourceMap[item]
                Len = len(tmp.split("/")[-1])
                aLiveGroupSource.append(tmp[:-Len])

            self.configPath = self.getMostCount(aLiveGroupSource)[0] + "config.json"
        file = open(self.configPath, "wb")
        json.dump(self.dLiveGroupData, file, indent=3)
        file.close()
        dialog = theDialog(self)
        dialog.showWindows(content="All selected LiveGroup have  uploaded!", plug="Confirm")
    def reload(self):
        listModel = self.listModel
        reloadList = []
        for i in range(listModel.rowCount()):
            if listModel.item(i).checkState() == 0:
                "Do Nothing"
            else:
                item = listModel.item(i).text()
                reloadList.append(str(item))

        for reloadItem in reloadList:
            if reloadItem not in self.dLiveGroupData:
                continue
            versionList = self.dLiveGroupData[reloadItem].keys()
            versionList.sort(reverse=True)
            version = versionList[0]
            path = self.dLiveGroupData[reloadItem][version][0]
            node = NodegraphAPI.GetNode(reloadItem)
            node.getParameter("source").setValue(str(path), 0)
            node.reloadFromSource()
        dialog = theDialog(self)
        dialog.showWindows(content="All selected LiveGroup have updated to\n the latest version!", plug="Confirm")
    def getMostCount(self,InList):
        ################################################################
        # When we start this plugin,we should get a _config.json file
        # at first,we will get this file`s path from the LiveGroup Node,
        # the most times that the path appear will be located as the
        # _config.json file path.
        countMap = {}
        result = []
        for item in InList:
            counter = 0
            for iterator in InList:
                if iterator == item:
                    counter += 1
            countMap[item] = counter
        for item in countMap:
            if countMap[item] == max(countMap.values()):
                result.append(item)
        return result
    def getPrimission(self):
        import getpass
        import production.shotgun_connection as shotgun
        sys.path.append('/mnt/work/soft/tank/studio/install/core/python')
        import sgtk

        SG = shotgun.Connection('get_project_info').sg
        #user = "liyidong"
        user = getpass.getuser()
        sgData = SG.find_one('HumanUser', [['login', 'is', user]], ['permission_rule_set'])
        primission = sgData["permission_rule_set"]["name"]
        if user == "xukai":
            primission = "Lead"
        return primission
    def setEnabledWidget(self):
        if self.primission == "Lead":
            self.editButton.setEnabled(True)
            self.saveButton.setEnabled(True)
            self.publishButton.setEnabled(True)
            self.reloadButton.setEnabled(True)
        else:
            self.editButton.setEnabled(False)
            self.saveButton.setEnabled(False)
            self.publishButton.setEnabled(False)
            self.reloadButton.setEnabled(True)

def main():
    global mainWin
    selfApp = os.path.basename(os.path.basename(sys.argv[0]))
    if selfApp == "maya.exe":
        mainWin = theDialog()
    elif selfApp.split('.')[-1] == 'py':
        app = QtGui.QApplication(sys.argv)
        mainWin = LiveGroupManager()
        mainWin.showWindows()
        sys.exit(app.exec_())
    elif selfApp == 'Main.pyc':
        mainWin = LiveGroupManager()
        mainWin.showWindows()
    else:
        mainWin = LiveGroupManager()



