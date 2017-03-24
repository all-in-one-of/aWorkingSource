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



from pxr import Usd,UsdGeom,Kind,Sdf
aPath = ("/mnt/proj/software/develop/usd/usd_test_data/tea_street_a_asb/tea_street_a.usda")
if os.path.isfile(aPath):
    stage = Usd.Stage.Open(aPath)
else:
    stage = Usd.Stage.CreateNew(aPath)
root = getRootName()
rootPrim = UsdGeom.Xform.Define(stage, '/'+root).GetPrim()
stage.SetDefaultPrim(rootPrim)
allReps = cmds.ls(type="assemblyReference",long=True)
file = open("/mnt/proj/software/develop/usd/usd_test_data/tea_street_a_asb/dUsdMap.json", "rb")
dUsdMap = json.load(file)
file.close()
for x in allReps:
    locPath = x.replace("|","/")
    repPath = cmds.getAttr("%s.definition"%x)
    abcPath = getAbc(repPath)
    prim = stage.DefinePrim(Sdf.Path(locPath.replace(":","_")), 'Xform')
    info_t=cmds.xform(x,t=True,r=True,q=True)
    info_r=cmds.xform(x,ro=True,r=True,q=True)
    info_s=cmds.xform(x,s=True,r=True,q=True)
    UsdGeom.XformCommonAPI(prim).SetTranslate(info_t)
    UsdGeom.XformCommonAPI(prim).SetRotate(info_r)
    UsdGeom.XformCommonAPI(prim).SetScale(info_s)
    if os.path.isfile(abcPath):
        refPrim = stage.OverridePrim(Sdf.Path(locPath.replace(":","_")+"/master"))
        Usd.ModelAPI(refPrim).SetKind(Kind.Tokens.component)
        print dUsdMap[locPath]
        try:
            refPrim.GetReferences().AppendReference(dUsdMap[locPath])
        except:
            print "@@@@@@@@@@\nError:%s\n@@@@@@@@@@"%dUsdMap[locPath]
stage.GetRootLayer().Save()