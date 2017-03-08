# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#//////////////////////////////////////////////////////////////////////////////
#// Imports
from Katana import QtCore, QtGui, QT4Widgets, UI4, Utils, QT4FormWidgets
from Katana import NodegraphAPI, GeoAPI, Nodes3DAPI, PyScenegraphAttr
from UI4 import Widgets
from UI4.Util import ScenegraphIconManager
from Util import undogroup, stacktrace
import ScriptActions as SA
import Util
import os
import logging

log = logging.getLogger("ShadowManager.Editor")


#//////////////////////////////////////////////////////////////////////////////
LOCALRESOURCES = os.path.join(os.path.dirname(__file__), 'Resources')
def getIcon(name):
    return UI4.Util.IconManager.GetIcon(os.path.join(LOCALRESOURCES, name))

#//////////////////////////////////////////////////////////////////////////////

def _GetLocationType(node, location):
    """
    Get the 'type' attribute value for the given location, using the Op tree
    for the given node.

    @type node: C{Node}
    @type location: C{str}
    @rtype: C{str} or C{None}
    @param node: The node to calculate the scenegraph for
    @param location: The scenegraph location to query the type of
    @return: The location type as a string, or C{None} if the location wasn't
        found or the type attribute wasn't found
    """
    producer = Nodes3DAPI.GetGeometryProducer(node)
    if not producer:
        return None

    locationProducer = producer.getProducerByPath(location)
    if not locationProducer:
        return None

    attr = locationProducer.getAttribute("type")
    if attr:
        return attr.getData()[0]

#//////////////////////////////////////////////////////////////////////////////

class ShadowManagerEditor(QtGui.QWidget):
    """
    The main class of the editor, containing the UI and event callbacks
    """
    def __init__(self, parent, node):
        """
        Initialize the editor and set up UI
        """
        self.__node = node
        self.__frozen = True
        
        QtGui.QWidget.__init__(self, parent)
        QtGui.QVBoxLayout(self)
        
        self.__tabs = QtGui.QTabWidget(self)
        
        #  Create Passes tab
        self.__renderPassTab = RenderPassTab(None, node)

        #  Add tab to UI
        try:
            self.__tabs.addTab(self.__renderPassTab, 'Render Passes')
        except Exception as exception:
            log.exception('Error adding "Render Passes" tab for "%s" node: %s'
                          % (node.getName(), str(exception)))
        
        self.layout().addWidget(self.__tabs)
        self.layout().addStretch()
    
    def showEvent(self, event):
        """
        Prepare widget to be displayed
        """
        QtGui.QWidget.showEvent(self, event)
        if self.__frozen:
            self.__frozen = False
            self._thaw()
    
    def hideEvent(self, event):
        """
        Called when widget disappeared
        """
        QtGui.QWidget.hideEvent(self, event)
        if not self.__frozen:
            self.__frozen = True
            self._freeze()
    
    def _freeze(self):
        """
        Pass freeze event through to tab
        """
        self.__renderPassTab._freeze()
    
    def _thaw(self):
        """
        Pass thaw event through to tab
        """
        self.__renderPassTab._thaw()
        
# /////////////////////////////////////////////////////////////////////////////

class connectsignal(object):
    """
    Used as a decorator to connect functions to one or more Qt signals
    """
    def __init__(self, target, *args):
        self.__target = target
        self.__signalList = args
    
    def __call__(self, f):
        for signalText in self.__signalList:
            self.__target.connect(self.__target, QtCore.SIGNAL(signalText), f)
        return f


# /////////////////////////////////////////////////////////////////////////////

