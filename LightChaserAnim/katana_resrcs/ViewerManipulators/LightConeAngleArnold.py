# Copyright (c) 2017 Light Chaser Animation Inc. All rights reserved.


from __future__ import with_statement
import Katana, FnKatImport
FnKatImport.FromObject(Katana.Plugins.ViewerManipulatorAPI, merge = ['*'])
from Manipulators.LightManipulatorBase import LightManipulator

LINE_COLOR                   = (1,    1,    0,    1)
HANDLE_COLOR                 = (0.5,  1,    1,    1)
DISABLED_HANDLE_COLOR        = (0.5,  0.5,  0.5,  1)
GEOMETRY_COLOR               = (0,    1,    0,    0.125)
BOLD_GEOMETRY_COLOR          = (0.5,  1,    0.5,  0.125)
LENS_COLOR                   = (0,    0.5,  1,    0.4)
RADIUS_COLOR                 = (1,    1,    1,    0.125)
BRIGHT_COLOR                 = (1,    1,    1,    0.125)
DISABLED_GEOMETRY_COLOR      = (0.5,  0.5,  0.5,  0.125)
BOLD_DISABLED_GEOMETRY_COLOR = (0.67, 0.67, 0.67, 0.125)

GL = GLStateManager.GLStateSet

def cPowerToAngle(cpower, intensity, wideAngle):
    if cpower == 0:
        return 0
    return math.degrees((math.radians(wideAngle) / math.pi
                         * math.acos(math.pow(intensity, 1.0 / cpower))) * 2.0)

def angleToCPower(angle, intensity, wideAngle):
    if angle > wideAngle:
        angle = wideAngle
    if angle <= 0:
        angle = 0.00001
    return (math.log(intensity)
            / math.log(math.cos(math.radians(angle) / 2.0
                                * (math.pi / math.radians(wideAngle)))))

