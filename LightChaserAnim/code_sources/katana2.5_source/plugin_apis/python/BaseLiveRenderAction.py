# Copyright (c) 2014 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import Configuration

if Configuration.get('KATANA_UI_MODE'):
    from PyQt4 import QtCore, QtGui


    class BaseLiveRenderAction(QtGui.QAction):
        """
        Base class for Live Render actions called from the Live Render menu.

        Classes derived from this class should be registered in
        L{PluginActions}.
        """

        def __init__(self, text, parent):
            """
            Initializes an instance of the class.

            @type text: C{str}
            @type parent: C{QtCore.QObject}
            @param text: The text to use for the action.
            @param parent: The parent to use for the action.
            """
            QtGui.QAction.__init__(self, text, parent)

            QtCore.QObject.connect(self, QtCore.SIGNAL('triggered(bool)'),
                                   self._on_triggered)

        def updateState(self):
            """
            Makes the action visible or enabled in the UI.

            This might be called many times during UI updates.
            """
            pass

        def _on_triggered(self, checked=None):
            """
            Handles the action triggering event.

            @type checked: C{bool}
            @param checked: Checked state if action is checkable.
            """
            pass

        @staticmethod
        def getActionText():
            """
            @rtype: C{str}
            @return: The text to use for the action in the UI.
            @raise NotImplementedError: If not implemented in a derived class.
            """
            raise NotImplementedError('getActionText() must be implemented in '
                                      'classes derived from '
                                      'BaseLiveRenderAction.')

        @staticmethod
        def getActionShortcut():
            """
            @rtype: C{str} or C{None}
            @return: The textual form of a keyboard shortcut to use for the
                action in the UI, for example C{"Alt+C"}, or C{None} if no
                keyboard shortcut is to be used (the default).
            """
            return None

        @staticmethod
        def getMenuTitle():
            """
            @rtype: C{str} or C{None}
            @return: The title to use for a menu in which the action is to
                appear, or C{None} if the action is not to appear in a submenu
                (the default).
            """
            return None

