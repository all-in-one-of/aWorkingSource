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

from Manipulators.TranslateBase import *

class ReLightCoi(Translate):

    UI_NAME = 'LCA Center Of Interest'
    UI_TYPE = 'Light'
    UI_SHORTCUTS = "S"
    UI_OPTIONS = { 'Plane Axis'          : 'Screen',
                   'Snap'                : 'Off',
                   'Lock Meter Distance' : 'Off' }
    UI_OPTIONS_HINTS = {
        'Plane Axis' : {
            'widget':'popup',
            'options':'Screen|X|Y|Z'
        },
        'Snap' : {
            'widget':'popup',
            'options':'Off|Surface|Location|Centroid'
        },
        'Lock Meter Distance' : {
            'widget':'popup',
            'options':'Off|On',
        },
    }

    vpw = ValuePolicyWrapper

    # These are the values we are editing
    centerOfInterest = vpw('geometry.centerOfInterest')
    rotate = vpw(('xform.interactive.rotateX[0]','xform.interactive.rotateY[0]','xform.interactive.rotateZ[0]'))

    meterDistance  = vpw('material.parameters.Meter_Distance', optional = True)

    def __init__(self, *args):
        Translate.__init__(self, *args)
        snapshot = self.snapshot
        # Redirect the translate node to point at our special COI accessors
        for handle in self.translateHandles + [self.planeHandle]:
            handle.setValue = self.setCOI
            handle.getValue = self.getCOI
            handle.getOrigin = lambda: Imath.V3d(self.getCOI())

        coiHandle           = self.createHandle(AxisHandle, 'centerOfInterest')
        coiHandle.name      = 'Center of Interest'
        coiHandle.getAxis   = lambda: self.getCOI().normalized()
        coiHandle.minimum   = 0.001
        self.setPreferredHandle(coiHandle)

        self.setTransformSpace(rotate = 'parent', scale = 'parent')

    def setCOI(self, coiVec):

        offset = Imath.V3d(coiVec)
        up     = Imath.V3d(0, 1, 0)
        dir    = offset.normalized()
        right  = dir.cross(up).normalized()
        up     = right.cross(dir)
        mat    = Imath.M44d(
                  (right.x, right.y, right.z, 0.0,
                   up.x,    up.y,    up.z,    0.0,
                  -dir.x,  -dir.y,  -dir.z,   0.0,
                   0.0,     0.0,     0.0,     1.0))

        rotate = Imath.V3d()
        mat.extractEulerXYZ(rotate)
        self.rotate = tuple(math.degrees(x) for x in rotate)

        if (self.meterDistance != None and
            self.getOptionValue('Lock Meter Distance') == 'On'):
            self.meterDistance *= offset.length() / self.centerOfInterest

        self.centerOfInterest = offset.length()

    def getCOI(self):
        rotationMatrix = Imath.M44d().rotate(Imath.V3d(math.radians(r) for r in self.rotate))
        result = Imath.V3d(0, 0, -self.centerOfInterest)
        rotationMatrix.multDirMatrix(result, result)
        return result
    def calculateSnapshot(self):
        centerOfInterest = self.centerOfInterest
        return locals()
    def drawOutline(self, drawPick, picked, currentView, currentCamera, manipulatorScale):
        centerOfInterest = self.snapshot['centerOfInterest']
        self.glVisState = GL(cameraLight= True,depthTest= True,depthMask= False,cullFace= False,transform= self.getWorldMatrix().sansScaling())
        self.glLineState = GL(color= (1,1,0,1), lighting= False)
        with self.glVisState:
            with self.glLineState:
                glBegin(GL_LINES)
                glVertex3f(0,0,0)
                glVertex3f(0,0,-self.centerOfInterest)
                glEnd()
    def _snapToGeometry(self, handle, event, snapMode, currentView, currentCamera):

        return snapToGeometry(handle, event, snapMode,
                              currentView, currentCamera,
                              handle.getMatrix())

PluginRegistry = [
    ("KatanaManipulator", 2.0, "LightCoi",  ReLightCoi)
]