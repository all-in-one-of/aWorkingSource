#coding: utf-8
__author__ = 'kai.xu:this guy is bloody cool!'
import psutil,sys
#ADDR = eval(sys.argv[1])
PID = 8944
process = psutil.Process(PID)
CPU = process.cpu_percent(interval=1)
print CPU,'Hello'