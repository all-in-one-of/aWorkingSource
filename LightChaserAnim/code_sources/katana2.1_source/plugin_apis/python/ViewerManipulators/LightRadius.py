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

class LightRadius(LightManipulator):

    UI_NAME = 'Radius'

    vpw = ValuePolicyWrapper

    radius = vpw('material.arnoldLightParams.radius')

    def __init__(self, *args):
        LightManipulator.__init__(self, *args)

        handle           = self.createHandle(AxisHandle, 'radius')
        handle.name      = 'Radius'
        handle.axis      = Imath.V3d(0.0, 0.0, -1.0)
        handle.minimum   = 0.001

    def drawOutline(self, drawPick, picked, currentView, currentCamera, manipulatorScale):

        if self.radius and self.radius > 0:
            with GL(color       = (1.0, 1.0, 1.0, 0.1),
                    transform   = self.getWorldMatrix().sansScaling(),
                    cameraLight = True,
                    depthTest   = True,
                    depthMask   = False,
                    cullFace    = False):
                ViewerUtils.drawSphere(radius = self.radius)
