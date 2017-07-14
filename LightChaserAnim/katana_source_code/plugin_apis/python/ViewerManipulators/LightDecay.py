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
from LightManipulator import LightManipulator

from math import sin, cos, tan, radians
from math import pi as PI

class LightDecay(LightManipulator):

    UI_NAME = 'Decay Regions'

    vpw = ValuePolicyWrapper

    useNear     = vpw('material.arnoldLightParams.decay_use_near_atten', uiOrder = 1)
    nearStart   = vpw('material.arnoldLightParams.decay_near_start',     uiOrder = 2)
    nearEnd     = vpw('material.arnoldLightParams.decay_near_end',       uiOrder = 3)
    useFar      = vpw('material.arnoldLightParams.decay_use_far_atten',  uiOrder = 4)
    farStart    = vpw('material.arnoldLightParams.decay_far_start',      uiOrder = 5)
    farEnd      = vpw('material.arnoldLightParams.decay_far_end',        uiOrder = 6)
    farExponent = vpw('material.arnoldLightParams.decay_far_exponent',   uiOrder = 7, optional=True)
    exponent    = vpw('material.arnoldLightParams.decay_exponent',       uiOrder = 8, optional=True)
    radius      = vpw('material.arnoldLightParams.decay_radius',         uiOrder = 9, optional=True)
    clamp       = vpw('material.arnoldLightParams.decay_clamp',          uiOrder = 10,optional=True)

    coneAngle   = vpw('material.arnoldLightParams.outer_cone_angle',     optional=True)

    def __init__(self, *args):
        LightManipulator.__init__(self, *args)

        nsHandle            = self.createHandle(AxisHandle, 'nearStart')
        nsHandle.getAxis    = lambda: self.getAxis(-0.667, -0.667)
        nsHandle.minimum    = 0.0
        neHandle            = self.createHandle(AxisHandle, 'nearEnd')
        neHandle.getAxis    = lambda: self.getAxis(0.0, 0.667)
        fsHandle            = self.createHandle(AxisHandle, 'farStart')
        fsHandle.getAxis    = lambda: self.getAxis(0.667, 0.667)
        feHandle            = self.createHandle(AxisHandle, 'farEnd')
        feHandle.getAxis    = lambda: self.getAxis(-0.667, 0.667)
        if self.__class__.radius:
            rHandle             = self.createHandle(AxisHandle, 'radius')
            rHandle.getAxis     = lambda: self.getAxis(0.0, 0.0)

        nsHandle.getMaximum = lambda: neHandle.getValue()
        neHandle.getMinimum = lambda: nsHandle.getValue()
        neHandle.getMaximum = lambda: fsHandle.getValue()
        fsHandle.getMinimum = lambda: neHandle.getValue()
        fsHandle.getMaximum = lambda: feHandle.getValue()
        feHandle.getMinimum = lambda: fsHandle.getValue()

        self.displaySettings = GL(depthTest = False,
                                  blend     = True,
                                  color     = (1,1,1,0.1),
                                  cameraLight = True)

        self.setTransformSpace(scale = 'world')

    def getAxis(self, lat, long):
        awa = self.coneAngle
        if not awa is None: mult = radians(awa) / 2
        else:               mult = PI
        lat  = mult * lat
        long = mult * long
        zr = sin(long)
        return Imath.V3d(sin(lat) * zr, cos(lat) * zr, -cos(long))


    def drawOutline(self, drawPick, pickId, currentView, currentCamera, manipulatorScale):

        angle = PI - radians(self.coneAngle or 360) / 2

        with GL(transform = self.getWorldMatrix().sansScaling()):
            with self.displaySettings:
                for radius in (self.nearStart, self.nearEnd,
                               self.farStart, self.farEnd,
                               self.radius):
                    if radius:
                        ViewerUtils.drawSphere(startAngle = angle, endAngle = PI, radius = radius)
                        ViewerUtils.drawSphere(startAngle = angle, endAngle = PI, radius = radius, reverseNormals = True)



