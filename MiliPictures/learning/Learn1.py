#coding:utf-8
__author__ = 'kai.xu'



from PySide import QtGui,QtCore

class FirstWindows(QtGui.QWidget):
    def __init__(self, parent=None):
        super(FirstWindows,self).__init__(parent)

        pe = QtGui.QPalette()
        pe.setColor(QtGui.QPalette.WindowText,QtCore.Qt.white)
        NameLabel = QtGui.QLabel()
        NameLabel.setText(u'用户名:')
        NameLabel.setFixedWidth(50)
        NameLabel.setFont(QtGui.QFont("Roman times",10,QtGui.QFont.Bold))
        NameLabel.setPalette(pe)
        NameLabel.setGeometry(281,0,27,23)
        self.usr = QtGui.QLineEdit()
        self.usr.setFixedWidth(150)
        PasswordLabel = QtGui.QLabel()
        PasswordLabel.setText(u'密码:')
        PasswordLabel.setFixedWidth(50)
        PasswordLabel.setFont(QtGui.QFont("Roman times",10,QtGui.QFont.Bold))
        PasswordLabel.setPalette(pe)
        self.password = QtGui.QLineEdit()
        self.password.setFixedWidth(150)
        self.password.setEchoMode(QtGui.QLineEdit.Password)

        formLayout = QtGui.QFormLayout()
        formLayout.setFormAlignment(QtCore.Qt.AlignCenter)
        formLayout.addRow(NameLabel, self.usr)
        formLayout.addRow(PasswordLabel, self.password)

        closeButton = QtGui.QPushButton(u'关闭')
        closeButton.setFixedSize(30,20)
        closeButton.move(300,0)
        closeButton.clicked.connect(self.close)
        buttonLayout = QtGui.QHBoxLayout()
        buttonLayout.addStretch(2)
        buttonLayout.addWidget(closeButton)

        self.mainlayout = QtGui.QVBoxLayout()
        self.mainlayout.setContentsMargins(0,0,0,0)
        self.mainlayout.setSpacing(0)
        self.mainlayout.addLayout(formLayout)
        #self.mainlayout.addLayout(buttonLayout)
        self.setLayout(self.mainlayout)

        """ set background picture """
        palette = QtGui.QPalette()
        #palette.setColor(self.backgroundRole(), QColor(192,253,123))# 设置背景颜色
        palette.setBrush(self.backgroundRole(), QtGui.QBrush(QtGui.QPixmap('image.jpg')))# 设置背景图片
        self.setPalette(palette)
        #~self.setAutoFillBackground(True) # 不设置也可以

        self.setWindowIcon(QtGui.QIcon('image.jpg'))
        self.setWindowFlags(QtCore.Qt.FramelessWindowHint|QtCore.Qt.WindowStaysOnTopHint)
        #self.setWindowFlags(QtCore.Qt.WindowStaysOnTopHint)
        self.setWindowTitle(u"注册")
        self.setFixedSize(300, 100)

        """
        #~ put windows to center of desktop
        desktop =QtGui.QApplication.desktop()
        width = desktop.width()
        height = desktop.height()
        self.move((width - self.width())/2, (height - self.height())/2)
        """
        self.show()

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
    # No any variate
    #############################################################################

if __name__ == '__main__':
    import sys
    app = QtGui.QApplication(sys.argv)
    window = FirstWindows()
    sys.exit(app.exec_())
