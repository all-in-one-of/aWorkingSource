import maya.cmds as cmds
import maya.mel as mel
import json
def exportAbc(path):
    start = 1
    end = 1
    root = " -root |master"
    save_name = path
    
    command = "-frameRange " + str(start) + " " + str(end) +" -uvWrite -worldSpace -dataFormat ogawa" + root + " -file " + save_name
    cmds.AbcExport ( j = command )

file = open("/home/xukai/Documents/TestDataFactory/tea_shop/assets/map/dAbcMap.json", "rb")
dAbcMap = json.load(file)
file.close()
dUsdMap={}
for abc in dAbcMap:
    name = abc
    path = dAbcMap[name]
    out_path = "/home/xukai/Documents/TestDataFactory/tea_shop/assets/abc/"+name.split("/")[-1].replace(":","_")+".abc"
    #print name,"---",path
    #print "-",out_path
    
    dUsdMap[name]=out_path
    
    cmds.file(new=True,f=True)
    
    cmds.AbcImport(dAbcMap[abc])
    
    groupList = cmds.ls(type="mesh")

    cmds.group(groupList,n="master",world=True)
    
    exportAbc(out_path)


file = open("/home/xukai/Documents/TestDataFactory/tea_shop/assets/map/dUsdMap.json", "wb")
json.dump(dUsdMap, file, indent=3)
file.close()