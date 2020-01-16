#coding:utf-8
__author__ = 'kai.xu'


from PySide import QtGui,QtCore
import sys,os,shiboken,time,json,datetime
import Mili.Module.utilties.MiliUser as MiliUser
import maya.cmds as cmds
from redmine import Redmine
import Mili.Module.utilties.Get as Get
import Mili.Module.utilties.Path as Path
import Mili.Module.utilties.lsrunas as lsrunas
reload(lsrunas)
reload(MiliUser)
reload(Get)
reload(Path)

##################################################################################################
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


# 以上三个class都是截屏的模块类
# 调用 Screenshot 即可执行
##################################################################################################

#定义全局变量
def globalDefine():
    global width,height,selfWidget,globalDepartment,globalPlay,globalShot,globalName,globalStartDate,selfApp,deleteFlag
    deleteFlag = False
    if 'selfWidget' not in globals():
        selfWidget = None
    width = 0
    height = 0
    globalDepartment = MiliUser.MiliUser()['department']
    globalName = MiliUser.MiliUser()['name']
    selfApp = os.path.basename(os.path.basename(sys.argv[0]))
    sceneTmp = cmds.file(q=True,sceneName=True)
    sceneTmpList = sceneTmp.split('/')[-1]
    globalPlay = sceneTmpList.split('_')[0]
    try:
        globalShotTmp = sceneTmpList.split('_')[1]
        globalShot = globalShotTmp.split('.')[0]
    except:
        globalShot = ' '
    globalStartDate = time.strftime("%Y-%m-%d")


