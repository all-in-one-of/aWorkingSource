#coding: utf-8
__author__ = 'kai.xu:this guy is bloody cool!'
from PySide import QtGui,QtCore


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
        self.setFixedSize(350,250)
        self.setWindowIcon(QtGui.QIcon('C:/Users/kai.xu/Pictures/image.jpg'))
        """
        #窗口居中显示
        desktop =QtGui.QApplication.desktop()
        width = desktop.width()
        height = desktop.height()
        self.move((width - self.width())/2, (height - self.height())/2)
        self.setMouseTracking(True)
        """
        #无边框
        #self.setWindowFlags(QtCore.Qt.FramelessWindowHint)
        #显示托盘信息
        self.trayIcon = QtGui.QSystemTrayIcon(self)
        self.trayIcon.setIcon(QtGui.QIcon('C:/Users/kai.xu/Pictures/image.jpg'))
        self.trayIcon.show()
        pe = QtGui.QPalette()
        pe.setColor(QtGui.QPalette.WindowText,QtCore.Qt.white)

        label_user = QtGui.QLabel(u"账号")
        label_user.setParent(self)
        label_user.setPalette(pe)
        label_user.setFont(QtGui.QFont("Roman times",10,QtGui.QFont.Bold))
        label_user.setGeometry(QtCore.QRect(100, 135, 50, 25))
        label_passwd = QtGui.QLabel(u"密码",self)
        label_passwd.setPalette(pe)
        label_passwd.setFont(QtGui.QFont("Roman times",10,QtGui.QFont.Bold))
        label_passwd.setGeometry(QtCore.QRect(100, 170, 50, 25))

        self.colorLine = QtGui.QLabel(self)
        self.colorLine.setPixmap("C:/Users/kai.xu/Desktop/MaterialsReload6.jpg")
        self.colorLinePal = QtGui.QPalette()
        self.colorLinePal.setColor(QtGui.QPalette.Window,QtCore.Qt.red)
        self.colorLine.setPalette(self.colorLinePal)
        self.colorLine.setGeometry(QtCore.QRect(0, 60, 350, 30))

        #self.pushButton_login = QtGui.QPushButton(QtGui.QIcon("images/login.png"),u"登录",self)
        #self.pushButton_login.setGeometry(QtCore.QRect(250, 235, 75, 22))

        self.lineEdit_user = QtGui.QLineEdit(u"root",self)
        self.lineEdit_user.setGeometry(QtCore.QRect(140, 135, 150, 25))

        self.lineEdit_passwd = QtGui.QLineEdit(u'1234',self)
        self.lineEdit_passwd.setGeometry(QtCore.QRect(140, 170, 150, 25))
        self.lineEdit_passwd.setEchoMode(QtGui.QLineEdit.Password)
        self.lineEdit_passwd.setValidator(QtGui.QRegExpValidator(QtCore.QRegExp("[A-Za-z0-9]+"),self))
        #这里也可以设置QLineEdit背景为透明


        self.btn_min=labelBtn(1)               #定义最小化按钮 ID:1
        self.btn_min.setParent(self)
        self.btn_min.setGeometry(230,0,60,60)
        self.btn_min.setToolTip(u"最小化")

        self.btn_close=labelBtn(2)              #定义关闭按钮 ID:2
        self.btn_close.setPixmap("D:/work/TD/RndTools/Pipeline/Maya/Icon/Check/checkErrors.png")
        self.btn_close.setParent(self)
        self.btn_close.setGeometry(290,0,60,60)
        self.btn_close.setToolTip(u"关闭")

        #重绘窗体背景
        pal=QtGui.QPalette()
        pal.setBrush(self.backgroundRole(),QtGui.QBrush(QtGui.QPixmap('C:/Users/kai.xu/Pictures/image.jpg')))

        self.setPalette(pal)

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
           self.btn_min.setPixmap(QtGui.QPixmap('D:/work/TD/RndTools/Pipeline/Maya/Icon/Check/checkErrors.png'))
       elif ID == 2:
           self.btn_close.setPixmap(QtGui.QPixmap('D:/work/TD/RndTools/Pipeline/Maya/Icon/Check/checkErrors.png'))

    def btnLeave(self,ID):
        '''
        false.png这张图片是不存在的，目的是要在鼠标
        离开后还原背景，因为默认按钮我已经PS在背景上了
        '''
        #鼠标离开
        self.btn_min.setPixmap(QtGui.QPixmap("images/false.png"))
        self.btn_close.setPixmap(QtGui.QPixmap("images/false.png"))



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