class LightConeAngleBasic(LightManipulator):

    vpw = ValuePolicyWrapper

    UI_NAME = 'LCA Cone Angle'
    UI_SHORTCUTS = "B"
    
    centerOfInterest  = vpw('geometry.centerOfInterest')
    outerAngle        = vpw('material.lightParams.Cone_Outer_Angle')
    innerAngle        = vpw('material.lightParams.Cone_Inner_Angle')
    lightType         = vpw('material.lightParams.Type')

    def __init__(self, *args):
        LightManipulator.__init__(self, *args)

        # To make the lambda functions more efficient, define a local name
        snapshot = self.snapshot

        ### Create the handles ###

        # Disabled since it doesn't play nice with COI manips
        #        coiHandle           = self.createHandle(AxisHandle, 'centerOfInterest')
        #        coiHandle.name      = 'Center of Interest'
        #        coiHandle.axis      = Imath.V3d(0, 0, -1)
        #        coiHandle.minimum   = 0.001

        # oHandle                = self.createHandle(AxisHandle, 'outerAngle')
        # oHandle.name           = 'Cone Outer Angle'
        # oHandle.getOrigin      = lambda: Imath.V3d(0, 0, -(self.centerOfInterest or 0))
        # oHandle.axis           = Imath.V3d(0, 1, 0)
        # oHandle.getValue       = lambda: snapshot['outerAngleRadius']
        # oHandle.setValue       = self.setConeOuterAngleRadius
        # oHandle.getPrettyValue = lambda: '%.02f' % (self.outerAngle or 0)
        # oHandle.minimum        = 0.001
        # oHandle.canEdit        = False
        # iHandle                = self.createHandle(AxisHandle, 'innerAngle')
        # iHandle.getOrigin      = lambda: Imath.V3d(0, 0, -(self.centerOfInterest or 0))
        # iHandle.axis           = Imath.V3d(0, 1, 0)
        # iHandle.getValue       = lambda: snapshot['innerAngleRadius']
        # iHandle.setValue       = self.setConeInnerAngleRadius
        # iHandle.getPrettyValue = lambda: '%.02f' % (self.innerAngle or 0)
        # iHandle.minimum        = 0.001
        # iHandle.canEdit        = False
        # self.outerAngleHandle = oHandle
        # self.innerAngleHandle = iHandle

        self.setTransformSpace(scale = 'world')

        self._quadric = gluNewQuadric()

    def __del__(self):
        gluDeleteQuadric(self._quadric)

    def setConeOuterAngleRadius(self, radius):
        angle = max(math.atan(radius / self.snapshot['coneHeight']) * 180.0
                    / math.pi * 2.0, 0.0)
        self.outerAngle = angle

        # Make the inner angle smaller if the outer angle is set below it
        if self.outerAngle < self.innerAngle:
            self.innerAngle = self.outerAngle
            innerAnglePolicy = self.innerAngleHandle.getValuePolicy()
            if innerAnglePolicy:
                innerAnglePolicy.setValue(self.innerAngle,
                                          not self.innerAngleHandle.isDragging())

    def setConeInnerAngleRadius(self, radius):
        innerAngle = max(math.atan(radius / self.snapshot['coneHeight'])
                         * 180.0 / math.pi * 2.0, 0.0)
        self.innerAngle = innerAngle

        # Make the outer angle bigger if the inner angle is set above it
        if self.innerAngle > self.outerAngle:
            self.outerAngle = self.innerAngle
            outerAnglePolicy = self.outerAngleHandle.getValuePolicy()
            if outerAnglePolicy:
                outerAnglePolicy.setValue(self.outerAngle,
                                          not self.outerAngleHandle.isDragging())

    def calculateSnapshot(self):
        # First we figure out some useful values for drawing stuff WRT the light cone
        # The tip of the cone is the origin of the light when the lens radius is zero,
        # but if lens radius goes above zero the origin will move back behind the light.
        outerAngle = self.outerAngle
        innerAngle = self.innerAngle
        halfConeOuterAngle = math.radians(outerAngle) / 2
        halfConeInnerAngle = math.radians(innerAngle) / 2
        coneHeight = self.centerOfInterest

        # The radius of the inner and outer cones at the base.
        innerAngleRadius = math.tan(math.radians(innerAngle) / 2) * coneHeight
        outerAngleRadius = math.tan(math.radians(outerAngle) / 2) * coneHeight

        return locals()


    def drawOutline(self, drawPick, picked, currentView, currentCamera, manipulatorScale):
        innerAngle           = self.snapshot['innerAngle']
        outerAngle           = self.snapshot['outerAngle']
        halfConeInnerAngle   = self.snapshot['halfConeInnerAngle']
        halfConeOuterAngle   = self.snapshot['halfConeOuterAngle']
        coneHeight           = self.snapshot['coneHeight']
        innerAngleRadius     = self.snapshot['innerAngleRadius']
        outerAngleRadius     = self.snapshot['outerAngleRadius']

        # Time to do the drawing
        GL = GLStateManager.GLStateSet

        objMatrix = self.getWorldMatrix()

        glVisState = GL(cameraLight = True,       depthTest   = True,
                        depthMask   = False,      cullFace    = False,
                        transform   = objMatrix.sansScaling())
        glLineState = GL(color      = LINE_COLOR, lighting   = False)

        glVisState.push()

        # Draw the cone geometry
        glState = GL(translate     = (0, 0, -self.centerOfInterest)).push()

        #silhouettes
        glLineState.push()

        gluQuadricDrawStyle(self._quadric, GLU_SILHOUETTE);
        if innerAngleRadius > 0:
            gluDisk(self._quadric, 0, innerAngleRadius, 64, 1) #inner angle line
        if outerAngleRadius > 0 and innerAngleRadius > 0 and innerAngleRadius <= outerAngleRadius:
            gluDisk(self._quadric, innerAngleRadius, outerAngleRadius, 64, 1) #outer angle line

        glLineState.pop()

        # shaft
        glState(translate = (0, 0, self.centerOfInterest))

        if outerAngleRadius > 0:

            lightCylinderTipRadius = innerAngleRadius
            lightCylinderHeight = coneHeight

            glState(scale = (1, 1, -1))
            glColor4f(*BRIGHT_COLOR)
            gluQuadricDrawStyle(self._quadric, GLU_FILL)
            gluCylinder(self._quadric, 0, lightCylinderTipRadius, lightCylinderHeight, 16, 10)

        glState.pop()

        # Draw four lines to represent the cone
        glLineState.push()
        try:
            glBegin(GL_LINES)
            LR = 0
            LH = 0
            OR = outerAngleRadius
            for x in [ ( 0,  LR, LH),  ( 0,  OR, -self.centerOfInterest),
                       ( 0, -LR, LH),  ( 0, -OR, -self.centerOfInterest),
                       ( LR,  0, LH),  ( OR, 0,  -self.centerOfInterest),
                       (-LR,  0, LH),  (-OR, 0,  -self.centerOfInterest),
                       ( 0,   0, 0),   ( 0,  0,  -self.centerOfInterest) ]:
                glVertex3f(*x)
        finally:
            glEnd()
        glLineState.pop()

        glVisState.pop()

