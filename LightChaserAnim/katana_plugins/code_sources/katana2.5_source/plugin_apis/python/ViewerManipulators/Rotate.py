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

class Rotate(Manipulator):
    """Clone of Maya's rotate manipulator"""

    UI_TYPE = 'Transform'
    UI_SHORTCUTS = "E"

    rotate = ValuePolicyWrapper(('xform.interactive.rotateX[0]',
                                 'xform.interactive.rotateY[0]',
                                 'xform.interactive.rotateZ[0]'))

    def __init__(self, viewerManager):
        Manipulator.__init__(self, viewerManager)

        self.rotateHandles = []

        # screen, X, Y and Z axis arrow handles
        for axis in ((1, 0, 0), (0, 1, 0), (0, 0, 1), None):
            handle = self.createHandle(RotateHandle, 'rotate', annotation = False)
            if axis is None: handle.axis = None
            else:            handle.axis = Imath.V3d(axis)
            handle.color = axis or (0.6, 0.8, 1)
            if axis is None:
                handle.radius = 1.2
            self.rotateHandles.append(handle)

        arcHandle = self.createHandle(ArcballHandle, 'rotate', annotation = False)
        arcHandle.radius = 1.2
        arcHandle.drawRadius = 0.9
        self.rotateHandles.append(arcHandle)

        self.setTransformSpace(scale = 'world')


class RotateWorld(Rotate):

    UI_NAME = 'Rotate (world)'
    UI_TYPE = 'Transform'
    UI_SHORTCUTS = "D"

    def __init__(self, viewerManager):
        Rotate.__init__(self, viewerManager)

        self.setTransformSpace(rotate = 'world', scale = 'world')

