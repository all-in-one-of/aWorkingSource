# Copyright (c) 2017 Light Chaser Animation Inc. All rights reserved.

from __future__ import with_statement

import Katana, FnKatImport
FnKatImport.FromObject(Katana.Plugins.ViewerManipulatorAPI, merge = ['*'])

from math import sin, cos, tan, radians
from math import pi as PI
from LightManipulatorBase import LightManipulator


#from LightManipulator import LightManipulator
class ReLightDecay(LightManipulator):

    UI_NAME = 'LCA Decay'
    UI_SHORTCUTS = "Tab"
    vpw = ValuePolicyWrapper

    useNear     = vpw('material.arnoldLightFilterParams.use_near_atten', uiOrder = 1)
    nearStart   = vpw('material.arnoldLightFilterParams.near_start',     uiOrder = 2)
    nearEnd     = vpw('material.arnoldLightFilterParams.near_end',       uiOrder = 3)
    useFar      = vpw('material.arnoldLightFilterParams.use_far_atten',  uiOrder = 4)
    farStart    = vpw('material.arnoldLightFilterParams.far_start',      uiOrder = 5)
    farEnd      = vpw('material.arnoldLightFilterParams.far_end',        uiOrder = 6)
    radius      = vpw('material.arnoldLightParams.radius',               uiOrder = 7, optional=True)
    coneAngle   = vpw('material.arnoldLightParams.cone_angle',optional=True)
    scale       = vpw('xform.interactive.scale')
    lightType   = vpw('material.arnoldLightShader')
    directionLight = ['spot_light','distant_light','disk_light','quad_light']
    def __init__(self, *args):
        LightManipulator.__init__(self, *args)
        snapshot = self.snapshot
        nsHandle            = self.createHandle(CircleHandle, 'nearStart')
        nsHandle.axis       = Imath.V3d(0.0, 1.0, 0.0)
        nsHandle.minimum    = 0.0
        nsHandle.color      = (1, 1, 0)
        nsHandle.getPosition = lambda:self.snapshot['nsHandlePos']
        neHandle            = self.createHandle(CircleHandle, 'nearEnd')
        neHandle.color      = (1, 1, 0)
        neHandle.axis       = Imath.V3d(0.0, 1.0, 0.0)
        neHandle.getPosition = lambda:self.snapshot['neHandlePos']
        fsHandle            = self.createHandle(CircleHandle, 'farStart')
        fsHandle.axis       = Imath.V3d(0.0, 1.0, 0.0)
        fsHandle.color      = (1, 1, 0)
        fsHandle.getPosition = lambda:self.snapshot['fsHandlePos']
        feHandle            = self.createHandle(CircleHandle, 'farEnd')
        feHandle.axis       = Imath.V3d(0.0, 1.0, 0.0)
        feHandle.color      = (1, 1, 0)
        feHandle.getPosition = lambda:self.snapshot['feHandlePos']
        # nsHandle.getMaximum = lambda: neHandle.getValue()
        # neHandle.getMinimum = lambda: nsHandle.getValue()
        # neHandle.getMaximum = lambda: fsHandle.getValue()
        # fsHandle.getMinimum = lambda: neHandle.getValue()
        # fsHandle.getMaximum = lambda: feHandle.getValue()
        # feHandle.getMinimum = lambda: fsHandle.getValue()
        

        self.setPreferredHandle(nsHandle)
        self.setPreferredHandle(neHandle)
        self.setPreferredHandle(fsHandle)
        self.setPreferredHandle(feHandle)
        self.setTransformSpace(translate = 'local', scale = 'world', rotate = 'local')
    def calculateSnapshot(self):
        coneAngle = self.coneAngle
        scale = self.scale 
        nearStart = self.nearStart
        nearEnd = self.nearEnd
        farStart = self.farStart
        farEnd = self.farEnd
        lat = 0.667
        lon = 0.667
        awa = self.coneAngle
        if self.lightType in self.directionLight:
            if awa is None:
                awa = 90
        else:
            awa = None
        if not awa is None: 
            mult = radians(awa) / 2
        else:               
            mult = PI
        lat  = mult * lat
        lon = mult * lon
        zr = sin(lon)
        if nearStart and nearEnd and farStart and farEnd:
            nsHandlePos= Imath.V3d((sin(lat) * zr)*nearStart, (cos(lat) * zr)*nearStart, (-cos(lon))*nearStart)
            neHandlePos= Imath.V3d((sin(lat) * zr)*nearEnd, (cos(lat) * zr)*nearEnd, (-cos(lon))*nearEnd)
            fsHandlePos= Imath.V3d((sin(lat) * zr)*farStart, (cos(lat) * zr)*farStart, (-cos(lon))*farStart)
            feHandlePos= Imath.V3d((sin(lat) * zr)*farEnd, (cos(lat) * zr)*farEnd, (-cos(lon))*farEnd)
        #print "LCA Dedug:scare %s"%(self.lightType)

        return locals()

    def getAxis(self, lat, long):
        awa = self.coneAngle
        if not awa is None: mult = radians(awa) / 2
        else:               mult = PI
        lat  = mult * lat
        long = mult * long
        zr = sin(long)
        return Imath.V3d(sin(lat) * zr, cos(lat) * zr, -cos(long))


    def drawOutline(self, drawPick, pickId, currentView, currentCamera, manipulatorScale):

        if self.lightType in self.directionLight:
            ang = 120
        else:
            ang = 360
        angle = PI - radians(self.coneAngle or ang) / 2

        self.displaySettings = GL(depthTest = True,depthMask = False,blend = True,color = (0.5,0.5,0.6,0.1),cameraLight = False,cullFace = False)

        with GL(transform = self.getWorldMatrix().sansScaling()):
            with self.displaySettings:
                for radius in (self.nearStart, self.nearEnd,self.farStart, self.farEnd):
                    if radius:
                        ViewerUtils.drawSphere(startAngle = angle, endAngle = PI, radius = radius)
                        ViewerUtils.drawSphere(startAngle = angle, endAngle = PI, radius = radius, reverseNormals = True)

PluginRegistry = [
    ("KatanaManipulator", 2.0, "LightDecay",  ReLightDecay)
]

