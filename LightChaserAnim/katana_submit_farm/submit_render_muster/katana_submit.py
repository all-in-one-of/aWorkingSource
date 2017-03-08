# -*- coding:utf-8 -*-
__author__ = 'chengshun'
__maintainer__ = 'chengshun and kevin-tsui'

import sys
import os
import re
import shutil
import getpass
import ConfigParser
import random
import datetime as dt
#import production.errorhandling

from PyQt4 import QtGui, QtCore
from PyQt4.QtGui import *
from PyQt4.QtCore import *

import Katana
from Katana import KatanaFile
from Katana import NodegraphAPI
from Katana import FarmAPI

import muster_submit
reload(muster_submit)
from muster_submit import MusterSubmit

import submit_ui
reload(submit_ui)
from submit_ui import Ui_MainWindow

import submit_exit
reload(submit_exit)
from submit_exit import Ui_ExitMainWindow

from render.auto_priority.shotgun_functions import get_task_due_date, get_task_status, check_lightrig_shot
from render.auto_priority.auto_priority import calculate_priority
from lgt.submit_render_muster.katana_functions import convert_frames_string_to_frame_list

sys.path.append('/mnt/utility/toolset/lib/production')
from shotgun_connection import Connection

# from wangmo
from production.pipeline.asset_node_base import *
from production.pipeline.lcProdProj import *
from render.scan_samples.scan_samples import checkSamples

# from jerry
import common.katanaUtils as cku
import production.rv_submit as prs
import production.python_job as ppj

# get user from db
from render.muster_functions.muster_user_data_utils import getUserFullFromDB

# get shot seq-batch
from render.auto_priority.shotgun_functions import get_shot_sequence_and_priority

import MySQLdb

import random

import submit_check as sck 

#runpassedlsfnode
lctools_env=os.getenv('LCTOOLSET','/mnt/utility/toolset')
sys.path.append(lctools_env+'/applications/katana_v2/Scripts')
sys.path.append(lctools_env+'/applications/katana/Scripts')
import subprocess
import common.katana_config as ckc
def checkFileNaming():
    # check file naming correctness
    file_path = FarmAPI.GetKatanaFileName()
    file_base, file_name = os.path.split(file_path)

    # LGT katana file
    lgt_naming_pattern = '([a-z][0-9]*)(\.[a-z]*)(\.[a-z]*)(\.v[0-9]*)\.katana'
    result = re.match(lgt_naming_pattern, file_name)
    if result == None:
        return -1
    elif file_name.split('.')[2] != 'lighting':
        return -1
    else:
        return 0


class ExitWindow(QtGui.QMainWindow):

    def __init__(self):
        QtGui.QMainWindow.__init__(self)
        self.ui = Ui_ExitMainWindow()
        self.ui.setupExitUi(self)
        self.move(QDesktopWidget().availableGeometry().center() -
                  self.frameGeometry().center())


class MyMessageBox(QMessageBox):

    def __init__(self):
        QMessageBox.__init__(self)
        self.setSizeGripEnabled(True)

    def event(self, e):
        result = QMessageBox.event(self, e)
        self.setMinimumHeight(0)
        self.setMaximumHeight(16777215)
        self.setMinimumWidth(0)
        self.setMaximumWidth(16777215)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        textEdit = self.findChild(QTextEdit)
        if textEdit != None:
            textEdit.setMinimumHeight(0)
            textEdit.setMaximumHeight(16777215)
            textEdit.setMinimumWidth(0)
            textEdit.setMaximumWidth(16777215)
            textEdit.setSizePolicy(QSizePolicy.Expanding,
                                   QSizePolicy.Expanding)

        return result


