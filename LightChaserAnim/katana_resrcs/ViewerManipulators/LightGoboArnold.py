#******************************************************************************
#
#              Copyright (c) 2009 Sony Pictures Imageworks Inc.
#                             All rights reserved.
#
#   This  material  contains  the confidential and  proprietary information
#   of Sony Pictures Imageworks Inc.   and may not be disclosed,  copied or
#   duplicated  in any form,  electronic or hardcopy,  in whole  or in part,
#   without  the express prior written  consent of Sony Pictures Imageworks
#   Inc. This copyright notice does not imply publication.
#
#******************************************************************************

from __future__ import with_statement

import Katana, FnKatImport
FnKatImport.FromObject(Katana.Plugins.ViewerManipulatorAPI, merge = ['*'])
from Manipulators.LightManipulatorBase import LightManipulator

from Katana import Nodes2DAPI

class ReLightGobo(LightManipulator):

    UI_NAME = 'LCA Gobo'
    UI_SHORTCUTS = "M"
    vpw = ValuePolicyWrapper
    centerOfInterest = vpw('geometry.centerOfInterest',                     uiOrder = 1)
    use_slidemap     = None
    coneAngle        = vpw('material.arnoldLightParams.cone_angle',         uiOrder = 3)
    rotate           = 0.0
    offsetS          = 0.0
    offsetT          = 0.0
    scaleS           = vpw('material.arnoldLightFilterParams.sscale',       uiOrder = 7)
    scaleT           = vpw('material.arnoldLightFilterParams.tscale',       uiOrder = 8)
    lightScale       = vpw('material.parameters.matrix',                    uiOrder = 9)
    

    def __init__(self, *args, **kwargs):
        LightManipulator.__init__(self, *args, **kwargs)

        ss = self.snapshot

    def applyRotation(self, v, r):
        rotateDir = self.rotateHandle.getValueScale()
        return v * Imath.M33d().setRotation(math.radians(rotateDir * r))

    def calculateSnapshot(self):
        centerOfInterest = self.centerOfInterest
        #lightScale = self.lightScale
        outerRadius = math.tan(math.radians(self.coneAngle) / 2.0) * centerOfInterest

        return locals()

    def drawOutline(self, drawPick, picked, currentView, currentCamera, manipulatorScale):

        GL = GLStateManager.GLStateSet

        outerRadius      = self.snapshot['outerRadius']
        centerOfInterest = self.snapshot['centerOfInterest']

        objMatrix = self.getWorldMatrix()
        self.glVisState = GL(cameraLight= True,depthTest= True,depthMask= False,cullFace= False,transform= self.getWorldMatrix().sansScaling())
        self.glLineState = GL(color= (1,1,0,1), lighting= False)
        scale  = 1.0
        scaleS = self.scaleS*self.centerOfInterest*0.25
        scaleT = self.scaleT*self.centerOfInterest*0.25
        with self.glVisState:
            with self.glLineState:
                glBegin(GL_LINES)
                glVertex3f(0,0,0)
                glVertex3f(0,0,-self.centerOfInterest)

                # glVertex3f(0,0,0)
                # glVertex3f(1*scale,1*scale,-self.centerOfInterest)
                # glVertex3f(0,0,0)
                # glVertex3f(-1*scale,1*scale,-self.centerOfInterest)
                # glVertex3f(0,0,0)
                # glVertex3f(-1*scale,-1*scale,-self.centerOfInterest)
                # glVertex3f(0,0,0)
                # glVertex3f(1*scale,-1*scale,-self.centerOfInterest)

                glVertex3f(1*scaleS,1*scaleT,-self.centerOfInterest)
                glVertex3f(-1*scaleS,1*scaleT,-self.centerOfInterest)

                glVertex3f(1*scaleS,1*scaleT,-self.centerOfInterest)
                glVertex3f(1*scaleS,-1*scaleT,-self.centerOfInterest)

                glVertex3f(-1*scaleS,-1*scaleT,-self.centerOfInterest)
                glVertex3f(-1*scaleS,1*scaleT,-self.centerOfInterest)

                glVertex3f(-1*scaleS,-1*scaleT,-self.centerOfInterest)
                glVertex3f(1*scaleS,-1*scaleT,-self.centerOfInterest)


                glVertex3f(1*scaleS,1*scaleT,-self.centerOfInterest)
                glVertex3f(-1*scaleS,-1*scaleT,-self.centerOfInterest)

                glVertex3f(-1*scaleS,1*scaleT,-self.centerOfInterest)
                glVertex3f(1*scaleS,-1*scaleT,-self.centerOfInterest)

                glEnd()

PluginRegistry = [
    ("KatanaManipulator", 2.0, "LightGobo",  ReLightGobo)
]