# Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

import logging

from UI4.Manifest import (
    QtGui,
    QtCore,
    ScenegraphManager,
    FnAttribute
)

log = logging.getLogger("ExampleViewer.ManipulatorMenu")

class NoManipulatorAction(QtGui.QAction):
    """
    A QAction that deactivates all the manipulators.
    """
    def __init__(self, parent, viewports):
        QtGui.QAction.__init__(self, 'No Manipulators', parent)

        self.__viewports = viewports

        self.setObjectName("noManipulatorAction")
        self.setShortcut("Esc")

        QtCore.QObject.connect(self,
            QtCore.SIGNAL("triggered()"), self.__on_triggered)

    def __on_triggered(self):
        """
        Slot that is called when the action has been triggered.
        """
        for viewport in self.__viewports:
            viewport.deactivateAllManipulators(True)
            viewport.setDirty(True)


class OrientationAction(QtGui.QAction):
    """
    A QAction that sets the manipulator orientation on the Viewport.
    """
    def __init__(self, text, parent, viewports, value, checked=False):
        QtGui.QAction.__init__(self, text, parent)

        self.__viewports = viewports
        self.__value = FnAttribute.IntAttribute(value)
        self.setCheckable(True)
        self.setObjectName(text + "OrientationAction")

        # See if this should be checked by checking the ManipulatorOrientation
        # on one of the Viewports
        checkedAttr = self.__viewports[0].getOptionByName("ManipulatorOrientation")
        if checkedAttr:
            if checkedAttr.getValue() == value:
                self.setChecked(True)
        elif value == 0:
            self.setChecked(True)

        QtCore.QObject.connect(self,
            QtCore.SIGNAL("triggered()"), self.__on_triggered)

    def __on_triggered(self):
        """
        Slot that is called when the action has been triggered.
        """
        for viewport in self.__viewports:
            viewport.setOptionByName("ManipulatorOrientation", self.__value)
            viewport.setDirty(True)

class OrientationActionGroup(QtGui.QActionGroup):
    """
    A QActionGroup with actions that select the manipulator orientation.
    """
    def __init__(self, parent, viewports):
        QtGui.QActionGroup.__init__(self, parent, exclusive=True)

        self.__viewports = viewports
        self.setObjectName("OrientationActionMenu")

        self.__objectAction = OrientationAction("Object", self, self.__viewports, 0, True)
        self.__worldAction = OrientationAction("World", self, self.__viewports, 1)
        self.__viewAction = OrientationAction("View", self, self.__viewports, 2)

        self.addAction(self.__objectAction)
        self.addAction(self.__worldAction)
        self.addAction(self.__viewAction)

