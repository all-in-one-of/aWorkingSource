from Katana import QtCore, QtGui, UI4, QT4Widgets, QT4FormWidgets

import ScriptActions as SA

import logging
log = logging.getLogger('GafferGhost')

# Class Definitions -----------------------------------------------------------

class GafferGhostEditor(QtGui.QWidget):
    """
    Example of a Super Tool editing widget that displays:
        - Its own parameters.
        - Parameters from node's in the SuperTool's internal node network.
        - Custom Qt Widgets.
    """

    # Initializer -------------------------------------------------------------

    def __init__(self, parent, node):
        """
        Initializes an instance of the class.
        """
        QtGui.QWidget.__init__(self, parent)

        self.__node = node

        # Get the SuperTool's parameters
        tagetsParameter = self.__node.getParameter('tagets')
        tunningParameter = self.__node.getParameter('tuning')
        translateParameter = self.__node.getParameter('translate')
        rotatexyzParameter = self.__node.getParameter('rotatexyz')
        scalebackParameter = self.__node.getParameter('scaleback')

        #######################################################################
        # Create parameter policies from the parameters and register callbacks
        # to be notified if anything changes in the underlying parameter.
        #
        # This is your Controller in the MVC pattern.
        #######################################################################
        CreateParameterPolicy = UI4.FormMaster.CreateParameterPolicy
        self.__tagetsParameterPolicy = CreateParameterPolicy(
            None, tagetsParameter)
        self.__tagetsParameterPolicy.addCallback(
            self.tagetsParameterChangedCallback)
        self.__tunningParameterPolicy = CreateParameterPolicy(
            None, tunningParameter)
        self.__rotationxParameterPolicy = CreateParameterPolicy(
            None, translateParameter)
        self.__rotatexyzParameterPolicy = CreateParameterPolicy(
            None, rotatexyzParameter)
        self.__scalebackParameterPolicy = CreateParameterPolicy(
            None, scalebackParameter)
        #######################################################################
        # Create UI widgets from the parameter policies to display the values
        # contained in the parameter.
        #
        # The widget factory will return an appropriate widget based on the
        # parameters type and any widget hints you've specified. For example
        # for a string parameter the factory will return a simple text editing
        # widget. But if you specify a widget hint of
        # 'widget': 'newScenegraphLocation' the factory will return a widget
        # that interacts with the Scene Graph tab.
        #
        # Other widget types you can use are:
        #   number -> Number Editor Widget
        #   assetIdInput -> Widget that provides hooks to your Asset system.
        #   color -> Widget to display a color
        #
        # This is your View in the MVC pattern.
        #######################################################################
        WidgetFactory = UI4.FormMaster.KatanaFactory.ParameterWidgetFactory
        tagetsWidget = WidgetFactory.buildWidget(
            self, self.__tagetsParameterPolicy)
        tunningWidget = WidgetFactory.buildWidget(
            self, self.__tunningParameterPolicy)
        translateWidget = WidgetFactory.buildWidget(
            self, self.__rotationxParameterPolicy)
        rotatexyzWidget = WidgetFactory.buildWidget(
            self, self.__rotatexyzParameterPolicy)
        scalebackWidget = WidgetFactory.buildWidget(
            self, self.__scalebackParameterPolicy)

        self.tunning_layout = tunningWidget.getPopdownWidget().layout()

        self.dial_topdown = QtGui.QDial()
        self.dial_topdown.setWrapping(True)
        self.dial_topdown.setNotchesVisible(True)

        self.dial_leftright = QtGui.QDial()
        self.dial_leftright.setWrapping(True)
        self.dial_leftright.setNotchesVisible(True)

        HBoxLayout = QtGui.QHBoxLayout()
        HBoxLayout_dial = QtGui.QHBoxLayout()
        VBoxLayout_button = QtGui.QVBoxLayout()
        VBoxLayout_data = QtGui.QVBoxLayout()
        VBoxLayout_data.addWidget(translateWidget)
        VBoxLayout_data.addWidget(rotatexyzWidget)
        VBoxLayout_data.addWidget(scalebackWidget)
        HBoxLayout_dial.addWidget(self.dial_topdown)
        HBoxLayout_dial.addSpacing(15)
        HBoxLayout_dial.addWidget(self.dial_leftright)
        # VBoxLayout_data.setFixedWidget
        HBoxLayout.addSpacing(50)
        HBoxLayout.addLayout(HBoxLayout_dial)
        HBoxLayout.addSpacing(30)
        HBoxLayout.addLayout(VBoxLayout_data)
        self.tunning_layout.addLayout(HBoxLayout)
        mainLayout = QtGui.QVBoxLayout()
        mainLayout.addWidget(tagetsWidget)
        mainLayout.addWidget(tunningWidget)
        # Apply the layout to the widget
        self.setLayout(mainLayout)

    def tagetsParameterChangedCallback(self, *args, **kwds):
        pass
    # Public Functions --------------------------------------------------------