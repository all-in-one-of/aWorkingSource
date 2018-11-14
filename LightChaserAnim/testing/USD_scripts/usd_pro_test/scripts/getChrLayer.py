from pxr import Usd,UsdGeom,Kind,Sdf
import os

dChrList = ["accounting","deer","pixiu","three_legs"]

sequenceFilePath = '/home/xukai/Documents/TestProgramFactory/usd_protest/assets/c20280.usda'
setsLayoutLayerFilePath = '/home/xukai/Documents/TestProgramFactory/usd_protest/assets/chr.usda'

if os.path.isfile(sequenceFilePath):
	stage = Usd.Stage.Open(sequenceFilePath)
else:
	stage = Usd.Stage.CreateNew(sequenceFilePath)
stage.SetStartTimeCode(1001)
stage.SetEndTimeCode(1088)
workingLayer = Sdf.Layer.FindOrOpen(setsLayoutLayerFilePath)
assert stage.HasLocalLayer(workingLayer)
stage.SetEditTarget(workingLayer)

Usd.ModelAPI(UsdGeom.Xform.Define(stage, '/assets')).SetKind(Kind.Tokens.group)
Usd.ModelAPI(UsdGeom.Xform.Define(stage, '/assets/chr')).SetKind(Kind.Tokens.group)
for chra in dChrList:
	refPrim = stage.OverridePrim(Sdf.Path('/assets/chr/%s'%chra))
	Usd.ModelAPI(refPrim).SetKind(Kind.Tokens.assembly)
	refPrim.GetReferences().AppendReference("/home/xukai/Documents/TestProgramFactory/usd_protest/assets/abc/"+chra+".abc")
stage.GetEditTarget().GetLayer().Save()