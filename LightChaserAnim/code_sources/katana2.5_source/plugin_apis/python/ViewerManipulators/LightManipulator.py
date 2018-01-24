# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


from __future__ import with_statement
import Katana, FnKatImport
FnKatImport.FromObject(Katana.Plugins.ViewerManipulatorAPI, merge = ['*'])

class LightManipulator(Manipulator):

    UI_TYPE = 'Light'

    def __init__(self, *args):
        Manipulator.__init__(self, *args)

    def getWorldMatrix(self):
        """The world matrix from the manipulator's point-of-view excludes the
        part of the matrix which is concatenated after the interactive entry."""
        location = self.getFirstSelectedLocation()
        if not location:
            return Imath.M44d()
        else:
            return Imath.M44d(location.computeLocalToWorldMatrix())
