# Copyright (c) 2017 Light Chaser Animation Inc. All rights reserved.

from __future__ import with_statement

import Katana, FnKatImport
FnKatImport.FromObject(Katana.Plugins.ViewerManipulatorAPI, merge = ['*'])
from LightManipulatorBase import LightManipulator

from Katana import Nodes2DAPI

class ReLightGobo(LightManipulator):

    UI_NAME = 'LCA Gobo'
    UI_SHORTCUTS = "Tab"

    vpw = ValuePolicyWrapper
    centerOfInterest = vpw('geometry.centerOfInterest',                  uiOrder = 1)
    use_slidemap     = vpw('material.arnoldLightFilterParams.slidemap',  uiOrder = 2)
    coneAngle        = vpw('material.arnoldLightParams.cone_angle',      uiOrder = 3)
    rotate           = vpw('material.arnoldLightFilterParams.rotate',    uiOrder = 4)
    offsetS          = vpw('material.arnoldLightFilterParams.offset[0]', uiOrder = 5)
    offsetT          = vpw('material.arnoldLightFilterParams.offset[1]', uiOrder = 6)
    scaleS           = vpw('material.arnoldLightFilterParams.scale_s',   uiOrder = 7)
    scaleT           = vpw('material.arnoldLightFilterParams.scale_t',   uiOrder = 8)


    def __init__(self, *args, **kwargs):
        LightManipulator.__init__(self, *args, **kwargs)

        ss = self.snapshot
        self.offsetSHandle = self.createHandle(ArrowHandle, 'offsetS')
        self.offsetSHandle.name = 'Offset S'
        self.offsetSHandle.color = (1.0, 0.0, 0.0)
        self.offsetSHandle.getAxis = lambda: self.applyRotation(Imath.V3d(1.0, 0, 0), self.rotate)
        self.offsetSHandle.getOrigin = lambda: self.applyRotation(
                                            Imath.V3d( self.offsetS / self.offsetSHandle.getValueScale(),
                                                       self.offsetT / self.offsetTHandle.getValueScale(),
                                                      -ss['centerOfInterest']),
                                            self.rotate)
        self.offsetSHandle.getValueScale = lambda: 1.0 / ss['centerOfInterest']

        self.offsetTHandle = self.createHandle(ArrowHandle, 'offsetT')
        self.offsetTHandle.name = 'Offset T'
        self.offsetTHandle.color = (0.0, 1.0, 0.0)
        self.offsetTHandle.getAxis = lambda: self.applyRotation(Imath.V3d(0, 1.0, 0), self.rotate)
        self.offsetTHandle.getOrigin = lambda: self.applyRotation(
                                            Imath.V3d( self.offsetS / self.offsetSHandle.getValueScale(),
                                                       self.offsetT / self.offsetTHandle.getValueScale(),
                                                      -ss['centerOfInterest']),
                                            self.rotate)
        self.offsetTHandle.getValueScale = lambda: 1.0 / ss['centerOfInterest']

        self.scaleSHandle = self.createHandle(ArrowHandle, 'scaleS')
        self.scaleSHandle.name = 'Scale S'
        self.scaleSHandle.type = 'cube'
        self.scaleSHandle.color = (1.0, 1.0, 0.0)
        self.scaleSHandle.getAxis = lambda: self.applyRotation(Imath.V3d(-1.0, 0, 0), self.rotate)
        self.scaleSHandle.getOrigin = lambda: self.applyRotation(
                                            Imath.V3d( self.offsetS / self.offsetSHandle.getValueScale(),
                                                       self.offsetT / self.offsetTHandle.getValueScale(),
                                                      -ss['centerOfInterest']),
                                            self.rotate)
        self.scaleSHandle.valueScale = -1.0

        self.scaleTHandle = self.createHandle(ArrowHandle, 'scaleT')
        self.scaleTHandle.name = 'Scale T'
        self.scaleTHandle.type = 'cube'
        self.scaleTHandle.color = (1.0, 1.0, 0.0)
        self.scaleTHandle.getAxis = lambda: self.applyRotation(Imath.V3d(0, -1.0, 0), self.rotate)
        self.scaleTHandle.getOrigin = lambda: self.applyRotation(
                                            Imath.V3d( self.offsetS / self.offsetSHandle.getValueScale(),
                                                       self.offsetT / self.offsetTHandle.getValueScale(),
                                                      -ss['centerOfInterest']),self.rotate)
        self.scaleTHandle.valueScale = -1.0

        self.rotateHandle = self.createHandle(RotateHandle, 'rotate')
        self.rotateHandle.name = 'Rotate'
        self.rotateHandle.color = (0.0, 0.0, 1.0)
        self.rotateHandle.axis = Imath.V3d(0.0, 0.0, -1.0)
        self.rotateHandle.radius = 0.7
        #self.rotateHandle.getOrigin = lambda: Imath.V3d(0.0, 0.0, -ss['centerOfInterest'])
        self.rotateHandle.getOrigin = lambda: self.applyRotation(
                                            Imath.V3d( self.offsetS / self.offsetSHandle.getValueScale(),
                                                       self.offsetT / self.offsetTHandle.getValueScale(),
                                                      -ss['centerOfInterest']),self.rotate)

        self.offsetHandle = self.createHandle(MultiHandle)
        self.offsetHandle.addHandle(self.offsetSHandle)
        self.offsetHandle.addHandle(self.offsetTHandle)
        self.offsetHandle.axis = Imath.V3d(0.0, 0.0, 1.0)
        self.offsetHandle.scale = 10.0, 10.0, 10.0
        self.offsetHandle.color = (1.0, 1.0, 0.0)
        self.offsetHandle.getPosition = lambda: self.applyRotation(
                                            Imath.V3d( self.offsetS / self.offsetSHandle.getValueScale(),
                                                       self.offsetT / self.offsetTHandle.getValueScale(),
                                                      -ss['centerOfInterest']),
                                            self.rotate)

        self.setPreferredHandle(self.offsetHandle)
        #self.setTransformSpace(translate = 'world', scale = 'world', rotate = 'world')


    def applyRotation(self, v, r):
        rotateDir = self.rotateHandle.getValueScale()
        return v * Imath.M33d().setRotation(math.radians(rotateDir * r))

    def calculateSnapshot(self):
        centerOfInterest = self.centerOfInterest
        outerRadius = math.tan(math.radians(self.coneAngle) / 2.0) * centerOfInterest

        return locals()

    def drawOutline(self, drawPick, picked, currentView, currentCamera, manipulatorScale):

        GL = GLStateManager.GLStateSet

        outerRadius      = self.snapshot['outerRadius']
        centerOfInterest = self.snapshot['centerOfInterest']

        objMatrix = self.getWorldMatrix()
        with GLStateManager.GLTransformState(objMatrix.sansScaling()):
            with GL(lighting  = False,
                    blend     = True,  color         = (1.0,1.0,1.0,1.0)):
                pass

# PluginRegistry = [
#     ("KatanaManipulator", 2.0, "LightGobo",  ReLightGobo)
# ]