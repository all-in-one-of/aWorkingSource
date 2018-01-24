"""
This module defines two classes:
    1). SimpleImageWithOverlayWidget is a custom Qt widget derived from
        QLabel (you are free to use any Qt code, for example, more complex
        drawing could be achieved using OpenGL-driven widgets).
        It provides the following features:
        - Ability to display an image which can be set by calling
          displayImage().
        - If specified (via setPointToDraw()), it will overlay a small green
          circle at the specified position. This provides a means of allowing a
          SuperTool to make changes to a custom UI widget.
        - It will propagate click events to its parent widget, which is
          assumed to be of type 'ImageCoordinateEditor'.
          ImageCoordinateEditor defines a method updateNumberValues() which
          SimpleImageWithOverlayWidget calls when it is clicked. This provides
          a means of pushing changes from a custom UI widget back to
          the SuperTool. You could also achieve this using regular Qt signals
          and slots.
    2). ImageCoordinateEditor is the top-level Qt Widget that is displayed
        as the SuperTool's interface in the Parameters tab. In this class we
        demonstrate how to:
            - Obtain a reference to a parameter on a node.
            - How to obtain a parameter policy which provides a means of
              communicating value changes to the underlying parameter and how
              to receive notifications when the underlying parameter value
              changes.
            - How to use the widget factory to create a number of pre-built
              Katana specific widgets. The widget returned by the widget
              factory is again just another Qt widget. The specific widget type
              returned is based on the parameter's data type and any widget
              hints that had been previously set.
"""

from Katana import QtCore, QtGui, UI4

import ScriptActions as SA

import logging
log = logging.getLogger('ImageCoordinateEditor')


# Class Definitions -----------------------------------------------------------

class SimpleImageWithOverlayWidget(QtGui.QLabel):
    """
    Custom Qt widget to provide a simple image display and means of overlaying
    drawing on-top.
    """

    # Initializer -------------------------------------------------------------

    def __init__(self, parent, filePath=''):
        """
        Initializes SimpleImageWithOverlayWidget and if specified loads the
        image from the given filePath.
        """
        QtGui.QLabel.__init__(self, parent)

        # Store a reference to our parent which we'll forward image click
        # events to.
        self.__clickerSuperToolEditor = parent

        # X, Y coordinates to draw (when set to -1 nothing will be drawn)
        self.__xPoint = -1
        self.__yPoint = -1

        # Show the image with the given filename in the widget
        self.displayImage(filePath)

    # Public Functions --------------------------------------------------------

    def displayImage(self, filePath):
        """
        Changes the image displayed by the widget.
        """
        self.__filePath = filePath
        if self.__filePath:
            self.setPixmap(QtGui.QPixmap(self.__filePath))

    def paintEvent(self, event):
        """
        Overrides the base paint event so we can overlay our circle.
        """
        QtGui.QLabel.paintEvent(self, event)

        if self.__xPoint > 0 and self.__yPoint > 0:
            paint = QtGui.QPainter()
            paint.begin(self)
            paint.setRenderHint(QtGui.QPainter.Antialiasing)

            paint.setPen(QtGui.QColor(0, 255, 0))
            paint.setBrush(QtGui.QColor(0, 255, 0))
            center = QtCore.QPoint(self.__xPoint, self.__yPoint)
            paint.drawEllipse(center, 5, 5)

            paint.end()

    def mouseReleaseEvent(self, event):
        """
        Forwards the details of our mouse click event to the parent widget.
        """
        self.__clickerSuperToolEditor.updateNumberValues(event.pos().x(),
                                                         event.pos().y())

    def setPointToDraw(self, x, y):
        """
        Sets the point to be drawn on top of the currently displayed image.
        """
        self.__xPoint = x
        self.__yPoint = y
        # Issue a repaint.
        self.repaint()

