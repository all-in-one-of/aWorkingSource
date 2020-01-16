#coding: utf-8
__author__ = 'kai.xu:this guy is bloody cool!'
import os,sys,subprocess,socket,time
import maya.cmds as cmds
sys.path.append("//hnas01/pipeline/RndTools/PIPE_Lib/Maya2014/")
import win32api

class CommandPort(object):
    def __init__(self):
        pass
    def create(self):
        """
        create a socket server in maya
        """
        localIP = socket.gethostbyname(socket.gethostname())
        pid = os.getgid()
        try:
            cmds.commandPort(n = "%s:%s"%(localIP, pid), cl = True)
        except RuntimeError:
            pass
        cmds.commandPort(n = "%s:%s"%(localIP, pid), stp = "python", eo = True)


class ProcessCmd(object):
    def __init__(self):
        pass
    def getHostPortInfo(self):
        """
        get user`s IP and port number,
        return the ADDR.
        """
        HOST = socket.gethostbyname(socket.gethostname())
        PORT = os.getpid()
        ADDR=(HOST,PORT)
        return ADDR
    def runCmd(self,ADDR):
        """
        run the cmd.exe command,
        input ADDR.
        """
        #~HOST = '192.168.70.93' # the local host
        #~PORT = 5184 # The same port as used by the server
        #~ADDR = (HOST,PORT)
        """get script path"""
        path = os.environ['PIPELINE_PATH'] +'/Maya/Python/System/ProcessMonitor.py'
        """get mayapy.exe path"""
        mayaPath = sys.argv[0].replace('\\','/')
        parts = mayaPath.split('/')
        exe = ''
        for i in range(len(parts)):
            if i == (len(parts)-1):
                tmp = 'mayapy.exe'
            else:
                tmp = parts[i] + '/'
            exe += tmp
        """run the cmd"""
        tmpCmd = '"%s" "%s" "%s"'%(exe,path,ADDR)
        subprocess.Popen(tmpCmd,shell=True)


class ProcessKiller(object):
    def __init__(self):
        pass
    def sendCommand(self,ADDR):
        """
        setup a socket connection,
        test for the signal.
        """
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        signal = client.connect_ex(ADDR)
        #client.send("cmds.sphere()")
        print 'Connecting',
        for i in range(10):
            time.sleep(0.5)
            print '.',
        print '\t'
        client.close()
        return signal
    def killProcess(self,PID):
        """
        input the process id,
        kill the process.
        """
        PROCESS_TERMINATE = 1
        handle = win32api.OpenProcess(PROCESS_TERMINATE,False,PID)
        win32api.TerminateProcess(handle,-1)
        win32api.CloseHandle(handle)
    def runMonitor(self):
        ADDR = eval(sys.argv[1]) # active the data structure
        while True:
            signal = self.sendCommand(ADDR)
            if signal != 0:
                PID = ADDR[1]
                self.killProcess(PID)
                print "Kill it!"
            else:
                print 'Safe in this moment!'
                time.sleep(1)

if __name__=='__main__':
    PK = ProcessKiller()
    PK.runMonitor()