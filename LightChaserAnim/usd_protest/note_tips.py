import maya.cmds as cmds

allReps = cmds.ls(type="assemblyReference")
for rep in allReps:
    print rep
    print cmds.xform(rep,m=True,query=True) 
    print cmds.getAttr("%s.definition"%rep)
    print "--------------------\n" 




import maya.cmds as cmds

def getRootName():
    dRootNameList = ["master","assets"]
    all = cmds.ls(type="transform")
    for one in all:
        parent = cmds.listRelatives(one,parent=True)
        if parent == None and one in dRootNameList:
            print one;return one;break
def getAbc(InPath):
    result=InPath.replace("assembly_definition","scene_graph_xml")
    name = InPath.split("/")[-1]
    result=result.replace(name,"hi.abc")
    print result;return result
root = getRootName()
print root
children = cmds.listRelatives(root,children=True)
print children

////
from pxr import Usd,UsdGeom,Kind,Sdf

stage = Usd.Stage.Open("/home/xukai/Documents/TestProgramFactory/usd_protest/assets/asb/tea_shop.usda")


allReps = cmds.ls(type="assemblyReference",long=True)
for x in allReps:
    locPath = x.replace("|","/")
    repPath = cmds.getAttr("%s.definition"%x)
    abcPath = getAbc(repPath)
    prim = stage.DefinePrim(Sdf.Path(locPath), 'Xform')
    #refPrim
    #prim.GetReferences().AppendReference(abcPath)


stage.GetRootLayer().Save()



//// get dAbcList

import json


def getAbc(InPath):
    result=InPath.replace("assembly_definition","scene_graph_xml")
    name = InPath.split("/")[-1]
    result=result.replace(name,"hi.abc")
    print result;return result

dAbcList=[]
allReps = cmds.ls(type="assemblyReference",long=True)
for x in allReps:
    locPath = x.replace("|","/")
    repPath = cmds.getAttr("%s.definition"%x)
    abcPath = getAbc(repPath)
    dAbcList.append(abcPath)

file = open("/home/xukai/Documents/TestProgramFactory/usd_protest/dAbcList.json", "wb")
json.dump(dAbcList, file, indent=3)
file.close()



//// translate abc

import maya.cmds as cmds
import maya.mel as mel
import json
def exportAbc(path):
    start = 1
    end = 1
    root = "-root master"
    save_name = path
    
    command = "-frameRange " + str(start) + " " + str(end) +" -uvWrite -worldSpace " + root + " -file " + save_name
    cmds.AbcExport ( j = command )

file = open("/home/xukai/Documents/TestProgramFactory/usd_protest/dAbcMap.json", "rb")
dAbcMap = json.load(file)
file.close()
dUsdMap={}
for abc in dAbcMap:
    name = abc
    path = dAbcMap[name]
    out_path = "/home/xukai/Documents/TestProgramFactory/usd_protest/assets/abc/"+abc.split("/")[-1]+".abc"
    #print name,"---",path
    #print "-",out_path
    
    dUsdMap[name]=out_path
    
    cmds.file(new=True,f=True)
    
    cmds.AbcImport("/mnt/proj/projects/tpr/asset/prp/package_a/mod/publish/package_a.mod.model/scene_graph_xml/hi.abc")
    
    ignore=["front","persp","side","top"]
    all = cmds.ls(transforms=True)
    groupList=[]
    for x in all:
        if x in ignore:
            pass
        else:
            groupList.append(x)
    cmds.group(groupList,n="master")
    
    exportAbc(out_path)


file = open("/home/xukai/Documents/TestProgramFactory/usd_protest/dUsdMap.json", "wb")
json.dump(dUsdMap, file, indent=3)
file.close()