#报错工具主类
class RedmineErroRepoter(QtGui.QWidget):
    '''
    日志（2015-11-3）：删除了预计完成日期和完成日期，因为Redmine上对这个数值有改动导致无法上传。
    '''
    def __init__(self, parent=None):
        globalDefine()
        if selfApp.lower()!='maya.exe':
            app = QtGui.QApplication(sys.argv)
        else:
            import maya.OpenMayaUI as omui
            try:
                mayaWindowParent = shiboken.wrapInstance(long(omui.MQtUtil.mainWindow()), QtGui.QWidget)
            except:
                pass
            parent = mayaWindowParent
        super(RedmineErroRepoter,self).__init__(parent)
        self.imageFilePath = ''
        self.imageFileName = ''
        global selfWidget
        if selfWidget != None:
            selfWidget.close()
        selfWidget = self
        self.setWindowTitle(u'Redmine报错工具')
        boldFont = QtGui.QFont()
        boldFont.setBold(True)

        LogonGroupBox = QtGui.QGroupBox(u'用户登录')
        NameLabel = QtGui.QLabel()
        NameLabel.setText(u'用户* :')
        NameLabel.setFixedWidth(50)
        NameLabel.setFont(boldFont)
        self.usr = QtGui.QLineEdit()
        self.usr.setFixedWidth(200)
        PasswordLabel = QtGui.QLabel()
        PasswordLabel.setText(u'密码* :')
        PasswordLabel.setFixedWidth(50)
        PasswordLabel.setFont(boldFont)
        self.password = QtGui.QLineEdit()
        self.password.setFixedWidth(200)
        self.password.setEchoMode(QtGui.QLineEdit.Password)
        grid0 = QtGui.QGridLayout()
        grid0.addWidget(NameLabel,1,0)
        grid0.addWidget(self.usr,1,1)
        grid0.addWidget(PasswordLabel,2,0)
        grid0.addWidget(self.password,2,1)
        LogonGroupBox.setFixedHeight(100)
        LogonGroupBox.setLayout(grid0)

        Label1 = QtGui.QLabel()
        Label1.setText(u'跟踪*:')
        Label1.setFont(boldFont)
        self.tracking = QtGui.QComboBox()
        self.tracking.setEnabled(True)
        self.tracking.setEditable(False)
        self.tracking.addItem(u'问题')
        self.tracking.setFixedWidth(50)
        Label2 = QtGui.QLabel()
        Label2.setText(u'主题*:')
        Label2.setFont(boldFont)
        self.topic = QtGui.QLineEdit()
        self.topic.setMinimumWidth(400)
        Label3 = QtGui.QLabel()
        Label3.setText(u'描述:')
        screenShotButton = QtGui.QPushButton(u'截屏')
        screenShotButton.clicked.connect(self.screenShot)
        screenShotButton.setFixedWidth(50)
        previewButton = QtGui.QPushButton(u'预览')
        previewButton.clicked.connect(self.preview)
        previewButton.setFixedWidth(50)
        uploadImageButton = QtGui.QPushButton(u'上传图片')
        uploadImageButton.clicked.connect(self.uploadImage)
        uploadImageButton.setFixedWidth(60)
        toolLayout = QtGui.QHBoxLayout()
        toolLayout.addWidget(uploadImageButton)
        toolLayout.addWidget(previewButton)
        toolLayout.addWidget(screenShotButton)
        toolLayout.addStretch(1)
        self.description = QtGui.QTextEdit()
        grid1 = QtGui.QGridLayout()
        grid1.addWidget(Label1,1,0)
        grid1.addWidget(self.tracking,1,1)
        grid1.addWidget(Label2,2,0)
        grid1.addWidget(self.topic,2,1)
        grid1.addWidget(Label3,3,0)
        grid1.addLayout(toolLayout,3,1)
        grid1.addWidget(self.description,4,1,5,1)

        conditionLabel = QtGui.QLabel()
        conditionLabel.setText(u'状态*:')
        conditionLabel.setFont(boldFont)
        conditionLabel.setAlignment(QtCore.Qt.AlignRight)
        self.condition = QtGui.QComboBox()
        self.condition.addItem(u'新建')
        self.condition.addItem(u'进行中')
        self.condition.addItem(u'暂停')
        self.condition.addItem(u'已解决')
        self.condition.addItem(u'反馈')
        self.condition.setFixedWidth(60)
        priorityLabel = QtGui.QLabel()
        priorityLabel.setText(u'优先级*:')
        priorityLabel.setFont(boldFont)
        priorityLabel.setAlignment(QtCore.Qt.AlignRight)
        self.priority = QtGui.QComboBox()
        self.priority.addItem(u'低')
        self.priority.addItem(u'普通')
        self.priority.addItem(u'重要')
        self.priority.addItem(u'紧急')
        self.priority.addItem(u'重要紧急')
        self.priority.setFixedWidth(70)
        assignedLabel = QtGui.QLabel()
        assignedLabel.setText(u'指派给*:')
        assignedLabel.setFont(boldFont)
        assignedLabel.setAlignment(QtCore.Qt.AlignRight)
        self.assigned = QtGui.QComboBox()
        self.assigned.setFixedWidth(70)
        self.assignedTo()
        #finishDateLabel = QtGui.QLabel()
        #finishDateLabel.setText(u'完成日期:')
        #finishDateLabel.setAlignment(QtCore.Qt.AlignRight)
        self.finishDate = QtGui.QLineEdit()
        global globalStartDate
        tmpFinishDate = globalStartDate.split('-')
        self.finishDateInPlan = tmpFinishDate[0]+'-'+tmpFinishDate[1]+'-'+str((int(tmpFinishDate[2])+5))
        self.finishDate.setText(self.finishDateInPlan)
        playLabel = QtGui.QLabel()
        playLabel.setText(u'场次:')
        playLabel.setAlignment(QtCore.Qt.AlignRight)
        self.play = QtGui.QLineEdit()
        global globalPlay
        self.play.setText(globalPlay)
        shotLabel = QtGui.QLabel()
        shotLabel.setText(u'镜号:')
        shotLabel.setAlignment(QtCore.Qt.AlignRight)
        self.shot = QtGui.QLineEdit()
        global globalShot
        self.shot.setText(globalShot)
        submitDepartmentLabel = QtGui.QLabel()
        submitDepartmentLabel.setText(u'提出部门*:')
        submitDepartmentLabel.setFont(boldFont)
        submitDepartmentLabel.setAlignment(QtCore.Qt.AlignRight)
        self.submitDepartment = QtGui.QComboBox()
        global globalDepartment
        self.submitDepartment.addItem(globalDepartment)
        submitPersonLabel = QtGui.QLabel()
        submitPersonLabel.setText(u'提出问题者*:')
        submitPersonLabel.setFont(boldFont)
        submitPersonLabel.setAlignment(QtCore.Qt.AlignRight)
        self.submitPerson = QtGui.QLineEdit()
        global globalName
        self.submitPerson.setText(globalName)
        reasonLabel = QtGui.QLabel()
        reasonLabel.setText(u'问题原因:')
        reasonLabel.setAlignment(QtCore.Qt.AlignRight)
        self.reason = QtGui.QLineEdit()
        grid2 = QtGui.QGridLayout()
        grid2.addWidget(conditionLabel,1,0)
        grid2.addWidget(self.condition,1,1)
        grid2.addWidget(priorityLabel,2,0)
        grid2.addWidget(self.priority,2,1)
        grid2.addWidget(assignedLabel,3,0)
        grid2.addWidget(self.assigned,3,1)
        grid2.addWidget(QtGui.QLabel(' '))
        #grid2.addWidget(finishDateLabel,5,0)
        #grid2.addWidget(self.finishDate,5,1)
        grid2.addWidget(playLabel,6,0)
        grid2.addWidget(self.play,6,1)
        grid2.addWidget(shotLabel,7,0)
        grid2.addWidget(self.shot,7,1)
        grid2.addWidget(submitDepartmentLabel,8,0)
        grid2.addWidget(self.submitDepartment,8,1)
        grid2.addWidget(submitPersonLabel,9,0)
        grid2.addWidget(self.submitPerson,9,1)
        grid2.addWidget(reasonLabel,10,0)
        grid2.addWidget(self.reason,10,1)

        startDateLabel = QtGui.QLabel()
        startDateLabel.setText(u'开始日期:')
        startDateLabel.setAlignment(QtCore.Qt.AlignRight)
        self.startDate = QtGui.QLineEdit()
        #global globalStartDate
        self.startDate.setText(globalStartDate)
        #finishDatePlanLabel = QtGui.QLabel()
        #finishDatePlanLabel.setText(u'计划完成日期:')
        #finishDatePlanLabel.setAlignment(QtCore.Qt.AlignRight)
        self.finishDatePlan = QtGui.QLineEdit()
        self.finishDatePlan.setText(self.finishDateInPlan)
        finishTimeLabel = QtGui.QLabel()
        finishTimeLabel.setText(u'预期时间:')
        finishTimeLabel.setAlignment(QtCore.Qt.AlignRight)
        self.finishTime = QtGui.QLineEdit()
        self.finishTime.setText(u'小时')
        self.finishTime.setFixedWidth(100)
        finishPercentLabel = QtGui.QLabel()
        finishPercentLabel.setText(u'% 完成:')
        finishPercentLabel.setAlignment(QtCore.Qt.AlignRight)
        self.finishPercent = QtGui.QComboBox()
        self.finishPercent.setFixedWidth(60)
        self.finishPercent.addItem('0%')
        self.finishPercent.addItem('10%')
        self.finishPercent.addItem('20%')
        self.finishPercent.addItem('30%')
        self.finishPercent.addItem('40%')
        self.finishPercent.addItem('50%')
        self.finishPercent.addItem('60%')
        self.finishPercent.addItem('70%')
        self.finishPercent.addItem('80%')
        self.finishPercent.addItem('90%')
        self.finishPercent.addItem('100%')
        finalSolutionLabel = QtGui.QLabel()
        finalSolutionLabel.setText(u'终极解决方案:')
        finalSolutionLabel.setAlignment(QtCore.Qt.AlignRight)
        self.finalSolution = QtGui.QLineEdit()
        finalSolutionPersonLabel = QtGui.QLabel()
        finalSolutionPersonLabel.setText(u'解决人:')
        finalSolutionPersonLabel.setAlignment(QtCore.Qt.AlignRight)
        self.finalSolutionPerson = QtGui.QLineEdit()
        belongingLabel = QtGui.QLabel()
        belongingLabel.setText(u'归属*:')
        belongingLabel.setFont(boldFont)
        belongingLabel.setAlignment(QtCore.Qt.AlignRight)
        self.belonging = QtGui.QComboBox()
        self.belonging.addItem(u'生产问题')
        responsiblePersonLabel = QtGui.QLabel()
        responsiblePersonLabel.setText(u'责任人:')
        responsiblePersonLabel.setAlignment(QtCore.Qt.AlignRight)
        self.responsiblePerson = QtGui.QLineEdit()
        responsibleDepartmentLabel = QtGui.QLabel()
        responsibleDepartmentLabel.setText(u'责任部门:')
        responsibleDepartmentLabel.setAlignment(QtCore.Qt.AlignRight)
        self.responsibleDepartment = QtGui.QLineEdit()
        provisionalSolutionLabel = QtGui.QLabel()
        provisionalSolutionLabel.setText(u'临时解决方案:')
        provisionalSolutionLabel.setAlignment(QtCore.Qt.AlignRight)
        self.provisionalSolution = QtGui.QLineEdit()

        projtctNameLabel = QtGui.QLabel()
        projtctNameLabel.setText(u'项目名:')
        projtctNameLabel.setAlignment(QtCore.Qt.AlignRight)
        self.projectName = QtGui.QLineEdit()
        Prj = Get.Get().GetActiveProject()
        self.projectName.setText(Prj)
        propertyNameLabel = QtGui.QLabel()
        propertyNameLabel.setText(u'资产名:')
        propertyNameLabel.setAlignment(QtCore.Qt.AlignRight)
        self.propertyName = QtGui.QLineEdit()
        fileName = cmds.file(q=True,expandName=True).split('/')[-1]
        self.propertyName.setText(fileName)

        grid3 = QtGui.QGridLayout()
        grid3.addWidget(startDateLabel,1,0)
        grid3.addWidget(self.startDate,1,1)
        #grid3.addWidget(finishDatePlanLabel,2,0)
        #grid3.addWidget(self.finishDatePlan,2,1)
        grid3.addWidget(finishTimeLabel,3,0)
        grid3.addWidget(self.finishTime,3,1)
        grid3.addWidget(finishPercentLabel,4,0)
        grid3.addWidget(self.finishPercent,4,1)
        grid3.addWidget(finalSolutionLabel,5,0)
        grid3.addWidget(self.finalSolution,5,1)
        grid3.addWidget(finalSolutionPersonLabel,6,0)
        grid3.addWidget(self.finalSolutionPerson,6,1)
        grid3.addWidget(belongingLabel,7,0)
        grid3.addWidget(self.belonging,7,1)
        grid3.addWidget(responsiblePersonLabel,8,0)
        grid3.addWidget(self.responsiblePerson,8,1)
        grid3.addWidget(responsibleDepartmentLabel,9,0)
        grid3.addWidget(self.responsibleDepartment,9,1)
        grid3.addWidget(provisionalSolutionLabel,10,0)
        grid3.addWidget(self.provisionalSolution,10,1)

        grid3.addWidget(projtctNameLabel,11,0)
        grid3.addWidget(self.projectName,11,1)
        grid3.addWidget(propertyNameLabel,12,0)
        grid3.addWidget(self.propertyName,12,1)

        Layout4 = QtGui.QHBoxLayout()
        Layout4.addLayout(grid2)
        Layout4.addSpacing(100)
        Layout4.addLayout(grid3)

        fileUploadLabel = QtGui.QLabel()
        fileUploadLabel.setText(u'上传文件:')
        fileUploadLabel.setFont(boldFont)
        fileUploadLabel.setAlignment(QtCore.Qt.AlignCenter)
        self.fileUploadComboBox = QtGui.QComboBox()
        self.fileUploadComboBox.setFixedWidth(90)
        self.fileUploadComboBox.addItem(u'请设置*')
        self.fileUploadComboBox.addItem(u'上传当前场景')
        self.fileUploadComboBox.addItem(u'不上传')

        fileUploadLayout = QtGui.QHBoxLayout()
        fileUploadLayout.addStretch(1)
        fileUploadLayout.addWidget(fileUploadLabel)
        fileUploadLayout.addWidget(self.fileUploadComboBox)

        upLoadButton = QtGui.QPushButton(u'上传')
        upLoadButton.setFixedWidth(70)
        upLoadButton.clicked.connect(self.upload)
        closeButton = QtGui.QPushButton(u'关闭')
        closeButton.setFixedWidth(70)
        closeButton.clicked.connect(self.close)
        buttonLayout = QtGui.QHBoxLayout()
        buttonLayout.addStretch(1)
        buttonLayout.addWidget(upLoadButton)
        buttonLayout.addWidget(closeButton)

        self.mainlayout = QtGui.QVBoxLayout()
        self.mainlayout.addWidget(LogonGroupBox)
        self.mainlayout.addSpacing(25)
        self.mainlayout.addLayout(grid1)
        self.mainlayout.addSpacing(25)
        self.mainlayout.addLayout(Layout4)
        self.mainlayout.addSpacing(25)
        self.mainlayout.addLayout(fileUploadLayout)
        self.mainlayout.addSpacing(10)
        self.mainlayout.addLayout(buttonLayout)
        self.setLayout(self.mainlayout)

        flags = QtCore.Qt.WindowFlags()
        flags |= QtCore.Qt.Tool
        self.setWindowFlags(flags)
        self.move(500,100)
        self.show()


    def assignedTo(self):

        redmine = Redmine('http://192.168.100.4/redmine/', key='fb77dd01d1c51f56596f5e3a36fcfc5f1f77d16f')
        users = redmine.user.all()
        profileDir=Path.Path().GetServerPath() + '/TeamConfig'
        MiliWork = {}
        if os.path.isdir(profileDir):
            for i in os.listdir(profileDir):
                name,ext = os.path.splitext(i)
                if ext=='.miliProfile':
                    tmpAccount = MiliUser.MiliUser(userName=name)
                    MiliWork[name] = tmpAccount['department']
        self.pipelineTDs = {}
        for user in users:
            userLogin = user['_attributes']['login']
            userID = user['_attributes']['id']
            if userLogin in list(MiliWork.keys()) and MiliWork[userLogin] == 'PipelineTD':
                tmpMap = {'userID':userID,'userLogin':userLogin}
                self.pipelineTDs[str(user).decode('utf-8')] = tmpMap
                #print userID,userLogin,user
        for item in list(self.pipelineTDs.keys()):
            self.assigned.addItem(item)
        #print self.pipelineTDs


    def screenShot(self):
        global selfView,deleteFlag
        selfView = Screenshot()
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.deleteFlagFresh)
        self.timer.start(50)


    def preview(self):
        pass


    def uploadImage(self):
        format1 = 'png'
        format2 = 'jpg'
        FileDialog = QtGui.QFileDialog.getOpenFileName(self, "Find File", 'D:/',
                                                            "%s Files (*.%s);;%s Files (*.%s);;All Files (*)" % (format1.upper(), format1,format2.upper(), format2))
        self.imageFilePath = FileDialog[0]
        self.imageFileName = FileDialog[0].split('/')[-1]
        if not self.imageFileName == '':
            tmp = self.description.toPlainText()+'\n'+'!%s!'%self.imageFileName
            self.description.setText(tmp)
        #print self.imageFileName
        #print self.imageFilePath


    def checkClipboard(self):
        global deleteFlag,clipboardImage
        if deleteFlag == True:
            self.imageFilePath = 'D:/%s'%clipboardImage
            self.imageFileName = clipboardImage.split('/')[-1]
            print self.imageFilePath
            print self.imageFileName


    def checkClipboardToDelete(self):
        global deleteFlag,clipboardImage
        if deleteFlag == True:
            imageFilePath = 'D:/%s'%clipboardImage
            os.remove(imageFilePath)
        deleteFlag = False


    def deleteFlagFresh(self):
        global deleteFlag,clipboardImage
        if deleteFlag == True:
            tmp = self.description.toPlainText()+'\n'+'!%s!'%clipboardImage
            self.description.setText(tmp)
            self.timer.stop()


    def uploadRedmine(self):
        '''
        print self.usr.text()#用户
        print self.password.text()#密码
        print self.tracking.currentText()#跟踪
        print self.topic.text()#主题
        print self.description.toPlainText()#描述
        print self.condition.currentText()#状态
        print self.priority.currentText()#优先级
        print self.assigned.currentText()#指派给
        print self.finishDate.text()#完成日期
        print self.play.text()#场次
        print self.shot.text()#镜号
        print self.submitDepartment.currentText()#提出部门
        print self.submitPerson.text()#提出问题者
        print self.reason.text()#问题原因
        print self.startDate.text()#开始时间
        print self.finishDatePlan.text()#计划完成时间
        print self.finishTime.text()#预期时间
        print self.finishPercent.currentText()#%完成
        print self.finalSolution.text()#终极解决方案
        print self.finalSolutionPerson.text()#解决人
        print self.belonging.currentText()#归属
        print self.responsiblePerson.text()#责任人
        print self.responsibleDepartment.text()#责任部门
        print self.provisionalSolution.text()#临时解决方案
        return
        '''
        redmine = Redmine('http://192.168.100.4/redmine/', username=self.usr.text(), password=self.password.text())
        #redmine = Redmine('http://192.168.100.4/redmine/', username='kai.xu', password='314159')

        try:
            #issue = redmine.issue.get(str(1609))
            issue = redmine.issue.new()
        except:
            text = u'用户名或者密码错误！'
            self.dialogUI(text)
            print '//redmine.exceptions.AuthError//'
            return
        issue.project_id = str(133)
        self.subject = self.topic.text()
        issue.subject = self.subject

        issue.tracker_id = 1

        issue.description = self.description.toPlainText()
        #issue.status_id

        if self.condition.currentText() == u'新建':
            issue.status_id = 0
        elif self.condition.currentText() == u'进行中':
            issue.status_id = 2
        elif self.condition.currentText() == u'暂停':
            issue.status_id = 1
        elif self.condition.currentText() == u'已解决':
            issue.status_id = 3
        else:
            issue.status_id = 4
        #issue.priority_id

        if self.priority.currentText() == u'低':
            issue.priority_id = 1
        elif self.priority.currentText() == u'普通':
            issue.priority_id = 2
        elif self.priority.currentText() == u'重要':
            issue.priority_id = 3
        elif self.priority.currentText() == u'紧急':
            issue.priority_id = 4
        else:
            issue.priority_id = 5

        issue.assigned_to_id = self.pipelineTDs[self.assigned.currentText()]['userID']
        #issue.watcher_user_ids = [0]
        #issue.parent_issue_id = 0

        issue.start_date = self.startDate.text()

        ##issue.due_date = self.finishDatePlan.text()
        print self.finishDatePlan.text()
        if self.finishTime.text() != u'小时':
            issue.estimated_hours = int(self.finishTime.text()[:-2])
        issue.done_ratio = int(self.finishPercent.currentText().split('%')[0])

        if self.submitDepartment.currentText() == 'PipelineTD':
            submitDepartment = u'其他'
        else:
            submitDepartment = self.submitDepartment.currentText()

        issue.custom_fields = [
            #{u'id': 53, u'value': self.finishDate.text()},#完成时间
            {u'id': 1, u'value':self.play.text()},#场次
            {u'id': 2, u'value': self.shot.text()},#镜号
            {u'id': 5, u'value': submitDepartment},#提出部门
            {u'id': 10, u'value': self.submitPerson.text()},#提出问题
            {u'id': 6, u'value': self.reason.text()},#问题原因
            {u'id': 7, u'value': self.finalSolution.text()},#终极解决方案
            {u'id': 9, u'value': self.finalSolutionPerson.text()},#解决人
            {u'id': 41, u'value': self.responsiblePerson.text()},#责任人
            {u'id': 42, u'value': self.responsibleDepartment.text()},#责任部门
            {u'id': 46, u'value': self.provisionalSolution.text()},#临时解决方案
            {u'id': 58, u'value': self.propertyName.text()},#资产名
            {u'id': 57, u'value': self.projectName.text()}]#项目名

        self.checkClipboard()

        if not self.imageFileName == '':
            issue.uploads = [{'path': self.imageFilePath,'filename':self.imageFileName}]
        #issue.uploads = [{'yohoo.png': 'C:/Users/kai.xu/Desktop/yohoo.png'}]

        try:
            issue.save()
            text = u'上传完成！'
            self.dialogUI(text)
        except:
            text = u'用户名或者密码错误！'
            self.dialogUI(text)
            print '//redmine.exceptions.AuthError//'
            return
        self.checkClipboardToDelete()


    def uploadServer(self):
        flag = self.fileUploadComboBox.currentText()
        if flag == u'请设置*':
            self.dialogUI(u'请设置是否上传错误文件！')
            return False
        elif flag == u'不上传':
            return True
        else:
            tmp1_current_time = str(datetime.datetime.now())
            tmp2_current_time = tmp1_current_time.replace(' ', '_')
            tmp3_current_time = tmp2_current_time.split('.')
            tmp4_current_time = tmp3_current_time[0].split(':')
            current_time = tmp4_current_time[0]+'-'+tmp4_current_time[1]+'-'+tmp4_current_time[2]
            sceneNameLongPath = cmds.file(q=True,sceneName=True)
            sceneName = sceneNameLongPath.split('/')[-1]
            self.ServerRootPath = '//hnas02/project_backup/RedmineErroReport/%s/%s'%(current_time,sceneName)
            self.LocalRootPath = sceneNameLongPath

            tmp = self.description.toPlainText()+'\n'+u'报错文件地址 : %s'%self.ServerRootPath.replace('/','\\')
            self.description.setText(tmp)

            uploadTime = time.time()
            if not os.path.isdir(os.path.dirname(self.ServerRootPath)):
                lsrunas.lsrunas().CreateDir_1(os.path.dirname(self.ServerRootPath))
            lsrunas.lsrunas(uploadTime).UploadFile(self.LocalRootPath, self.ServerRootPath, DeleteSource=False)
            return True


    def upload(self):
        key = self.uploadServer()
        if key == True:
            self.uploadRedmine()
        else:
            return


    def dialogUI(self,text):
        self.dialog = QtGui.QDialog()
        self.dialog.setWindowTitle(u'提示')
        self.dialog.setModal(True)
        Message = QtGui.QLabel(text)
        Message.setAlignment(QtCore.Qt.AlignCenter)
        button = QtGui.QPushButton(u'关闭')
        button.clicked.connect(self.dialog.close)
        MainLayout = QtGui.QVBoxLayout()
        MainLayout.addWidget(Message)
        MainLayout.addWidget(button)
        self.dialog.setLayout(MainLayout)
        self.dialog.setFixedSize(150,100)
        self.dialog.show()


if __name__ == '__main__':
    #app = QtGui.QApplication(sys.argv)
    redmine = RedmineErroRepoter()
    #sys.exit(app.exec_())