class RenderPassTab(QtGui.QWidget):
    """
    The 'Render Passes' tab
    """
    def __init__(self, parent, node):
        """
        Initialize the editor and set up UI
        """
        self.__node = node
        self.__preselectionKey = None
        self.__lightselectionKey = None
        self.__editorKey = None
        self.__updating = False
        
        QtGui.QWidget.__init__(self, parent)
        QtGui.QHBoxLayout(self)
        
        self.__treeLayout = QtGui.QVBoxLayout()
        self.layout().addLayout(self.__treeLayout)
        
        
        # toolbar {
        self.__treeToolbarLayout = QtGui.QHBoxLayout()
        self.__treeLayout.addLayout(self.__treeToolbarLayout)
        
        self.__addButton = UI4.Widgets.ToolbarButton(
                'Add Render Pass', self,
                UI4.Util.IconManager.GetPixmap('Icons/plus16.png'),
                rolloverPixmap = UI4.Util.IconManager.GetPixmap(
                        'Icons/plusHilite16.png'))
        @connectsignal(self.__addButton, 'clicked()')
        def addButtonClick():
            renderPass = self.__node.addRenderPass('pass1')
            self.__preselectionKey = hash(renderPass.getShadowBranchNode())
        
        self.__treeToolbarLayout.addWidget(self.__addButton)
        self.__treeToolbarLayout.addStretch()
        # }
        
        
        self.__treeStretchBox = UI4.Widgets.StretchBox(self,
                allowHorizontal=True, allowVertical=True)
        self.__treeLayout.addWidget(self.__treeStretchBox)
        
        
        #######################################################################
        # Set up the treeWidget which represents the passes
        treeWidget = QT4Widgets.SortableTreeWidget(self.__treeStretchBox)
        treeWidget.setHeaderLabels(['Name'])
        treeWidget.setSelectionMode(QtGui.QTreeWidget.ExtendedSelection)
        treeWidget.setAllColumnsShowFocus(True)
        treeWidget.setRootIsDecorated(False)
        treeWidget.header().setResizeMode(QtGui.QHeaderView.ResizeToContents)
        treeWidget.header().setClickable(False)
        treeWidget.header().hide()
        treeWidget.setDraggable(True)
        
        #######################################################################
        # treeWidget events
        @connectsignal(treeWidget, 'mousePressEvent')
        def treeWidgetMousePressEvent(event):
            """
            Handle mouse events
            """
            # Set up context menu (right-click)
            if event.button() == QtCore.Qt.RightButton:
                item = treeWidget.itemAt(event.pos())
                if not item:
                    return
                
                scriptItem = item.getItemData()                
                item.setHiliteColor(QtGui.QColor(32,32,32))
                treeWidget.update(treeWidget.indexFromItem(item))
                
                menu = QtGui.QMenu(None)
                
                # Add 'Delete' action
                menu.addAction('Delete', scriptItem.delete)
                menu.addSeparator()
                
                # Add 'View From ShadowBranch Node' action
                def viewNode():
                    NodegraphAPI.SetNodeViewed(
                        scriptItem.getShadowBranchNode(), True, exclusive=True)
                action = menu.addAction('View From ShadowBranch Node',viewNode)
                action.setIcon(UI4.Util.IconManager.GetIcon(
                        'Icons/AttributeEditor/blue_snapback.png'))                
                
                # Show menu
                pos = event.pos()
                pos = treeWidget.mapToGlobal(QtCore.QPoint(
                                pos.x(), pos.y()))
                
                try:
                    menu.exec_(pos)
                finally:
                    item.setHiliteColor(None)
                    treeWidget.update(treeWidget.indexFromItem(item))
                    event.accept()
        
        @connectsignal(treeWidget, 'keyPressEvent')
        def keyPressEvent(event):
            """
            Handle key events
            """
            selectedItems = treeWidget.selectedItems()
            if not selectedItems:
                return
            
            # Single selection for now
            scriptItem = selectedItems[0].getItemData()
            
            # Delete key: Remove selected items
            if event.key() == QtCore.Qt.Key_Delete:
                for item in selectedItems:
                    item.getItemData().delete()


        # Configure treeWidget callbacks and delegates
        self.__treeWidget = treeWidget        
        self.__treeWidget.setItemDelegate(
                self.RenderPassItemDelegate(self.__treeWidget))        
        self.__handleDragEvents(self.__treeWidget)
        self.connect(treeWidget, QtCore.SIGNAL('itemSelectionChanged()'),
                self.__selectionChangedCallback)

        # Add treeWidget to layout
        self.__treeStretchBox.layout().addWidget(treeWidget)
        self.__treeStretchBox.setFixedHeight(160)
        self.__treeStretchBox.setFixedWidth(120)
        self.__treeLayout.addStretch()
        
        # Layout settings
        self.__editorMainLayout = QtGui.QVBoxLayout()
        self.layout().addLayout(self.__editorMainLayout, 10)
        self.__editorMainLayout.addSpacing(4)        
        self.__editorWidget = QtGui.QWidget(self)
        self.__editorMainLayout.addWidget(self.__editorWidget)
        QtGui.QVBoxLayout(self.__editorWidget)
        self.__editorMainLayout.addStretch()
        
        # Create lightSettings widget containing the UI for a light
        self.__lightSettingsWidget = QtGui.QWidget(self)
        self.__editorWidget.layout().addWidget(self.__lightSettingsWidget)
        QtGui.QVBoxLayout(self.__lightSettingsWidget)

        
    #/////////////////////////////////////////////////////////////////////////
    def getNode(self):
        return self.__node

    #/////////////////////////////////////////////////////////////////////////

    class RenderPassItemDelegate(QT4Widgets.SortableTreeWidgetItemDelegate):
        """
        Delegate class of a render pass item. Responsable for drawing the item
        """
        def setModelData(self, editor, model, index):
            QT4Widgets.SortableTreeWidgetItemDelegate.setModelData(
                    self, editor, model, index)
            
            widget = self.parent()
            if widget:
                item = widget.itemFromIndex(index)
                item.getItemData().setName(
                        str(model.itemData(index)[0].toString()))
        
        def paint(self, painter, option, index):
            self.__drawViewBadge = False
            
            if index.column() == 0:
                widget = self.parent()
                if widget:
                    item = widget.itemFromIndex(index)
                    if NodegraphAPI.IsNodeViewed(
                            item.getItemData().getShadowBranchNode()):
                        self.__drawViewBadge = True
            
            QT4Widgets.SortableTreeWidgetItemDelegate.paint(
                    self, painter, option, index)
        
        def drawDecoration(self, painter, option, rect, pixmap):
            if self.__drawViewBadge:
                painter.fillRect(rect, QtGui.QBrush(QtGui.QColor(93,93,187)))
                painter.save()
                painter.setPen(QtGui.QPen(QtGui.QColor(80,80,160)))
                painter.drawRect(rect)
                painter.restore()
                self.__drawViewBadge = False
            if pixmap:
                painter.drawPixmap(rect.left()+1,
                    rect.top()+(rect.height()-pixmap.height())/2, pixmap)
    
    
    #/////////////////////////////////////////////////////////////////////////
    
    """
    Delegate class of a render pass item. Responsable for drawing the item
    """
    __EVENTNAMES = (
        'port_disconnect',
        'port_connect',
        'parameter_finalizeValue',
        'node_setViewed',
    )
    
    def _freeze(self):
        """
        Is called when the node is frozen. This will register a number of
        events in the event module.
        """
        for name in self.__EVENTNAMES:
            Utils.EventModule.UnregisterCollapsedHandler(
                    self.__collapsedEventHandler, name)

    def _thaw(self):
        """
        Is called when the node is thawed. This will register a number of
        events in the event module and also update the render pass list.
        """
        for name in self.__EVENTNAMES:
            Utils.EventModule.RegisterCollapsedHandler(
                    self.__collapsedEventHandler, name)
        self.update()

    def __collapsedEventHandler(self, args):
        """
        Is called when a registered event (e.g. port_connect, node_setViewed,
        etc.) occurs. We can use this to detect changes in the internal node
        hierarchy and update the UI depending on what nodes were involved.
        """
        if not self.__node.getParent():
            return

        needsUpdate = False
        needsUpdateLight = False

        for arg in args:
            if arg[0] == 'parameter_finalizeValue':
                node = arg[2]['node']
                if (node.getType() == 'Render'
                        and node.getParent() == self.__node
                        and arg[2]['param'].getName() == 'passName'):
                    needsUpdate = True
                    break
            elif arg[0] in ('port_connect, port_disconnect'):
                # Some port connections were made
                for key in 'portA', 'portB':
                    node = arg[2][key].getNode()
                    selectedItems = self.__treeWidget.selectedItems()

                    # Check if connections happened on the node directly ...
                    if node == self.__node or node.getParent() == self.__node:
                        needsUpdate = True
                        break
                    # ... or if they happened on the shadow branch node
                    elif selectedItems:
                        selectedItem = selectedItems[0]
                        renderPassItem = selectedItem.getItemData()
                        if node == renderPassItem.getShadowBranchNode():
                            # In this case, we manage the updates manually,
                            # i.e. when adding and removing lights.
                            needsUpdateLight = True
                            break

            elif arg[0] == 'node_setViewed':
                node = arg[2]['node']
                if node.getParent() == self.__node:
                    needsUpdate = True
                    break

        if needsUpdateLight:
            self.updateLightList()
        elif needsUpdate:
            self.update()


    #/////////////////////////////////////////////////////////////////////////

    def __handleDragEvents(self, treeWidget):
        """
        How to handle dragging and dropping of render pass items
        """
        @connectsignal(treeWidget, 'aboutToDrag')
        def aboutToDrag(item, dragObject):
            if not item: return
            item = item[0]

            mimeData = dragObject.mimeData()
            if not mimeData:
                mimeData = QtCore.QMimeData()
                dragObject.setMimeData(mimeData)

            mimeData.setData('ShadowManager/renderpass',
                    item.getItemData().getName())
            mimeData.setData('python/text',
                    'NodegraphAPI.GetNode(%r).getRenderPasses()[%i]' % (
                            self.__node.getName(),
                                    item.getItemData().getIndex()))

        @connectsignal(treeWidget, 'dragEnterEvent', 'dragMoveEvent')
        def processDrag(event, parent, index, callbackRecord):
            if event.source() == treeWidget and parent == None:
                dragItem = treeWidget.getDragItems()[0]
                dragIndex = treeWidget.findItemLocalIndex(dragItem)
                if index not in (dragIndex, dragIndex+1):
                    callbackRecord.accept()

        @connectsignal(treeWidget, 'dropEvent')
        def dropEvent(event, parent, index):
            if event.source() == treeWidget:

                # We only accept top-level drops
                if parent is not None:
                    return

                dragItems = self.__treeWidget.getDragItems()
                if not dragItems:
                    return
                renderPass = dragItems[0].getItemData()

                if index > renderPass.getIndex():
                    index -= 1
                renderPass.reorder(index)

    #/////////////////////////////////////////////////////////////////////////

    def __handleLightListEvents(self, lightListWidget):
        """
        How to handle events of light pass items
        """
        @connectsignal(lightListWidget, 'mousePressEvent')
        def lightListWidgetMousePressEvent(event):
            """
            Handle mouse events
            """
            # Set up context menu (right-click)
            if event.button() == QtCore.Qt.RightButton:
                item = lightListWidget.itemAt(event.pos())
                if not item:
                    return

                scriptItem = item.getItemData()
                item.setHiliteColor(QtGui.QColor(32,32,32))
                lightListWidget.update(lightListWidget.indexFromItem(item))

                menu = QtGui.QMenu(None)

                # Render options
                UI4.Util.RenderMenu.AddRenderItemsToMenu(
                        menu, scriptItem.getRenderNode(), None)
                menu.addSeparator()

                # Add 'Delete' action
                def delete():
                    # Delete selected light pass items
                    for item in lightListWidget.selectedItems():
                        item.getItemData().delete()


                menu.addAction('Delete', delete)
                menu.addSeparator()

                # Add 'View From Render Node' action
                def viewNode():
                    NodegraphAPI.SetNodeViewed(
                            scriptItem.getRenderNode(), True, exclusive=True)
                action = menu.addAction('View From Render Node', viewNode)
                action.setIcon(UI4.Util.IconManager.GetIcon(
                        'Icons/AttributeEditor/blue_snapback.png'))

                # Show menu
                pos = event.pos()
                pos = lightListWidget.mapToGlobal(QtCore.QPoint(
                         pos.x(), pos.y() + lightListWidget.header().height()))
                try:
                    menu.exec_(pos)
                finally:
                    item.setHiliteColor(None)
                    lightListWidget.update(lightListWidget.indexFromItem(item))
                    event.accept()
                    self.updateLightList()


        @connectsignal(lightListWidget, 'keyPressEvent')
        def lightListWidgetKeyPressEvent(event):
            """
            Handle key events
            """
            selectedItems = lightListWidget.selectedItems()
            if not selectedItems:
                return

            # Delete key: Remove selected items
            if event.key() == QtCore.Qt.Key_Delete:
                for item in selectedItems:
                    item.getItemData().delete()


        @connectsignal(lightListWidget, 'aboutToDrag')
        def aboutToDrag(item, dragObject):
            if not item: return
            item = item[0]

            mimeData = dragObject.mimeData()
            if not mimeData:
                mimeData = QtCore.QMimeData()
                dragObject.setMimeData(mimeData)

            mimeData.setData('ShadowManager/lightpass',
                    item.getItemData().getName())
            mimeData.setData('scenegraph/paths', item.getItemData().getName())

        @connectsignal(lightListWidget, 'dragEnterEvent', 'dragMoveEvent')
        def processDrag(event, parent, index, callbackRecord):
            # Accept drags from items in the list
            if event.source() == lightListWidget and parent == None:
                dragItem = lightListWidget.getDragItems()[0]
                dragIndex = lightListWidget.findItemLocalIndex(dragItem)
                if index not in (dragIndex, dragIndex+1):
                    callbackRecord.accept()

            # Accept drops with scene graph locations
            elif event.mimeData().hasFormat('scenegraph/paths'):
                callbackRecord.accept()


        @connectsignal(lightListWidget, 'dropEvent')
        def dropEvent(event, parent, index):
            # Drops from same widget
            if event.source() == lightListWidget:
                # We only accept top-level drops
                if parent is not None:
                    return

                dragItems = self.__lightWidget.getDragItems()
                if not dragItems:
                    return
                lightPass = dragItems[0].getItemData()

                if index > lightPass.getIndex():
                    index -= 1
                lightPass.reorder(index)

            # Drops from Scene Graph
            elif event.mimeData().hasFormat('scenegraph/paths'):
                locations = str(event.mimeData().data('scenegraph/paths')).split()
                # Add light pass for every light location
                for i, location in enumerate(locations):
                    if _GetLocationType(self.__node, location) == "light":
                        self.addLightPass(location, index + i)


    #//////////////////////////////////////////////////////////////////////////
    
    def __selectionChangedCallback(self):
        """
        Is called when a pass is selected. This will create the UI for altering
        the shadowBranch and adding new light passes to another tree list.
        """
        # Don't respond to changes while we update
        if self.__updating:
            return
        
        selectedItems = self.__treeWidget.selectedItems()
        
        # If more than one item is selected, don't display settings
        if len(selectedItems) != 1:
            self.__clearEditor()
            return

        if not selectedItems:
            selectedItem = None
            selectedKey = None
        else:
            selectedItem = selectedItems[0]
            selectedKey = hash(selectedItem.getItemData().getShadowBranchNode())
        
        if self.__editorKey == selectedKey:
            return

        self.__editorKey = selectedKey
        
        if self.__editorKey is None:
            return

        self.__clearEditor()
        renderPass = selectedItem.getItemData()

        # Shadow Branch
        shadowBranchNode = renderPass.getShadowBranchNode()
        policy = UI4.FormMaster.CreateParameterPolicy(None,
                    shadowBranchNode.getParameters())        
        # Hints can be used to specify additional options. Setting 'hideTitle'
        # to true will display the contents of the parameter group directly
        # rather than showing them as group with a title.
        policy.getWidgetHints()['hideTitle'] = False
        self.__editorWidget.layout().addWidget(
                UI4.FormMaster.ParameterWidgetFactory.buildWidget(
                        self.__editorWidget, policy))

        #######################################################################
        # Set up Add button and define click action
        addLightButton = LightPickerButton(self)
        addLightButton.setDisplayMode(DISPLAY_LIGHTS)
        addLightButton.setText('Add light')
        addLightButton.setIcon(UI4.Util.IconManager.GetIcon(
                        'Icons/plus16.png'))

        @connectsignal(addLightButton, 'itemChosen')
        def addButtonClick(value, meta):
            lightPath = meta[0]
            type = meta[1]
            self.addLightPass(lightPath)


        self.__editorWidget.layout().addWidget(addLightButton)
        self.__addLightButton = addLightButton

        # Light list stretch box
        self.__lightListStretchBox = UI4.Widgets.StretchBox(self,
                allowHorizontal=False, allowVertical=True)
        self.__editorWidget.layout().addWidget(self.__lightListStretchBox)

        #######################################################################
        # Set up the lightListWidget which represents the light passes
        lightListWidget = QT4Widgets.SortableTreeWidget(self.__lightListStretchBox)
        lightListWidget.setHeaderLabels(['Name'])
        lightListWidget.setSelectionMode(QtGui.QTreeWidget.ExtendedSelection)
        lightListWidget.setAllColumnsShowFocus(True)
        lightListWidget.setRootIsDecorated(False)
        lightListWidget.setDraggable(True)
        lightListWidget.setMinimumHeight(100)
        self.__lightListStretchBox.layout().addWidget(lightListWidget)
        self.__lightListStretchBox.setMinHeight(100)
        self.__lightListStretchBox.setFixedHeight(100)

        #######################################################################
        # lightListWidget events

        # Configure treeWidget that holds passes
        self.__lightWidget = lightListWidget

        self.__handleLightListEvents(self.__lightWidget)
        self.connect(lightListWidget, QtCore.SIGNAL('itemSelectionChanged()'),
                self.__lightSelectionChangedCallback)


        self.__editorWidget.layout().addWidget(self.__lightSettingsWidget)
        self.updateLightList()


    def __clearEditor(self):
        """
        Will delete all child widgets from the node
        """
        for child in self.__editorWidget.children():
            if isinstance(child, QtGui.QWidget):
                child.setParent(None)
                self.__editorKey = None

    #/////////////////////////////////////////////////////////////////////////
    def __removeItemsFromLayout(self, layout):
        """
        Helper function to remove all items from a layout
        """
        if layout is not None:
            while layout.count():
                item = layout.takeAt(0)
                widget = item.widget()
                if widget is not None:
                    widget.deleteLater()
                else:
                    self.__removeItemsFromLayout(item.layout())

    #/////////////////////////////////////////////////////////////////////////
    def __lightSelectionChangedCallback(self):
        """
        Is called when a light pass is selected. This will update the UI that
        displays information about the pass such as resolution, render
        location, etc.
        """
        # Don't respond to changes while we update
        if self.__updating:
            return


        # Remove previous settings from layout
        self.__removeItemsFromLayout(self.__lightSettingsWidget.layout())

        # Rebuild settings
        lightItems = self.__lightWidget.selectedItems()
        if len(lightItems) == 1:
            selectedLightItem = lightItems[0]

            lightPass = selectedLightItem.getItemData()

            # Get node references
            renderSettingsNode = lightPass.getRenderSettingsNode()
            renderOutputDefNode = lightPass.getRenderOutputDefineNode()
            renderPassNode = lightPass.getRenderPassItem().getGroupNode()
            lightParamGroup = renderPassNode.getParameter('lightPasses')
            passGroups = lightParamGroup.getChildren()
            lightPassParamName = passGroups[lightPass.getIndex()].getName()

            # Create group to hold parameters and hide its title
            group = QT4FormWidgets.PythonGroupPolicy("Light Pass Settings")
            group.getWidgetHints()['hideTitle'] = True

            # Define parameters to be added to the group
            params = []
            params.append(renderPassNode.getParameter(
                'lightPasses.' + lightPassParamName + '.renderLocation'))
            params.append(renderSettingsNode.getParameter(
                'args.renderSettings.resolution'))
            params.append(renderOutputDefNode.getParameter(
                'args.renderSettings.outputs.outputName.rendererSettings.shadowType'))
            params.append(renderOutputDefNode.getParameter(
                'args.renderSettings.outputs.outputName.rendererSettings.' +
                    'deepshadVolumeInterpretation'))

            # Create policies for each parameter and add it to the group.
            paramPolicy = UI4.FormMaster.ParameterPolicy
            for param in params:
                # Create a policy for this param
                policy = paramPolicy.CreateParameterPolicy(group, param)

                # Copy widget hints from the original policy to the parameter
                hints = policy.getWidgetHints()
                param.setHintString(repr(hints))

                # Add to the group
                group.addChildPolicy(policy)

            # Create a widget for the parameter group and add it to the layout
            factory = UI4.FormMaster.KatanaFactory.ParameterWidgetFactory
            widget = factory.buildWidget(None, group)
            self.__lightSettingsWidget.layout().addWidget(widget)

    #/////////////////////////////////////////////////////////////////////////

    def addLightPass(self, lightPath, index=None):
        """
        Adds a light path to the currently selected render pass
        """
        selectedItem = self.__treeWidget.selectedItems()[0]
        renderPassItem = selectedItem.getItemData()

        # Show error dialog if light already added
        lightPasses = self.__node.getLightPasses(renderPassItem)
        if lightPath in [p.getName() for p in lightPasses]:
            UI4.Widgets.MessageBox.Information("Light Not Added","The " \
                "selected light is already in the list.")
        else:
            item = self.__node.addLightPass(lightPath, renderPassItem, index)
            self.__lightselectionKey = hash(item.getGroupNode())

    #/////////////////////////////////////////////////////////////////////////

    def update(self):
        """
        Is called to update the UI
        """
        passes = self.__node.getRenderPasses()
        
        scrollToSelection = False
        
        if self.__preselectionKey is not None:
            selectedKeys = [self.__preselectionKey]
            self.__preselectionKey = None
            scrollToSelection = True
        else:
            selectedKeys = [hash(item.getItemData().getShadowBranchNode())
                    for item in self.__treeWidget.selectedItems()]
        
        scrollMemory = UI4.Widgets.ScrollAreaMemory.ScrollAreaMemory(
                self.__treeWidget)
        
        # Set updating flag, so __selectionChangedCallback will not update
        # the UI while we are re-building the list
        self.__updating = True
        try:
            with self.__treeWidget.getUpdateSuppressor():
                # Clear tree widget holding render passes
                self.__treeWidget.clear()

                # Add pass items
                for renderPassItem in self.__node.getRenderPasses():
                    item = QT4Widgets.SortableTreeWidgetItem(
                            self.__treeWidget, renderPassItem.getName(),
                                    data=renderPassItem)
                    shadowBranch = renderPassItem.getShadowBranchNode()
                    if hash(shadowBranch) in selectedKeys:
                        item.setSelected(True)

                    item.setFlags(item.flags() | QtCore.Qt.ItemIsEditable)
                    item.setIcon(0, getIcon('render16.png'))


            selectedItems = self.__treeWidget.selectedItems()
            if selectedItems:
                if scrollToSelection:
                    del scrollMemory
                    self.__treeWidget.scrollToItem(selectedItems[0])

        finally:
            self.__updating = False
            self.__selectionChangedCallback()

    #/////////////////////////////////////////////////////////////////////////

    def updateLightList(self, refreshCustomParams=True):
        """
        Will update the light passes in the lightWidget
        """
        # Set updating flag, so __lightSelectionChangedCallback will not update
        # the UI while we are re-building the list
        self.__updating = True

        selectedItems = self.__treeWidget.selectedItems()
        if selectedItems:
            passScriptItem = selectedItems[0].getItemData()
            scrollToSelection = False

            try:
                with self.__lightWidget.getUpdateSuppressor():

                    if self.__lightselectionKey is not None:
                        selectedKeys = [self.__lightselectionKey]
                        self.__lightselectionKey = None
                        scrollToSelection = True
                    else:
                        selectedKeys = [hash(item.getItemData().getGroupNode())
                                for item in self.__lightWidget.selectedItems()]


                    # Clear light pass list
                    self.__lightWidget.clear()

                    # Build light list for selected pass
                    lightPasses = self.__node.getLightPasses(passScriptItem)
                    for lightPassItem in lightPasses:
                        item = QT4Widgets.SortableTreeWidgetItem(
                                self.__lightWidget, lightPassItem.getName(),
                                        data=lightPassItem)
                        passGroup = lightPassItem.getGroupNode()
                        if hash(passGroup) in selectedKeys:
                            item.setSelected(True)
                        item.setIcon(0, getIcon('render16.png'))
            finally:
                self.__updating = False

                if refreshCustomParams:
                    self.__lightSelectionChangedCallback()


