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

class MagnetTools(Manipulator):

    vpw = ValuePolicyWrapper

    _2dDepth = vpw('magnetData.2dDepth')

    def __init__(self, *args):
        Manipulator.__init__(self, *args)

        handle           = self.createHandle(AxisHandle, '_2dDepth')
        handle.name      = '2D Depth'
        handle.axis      = Imath.V3d(1.0, 0.0, 0.0)


