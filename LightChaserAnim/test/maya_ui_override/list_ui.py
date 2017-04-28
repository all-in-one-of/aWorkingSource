import sip
import PyQt4.QtGui as QtGui
import PyQt4.QtCore as QtCore
import maya.OpenMayaUI as omui

allUI = cmds.lsUI(ed=True)
for i in range(len(allUI)):
    print i,allUI[i]
parentLayout = cmds.editor('polyTexturePlacementPanel1', q=True, p=True)
uvLayout = sip.wrapinstance(long(omui.MQtUtil.findLayout(parentLayout)), QtCore.QObject)
uvLayout = uvLayout.children()[0].layout()
qtButton = QtGui.QPushButton('TEST')
uvLayout.addWidget(qtButton)


import maya.OpenMayaUI as omui
from PySide import QtGui,QtCore
import sys,os,shiboken,time,json,datetime
import sip
mayaWindowParent = shiboken.wrapInstance(long(omui.MQtUtil.mainWindow()), QtGui.QWidget)
print long(omui.MQtUtil.mainWindow())


ptr = omui.MQtUtil.mainWindow()
mainWin = sip.wrapinstance(long(ptr), QtGui.QWidget)
mainLay = mainWin.layout()
menuLayout = mainLay.item()
print menuLayout