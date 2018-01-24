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

NAME_TO_AXIS = {
    'Screen'  : None,
    'X'       : Imath.V3d(1, 0, 0),
    'Y'       : Imath.V3d(0, 1, 0),
    'Z'       : Imath.V3d(0, 0, 1)
}

AXIS_TO_NAME = {
    None      : 'Screen',
    (1, 0, 0) : 'X',
    (0, 1, 0) : 'Y',
    (0, 0, 1) : 'Z'
}

def _axis_to_name(axis):
    if not axis is None: axis = tuple(axis)
    return AXIS_TO_NAME.get(axis)

def _name_to_axis(name):
    return NAME_TO_AXIS.get(name, 'Screen')

class Translate(Manipulator):
    """Clone of Maya's translate manipulator"""

    UI_TYPE = 'Transform'
    UI_SHORTCUTS = "W"
    UI_OPTIONS = { 'Orientation' : 'Local',
                   'Plane Axis'  : 'Screen',
                   'Snap'        : 'Off' }
    UI_OPTIONS_HINTS = {
        'Orientation' : {
            'widget':'popup',
            'options':'World|Parent|Local'
        },
        'Plane Axis' : {
            'widget':'popup',
            'options':'Screen|X|Y|Z'
        },
        'Snap' : {
            'widget':'popup',
            'options':'Off|Surface|Location|Centroid'
        }
    }

    translate = ValuePolicyWrapper('xform.interactive.translate')

    def __init__(self, viewerManager):
        Manipulator.__init__(self, viewerManager)

        self.translateHandles = []
        self.__snapVector = None

        # X, Y and Z axis arrow handles
        for axis in ((1, 0, 0), (0, 1, 0), (0, 0, 1)):
            handle = self.createHandle(TranslateHandle, 'translate', annotation = None)
            handle.color = axis + (0.8,)
            handle.axis = Imath.V3d(axis)
            def bakeScopeHack(self, handle):
                return lambda x: self.handleDragEvent(handle, x)
            handle.dragEvent = bakeScopeHack(self, handle)
            self.translateHandles.append(handle)

        # Yellow square aligned to axis or camera
        self.planeHandle           = self.createHandle(PlaneTranslateHandle, 'translate', annotation = False)
        self.planeHandle.color     = (0.6, 0.8, 1, 0.8)
        self.planeHandle.dragEvent = lambda x: self.handleDragEvent(self.planeHandle, x)

        # Picking the plane handle
        self.setPreferredHandle(self.planeHandle)

        self.loadOptions()


    def handleDragEvent(self, handle, event):
        """When a handle is ctrl-clicked, switch the axis on the planeHandle."""

        ctldown = (event.type() in (QtCore.QEvent.MouseMove, QtCore.QEvent.MouseButtonPress)
                   and (event.modifiers() & QtCore.Qt.ControlModifier))

        if handle is self.planeHandle:
            if ctldown:
                self.setOptionValue('Plane Axis', 'Screen')
                handle.axis = None
        else:
            if ctldown:
                handle.planeWise = True
                self.setOptionValue('Plane Axis', _axis_to_name(handle.axis))
            else:
                handle.planeWise = False

    def optionUpdated(self, name, value):
        if name == 'Orientation':
            self.setTransformSpace(rotate = value.lower(), scale = 'world')
        elif name == 'Plane Axis':
            self.planeHandle.axis = _name_to_axis(value)

    def doDrag(self, event, pickId, currentView, currentCamera, changeCOI = False):
        Manipulator.doDrag(self, event, pickId, currentView, currentCamera)

        handle = self.getHandleByPickID(pickId)

        snapMode = self.getOptionValue('Snap')

        if QtGui.QApplication.keyboardModifiers() & QtCore.Qt.ShiftModifier:
            snapMode = 'Surface'

        if ((handle in self.translateHandles or handle is self.planeHandle)
            and snapMode in ('Surface', 'Location', 'Centroid')):

            self.__snapVector = self._snapToGeometry(handle, event, snapMode,
                                                     currentView,
                                                     currentCamera)

    def drawOutline(self, drawPick, pickId, currentView, currentCamera, manipulatorScale):

        GL = GLStateManager.GLStateSet

        if self.__snapVector:
            point, dir = self.__snapVector
            with GL(color = (1.0, 0.6, 0.0), lighting = False,
                    translate = point)(fixedScale = None):
                glBegin(GL_LINES)
                glVertex3f(0.0, 0.0, 0.0)
                glVertex3f(*dir)
                glEnd()

    def _snapToGeometry(self, handle, event, snapMode, currentView, currentCamera):

        return snapToGeometry(handle, event, snapMode,
                              currentView, currentCamera,
                              handle.getParentMatrix(),
                              handle.getParentInverseMatrix())



class TranslateWorld(Translate):

    UI_NAME = 'Translate (world)'
    UI_TYPE = 'Transform'
    UI_SHORTCUTS = "S"
    UI_OPTIONS = dict(Translate.UI_OPTIONS)
    UI_OPTIONS['Orientation'] = 'World'

    def __init__(self, viewerManager):
        Translate.__init__(self, viewerManager)

        self.setTransformSpace(rotate = 'world', scale = 'world')
