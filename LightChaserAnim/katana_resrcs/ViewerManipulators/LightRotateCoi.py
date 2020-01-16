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

from Manipulators.RotateBase import *

class LightRotateCoi(Rotate):

    UI_NAME = 'LCA Rotate Around COI'
    UI_TYPE = 'Transform'

    vpw = ValuePolicyWrapper

    # These are the values we are editing
    centerOfInterest = vpw('geometry.centerOfInterest')
    translate = vpw('xform.interactive.translate')

    def __init__(self, *args):
        Rotate.__init__(self, *args)

        # Redirect the rotate handles to point at our special COI accessors
        for handle in self.rotateHandles:
            handle.setValue = self.setRotation
            def getOriginFunc(self):
                def getOrigin():
                    o = Imath.V3d(0.0, 0.0, -self.centerOfInterest)
                    v = Imath.V3d()
                    self.getParentMatrix().extractScaling(v)
                    return o * v
                return getOrigin
            handle.getOrigin = getOriginFunc(self)


    def setRotation(self, rotation):

        coi            = Imath.V3d(self.translate) + self.getCOI()
        offset         = Imath.V3d(0.0, 0.0, self.centerOfInterest)
        translation    = (offset * Imath.M44d().rotate(Imath.V3d(math.radians(r) for r in rotation)))

        self.translate = coi + translation
        self.rotate    = rotation

    def getCOI(self):
        rotationMatrix = Imath.M44d().rotate(Imath.V3d(math.radians(r) for r in self.rotate))
        result = Imath.V3d(0, 0, -self.centerOfInterest)
        rotationMatrix.multDirMatrix(result, result)
        return result

# PluginRegistry = [
#     ("KatanaManipulator", 2.0, "LightRotateCoi",  LightRotateCoi)
# ]