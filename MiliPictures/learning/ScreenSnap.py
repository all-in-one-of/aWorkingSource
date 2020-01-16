#!/usr/bin/env python
__author__ = 'kai.xu'

####################################################################################
#
# This app is a screen snap app
#
####################################################################################

import sys
from PySide import QtCore, QtGui


class ScreenCutView(QtGui.QGraphicsView):
    def __init__(self,coords):
        self.scene = QtGui.QGraphicsScene()
        self.coords = coords
        self.originalPixmap = QtGui.QPixmap.grabWindow(QtGui.QApplication.desktop().winId(),self.coords[0],self.coords[1],self.coords[2],self.coords[3])
        self.scene.setBackgroundBrush(self.originalPixmap)
        self.scene.setSceneRect(QtCore.QRectF(0, 0, self.coords[2], self.coords[3]))
        super(ScreenCutView,self).__init__(self.scene)

        self.toolBar()
        self.timer = QtCore.QTimer()
        self.rectangleFlag = False
        self.ellipseFlag = False
        self.lineFlag = [False,False]
        self.textFlag = False
        self.movePointX = 0
        self.movePointY = 0

        self.setWindowFlags(QtCore.Qt.FramelessWindowHint|QtCore.Qt.WindowStaysOnTopHint)
        self.setWindowTitle("Screenshot")
        self.resize(self.coords[2]+5, self.coords[3]+5)
        self.move(self.coords[0],self.coords[1])
        self.show()

    def toolBar(self):
        self.widget = QtGui.QWidget()
        self.widget.setBackgroundRole(QtGui.QPalette.Highlight)

        button1 = self.createButton('Rectangle',self.drawRectangle)
        button2 = self.createButton('Ellipse',self.drawEllipse)
        button3 = self.createButton('Line',self.drawLine)
        button4 = self.createButton('Text',self.writeText)
        button5 = self.createButton('Clear',self.clear)
        button6 = self.createButton('Save',self.save)
        button7 = self.createButton('Clipboard',self.clipboard)
        self.fontCombo = QtGui.QFontComboBox()
        #self.fontCombo.setEnabled(False)
        self.fontSizeCombo = QtGui.QComboBox()
        #self.fontSizeCombo.setEnabled(False)
        self.fontSizeCombo.setEditable(True)

        for i in range(12, 48, 2):
            self.fontSizeCombo.addItem(str(i))
        validator = QtGui.QIntValidator(2, 64, self)
        self.fontSizeCombo.setValidator(validator)

        self.fontColorCombo = QtGui.QComboBox()
        self.fontColorCombo.setEditable(True)
        #self.fontColorCombo.setEnabled(False)
        colorList = ['red','black','blue','white']
        for item in colorList:
            self.fontColorCombo.addItem(item)

        mainLayOut = QtGui.QVBoxLayout()
        layOut1 = QtGui.QHBoxLayout()
        layOut2 = QtGui.QHBoxLayout()
        layOut3 = QtGui.QHBoxLayout()
        layOut1.setSizeConstraint(QtGui.QLayout.SetFixedSize)
        layOut1.addWidget(button1)
        layOut1.addWidget(button2)
        layOut1.addWidget(button3)
        layOut2.addWidget(self.fontCombo)
        layOut2.addWidget(self.fontSizeCombo)
        layOut2.addWidget(self.fontColorCombo)
        layOut2.addWidget(button4)
        layOut3.addWidget(button5)
        layOut3.addWidget(button6)
        layOut3.addWidget(button7)
        mainLayOut.addLayout(layOut1)
        mainLayOut.addLayout(layOut2)
        mainLayOut.addLayout(layOut3)
        self.widget.setLayout(mainLayOut)
        self.widget.setWindowFlags(QtCore.Qt.FramelessWindowHint|QtCore.Qt.WindowStaysOnTopHint)
        self.widget.setMinimumSize(150,50)
        self.widget.resize(220,100)
        self.widget.setWindowTitle("ToolBar")
        widgetWidth = self.widget.width()
        widgetHeight = self.widget.height()

        self.widget.show()
        x1 = self.coords[0]
        y1 = self.coords[1]
        x2 = self.coords[0]+self.coords[2]
        y2 = self.coords[1]+self.coords[3]
        global width,height

        if not (x2 + widgetWidth) > width and not (y2 + widgetHeight) > height:
            self.widget.move(self.coords[0]+self.coords[2], self.coords[1]+self.coords[3])
        elif not (x1 - widgetWidth) < 0 and not (y1 - widgetHeight) < 0:
            self.widget.move(self.coords[0]-widgetWidth, self.coords[1]-widgetHeight)
        elif not (x2 + widgetWidth) > width and not (y1 - widgetHeight) < 0:
            self.widget.move(x2, y1 - widgetHeight)
        elif not (x1 - widgetWidth) < 0 and not (y2 + widgetHeight) > height:
            self.widget.move(x1 - widgetWidth, y2)
        else:
            self.widget.move(x1, y1)


    def createButton(self,title,function):
        button = QtGui.QPushButton(title)
        button.clicked.connect(function)
        return button


    def drawRectangle(self):
        self.rectangleFlag = True
        self.ellipseFlag = False
        self.lineFlag[0] = False
        self.lineFlag[1] = False
        self.textFlag = False


    def drawEllipse(self):
        self.rectangleFlag = False
        self.ellipseFlag = True
        self.lineFlag[0] = False
        self.lineFlag[1] = False
        self.textFlag = False


    def drawLine(self):
        self.rectangleFlag = False
        self.ellipseFlag = False
        self.lineFlag[0] = True
        self.lineFlag[1] = False
        self.textFlag = False


    def writeText(self):
        self.rectangleFlag = False
        self.ellipseFlag = False
        self.lineFlag[0] = False
        self.lineFlag[1] = False
        self.textFlag = True


    def mousePressEvent(self,event):
        #begin to draw rectangle
        if self.rectangleFlag == True:
            if event.button() == QtCore.Qt.LeftButton:
                self.pointX = event.x()
                self.pointY = event.y()
                self.movePointX = 0
                self.movePointY = 0

                self.element = QtGui.QGraphicsRectItem()
                self.scene.addItem(self.element)
                self.timer.timeout.connect(lambda:self.element.setRect(self.pointX, self.pointY, self.movePointX, self.movePointY))
                self.timer.start(50)

        #begin to draw ellipse
        if self.ellipseFlag == True:
            if event.button() == QtCore.Qt.LeftButton:
                self.pointX = event.x()
                self.pointY = event.y()
                self.movePointX = 0
                self.movePointY = 0
                ####################################################
                # Use the same self.rect argument
                # because python can not delete the self argument
                # you will create rectangle and ellipse at same time
                ####################################################
                self.element = QtGui.QGraphicsEllipseItem()
                self.scene.addItem(self.element)
                self.timer.timeout.connect(lambda:self.element.setRect(self.pointX, self.pointY, self.movePointX, self.movePointY))
                self.timer.start(50)


        #begin to draw line
        if self.lineFlag[0] == True and self.lineFlag[1] == False:
            if event.button() == QtCore.Qt.LeftButton:
                self.pointX = event.x()
                self.pointY = event.y()
                self.line = QtGui.QGraphicsLineItem(QtCore.QLineF(event.pos(),event.pos()))
                self.line.setPen(QtGui.QPen(QtCore.Qt.blue, 2))
                self.scene.addItem(self.line)
                self.lineFlag[1] = True

        #begin to wirte text
        if self.textFlag == True:
            if event.button() == QtCore.Qt.LeftButton:
                self.pointX = event.x()
                self.pointY = event.y()
                self.element = QtGui.QGraphicsTextItem()
                self.element.setTextInteractionFlags(QtCore.Qt.TextEditorInteraction)
                self.element.setPos(self.pointX,self.pointY)

                cursor = self.element.textCursor()
                self.element.setTextCursor(cursor)

                font = self.fontCombo.currentFont()
                font.setPointSize(int(self.fontSizeCombo.currentText()))
                color = self.fontColorCombo.currentText()
                if color == 'red':
                    textColor = QtCore.Qt.red
                elif color == 'black':
                    textColor = QtCore.Qt.black
                elif color == 'blue':
                    textColor = QtCore.Qt.blue
                elif color == 'white':
                    textColor = QtCore.Qt.white
                else:
                    pass
                self.element.setFont(font)
                self.element.setDefaultTextColor(textColor)
                self.scene.addItem(self.element)

        super(ScreenCutView, self).mousePressEvent(event)


    def mouseMoveEvent(self,event):
        if self.rectangleFlag == True or self.ellipseFlag == True:
            self.movePointX = event.x()-self.pointX
            self.movePointY = event.y()-self.pointY
        if self.lineFlag[0] == True and self.lineFlag[1] == True:
            newLine = QtCore.QLineF(self.line.line().p1(), event.pos())
            self.line.setLine(newLine)
        super(ScreenCutView, self).mouseMoveEvent(event)


    def mouseReleaseEvent(self, event):
        if self.rectangleFlag == True or self.ellipseFlag == True:
            if event.button() == QtCore.Qt.LeftButton:
                self.timer.stop()
        self.lineFlag[1] = False

        super(ScreenCutView, self).mouseReleaseEvent(event)


    def clear(self):
        self.scene.clear()


    def save(self):
        x1 = self.coords[0]
        y1 = self.coords[1]
        x2 = self.coords[2]
        y2 = self.coords[3]
        self.originalPixmap = QtGui.QPixmap.grabWindow(QtGui.QApplication.desktop().winId(),x1,y1,x2,y2)
        format = 'png'
        initialPath = QtCore.QDir.currentPath() + "/untitled." + format

        fileName,_ = QtGui.QFileDialog.getSaveFileName(self, "Save As",
                initialPath,
                "%s Files (*.%s);;All Files (*)" % (format.upper(), format))
        if fileName:
            #print fileName, format
            self.originalPixmap.save(fileName, format)
            self.close()
            self.widget.close()


    def clipboard(self):
        x1 = self.coords[0]
        y1 = self.coords[1]
        x2 = self.coords[2]
        y2 = self.coords[3]
        self.originalPixmap = QtGui.QPixmap.grabWindow(QtGui.QApplication.desktop().winId(),x1,y1,x2,y2)
        clipboard = QtGui.QApplication.clipboard()
        mimeData = QtCore.QMimeData()
        mimeData.setImageData(self.originalPixmap)
        clipboard.setMimeData(mimeData)
        global deleteFlag,clipboardImage
        clipboardImage = str(time.time()).replace('.', '_')+'.png'
        self.originalPixmap.save('D:/%s'%clipboardImage, 'png')
        deleteFlag = True
        #print self.clipboardImage
        self.close()
        self.widget.close()


    def keyPressEvent(self, event):
         if event.key() == QtCore.Qt.Key_Escape:
            self.close()
            self.widget.close()
         super(ScreenCutView, self).keyPressEvent(event)


