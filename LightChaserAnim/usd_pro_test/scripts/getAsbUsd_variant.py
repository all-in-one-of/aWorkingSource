import json,os,time
from pxr import Usd,UsdGeom,Kind,Sdf

file = open("/home/xukai/Documents/TestDataFactory/tea_street/dUsdMap_hdf.json", "rb")
dUsdMap = json.load(file)
file.close()
file = open("/home/xukai/Documents/TestDataFactory/tea_street/dTransMap.json", "rb")
dTransMap = json.load(file)
file.close()


aPath = ("/home/xukai/Documents/TestDataFactory/tea_street/tea_street_a_variant.usda")
if os.path.isfile(aPath):
    stage = Usd.Stage.Open(aPath)
else:
    stage = Usd.Stage.CreateNew(aPath)
root = "master"
rootPrim = UsdGeom.Xform.Define(stage, '/'+root).GetPrim()
stage.SetDefaultPrim(rootPrim)


i = 0
for x in dTransMap:
    locPath = x    
    i += 1
    prim = stage.DefinePrim(Sdf.Path(locPath.replace(":","_")), 'Xform')
    info_t=dTransMap[x][0]
    info_r=dTransMap[x][0]
    info_s=dTransMap[x][0]
    if locPath in dUsdMap and os.path.isfile(dUsdMap[locPath]):
        refPrim = stage.OverridePrim(Sdf.Path(locPath.replace(":","_")+"/master"))
        Usd.ModelAPI(refPrim).SetKind(Kind.Tokens.component)
        print i," : ",dUsdMap[locPath]
        refPrim.GetReferences().AppendReference(dUsdMap[locPath])
        UsdGeom.XformCommonAPI(refPrim).SetTranslate(info_t)
        UsdGeom.XformCommonAPI(refPrim).SetRotate(info_r)
        UsdGeom.XformCommonAPI(refPrim).SetScale(info_s)

    if i%500 == 0:
        print "Saving..."
        time.sleep(3)
        stage.GetRootLayer().Save()
        stage = Usd.Stage.Open(aPath)
