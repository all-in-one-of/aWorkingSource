from Katana import QtCore, QtGui, UI4, QT4Widgets, QT4FormWidgets
from Katana import NodegraphAPI, Nodes3DAPI, AssetAPI, Plugins

import ScriptActions as SA

import logging
log = logging.getLogger('GafferThreeRigs')

# Class Definitions -----------------------------------------------------------

class GafferThreeRigsEditor(QtGui.QWidget):
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
        rootLocationsParameter = self.__node.getParameter('rootLocations')
        threePointLightingParameter = self.__node.getParameter('LightTypes')
        settingParameter = self.__node.getParameter('Setting')
        importLightSetsParameter = self.__node.getParameter("importRigsSet")
        updateLightSetsParameter = self.__node.getParameter("updateParameters")

        #######################################################################
        # Create parameter policies from the parameters and register callbacks
        # to be notified if anything changes in the underlying parameter.
        #
        # This is your Controller in the MVC pattern.
        #######################################################################
        CreateParameterPolicy = UI4.FormMaster.CreateParameterPolicy

        self.__rootLocationsParameterPolicy = CreateParameterPolicy(
            None, rootLocationsParameter)
        self.__rootLocationsParameterPolicy.addCallback(
            self.rootLocationsParameterChangedCallback)

        self.__threePointLightingParameterPolicy = CreateParameterPolicy(
            None, threePointLightingParameter)

        self.__settingParameterPolicy = CreateParameterPolicy(
            None, settingParameter)

        self.__importLightSetsPolicy = CreateParameterPolicy(
                None, importLightSetsParameter)

        self.__updateLightSetsPolicy = CreateParameterPolicy(
                None, updateLightSetsParameter)

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
        rootLocationsWidget = WidgetFactory.buildWidget(
            self, self.__rootLocationsParameterPolicy)

        threePointLightingWidget = WidgetFactory.buildWidget(
            self, self.__threePointLightingParameterPolicy)

        settingWidget = WidgetFactory.buildWidget(
            self, self.__settingParameterPolicy)

        importLightSetsWidget = WidgetFactory.buildWidget(
            self, self.__importLightSetsPolicy)

        updateLightSetsWidget = WidgetFactory.buildWidget(
            self, self.__updateLightSetsPolicy)

        mainLayout = QtGui.QVBoxLayout()
        mainLayout.addWidget(rootLocationsWidget)
        mainLayout.addWidget(threePointLightingWidget)
        mainLayout.addWidget(settingWidget)
        mainLayout.addWidget(importLightSetsWidget)
        mainLayout.addWidget(updateLightSetsWidget)

        # Apply the layout to the widget
        self.setLayout(mainLayout)

    def rootLocationsParameterChangedCallback(self, *args, **kwds):
        root_locations = self.__node.getParameter('rootLocations')
        groupStackNode = SA.GetRefNode(self.__node, 'groupStack')
        opScriptNode = SA.GetRefNode(self.__node, 'opScript')
        location_list = []
        for location in root_locations.getChildren():
            location_value = location.getValue(0)           
            if location_value is not "":
                location_list.append(location_value)

        for child in groupStackNode.getChildNodes():
            groupStackNode.deleteChildNode(child)

        cel_string = ""
        Producer = Nodes3DAPI.GetRenderProducer(self.__node, 0, False, 0)
        for location in location_list:
            cel_string = cel_string + " " + location
            gnode = groupStackNode.buildChildNode(adoptNode=None)
            gnode.getParameter("paths.i0").setValue(location, 0)
            gnode.getParameter("attributeName").setValue("LightChaserAnimation.lightRig.assetName",0)
            gnode.getParameter("attributeType").setValue("string",0)
            gnode.getParameter('stringValue.i0').setValue(location,0)

        opScriptNode.getParameter('CEL').setValue(cel_string, 0)