class MainWindow(QtGui.QDialog):
    _instances = {}

    def __init__(self):
        QtGui.QDialog.__init__(self)
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.ui.pushButton.clicked.connect(self.submit_to_farm)
        self.ui.pushButton_2.clicked.connect(self.showAllLeft)
        self.ui.pushButton_3.clicked.connect(self.hideAllLeft)
        self.ui.pushButton_5.clicked.connect(self.showAllRight)
        self.ui.pushButton_4.clicked.connect(self.hideAllRight)
        self.output_directory = ''
        self.ui.checkBox_3.stateChanged.connect(self.changeCheckState)
        self.__class__._instances[id(self)] = self
        return

    def closeEvent(self, event):
        del self.__class__._instances[id(self)]
        return QtGui.QMainWindow.closeEvent(self, event)

    def showAllLeft(self):
        for i in xrange(0, self.ui.tableWidget.rowCount()):
            self.ui.tableWidget.item(i, 0).setCheckState(2)
        return

    def hideAllLeft(self):
        for i in xrange(0, self.ui.tableWidget.rowCount()):
            self.ui.tableWidget.item(i, 0).setCheckState(0)
        return

    def showAllRight(self):
        for i in xrange(0, self.ui.tableWidget_2.rowCount()):
            self.ui.tableWidget_2.item(i, 0).setCheckState(2)
        return

    def hideAllRight(self):
        for i in xrange(0, self.ui.tableWidget_2.rowCount()):
            self.ui.tableWidget_2.item(i, 0).setCheckState(0)
        return

    def changeCheckState(self, state):
        if state == QtCore.Qt.Checked:
            self.ui.lineEdit.setEnabled(0)
            self.ui.lineEdit_2.setEnabled(0)
            self.ui.lineEdit_3.setEnabled(0)
            for i in xrange(0, self.ui.tableWidget.rowCount()):
                self.ui.tableWidget.item(i, 1).setCheckState(0)
            for i in xrange(0, self.ui.tableWidget_2.rowCount()):
                self.ui.tableWidget.item(i, 1).setCheckState(0)
        else:
            self.ui.lineEdit.setEnabled(1)
            self.ui.lineEdit_2.setEnabled(1)
            self.ui.lineEdit_3.setEnabled(1)

    def setup_ui(self):
        l_nodes_raw = NodegraphAPI.GetAllNodesByType('Render')
        l_nodes = []
        for n in l_nodes_raw:
            np = n.getInputPortByIndex(0)
            if np.getNumConnectedPorts():
                l_nodes.append(n)
        l_renders = FarmAPI.GetSortedDependencyList(l_nodes)
        l_aovs = []
        for render_node in l_renders:
            if len(render_node['outputs']) > 0:
                for aov in render_node['outputs']:
                    l_aovs.append([render_node['name'], aov[
                                  'name'], aov['outputLocation']])
        # setting output directory to display in label
        if len(l_renders) > 0:
            if len(l_renders[0]['outputs']) > 1:
                output_location_of_some_layer = l_renders[
                    0]['outputs'][1]['outputLocation']
                self.output_directory, layer = os.path.split(
                    output_location_of_some_layer)
                self.ui.label_5.setText(self.output_directory)
        else:
            self.ui.label_5.setText("Output path not found. Contact TD.")
        checkboxList = []
        nodes_L, nodes_R = [], []
        for item in l_aovs:
            title = item[0]
            checkboxList.append(title)
        checkboxList = sorted(list(set(checkboxList)))
        for i in range(len(checkboxList)):
            name = checkboxList[i]
            if name[len(name) - 1] == "L":
                nodes_L.append(name)
            else:
                nodes_R.append(name)
        # Add to list showing L nodes
        self.ui.tableWidget.setRowCount(len(nodes_L))
        for i in range(len(nodes_L)):
            name = nodes_L[i]
            item = QtGui.QTableWidgetItem('%s' % name)
            check = Qt.Checked
            item.setCheckState(check)
            self.ui.tableWidget.setItem(i, 0, item)

            sf_mode = QTableWidgetItem(u"*双击编辑")
            check = Qt.Unchecked
            sf_mode.setCheckState(check)
            self.ui.tableWidget.setItem(i, 1, sf_mode)

       # Add to list showing R and other nodes
        self.ui.tableWidget_2.setRowCount(len(nodes_R))
        for i in range(len(nodes_R)):
            name = nodes_R[i]
            item = QtGui.QTableWidgetItem('%s' % name)
            check = Qt.Checked
            item.setCheckState(check)
            self.ui.tableWidget_2.setItem(i, 0, item)

            sf_mode = QTableWidgetItem(u"*双击编辑")
            check = Qt.Unchecked
            sf_mode.setCheckState(check)
            self.ui.tableWidget_2.setItem(i, 1, sf_mode)

        f_range = FarmAPI.GetSceneFrameRange()
        self.ui.lineEdit.setText(str(f_range['start']))
        self.ui.lineEdit_2.setText(str(f_range['end']))
        return

    def passedLgtShotSimplifyNode(self):
        lgtsimplifynode=NodegraphAPI.GetNode('LgtShotSimplify_')
        if lgtsimplifynode:
            lgtsimplifynode.setBypassed(True)
            KatanaFile.Save(FarmAPI.GetKatanaFileName())
            print "setBypassed ok"

    def passedlsfnodework(self,katanafile):
        katana_exc = ckc.katanaApp_xiaos
        #katanafile="'/mnt/work/home/wangbin/Wb2015/CodeWork/lgtsmp/test.katana'"
        #runscript="/mnt/work/home/wangbin/Wb2015/CodeWork/lgtsmp/runpassedlsfnode.py"
        runscript=lctools_env+"/tools/set/runlgt/runpassedlsfnode.py"
        cmd="%s --script=%s %s"%(katana_exc,runscript,katanafile)
        pp = subprocess.Popen(cmd, shell=True, stdout = subprocess.PIPE)
        line="start passedlsfnode"
        while line != "":
            line = pp.stdout.readline()
            print line,
        print "passedlsfnode ok"

    #@production.errorhandling.BaseHandler
    def submit_to_farm(self):
        sg = Connection('get_project_info').get_sg()
        file_path = FarmAPI.GetKatanaFileName()
        file_name = os.path.basename(file_path)
        userdata = getUserFullFromDB()
        muster_connection = MusterSubmit(userdata)
        shot_name = file_name.split('.')[0]
        listOfJobsToSubmit = []
        #~ this map would write the render node which one need to render single frame with frame number dowm!
        aSingleFrameNodeMap = {}
        # copy tmp file
        base_dir = os.path.dirname(file_path).replace('/work/', '/projcache/')
        user_tmp = os.path.join(base_dir, getpass.getuser() + '_tmp')
        try:
            #katanafile='/mnt/work/home/wangbin/Wb2015/CodeWork/lgtsmp/test.katana'
            self.passedLgtShotSimplifyNode()
        except Exception, e:
            print e
        # get quality
        if not os.path.isdir(user_tmp):
            os.system('mkdir -p ' + user_tmp)
            os.system('chmod -R 777' + user_tmp)
        file_tmp = user_tmp + "/" + "t" + dt.datetime.now().strftime("%s") + "." + \
            os.path.basename(file_path)
        shutil.copyfile(file_path, file_tmp)
        # try:
        #     #katanafile='/mnt/work/home/wangbin/Wb2015/CodeWork/lgtsmp/test.katana'
        #     self.passedlsfnodework(file_tmp)
        # except Exception, e:
        #     print e
        # get quality
        resolution_node = NodegraphAPI.GetNode('AGS_and_RS_Lgt')
        if resolution_node:
            quality = resolution_node.getParameter('user.Resolution').getValue(0).split('_')[1]
            print 'file quality: ', quality
        else:
            resolution_node = NodegraphAPI.GetNode('AGS_and_RS')
            if resolution_node:
                quality = resolution_node.getParameter('user.Resolution').getValue(0).split('_')[1]
                print 'file quality: ', quality
            else:
                msgBox = QtGui.QMessageBox()
                msgBox.setText(u"没有找到AGS_and_RS节点，请检查!")
                msgBox.exec_()
                return

        # get proj name
        lgtshotmaker_node = cku.get_lgt_shot_maker_node()
        if lgtshotmaker_node:
            proj_name = lgtshotmaker_node.getParameter('user.proj').getValue(0).upper()
            print 'project name: ', proj_name
        else:
            msgBox = QtGui.QMessageBox()
            msgBox.setText(u"没有找到lgtshotmaker节点，请检查!")
            msgBox.exec_()
            return

        # find shot sequence and batch eg. p30-2
        seq_name = get_shot_sequence_and_priority(
            shotgun_instance=sg, project_name=proj_name, shot_name=shot_name)

        # camera check
        if not cku.is_camera_latest():
            QtGui.QMessageBox.critical(
                self, 'ERROR', u'摄像机不是最新，请检查', QtGui.QMessageBox.Abort)
            return

        # frame fix
        if self.ui.checkBox_3.checkState() == 2:
            l_renders = []
            # collecting nodes to render
            for i in xrange(0, self.ui.tableWidget.rowCount()):
                if self.ui.tableWidget.item(i, 0).checkState() == 2:
                    l_renders.append(self.ui.tableWidget.item(i, 0).text())
            for i in xrange(0, self.ui.tableWidget_2.rowCount()):
                if self.ui.tableWidget_2.item(i, 0).checkState() == 2:
                    l_renders.append(self.ui.tableWidget_2.item(i, 0).text())
            l_renders = list(set(l_renders))
            if len(l_renders) > 1:
                print u'ERROR：渲染层数大于1'
                return

            render_node = str(l_renders[0])

            # get frame clips
            frame_clips_string = str(self.ui.lineEdit_4.text())
            frame_clips = convert_frames_string_to_frame_list(
                frame_clips_string)

            if frame_clips != None:

                for frame_clip in frame_clips:

                    if re.match('\d{1,4}$', frame_clip) != None:  # type a: ####, no '-'

                        job = {}
                        job['department'] = userdata['department']
                        job['name'] = file_name + '-' + render_node + "-" + \
                            frame_clip + "-" + frame_clip + ' by ' + getpass.getuser()
                        job['outputpath'] = self.output_directory
                        job['file'] = file_tmp
                        job['node'] = render_node
                        job['shot'] = file_name.split(".")[0]
                        job['project'] = proj_name
                        job['f_chunk'] = '1'
                        job['f_start'] = frame_clip
                        job['f_end'] = frame_clip
                        if render_node[-1] == "R":
                            job["isR"] = 1
                        else:
                            job["isR"] = 0
                        job['fix'] = True

                        listOfJobsToSubmit.append(job)
                print listOfJobsToSubmit
        else:

            l_renders = []
            l_renders_sf = []

            # collecting nodes to render
            for i in xrange(0, self.ui.tableWidget.rowCount()):
                if self.ui.tableWidget.item(i, 0).checkState() == 2:
                    if self.ui.tableWidget.item(i, 1).checkState() == 2:
                        l_renders_sf.append(
                            self.ui.tableWidget.item(i, 0).text())
                        aSingleFrameNodeMap[str(self.ui.tableWidget.item(i, 0).text())] = self.ui.tableWidget.item(i, 1).text()
                    else:
                        l_renders.append(self.ui.tableWidget.item(i, 0).text())

            for i in xrange(0, self.ui.tableWidget_2.rowCount()):
                if self.ui.tableWidget_2.item(i, 0).checkState() == 2:
                    if self.ui.tableWidget_2.item(i, 1).checkState() == 2:
                        l_renders_sf.append(
                            self.ui.tableWidget_2.item(i, 0).text())
                        aSingleFrameNodeMap[str(self.ui.tableWidget_2.item(i, 0).text())] = self.ui.tableWidget_2.item(i, 1).text()
                    else:
                        l_renders.append(
                            self.ui.tableWidget_2.item(i, 0).text())

            l_renders = list(set(l_renders))
            l_renders_sf = list(set(l_renders_sf))
            f_start = str(self.ui.lineEdit.text())
            f_end = str(self.ui.lineEdit_2.text())
            f_chunk = str(self.ui.lineEdit_3.text())
            if f_chunk != '1':
                frame_interval_flag = True
            else:
                frame_interval_flag = False

        
            #
            #   Main Section for submit to render
            #

            for render in l_renders:
                render = str(render)

                job = {}
                job['department'] = userdata['department']
                job['name'] = file_name + '-' + render + "-" + \
                    f_start + "-" + f_end + ' by ' + getpass.getuser()
                job['outputpath'] = self.output_directory
                job['fix'] = False
                job['file'] = file_tmp
                job['node'] = render
                job['shot'] = shot_name
                job['project'] = proj_name
                job['f_chunk'] = f_chunk
                job['f_start'] = f_start
                job['f_end'] = f_end
                job['seq_batch'] = seq_name
                job['resolution'] = quality
                if render[-1] == "R":
                    job["isR"] = 1
                else:
                    job["isR"] = 0
                listOfJobsToSubmit.append(job)

            # TODO: optimize code
            for render in l_renders_sf:
                render = str(render)
                
                #~ add single frame number into job map
                if render in aSingleFrameNodeMap:
                    try:
                        print str(aSingleFrameNodeMap[render])
                        single_frame = str(aSingleFrameNodeMap[render])
                    except ValueError:
                        print "Please enter a number instand of string!"
                        single_frame = f_start
                else:
                    single_frame = f_start

                job = {}
                job['department'] = userdata['department']
                job['name'] = file_name + '-' + render + "-" + single_frame + \
                    "-" + single_frame + ' by ' + getpass.getuser()  # single frame
                job['outputpath'] = self.output_directory
                job['file'] = file_tmp
                job['node'] = render
                job['shot'] = file_name.split(".")[0]
                job['project'] = proj_name
                job['f_chunk'] = f_chunk
                job['f_start'] = single_frame
                job['f_end'] = single_frame  # single frame
                job['fix'] = False
                job['seq_batch'] = seq_name
                job['resolution'] = quality
                if render[-1] == "R":
                    job["isR"] = 1
                else:
                    job["isR"] = 0
                listOfJobsToSubmit.append(job)


            # check if the shot is a light rig
            lgt_rig = check_lightrig_shot(sg, proj_name, shot_name) and (userdata['full_name'] in ['liyidong','wulili','xurui'])

            # farm cache random
            cf = ConfigParser.ConfigParser()
            cf.read("/mnt/public/Share/chengshun/conf/farm_cache.conf")
            percentage = cf.getint("random_value", "percentage")
            cymat_flag = os.getenv('CRYPTOMATTE')
            if random.randint(1, 10) < percentage and proj_name in ['TPR', 'CAT'] and shot_name != 'e60560' and userdata['department'] == 'LGT' and not lgt_rig and not cymat_flag:
                print 'using farm cache...'

                # prepare job setup
                p_job = {}
                p_job['file'] = file_tmp
                p_job['name'] = '[prepare]' + \
                    file_name + ' by ' + getpass.getuser()
                p_job['project'] = proj_name
                p_job['cache'] = 'farm-cache'

                err = muster_connection.send_prepare_job(p_job)
                if err == -1:
                    msg_box = QtGui.QMessageBox()
                    msg_box.setText(u"提交prepare任务失败!!! ")
                    msg_box.exec_()
                    return
                p_jobid = str(err)
                print 'prepare job send! ', p_jobid
                muster_IDs = []
                for job in listOfJobsToSubmit:

                    job['dep_id'] = p_jobid
                    err = muster_connection.sendJob_lgt(job)
                    muster_IDs.append(str(err))

                    if err == -1:
                        msg_box = QtGui.QMessageBox()
                        msg_box.setText(u"提交任务失败!!! 层级：" + str(job['node']))
                        msg_box.exec_()
                        return

            else:
                print 'not farm cache...'
                muster_IDs = []
                for job in listOfJobsToSubmit:
                    job['dep_id'] = None
                    err = muster_connection.sendJob_lgt(job)
                    muster_IDs.append(str(err))

                    if err == -1:
                        msg_box = QtGui.QMessageBox()
                        msg_box.setText(u"提交任务失败!!! 层级：" + str(job['node']))
                        msg_box.exec_()

                        return

            # remark & log
            if ((userdata['department'] == 'LGT') | (userdata['department'] == 'administrators')) and self.ui.checkBox_3.checkState() != 2:
                '''
                make symlink after lgt render
                '''
                try:
                    symlink_id=ppj.send_job('/mnt/utility/toolset/tools/lgt/katanta_render_symlink/make_render_symlink.py',
                        proj=proj_name,
                        step='LGT',
                        args=proj_name.lower()+' '+file_name.split(".")[0],
                        job_name_prefix='Make Render Symlink '+file_name,
                        user='farmer',
                        depends=','.join(muster_IDs))
                    print 'Send farm job to make symlink',symlink_id
                except:
                    raise Exception( traceback.format_exc())

                filters = [
                    ['project', 'name_is', proj_name],
                    ['entity', 'type_is', 'shot'],
                    ['entity', 'name_is', shot_name],
                    ['step', 'name_is', 'lgt'],
                    ['content', 'is', 'lighting'],
                ]

                fields = [
                    'sg_remark'  # remark
                ]

                task = sg.find_one('Task', filters, fields)
                remark = task['sg_remark']
                today = str(dt.date.today().month) + \
                    '/' + str(dt.date.today().day)
                today_isoformat = dt.date.today().isoformat()
                sub_type = quality
                for job in listOfJobsToSubmit:
                    if '_R' in job['node']:
                        sub_type = 'R'
                        break
                if remark == None:
                    remark = today + ' ' + sub_type
                else:
                    if remark.split('\n')[0].split(' ')[0] == today and remark.split('\n')[0].split(' ')[1] != 'COMP':
                        remark = today + ' ' + sub_type + \
                            remark[len(remark.split('\n')[0]):]
                    else:
                        remark = today + ' ' + sub_type + '\n' + remark
                print remark
                sg.update('Task', entity_id=task[
                          'id'], data={'sg_remark': remark})
                # write to log file
                # check if there is a file matchs matchs date
                if os.path.exists('/mnt/public/Share/chengshun/log/lgt/submit_remark/' + today_isoformat):
                    with open('/mnt/public/Share/chengshun/log/lgt/submit_remark/' + today_isoformat, 'r') as f:
                        data = f.read()
                    data = dt.datetime.now().isoformat() + ' ' + getpass.getuser() + \
                        ' ' + shot_name + ' ' + sub_type + '\n' + data
                    with open('/mnt/public/Share/chengshun/log/lgt/submit_remark/' + today_isoformat, 'w') as f:
                        f.write(data)
                else:
                    with open('/mnt/public/Share/chengshun/log/lgt/submit_remark/' + today_isoformat, 'w') as f:
                        f.write(dt.datetime.now().isoformat() + ' ' +
                                getpass.getuser() + ' ' + shot_name + ' ' + sub_type)
                    os.system(
                        'chmod -R 777 ' + '/mnt/public/Share/chengshun/log/lgt/submit_remark/' + today_isoformat)

        msgBox = QtGui.QMessageBox()
        msgBox.setText(u"提交成功. 任务id: " + str(muster_IDs))
        msgBox.exec_()
        muster_connection.disconnect()

        self.close()

        return