#//////////////////////////////////////////////////////////////////////////////

DISPLAY_CAMERAS = 1
DISPLAY_LIGHTS = 2

class LightPickerButton(Widgets.FilterablePopupButton):
    """
    Helper class used for the selection of a light and/or camera from a list
    of available lights or camera items.
    """
    __builtins = [ ]

    def __init__(self, *args):
        """
        Initialize and configure the button widget
        """
        Widgets.FilterablePopupButton.__init__(self, *args)
        self.setFixedHeight(24)
        self.setFlat(False)
        self.setSorting(False)

        QtCore.QObject.connect(self, QtCore.SIGNAL('aboutToShow'),
                               self.__on_aboutToShow)
        QtCore.QObject.connect(self, QtCore.SIGNAL('itemChosen'),
                               self.__on_itemChosen)

        self.__pathToDisplayInfo = {}
        self.__pixmaps = \
           {'camera': QtGui.QPixmap(ScenegraphIconManager.GetPixmap('camera')),
            'light': QtGui.QPixmap(ScenegraphIconManager.GetPixmap('light'))}

        self.__scenegraphPixmap = None

        # Argument is reference to the widget which allows us to get the node
        if args[0]:
            self.__node = args[0].getNode()


        # displayMode will be set to either <DISPLAY_LIGHTS, DISPLAY_CAMERAS,
        # DISPLAY_LIGHTS | DISPLAY_CAMERAS> if the mode is
        # DISPLAY_LIGHTS | DISPLAY_CAMERAS, then this indicates that the button
        # is the big center one at the bottom of the viewports. It will draw
        # the camera/light name in text, rather than pixmaps as its icon.
        self.__displayMode = 0

        self.setAcceptDrops(True)

    def setScenegraphPixmap(self, pixmap):
        """
        Sets the icon from a given pixmap
        """
        self.__scenegraphPixmap = pixmap
        self.setIcon(QtGui.QIcon(pixmap))

    def sizeHint(self):
        """
        Get the size
        """
        if self.__displayMode == (DISPLAY_LIGHTS | DISPLAY_CAMERAS):
            w = self.fontMetrics().width(self.text()) + 16
        else:
            w=8     #set some initial arbitrary size
        if self.__scenegraphPixmap:
            w += self.__scenegraphPixmap.width()
        return QtCore.QSize(w, 24)

    def drawButtonLabel(self, p):
        """
        Draw the button label depending on the display mode
        """
        x = 0
        if self.__scenegraphPixmap:
            x = y = (self.height() - self.__scenegraphPixmap.height())/2
            p.drawPixmap(x, y, self.__scenegraphPixmap)
            x = x + y
        if self.__displayMode == (DISPLAY_LIGHTS | DISPLAY_CAMERAS):
            p.drawText(x, 0, self.width()-x, self.height(),
                        QtCore.Qt.AlignCenter, self.text())

    def dragEnterEvent(self, event):
        """
        Handle drag enter events on the camera/light picker button
        """
        if event.source() == self:
            return

        if event.mimeData().hasFormat('scenegraph/paths'):
            locations = str(event.mimeData().data('scenegraph/paths')).split()
            if len(locations) == 1:
                if _GetLocationType(self.__node, locations[0]) in ('light',
                                                                   'camera'):
                    event.accept()

    def dropEvent(self, event):
        """
        Handle drop events on the camera/light picker button
        """
        if event.source() == self: return

        if event.mimeData().hasFormat('scenegraph/paths'):
            locations = str(event.mimeData().data('scenegraph/paths')).split()
            if len(locations) == 1:
                location = locations[0]
                if _GetLocationType(self.__node, location) in ('light',
                                                               'camera'):
                    self.emit(QtCore.SIGNAL('itemChosen'), None,
                                [location, 'camera'])

    def setDisplayMode(self, mode):
        """
        Set display mode and corresponding tool tips, icons, etc.
        """
        self.__displayMode = mode
        if self.__displayMode == (DISPLAY_LIGHTS | DISPLAY_CAMERAS):
            switchArea = self.getWidgetSwitchArea()
            self.__topHBox = QtGui.QFrame(switchArea)
            switchArea.layout().addWidget(self.__topHBox)
            self.__topHBox.setLayout(QtGui.QHBoxLayout())

            spacer = QtGui.QLabel('', self)
            self.__topHBox.layout().addWidget(spacer)
            self.__topHBox.layout().setStretchFactor(spacer, 100)

            self.__lightsCheckbox = QtGui.QCheckBox("Lights", self.__topHBox)
            self.__lightsCheckbox.setChecked(True)
            self.__topHBox.layout().addWidget(self.__lightsCheckbox)
            QtCore.QObject.connect(self.__lightsCheckbox,
                                   QtCore.SIGNAL('stateChanged(int)'),
                                   self.__on_lightsCheckbox_stateChanged)

            self.__camerasCheckbox = QtGui.QCheckBox("Cameras", self.__topHBox)
            self.__camerasCheckbox.setChecked(True)
            self.__topHBox.layout().addWidget(self.__camerasCheckbox)
            QtCore.QObject.connect(self.__camerasCheckbox,
                                   QtCore.SIGNAL('stateChanged(int)'),
                                   self.__on_camerasCheckbox_stateChanged)

            self.setToolTip("Choose Light or Camera")

        elif mode & DISPLAY_LIGHTS:
            self.setScenegraphPixmap(self.__pixmaps['light'])
            self.setToolTip("Choose Light")
        else:
            self.setScenegraphPixmap(self.__pixmaps['camera'])
            self.setToolTip("Choose Camera")

    # Private Functions -------------------------------------------------------

    def __populate(self):
        """
        Populates the UI.
        """
        showLights = self.__displayMode & DISPLAY_LIGHTS
        showCameras = self.__displayMode & DISPLAY_CAMERAS

        if self.__displayMode == (DISPLAY_LIGHTS | DISPLAY_CAMERAS):
            showLights = self.__lightsCheckbox.isChecked()
            showCameras = self.__camerasCheckbox.isChecked()


        self.__pathToDisplayInfo = {}
        self.clear()

        items = []

        # Add builtin cameras
        if showCameras:
            for builtin_camera in LightPickerButton.__builtins:
                items.append((builtin_camera, None, (builtin_camera + 'Shape', 'camera')))
                self.__pathToDisplayInfo[builtin_camera+'Shape'] = (builtin_camera, '')

        prod = Nodes3DAPI.GetGeometryProducer(self.__node)
        world = prod.getChildByName('world')

        if showCameras:
            # Build camera list
            globalsGroup = world.getAttribute("globals")
            camList = globalsGroup.childByName("cameraList")
            if camList:
                camera_locations = camList.getNearestSample(0)
                camera_locations.sort()

            abbreviatedCameraLocationNames = GeoAPI.ScenegraphUtils.AbbreviateScenegraphLocations(
                camera_locations)
            for fullLoc,shortLoc in zip(camera_locations,abbreviatedCameraLocationNames):
                items.append((shortLoc, self.__pixmaps['camera'], (fullLoc, 'camera')))
                self.__pathToDisplayInfo[fullLoc] = (shortLoc, 'camera')
        if showLights:
            # Build light list
            light_locations = []
            lightList = world.getAttribute("lightList")
            if lightList:
                for lightKey, lightAttr in lightList.childDict().items():
                    path = lightAttr.childByName("path")
                    lightPath = path.getNearestSample(0)[0]
                    light_locations.append(lightPath)

            light_locations.sort()
            abbreviatedLightLocationNames = GeoAPI.ScenegraphUtils.AbbreviateScenegraphLocations(
                light_locations)
            for fullLoc,shortLoc in zip(light_locations,abbreviatedLightLocationNames):
                items.append((shortLoc, self.__pixmaps['light'], (fullLoc, 'light')))
                self.__pathToDisplayInfo[fullLoc] = (shortLoc, 'light')


        for (shortLoc, pixmap, meta) in items:
            self.addItem(shortLoc, pixmap=pixmap, meta=meta)

    # Private Slots -----------------------------------------------------------

    def __on_aboutToShow(self):
        self.__populate()

    def __on_itemChosen(self, value, meta):
        """
        Is called when an item is chosen and will return its scene graph
        location.
        """
        path = meta[0]
        _itemType = meta[1]

        return path

    def __on_lightsCheckbox_stateChanged(self, state):
        self.__populate()

    def __on_camerasCheckbox_stateChanged(self, state):
        self.__populate()

