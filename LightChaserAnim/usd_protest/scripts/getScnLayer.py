from pxr import Usd,UsdGeom,Kind,Sdf
import os

dChrList = ["accounting","deer","pixiu","three_legs"]

sequenceFilePath = '/home/xukai/Documents/TestProgramFactory/usd_protest/assets/c20280.usda'
setsLayoutLayerFilePath = '/home/xukai/Documents/TestProgramFactory/usd_protest/assets/scn.usda'

if os.path.isfile(sequenceFilePath):
	stage = Usd.Stage.Open(sequenceFilePath)
else:
	stage = Usd.Stage.CreateNew(sequenceFilePath)
workingLayer = Sdf.Layer.FindOrOpen(setsLayoutLayerFilePath)
assert stage.HasLocalLayer(workingLayer)
stage.SetEditTarget(workingLayer)

Usd.ModelAPI(UsdGeom.Xform.Define(stage, '/assets')).SetKind(Kind.Tokens.group)
Usd.ModelAPI(UsdGeom.Xform.Define(stage, '/assets/scn')).SetKind(Kind.Tokens.group)
stage.DefinePrim('/assets/scn/tea_shop').GetReferences().SetReferences([Sdf.Reference("/home/xukai/Documents/TestProgramFactory/usd_protest/assets/asb/tea_shop.usda")])
stage.DefinePrim('/assets/scn/tea_shop')
refPrim = stage.OverridePrim(Sdf.Path('/assets/scn/tea_shop'))
Usd.ModelAPI(refPrim).SetKind(Kind.Tokens.assembly)
stage.GetEditTarget().GetLayer().Save()





