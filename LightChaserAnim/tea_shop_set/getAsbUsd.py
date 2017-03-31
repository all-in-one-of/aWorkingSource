import json,os,time
from pxr import Usd,UsdGeom,Kind,Sdf

file = open("/home/xukai/Documents/TestDataFactory/tea_shop/assets/map/dUsdMap.json", "rb")
dUsdMap = json.load(file)
file.close()
file = open("/home/xukai/Documents/TestDataFactory/tea_shop/assets/map/dTransMap.json", "rb")
dTransMap = json.load(file)
file.close()


out_path = ("/home/xukai/Documents/TestDataFactory/tea_shop/tea_shop.usda")
if os.path.isfile(out_path):
    stage = Usd.Stage.Open(out_path)
else:
    stage = Usd.Stage.CreateNew(out_path)
root = "master"
rootPrim = UsdGeom.Xform.Define(stage, '/'+root).GetPrim()
stage.SetDefaultPrim(rootPrim)
i = 0
for node in dTransMap:
    i += 1
    locPath = node
    #prim = stage.DefinePrim(Sdf.Path(locPath.replace(":","_")), 'Xform').GetPrim()
    prim = UsdGeom.Xform.Define(stage,locPath.replace(":","_")).GetPrim()
    #stage.DefinePrim(Sdf.Path(locPath.replace(":","_")+"/master"), 'Xform')
    UsdGeom.Xform.Define(stage,locPath.replace(":","_")+"/master").GetPrim()
    Usd.ModelAPI(prim).SetKind(Kind.Tokens.assembly)
    info_t=dTransMap[node][0]
    info_r=dTransMap[node][1]
    info_s=dTransMap[node][2]
    UsdGeom.XformCommonAPI(prim).SetTranslate(info_t)
    UsdGeom.XformCommonAPI(prim).SetRotate(info_r)
    UsdGeom.XformCommonAPI(prim).SetScale(info_s)
    if locPath in dUsdMap and os.path.isfile(dUsdMap[locPath]):
        refPrim = stage.OverridePrim(Sdf.Path(locPath.replace(":","_")+"/master/poly")).GetPrim()
        Usd.ModelAPI(refPrim).SetKind(Kind.Tokens.component)
        print i," : ",dUsdMap[locPath]
        refPrim.GetReferences().AppendReference(dUsdMap[locPath])
print "Saving..."
stage.GetRootLayer().Save()
print "Mission Completed!"
