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

class MeasurementTool(Manipulator):

    UI_NAME = 'Measurement Tool'
    UI_SHORTCUTS = "Tab"
    UI_TYPE = 'Tool'

    UI_OPTIONS = { 'Point A'      : [0.0, 0.0, 0.0],
                   'Point B'      : [0.0, 1.0, 0.0],
                   'Length'       : 1.0,
                   'Radius A'     : 0.0,
                   'Radius B'     : 0.0,
                   'Snap'         : 'Off',
                   'Snap Selected': 'Off',
                   'B Follows A'  : 'Off',
                   '__childOrder' : ('Point A',  'Point B',  'Length',
                                     'Radius A', 'Radius B', 'Snap',
                                     'Snap Selected', 'B Follows A')
                 }

    UI_OPTIONS_HINTS = {
        'Snap' : {
            'widget':'popup',
            'options':'Off|Surface|Location|Centroid'
        },
        'Snap Selected' : {
            'widget':'popup',
            'options':'Off|On'
        },
        'B Follows A' : {
            'widget':'popup',
            'options':'Off|On'
        },
    }


    def __init__(self, *args):
        Manipulator.__init__(self, *args)

        aHandles = createTranslateHandle(self, option = 'Point A')
        bHandles = createTranslateHandle(self, option = 'Point B')

        for handle in aHandles:
            handle.getOrigin = lambda: Imath.V3d(self.getOptionValue('Point A'))
        for handle in bHandles:
            handle.getOrigin = lambda: self.getPointB()

        lengthHandle = self.createHandle(AxisHandle, option = 'Length')
        lengthHandle.getPosition = lengthHandle.getOrigin = \
                        lambda: (Imath.V3d(self.getOptionValue('Point A'))
                               + self.getPointB()) * 0.5
        lengthHandle.canEdit = lambda: False

        lengthHandle.displayTogether = lambda h: h in aHandles or h in bHandles

        self.setTransformSpace(translate = 'world', scale = 'world', rotate = 'world')

        radiusAHandle           = self.createHandle(AxisHandle, option = 'Radius A')
        radiusAHandle.name      = 'Radius A'
        radiusAHandle.getAxis   = lambda: (self.getPointB() - Imath.V3d(self.getOptionValue('Point A'))).normalize()
        radiusAHandle.getOrigin = lambda: Imath.V3d(self.getOptionValue('Point A'))
        radiusAHandle.minimum   = 0.0
        radiusAHandle.isVisible = lambda: self.getOptionValue('Radius A') > 0

        radiusBHandle           = self.createHandle(AxisHandle, option = 'Radius B')
        radiusBHandle.name      = 'Radius B'
        radiusBHandle.getAxis   = lambda: (Imath.V3d(self.getOptionValue('Point A')) - self.getPointB()).normalize()
        radiusBHandle.getOrigin = lambda: self.getPointB()
        radiusBHandle.minimum   = 0.0
        radiusBHandle.isVisible = lambda: self.getOptionValue('Radius B') > 0

        self.radiusAShape = SphereShape(color = (1.0, 1.0, 1.0, 0.125))
        self.radiusAShape.getTranslate = lambda: Imath.V3d(self.getOptionValue('Point A'))
        self.radiusAShape.getRadius    = lambda: self.getOptionValue('Radius A')
        self.radiusAShape.isVisible    = lambda: self.getOptionValue('Radius A') > 0

        self.radiusBShape = SphereShape(color = (1.0, 1.0, 1.0, 0.125))
        self.radiusBShape.getTranslate = lambda: self.getPointB()
        self.radiusBShape.getRadius    = lambda: self.getOptionValue('Radius B')
        self.radiusBShape.isVisible    = lambda: self.getOptionValue('Radius B') > 0

        self.__snapVector = None

        self.loadOptions()

    def getPointB(self):
        b = Imath.V3d(self.getOptionValue('Point B'))
        if self.getOptionValue('B Follows A') == 'On':
            b += Imath.V3d(self.getOptionValue('Point A'))
        return b

    def isEnabled(self):
        return True

    def optionUpdated(self, name, value):
        if name in ('Point A', 'Point B', 'B Follows A'):
            a = Imath.V3d(self.getOptionValue('Point A'))
            b = self.getPointB()
            self.setOptionValue('Length', (a - b).length())

    def setSelection(self, selection):
        Manipulator.setSelection(self, selection)
        if self.getOptionValue('Snap Selected') == 'On':
            translation = self.getWorldMatrix().translation()
            self.setOptionValue('Point A', list(translation))

    def doDrag(self, event, pickId, currentView, currentCamera, changeCOI = False):
        Manipulator.doDrag(self, event, pickId, currentView, currentCamera)

        handle = self.getHandleByPickID(pickId)

        self.__snapVector = None

        snapMode = self.getOptionValue('Snap')

        if QtGui.QApplication.keyboardModifiers() & QtCore.Qt.ShiftModifier:
            snapMode = 'Surface'

        if snapMode in ('Surface', 'Location', 'Centroid'):
            self.__snapVector = snapToGeometry(handle, event, snapMode,
                                               currentView, currentCamera)

    def drawOutline(self, drawPick, pickId, currentView, currentCamera, manipulatorScale):

        GL = GLStateManager.GLStateSet

        with GL(cameraLight = True,  depthTest = True,
                depthMask   = False, cullFace  = False):
            self.radiusAShape.draw(currentView)
            self.radiusBShape.draw(currentView)

        with GL(color = (1.0, 0.6, 0.0, 0.5), lighting = False):

            glBegin(GL_LINES)

            glVertex3f(*self.getOptionValue('Point A'))
            glVertex3f(*self.getPointB())

            glEnd()

        if self.__snapVector:
            point, dir = self.__snapVector
            with GL(color = (1.0, 0.6, 0.0), lighting = False,
                    translate = point)(fixedScale = None):
                glBegin(GL_LINES)
                glVertex3f(0.0, 0.0, 0.0)
                glVertex3f(*dir)
                glEnd()