class ImageCoordinateEditor(QtGui.QWidget):
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

        #######################################################################
        # Obtain references to the parameters we'd like to display.
        #
        # This is your Model in the MVC pattern.
        #######################################################################

        # Get the SuperTool's parameters
        locationParameter = self.__node.getParameter('location')
        filePathParameter = self.__node.getParameter('filePath')

        # Get a parameter that belongs to a node within our internal node
        # network.
        attributeSetNode = SA.GetRefNode(self.__node, 'attributes')
        numberValueParameter = None
        if attributeSetNode:
            numberValueParameter = attributeSetNode.getParameter('numberValue')

        if numberValueParameter is None:
            log.error('Could not get numberValue array parameter.')

        #######################################################################
        # Create parameter policies from the parameters and register callbacks
        # to be notified if anything changes in the underlying parameter.
        #
        # This is your Controller in the MVC pattern.
        #######################################################################
        CreateParameterPolicy = UI4.FormMaster.CreateParameterPolicy
        self.__locationParameterPolicy = CreateParameterPolicy(
            None, locationParameter)
        self.__locationParameterPolicy.addCallback(
            self.locationParameterChangedCallback)

        self.__filePathParameterPolicy = CreateParameterPolicy(
            None, filePathParameter)
        self.__filePathParameterPolicy.addCallback(
            self.filePathParameterChangedCallback)

        self.__numberValueParameterPolicy = CreateParameterPolicy(
            None, numberValueParameter)
        self.__numberValueParameterPolicy.addCallback(
            self.numberValueParameterChangedCallback)

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
        locationWidget = WidgetFactory.buildWidget(
            self, self.__locationParameterPolicy)
        filePathWidget = WidgetFactory.buildWidget(
            self, self.__filePathParameterPolicy)
        numberValueArrayWidget = WidgetFactory.buildWidget(
            self, self.__numberValueParameterPolicy)

        # Create a custom widget that hasn't come from Katana's widget factory
        self.__simpleImageWithOverlayWidget = SimpleImageWithOverlayWidget(
            self, filePathParameter.getValue(0))

        # Create a layout and add the parameter editing widgets to it
        mainLayout = QtGui.QVBoxLayout()
        mainLayout.addWidget(locationWidget)
        mainLayout.addWidget(filePathWidget)
        mainLayout.addWidget(self.__simpleImageWithOverlayWidget)
        mainLayout.addWidget(numberValueArrayWidget)

        # Apply the layout to the widget
        self.setLayout(mainLayout)

    # Public Functions --------------------------------------------------------

    def updateNumberValues(self, x, y):
        """
        This will be called by our custom "SimpleImageWithOverlayWidget" when
        the user clicks the image.

        We'll use the information provided to update the parameter values on
        our AttributeSet node.
        """
        self.__numberValueParameterPolicy.setValue([x, y], 0)

    def filePathParameterChangedCallback(self, *args, **kwds):
        """
        Called by the filePathParameterPolicy when ever a modification is made
        to the underlying filePath parameter data.

        We pass the new file path to our custom widget so it can display the
        new image.
        """
        # Update our custom widget to display the new image
        filePath = self.__filePathParameterPolicy.getValue()
        self.__simpleImageWithOverlayWidget.displayImage(filePath)

    def locationParameterChangedCallback(self, *args, **kwds):
        """
        This function is called by our locationParameterPolicy whenever the
        underlying parameter changes.

        The LocationCreate and AttributeSet node's both expose a parameter on
        their own node interface which determine which scene graph location to
        operate on.

        Our SuperTool exposes its own location parameter and this function
        will keep the two nodes' values in sync.
        """
        # Get the new value
        locationPath = self.__locationParameterPolicy.getValue()

        # Get references to the nodes
        locationCreateNode = SA.GetRefNode(self.__node, 'locationcreate')
        attributeSetNode = SA.GetRefNode(self.__node, 'attributes')

        # Set these parameters directly (as opposed to via a policy)
        lcLocationsParameterArray = locationCreateNode.getParameter(
            'locations')
        lcLocationsParameterArray.getChildByIndex(0).setValue(locationPath, 0)

        asPathsParameterArray = attributeSetNode.getParameter('paths')
        asPathsParameterArray.getChildByIndex(0).setValue(locationPath, 0)

    def numberValueParameterChangedCallback(self, *args, **kwds):
        """
        This is called by our numberValueParameterPolicy whenever the
        underlying parameter changes.

        We pass this information to our custom widget so it can draw the
        appropriate overlay.
        """
        x, y = self.__numberValueParameterPolicy.getValue()
        self.__simpleImageWithOverlayWidget.setPointToDraw(x, y)

