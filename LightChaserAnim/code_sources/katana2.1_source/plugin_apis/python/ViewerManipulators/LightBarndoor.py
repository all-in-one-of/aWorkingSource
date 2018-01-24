# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


from __future__ import with_statement
import Katana, FnKatImport
FnKatImport.FromObject(Katana.Plugins.ViewerManipulatorAPI, merge = ['*'])
from LightManipulator import LightManipulator

class LightBarndoor(LightManipulator):

    UI_NAME = 'Barn Door'

    vpw = ValuePolicyWrapper

    leftTop           = vpw('material.arnoldLightParams.barndoor_left_top',     uiOrder = 1)
    leftBottom        = vpw('material.arnoldLightParams.barndoor_left_bottom',  uiOrder = 2)
    rightTop          = vpw('material.arnoldLightParams.barndoor_right_top',    uiOrder = 3)
    rightBottom       = vpw('material.arnoldLightParams.barndoor_right_bottom', uiOrder = 4)
    topLeft           = vpw('material.arnoldLightParams.barndoor_top_left',     uiOrder = 5)
    topRight          = vpw('material.arnoldLightParams.barndoor_top_right',    uiOrder = 6)
    bottomLeft        = vpw('material.arnoldLightParams.barndoor_bottom_left',  uiOrder = 7)
    bottomRight       = vpw('material.arnoldLightParams.barndoor_bottom_right', uiOrder = 8)

    # Read-only:
    centerOfInterest  = vpw('geometry.centerOfInterest', hidden = True)
    coneAngle         = vpw('material.arnoldLightParams.outer_cone_angle')

    def __init__(self, *args):
        LightManipulator.__init__(self, *args)

        # This is where we'll calculate all the useful variables each time the
        # manipulator is refreshed
        self.snapshot = {}

        # To make the lambda functions more efficient, define a local name
        snapshot = self.snapshot

        tlHandle = self.createHandle(LeverHandle, 'topLeft')
        trHandle = self.createHandle(LeverHandle, 'topRight')
        blHandle = self.createHandle(LeverHandle, 'bottomLeft')
        brHandle = self.createHandle(LeverHandle, 'bottomRight')
        ltHandle = self.createHandle(LeverHandle, 'leftTop')
        rtHandle = self.createHandle(LeverHandle, 'rightTop')
        lbHandle = self.createHandle(LeverHandle, 'leftBottom')
        rbHandle = self.createHandle(LeverHandle, 'rightBottom')

        # Hook up the min and max points to the corners
        tlHandle.getMinPoint = blHandle.getMinPoint = \
        ltHandle.getMinPoint = rtHandle.getMinPoint = lambda: snapshot['topLeftCorner']
        tlHandle.getMaxPoint = blHandle.getMaxPoint = \
        lbHandle.getMinPoint = rbHandle.getMinPoint = lambda: snapshot['bottomLeftCorner']
        trHandle.getMinPoint = brHandle.getMinPoint = \
        ltHandle.getMaxPoint = rtHandle.getMaxPoint = lambda: snapshot['topRightCorner']
        trHandle.getMaxPoint = brHandle.getMaxPoint = \
        lbHandle.getMaxPoint = rbHandle.getMaxPoint = lambda: snapshot['bottomRightCorner']

        # Hook up each origin to the position of the other handle on each edge
        trHandle.getOrigin = tlHandle.getPosition
        tlHandle.getOrigin = trHandle.getPosition
        blHandle.getOrigin = brHandle.getPosition
        brHandle.getOrigin = blHandle.getPosition
        ltHandle.getOrigin = lbHandle.getPosition
        lbHandle.getOrigin = ltHandle.getPosition
        rtHandle.getOrigin = rbHandle.getPosition
        rbHandle.getOrigin = rtHandle.getPosition

        # Make them all draw the first half only
        for handle in self.getHandles().values(): handle.drawMin = 0.6
        ltHandle.getDrawMax = lambda: 1.0 - snapshot['topLeftInnerUV'].y
        rtHandle.getDrawMax = lambda: 1.0 - snapshot['topRightInnerUV'].y
        tlHandle.getDrawMax = lambda: 1.0 - snapshot['topLeftInnerUV'].x
        trHandle.getDrawMax = lambda:       snapshot['topRightInnerUV'].x
        lbHandle.getDrawMax = lambda:       snapshot['bottomLeftInnerUV'].y
        rbHandle.getDrawMax = lambda:       snapshot['bottomRightInnerUV'].y
        blHandle.getDrawMax = lambda: 1.0 - snapshot['bottomLeftInnerUV'].x
        brHandle.getDrawMax = lambda:       snapshot['bottomRightInnerUV'].x
        ltHandle.getDrawMin = lambda: (ltHandle.getDrawMax() + 1.0 - lbHandle.getDrawMax()) / 2.0
        rtHandle.getDrawMin = lambda: (rtHandle.getDrawMax() + 1.0 - rbHandle.getDrawMax()) / 2.0
        tlHandle.getDrawMin = lambda: (tlHandle.getDrawMax() + 1.0 - trHandle.getDrawMax()) / 2.0
        blHandle.getDrawMin = lambda: (blHandle.getDrawMax() + 1.0 - brHandle.getDrawMax()) / 2.0
        lbHandle.getDrawMin = lambda: 1.0 - ltHandle.getDrawMin()
        rbHandle.getDrawMin = lambda: 1.0 - rtHandle.getDrawMin()
        trHandle.getDrawMin = lambda: 1.0 - tlHandle.getDrawMin()
        brHandle.getDrawMin = lambda: 1.0 - blHandle.getDrawMin()

        # Create combined handles for each inner corner
        tlltHandle = self.createHandle(MultiHandle)
        tlltHandle.addHandle(tlHandle)
        tlltHandle.addHandle(ltHandle)
        tlltHandle.getOrigin = tlltHandle.getPosition = lambda: snapshot['topLeftInnerCorner']
        trrtHandle = self.createHandle(MultiHandle)
        trrtHandle.addHandle(trHandle)
        trrtHandle.addHandle(rtHandle)
        trrtHandle.getOrigin = trrtHandle.getPosition = lambda: snapshot['topRightInnerCorner']
        bllbHandle = self.createHandle(MultiHandle)
        bllbHandle.addHandle(blHandle)
        bllbHandle.addHandle(lbHandle)
        bllbHandle.getOrigin = bllbHandle.getPosition = lambda: snapshot['bottomLeftInnerCorner']
        brrbHandle = self.createHandle(MultiHandle)
        brrbHandle.addHandle(brHandle)
        brrbHandle.addHandle(rbHandle)
        brrbHandle.getOrigin = brrbHandle.getPosition = lambda: snapshot['bottomRightInnerCorner']

        # Create four pairs of invisible side handles, and combine them to
        # create a central handle for each side
        tHandleLeft = self.createHandle(PointToPointHandle, 'topLeft')
        tHandleLeft.getOrigin = lambda: snapshot['topLeftCorner']
        tHandleLeft.getTarget = lambda: snapshot['bottomLeftCorner']
        tHandleLeft.isVisible = lambda: False
        tHandleRight = self.createHandle(PointToPointHandle, 'topRight')
        tHandleRight.getOrigin = lambda: snapshot['topRightCorner']
        tHandleRight.getTarget = lambda: snapshot['bottomRightCorner']
        tHandleRight.isVisible = lambda: False
        tHandle = self.createHandle(MultiHandle)
        tHandle.addHandle(tHandleLeft)
        tHandle.addHandle(tHandleRight)
        tHandle.getPosition = lambda: snapshot['topInner']
        tHandle.isVisible = lambda: True

        lHandleTop = self.createHandle(PointToPointHandle, 'leftTop')
        lHandleTop.getOrigin = lambda: snapshot['topLeftCorner']
        lHandleTop.getTarget = lambda: snapshot['topRightCorner']
        lHandleTop.isVisible = lambda: False
        lHandleBottom = self.createHandle(PointToPointHandle, 'leftBottom')
        lHandleBottom.getOrigin = lambda: snapshot['bottomLeftCorner']
        lHandleBottom.getTarget = lambda: snapshot['bottomRightCorner']
        lHandleBottom.isVisible = lambda: False
        lHandle = self.createHandle(MultiHandle)
        lHandle.addHandle(lHandleTop)
        lHandle.addHandle(lHandleBottom)
        lHandle.getPosition = lambda: snapshot['leftInner']
        lHandle.isVisible = lambda: True

        rHandleTop = self.createHandle(PointToPointHandle, 'rightTop')
        rHandleTop.getOrigin = lambda: snapshot['topLeftCorner']
        rHandleTop.getTarget = lambda: snapshot['topRightCorner']
        rHandleTop.isVisible = lambda: False
        rHandleBottom = self.createHandle(PointToPointHandle, 'rightBottom')
        rHandleBottom.getOrigin = lambda: snapshot['bottomLeftCorner']
        rHandleBottom.getTarget = lambda: snapshot['bottomRightCorner']
        rHandleBottom.isVisible = lambda: False
        rHandle = self.createHandle(MultiHandle)
        rHandle.addHandle(rHandleTop)
        rHandle.addHandle(rHandleBottom)
        rHandle.getPosition = lambda: snapshot['rightInner']
        rHandle.isVisible = lambda: True

        bHandleLeft = self.createHandle(PointToPointHandle, 'bottomLeft')
        bHandleLeft.getOrigin = lambda: snapshot['topLeftCorner']
        bHandleLeft.getTarget = lambda: snapshot['bottomLeftCorner']
        bHandleLeft.isVisible = lambda: False
        bHandleRight = self.createHandle(PointToPointHandle, 'bottomRight')
        bHandleRight.getOrigin = lambda: snapshot['topRightCorner']
        bHandleRight.getTarget = lambda: snapshot['bottomRightCorner']
        bHandleRight.isVisible = lambda: False

        bHandle = self.createHandle(MultiHandle)
        bHandle.addHandle(bHandleLeft)
        bHandle.addHandle(bHandleRight)
        bHandle.getPosition = lambda: snapshot['bottomInner']
        bHandle.isVisible = lambda: True

        aHandle = self.createHandle(MultiHandle)
        aHandle.addHandle(tHandleLeft)
        aHandle.addHandle(tHandleRight)
        aHandle.addHandle(lHandleTop)
        aHandle.addHandle(lHandleBottom)
        aHandle.addHandle(bHandleLeft)
        aHandle.addHandle(bHandleRight)
        aHandle.addHandle(rHandleTop)
        aHandle.addHandle(rHandleBottom)
        aHandle.getPosition = lambda: snapshot['center']
        aHandle.isVisible = lambda: True

        self.setPreferredHandle(tlltHandle)
        self.setPreferredHandle(bllbHandle)
        self.setPreferredHandle(brrbHandle)
        self.setPreferredHandle(trrtHandle)
        self.setPreferredHandle(tHandle)
        self.setPreferredHandle(lHandle)
        self.setPreferredHandle(rHandle)
        self.setPreferredHandle(bHandle)
        self.setPreferredHandle(aHandle)


        # Ignore scaling, since it's a light
        self.setTransformSpace(scale = 'world')

        GL = GLStateManager.GLStateSet

        self.__glStateYellowLine  = GL(color = (1, 1, 0), lighting = False)
        self.__glStateMaskPolygon = GL(color         = (0.5, 0.5, 0.5, 0.5),
                                       lighting      = False,
                                       polygonOffset = (1, 1),
                                       stipple       = True
                                       )

    def calculateSnapshot(self):
        """Calculate the values used by the manipulators and draw functions,
        which update each time something changes."""

        topLeft            = self.topLeft
        leftTop            = self.leftTop
        topRight           = self.topRight
        rightTop           = self.rightTop
        bottomLeft         = self.bottomLeft
        leftBottom         = self.leftBottom
        bottomRight        = self.bottomRight
        rightBottom        = self.rightBottom

        topLeftInnerUV     = ViewerUtils.findLineIntersection2D((0.0, topLeft), (1.0, topRight),
                                                                (leftTop, 0.0), (leftBottom, 1.0))
        topRightInnerUV    = ViewerUtils.findLineIntersection2D((0.0, topLeft),  (1.0, topRight),
                                                                (rightTop, 0.0), (rightBottom, 1.0))
        bottomLeftInnerUV  = ViewerUtils.findLineIntersection2D((0.0, bottomLeft), (1.0, bottomRight),
                                                                (leftTop, 0.0), (leftBottom, 1.0))
        bottomRightInnerUV = ViewerUtils.findLineIntersection2D((0.0, bottomLeft),  (1.0, bottomRight),
                                                                (rightTop, 0.0), (rightBottom, 1.0))

        centerOfInterest   = self.centerOfInterest

        coneBaseRadius     = math.tan(math.radians(self.coneAngle) / 2.0) * centerOfInterest

        topLeftCorner      = Imath.V3d(-coneBaseRadius, coneBaseRadius, -centerOfInterest)
        topRightCorner     = Imath.V3d( coneBaseRadius, coneBaseRadius, -centerOfInterest)
        bottomLeftCorner   = Imath.V3d(-coneBaseRadius, -coneBaseRadius, -centerOfInterest)
        bottomRightCorner  = Imath.V3d( coneBaseRadius, -coneBaseRadius, -centerOfInterest)

        topLeftInnerCorner      = Imath.V3d((topLeftInnerUV[0]     - 0.5) * coneBaseRadius  * 2.0,
                                            (topLeftInnerUV[1]     - 0.5) * -coneBaseRadius * 2.0,
                                            -centerOfInterest)
        topRightInnerCorner     = Imath.V3d((topRightInnerUV[0]    - 0.5) * coneBaseRadius  * 2.0,
                                            (topRightInnerUV[1]    - 0.5) * -coneBaseRadius * 2.0,
                                            -centerOfInterest)
        bottomLeftInnerCorner   = Imath.V3d((bottomLeftInnerUV[0]  - 0.5) * coneBaseRadius  * 2.0,
                                            (bottomLeftInnerUV[1]  - 0.5) * -coneBaseRadius * 2.0,
                                            -centerOfInterest)
        bottomRightInnerCorner  = Imath.V3d((bottomRightInnerUV[0] - 0.5) * coneBaseRadius  * 2.0,
                                            (bottomRightInnerUV[1] - 0.5) * -coneBaseRadius * 2.0,
                                            -centerOfInterest)

        topInner           = (topLeftInnerCorner     + topRightInnerCorner)    * 0.5
        bottomInner        = (bottomLeftInnerCorner  + bottomRightInnerCorner) * 0.5
        leftInner          = (bottomLeftInnerCorner  + topLeftInnerCorner)     * 0.5
        rightInner         = (bottomRightInnerCorner + topRightInnerCorner)    * 0.5

        center             = (topLeftInnerCorner    + topRightInnerCorner
                            + bottomLeftInnerCorner + bottomRightInnerCorner) * 0.25

        return locals()

    def drawOutline(self, drawPick, picked, currentView, currentCamera, manipulatorScale):

        snapshot = self.snapshot
        tlc  = snapshot['topLeftCorner']
        trc  = snapshot['topRightCorner']
        blc  = snapshot['bottomLeftCorner']
        brc  = snapshot['bottomRightCorner']
        itlc = snapshot['topLeftInnerCorner']
        itrc = snapshot['topRightInnerCorner']
        iblc = snapshot['bottomLeftInnerCorner']
        ibrc = snapshot['bottomRightInnerCorner']
        org  = (0, 0, 0)

        # Transform into the space of the light
        with GLStateManager.GLTransformState(self.getWorldMatrix().sansScaling()):

            # Draw the border and lines connecting to the light
            with self.__glStateYellowLine:
                glBegin(GL_LINES)
                for point in (tlc, trc, trc, brc, brc, blc, blc, tlc,
                              org, tlc, org, trc, org, blc, org, brc):
                    glVertex3d(*point)
                glEnd()

            # Draw the shaded area denoting the barndoor coverage
            with self.__glStateMaskPolygon:
                glBegin(GL_QUAD_STRIP)
                for point in (tlc, itlc, trc, itrc, brc, ibrc, blc, iblc, tlc, itlc):
                    glVertex3d(*point)
                glEnd()



    def getWorldMatrix(self):
        """The world matrix from the manipulator's point-of-view excludes the
        part of the matrix which is concatenated after the interactive entry."""
        location = self.getFirstSelectedLocation()
        if not location:
            return Imath.M44d()
        else:
            return Imath.M44d(location.computeLocalToWorldMatrix())
