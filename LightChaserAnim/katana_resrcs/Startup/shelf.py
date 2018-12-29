# Copyright (c) 2016 Light Chaser Animation. All Rights Reserved.
__author__ = "Kevin Tsui"

from PyQt4 import QtGui,QtCore
from lxml import etree
import os,sys

LCAREZ_RESOURCES_PATH = os.environ.get('LCAREZ_RESOURCES_PATH','/mnt/utility/linked_tools/katana/resources')
STARTUP_PATH = os.path.join(LCAREZ_RESOURCES_PATH,'2.5','Startup')
ICON_PATH = os.path.join(STARTUP_PATH, 'icon')

class shelfGui(object):
    def __init__(self):
        dataReader = opXml()
        self.projectList = dataReader.readProjs(STARTUP_PATH)
        self.buttonList = dataReader.readTools(STARTUP_PATH)
        self.iconPath = ICON_PATH
    def get(self):
        iconPath = self.iconPath
        projectList = self.projectList
        mainWidget = QtGui.QWidget()
        mainLayout = QtGui.QHBoxLayout()
        mainLayout.setContentsMargins(0,0,0,0)
        mainLayout.setSpacing(0)
        self.buttonLayout = QtGui.QHBoxLayout()
        self.buttonLayout.setContentsMargins(0,0,0,0)
        leftFrameWidget = QtGui.QFrame()
        leftFrameWidget.setFrameShadow(QtGui.QFrame.Raised)
        leftFrameWidget.setFrameShape(QtGui.QFrame.StyledPanel)
        leftFrameLayout = QtGui.QVBoxLayout()
        leftFrameLayout.setContentsMargins(0,0,0,0)
        rightFrameWidget = QtGui.QFrame()
        rightFrameWidget.setFrameShadow(QtGui.QFrame.Raised)
        rightFrameWidget.setFrameShape(QtGui.QFrame.StyledPanel)
        rightFrameLayout = QtGui.QVBoxLayout()
        rightFrameLayout.setContentsMargins(0,0,0,0)
        rightFrameLayout.setMargin(0)
        theLabel = QtGui.QLabel()
        theLabel.setPixmap(QtGui.QPixmap(os.path.join(iconPath,"logo.png")))
        self.theComboBox = QtGui.QComboBox()
        self.theComboBox.setFixedHeight(25)
        self.theComboBox.setIconSize(QtCore.QSize(30, 24))
        for item in projectList:
            self.theComboBox.addItem(QtGui.QIcon(os.path.join(iconPath,item["icon"])),item["name"])
        leftFrameWidget.setLayout(leftFrameLayout)
        rightFrameWidget.setLayout(rightFrameLayout)
        for buttonData in self.buttonList:
            if buttonData["type"] == "sub_tool":
                button = self.createButton(os.path.join(iconPath,buttonData['icon']),buttonData['command'])
                self.buttonLayout.addWidget(button)
            elif buttonData["type"] == "spacing":
                self.buttonLayout.addSpacing(int(buttonData['value']))
            else:
                "Light Chaser Animation"

        mainLayout.addSpacing(100)
        mainLayout.addWidget(leftFrameWidget)
        mainLayout.addSpacing(5)
        mainLayout.addWidget(theLabel)
        mainLayout.addSpacing(10)
        # mainLayout.addWidget(self.theComboBox)
        mainLayout.addSpacing(20)
        mainLayout.addLayout(self.buttonLayout)
        mainLayout.addSpacing(10)
        mainLayout.addWidget(rightFrameWidget)
        mainWidget.setLayout(mainLayout)

        return mainWidget
    def createButton(self,InIconPath,InFunction):
        button = QtGui.QToolButton()
        button.setIcon(QtGui.QIcon(InIconPath))
        button.setIconSize(QtCore.QSize(24,24))
        button.setAutoRaise(True)
        if InFunction != "":
            button.clicked.connect(lambda arg=None,args=InFunction:self.runScript(args))
        return button

    def runScript(self,InScript):
        exec (InScript)

class opXml():
    def __init__(self):
        pass
    def writeProjs(self,InPath):
        root = etree.Element("root")
        tree = etree.ElementTree(root)
        child1 = etree.SubElement(root, "project")
        child1.set("name", "CAT")
        child1.set("icon", "logocat.png")
        child1.set("path", "")
        child2 = etree.SubElement(root, "project")
        child2.set("name", "TPR")
        child2.set("icon", "logoteapet.png")
        child2.set("path", "")
        child3 = etree.SubElement(root, "project")
        child3.set("name", "GOD")
        child3.set("icon", "logodoorgod.png")
        child3.set("path", "")
        tree.write(InPath+'shelf_root.xml', pretty_print=True, xml_declaration=True, encoding='utf-8')

    def writeTools(self,InPath):
        root = etree.Element("root")
        tree = etree.ElementTree(root)
        child1 = etree.SubElement(root, "sub_tool")
        child1.set("label", "download")
        child1.set("icon", "downloadcloud.png")
        child1.set("command", "")
        child2 = etree.SubElement(root, "sub_tool")
        child2.set("label", "upload")
        child2.set("icon", "uploadcloud.png")
        child2.set("command", "")
        child3 = etree.SubElement(root, "sub_tool")
        child3.set("label", "tool_box")
        child3.set("icon", "colorwheel.png")
        child3.set("command", "")
        child4 = etree.SubElement(root, "sub_tool")
        child4.set("label", "rocket")
        child4.set("icon", "rocket.png")
        child4.set("command", "")
        child5 = etree.SubElement(root, "spacing")
        child5.set("value","3")
        child6 = etree.SubElement(root, "sub_tool")
        child6.set("label", "edit")
        child6.set("icon", "compose.png")
        child6.set("command", "")
        tree.write(InPath+'shelf_lgt.xml', pretty_print=True, xml_declaration=True, encoding='utf-8')
    def readProjs(self,InPath):
        result = []
        tree = etree.parse(os.path.join(InPath,"shelf_projs.xml"))
        root = tree.getroot()
        for node in root:
            #print node.tag
            tmpMap = {}
            name = node.attrib['name']
            icon = node.attrib['icon']
            path = node.attrib['path']
            tmpMap["name"] = name
            tmpMap["icon"] = icon
            tmpMap["path"] = path
            result.append(tmpMap)
        return result
    def readTools(self,InPath):
        result = []
        tree = etree.parse(os.path.join(InPath,"shelf_tools.xml"))
        root = tree.getroot()
        for node in root:
            if node.tag == "sub_tool":
                tmpMap = {}
                name = node.attrib['label']
                icon = node.attrib['icon']
                command = node.attrib['command']
                tmpMap["label"] = name
                tmpMap["icon"] = icon
                tmpMap["command"] = command
                tmpMap["type"] = node.tag
                result.append(tmpMap)
            elif node.tag == "spacing":
                tmpMap = {}
                name = node.attrib['value']
                tmpMap["value"] = name
                tmpMap["type"] = node.tag
                result.append(tmpMap)
            else:
                "Light Chaser Animation"
        return result
