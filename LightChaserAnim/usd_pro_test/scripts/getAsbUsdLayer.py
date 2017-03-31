import json,os,time
from pxr import Usd,UsdGeom,Kind,Sdf

file = open("/home/xukai/Documents/TestDataFactory/tea_street/dUsdSequenceList.json", "rb")
dUsdSequenceList = json.load(file)
file.close()
aPath = ("/home/xukai/Documents/TestDataFactory/tea_street/usda/tea_street_a_layer.usda")


if os.path.isfile(aPath):
	stage = Usd.Stage.Open(aPath)
else:
	stage = Usd.Stage.CreateNew(aPath)
rootLayer = stage.GetRootLayer()
subLayers = dUsdSequenceList
for subLayerPath in subLayers:
    Sdf.Layer.CreateNew(subLayerPath)
    rootLayer.subLayerPaths.append(subLayerPath)

UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.y)
stage.GetRootLayer().Save()