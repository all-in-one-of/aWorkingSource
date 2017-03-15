import sip
import PyQt4.QtGui as QtGui
import PyQt4.QtCore as QtCore
import maya.OpenMayaUI as omui

allUI = cmds.lsUI(ed=True)
for ui in allUI:
    print ui
#parentLayout = cmds.editor('polyTexturePlacementPanel1', q=True, p=True)
#uvLayout = sip.wrapinstance(long(omui.MQtUtil.findLayout(parentLayout)), QtCore.QObject)
#uvLayout = uvLayout.children()[0].layout()
#qtButton = QtGui.QPushButton('TEST')
#uvLayout.addWidget(qtButton)


