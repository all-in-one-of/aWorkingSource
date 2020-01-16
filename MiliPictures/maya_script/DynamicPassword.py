#coding:utf-8
__author__ = 'kai.xu:This guy is bloody cool!'

import os,datetime,sys
from PySide import QtGui,QtCore,QtNetwork


class DynamicPassword(object):
    def __init__(self,username=None):
        if username == None:
            self.username = os.environ['username']
        else:
            self.username = username
        self.currentdate = str(datetime.datetime.now()).split(' ')[0]
        self.password = None
        self.generatePassword()

    def generatePassword(self):
        nameparts=self.username.split('.')
        if len(nameparts) == 1:
            firstname = nameparts[0]
            lastname = nameparts[0]
        elif len(nameparts) == 2:
            firstname = nameparts[0]
            lastname = nameparts[1]
        else:
            firstname = 'kai'
            lastname = 'xu'
        mouth = self.currentdate.split('-')[-2]
        day = self.currentdate.split('-')[-1]
        x = int(str(mouth)[-1])+int(str(day)[-1])
        tmpKey1_A = firstname[-1]
        tmpKey2_A = lastname[-1]
        alphabet = ['a','b','c','d','e','f','g','h','i','j','k','l',
                    'm','n','o','p','q','r','s','t','u','v','w','x','y','z']
        tmpKey1_B = int(str(alphabet.index(tmpKey1_A))[-1])
        tmpKey2_B = int(str(alphabet.index(tmpKey2_A))[-1])
        y = int(str(tmpKey1_B + tmpKey2_B)[-1])
        key1 = int(str(tmpKey1_B + x)[-1])
        key2 = int(str(tmpKey2_B + x)[-1])
        key34 = int(mouth) + int(day) + y
        self.password = str(key1) + str(key2) + str(key34)

    def getPassword(self):
        return self.password

    def getUsername(self):
        return self.username

