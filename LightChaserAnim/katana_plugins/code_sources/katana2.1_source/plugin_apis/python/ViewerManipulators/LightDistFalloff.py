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

import Katana, FnKatImport
FnKatImport.FromObject(Katana.Plugins.ViewerManipulatorAPI, merge = ['*'])
from LightManipulator import LightManipulator

class LightDistFalloff(LightManipulator):

    UI_NAME = 'Meter Distance'

    vpw = ValuePolicyWrapper

    meterDistance  = vpw('material.arnoldLightParams.decay_radius')
    meterDistance2 = vpw('material.arnoldLightParams.decay_radius')
    arnConeAngle   = vpw('material.arnoldLightParams.outer_cone_angle', optional=True)


    def __init__(self, *args):
        LightManipulator.__init__(self, *args)

        handle           = self.createHandle(AxisHandle, 'meterDistance')
        handle.name      = 'Decay Radius'
        handle.axis      = Imath.V3d(0, 0, -1)
        handle.minimum   = 0.001
        handle.isVisible = lambda: not self.arnConeAngle is None

        handle           = self.createHandle(CircleHandle, 'meterDistance2')
        handle.name      = 'Decay Radius'
        handle.axis      = Imath.V3d(0, 1, 0)
        handle.isVisible = lambda: self.arnConeAngle is None

        self.meterDistanceHandle = handle

        self.setTransformSpace(scale = 'world')

    def drawOutline(self, drawPick, picked, currentView, currentCamera, manipulatorScale):

        GL = GLStateManager.GLStateSet

        # Transform into the space of the light
        objMatrix = self.getWorldMatrix()
        transformState = GL(transform = objMatrix.sansScaling()).push()

        d = self.meterDistance

        lineState = GL(color = (1, 1, 0), lighting = False)

        if not self.arnConeAngle is None and self.meterDistance > 0:
            s = math.tan(math.radians(self.arnConeAngle) / 2) * self.meterDistance

            # Draw the border and lines connecting to the light
            lineState.push()
            glBegin(GL_LINES)
            for point in ((0, 0, 0), (-s, 0, -d), (0, 0, 0), (0, -s, -d),
                          (0, 0, 0), ( 0, s, -d), (0, 0, 0), (s,  0, -d)):
                glVertex3f(*point)
            glEnd()

            lineState(translate = (0, 0, -d), name = self.meterDistanceHandle.getGLName())

            gluQuadricDrawStyle(quadric(), GLU_SILHOUETTE)
            gluDisk(quadric(), 0, s, 64, 1)

            lineState.pop()

        transformState.pop()
