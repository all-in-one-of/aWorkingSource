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

class Scale(Manipulator):
    """Clone of Maya's scale manipulator"""

    UI_TYPE = 'Transform'
    UI_SHORTCUTS = "R"

    scale = ValuePolicyWrapper('xform.interactive.scale')

    def __init__(self, viewerManager):
        Manipulator.__init__(self, viewerManager)

        # X, Y and Z axis arrow handles
        for axis in ((1, 0, 0), (0, 1, 0), (0, 0, 1)):
            handle = self.createHandle(ScaleArrowHandle, 'scale', annotation = False)
            handle.axis = Imath.V3d(axis)
            handle.color = axis
            def bakeScopeHack(self, handle):
                return lambda x: self.handleDragEvent(handle, x)
            handle.dragEvent = bakeScopeHack(self, handle)

        # The central scale handle, which works in screen space
        handle = self.createHandle(ScaleHandle, 'scale', annotation = False)
        handle.color = 0.6, 0.8, 1

        self.setTransformSpace(scale = 'world')
        self.setPreferredHandle(handle)

    def handleDragEvent(self, handle, event):
        """When a handle is ctrl-clicked, switch the axis on the planeHandle."""
        handle.planeWise = (event.type() in (QtCore.QEvent.MouseMove, QtCore.QEvent.MouseButtonPress)
                            and (event.modifiers() & QtCore.Qt.ControlModifier))