class ManipulatorMenu(QtGui.QMenu):
    """
    A QMenu that controlls the active Manipulators on a Viewport.
    """

    def __init__(self, name, parent, viewerDelegate, viewportWidgets):
        QtGui.QMenu.__init__(self, name, parent)

        self.__viewerDelegate = viewerDelegate
        self.__viewportWidgets = viewportWidgets
        self.__viewports = [vw.getViewport() for vw in viewportWidgets]

        # Get all the viewports for this viewer delegate, required by the
        # Manipulator Orientation Actions
        allViewports = []
        for i in range(viewerDelegate.getNumberOfViewports()):
            allViewports.append(viewerDelegate.getViewportByIndex(i))

        # No manipulator action
        noManipulatorAction = NoManipulatorAction(self, self.__viewports)
        self.addAction(noManipulatorAction)
        QtCore.QObject.connect(noManipulatorAction,
                               QtCore.SIGNAL("triggered()"),
                               self.__uncheckManipulators)

        self.addSeparator()

        # Initializes the Manipulator Orientation Actions
        orientationActionGroup = OrientationActionGroup(
            self, allViewports)
        for action in orientationActionGroup.actions():
            self.addAction(action)

        # Manipulator actions. These actions are added or removed
        # based on the selected items in the scene.
        self._manipulatorActions = []

    def __isManipulatorActive(self, pluginName):
        """
        Returns True if the Manipulator with this plugin name has been
        activated in any Viewport.
        """
        for viewport in self.__viewports:
            for i in range(viewport.getNumberOfActiveManipulators()):
                if viewport.getActiveManipulatorPluginName(i) == pluginName:
                    return True
        return False

    def __addCompatibleManipulators(self):
        """
        Adds compatible manipulators and their shortcuts to the menu.
        """
        # Clear current manipulator actions
        for action in self._manipulatorActions:
            self.removeAction(action)

        selectedLocations = ScenegraphManager.getActiveScenegraph().getSelectedLocations()
        manipsAttr = self.__viewerDelegate.getCompatibleManipulatorsInfo(
            selectedLocations)

        # manipNames = ['Translate','Rotate','Scale']
        actionGroups = {}

        # Assemble all registered manipulators into groups
        for i in range(manipsAttr.getNumberOfChildren()):
            manipulator = manipsAttr.getChildByIndex(i)
            pluginName = manipsAttr.getChildName(i)
            manipulatorName = manipulator.getChildByName("name").getValue(pluginName, False)
            shortcutKey = manipulator.getChildByName("shortcut").getValue("", False)
            group = manipulator.getChildByName("group").getValue("", False)
            technology = manipulator.getChildByName("technology").getValue("", False)

            # Create an action to select the manipulator of the current class
            action = QtGui.QAction(manipulatorName, self)
            action.setObjectName(pluginName.replace(" ", "") + "Action")
            action.setCheckable(True)
            action.setData(pluginName)
            action.setChecked(self.__isManipulatorActive(pluginName))
            action.setEnabled(True)
            action.setShortcut(shortcutKey)
            self._manipulatorActions.append(action)

            QtCore.QObject.connect(action, QtCore.SIGNAL("triggered()"),
                                   self.__on_action_triggered)

            if group not in actionGroups:
                actionGroups[group] = []

            actionGroups[group].append(action)

        # Build menu from action groups
        for group in actionGroups:
            action = QtGui.QAction(group, self)
            action.setSeparator(True)
            self._manipulatorActions.append(action)
            self.addAction(action)
            self.addActions(actionGroups[group])

    def __uncheckManipulators(self):
        """
        Unchecks all the manipulators.
        """
        for action in self._manipulatorActions:
            action.setChecked(False)

    def __on_action_triggered(self):
        """
        Slot that is called when an action for selecting a specific manipulator
        has been triggered.
        """
        if self.sender() is not None:
            if not self.sender().data().isNull():
                manipulatorName = str(self.sender().data().toPyObject())
                if self.sender().isChecked():
                    selectedLocations = ScenegraphManager.getActiveScenegraph().getSelectedLocations()

                    for viewport in self.__viewports:
                        # We want to exclude the current look-through location
                        # from the selected paths for each viewport.
                        filteredLocations = \
                            ManipulatorMenu.filterLookThroughLocation(
                                viewport, selectedLocations)

                        viewport.activateManipulator(
                            manipulatorName, filteredLocations, True)
                        viewport.setDirty(True)
                else:
                    for viewport in self.__viewports:
                        viewport.deactivateManipulator(
                            manipulatorName, True)
                        viewport.setDirty(True)
            else:
                log.error("The action that called "
                          "__on_action_triggered() does not have any "
                          "data attached to it.")
        else:
            log.error("__on_action_triggered() is expected to be "
                      "called from an action in the Manipulators menu.")

    def _on_selection_changed(self):
        """
        Manipulator menu updates when the selection changes in the scene.
        """
        self.__addCompatibleManipulators()

    @staticmethod
    def filterLookThroughLocation(viewport, locations):
        """
        Filters the viewport's current look-through location from the given
        list of locations.

        @type viewport: C{Viewport}
        @type locations: C{list} of C{str}
        @rtype: C{list} of C{str}
        @param viewport: The viewport.
        @param locations: The currently selected locations.
        @return: The new subset of locations that does not contain the
            viewport's current look-through location.
        """
        filteredLocations = locations
        camera = viewport.getActiveCamera()
        if camera:
            cameraLocation = camera.getLocationPath()
            if cameraLocation and cameraLocation in locations:
                filteredLocations = list(locations)
                filteredLocations.remove(cameraLocation)
        return filteredLocations
