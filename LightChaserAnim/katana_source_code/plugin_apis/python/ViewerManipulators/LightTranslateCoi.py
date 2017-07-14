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

from Translate import *

class LightTranslateCoi(TranslateWorld):

    UI_NAME = 'Translate Around COI'
    UI_TYPE = 'Transform'

    UI_OPTIONS = dict(TranslateWorld.UI_OPTIONS)
    del UI_OPTIONS['Orientation']
    UI_OPTIONS.update({'Lock Meter Distance' : 'Off' })

    UI_OPTIONS_HINTS = dict(TranslateWorld.UI_OPTIONS_HINTS)
    del UI_OPTIONS_HINTS['Orientation']
    UI_OPTIONS_HINTS.update({
        'Lock Meter Distance' : {
            'widget' :'popup',
            'options':'Off|On',
        },
    })

    vpw = ValuePolicyWrapper

    centerOfInterest = vpw('geometry.centerOfInterest')
    rotate = vpw(('xform.interactive.rotateX[0]',
                  'xform.interactive.rotateY[0]',
                  'xform.interactive.rotateZ[0]'))
    meterDistance  = vpw('material.parameters.Meter_Distance', optional = True)

    def __init__(self, *args):
        TranslateWorld.__init__(self, *args)

        # Redirect the rotate handles to point at our special COI accessors
        for handle in self.translateHandles:
            handle.setValue = self.setTranslation

        self.planeHandle.setValue = self.setTranslation

    def setTranslation(self, translate):

        rotationMatrix = Imath.M44d().rotate(Imath.V3d([math.radians(r) for r in self.rotate]))
        oldCoi         = Imath.V3d(0, 0, -self.centerOfInterest)
        rotationMatrix.multDirMatrix(oldCoi, oldCoi)

        newCoi         = oldCoi - Imath.V3d(translate) + Imath.V3d(self.translate)
        #upVector       = Imath.V3d(rotationMatrix[4:7])
        upVector       = Imath.V3d(0.0, 1.0, 0.0)
        rotationMatrix = Imath.M44d().alignZAxisWithTargetDir(-newCoi, upVector)
        rotate         = Imath.V3d()
        rotationMatrix.extractEulerXYZ(rotate)
        rotate         = Imath.V3d([math.degrees(r) for r in rotate])

        self.translate = translate
        self.rotate    = rotate

        if (self.meterDistance != None and
            self.getOptionValue('Lock Meter Distance') == 'On'):
            self.meterDistance *= newCoi.length() / self.centerOfInterest

        self.centerOfInterest = newCoi.length()