# import common.katanaUtils as cku
# reload(cku)
def add_new_cp_uv():
    node=cku.get_lgt_shot_maker_node()
    if cku.get_node_children_by_name(node,'CopyUv_\d*$') or \
        cku.get_node_children_by_name(node,'CopyUV_\d*$'):
        return

    old_cp=cku.get_node_children_by_name(node,'CopyUv_Ani_Srf_\d*',operation=lambda x:x.setBypassed(True))
    if old_cp:
        new_cp_op=cku.replace_me_with(old_cp[0], new_node='CopyUV_Lc', bypass_old_node=True)
        if new_cp_op:
            print '\n\nAdd new copy uv op node to fix srf v000 transfering uv bug.\n\n'
            new_cp_op.setName('CopyUV_')
            KatanaFile.Save(FarmAPI.GetKatanaFileName())

def statusErroMessageWindows():
    messageWindow = QtGui.QMessageBox()
    messageWindow.setWindowTitle("Status Error")
    messageWindow.setText(u"由于Sup暂时没有通过你的镜头，不能发布R2到农场呢!")
    messageWindow.show()
    messageWindow.exec_()

def onCheckLgtShotStatus():
    from shotgun_connection import Connection
    file_path = FarmAPI.GetKatanaFileName()
    file_name = os.path.basename(file_path)
    shot = file_name.split(".")[0]
    job_filters = [
        ['project', 'name_is', 'cat'],
        ['entity', 'type_is', 'Shot'],
        ['entity.Shot.code', 'is', str(shot)],
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



def start():
    try:
        add_new_cp_uv()
    except:
        traceback.print_exc()

    user_data = getUserFullFromDB()
    naming_status = 0
    if user_data['department'] == 'EFX':
        print 'You are using wrong tool!'
        return
    if user_data['department'] == 'LGT':
        naming_status = checkFileNaming()

    # asset check and warning before submit window
    if NodegraphAPI.GetNode('AGS_and_RS_Lgt').getParameter('user.Resolution').getValue(0).split('_')[1] == 'R1':
        sck.startCheck()

    # check status of shot from shotgun and check if the shot is got through by sup,if not,can not sent R2 to farm!
    if NodegraphAPI.GetNode('AGS_and_RS_Lgt').getParameter('user.Resolution').getValue(0).split('_')[1] == 'R2':
        if not onCheckLgtShotStatus():
            statusErroMessageWindows()
            return

    if naming_status == 0:
        window = MainWindow()
        window.setup_ui()
        window.show()
    elif naming_status == -1:
        window = ExitWindow()
        window.show()
    return window
