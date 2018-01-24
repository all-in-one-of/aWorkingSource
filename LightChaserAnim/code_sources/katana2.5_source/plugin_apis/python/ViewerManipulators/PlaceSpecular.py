# Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.

import Katana, FnKatImport
FnKatImport.FromObject(Katana.Plugins.ViewerManipulatorAPI, merge = ['*'])

class PlaceSpecular(Manipulator):
    """
    Class implementing a viewer manipulator for placing a light so that a
    specular highlight will appear approximately at the clicked position. This
    works by reflecting the incident direction in the clicked surface normal.
    """

    # Constants ---------------------------------------------------------------

    UI_TYPE = 'Light'

    # Class Variables ---------------------------------------------------------

    vpw = ValuePolicyWrapper
    translate        = vpw('xform.interactive.translate')
    rotate           = vpw(('xform.interactive.rotateX[0]',
                            'xform.interactive.rotateY[0]',
                            'xform.interactive.rotateZ[0]'))
    centerOfInterest = vpw('geometry.centerOfInterest')

    # Instance Functions ------------------------------------------------------

    def getHandleByPickID(self, pickId):
        return None

    def startDrag(self, pickId, event, currentView, currentCamera):
        """
        Starts a drag operation for the given mouse event.

        @type pickId: C{int}
        @type event: C{QtGui.QMouseEvent}
        @type currentView: C{OSGViewerTab.ViewerWidget.ViewerWidget}
        @type currentCamera: C{OSGViewerTab.Layers.CameraBases.PerspectiveCamera}
        @rtype: C{bool}
        @param pickId: The ID of this manipulator.
        @param event: The mouse event which we're handling.
        @param currentView: The viewer widget on which this manipulator is
            operating.
        @param currentCamera: The camera being looked through in the given
            viewer widget.
        """
        if self.doDrag(event, pickId, currentView, currentCamera):
            self._isDragging = True
            return True
        return False

    def doDrag(self, event, pickId, currentView, currentCamera, changeCOI=False):
        """
        Processes the drag operation, setting the C{translate}, C{rotate} and
        C{centerOfInterest} instance variables depending on what object is
        under the cursor.

        @type event: C{QtGui.QMouseEvent}
        @type pickId: C{int}
        @type currentView: C{OSGViewerTab.ViewerWidget.ViewerWidget}
        @type currentCamera: C{OSGViewerTab.Layers.CameraBases.PerspectiveCamera}
        @type changeCOI: C{bool}
        @rtype: C{bool}
        @param event: The mouse event which we're handling.
        @param pickId: The ID of this manipulator.
        @param currentView: The viewer widget on which this manipulator is
            operating.
        @param currentCamera: The camera being looked through in the given
            viewer widget.
        @param changeCOI: A flag indicating whether the current center-of-
            interest of the light should be modified by this manipulator.
        """
        # Get information about the position of the camera and the cursor
        eventPos = event.pos()
        cameraRay = currentCamera.getCameraRay()
        screenRay = currentView.getScreenRay(
            eventPos.x(), currentView.height() - eventPos.y(), cameraRay)

        # Get the position, normal and scene graph path of the location under
        # the mouse
        viewer = currentView.getViewer()
        hitPositionNormalPath = \
            viewer.getPositionAndNormalAndPathByRayIntersection(
                tuple(screenRay[0]), tuple(screenRay[1]))

        # Exit early if we didn't hit anything
        if not hitPositionNormalPath:
            return False

        # Extract the position, normal and path from the ray hit
        hitPosition = Imath.V3d(hitPositionNormalPath[0])
        hitNormal = Imath.V3d(hitPositionNormalPath[1]).normalized()
        hitLocationPath = hitPositionNormalPath[2]

        # The current position of the light and camera
        cameraPosition = Imath.V3d(cameraRay[0])

        if self.translate is None:
            # We don't have a valid current translation, so we can't preserve
            # the current distance from the hit point to the light
            distanceFromHitPointToLight = self.centerOfInterest

        else:
            lightPosition = Imath.V3d(self.translate)

            # Vector from the hit point to the light
            hitPointToLight = (hitPosition - lightPosition)
            distanceFromHitPointToLight = hitPointToLight.length()

            # Modify the centerOfInterest instance variable if we're allowed
            if changeCOI:
                self.centerOfInterest = distanceFromHitPointToLight

        # Compute the reflection direction
        incidentDirection = (hitPosition - cameraPosition).normalized()
        incidentDotNormal = incidentDirection.dot(hitNormal)
        reflectionDirection = (incidentDirection
                               - 2 * incidentDotNormal * hitNormal)

        # Calculate a new position of the light along the reflection direction
        newLightPosition = (hitPosition
                            + self.centerOfInterest * reflectionDirection)

        # The new direction for the light is the reverse of the reflection dir
        newLightDirection = -reflectionDirection

        # Calculate the new rotation for the light
        rotationMatrix = Imath.M44d.rotationMatrixWithUpDir(
            Imath.V3d(0, 0, -1), newLightDirection, Imath.V3d(0, 1, 0))
        rotation = Imath.V3d()
        rotationMatrix.extractEulerXYZ(rotation)

        # Set the instance variables as the output of this manipulator
        self.translate = [component for component in newLightPosition]
        self.rotate = [math.degrees(component) for component in rotation]
        return True

    def endDrag(self, pickId, currentView, currentCamera):
        """
        Ends the drag operation.

        @type pickId: C{int}
        @type currentView: C{OSGViewerTab.ViewerWidget.ViewerWidget}
        @type currentCamera: C{OSGViewerTab.Layers.CameraBases.PerspectiveCamera}
        @rtype: C{bool}
        @param pickId: The ID of this manipulator.
        @param currentView: The viewer widget on which this manipulator is
            operating.
        @param currentCamera: The camera being looked through in the given
            viewer widget.
        """
        self._isDragging = False
