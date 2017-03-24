import json,os
from pxr import Usd,UsdGeom,Kind,Sdf

aPath = ("/mnt/proj/software/develop/usd/usd_test_data/tea_street_a_asb/tea_street_a.usda")
if os.path.isfile(aPath):
    stage = Usd.Stage.Open(aPath)
else:
    stage = Usd.Stage.CreateNew(aPath)
root = "master"
rootPrim = UsdGeom.Xform.Define(stage, '/'+root).GetPrim()
stage.SetDefaultPrim(rootPrim)
file = open("/mnt/proj/software/develop/usd/usd_test_data/tea_street_a_asb/dUsdMap.json", "rb")
dUsdMap = json.load(file)
file.close()
file = open("/mnt/proj/software/develop/usd/usd_test_data/tea_street_a_asb/dTransMap.json", "rb")
dTransMap = json.load(file)
file.close()
i = 0
for x in dTransMap:
    i += 1
    locPath = x
    prim = stage.DefinePrim(Sdf.Path(locPath.replace(":","_")), 'Xform')
    info_t=dTransMap[x][0]
    info_r=dTransMap[x][0]
    info_s=dTransMap[x][0]
    UsdGeom.XformCommonAPI(prim).SetTranslate(info_t)
    UsdGeom.XformCommonAPI(prim).SetRotate(info_r)
    UsdGeom.XformCommonAPI(prim).SetScale(info_s)
    if locPath in dUsdMap and os.path.isfile(dUsdMap[locPath]):
        refPrim = stage.OverridePrim(Sdf.Path(locPath.replace(":","_")+"/master"))
        Usd.ModelAPI(refPrim).SetKind(Kind.Tokens.component)
        print i," : ",dUsdMap[locPath]
        try:
            refPrim.GetReferences().AppendReference(dUsdMap[locPath])
        except:
            print "@@@@@@@@@@\nError:%s\n@@@@@@@@@@"%dUsdMap[locPath]
stage.GetRootLayer().Save()
