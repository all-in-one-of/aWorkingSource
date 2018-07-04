# Copyright (c) 2016 Light Chaser Animation. All Rights Reserved.

#coding = utf-8
__author__ = 'Kevin Tsui'

FILEBASE="/home/xukai/Documents/TestDataFactory/USD/"
import os
ASSET_BASE = os.path.join('../../', 'models')
from pxr import Usd,UsdGeom,Kind,Sdf

def createDefalut():
    try:
        chrStage = Usd.Stage.CreateNew(FILEBASE+'chr.usda')
    except:
        chrStage = Usd.Stage.Open(FILEBASE + 'chr.usda')
    setChrPrim = UsdGeom.Xform.Define(chrStage, '/chr').GetPrim()
    chrStage.SetDefaultPrim(setChrPrim)
    UsdGeom.SetStageUpAxis(chrStage, UsdGeom.Tokens.y)
    Usd.ModelAPI(setChrPrim).SetKind(Kind.Tokens.assembly)
    chrStage.GetRootLayer().Save()

    try:
        prpStage = Usd.Stage.CreateNew(FILEBASE+'prp.usda')
    except:
        prpStage = Usd.Stage.Open(FILEBASE + 'prp.usda')
    setPrpPrim = UsdGeom.Xform.Define(prpStage, '/prp').GetPrim()
    prpStage.SetDefaultPrim(setPrpPrim)
    UsdGeom.SetStageUpAxis(prpStage, UsdGeom.Tokens.y)
    Usd.ModelAPI(setPrpPrim).SetKind(Kind.Tokens.assembly)
    prpStage.GetRootLayer().Save()

    try:
        scnStage = Usd.Stage.CreateNew(FILEBASE+'scn.usda')
    except:
        scnStage = Usd.Stage.Open(FILEBASE + 'scn.usda')
    setScnPrim = UsdGeom.Xform.Define(scnStage, '/scn').GetPrim()
    scnStage.SetDefaultPrim(setScnPrim)
    UsdGeom.SetStageUpAxis(scnStage, UsdGeom.Tokens.y)
    Usd.ModelAPI(setScnPrim).SetKind(Kind.Tokens.assembly)
    scnStage.GetRootLayer().Save()


try:
    stage = Usd.Stage.CreateNew(FILEBASE+'shot.usda')
except:
    stage = Usd.Stage.Open(FILEBASE + 'shot.usda')
setShotPrim = UsdGeom.Xform.Define(stage, '/assets').GetPrim()
stage.SetDefaultPrim(setShotPrim)
UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.y)
Usd.ModelAPI(setShotPrim).SetKind(Kind.Tokens.assembly)

setChrPrim = UsdGeom.Xform.Define(stage, '/assets/chr').GetPrim()
setPrpPrim = UsdGeom.Xform.Define(stage, '/assets/prp').GetPrim()
setScnPrim = UsdGeom.Xform.Define(stage, '/assets/scn').GetPrim()

setChrPrim.GetReferences().AppendReference(FILEBASE+'chr.usda')
setPrpPrim.GetReferences().AppendReference(FILEBASE+'prp.usda')
setScnPrim.GetReferences().AppendReference(FILEBASE+'scn.usda')

stage.GetRootLayer().Save()


"""
import maya.cmds as cmds
ROOT="assets"
cmds.select(ROOT)
print "--------------------------"

print cmds.ls("plant",long=True)
#print cmds.xform(matrix=True,q=True)
print cmds.listRelatives(ROOT,ad=True,f=False)

cmds.select(None)
"""