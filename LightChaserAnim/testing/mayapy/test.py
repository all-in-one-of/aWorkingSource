from argparse import ArgumentParser

def argsChaser():
    parser = ArgumentParser(usage='Open Maya File')
    parser.add_argument("-s", "--sceneFolder",help="scenegraph xml folder to get abc files", default="")
    parser.add_argument("-u", "--usdFolder",help="put usd files in this folder",default="")
    parser.add_argument("-n", "--assetName",help="asset name of the usda file,we use the scenegraph xml name if -n is empty",default='')

    args=None
    temp_args,unknow = parser.parse_known_args()
    args=vars(temp_args)
    
    return args

import maya.cmds as cmds
import pymel.core as pm

print "\n\n\n\n------------------------------------------------------------------"

file_name="/home/xukai/Documents/TestProgramFactory/testing/mayapy/test.ma"
cmds.file(file_name,o=True,f=True)
for x in pm.ls():
    print x
