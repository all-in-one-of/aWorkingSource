# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

from PluginAPI.BaseProxyLoader import BaseProxyLoader
from Nodes3DAPI.Manifest import FnAttribute


class AlembicProxyLoader(BaseProxyLoader):
    @classmethod
    def getFileExtensions(cls):
        return ['abc']

    @classmethod
    def createProxyAttr(cls, proxyFile, time, args):
        gb = FnAttribute.GroupBuilder()

        gb.set('opType', FnAttribute.StringAttribute('AlembicIn'))

        # Build the attributes for the AlembicIn Op:
        gb.set('opArgs.fileName', FnAttribute.StringAttribute(proxyFile))
        gb.set('opArgs.system.timeSlice.currentTime',
               FnAttribute.FloatAttribute(time))
        gb.set('opArgs.system.timeSlice.numSamples',
               FnAttribute.IntAttribute(1))

        return gb.build()

PluginRegistry = [
    ("ViewerProxyLoader", 1, "Alembic", AlembicProxyLoader),
]
