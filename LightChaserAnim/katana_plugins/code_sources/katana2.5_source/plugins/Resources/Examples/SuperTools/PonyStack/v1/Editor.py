# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import QtCore, QtGui, UI4, QT4Widgets, QT4FormWidgets
from Katana import NodegraphAPI, Utils
from Katana import UniqueName, FormMaster

import ScriptActions as SA

# This editor shows three different types of UI:
#  1) A standard FormWidget is used for the 'location' parameter.  It automatically
#     tracks changes to the parameter.
#  2) A custom tree view shows the list of ponies.  We are responsible for rebuilding
#     this when the internal network changes.
#  3) A FormItem tree is used to expose the 'transform' parameters of the selected
#     pony's internal node.  We track the selection, but the FormItems take care of
#     syncing with the internal parameters (just like the FormWidget).

#//////////////////////////////////////////////////////////////////////////////
#///
class PonyStackEditor(QtGui.QWidget):
    def __init__(self, parent, node):
        node.upgrade()
        
        self.__node = node
        
        QtGui.QWidget.__init__(self, parent)
        QtGui.QVBoxLayout(self)
        
        self.__frozen = True
        self.__updateTreeOnIdle = False
        self.__selectedPonyPolicy = None
        self.__preselectName = None

        # location FormWidget
        locationPolicy = UI4.FormMaster.CreateParameterPolicy(
            None, self.__node.getParameter('location'))
        factory = UI4.FormMaster.KatanaFactory.ParameterWidgetFactory
        w = factory.buildWidget(self, locationPolicy)
        self.layout().addWidget(w)

        # toolbar
        self.__toolbarLayout = QtGui.QHBoxLayout()
        self.layout().addItem(self.__toolbarLayout)

        self.__addButton = UI4.Widgets.ToolbarButton(
            'Add Pony', self,
            UI4.Util.IconManager.GetPixmap('Icons/plus16.png'),
            rolloverPixmap = UI4.Util.IconManager.GetPixmap('Icons/plusHilite16.png'))
        self.connect(self.__addButton, QtCore.SIGNAL('clicked()'), self.__addButtonClicked)
        self.__toolbarLayout.addWidget(self.__addButton)
        self.__toolbarLayout.addStretch()

        # tree widget
        self.__treeStretchBox = UI4.Widgets.StretchBox(self,
                allowHorizontal=False, allowVertical=True)
        self.layout().addWidget(self.__treeStretchBox)

        self.__treeWidget = QT4Widgets.SortableTreeWidget(self.__treeStretchBox)
        self.__treeWidget.setHeaderLabels(['Name'])
        self.__treeWidget.setSelectionMode(QtGui.QTreeWidget.SingleSelection)
        self.__treeWidget.setAllColumnsShowFocus(True)
        self.__treeWidget.setRootIsDecorated(False)
        self.__treeWidget.header().setResizeMode(QtGui.QHeaderView.ResizeToContents)
        self.__treeWidget.header().setClickable(False)

        self.connect(self.__treeWidget,
            QtCore.SIGNAL('itemSelectionChanged()'), self.__treeWidgetSelectionChanged)
        self.connect(self.__treeWidget,
            QtCore.SIGNAL('keyPressEvent'), self.__treeWidgetKeyPressCallback)

        self.__treeStretchBox.layout().addWidget(self.__treeWidget)
        self.__treeStretchBox.setFixedHeight(120)

        self.__formItemTree = QT4FormWidgets.FormItemTree(self)
        self.layout().addWidget(self.__formItemTree)

        self.__updateTreeContents()

        self.layout().addStretch()
        
    
    # We thaw/freeze the UI when it is shown/hidden.  This means that we aren't
    # wasting CPU cycles by listening and responding to events when the editor
    # is not active.
    def showEvent(self, event):
        QtGui.QWidget.showEvent(self, event)
        if self.__frozen:
            self.__frozen = False
            self._thaw()
    
    def hideEvent(self, event):
        QtGui.QWidget.hideEvent(self, event)
        if not self.__frozen:
            self.__frozen = True
            self._freeze()
    
    def _thaw(self):
        self.__setupEventHandlers(True)
    
    def _freeze(self):
        self.__setupEventHandlers(False)

    def __setupEventHandlers(self, enabled):
        Utils.EventModule.RegisterEventHandler(self.__idle_callback,
            'event_idle', enabled=enabled)

        Utils.EventModule.RegisterCollapsedHandler(self.__updateCB,
            'port_connect', enabled=enabled)
        Utils.EventModule.RegisterCollapsedHandler(self.__updateCB,
            'port_disconnect', enabled=enabled)
        Utils.EventModule.RegisterCollapsedHandler(self.__updateCB,
            'parameter_finalizeValue', enabled=enabled)

    def __updateCB(self, args):
        if self.__updateTreeOnIdle:
            return

        for arg in args:
            if arg[0] in ('port_connect', 'port_disconnect'):
                for nodeNameKey in 'nodeNameA', 'nodeNameB':
                    nodeName = arg[2][nodeNameKey]
                    node = NodegraphAPI.GetNode(nodeName)
                    if node is not None and node.getParent() == self.__node:
                        self.__updateTreeOnIdle = True
                        return
            
            if arg[0] in ('parameter_finalizeValue'):
                node = arg[2].get('node')
                param = arg[2].get('param')
                if node.getParent() == self.__node and param == node.getParameter('name'):
                    self.__updateTreeOnIdle = True
                    return


    
    def __idle_callback(self, *args, **kwargs):
        if self.__updateTreeOnIdle:
            self.__updateTreeContents()
            self.__updateTreeOnIdle = False

    def __addButtonClicked(self):
        name, index = self.__node.addPony('pony')
        self.__preselectName = name

    def __updateTreeContents(self):
        scrollToPreselect = False
        scrollToItem = None
        vScrollbar = self.__treeWidget.verticalScrollBar()
        if vScrollbar and vScrollbar.isVisible():
            yPos = vScrollbar.value()
        else:
            vScrollbar = None

        try:
            self.__rebuilding = True
            node = self.__node
            
            selectedTable, openTable = self.__treeWidget.getExpandedAndSelectionTables()
            
            if self.__preselectName:
                scrollToPreselect = True
                selectedNames = set([self.__preselectName])
                self.__preselectName = None
            else:
                selectedNames = set([str(x.text(0)) for x in selectedTable.itervalues()])

            self.__treeWidget.clear()

            ponyNames = node.getPonyNames()
            for ponyName in ponyNames:
                item = QT4Widgets.SortableTreeWidgetItem(
                    self.__treeWidget, ponyName, data=None)
                selected = ponyName in selectedNames
                item.setSelected(selected)
                if scrollToPreselect and selected:
                    scrollToItem = item

        finally:
            if scrollToItem:
                self.__treeWidget.scrollToItem(scrollToItem)
            else:
                if vScrollbar:
                    vScrollbar.setValue(yPos)
            
            self.__rebuilding = False

        self.__treeWidgetSelectionChanged()

    def __treeWidgetSelectionChanged(self):
        if self.__rebuilding: return

        selection = self.__treeWidget.selectedItems()
        selectedPonyNode = None
        if selection:
            index = self.__treeWidget.indexOfTopLevelItem(selection[0])
            selectedPonyNode = SA.GetPonyNode(self.__node, index)

        selectedPonyParam = None
        if selectedPonyNode:
            selectedPonyParam = selectedPonyNode.getParameter('transform')

        if self.__selectedPonyPolicy and \
           self.__selectedPonyPolicy.getParameter() == selectedPonyParam:
            return

        self.__selectedPonyPolicy = None
        self.__formItemTree.clear()

        if selectedPonyParam:
            self.__selectedPonyPolicy = UI4.FormMaster.CreateParameterPolicy(None,
                selectedPonyParam)
            factory = UI4.FormMaster.ParameterItemFactory
            QT4FormWidgets.OpenState.RegisterOpenState(
                self.__selectedPonyPolicy.getOpenStateKey(), False)
            item = self.__formItemTree.buildLayoutItem(self.__formItemTree,
                self.__selectedPonyPolicy, factory)
            item.setExpanded(True)


    def __treeWidgetKeyPressCallback(self, event):
        if event.isAccepted(): return
        
        if event.key() == QtCore.Qt.Key_Delete:
            event.accept()

            selection = self.__treeWidget.selectedItems()
            if selection:
                index = self.__treeWidget.indexOfTopLevelItem(selection[0])
                self.__node.deletePony(index)


# The following code shows how to register a custom NodeActionDelegate for
# the PonyStack node.
# NodeActionDelegates are subclasses of BaseNodeActionDelegate and allow to
# add QActions to the node's context menu and wrench menu

from UI4.FormMaster.NodeActionDelegate import BaseNodeActionDelegate

class PonyStackActionDelegate(BaseNodeActionDelegate.BaseNodeActionDelegate):
    class _AddPony(QtGui.QAction):
        def __init__(self, parent, node):
            QtGui.QAction.__init__(self, "Add Pony", parent)
            self.__node = node
            if node:
                self.connect(self, QtCore.SIGNAL('triggered(bool)'), self.__triggered)
        def __triggered(self, checked):
            self.__node.addPony('pony')

    def addToWrenchMenu(self, menu, node, hints=None):
        menu.addAction(self._AddPony(menu, node))

    def addToContextMenu(self, menu, node):
        menu.addAction(self._AddPony(menu, node))


UI4.FormMaster.NodeActionDelegate.RegisterActionDelegate("PonyStack", PonyStackActionDelegate())
