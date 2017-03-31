import maya.cmds as cmds
import maya.mel as mel
import json

def exportUsd(path):
    cmds.select("master")
    command = """file -force -options ";shadingMode=None;exportReferencesAsInstanceable=0;exportUVs=1;normalizeUVs=0;exportColorSets=0;renderableOnly=0;allCameras=0;renderLayerMode=Modeling Variant Per Layer;mergeXForm=1;defaultMeshScheme=Polygonal Mesh;exportVisibility=1;animation=0;startTime=1;endTime=1;frameSample=0" -typ "pxrUsdExport" -pr -es "%s";"""%path
    mel.eval(command)

file = open("/home/xukai/Documents/TestDataFactory/tea_street/assets/map/dAbcMap.json", "rb")
dAbcMap = json.load(file)
file.close()
dUsdMapX={}
for abc in dAbcMap:
    name = abc
    path = dAbcMap[name]
    out_path = "/home/xukai/Documents/TestDataFactory/tea_street/assets/usd/"+name.split("/")[-1].replace(":","_")+".usd"
    #print name,"---",path
    #print "-",out_path
    
    dUsdMapX[name]=out_path
    
    cmds.file(new=True,f=True)
    
    cmds.AbcImport(dAbcMap[abc])
    
    groupList = cmds.ls(type="mesh")

    cmds.group(groupList,n="master",world=True)
    
    exportUsd(out_path)


file = open("/home/xukai/Documents/TestDataFactory/tea_street/assets/map/dUsdMapX.json", "wb")
json.dump(dUsdMapX, file, indent=3)
file.close()