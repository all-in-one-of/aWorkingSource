from pxr import Usd,UsdGeom,Kind
from maya import cmds

PATH="/mnt/proj/software/develop/usd/usd_test_data/usda_test/usd/"
ASB="/mnt/proj/software/develop/usd/usd_test_data/usda_test/usd/"
ROOT="scn"
allObj=cmds.listRelatives(ROOT,children=True)
print allObj
try:
    stage = Usd.Stage.CreateNew(PATH+'flo.usda')
except:
    stage = Usd.Stage.Open(PATH + 'flo.usda')

shotPrim = UsdGeom.Xform.Define(stage, "/"+ROOT).GetPrim()
stage.SetDefaultPrim(shotPrim)
UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.y)
Usd.ModelAPI(shotPrim).SetKind(Kind.Tokens.assembly)

for obj in allObj:
    name = obj.split("_")[0]
    print name
    print ASB+"%s.usd"%name
    prim = UsdGeom.Xform.Define(stage, "/"+ROOT+"/"+obj).GetPrim()
    prim.GetReferences().AppendReference(ASB+"%s.usda"%name)

    info_t=cmds.xform(obj,t=True,q=True)
    info_r=cmds.xform(obj,ro=True,q=True)
    info_s=cmds.xform(obj,s=True,q=True)
    UsdGeom.XformCommonAPI(prim).SetTranslate(info_t)
    UsdGeom.XformCommonAPI(prim).SetRotate(info_r)
    UsdGeom.XformCommonAPI(prim).SetScale(info_s)

stage.GetRootLayer().Save()
