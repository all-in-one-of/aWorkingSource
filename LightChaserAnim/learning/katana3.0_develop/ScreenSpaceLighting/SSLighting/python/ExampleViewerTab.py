# Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import (
    FnAttribute,
    FnGeolib,
    FnGeolibServices,
    LiveRenderAPI,
    PluginSystemAPI,
    NodegraphAPI,
    Nodes3DAPI,
    QtGui,
    QtCore,
    RenderManager,
    UI4,
    Utils,
    ViewerAPI,
    WorkingSetManager,
    WorkingSet
)
from UI4.Widgets import ViewerWorkingSetButton, LiveRenderViewerCameraButton
import UI4.Widgets
import ViewerAPI_cmodule

from ManipulatorMenu import ManipulatorMenu

import logging
log = logging.getLogger("ExampleViewer")
RenderGlobals = RenderManager.RenderGlobals

class ExampleViewerTab(UI4.Tabs.BaseViewerTab):
    """
    Class implementing a tab for hosting an example Viewer widget.
    """

    def __init__(self, parent, flags=0):
        """
        Initializes an instance of the class.

        @type parent: C{QtGui.QWidget} or C{None}
        @type flags: C{QtCore.Qt.WindowFlags}
        @param parent: The parent widget to own the new instance. Passed
            verbatim to the base class initializer.
        @param flags: The window flags to use in case no parent is given.
            Passed verbatim to the base class initializer.
        """
        UI4.Tabs.BaseViewerTab.__init__(self, parent, flags)

        self.__selectedManipulators = []
        # The desired layers to be added to each viewport
        self.__addedLayers = ["GridLayer",
                              "ExampleSceneLayer",
                              "SelectionLayer",
                              "GLManipulatorLayer",
                              "HUDLayer",
                              "CameraControlLayer",
                              "CameraGateLayer",
                              ]

        # The terminal Op. It will be defined after L{applyTerminalOp} gets
        # called.
        self.__terminalOp = None
        self.__cameraName = "persp"

        # A list of viewports whose camera lists need populating and their
        # associated camera picker buttons
        self.__populateViewportCameras = []

        # Holds the viewport names as well as their UI elements such as the
        # camera picker buttons or the active camera status icon.
        self.__viewportNamesAndButtons = {}

        # Instantiate a Viewer Delegate and the left and right Viewport widgets
        self.__viewerDelegate = self.addViewerDelegate("ExampleViewerDelegate")

        self.__viewportWidgetL = self.addViewport("ExampleViewport",
                                                  "left",
                                                  self.__viewerDelegate,
                                                  layers=self.__addedLayers)
        self.__viewportWidgetR = self.addViewport("ExampleViewport",
                                                  "right",
                                                  self.__viewerDelegate,
                                                  layers=self.__addedLayers)

        self.__viewports = [self.__viewportWidgetL, self.__viewportWidgetR]

        # A map of viewport names and the pending built-in cameras that will
        # be set in the next loop event.
        self.__pendingBuiltInCameraRequests = {}

        # Default background color
        backgroundColorAttr = FnAttribute.FloatAttribute([0.22, 0.22, 0.22, 1])

        for viewportWidget in  self.__viewports:
            # Set the background color
            viewportWidget.setOptionByName("Viewport.BackgroundColor",
                                           backgroundColorAttr)
            gridLayer = viewportWidget.getLayer('GridLayer')
            if gridLayer:
                gridLayer.setOptionByName("Viewport.BackgroundColor",
                                          backgroundColorAttr)

        # Set up the viewport container
        self.setLayout(QtGui.QVBoxLayout())
        viewportFrame = QtGui.QSplitter(self)
        self.layout().addWidget(viewportFrame)
        viewportFrame.setOrientation(QtCore.Qt.Horizontal)
        viewportFrame.setSizePolicy(QtGui.QSizePolicy.Expanding,
                                    QtGui.QSizePolicy.Expanding)

        # Create a pane for each viewport with viewport-specific controls
        # e.g. for camera selection
        leftPane = self.createViewportPane(self.__viewportWidgetL)
        viewportFrame.addWidget(leftPane)

        rightPane = self.createViewportPane(self.__viewportWidgetR)
        viewportFrame.addWidget(rightPane)

        # Create a status bar
        self.__statusBar = QtGui.QStatusBar()
        self.__statusBar.setObjectName('statusBar')
        self.__statusBar.setSizeGripEnabled(False)
        self.__statusBar.showMessage('Ready.')
        self.layout().addWidget(self.__statusBar)

        self.__statusBarMessage = ""

        # Create the Manipulators menu.
        self.__manipulatorsMenu = ManipulatorMenu("Manipulators",
                                                  self, self.__viewerDelegate,
                                                  self.__viewports)
        self.__manipulatorsMenu.setObjectName("ManipulatorMenu")

        self.__layersMenu = QtGui.QMenu("Layers", self)
        self.__layersMenu.setObjectName("LayersMenu")
        QtCore.QObject.connect(self.__layersMenu,
                               QtCore.SIGNAL("aboutToShow()"),
                               self.on_layersMenu_aboutToShow)

        menuBar = QtGui.QMenuBar(self)
        menuBar.addMenu(self.__manipulatorsMenu)
        menuBar.addMenu(self.__layersMenu)
        self.layout().setMenuBar(menuBar)

    def applyTerminalOps(self, txn, op, viewerDelegate):
        '''
        This adds an Op that sets an attribute called 'viewerType' with value
        'ExampleViewer' on /root. This demonstrates how to set terminal Ops for
        a custom viewer.

        @type txn: C{PyFnGeolib.GeolibRuntime.Transaction}
        @type op: C{PyFnGeolib.GeolibRuntime.Op}
        @type viewerDelegate: C{ViewerAPI.ViewerDelegate}
        @rtype: C{PyFnGeolib.GeolibRuntime.Op}
        @param txn: A Geolib3 transaction.
        @param op: The last Op in the current Op-tree.
        @param viewerDelegate: The viewer delegate.
        @return: The last appended Op, or the given Op if no Ops are appended.
        '''
        terminalOp = txn.createOp()
        argsBuilder = FnGeolibServices.OpArgsBuilders.AttributeSet()
        argsBuilder.setCEL(['/root'])
        argsBuilder.setAttr('viewer.name',
            FnAttribute.StringAttribute('ExampleViewer'))
        argsBuilder.build()
        txn.setOpArgs(terminalOp, 'AttributeSet', argsBuilder.build())
        txn.setOpInputs(terminalOp, [op])

        return terminalOp

    def on_layersMenu_aboutToShow(self):
        """
        Populates the Layers menu, allowing the arbitrary insertion and removal
        of layers.
        """
        self.__layersMenu.clear()

        addLayerMenu = QtGui.QMenu("Add Layer", self)
        addLayerMenu.setObjectName("AddLayerMenu")

        # List registered layer plug-ins
        pluginVersion = ViewerAPI_cmodule.kViewportLayerPluginVersion
        pluginTypes = PluginSystemAPI.PluginCache.GetPluginNames(pluginVersion)
        for layerName in pluginTypes['ViewportLayerPlugin']:
            action = QtGui.QAction(layerName, addLayerMenu)
            action.setObjectName("Add%sAction" % layerName.replace(" ", ""))
            action.setData(layerName)
            addLayerMenu.addAction(action)
            # Clicking one of these actions will add the layer
            QtCore.QObject.connect(action, QtCore.SIGNAL("triggered()"),
                                   self.on_addLayerAction_triggered)
        self.__layersMenu.addMenu(addLayerMenu)

        action = QtGui.QAction("Remove Layers", self.__layersMenu)
        action.setObjectName("RemoveLayerAction")
        action.setSeparator(True)
        self.__layersMenu.addAction(action)

        # List the currently added layers
        for index, layerName in enumerate(self.__addedLayers):
            action = QtGui.QAction("%s) %s" % (index, layerName), self.__layersMenu)
            action.setObjectName(layerName.replace(" ", "") + "Action")
            action.setData(index)
            # Clicking one of these actions will remove this layer
            QtCore.QObject.connect(action, QtCore.SIGNAL("triggered()"),
                                   self.on_removeLayerAction_triggered)
            self.__layersMenu.addAction(action)

    def on_removeLayerAction_triggered(self):
        """
        Slot to remove a layer from viewports.
        """
        if self.sender() is not None:
            if not self.sender().data().isNull():
                index = int(self.sender().data().toPyObject())
                del self.__addedLayers[index]
                for viewport in self.__viewports:
                    viewport.removeLayerByIndex(index)
            else:
                log.error("The action that called "
                          "on_removeLayerAction_triggered() does not have any "
                          "data attached to it.")
        else:
            log.error("on_removeLayerAction_triggered() is expected to be "
                      "called from an action in the Layers menu.")

    def on_addLayerAction_triggered(self):
        """
        Slot to remove a layer from viewports.
        """
        if self.sender() is not None:
            if not self.sender().data().isNull():
                layerType = str(self.sender().data().toPyObject())

                if layerType in self.__addedLayers:
                    log.warning("Attempting to add layer '%s' to viewports, "
                                "but it has already been added." % layerType)
                    return

                self.__addedLayers.append(layerType)

                for viewport in self.__viewports:
                    viewport.addLayer(layerType, layerType)
            else:
                log.error("The action that called "
                          "on_removeLayerAction_triggered() does not have any "
                          "data attached to it.")
        else:
            log.error("on_removeLayerAction_triggered() is expected to be "
                      "called from an action in the Layers menu.")

    def on_cameraPicker_aboutToShow(self):
        """
        Slot that is called when the popup for the camera picker button is
        about to be opened.

        Rebuilds the list of available items in the popup.
        """
        # Check that all camera pickers have the default cameras visible
        for viewport, cameraPickerButton in self.__populateViewportCameras:
            cameraPickerButton.clearBuiltins()
            for index in range(viewport.getNumberOfCameras()):
                camName = viewport.getCameraName(index)
                cameraPickerButton.addBuiltin(camName)

            # Manually populate now because the internal on_aboutToShow()
            # function gets called before this one.
            cameraPickerButton.populate()

        self.__populateViewportCameras = []

    def createViewportPane(self, viewport):
        """
        Creates a viewport widget and a camera selection control.
        """
        # Add the camera picker button in the center
        cameraPickerButton = UI4.Widgets.CameraPickerButton(
            self, showIcon=True, showBuiltIns=True, abbreviateNames=False)
        cameraPickerButton.setDisplayMode(
            UI4.Widgets.CameraPickerButton.DISPLAY_CAMERAS
            | UI4.Widgets.CameraPickerButton.DISPLAY_LIGHTS)
        cameraPickerButton.setButtonType("textToolbar")
        cameraPickerButton.setText("persp")

        self.__populateViewportCameras.append((viewport, cameraPickerButton))

        QtCore.QObject.connect(cameraPickerButton,
                               QtCore.SIGNAL('aboutToShow'),
                               self.on_cameraPicker_aboutToShow)
        QtCore.QObject.connect(cameraPickerButton,
                               QtCore.SIGNAL('lookThrough'),
                               lambda path: self.setCamera(
                                   viewport, path))

        # Create the status icon for the active camera.
        lookThroughWarningIcon = QtGui.QLabel(self)
        lookThroughWarningIcon.setToolTip("Location does not exist")
        lookThroughWarningIcon.setPixmap(
            UI4.Util.IconManager.GetPixmap("Icons/Scenegraph/warning16.png"))
        lookThroughWarningIcon.setFixedSize(
            UI4.Util.IconManager.GetSize("Icons/Scenegraph/warning16.png"))
        lookThroughWarningIcon.hide()

        # Create the status icon for the active camera.
        lookThroughCookingIcon = UI4.Widgets.SimpleStopWidget(self, small=True)
        lookThroughCookingIcon.setToolTip("Waiting for location")
        lookThroughCookingIcon.setFixedHeight(16)
        lookThroughCookingIcon.setClickable(False)
        lookThroughCookingIcon.setActive(False)
        lookThroughCookingIcon.setVisible(False)

        # Bind the viewport name and the UI elements into the map for future
        # access.
        self.__viewportNamesAndButtons[viewport.getName()] = \
            (cameraPickerButton,
             lookThroughWarningIcon,
             lookThroughCookingIcon)

        # Create a viewer visibility button
        viewerVisibilityButton = ViewerWorkingSetButton(self)
        liveRenderFromCameraButton = LiveRenderViewerCameraButton(self, viewport)

        # Horizontal layout with the camera picker button and the status icon.
        hboxLayout = QtGui.QHBoxLayout()
        hboxLayout.addWidget(cameraPickerButton)
        hboxLayout.addWidget(lookThroughWarningIcon)
        hboxLayout.addWidget(lookThroughCookingIcon)
        hboxLayout.addStretch()
        hboxLayout.addWidget(liveRenderFromCameraButton)
        hboxLayout.addWidget(viewerVisibilityButton)

        # Layout for the pane.
        layout = QtGui.QVBoxLayout()
        layout.addWidget(viewport)
        layout.addLayout(hboxLayout)

        # Create the actual pane and set the layout.
        vbox = QtGui.QFrame(self)
        vbox.setLayout(layout)
        return vbox

    def setCamera(self, viewportWidget, nameOrPath):
        """
        Tells the Viewport which camera to draw from.

        @type viewportWidget: C{ViewportWidget}
        @type nameOrPath: C{str}
        @param viewportWidget: The viewport whose camera location should be
            set.
        @param nameOrPath: The name of the built in camera or the path to the
            camera location from which the given viewport should view the
            scene.
        """
        if not isinstance(nameOrPath, str):
            return

        viewport = viewportWidget.getViewport()

        # Is this a location backed camera?
        if nameOrPath and nameOrPath[0] == '/':
            # We do not wish to allow interactions until the new look-through
            # location is ready.
            camera = viewport.getActiveCamera()
            if camera:
                camera.disableInteraction(True)

            # We're going to set the location as the look-through location.
            # When the location is ready, we'll receive a callback, and only
            # then we'll reset the active camera.
            self.__viewerDelegate.setLookThroughLocation(viewport, nameOrPath)
            return

        # Otherwise, this is considered a built-in camera, which can be set
        # immediately.
        self.setBuiltInCamera(viewportWidget, nameOrPath)

    def setBuiltInCamera(self, viewportWidget, cameraName):
        """
        Tells the Viewport which built-in camera to draw from.

        @type viewportWidget: C{ViewportWidget}
        @type cameraName: C{str}
        @param viewportWidget: The viewport whose camera location should be
            set.
        @param cameraName: The name of the built in camera from which the given
            viewport should view the scene.
        """
        viewport = viewportWidget.getViewport()

        camera = viewportWidget.getCameraByName(cameraName)
        if not camera:
            log.warning("No camera found in Viewport '%s': %s",
                        viewportWidget.getViewport().getName(),
                        cameraName)
            return

        # Make sure that interactions are allowed.
        camera.disableInteraction(False)

        # Clear the look-through location from the Viewer Delegate.
        self.__viewerDelegate.setLookThroughLocation(viewport, "")

        # Set this camera as the active camera
        viewportWidget.setActiveCamera(camera)
        camera.setViewportDimensions(viewportWidget.width(),
                                     viewportWidget.height())
        viewportWidget.update()
        viewportWidget.setDirty(True)

        # Need to refresh the manipulators, as the viewport's active camera
        # have changed.
        optionId = ViewerAPI.GenerateOptionId("ReactivateManipulators")
        optionValue = FnAttribute.NullAttribute()
        self.__viewerDelegate.setOption(optionId, optionValue)

        # Ensure live rendering knows about the new camera
        cameraType, _ = RenderGlobals.GetRerenderCamera()
        useViewerCamera = (
            cameraType == RenderGlobals.RENDERCAMERATYPE_VIEWER or
            cameraType == RenderGlobals.RENDERCAMERATYPE_SPECIFIC)
        self.setActiveLiveRenderCamera(useViewerCamera, cameraName, camera)

    def saveViewports(self):
        """
        Saves the contents of the viewports to images on disk. Hard-wired to
        write to "/tmp/viewport_left.png" and "/tmp/viewport_right.png".
        """
        self.__viewportWidgetL.writeToFile("/tmp/viewport_left.png")
        if self.__viewportWidgetR:
            self.__viewportWidgetR.writeToFile("/tmp/viewport_right.png")

    def _on_scenegraphManager_selectionChanged(self, eventType, eventID, **kwargs):
        """
        Handles C{scenegraphManager_selectionChanged} events by dirtying
        attached viewports.
        """
        UI4.Tabs.BaseViewerTab._on_scenegraphManager_selectionChanged(self,
                                                                      eventType,
                                                                      eventID,
                                                                      **kwargs)
        self.__manipulatorsMenu._on_selection_changed()

    def _on_event_idle(self, _eventType, _eventID):
        """
        This extends the BaseViewerTab implementation.
        """
        UI4.Tabs.BaseViewerTab._on_event_idle(self, _eventType, _eventID)

        # Update the cooking icon, so that it spins.
        for _, _, cookingIcon in self.__viewportNamesAndButtons.itervalues():
            if cookingIcon.getActive():
                cookingIcon.advanceTick()

        # Check if there was a pending request to change to a built-in camera.
        for viewportName, cameraName in \
                self.__pendingBuiltInCameraRequests.iteritems():
            viewportWidget = self.getViewportWidget(
                viewportName, self.__viewerDelegate)
            if viewportWidget:
                self.setBuiltInCamera(viewportWidget, cameraName)
                cameraPickerButton, _, _ = \
                    self.__viewportNamesAndButtons[viewportName]
                cameraPickerButton.setText(cameraName)

        self.__pendingBuiltInCameraRequests = {}

        # Check if there is a status update to display
        statusOptionId = ViewerAPI.GenerateOptionId("ExampleViewerStatus")
        statusOptionAttr = self.__viewerDelegate.getOption(statusOptionId)
        if statusOptionAttr:
            status = statusOptionAttr.getValue("")
            self.__setStatusBarMessage(status)

    def _on_lookThroughLocation_changeRequested(self, viewportName, location):
        """
        Overridden from base class to update the picker and the status icon.
        """
        UI4.Tabs.BaseViewerTab._on_lookThroughLocation_changeRequested(
            self, viewportName, location)

        _, warningIcon, cookingIcon = \
            self.__viewportNamesAndButtons[viewportName]
        warningIcon.hide()
        cookingIcon.setActive(True)
        cookingIcon.setVisible(True)

    def _on_lookThroughLocation_doesNotExist(self, viewportName, location):
        """
        Overridden from base class to update the picker and the status icon.
        """
        UI4.Tabs.BaseViewerTab._on_lookThroughLocation_doesNotExist(
            self, viewportName, location)

        _, warningIcon, cookingIcon = \
            self.__viewportNamesAndButtons[viewportName]
        warningIcon.show()
        cookingIcon.setActive(False)
        cookingIcon.setVisible(False)

        # Mark the viewport dirty, so that things like the CameraGateLayer
        # can be re-drawn.
        viewportWidget = self.getViewportWidget(viewportName,
                                                self.__viewerDelegate)
        viewportWidget.setDirty(True)

        # Pop to "persp". However, we cannot do this during the callback, we
        # need to defer it.
        self.__pendingBuiltInCameraRequests[viewportName] = "persp"

    def _on_lookThroughLocation_changed(self, viewportName, location, attrs):
        """
        Overridden from base class to update the picker and the status icon.
        """
        UI4.Tabs.BaseViewerTab._on_lookThroughLocation_changed(
            self, viewportName, location, attrs)

        _, warningIcon, cookingIcon = \
            self.__viewportNamesAndButtons[viewportName]
        warningIcon.hide()
        cookingIcon.setActive(False)
        cookingIcon.setVisible(False)

        # Setup the camera with the new location and attributes.
        viewportWidget = self.getViewportWidget(viewportName,
                                                self.__viewerDelegate)
        self.__setupCamera(viewportWidget, location, attrs)

        # Get the viewport and the active camera.
        viewport = self.__viewerDelegate.getViewport(viewportName)
        camera = viewport.getActiveCamera()

        # Make sure that interactions are allowed.
        camera.disableInteraction(False)

        # Need to refresh the manipulators, as the viewport's active camera
        # have changed.
        optionId = ViewerAPI.GenerateOptionId("ReactivateManipulators")
        optionValue = FnAttribute.NullAttribute()
        self.__viewerDelegate.setOption(optionId, optionValue)

        # Ensure live rendering knows about the new camera
        cameraType, _ = RenderGlobals.GetRerenderCamera()
        useViewerCamera = (
            cameraType == RenderGlobals.RENDERCAMERATYPE_VIEWER or
            cameraType == RenderGlobals.RENDERCAMERATYPE_SPECIFIC)
        self.setActiveLiveRenderCamera(useViewerCamera, location, camera)

    def _on_lookThroughLocation_cooked(self, viewportName, location, attrs):
        """
        Overridden from base class to update the camera, in case its attributes
        have changed.
        """
        UI4.Tabs.BaseViewerTab._on_lookThroughLocation_cooked(
            self, viewportName, location, attrs)

        # Setup the camera with the new location and attributes.
        viewportWidget = self.getViewportWidget(viewportName,
                                                self.__viewerDelegate)
        self.__setupCamera(viewportWidget, location, attrs)

    def __setupCamera(self, viewportWidget, location, attrs):
        """
        Sets the camera up given the viewport, the scene graph location and its
        attributes.

        It resets the camera geometry, dimensions, location path, sets the
        camera active, marks the viewport dirty, recreates the camera if the
        projection has changed, etc.

        @type viewportWidget: C{ViewportWidget}
        @type location: C{str}
        @type attrs: C{FnAttribute.GroupAttribute}
        @param viewportWidget: The viewport whose camera location should be
            set.
        @param location: This is the look-through location for the camera.
        @param attrs: Attributes for the camera that includes the xform, center
            of interest, near/fov planes, etc.
        """
        # Get the expected camera type for the current projection.
        cameraTypeID = ViewerAPI.kFnViewportCameraTypePerspective
        projectionAttr = attrs.getChildByName("geometry.projection")
        if projectionAttr and projectionAttr.getValue("") == "orthographic":
            cameraTypeID = ViewerAPI.kFnViewportCameraTypeOrthographic

        # Create the camera if it doesn't exist, or if camera type is not the
        # one that is required for the current projection.
        cameraName = "__location"
        camera = viewportWidget.getCameraByName(cameraName)
        if not camera or camera.getCameraTypeID() != cameraTypeID:
            pluginName = "PerspectiveCamera"
            if cameraTypeID == ViewerAPI.kFnViewportCameraTypeOrthographic:
                pluginName = "OrthographicCamera"

            # Add (or replace) the camera with the correct plug-in name.
            camera = viewportWidget.addCamera(pluginName, cameraName)

        # Setup the camera.
        camera.setLocationPath(location)
        camera.setup(attrs)
        camera.setViewportDimensions(viewportWidget.width(),
                                     viewportWidget.height())
        viewportWidget.setActiveCamera(camera)
        viewportWidget.update()
        viewportWidget.setDirty(True)

    def __setStatusBarMessage(self, statusBarMessage):
        """
        Sets the message in the status bar.

        @type statusBarMessage: C{str}
        @param statusBarMessage: The message to display in the status bar.
        """
        if statusBarMessage != self.__statusBarMessage:
            self.__statusBarMessage = statusBarMessage
            self.__statusBar.showMessage(statusBarMessage)
