from PySide2.QtGui import *
from PySide2.QtCore import *
import os, sys, time

class WaitingSignal(QObject):
        sig = Signal(str)

class WaitingThread(QThread):
        def __init__(self, parent = None):
                QThread.__init__(self, parent)
                self.exiting = False
                self.signal = WaitingSignal()

        def run(self):
                end = time.time()+3
                while self.exiting==False:
                        sys.stdout.write('*')
                        sys.stdout.flush()
                        time.sleep(1)
                        now = time.time()
                        if now>=end:
                                self.exiting=True
                self.signal.sig.emit('OK')

class Wait(QWidget):
        def __init__(self, parent=None):
                QWidget.__init__(self,parent)
                self.waitingthread = WaitingThread()
                self.waitingthread.signal.sig.connect(self.operationComplete)

        def operationStart(self):
            if not self.waitingthread.isRunning():
                self.waitingthread.exiting=False
                self.waitingthread.start()

        def operationComplete(self,data):
            self.close()
            print "OK"

if __name__=='__main__':
        app = QApplication(sys.argv)
        wait = Wait()
        wait.operationStart()
        sys.exit(app.exec_())