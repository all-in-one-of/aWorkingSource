# -*- coding: utf-8 -*-
"""
Created on Wed Sep 10 13:49:52 2014
@author: libin
"""
import sys
from PySide.QtCore import *
from PySide.QtGui import *
from PySide.QtWebKit import *
from PySide import QtGui, QtCore
from myButton import PushButton
class Mywindow(QWidget):
    def __init__(self,parent=None):
        super(Mywindow,self).__init__(parent)
        self.setWindowTitle('my order edit.')
        self.setFixedSize(330,400)
        #定义的关闭按钮
        self.close_button = PushButton(self)
        self.close_button.loadPixmap('C:/Users/kai.xu/Pictures/close.png')
        self.connect(self.close_button, QtCore.SIGNAL('clicked()'),self, QtCore.SLOT('close()'))
        pe = QtGui.QPalette()
        pe.setColor(QtGui.QPalette.WindowText,QtCore.Qt.white)
        label_user = QtGui.QLabel(u"账号")
        label_user.setParent(self)
        label_user.setPalette(pe)
        label_user.setFont(QtGui.QFont("Roman times",10,QtGui.QFont.Bold))
        label_user.setGeometry(QtCore.QRect(100, 135, 50, 25))
        #去掉边框的代码
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setWindowFlags(Qt.MSWindowsFixedSizeDialogHint | Qt.FramelessWindowHint)

        """ set background picture """
        palette = QtGui.QPalette()
        #palette.setColor(self.backgroundRole(), QColor(192,253,123))# 设置背景颜色
        palette.setBrush(self.backgroundRole(), QtGui.QBrush(QtGui.QPixmap('C:/Users/kai.xu/Pictures/image.jpg')))# 设置背景图片
        self.setPalette(palette)
        self.setAutoFillBackground(True) # 不设置也可以

    #############################################################################
    def mousePressEvent(self,event):
       #鼠标点击事件
       if event.button() == QtCore.Qt.LeftButton:
           self.dragPosition = event.globalPos() - self.frameGeometry().topLeft()
           event.accept()

    def mouseMoveEvent(self,event):
       #鼠标移动事件
        if event.buttons() ==QtCore.Qt.LeftButton:
            self.move(event.globalPos() - self.dragPosition)
            event.accept()
    # This two functions make the FramelessWindow moving
    #############################################################################
if __name__ == '__main__':
    app = QApplication(sys.argv)
    mywindow =Mywindow()
    mywindow.show()
    sys.exit(app.exec_())
