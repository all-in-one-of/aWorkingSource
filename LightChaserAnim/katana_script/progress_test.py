#coding=utf-8
from PyQt4 import QtGui,QtCore
import time,sys,os

sys.path.append('/mnt/utility/toolset/lib/production')


class StatusChecker(QtGui.QDialog):
    def __init__(self, shot,parent=None):
        super(StatusChecker, self).__init__(parent)
        self.setWindowTitle("ShotGun")
        layout = QtGui.QVBoxLayout(self)

        self.infoText = QtGui.QLabel("Connectting to Shotgun...")
        layout.addWidget(self.infoText)
        # Create a progress bar and a button and add them to the main layout
        self.progressBar = QtGui.QProgressBar(self)
        self.progressBar.setRange(0,0)
        layout.addWidget(self.progressBar)

        self.myLongTask = TaskThread("c10170")
        self.myLongTask.taskFinished.connect(self.close)
        self.myLongTask.start()



class TaskThread(QtCore.QThread):
    taskFinished = QtCore.pyqtSignal()
    def __init__(self,InShotName):
        super(TaskThread,self).__init__()
        self.InShotName = InShotName
    def run(self):
        self.onCheckShotGunStatus()
        self.taskFinished.emit()

    def onCheckShotGunStatus(self):
        from shotgun_connection import Connection
        job_filters = [
            ['project', 'name_is', 'cat'],
            ['entity', 'type_is', 'Shot'],
            ['entity.Shot.code', 'is', str(self.InShotName)],
            ['step', 'name_is', 'lgt']
        ]

        job_fields = [
            'entity.Shot.code',
            'step',
            'sg_status_list'
        ]

        sg = Connection('get_project_info').get_sg()

        tasks = sg.find_one("Task", job_filters, job_fields)
        print tasks


def run():
    app = QtGui.QApplication(sys.argv)
    GUI = StatusChecker("c10160")
    GUI.show()
    sys.exit(app.exec_())

def onCheckLgtShotStatus(InShotName):
    from shotgun_connection import Connection
    job_filters = [
        ['project', 'name_is', 'cat'],
        ['entity', 'type_is', 'Shot'],
        ['entity.Shot.code', 'is', str(InShotName)],
        ['step', 'name_is', 'lgt']
    ]
    job_fields = [
        'entity.Shot.code',
        'step',
        'sg_status_list'
    ]
    sg = Connection('get_project_info').get_sg()
    task = sg.find_one("Task", job_filters, job_fields)
    Status = task['sg_status_list']
    if Status in ['ip','aaa','rtk']:
        return True
    else:
        return False
task=onCheckLgtShotStatus("c10160")
print task
