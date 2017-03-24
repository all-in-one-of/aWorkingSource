from pxr import Usd,UsdGeom,Kind,Sdf
import os
basePath = "/home/xukai/Documents/TestProgramFactory/usd_protest/assets/"
shotPath = basePath + "c20280.usda"
if os.path.isfile(shotPath):
	stage = Usd.Stage.Open(shotPath)
else:
	stage = Usd.Stage.CreateNew(shotPath)
stage.SetStartTimeCode(1001)
stage.SetEndTimeCode(1088)
rootLayer = stage.GetRootLayer()
subLayers = [basePath+"chr.usda",basePath+"prp.usda",basePath+"scn.usda"]
for subLayerPath in subLayers:
    Sdf.Layer.CreateNew(subLayerPath)
    rootLayer.subLayerPaths.append(subLayerPath)

UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.y)
stage.GetRootLayer().Save()