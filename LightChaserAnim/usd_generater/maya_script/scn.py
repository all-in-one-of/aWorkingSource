from pxr import Usd,UsdGeom
from maya import cmds

FILEBASE="/home/xukai/Documents/TestDataFactory/USD/assets/shot/scn_build/"
SCN="/home/xukai/Documents/TestDataFactory/USD/assets/scn/"
try:
    scnStage = Usd.Stage.CreateNew(FILEBASE+'scn.usda')
except:
    scnStage = Usd.Stage.Open(FILEBASE + 'scn.usda')
setScnPrim = UsdGeom.Xform.Define(scnStage, '/scn').GetPrim()
scnStage.SetDefaultPrim(setScnPrim)
UsdGeom.SetStageUpAxis(scnStage, UsdGeom.Tokens.y)
Usd.ModelAPI(setScnPrim).SetKind(Kind.Tokens.assembly)
scnStage.GetRootLayer().Save()

cmds.listRelatives("scn",children=True)

scnMap={"building":"scn|building|building","building1":"scn|building1|building","building2":"scn|building2|building","plant":"scn|plant|plant"}
buildingList=["building","building1","building2"]
for child in scnMap:
    prim = UsdGeom.Xform.Define(scnStage,'/scn/%s'%child).GetPrim()
    if child in buildingList:
        prim.GetReferences().AppendReference(SCN+"building/building.usd")
    else:
        prim.GetReferences().AppendReference(SCN+"plant/plant.usd")
    info=cmds.xform(scnMap[child],m=True,q=True)
    UsdGeom.XformCommonAPI(b).SetTranslate(ballXlate)
    print info

