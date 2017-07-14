"""
Module implementing a UI tab that shows how action and key event shortcuts can
be registered and used.
"""

from Katana import QtGui, QtCore, UI4
import UI4.App.Tabs
import UI4.App.Layouts
import UI4.Widgets
from UI4.Tabs import BaseTab
from UI4.Widgets.SceneGraphView import SceneGraphViewIconManager


# Class Definitions -----------------------------------------------------------

class KeyboardShortcutsExampleTab(BaseTab):
    """
    Simple tab showing how to register action and key event shortcuts.
    """

    # Initializer -------------------------------------------------------------

    def __init__(self, parent):
        """
        Initializes the tab with a 'greetings' list widget.
        """
        BaseTab.__init__(self, parent)

        # Show a message at the top of the tab to mention the separate Keyboard
        # Shortcuts tab
        self.__iconLabelFrame = UI4.Widgets.IconLabelFrame(
            SceneGraphViewIconManager.GetPixmap('muteHeader'),
            'Have a look at the <a href="Keyboard Shortcuts"><b>Keyboard '
            'Shortcuts</b></a> tab for the list of defined keyboard '
            'shortcuts.', margin=8, parent=self)
        self.__iconLabelFrame.setLinkClickCallback(self.__linkClickCallback)
        self.__iconLabelFrame.setMaximumHeight(32)

        # Create a QListWidget for showing greeting messages created through
        # keyboard shortcuts
        self.__greetingsListWidget = QtGui.QListWidget(self)

        # Initialize the instance variable that stores a QLabel that is shown
        # when the user presses down a key, and hidden when the user releases
        # that key
        self.__popupLabel = None

        # Create the tab's layout
        layout = QtGui.QVBoxLayout()
        layout.addWidget(self.__iconLabelFrame)
        layout.addWidget(self.__greetingsListWidget)
        self.setLayout(layout)

    # Static Functions --------------------------------------------------------

    @staticmethod
    def registerKeyboardShortcuts():
        """
        Registers action and key event callbacks for this tab using functions
        provided by C{BaseTab}.
        """
        # Define a tab name for actions and key events.
        KeyboardShortcutsExampleTab.setShortcutsContextName(
            "KeyboardShortcutsExampleTab")

        # Notice that action IDs must be unique.
        # The IDs defined in this function have been created using the
        # hexdigest() function provided by the md5 hash object available
        # through the hashlib Python module.
        # Each ID is based on the corresponding action's name.
        # The following code shows how the helloActionID was generated:
        # 
        # >>> import hashlib
        # >>> actionName = 'KeyboardShortcutsExampleTab.Greetings.SayHello'
        # >>> print(hashlib.md5(actionName).hexdigest())
        # e449e953455dc4ce57cb3986ae9e78b3

        # Define IDs for actions and key events
        helloActionID = "e449e953455dc4ce57cb3986ae9e78b3"
        byeActionID = "786af259a66893fadbf97ea6aabd7733"
        clearGreetingsActionID = "3a3babe35fcbd0ec54924eab7015d1eb"
        messageActionID = "1a8f0532ecb1ee2bd8060f5db30b0c5b"

        # Register keyboard shortcuts
        KeyboardShortcutsExampleTab.registerKeyboardShortcut(
            helloActionID, "Greetings.SayHello", "H",
            KeyboardShortcutsExampleTab.helloActionCallback)
        KeyboardShortcutsExampleTab.registerKeyboardShortcut(
            byeActionID, "Greetings.SayBye", "B",
            KeyboardShortcutsExampleTab.byeActionCallback)
        KeyboardShortcutsExampleTab.registerKeyboardShortcut(
            clearGreetingsActionID, "Greetings.Clear", "C",
            KeyboardShortcutsExampleTab.clearGreetings)

        # Register key event callbacks
        KeyboardShortcutsExampleTab.registerKeyboardShortcut(
            messageActionID, "Message", "M",
            KeyboardShortcutsExampleTab.messageKeyPressCallback,
            KeyboardShortcutsExampleTab.messageKeyReleaseCallback)

    # Action Callbacks --------------------------------------------------------

    def helloActionCallback(self):
        """
        Appends an item to the greetings list widget.
        """
        self.__greetingsListWidget.addItem("Hello!")

    def byeActionCallback(self):
        """
        Appends an item to the greetings list widget.
        """
        self.__greetingsListWidget.addItem("Bye!")

    def clearGreetings(self):
        """
        Clears the greetings list widget.
        """
        self.__greetingsListWidget.clear()

    def messageKeyPressCallback(self):
        """
        Shows a message label.
        """
        if self.__popupLabel is None:
            self.__popupLabel = QtGui.QLabel("Key is being pressed!", self)
        w = 150
        h = 50
        self.__popupLabel.setGeometry((self.width() - w) / 2,
                                      (self.height() - h) / 2, w, h)
        self.__popupLabel.show()

    def messageKeyReleaseCallback(self):
        """
        Hides the message label.
        """
        if not self.__popupLabel is None:
            self.__popupLabel.hide()

    # Private Callbacks -------------------------------------------------------

    def __linkClickCallback(self, link):
        """
        Callback for clicks of links in the icon label frame's text.

        @type link: C{str}
        @param link: The value of the C{href} attribute of the link that was
            clicked.
        """
        tabName = link
        widget = UI4.App.Tabs.FindTopTab(tabName, alsoRaise=True)
        if widget is None:
            UI4.App.Layouts.CreateFloatingTab(tabName)


PluginRegistry = [("KatanaPanel", 2.0, "Keyboard Shortcuts Example",
                   KeyboardShortcutsExampleTab)]

