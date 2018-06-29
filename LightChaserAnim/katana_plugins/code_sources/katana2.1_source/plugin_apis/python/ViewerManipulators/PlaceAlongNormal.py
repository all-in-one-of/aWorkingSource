# Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.

import Katana, FnKatImport
FnKatImport.FromObject(Katana.Plugins.ViewerManipulatorAPI, merge = ['*'])

class PlaceAlongNormal(Manipulator):

    UI_TYPE = 'Light'

    vpw = ValuePolicyWrapper

    translate        = vpw('xform.interactive.translate')
    rotate           = vpw(('xform.interactive.rotateX[0]',
                            'xform.interactive.rotateY[0]',
                            'xform.interactive.rotateZ[0]'))
    centerOfInterest = vpw('geometry.centerOfInterest')

    def getHandleByPickID(self, pickId): return None

    def startDrag(self, pickId, event, currentView, currentCamera):

        if self.doDrag(event, pickId,  currentView, currentCamera):
            self._isDragging = True
            return True
        else:
            return False

    def doDrag(self, event, pickId, currentView, currentCamera, changeCOI = False):

        viewer = currentView.getViewer()
        camRay = currentCamera.getCameraRay()
        endRay = currentView.getScreenRay(event.pos().x(),
                                          currentView.height()-event.pos().y(), camRay)
        positionNormalPath = viewer.getPositionAndNormalAndPathByRayIntersection(tuple(endRay[0]), tuple(endRay[1]))

        if not positionNormalPath: return False

        position = Imath.V3d(positionNormalPath[0])
        normal   = Imath.V3d(positionNormalPath[1])
        path     = positionNormalPath[2]

        if changeCOI:
            self.centerOfInterest = (position - Imath.V3d(self.translate)).length()

        translate = position + normal * self.centerOfInterest
        self.translate = translate

        dir = (position - translate).normalized()
        up = Imath.V3d(0, 1, 0)

        rotate = Imath.V3d()
        Imath.M44d.rotationMatrixWithUpDir(Imath.V3d(0.0,0.0,-1.0), dir, up).extractEulerXYZ(rotate)
        self.rotate = [math.degrees(x) for x in rotate]

        return True

    def endDrag(self, pickId, currentView, currentCamera):
        self._isDragging = False
