__author__ = 'kai.xu'
# -*- coding: utf-8 -*-
from PySide import QtGui,QtCore

image=QtGui.QImage()
bgImage=image.load("image",format = 'JPG')

class labelBtn(QtGui.QLabel):
    """自定义图片按钮类 """
    def __init__(self,ID):
        super(labelBtn, self).__init__()
        self.setMouseTracking(True)
        assert isinstance(ID, object)
        self.ID=ID

    def mouseReleaseEvent(self,event):  #注:
        #鼠标点击事件
        self.parent().btnHandle(self.ID)

    def enterEvent(self,event):
        #鼠标进入时间
        self.parent().btnEnter(self.ID)

    def leaveEvent(self,event):
        #鼠标离开事件
        self.parent().btnLeave(self.ID)

class login(QtGui.QMainWindow):
    def __init__(self,parent=None):
        super(login, self).__init__(parent)
        self.setWindowTitle(u"学生体能健康测试软件")
        self.setFixedSize(347,264)
        self.setWindowIcon(QtGui.QIcon("images/umbrella.png"))
        #窗口居中显示
        desktop =QtGui.QApplication.desktop()
        width = desktop.width()
        height = desktop.height()
        self.move((width - self.width())/2, (height - self.height())/2)
        self.setMouseTracking(True)
        #无边框
        self.setWindowFlags(QtCore.Qt.FramelessWindowHint)
        #显示托盘信息
        self.trayIcon = QtGui.QSystemTrayIcon(self)
        self.trayIcon.setIcon(QtGui.QIcon("images/umbrella.png"))
        self.trayIcon.show()
        self.trayIcon.activated.connect(self.trayClick)       #点击托盘
        self.trayMenu()                                       #右键菜单

        label_user = QtGui.QLabel(u"账号",self)
        label_user.setGeometry(QtCore.QRect(125, 135, 50, 22))
        label_passwd = QtGui.QLabel(u"密码",self)
        label_passwd.setGeometry(QtCore.QRect(125, 170, 50, 22))

        self.label = QtGui.QLabel(self)
        self.label.setGeometry(QtCore.QRect(19, 129, 80, 80))
        self.label.setPixmap(QtGui.QPixmap("images/teacher.png"))

        self.lineEdit_user = QtGui.QLineEdit(u"root",self)
        self.lineEdit_user.setGeometry(QtCore.QRect(160, 135, 150, 22))

        self.lineEdit_passwd = QtGui.QLineEdit(u'1234',self)
        self.lineEdit_passwd.setGeometry(QtCore.QRect(160, 170, 150, 22))
        self.lineEdit_passwd.setEchoMode(QtGui.QLineEdit.Password)
        self.lineEdit_passwd.setValidator(QtGui.QRegExpValidator(QtCore.QRegExp("[A-Za-z0-9]+"),self))
        #这里也可以设置QLineEdit背景为透明
        self.pushButton_login = QtGui.QPushButton(QtGui.QIcon("images/login.png"),u"登录",self)
        self.pushButton_login.setGeometry(QtCore.QRect(250, 235, 75, 22))

        self.pushButton_change = QtGui.QPushButton(QtGui.QIcon("images/onetwo.png"),u"身份切换",self)
        self.pushButton_change.setGeometry(QtCore.QRect(10, 235, 75, 22))
        self.pushButton_change.setFlat(True)
        self.pushButton_change.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)

        self.btn_min=labelBtn(1)               #定义最小化按钮 ID:1
        self.btn_min.setParent(self)
        self.btn_min.setGeometry(281,0,27,23)
        self.btn_min.setToolTip(u"最小化")

        self.btn_close=labelBtn(2)              #定义关闭按钮 ID:2
        self.btn_close.setParent(self)
        self.btn_close.setGeometry(310,0,38,21)
        self.btn_close.setToolTip(u"关闭")

        self.connect(self.pushButton_change, QtCore.SIGNAL("clicked()"),self.contextMenu)
        self.connect(self.pushButton_login, QtCore.SIGNAL("clicked()"),self.log_in)

    def contextMenu(self):

        self.userOption = QtGui.QAction(QtGui.QIcon("images/user.png"),u"学生", self)

        self.rootOption = QtGui.QAction(QtGui.QIcon("images/root.png"),u"老师", self)



        self.userOption.triggered.connect(self.user)

        self.rootOption.triggered.connect(self.root)



        menu = QtGui.QMenu(self)

        menu.addAction(self.rootOption)

        menu.addAction(self.userOption)

        menu.exec_(QtGui.QCursor.pos())

    def root(self):
       self.pushButton_change.setText(u"老师")
       self.label.setPixmap(QtGui.QPixmap("images/teacher.png"))

    def user(self):
        self.pushButton_change.setText(u"学生")
        self.label.setPixmap(QtGui.QPixmap("images/students.png"))

    def log_in(self):
        dlg=QtGui.QMessageBox(self)
        dlg.information(self, u"提示",u"应该跳向另一个窗口，这里没有写！",QtGui.QMessageBox.Ok)

    def btnHandle(self,ID):
        #最小化
        if ID==1:
            self.hide()
            self.showMinimized()
        elif ID==2:
           #关闭
           self.trayIcon.hide()
           self.close()

    def btnEnter(self,ID):
       #鼠标进入
       if ID == 1:
           self.btn_min.setPixmap(QtGui.QPixmap("images/min.png"))
       elif ID == 2:
           self.btn_close.setPixmap(QtGui.QPixmap("images/close.png"))

    def btnLeave(self,ID):
       #鼠标离开
       '''false.png这张图片是不存在的，目的是要在鼠标
        离开后还原背景，因为默认按钮我已经PS在背景上了'''
       self.btn_min.setPixmap(QtGui.QPixmap("images/false.png"))
       self.btn_close.setPixmap(QtGui.QPixmap("images/false.png"))

    def trayClick(self,reason):
       #双击托盘
       if reason==QtGui.QSystemTrayIcon.DoubleClick:
           self.showNormal()
       else:
           pass

    def trayMenu(self):
       #右击托盘弹出的菜单
       img_main = QtGui.QIcon("images/main.png")
       img_exit = QtGui.QIcon("images/exit.png")
       self.trayIcon.setToolTip(u"学生体能健康测试软件")
       self.restoreAction = QtGui.QAction(img_main,u"打开主窗口", self)
       self.restoreAction.triggered.connect(self.showNormal)
       self.quitAction = QtGui.QAction(img_exit,u"退出", self)
       self.quitAction.triggered.connect(QtGui.qApp.quit)
       self.trayIconMenu = QtGui.QMenu(self)
       self.trayIconMenu.addAction(self.restoreAction)
       self.trayIconMenu.addSeparator()
       self.trayIconMenu.addAction(self.quitAction)
       self.trayIcon.setContextMenu(self.trayIconMenu)

    def resizeEvent(self,event):
       #重绘窗体背景
       pal=QtGui.QPalette()
       pal.setBrush(QtGui.QPalette.Window,QtGui.QBrush(image.scaled(event.size(),
           QtCore.Qt.KeepAspectRatioByExpanding,QtCore.Qt.SmoothTransformation)))
       self.setPalette(pal)

    """下面这两个才是重点，是动得关键"""
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

if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    frm = login()
    frm.show()
    sys.exit(app.exec_())
