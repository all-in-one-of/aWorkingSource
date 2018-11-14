import maya.cmds as cmds
import json
def getRootName():
    dRootNameList = ["master","assets"]
    all = cmds.ls(type="transform")
    for one in all:
        parent = cmds.listRelatives(one,parent=True)
        if parent == None and one in dRootNameList:
            return one;break
def getAbc(InPath):
    result=InPath.replace("assembly_definition","scene_graph_xml")
    name = InPath.split("/")[-1]
    result=result.replace(name,"hi.abc")
    return result



allReps = cmds.ls(type="assemblyReference",long=True)
dTransMap = {}
for x in allReps:
    locPath = x.replace("|","/")
    repPath = cmds.getAttr("%s.definition"%x)
    abcPath = getAbc(repPath)
    if os.path.isfile(abcPath):
        #info_t=cmds.xform(x,t=True,r=True,q=True)
        #info_r=cmds.xform(x,ro=True,r=True,q=True)
        #info_s=cmds.xform(x,s=True,r=True,q=True)
        info_t = cmds.xform(x,worldSpace=True,translation=True,query=True)
        info_r = cmds.xform(x,worldSpace=True,rotation=True,query=True)
        info_s = cmds.xform(x,worldSpace=True,scale=True,query=True)
        dTransMap[locPath] = [info_t,info_r,info_s]
file = open("/home/xukai/Documents/TestDataFactory/tea_street/dTransMap.json", "wb")
json.dump(dTransMap, file, indent=3)
file.close()