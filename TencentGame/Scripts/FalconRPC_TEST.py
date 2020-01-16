import sys,os
sys.path.append("E:/CodeGit/Falcon_Houdini/Houdini/Falcon_v3/lib/python")

import FalconLib.FalconUE as FalconUE
ClassModule = FalconUE.ClassModule

# print ClassModule.PyCurve
# import FalconLib.FalconUE as FalconUE
# # import FalconLib.FalconWrapper.FalconClasses.FalconHELandscapeCreator as FalconHE
# # import FalconLib.FalconWrapper.FalconClasses.FalconPyWaterSystem as FalconPyWaterSystem
# FalconPyWaterSystem = FalconPyWaterSystem.FalconPyWaterSystem
# FalconPyWaterSystemInstance = FalconUE.ClassModule.FalconPyWaterSystem

# FalconPyWaterSystemObject = FalconPyWaterSystem(FalconPyWaterSystemInstance)
# FalconPyWaterSystemObject.FalconSwampDataRefresh()

# # FalconPyWaterSystemObject.ImportTextures()
# # FalconPyWaterSystemObject.ImportFBXs()
# # FalconPyWaterSystemObject.LoadingAssetsIntoEditor()

PyCurve = ClassModule.FalconPyCurve
print dir(PyCurve)
# import FalconLib.FalconWrapper.FalconClasses.FalconPyLandscape as FalconPyLandscape
# from FalconLib.FalconUE import StructsModule, ClassModule, doInGameThread
# FalconPyLandscape = FalconPyLandscape.FalconPyLandscape
# FalconPyLandscapeInstance = ClassModule.FalconPyLandscape
# FalconPyLandscapeObject = FalconPyLandscape(FalconPyLandscapeInstance)
# FalconPyLandscapeObject.ExportExternalMask(0,0,"RiverMask", True)