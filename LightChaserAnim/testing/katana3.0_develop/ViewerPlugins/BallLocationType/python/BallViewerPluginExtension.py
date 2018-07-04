# Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import FnAttribute, FnGeolibServices
from PluginAPI.BaseViewerPluginExtension import BaseViewerPluginExtension
       
class BallAppender(BaseViewerPluginExtension):
    """
    This class is responsible for automatically adding the 'BallComponent' 
    ViewerDelegateComponent to a viewer delegate, and also for adding the
    C{BallLayer} to viewports in order to draw ball locations.
    
    It additionally adds a terminal op which provides a default radius for
    ball locations, but in reality, this is best left as a default value
    within the C{BallComponent} plug-in.
    """
    def __init__(self):
        BaseViewerPluginExtension.__init__(self)
        
    def onDelegateCreated(self, viewerDelegate, pluginName):
        """
        Adds the C{BallComponent} VDC to the new viewer delegate.
        
        @type viewerDelegate: C{ViewerDelegate}
        @type pluginName: C{str}
        @param viewerDelegate: The C{ViewerDelegate} that was created.
        @param pluginName: The registered plug-in name of the C{ViewerDelegate}.
        """
        viewerDelegate.addComponent('BallComponent', 'BallComponent')
        
    def onViewportCreated(self, viewportWidget, pluginName, viewportName):
        """
        Adds the C{BallLayer} viewport layer to the new viewport.
        
        @type viewportWidget: C{ViewportWidget}
        @type pluginName: C{str}
        @type viewportName: C{str}
        @param viewportWidget: The C{ViewportWidget} that was created.
        @param pluginName: The registered plug-in name of the C{ViewportWidget}.
        @param viewportName: The easily identifiable name of the viewport
            which can be used to look-up the viewport in the C{ViewerDelegate}.
        """
        BaseViewerPluginExtension.onViewportCreated(self, viewportWidget, pluginName, viewportName)
        viewportWidget.insertLayer('BallLayer', 'BallLayer', 4)
        
    def onApplyTerminalOps(self, txn, inputOp, viewerDelegate):
        """
        Appends an AttributeSet op to the terminal ops, to set the
        'geometry.radius' attribute on any 'ball' locations that 
        do not already have a radius set.
        
        @type txn: C{FnGeolib.GeolibRuntime.Transaction}
        @type inputOp: C{FnGeolib.GeolibRuntimeOp} or C{None}
        @type viewerDelegate: C{ViewerAPI.ViewerDelegate}
        @rtype: C{FnGeolib.GeolibRuntimeOp}
        @param txn: The current Geolib3 tansaction.
        @param op: The last Op in the current Op-tree.
        @param viewerDelegate: The ViewerDelegate.
        @return: The final terminal Op.
        """
        terminalOp = txn.createOp()
        argsBuilder = FnGeolibServices.OpArgsBuilders.AttributeSet()
        argsBuilder.setCEL([
            '/root/world//*{attr("type") == "ball" '
            'and not hasattr("geometry.radius")}'])
        argsBuilder.setAttr('geometry.radius',
            FnAttribute.DoubleAttribute(5.0))
        txn.setOpArgs(terminalOp, 'AttributeSet', argsBuilder.build())
        txn.setOpInputs(terminalOp, [inputOp])
        
        return terminalOp
        
        
PluginRegistry = [
    ("ViewerPluginExtension", 1, "BallAppender", BallAppender),
]
