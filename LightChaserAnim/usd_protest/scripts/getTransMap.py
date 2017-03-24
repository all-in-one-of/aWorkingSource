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
    info_t=cmds.xform(x,t=True,r=True,q=True)
    info_r=cmds.xform(x,ro=True,r=True,q=True)
    info_s=cmds.xform(x,s=True,r=True,q=True)
    dTransMap[locPath] = [info_t,info_r,info_s]
    #UsdGeom.XformCommonAPI(prim).SetTranslate(info_t)
    #UsdGeom.XformCommonAPI(prim).SetRotate(info_r)
    #UsdGeom.XformCommonAPI(prim).SetScale(info_s)
    #if os.path.isfile(abcPath):
    #    refPrim = stage.OverridePrim(Sdf.Path(locPath.replace(":","_")+"/master"))
    #    Usd.ModelAPI(refPrim).SetKind(Kind.Tokens.component)
    #    print dUsdMap[locPath]
    #    try:
    #        refPrim.GetReferences().AppendReference(dUsdMap[locPath])
    #    except:
    #        print "@@@@@@@@@@\nError:%s\n@@@@@@@@@@"%dUsdMap[locPath]
file = open("/mnt/proj/software/develop/usd/usd_test_data/tea_street_a_asb/dTransMap.json", "wb")
json.dump(dTransMap, file, indent=3)
file.close()