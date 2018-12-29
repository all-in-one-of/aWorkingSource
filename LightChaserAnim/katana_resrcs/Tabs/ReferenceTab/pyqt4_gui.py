import os
import sys
from PyQt4 import QtGui,QtCore
 
cImagePath = '/output/projects/pws/shot/m90/m90030/lgt/output/nuke/R1/v011/jpg_L/m90030.lgt.comp.1001.jpg'


from PyQt4 import QtGui
from PyQt4 import QtCore

class kImageWidget(QtGui.QWidget):
    def __init__(self, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.picture = None
        self.setMinimumSize(204.8,85.8)
        self.setMaximumSize(2048,858)

    def setPixmap(self, picture):
        self.picture = picture

    def paintEvent(self, event):
        if self.picture:
            painter = QtGui.QPainter(self)
            painter.setRenderHint(QtGui.QPainter.SmoothPixmapTransform)
            width = self.width()
            height = (width * 429)/1024
            if height < self.height():
                pass
            else:
                height = self.height()
            rect = QtCore.QRect(0, 0, width, height)
            painter.drawPixmap(rect, self.picture)
class Widget(QtGui.QWidget):
    def __init__(self, parent=None):
        QtGui.QWidget.__init__(self, parent)
        lay = QtGui.QVBoxLayout(self)
        lb = kImageWidget(self)
        lb.setPixmap(QtGui.QPixmap(cImagePath))
        self.update()
        lay.addWidget(lb)

from PyQt4.QtGui import *


from PyQt4 import QtGui

class Window(QtGui.QWidget):
    def __init__(self, val):
        QtGui.QWidget.__init__(self)
        t = Widget()
        t.resize(1024,429)
        scroll = QtGui.QScrollArea()
        scroll.setWidget(t)
        scroll.setWidgetResizable(True)
        scroll.setFixedHeight(400)
        layout = QtGui.QFormLayout()
        self.setLayout(layout)
        layout.addWidget(t)


__auther__ = "kevin tsui"

# from Katana import os, QtGui, QtCore, QT4Widgets, QT4FormWidgets, QT4Color, Utils
# import UI4

# Common2D = UI4.FormMaster.NodeHints.Common2D
__all__ = ['MonitorCCPanel']

class kImageWidget(QtGui.QWidget):
    def __init__(self, parent=None):
        QtGui.QLabel.__init__(self, parent)
        self.picture = None

        self.setMinimumSize(250,250)
        self.setMaximumSize(20480,8580)

    def setPixmap(self, picture):
        self.picture = picture

    def paintEvent(self, event):
        if self.picture:
            painter = QtGui.QPainter(self)
            painter.setRenderHint(QtGui.QPainter.SmoothPixmapTransform)
            width = self.parent().width()
            height = self.parent().height()
            if height >  (width * 429)/1024:
                rect = QtCore.QRect(0, (height-(width * 429)/1024)/2, width, (width * 429)/1024)
            else:
                rect = QtCore.QRect((width-(height * 1024)/429)/2, 0, (height * 1024)/429, height)
            painter.drawPixmap(rect, self.picture)

class kTitleLabel(QtGui.QLabel):
    resized = QtCore.pyqtSignal()
    def __init__(self, parent=None):
        QtGui.QToolButton.__init__(self, parent)
        self.resized.connect(self.setButtonGeometry)
        self.h = 40
        self.button = QtGui.QToolButton()
        self.button.setAutoRaise(True)
        self.button.setParent(self)
    def resizeEvent(self, event):
        self.resized.emit()
        return super(kTitleLabel, self).resizeEvent(event)
    def setButtonGeometry(self):

        self.button.setGeometry(self.width()-self.h,0,self.h,self.h)
    def setIcon(self, path):
        self.button.setIcon(QtGui.QIcon(path))        
    def setScript(self,function):
        self.button.clicked.connect(lambda arg=None,args=function:self.runScript(args))
    def runScript(self,script):
        exec (script)
    def setHeight(self,height):
        self.h = height
        self.setFixedHeight(height)

#UI4.Tabs.BaseTab
class ShotColorReference(QtGui.QWidget):
    def __init__(self, parent):
        QtGui.QWidget.__init__(self, parent)
        QtGui.QVBoxLayout(self)
        
        # add title
        self.commonLabel = kTitleLabel()
        iconsDir = os.path.join(os.path.dirname(__file__), 'icons', "settings-work-tool.png")
        self.commonLabel.setIcon(iconsDir)
        self.commonLabel.button.clicked.connect(lambda arg=None,args=None:self.buttonScript())
        self.commonLabel.button.setIconSize(QtCore.QSize(28,28))
        self.commonLabel.setHeight(40)
        self.commonLabel.setMinimumWidth(330)
        palette = QtGui.QPalette()
        palette.setColor(QtGui.QPalette.Background,QtGui.QColor(0.5*255,0.5*255,0.6*255))
        palette.setColor(QtGui.QPalette.WindowText,QtGui.QColor(0.0,0.0,0.0))
        self.commonLabel.setAutoFillBackground(True)
        self.commonLabel.setText("Lighting Shot Color Reference")
        self.commonLabel.setPalette(palette)
        self.commonLabel.setAlignment(QtCore.Qt.AlignCenter)
        self.commonLabel.setFont(QtGui.QFont("Rome times",10,QtGui.QFont.Bold))

        self.layout().setContentsMargins(0,0,0,0)
        self.layout().setSpacing(0)
        self.layout().addWidget(self.commonLabel)
        main_widget = self.create()
        scroll = QtGui.QScrollArea()
        scroll.setWidget(main_widget)
        scroll.setWidgetResizable(True)
        # scroll.setFixedHeight(400)
        self.layout().addWidget(scroll)


    def create(self):
        widget = QtGui.QWidget()
        widget_a = QtGui.QWidget()
        widget_b = QtGui.QWidget()
        palette = widget.palette()
        palette.setColor(widget.backgroundRole(), QtGui.QColor(0.14*255,0.14*255,0.14*255))
        widget.setPalette(palette)

        layout = QtGui.QVBoxLayout(widget)
        layout.setSpacing(5)
        layout.setContentsMargins(0,0,0,0)
        layout_a = QtGui.QVBoxLayout(widget_a)
        layout_b = QtGui.QVBoxLayout(widget_b)
        layout_a.setContentsMargins(10,10,10,10)
        layout_b.setContentsMargins(10,10,10,10)
        widget_line = QtGui.QFrame()
        widget_line.setFrameShape(QtGui.QFrame.HLine)
        widget_line.setFrameShadow(QtGui.QFrame.Sunken)
        layout.addWidget(widget_a)
        layout.addWidget(widget_line)
        layout.addWidget(widget_b)
        
        self.pre_shot_image = kImageWidget()
        pre_shot_image_path = '/output/projects/pws/shot/m90/m90030/lgt/output/nuke/R1/v011/jpg_L/m90030.lgt.comp.1001.jpg'
        pixmap = QtGui.QPixmap(pre_shot_image_path)
        self.pre_shot_image.setPixmap(pixmap)

        self.post_shot_image = kImageWidget()
        post_shot_image_path = '/output/projects/pws/shot/m90/m90060/lgt/output/nuke/R1/v008/jpg_L/m90060.lgt.comp.1001.jpg'
        pixmap = QtGui.QPixmap(post_shot_image_path)
        self.post_shot_image.setPixmap(pixmap)

        self.pre_shot_label = QtGui.QLabel()
        self.pre_shot_label.setFont(QtGui.QFont("Rome times", 6,QtGui.QFont.Bold))
        self.pre_shot_label.setText("<font color='white'>Pre Shot : m90060</font>")
        self.pre_shot_label.setAlignment(QtCore.Qt.AlignLeft)
        self.pre_shot_label.setGeometry(5,5,300,100)
        self.pre_shot_label.setParent(self.pre_shot_image)
        self.post_shot_label = QtGui.QLabel()
        self.post_shot_label.setFont(QtGui.QFont("Rome times", 6,QtGui.QFont.Bold))
        self.post_shot_label.setText("<font color='white'>Post Shot : m90060</font>")
        self.post_shot_label.setAlignment(QtCore.Qt.AlignLeft)
        self.post_shot_label.setGeometry(5,5,300,100)
        self.post_shot_label.setParent(self.post_shot_image)

        layout_a.addWidget(self.pre_shot_image)
        layout_b.addWidget(self.post_shot_image)

        return widget

    def buttonScript(self):
        import SettingWidget
        reload(SettingWidget)
        SettingWidget.main(self)

if __name__ == '__main__':

    import sys
    app = QtGui.QApplication(sys.argv)
    window = ShotColorReference(None)
    window.resize(800,600)
    window.show()
    sys.exit(app.exec_())