class LightConeAngleArnold(LightConeAngleBasic):

    vpw = ValuePolicyWrapper


    coneAngle     = vpw('material.arnoldLightParams.cone_angle')
    penumbraAngle = vpw('material.arnoldLightParams.penumbra_angle') 
    outerAngle    = None
    innerAngle    = None
    lightType     = None

    def __init__(self, *args):
        LightConeAngleBasic.__init__(self, *args)

        # Slight optimization for lambdas referring to self.snapshot
        snapshot = self.snapshot

        cHandle                = self.createHandle(CircleHandle, 'coneAngle')
        cHandle.name           = 'Cone Angle'
        cHandle.getOrigin      = lambda: Imath.V3d(0, 0, -self.centerOfInterest)
        cHandle.axis           = Imath.V3d(0, 0, 1)
        cHandle.getValue       = lambda: snapshot.get('outerAngleRadius', 0)
        cHandle.setValue       = self.setConeOuterAngleRadius
        cHandle.getPrettyValue = lambda: '%.02f' % (self.coneAngle or 0)
        #cHandle.color          = (0.5, 1.0, 0.5)
        pHandle                = self.createHandle(CircleHandle, 'penumbraAngle')
        pHandle.name           = 'Penumbra Angle'
        pHandle.getOrigin      = lambda: Imath.V3d(0, 0, -(self.centerOfInterest or 0))
        pHandle.axis           = Imath.V3d(0, 0, -1)
        pHandle.getValue       = lambda: snapshot.get('innerAngleRadius', 0)
        pHandle.setValue       = self.setConeInnerAngleRadius
        pHandle.getPrettyValue = lambda: '%.02f' % (self.penumbraAngle or 0)
        #pHandle.color          = (0.5, 1.0, 0.5)
        
        cHandle.getMinimum     = lambda: pHandle.getValue()
        pHandle.getMaximum     = lambda: cHandle.getValue()
        pHandle.minimum        = 0.001

    def calculateSnapshot(self):
        # Put the values where the original version of calculateSnapshot
        # expects to find them
        self.outerAngle = self.coneAngle
        self.innerAngle = self.coneAngle - self.penumbraAngle

        return LightConeAngleBasic.calculateSnapshot(self)

    def setConeOuterAngleRadius(self, radius):
        angle = max(math.atan(radius / self.snapshot['coneHeight']) * 180.0
                    / math.pi * 2.0, 0.0)
        self.coneAngle = angle
        if self.penumbraAngle > angle:
            self.penumbraAngle = angle

    def setConeInnerAngleRadius(self, radius):
        innerAngle = max(math.atan(radius / self.snapshot['coneHeight'])
                         * 180.0 / math.pi * 2.0, 0.0)
        self.penumbraAngle = self.snapshot['outerAngle'] - innerAngle


class LightConeAngleArnoldOSL(LightConeAngleBasic):

    vpw = ValuePolicyWrapper

    coneAngle     = vpw('material.parameters.cone_angle')
    penumbraAngle = vpw('material.parameters.penumbra_angle') 
    outerAngle    = None
    innerAngle    = None
    lightType     = None

    def __init__(self, *args):
        LightConeAngleBasic.__init__(self, *args)

        # Slight optimization for lambdas referring to self.snapshot
        snapshot = self.snapshot

        cHandle                = self.createHandle(CircleHandle, 'coneAngle')
        cHandle.name           = 'Cone Angle'
        cHandle.getOrigin      = lambda: Imath.V3d(0, 0, -self.centerOfInterest)
        cHandle.axis           = Imath.V3d(0, 0, 1)
        cHandle.getValue       = lambda: snapshot.get('outerAngleRadius', 0)
        cHandle.setValue       = self.setConeOuterAngleRadius
        cHandle.getPrettyValue = lambda: '%.02f' % (self.coneAngle or 0)
        #cHandle.color          = (0.5, 1.0, 0.5)
        pHandle                = self.createHandle(CircleHandle, 'penumbraAngle')
        pHandle.name           = 'Penumbra Angle'
        pHandle.getOrigin      = lambda: Imath.V3d(0, 0, -(self.centerOfInterest or 0))
        pHandle.axis           = Imath.V3d(0, 0, -1)
        pHandle.getValue       = lambda: snapshot.get('innerAngleRadius', 0)
        pHandle.setValue       = self.setConeInnerAngleRadius
        pHandle.getPrettyValue = lambda: '%.02f' % (self.penumbraAngle or 0)
        #pHandle.color          = (0.5, 1.0, 0.5)
        
        cHandle.getMinimum     = lambda: pHandle.getValue()
        pHandle.getMaximum     = lambda: cHandle.getValue()
        pHandle.minimum        = 0.001

    def calculateSnapshot(self):
        # Put the values where the original version of calculateSnapshot
        # expects to find them
        self.outerAngle = self.coneAngle
        self.innerAngle = self.coneAngle - self.penumbraAngle

        return LightConeAngleBasic.calculateSnapshot(self)

    def setConeOuterAngleRadius(self, radius):
        angle = max(math.atan(radius / self.snapshot['coneHeight']) * 180.0
                    / math.pi * 2.0, 0.0)
        self.coneAngle = angle
        if self.penumbraAngle > angle:
            self.penumbraAngle = angle

    def setConeInnerAngleRadius(self, radius):
        innerAngle = max(math.atan(radius / self.snapshot['coneHeight'])
                         * 180.0 / math.pi * 2.0, 0.0)
        self.penumbraAngle = self.snapshot['outerAngle'] - innerAngle


PluginRegistry = [
    ("KatanaManipulator", 2.0, "LightConeAngleArnold",  LightConeAngleArnold),
    ("KatanaManipulator", 2.0, "LightConeAngleArnoldOSL",  LightConeAngleArnoldOSL),
]
