# Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import QtGui, QtCore
from Katana import Nodes3DAPI, Utils, NodegraphAPI
from Nodes3DAPI import Node3D
import UI4.Widgets
from UI4.Widgets.SceneGraphView.ColumnDataType import ColumnDataType
import UI4.Util.IconManager
from UI4.Widgets.SceneGraphView import SceneGraphViewIconManager
from UI4.Widgets.SceneGraphView.SceneGraphLocationTranslation import \
        GetParentPath, GetLocationName, IsLocationUnderTopLevelLocation

from UI4.Widgets.SceneGraphView.Filtering import GetRegisteredEvaluators, \
        GetEvaluatorCriteria, GetEvaluatorInstance, Criteria

from Katana import FnAttribute, FnGeolib, FnGeolibServices


from UI4.Tabs import BaseTab
import re
import logging

log = logging.getLogger('Resources.Examples.Tabs.PrototypeSceneGraphTab')

class ExampleSceneGraphTab(BaseTab):

    # Initializer -------------------------------------------------------------

    def __init__(self, parent=None):
        BaseTab.__init__(self, parent)

        # Create the SceneGraphView widget
        self.__sceneGraphView = UI4.Widgets.SceneGraphView(parent=self)
        self.__sceneGraphView.addTopLevelLocation('/root')

        # Configure the SceneGraphView
        # You get "Name" column for free, just add the type of
        # this location for the SceneGraphView
        typeColumn = self.__sceneGraphView.addColumn("Type")
        typeColumn.setAttributeName("type")
        typeColumn.setIndex(1)

        # Configure the vbox layout and ask that it manages this widget
        vBoxLayout = QtGui.QVBoxLayout()
        vBoxLayout.setObjectName("vBoxLayout_ExampleTab")
        vBoxLayout.addWidget(self.__sceneGraphView.getWidget())

        self.setLayout(vBoxLayout)

        Utils.EventModule.RegisterCollapsedHandler(self.__on_node_setViewed,
                                                  "node_setViewed")

        Utils.EventModule.RegisterCollapsedHandler(self.__on_nodegraph_setCurrentTime,
                                                   "nodegraph_setCurrentTime")

    # Private Instance Functions ----------------------------------------------

    def __on_node_setViewed(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

    def __on_nodegraph_setCurrentTime(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

# -----------------------------------------------------------------------------

class LightViewExampleTab(BaseTab):

    # Initializer -------------------------------------------------------------

    def __init__(self, parent=None):
        BaseTab.__init__(self, parent)

        # Create the SceneGraphView widget
        self.__sceneGraphView = UI4.Widgets.SceneGraphView(parent=self)
        self.__sceneGraphView.addTopLevelLocation('/root/world/lgt')

        # The SGV widget should not freeze when it is hidden. This will allow
        # the locations to be cooked even when this tab is not visible.
        self.__sceneGraphView.setFrozenWhenHidden(False)

        # You get "Name" column for free, just add the Basic Light Properties
        # group
        basicLightPropertiesColumnSet = self.__sceneGraphView.addColumnSet("Basic Light Properties")
        basicLightPropertiesColumnSet.setIndex(1)

        fovColumn = basicLightPropertiesColumnSet.addColumn("FOV")
        fovColumn.setIndex(0)
        fovColumn.setAttributeName("geometry.fov")
        fovColumn.setDataType(ColumnDataType.Number)
        fovColumn.setEditable(True)

        nearColumn = basicLightPropertiesColumnSet.addColumn("Near")
        nearColumn.setIndex(1)
        nearColumn.setAttributeName("geometry.near")
        nearColumn.setDataType(ColumnDataType.Number)
        nearColumn.setEditable(True)

        farColumn = basicLightPropertiesColumnSet.addColumn("Far")
        farColumn.setIndex(2)
        farColumn.setAttributeName("geometry.far")
        farColumn.setDataType(ColumnDataType.Number)
        farColumn.setEditable(True)

        # Configure the vbox layout and ask that it manages this widget
        vBoxLayout = QtGui.QVBoxLayout()
        vBoxLayout.setObjectName("vBoxLayout_LightViewExampleTab")
        vBoxLayout.addWidget(self.__sceneGraphView.getWidget())

        self.setLayout(vBoxLayout)

        Utils.EventModule.RegisterCollapsedHandler(self.__on_node_setViewed,
                                                  "node_setViewed")

        Utils.EventModule.RegisterCollapsedHandler(self.__on_nodegraph_setCurrentTime,
                                                   "nodegraph_setCurrentTime")

    # Private Instance Functions ----------------------------------------------

    def __on_node_setViewed(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

    def __on_nodegraph_setCurrentTime(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

# -----------------------------------------------------------------------------

class TopLevelItemExampleTab(BaseTab):

    # Initializer -------------------------------------------------------------

    def __init__(self, parent=None):
        BaseTab.__init__(self, parent)

        self.__topLevelItemLineEdit = QtGui.QLineEdit(self)
        self.__topLevelItemLineEdit.setObjectName("topLevelItemLineEdit")
        self.__topLevelItemLineEdit.setToolTip('Location path to add or '
                                               'remove as a top-level item')

        self.__addTopLevelItemButton = QtGui.QPushButton("Add Top Level Item", self)
        self.__addTopLevelItemButton.setObjectName("addTopLevelItemButton")
        self.__addTopLevelItemButton.setEnabled(False)

        self.__removeTopLevelItemButton = QtGui.QPushButton("Remove Top Level Item", self)
        self.__removeTopLevelItemButton.setObjectName("removeTopLevelItemButton")
        self.__removeTopLevelItemButton.setEnabled(False)

        self.__printSelectedLocationsButton = QtGui.QPushButton("Print Selected Locations", self)
        self.__printSelectedLocationsButton.setObjectName("printSelectedLocationsButton")

        # Set up signal/slot connections
        QtCore.QObject.connect(self.__topLevelItemLineEdit,
                               QtCore.SIGNAL('textChanged(QString)'),
                               self.on_topLevelItemLineEdit_textChanged)
        QtCore.QObject.connect(self.__addTopLevelItemButton,
                               QtCore.SIGNAL('clicked()'),
                               self.on_addTopLevelItemButton_clicked)
        QtCore.QObject.connect(self.__removeTopLevelItemButton,
                               QtCore.SIGNAL('clicked()'),
                               self.on_removeTopLevelItemButton_clicked)
        QtCore.QObject.connect(self.__printSelectedLocationsButton,
                               QtCore.SIGNAL('clicked()'),
                               self.on_printSelectedLocationsButton_clicked)

        hBoxLayout = QtGui.QHBoxLayout()
        hBoxLayout.addWidget(self.__addTopLevelItemButton, 1)
        hBoxLayout.addWidget(self.__removeTopLevelItemButton, 1)
        hBoxLayout.addWidget(self.__printSelectedLocationsButton, 1)

        # Create the SceneGraphView widget
        self.__sceneGraphView = UI4.Widgets.SceneGraphView(parent=self)
        self.__sceneGraphView.addTopLevelLocation('/root')

        # The SGV widget should not freeze when it is hidden. This will allow
        # the locations to be cooked even when this tab is not visible.
        self.__sceneGraphView.setFrozenWhenHidden(False)

        # Configure the SceneGraphView
        typeColumn = self.__sceneGraphView.addColumn("Type")
        typeColumn.setAttributeName("type")
        typeColumn.setIndex(1)

        # Configure the vbox layout and ask that it manages this widget
        vBoxLayout = QtGui.QVBoxLayout()
        vBoxLayout.setObjectName("vBoxLayout_TopLevelItemExampleTab")
        vBoxLayout.addWidget(self.__sceneGraphView.getWidget())
        vBoxLayout.addWidget(self.__topLevelItemLineEdit)
        self.setLayout(vBoxLayout)

        self.layout().addLayout(hBoxLayout)

        # Set up event callbacks for drag and drop
        self.__sceneGraphView.setDragMoveEventCallback(
            self.__dragMoveEventCallback)
        self.__sceneGraphView.setDropEventCallback(self.__dropEventCallback)

        # Register event handlers
        Utils.EventModule.RegisterCollapsedHandler(
            self.__on_node_setViewed, "node_setViewed")

        Utils.EventModule.RegisterCollapsedHandler(
            self.__on_nodegraph_setCurrentTime, "nodegraph_setCurrentTime")

        Utils.EventModule.RegisterEventHandler(
            self.__on_gaffer_locationRenamed, "gaffer_locationRenamed")

    # Slot Functions ----------------------------------------------------------

    def on_topLevelItemLineEdit_textChanged(self, text):
        locationPath = str(text).strip()
        isLocationPathValid = (locationPath.startswith('/')
                               and not ' ' in locationPath)

        self.__addTopLevelItemButton.setEnabled(isLocationPathValid)
        self.__removeTopLevelItemButton.setEnabled(isLocationPathValid)

    def on_addTopLevelItemButton_clicked(self):
        locationPath = str(self.__topLevelItemLineEdit.text()).strip()
        self.__sceneGraphView.addTopLevelLocation(locationPath)

    def on_removeTopLevelItemButton_clicked(self):
        locationPath = str(self.__topLevelItemLineEdit.text()).strip()
        self.__sceneGraphView.removeTopLevelLocation(locationPath)

    def on_printSelectedLocationsButton_clicked(self):
        selectedLocations = self.__sceneGraphView.getSelectedLocations()
        if selectedLocations is not None:
            for topLevelLocationPath, locationPath in selectedLocations:
                print("    %s: %s" % (topLevelLocationPath, locationPath))

    # Private Instance Functions ----------------------------------------------

    def __on_node_setViewed(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

    def __on_nodegraph_setCurrentTime(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

    def __on_gaffer_locationRenamed(self, eventType, index, oldLocationPath,
                                    newLocationPath):
        """
        Update any top-level items that this rename event will have affected -
        i.e. the location renamed, and any top-level items beneath it in the
        hierarchy.
        """
        topLevelLocations = list(self.__sceneGraphView.getTopLevelLocations())
        for topLevelLocation in topLevelLocations:
            if topLevelLocation == oldLocationPath:
                self.__sceneGraphView.topLevelLocationRenamed(oldLocationPath,
                                                              newLocationPath)
            elif IsLocationUnderTopLevelLocation(topLevelLocation,
                                                 oldLocationPath):
                newChildPath = topLevelLocation.replace(oldLocationPath,
                                                        newLocationPath)
                self.__sceneGraphView.topLevelLocationRenamed(topLevelLocation,
                                                              newChildPath)

    # Private Callbacks -------------------------------------------------------

    def __dragMoveEventCallback(self, dragMoveEvent, draggedItems, parentItem,
                                childItemIndex):
        """
        Callback of the SceneGraphView widget that is called when items are
        dragged over the SceneGraphView widget in order to determine whether
        the dragged items could be dropped onto the widget.

        Accepts the dragged items if they are dragged below the last of the
        available top-level locations, but only if at least one item is dragged
        that doesn't already appear as a top-level location.

        By default, the given C{dragMoveEvent} is ignored, meaning that dragged
        items cannot be dropped. If you want to allow dragged items to be
        dropped, call C{accept()} on the given C{dragMoveEvent}.

        @type dragMoveEvent: C{QtGui.QDragMoveEvent}
        @type draggedItems: C{list} of C{LocationItem}
        @type parentItem: C{QtGui.QTreeWidgetItem} or C{None}
        @type childItemIndex: C{int}
        @param dragMoveEvent: Provides details about the drag operation in
            progress.
        @param draggedItems: A list of specialized C{QtGui.QTreeWidgetItem}
            instances that represent the items that are being dragged over the
            widget.
        @param parentItem: A specialized C{QtGui.QTreeWidgetItem} instance that
            represents an item in the SceneGraphView widget that contains the
            item over which the items are being dragged, or C{None} if items
            are being dragged over empty space in the SceneGraphView widget.
        @param childItemIndex: The index of the item under the mouse pointer
            underneath the given C{parentItem}.
        """
        topLevelLocationPaths = self.__sceneGraphView.getTopLevelLocations()
        if parentItem is None and childItemIndex >= len(topLevelLocationPaths):
            for draggedItem in draggedItems:
                locationPath = draggedItem.getLocationPath()
                if not locationPath in topLevelLocationPaths:
                    dragMoveEvent.accept()
                    break

    def __dropEventCallback(self, dropEvent, droppedItems, parentItem,
                            childItemIndex):
        """
        Callback of the SceneGraphView widget that is called when dragged items
        are dropped over the widget, triggering custom behavior depending on
        the number and type of the dragged items and the item onto which the
        dragged items have been dropped.

        Adds new top-level locations to the SceneGraphView widget based on the
        location paths of the dropped items.

        @type dropEvent: C{QtGui.QDropEvent}
        @type droppedItems: C{list} of C{LocationItem}
        @type parentItem: C{QtGui.QTreeWidgetItem} or C{None}
        @type childItemIndex: C{int}
        @param dropEvent: Provides details about the drop operation.
        @param droppedItems: A list of specialized C{QtGui.QTreeWidgetItem}
            instances that represent the items that have been been dropped onto
            the widget.
        @param parentItem: A specialized C{QtGui.QTreeWidgetItem} instance that
            represents an item in the SceneGraphView widget that contains the
            item over which the dragged items have been dropped, or C{None} if
            items have been dropped over empty space in the SceneGraphView
            widget.
        @param childItemIndex: The index of the item under the mouse pointer
            underneath the given C{parentItem}.
        """
        topLevelLocationPaths = self.__sceneGraphView.getTopLevelLocations()
        for droppedItem in droppedItems:
            locationPath = droppedItem.getLocationPath()
            if not locationPath in topLevelLocationPaths:
                self.__sceneGraphView.addTopLevelLocation(locationPath)
        dropEvent.accept()

# -----------------------------------------------------------------------------

class DebugExampleTab(BaseTab):

    # Initializer -------------------------------------------------------------

    def __init__(self, parent=None):
        BaseTab.__init__(self, parent)

        # Add Debug controls to this hbox.
        hBoxLayout = QtGui.QHBoxLayout()

        # Create the SceneGraphView widget
        self.__sceneGraphView = UI4.Widgets.SceneGraphView(debugMode=True,
                                                           parent=self)
        self.__sceneGraphView.addTopLevelLocation('/root')

        # The SGV widget should not freeze when it is hidden. This will allow
        # the locations to be cooked even when this tab is not visible.
        self.__sceneGraphView.setFrozenWhenHidden(False)

        # Configure the SceneGraphView
        typeColumn = self.__sceneGraphView.addColumn("Type")
        typeColumn.setAttributeName("type")
        typeColumn.setIndex(1)

        # Configure the vbox layout and ask that it manages this widget
        vBoxLayout = QtGui.QVBoxLayout()
        vBoxLayout.setObjectName("vBoxLayout_DebugExampleTab")
        vBoxLayout.addWidget(self.__sceneGraphView.getWidget())

        self.setLayout(vBoxLayout)

        self.layout().addLayout(hBoxLayout)

        Utils.EventModule.RegisterCollapsedHandler(self.__on_node_setViewed,
                                                  "node_setViewed")

        Utils.EventModule.RegisterCollapsedHandler(self.__on_nodegraph_setCurrentTime,
                                                   "nodegraph_setCurrentTime")

    # Private Instance Functions ----------------------------------------------

    def __on_node_setViewed(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

    def __on_nodegraph_setCurrentTime(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

# -----------------------------------------------------------------------------

class GetSceneGraphAttributesExampleTab(BaseTab):

    # Initializer -------------------------------------------------------------

    def __init__(self, parent=None):
        BaseTab.__init__(self, parent)

        self.__locationLineEdit = QtGui.QLineEdit(self)
        self.__withCallbackCheckBox = QtGui.QCheckBox("With Callback", self)
        self.__oneShotCheckBox = QtGui.QCheckBox("One Shot", self)
        self.__getAttributeAtLocationButton = QtGui.QPushButton("Get Attribute at Location", self)

        # Configure the widgets
        self.__locationLineEdit.setObjectName("locationLineEdit")
        self.__withCallbackCheckBox.setObjectName("withCallbackCheckBox")
        self.__oneShotCheckBox.setObjectName("oneShotCheckBox")
        self.__getAttributeAtLocationButton.setObjectName("getAttributeAtLocationButton")

        self.connect(self.__getAttributeAtLocationButton, QtCore.SIGNAL('clicked()'), self.on_getAttributeAtLocationButton_clicked)

        hBoxLayout = QtGui.QHBoxLayout()
        hBoxLayout.addWidget(self.__getAttributeAtLocationButton, 1)

        # Create the SceneGraphView widget
        self.__sceneGraphView = UI4.Widgets.SceneGraphView(parent=self)
        self.__sceneGraphView.addTopLevelLocation('/root')

        # The SGV widget should not freeze when it is hidden. This will allow
        # the locations to be cooked even when this tab is not visible.
        self.__sceneGraphView.setFrozenWhenHidden(False)

        # Configure the SceneGraphView
        typeColumn = self.__sceneGraphView.addColumn("Type")
        typeColumn.setAttributeName("type")
        typeColumn.setIndex(1)

        # Configure the vbox layout and ask that it manages this widget
        vBoxLayout = QtGui.QVBoxLayout()
        vBoxLayout.setObjectName("vBoxLayout_GetSceneGraphAttributesExampleTab")
        vBoxLayout.addWidget(self.__sceneGraphView.getWidget())

        inputLayout = QtGui.QHBoxLayout()
        inputLayout.setObjectName("inputLayout")
        inputLayout.addWidget(self.__locationLineEdit)
        inputLayout.addWidget(self.__withCallbackCheckBox)
        inputLayout.addWidget(self.__oneShotCheckBox)

        self.setLayout(vBoxLayout)
        self.layout().addLayout(inputLayout)
        self.layout().addLayout(hBoxLayout)

        Utils.EventModule.RegisterCollapsedHandler(self.__on_node_setViewed,
                                                  "node_setViewed")

        Utils.EventModule.RegisterCollapsedHandler(self.__on_nodegraph_setCurrentTime,
                                                   "nodegraph_setCurrentTime")

    # Slot Functions ----------------------------------------------------------

    def on_getAttributeAtLocationButton_clicked(self):
        locationPath = str(self.__locationLineEdit.text())
        withCallback = self.__withCallbackCheckBox.isChecked()
        oneShot = self.__oneShotCheckBox.isChecked()

        if not locationPath.startswith('/'):
            return

        if withCallback:
            if oneShot:
                attributes = self.__sceneGraphView.getSceneGraphAttributes(
                    locationPath, self.__oneShotSceneGraphAttributeCallback,
                    True)
            else:
                attributes = self.__sceneGraphView.getSceneGraphAttributes(
                    locationPath, self.__sceneGraphAttributeCallback, False)
        else:
            attributes = self.__sceneGraphView.getSceneGraphAttributes(
                locationPath)

        if attributes is None:
            if withCallback:
                log.info('Attributes for "%s" not available immediately. '
                         'The given callback will be called when they are '
                         'ready.' % locationPath)
            else:
                log.info('Attributes for "%s" not available immediately. '
                         'Please call again later.' % locationPath)
            return

        if attributes is not None:
            if oneShot:
                log.info('Attributes for "%s" available immediately. No '
                         'further updates. Attribute data:\n%s'
                         % (locationPath, attributes.getXML()))
            else:
                log.info('Attributes for "%s" available immediately. Further '
                         'updates will be delivered. Attribute data:\n%s'
                         % (locationPath, attributes.getXML()))

    # Private Instance Functions ----------------------------------------------

    def __on_node_setViewed(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

    def __on_nodegraph_setCurrentTime(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

    def __oneShotSceneGraphAttributeCallback(self, location, attribute):
        log.info("One Shot: Attributes for %s weren't available immediately but have now returned with:\n%s"
                 % (location, attribute.getXML()))

    def __sceneGraphAttributeCallback(self, location, attribute):
        log.info("Continuous: Attributes for %s weren't available immediately but have now returned with:\n%s"
                 % (location, attribute.getXML()))

        # Would you like to be called again?
        callAgainMessageBox = QtGui.QMessageBox(self)
        callAgainMessageBox.setText('Continue receiving attribute updates?')
        callAgainMessageBox.setInformativeText('Would you like to continue to receieve attribute updates for "%s"?' % location)
        callAgainMessageBox.setStandardButtons(QtGui.QMessageBox.Yes | QtGui.QMessageBox.No)
        callAgainMessageBox.setDefaultButton(QtGui.QMessageBox.Yes)
        callAgainMessageBox.setIcon(QtGui.QMessageBox.Question)

        return callAgainMessageBox.exec_() == QtGui.QMessageBox.Yes

# -----------------------------------------------------------------------------

class ContextMenuExampleTab(BaseTab):

    # Initializer -------------------------------------------------------------

    def __init__(self, parent=None):
        BaseTab.__init__(self, parent)

        # Create the SceneGraphView widget
        self.__sceneGraphView = UI4.Widgets.SceneGraphView(parent=self)
        self.__sceneGraphView.addTopLevelLocation('/root')

        # The SGV widget should not freeze when it is hidden. This will allow
        # the locations to be cooked even when this tab is not visible.
        self.__sceneGraphView.setFrozenWhenHidden(False)

        # Configure the SceneGraphView
        # You get "Name" column for free, just add the type of
        # this location for the SceneGraphView
        typeColumn = self.__sceneGraphView.addColumn("Type")
        typeColumn.setAttributeName("type")
        typeColumn.setIndex(1)

        self.__sceneGraphView.setContextMenuEventCallback(
            self.__contextMenuEventCallback)

        # Configure the vbox layout and ask that it manages this widget
        vBoxLayout = QtGui.QVBoxLayout()
        vBoxLayout.setObjectName("vBoxLayout_ContextMenuExampleTab")
        vBoxLayout.addWidget(self.__sceneGraphView.getWidget())

        self.setLayout(vBoxLayout)

        Utils.EventModule.RegisterCollapsedHandler(self.__on_node_setViewed,
                                                  "node_setViewed")

        Utils.EventModule.RegisterCollapsedHandler(self.__on_nodegraph_setCurrentTime,
                                                   "nodegraph_setCurrentTime")

    # Private Instance Functions ----------------------------------------------

    def __on_node_setViewed(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

    def __on_nodegraph_setCurrentTime(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

    def __inspectXmlAttributesActionTriggered(self):
        # Get the attributes, we know they're available because we clicked on
        # the location
        selectedLocations = self.__sceneGraphView.getSelectedLocations()
        locationPath = selectedLocations[0][1]
        attributeXML = self.__sceneGraphView.getSceneGraphAttributes(
            locationPath).getXML()

        messageBox = QtGui.QMessageBox(self)
        messageBox.setWindowTitle('Inspect XML Attributes')
        messageBox.setText('Click <b>Show Details...</b> to view scene graph '
                           'attribute data at <tt>%s</tt>.' % locationPath)
        messageBox.setDetailedText(attributeXML)
        messageBox.setStandardButtons(QtGui.QMessageBox.Close)
        messageBox.setDefaultButton(QtGui.QMessageBox.Close)
        messageBox.setIcon(QtGui.QMessageBox.Information)

        # Workaround to get QMessageBox to display a nice size
        horizontalSpacer = QtGui.QSpacerItem(640, 0, QtGui.QSizePolicy.Minimum,
                                             QtGui.QSizePolicy.Expanding)
        layout = messageBox.layout()
        layout.addItem(horizontalSpacer, layout.rowCount(), 0, 1,
                       layout.columnCount())

        messageBox.exec_()

    def __pruneSelectedActionTriggered(self):
        selectedLocations = self.__sceneGraphView.getSelectedLocations()
        locationPath = selectedLocations[0][1]
        currentTime = NodegraphAPI.GetCurrentTime()
        viewNode = NodegraphAPI.GetViewNode()

        Utils.UndoStack.OpenGroup('Prune "%s"' % locationPath)
        try:
            # Create and configure a Prune node to prune the selected location
            pruneNode = NodegraphAPI.CreateNode("Prune",
                                                NodegraphAPI.GetRootNode())
            pruneNode.getInputPortByIndex(0).connect(
                viewNode.getOutputPortByIndex(0))
            celParameter = pruneNode.getParameter('cel')
            celParameter.setValue(locationPath, currentTime)

            NodegraphAPI.SetNodeViewed(pruneNode, True, True)

            # Position the prune node underneath the view node
            viewNodePosition = NodegraphAPI.GetNodePosition(viewNode)
            NodegraphAPI.SetNodePosition(pruneNode, (viewNodePosition[0],
                                                     viewNodePosition[1] - 54))
        finally:
            Utils.UndoStack.CloseGroup()

    def __contextMenuEventCallback(self, contextMenuEvent, menu):
        # Add an option to inspect this location if there is only a single
        # location in the selection state.
        if len(contextMenuEvent.getSelectedLocations()) == 1:
            menu.addAction("Inspect XML Attributes...",
                           self.__inspectXmlAttributesActionTriggered)

            if contextMenuEvent.getLocationPath() != "/root":
                menu.addSeparator()
                menu.addAction("Prune Selected",
                               self.__pruneSelectedActionTriggered)

# -----------------------------------------------------------------------------

class ColumnTestExampleTab(BaseTab):

    # Initializer -------------------------------------------------------------

    def __init__(self, parent=None):
        """
        Initializes an instance of the class.

        @type parent: C{QtGui.QWidget} or C{None}
        @param parent: The widget to use as the frame's parent, or C{None} if
            the new instance is to be owned by PyQt instead of Qt.
        """
        BaseTab.__init__(self, parent)

        # Create widgets
        self.__showColumnsButton = QtGui.QPushButton("Show Columns", self)
        self.__showColumnsButton.setObjectName("showColumnsButton")
        self.__hideColumnsButton = QtGui.QPushButton("Hide Columns", self)
        self.__hideColumnsButton.setObjectName("hideColumnsButton")
        self.__showInfoButton = QtGui.QPushButton("Show Info", self)
        self.__showInfoButton.setObjectName("showInfoButton")

        # Set up signal/slot connections
        self.connect(self.__showColumnsButton, QtCore.SIGNAL('clicked()'),
                     self.on_showColumnsButton_clicked)
        self.connect(self.__hideColumnsButton, QtCore.SIGNAL('clicked()'),
                     self.on_hideColumnsButton_clicked)
        self.connect(self.__showInfoButton, QtCore.SIGNAL('clicked()'),
                     self.on_showInfoButton_clicked)

        # Create a layout for the buttons
        hBoxLayout = QtGui.QHBoxLayout()
        hBoxLayout.setObjectName('ColumnTestExample_hBoxLayout')
        hBoxLayout.addWidget(self.__showColumnsButton, 1)
        hBoxLayout.addWidget(self.__hideColumnsButton, 1)
        hBoxLayout.addWidget(self.__showInfoButton, 1)

        self.__columnSetToShowAndHide = ""

        # Create the SceneGraphView widget
        self.__sceneGraphView = UI4.Widgets.SceneGraphView(parent=self)
        self.__sceneGraphView.addTopLevelLocation('/root/world/lgt')

        # The SGV widget should not freeze when it is hidden. This will allow
        # the locations to be cooked even when this tab is not visible.
        self.__sceneGraphView.setFrozenWhenHidden(False)

        # You get the "Name" column for free.
        # We must create all the other columns.

        # Make one set to encompass all the light properties
        self.__allLightPropertiesSet = self.__sceneGraphView.addColumnSet("All Light Properties")

        # Basic Light Properties
        basicLightPropertiesColumnSet = self.__allLightPropertiesSet.addColumnSet("Basic Light Properties")

        # Add a shader column to the SceneGraphView
        shaderColumn = basicLightPropertiesColumnSet.addColumn("Shader")
        shaderColumn.setAttributeName("material.prmanLightShader")

        # Sub-set containing some icon columns
        iconsPropertiesColumnSet = basicLightPropertiesColumnSet.addColumnSet("Icons")

        # TODO: not currently linked to the mute/solo parameters in the Gaffer
        muteColumn = iconsPropertiesColumnSet.addColumn("Mute")
        muteColumn.setTitleIconName("muteHeader")
        muteColumn.setTitleStyle("iconOnly")
        muteColumn.setEditable(True)

        soloColumn = iconsPropertiesColumnSet.addColumn("Solo")
        soloColumn.setTitleIconName("soloHeader")
        soloColumn.setTitleStyle("iconOnly")
        soloColumn.setEditable(True)

        fovColumn = basicLightPropertiesColumnSet.addColumn("FOV")
        fovColumn.setAttributeName("geometry.fov")
        fovColumn.setDataType(ColumnDataType.Number)
        fovColumn.setEditable(False)

        nearColumn = basicLightPropertiesColumnSet.addColumn("Near")
        nearColumn.setAttributeName("geometry.near")
        nearColumn.setDataType(ColumnDataType.Number)
        nearColumn.setEditable(True)

        farColumn = basicLightPropertiesColumnSet.addColumn("Far")
        farColumn.setAttributeName("geometry.far")
        farColumn.setDataType(ColumnDataType.Number)
        farColumn.setEditable(True)

        # Advanced Light Properties
        advancedLightPropertiesColumnSet = self.__allLightPropertiesSet.addColumnSet("Advanced Light Properties")

        radiusColumn = advancedLightPropertiesColumnSet.addColumn("Radius")
        radiusColumn.setAttributeName("geometry.radius")
        radiusColumn.setDataType(ColumnDataType.Number)
        radiusColumn.setEditable(True)

        coiColumn = advancedLightPropertiesColumnSet.addColumn("COI")
        coiColumn.setAttributeName("geometry.centerOfInterest")
        coiColumn.setDataType(ColumnDataType.Number)
        coiColumn.setEditable(True)

        self.__columnSetToShowAndHide = advancedLightPropertiesColumnSet.getName()
        self.__showColumnsButton.setText('Show %s'
                                         % self.__columnSetToShowAndHide)
        self.__hideColumnsButton.setText('Hide %s'
                                         % self.__columnSetToShowAndHide)

        # Set up keyboard shortcuts for column configuration
        presetManager = self.__sceneGraphView.getColumnPresetManager()

        preset1 = presetManager.addPreset(QtCore.Qt.Key_1)
        preset1.addShown(muteColumn)
        preset1.addShown(soloColumn)

        preset2 = presetManager.addPreset(QtCore.Qt.Key_2)
        preset2.addShown(muteColumn)
        preset2.addHidden(soloColumn)

        preset3 = presetManager.addPreset(QtCore.Qt.Key_3)
        preset3.addHidden(muteColumn)
        preset3.addShown(soloColumn)

        preset4 = presetManager.addPreset(QtCore.Qt.Key_4)
        preset4.addToggled(fovColumn)

        # Configure the vbox layout and ask that it manages our widget
        vBoxLayout = QtGui.QVBoxLayout()
        vBoxLayout.setObjectName("ColumnTestExample_vBoxLayout")
        vBoxLayout.addWidget(self.__sceneGraphView.getWidget())
        vBoxLayout.addLayout(hBoxLayout)

        self.setLayout(vBoxLayout)

        Utils.EventModule.RegisterCollapsedHandler(self.__on_node_setViewed,
                                                  "node_setViewed")

        Utils.EventModule.RegisterCollapsedHandler(self.__on_nodegraph_setCurrentTime,
                                                   "nodegraph_setCurrentTime")

    # Slot Functions ----------------------------------------------------------

    def on_showColumnsButton_clicked(self):
        self.__sceneGraphView.showColumnSet(self.__columnSetToShowAndHide)

    def on_hideColumnsButton_clicked(self):
        self.__sceneGraphView.hideColumnSet(self.__columnSetToShowAndHide)

    def on_showInfoButton_clicked(self):
        # Iterate over all visible columns as a list
        log.info('ColumnTestExampleTab: Visible columns, iterated over as a '
                 'list:')
        columns = self.__allLightPropertiesSet.getColumns(visibleOnly=True)
        for i, column in enumerate(columns):
            log.info('ColumnTestExampleTab: Column %d, "%s", parent set: "%s"'
                     % (i, column.getName(), column.getParentSet().getName()))

        # Iterate over all visible columns by index
        log.info('ColumnTestExampleTab: Visible columns, iterated over by index:')
        numColumns = self.__allLightPropertiesSet.getColumnCount(
            visibleOnly=True)
        for i in xrange(numColumns):
            column = self.__allLightPropertiesSet.getColumnByIndex(
                i, visibleOnly=True)
            log.info('ColumnTestExampleTab: Column %d, "%s", parents: ["%s"]'
                     % (i, column.getName(),
                        '", "'.join(column.getParentSetNames())))

    # Instance Functions ------------------------------------------------------

    def getSettings(self):
        """
        @rtype: C{dict}
        @return: A dictionary of settings that should be stored and restored as
            part of a tab layout.
        """
        return {"columnVisibilitySettings":
                self.__sceneGraphView.getColumnVisibilitySettings()}

    def applySettings(self, settings):
        """
        Applies the given settings to the tab.

        @type settings: C{dict}
        @param settings: A dictionary of settings to apply to this tab.
        """
        if "columnVisibilitySettings" in settings:
            self.__sceneGraphView.applyColumnVisibilitySettings(settings["columnVisibilitySettings"])

    # Private Instance Functions ----------------------------------------------

    def __on_node_setViewed(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

    def __on_nodegraph_setCurrentTime(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

# -----------------------------------------------------------------------------

class FilteringExampleTab(BaseTab):

    # Initializer -------------------------------------------------------------

    def __init__(self, parent=None):
        QtGui.QFrame.__init__(self, parent)

        # Create and configure the widgets
        self.__ruleDialogButton = QtGui.QPushButton("Rule Dialog", self)
        self.__ruleDialogButton.setObjectName("ruleDialogButton")
        self.connect(self.__ruleDialogButton, QtCore.SIGNAL('clicked()'), self.on_ruleDialogButton_clicked)

        # Create the SceneGraphView widget
        self.__sceneGraphView = UI4.Widgets.SceneGraphView(parent=self)
        self.__sceneGraphView.addTopLevelLocation('/root')

        # The SGV widget should not freeze when it is hidden. This will allow
        # the locations to be cooked even when this tab is not visible.
        self.__sceneGraphView.setFrozenWhenHidden(False)

        # Configure the SceneGraphView
        # You get "Name" column for free, just add the type of
        # this location for the SceneGraphView
        typeColumn = self.__sceneGraphView.addColumn("Type")
        typeColumn.setAttributeName("type")
        typeColumn.setIndex(1)
        typeColumn.setEditable(False)

        fovColumn = self.__sceneGraphView.addColumn("FOV")
        fovColumn.setAttributeName("geometry.fov")
        fovColumn.setEditable(False)

        # Configure the widget layouts
        inputLayout = QtGui.QHBoxLayout()
        inputLayout.setObjectName("inputLayout")
        inputLayout.addWidget(self.__ruleDialogButton)

        vBoxLayout = QtGui.QVBoxLayout()
        vBoxLayout.setObjectName("vBoxLayout_FilteringExampleTab")
        vBoxLayout.addWidget(self.__sceneGraphView.getWidget())
        vBoxLayout.addLayout(inputLayout)

        self.setLayout(vBoxLayout)

        Utils.EventModule.RegisterCollapsedHandler(self.__on_node_setViewed,
                                                  "node_setViewed")

        Utils.EventModule.RegisterCollapsedHandler(self.__on_nodegraph_setCurrentTime,
                                                   "nodegraph_setCurrentTime")

    # Slot Functions ----------------------------------------------------------

    def on_ruleDialogButton_clicked(self):
        self.__sceneGraphView.showRuleConfigDialog()

    # Private Instance Functions ----------------------------------------------

    def __on_node_setViewed(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

    def __on_nodegraph_setCurrentTime(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

# -----------------------------------------------------------------------------

class NamePolishingExampleTab(BaseTab):

    # Initializer -------------------------------------------------------------

    def __init__(self, parent=None):
        QtGui.QFrame.__init__(self, parent)

        # Create the SceneGraphView widget
        self.__sceneGraphView = UI4.Widgets.SceneGraphView(parent=self)
        self.__sceneGraphView.addTopLevelLocation('/root')

        # The SGV widget should not freeze when it is hidden. This will allow
        # the locations to be cooked even when this tab is not visible.
        self.__sceneGraphView.setFrozenWhenHidden(False)

        # Configure the SceneGraphView
        typeColumn = self.__sceneGraphView.addColumn("Type")
        typeColumn.setAttributeName("type")

        self.__sceneGraphView.setLocationNamePolishCallback(
            self.__locationNamePolishCallback)

        # Create widgets
        self.__polishLocationNamesCheckBox = QtGui.QCheckBox("Polish Location Names", self)
        self.__polishLocationNamesCheckBox.setChecked(self.__sceneGraphView.locationNamesPolished())
        self.__modeAButton = QtGui.QPushButton("Normal", self)
        self.__modeBButton = QtGui.QPushButton("Camel Case", self)
        self.__modeCButton = QtGui.QPushButton("Dave", self)

        # Configure widgets
        self.__polishLocationNamesCheckBox.setObjectName("polishLocationNamesCheckBox")
        self.__modeAButton.setObjectName("modeAButton")
        self.__modeBButton.setObjectName("modeBButton")
        self.__modeCButton.setObjectName("modeCButton")

        self.connect(self.__polishLocationNamesCheckBox, QtCore.SIGNAL('clicked()'), self.on_polishLocationNamesCheckbox_clicked)
        self.connect(self.__modeAButton, QtCore.SIGNAL('clicked()'), self.on_modeAButton_clicked)
        self.connect(self.__modeBButton, QtCore.SIGNAL('clicked()'), self.on_modeBButton_clicked)
        self.connect(self.__modeCButton, QtCore.SIGNAL('clicked()'), self.on_modeCButton_clicked)

        hBoxLayout = QtGui.QHBoxLayout()
        hBoxLayout.addWidget(self.__polishLocationNamesCheckBox, 1, QtCore.Qt.AlignCenter)
        hBoxLayout.addWidget(self.__modeAButton, 1)
        hBoxLayout.addWidget(self.__modeBButton, 1)
        hBoxLayout.addWidget(self.__modeCButton, 1)

        # Configure the vbox layout and ask that it manages this widget
        vBoxLayout = QtGui.QVBoxLayout()
        vBoxLayout.setObjectName("vBoxLayout_NamePolishingExampleTab")
        vBoxLayout.addWidget(self.__sceneGraphView.getWidget())
        vBoxLayout.addLayout(hBoxLayout)

        self.setLayout(vBoxLayout)

        Utils.EventModule.RegisterCollapsedHandler(self.__on_node_setViewed,
                                                  "node_setViewed")

        Utils.EventModule.RegisterCollapsedHandler(self.__on_nodegraph_setCurrentTime,
                                                   "nodegraph_setCurrentTime")

    # Slot Functions ----------------------------------------------------------

    def on_polishLocationNamesCheckbox_clicked(self):
        self.__sceneGraphView.setLocationNamesPolished(
            self.__polishLocationNamesCheckBox.isChecked())

    def on_modeAButton_clicked(self):
        self.__sceneGraphView.setLocationNamePolishCallback(
            self.__locationNamePolishCallback)

    def on_modeBButton_clicked(self):
        self.__sceneGraphView.setLocationNamePolishCallback(
            self.__camelLocationNamePolishCallback)

    def on_modeCButton_clicked(self):
        self.__sceneGraphView.setLocationNamePolishCallback(
            self.__daveLocationNamePolishCallback)

    # Private Instance Functions ----------------------------------------------

    def __on_node_setViewed(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

    def __on_nodegraph_setCurrentTime(self, args):
        self.__sceneGraphView.setViewNode(NodegraphAPI.GetViewNode())

    def __locationNamePolishCallback(self, locationPath, topLevelLocationPath):
        """
        This callback takes a full path to a scene graph location and returns
        a polished version of the path for display in the tab.

        In this case, if the name of the location is prefixed with the name of
        its parent, then we remove the parent name and any separating characters
        (in this case underscores) and replace the prefix with "...".

        e.g. "/root/world/lgt/charA/charA_keyLight" --> "...keyLight"

        @type locationPath: C{str}
        @type topLevelLocationPath: C{str}
        @rtype: C{str}
        @param locationPath: The full path to a scene graph location whose
            name we wish to polish,
            e.g. "/root/world/lgt/charA/charA_keyLight".
        @param topLevelLocationPath: The full path to the top-level location
            under which C{locationPath} is located.
        @return: The polished name for the location, e.g. "...keyLight".
        """
        locationName = GetLocationName(locationPath)

        if locationPath == topLevelLocationPath:
            return locationName

        parentPath = GetParentPath(locationPath)
        if parentPath is None:
            return locationName

        parentName = GetLocationName(parentPath)
        if self.__parentIsPrefixOfChild(parentName, locationName):
            polishedName = locationName.replace(parentName, '', 1).strip('_')
            if polishedName is not None:
                return "...%s" % polishedName

        return locationName

    def __parentIsPrefixOfChild(self, parentName, childName, separators='_'):
        """
        Determine whether parentName is intended as a prefix of childName, by
        checking whether parentName appears as a whole word at the start of
        childName. Word boundaries are defined here by use of camelCase, or
        any character in the separators string parameter.

        (charA,  charA)        -> False
        (charA,  charAlpha)    -> False
        (charA1, charA101)     -> False

        (charA,  charAA)       -> True
        (charA,  charA_a)      -> True
        (charA,  charAAlpha)   -> True

        @type parentName: C{str}
        @type childName: C{str}
        @type separators: C{str}
        @rtype: C{bool}
        @param parentName: The name of the parent location
        @param childName: The name of the child location
        @param separators: A string containing characters to be considered as 
            valid separators between words in location names, e.g. '_'.
        @return: A value indicating whether parentName is a whole-word prefix
            of childName.
        """
        if parentName is None or childName is None or parentName == childName:
            return False

        # Is childName of the form: [parentName][separators][not separators]?
        expression = '{0}[{1}]+[^{1}]+'.format(parentName, separators)
        if re.match(expression, childName):
            return True

        # Check to see if there is camelCase separation from the parent.
        # This will permit (char, charA) but not (char, character),
        # and (charA1, charA1Shot01) but not (charA1, charA101).
        splitChild = self.__splitByCamelCase(childName)
        splitParent = self.__splitByCamelCase(parentName)

        if splitChild[:len(splitParent)] == splitParent:
            return True

        return False

    def __splitByCamelCase(self, text):
        separator = "."
        s1 = re.sub('(.)([A-Z][a-z]+)', r'\1%s\2' % separator, text)
        s2 = re.sub('([a-z0-9])([A-Z])', r'\1%s\2' % separator, s1)
        return s2.split(separator)

    def __camelLocationNamePolishCallback(self, locationPath,
                                          topLevelLocationPath):
        locationName = GetLocationName(locationPath)
        splitName = self.__splitByCamelCase(locationName)
        return re.sub('[_ ]+', ' ', ' '.join(splitName)).title()

    def __daveLocationNamePolishCallback(self, locationPath,
                                         topLevelLocationPath):
        return 'dave'


# -----------------------------------------------------------------------------

class OpViewExampleTab(BaseTab):

    LocationsToCreate = ["/root/world/geo/foo", "/root/world/geo/bar"]

    # Initializer -------------------------------------------------------------

    def __init__(self, parent=None):
        BaseTab.__init__(self, parent)

        # Add Debug controls to this hbox.
        hBoxLayout = QtGui.QHBoxLayout()

        # Create the SceneGraphView widget
        self.__sceneGraphView = UI4.Widgets.SceneGraphView(parent=self)
        self.__sceneGraphView.addTopLevelLocation('/root')
        op = self.__createTestOp()
        self.__sceneGraphView.setViewOp(op)

        # The SGV widget should not freeze when it is hidden. This will allow
        # the locations to be cooked even when this tab is not visible.
        self.__sceneGraphView.setFrozenWhenHidden(False)

        # Configure the SceneGraphView
        typeColumn = self.__sceneGraphView.addColumn("Type")
        typeColumn.setAttributeName("type")

        # Configure the vbox layout and ask that it manages this widget
        vBoxLayout = QtGui.QVBoxLayout()
        vBoxLayout.setObjectName("vBoxLayout_OpViewExampleTab")
        vBoxLayout.addWidget(self.__sceneGraphView.getWidget())

        self.setLayout(vBoxLayout)

        self.layout().addLayout(hBoxLayout)

    # Private Instance Functions ----------------------------------------------

    def __createTestOp(self):
        """
        Set up a client, create an Op which generates some scene graph data
        """
        # Create an Op
        runtime = FnGeolib.GetRegisteredRuntimeInstance()
        txn = runtime.createTransaction()
        op = txn.createOp()

        # Build Op args for a StaticSceneCreate
        sscb = FnGeolibServices.OpArgsBuilders.StaticSceneCreate()
        terminalType = "group"
        opType = "StaticSceneCreate"

        for locationPath in self.__class__.LocationsToCreate:
            if not locationPath:
                continue
            sscb.createEmptyLocation(locationPath, terminalType)

        opArgs = sscb.build()
        txn.setOpArgs(op, opType, opArgs)

        runtime.commit([txn])

        return op


# Main Code -------------------------------------------------------------------

PluginRegistry = [("KatanaPanel", 2.0, "SceneGraphView/Top-Level Items Example", TopLevelItemExampleTab)]
PluginRegistry += [("KatanaPanel", 2.0, "SceneGraphView/Scene Graph Example", ExampleSceneGraphTab)]
PluginRegistry += [("KatanaPanel", 2.0, "SceneGraphView/Light View Example", LightViewExampleTab)]
#PluginRegistry += [("KatanaPanel", 2.0, "SceneGraphView/Debug Example", DebugExampleTab)]
PluginRegistry += [("KatanaPanel", 2.0, "SceneGraphView/Scene Graph Attributes Example", GetSceneGraphAttributesExampleTab)]
PluginRegistry += [("KatanaPanel", 2.0, "SceneGraphView/Context Menu Example", ContextMenuExampleTab)]
PluginRegistry += [("KatanaPanel", 2.0, "SceneGraphView/Column Test Example", ColumnTestExampleTab)]
PluginRegistry += [("KatanaPanel", 2.0, "SceneGraphView/Filtering Example", FilteringExampleTab)]
PluginRegistry += [("KatanaPanel", 2.0, "SceneGraphView/Name Polishing Example", NamePolishingExampleTab)]
PluginRegistry += [("KatanaPanel", 2.0, "SceneGraphView/Op View Example", OpViewExampleTab)]