class LockScreenView(QtGui.QGraphicsView):
    def __init__(self,scene):
        super(LockScreenView,self).__init__(scene)
        self.scene = scene
        self.setWindowFlags(QtCore.Qt.FramelessWindowHint|QtCore.Qt.WindowStaysOnTopHint)
        self.timer = QtCore.QTimer()
        self.pointX = 0
        self.pointY = 0
        self.movePointX = 0
        self.movePointY = 0


    def mousePressEvent(self,event):
        if event.button() == QtCore.Qt.LeftButton:
            self.pointX = event.x()
            self.pointY = event.y()
            self.movePointX = 0
            self.movePointY = 0

            self.rect = QtGui.QGraphicsRectItem()
            self.scene.addItem(self.rect)
            self.timer.timeout.connect(lambda:self.drawRect())
            self.timer.start(50)


    def mouseMoveEvent(self,event):
        self.movePointX = event.x()-self.pointX
        self.movePointY = event.y()-self.pointY


    def mouseReleaseEvent(self, event):
        if event.button() == QtCore.Qt.LeftButton:
            self.timer.stop()
            self.doScreenshotAgain()


    def keyPressEvent(self, event):
         if event.key() == QtCore.Qt.Key_Escape:
            self.close()


    def drawRect(self):
        self.rect.setRect(self.pointX, self.pointY, self.movePointX, self.movePointY)


    def doScreenshotAgain(self):
        self.close()
        x1 = self.pointX
        y1 = self.pointY
        x2 = self.movePointX
        y2 = self.movePointY
        self.coords = (x1,y1,x2,y2)

        self.graphicsView = ScreenCutView(self.coords)


