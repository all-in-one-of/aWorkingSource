#!/usr/bin/python
# -*- coding: utf-8 -*-
import os,time,sys
import maya.mel as mm 
import maya.cmds as cmds
from functools import partial 
 
xx_scriptPath = 'N:/Production/Comp/Tools/maya/script'
if xx_scriptPath not in sys.path :
    sys.path.append(xx_scriptPath) 
import MiliRenderLightTool
reload(MiliRenderLightTool)

MiliRenderLightTool_Class = MiliRenderLightTool.MiliRenderLightTool_Class()
MiliRenderLightTool_Class.MiliRenderLightToolWin()