# Copyright (c) 2017 Light Chaser Animation Inc. All rights reserved.

from __future__ import with_statement

import Katana, FnKatImport
FnKatImport.FromObject(Katana.Plugins.ViewerManipulatorAPI, merge = ['*'])
from Manipulators.LightManipulatorBase import LightManipulator

class ReLightRadius(LightManipulator):

    UI_NAME = 'LCA Light Radius'
    UI_SHORTCUTS = "Tab"
    vpw = ValuePolicyWrapper

    radius = vpw('material.arnoldLightParams.radius')
    coneAngle   = vpw('material.arnoldLightParams.cone_angle',optional=True)
    def __init__(self, *args):
        LightManipulator.__init__(self, *args)

        handle           = self.createHandle(CircleHandle, 'radius')
        handle.name      = 'Radius'
        handle.axis      = Imath.V3d(0.0, 1.0, 0.0)
        handle.minimum   = 0.001

        self.setTransformSpace(scale = 'world')

    def drawOutline(self, drawPick, picked, currentView, currentCamera, manipulatorScale):

        if self.radius and self.radius > 0:
            with GL(color       = (0.5, 0.6, 0.5, 0.05),
                    transform   = self.getWorldMatrix().sansScaling(),
                    cameraLight = False,
                    depthTest   = True,
                    depthMask   = False,
                    cullFace    = False):
                ViewerUtils.drawSphere(radius = self.radius)

PluginRegistry = [
    ("KatanaManipulator", 2.0, "LightRadius",  ReLightRadius)
]