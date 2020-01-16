import urllib2
# contents = urllib2.urlopen("http://localhost:52066/export?type=swamp").read()
# print contents
# import?type=baselandscape&tiles=x0_y0
# contents_baselandscape = urllib2.urlopen("http://localhost:52066/export?type=baselandscape&tiles=x0_y0").read()
# # contents_generatedlandscape = urllib2.urlopen("http://localhost:52066/export?type=generatedlandscape&tiles=x0_y0").read()
# print contents_baselandscape
import sys
sys.path.append("D:\\CodeGit\\Falcon_Houdini\\Houdini\\Falcon_v2\\python2.7libs")
# from FalconLib.FalconUE import util_getCurrentWorldName
from FalconLib.FalconUE import *
class_module = ClassModule.AssetImportTask()