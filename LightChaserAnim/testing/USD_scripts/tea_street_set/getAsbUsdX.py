import json,os,time
from pxr import Usd,UsdGeom,Kind,Sdf

file = open("/home/xukai/Documents/TestDataFactory/tea_street/assets/map/dUsdMapX.json", "rb")
dUsdMapX = json.load(file)
file.close()
file = open("/home/xukai/Documents/TestDataFactory/tea_street/assets/map/dTransMap.json", "rb")
dTransMap = json.load(file)
file.close()


out_path = ("/home/xukai/Documents/TestDataFactory/tea_street/tea_street_x.usda")
if os.path.isfile(out_path):
    stage = Usd.Stage.Open(out_path)
else:
    stage = Usd.Stage.CreateNew(out_path)
root = "master"
rootPrim = UsdGeom.Xform.Define(stage, '/'+root).GetPrim()
stage.SetDefaultPrim(rootPrim)
i = 0
for x in dTransMap:
    i += 1
    locPath = x
    prim = stage.DefinePrim(Sdf.Path(locPath.replace(":","_")), 'Xform')
    info_t=dTransMap[x][0]
    info_r=dTransMap[x][1]
    info_s=dTransMap[x][2]
    UsdGeom.XformCommonAPI(prim).SetTranslate(info_t)
    UsdGeom.XformCommonAPI(prim).SetRotate(info_r)
    UsdGeom.XformCommonAPI(prim).SetScale(info_s)
    if locPath in dUsdMapX and os.path.isfile(dUsdMapX[locPath]):
        refPrim = stage.OverridePrim(Sdf.Path(locPath.replace(":","_")+"/master"))
        Usd.ModelAPI(refPrim).SetKind(Kind.Tokens.component)
        print i," : ",dUsdMapX[locPath]
        refPrim.GetReferences().AppendReference(dUsdMapX[locPath])

stage.GetRootLayer().Save()