class Screenshot():
    def __init__(self):
        global width,height,deleteFlag
        deleteFlag = False
        self.originalPixmap = QtGui.QPixmap.grabWindow(QtGui.QApplication.desktop().winId())
        width = self.originalPixmap.width()
        height = self.originalPixmap.height()
        self.scene = QtGui.QGraphicsScene()
        self.view = LockScreenView(self.scene)
        #self.view = QtGui.QGraphicsView(self.scene)
        self.view.setWindowFlags(QtCore.Qt.FramelessWindowHint|QtCore.Qt.WindowStaysOnTopHint)
        self.doScreenshot()

    def doScreenshot(self):
        self.scene.setBackgroundBrush(self.originalPixmap)
        self.scene.setSceneRect(QtCore.QRectF(0, 0, width, height))

        self.view.setWindowTitle("Screenshot")
        self.view.resize(width+5, height+5)
        self.view.move(-1,-1)
        self.view.show()


def main():
    global selfScreenshot
    ########################################################################
    # If this app be used in maya,
    # you should define a global variable like 'selfScreenshot'.
    # Well,that`s it!
    ########################################################################
    app = QtGui.QApplication(sys.argv)
    selfScreenshot = Screenshot()
    sys.exit(app.exec_())



if __name__ == '__main__':
    main()
