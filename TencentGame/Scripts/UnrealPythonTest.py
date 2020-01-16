import sys
sys.path.append("D:\\CodeGit\\Falcon_Houdini\\Houdini\\Falcon_v2\\python2.7libs")

import FalconLib.FalconWrapper.FalconClasses.FalconHELandscapeCreator as FalconHE
import FalconLib.FalconUE as FalconUE
reload(FalconUE)
reload(FalconHE)

FalconHELandscapeCreator = FalconHE.FalconHELandscapeCreator
FalconHELandscapeCreatorInstance = FalconUE.ClassModule.FalconHELandscapeCreator

FalconHELandscapeCreatorObject = FalconHELandscapeCreator(FalconHELandscapeCreatorInstance)
FalconHELandscapeCreatorObject.ImportFbx("D:\\Example.fbx","/Game/Example")