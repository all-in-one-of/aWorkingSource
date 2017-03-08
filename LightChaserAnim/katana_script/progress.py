from PyQt4 import QtGui,QtCore
import time,sys
class SystemBusyWidget(QtGui.QDialog):
    def __init__(self, parent=None):
        super(SystemBusyWidget, self).__init__(parent)
        self.setWindowTitle("ShotGun")
        layout = QtGui.QVBoxLayout(self)

        self.infoText = QtGui.QLabel("Connectting to Shotgun...")
        layout.addWidget(self.infoText)
        # Create a progress bar and a button and add them to the main layout
        self.progressBar = QtGui.QProgressBar(self)
        self.progressBar.setRange(0,0)
        layout.addWidget(self.progressBar)

        self.myLongTask = TaskThread()
        self.myLongTask.taskFinished.connect(self.close)
        self.myLongTask.start()


class TaskThread(QtCore.QThread):
    taskFinished = QtCore.pyqtSignal()
    def run(self):
        time.sleep(3)
        self.taskFinished.emit()

def run():
    app = QtGui.QApplication(sys.argv)
    GUI = SystemBusyWidget()
    GUI.show()
    sys.exit(app.exec_())


run()
