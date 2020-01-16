#coding: utf-8
__author__ = 'kai.xu:this guy is bloody cool!'
import os,sys,subprocess,socket,time
import psutil,ctypes

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
    def runPyCmd(self,ADDR):
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

    def runExeCmd(self,ADDR):
        #exe = 'd:/Projects/_mili_common_push/ProcessMonitor/ProcessMonitor.exe'
        exe = 'd:/ProcessMonitor/ProcessMonitor.exe'
        tmpCmd = '"%s" "%s"'%(exe,ADDR)
        try:
            subprocess.Popen(tmpCmd,shell=False)
        except:
            self.erroRaise(u"监控插件没有加载")

    def erroRaise(self,info=None,kill=False):
        '''
        这个函数用于收集错误信息
        kill参数为True时会在报错时终止程序
        '''
        s=sys.exc_info()
        erro = "===Erro Report:===\n" \
               "Error : '%s' \n" \
               "Happened on line %d\n" \
               "%s\n" \
               "==================\n\n" % (s[1],s[2].tb_lineno,info)
        print erro
        #~ 是否终止程序
        if kill == True:
            sys.exit()


class ProcessKiller(object):
    def __init__(self):
        pass
    def sendSignal(self,ADDR):
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
    def runSignalMonitor(self):
        ADDR = eval(sys.argv[1]) # active the data structure
        while True:
            signal = self.sendSignal(ADDR)
            if signal != 0:
                PID = ADDR[1]
                self.killProcess(PID)
                print "Kill it!"
            else:
                print 'Safe in this moment!'
                time.sleep(1)
    def getCPU(self,PID):
        process = psutil.Process(PID)
        CPU = process.cpu_percent(interval=1)
        return CPU
    def runCPUMonitor(self):
        ADDR = eval(sys.argv[1]) # active the data structure
        PID = ADDR[1]
        while True:
            CPU = self.getCPU(PID)
            if CPU == 0.0:
                flag = False
                """test in a long time range"""
                print '====Dead Maya Check Begin===='
                for i in range(60):
                    CPU_check = self.getCPU(PID)
                    if CPU_check != 0.0:
                        flag = True
                        print 'Maya is alive in %s second!!!'%i
                        print '====Check End===='
                        break
                    else:
                        flag = False
                        print 'Maya is dead in %s second......'%i
                        time.sleep(1)
                if flag == False:
                    print 'Kill maya.exe!!!'
                    print '====Check End===='
                    self.killProcess(PID)
                else:
                    pass
            else:
                print 'Monitoring maya.exe: '
                for i in range(20):
                    time.sleep(0.25)
                    print '.',
                print '\t'
    def killProcess(self,PID):
        """
        input the process id,
        kill the process.
        """
        PROCESS_TERMINATE = 1
        handle = ctypes.windll.kernel32.OpenProcess(PROCESS_TERMINATE,False,PID)
        ctypes.windll.kernel32.TerminateProcess(handle,-1)
        ctypes.windll.kernel32.CloseHandle(handle)


if __name__=='__main__':
    ProcessKiller = ProcessKiller()
    ProcessKiller.runCPUMonitor()