class DynamicPasswordGui(QtGui.QDialog):
    def __init__(self):
        super(DynamicPasswordGui,self).__init__()

    def showGetPasswordWindows(self):
        self.setModal(True)
        self.setWindowTitle('Mili Tools')
        title = QtGui.QLabel()
        title.setText("Password for Today:")
        self.OTPvalue = QtGui.QTextEdit()
        self.OTPvalue.setFont(QtGui.QFont("Roman times",12,QtGui.QFont.Bold))
        self.OTPvalue.setReadOnly(True)
        self.OTPvalue.setFixedHeight(50)
        button = QtGui.QPushButton()
        button.setText("Generate")
        button.clicked.connect(self.generateButton)
        button.setFixedHeight(35)
        mainLayout = QtGui.QVBoxLayout()
        mainLayout.addWidget(title)
        mainLayout.addWidget(self.OTPvalue)
        #~ mainLayout.addWidget(button)
        self.setLayout(mainLayout)
        flags = QtCore.Qt.WindowFlags()
        flags |= QtCore.Qt.Tool
        flags |= QtCore.Qt.WindowStaysOnTopHint
        self.setWindowFlags(flags)
        self.setFixedSize(300,150)
        self.generateButton()
        self.show()
        self.timer= QtCore.QTimer()
        self.timer.timeout.connect(lambda *args: self.autoClose())
        self.timer.start(3000)
        self.exec_()
    def autoClose(self):
        self.timer.stop()
        self.hide()

    def showUserLoginWindows(self,message=None,title='Mili Tools',grouplabel=u'用户登录',
                             namelabel=u'用户 :',passwordlabel=u'密码 :',buttonlabel=u'进入'):
        #~ message 窗口的附加提示信息
        #~ title 窗口的标题
        #~ grouplabel 登陆组的标题
        #~ namelabel 用户栏标题
        #~ passwordlabel 密码栏标题
        #~ buttonlabel 按钮的标题
        self.setModal(True)
        self.setWindowTitle(title)
        boldFont = QtGui.QFont()
        boldFont.setBold(True)
        messageLabel = QtGui.QLabel()
        messageLabel.setText(message)
        messageLabel.setFont(QtGui.QFont("Roman times",10))
        LogonGroupBox = QtGui.QGroupBox(grouplabel)
        NameLabel = QtGui.QLabel()
        NameLabel.setText(namelabel)
        NameLabel.setFixedWidth(50)
        NameLabel.setFont(boldFont)
        self.user = QtGui.QLineEdit()
        DP = DynamicPassword()
        username = DP.getUsername()
        self.user.setFixedWidth(200)
        PasswordLabel = QtGui.QLabel()
        PasswordLabel.setText(passwordlabel)
        PasswordLabel.setFixedWidth(50)
        PasswordLabel.setFont(boldFont)
        self.password = QtGui.QLineEdit()
        self.password.setFixedWidth(200)
        self.password.setEchoMode(QtGui.QLineEdit.Password)
        grid = QtGui.QGridLayout()
        grid.addWidget(NameLabel,1,0)
        grid.addWidget(self.user,1,1)
        grid.addWidget(PasswordLabel,2,0)
        grid.addWidget(self.password,2,1)
        LogonGroupBox.setFixedHeight(100)
        LogonGroupBox.setLayout(grid)
        self.tipsLabel = QtGui.QLabel()
        button = QtGui.QPushButton()
        button.setText(buttonlabel)
        button.setFixedSize(100,30)
        button.clicked.connect(self.loginButton)
        buttonLayout = QtGui.QHBoxLayout()
        buttonLayout.addWidget(self.tipsLabel)
        buttonLayout.addWidget(button)
        mainLayout = QtGui.QVBoxLayout()
        mainLayout.addWidget(messageLabel)
        mainLayout.addWidget(LogonGroupBox)
        mainLayout.addLayout(buttonLayout)
        self.setLayout(mainLayout)
        flags = QtCore.Qt.WindowFlags()
        flags |= QtCore.Qt.Tool
        flags |= QtCore.Qt.WindowStaysOnTopHint
        self.setWindowFlags(flags)
        self.show()
        self.exec_()

    def generateButton(self):
        try:
            import Mili.Module.utilties.MiliUser as MiliUser
            self.department = MiliUser.MiliUser()['department']
        except ImportError:
            self.OTPvalue.setText('Sorry,this plug-in only support running in maya now!')
            self.OTPvalue.setFont(QtGui.QFont("Roman times",9,QtGui.QFont.Bold))
            self.OTPvalue.setAlignment(QtCore.Qt.AlignHCenter)
            return
        if self.department in ['PipelineTD' , 'RnD', 'PM']:
            DP = DynamicPassword()
            self.password = DP.getPassword()
            self.OTPvalue.setText(self.password)
            self.OTPvalue.setAlignment(QtCore.Qt.AlignHCenter)
        else:
            self.OTPvalue.setText('Sorry,only the PepilineTD or PM can get On-Time-Password!')
            self.OTPvalue.setFont(QtGui.QFont("Roman times",10,QtGui.QFont.Bold))
            self.OTPvalue.setAlignment(QtCore.Qt.AlignHCenter)

    def loginButton(self):
        user = self.user.text()
        password = self.password.text()
        DP = DynamicPassword(user)
        key = DP.getPassword()
        if password == key:
            self.done(1)
        else:
            pe = QtGui.QPalette()
            pe.setColor(QtGui.QPalette.WindowText,QtCore.Qt.red)
            self.tipsLabel.setPalette(pe)
            self.tipsLabel.setText("Sorry,the password is wrong!")

if __name__ == '__main__':
    selfApp = os.path.basename(os.path.basename(sys.argv[0]))
    if selfApp.lower()!='maya.exe':
        app = QtGui.QApplication(sys.argv)
        myApp = DynamicPasswordGui()
        myApp.showGetPasswordWindows()
        #myApp.showUserLoginWindows()
        sys.exit(app.exec_())
    else:
        myApp = DynamicPasswordGui()
        myApp.showGetPasswordWindows()
        #myApp.showUserLoginWindows()
