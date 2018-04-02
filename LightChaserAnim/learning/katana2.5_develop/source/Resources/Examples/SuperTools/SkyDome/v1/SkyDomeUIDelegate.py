# Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.

from PackageSuperToolAPI import UIDelegate
from PackageSuperToolAPI import NodeUtils as NU
from PackageSuperToolAPI import Packages
from SkyDomePackage import (SkyDomePackage, SkyDomeEditPackage)
from Katana import QT4FormWidgets, FormMaster, Plugins

# Get the base classes for our UI delegate classes from the PackageSuperToolAPI
# using the base classes of our custom Sky Dome Package classes
GafferThreeAPI = Plugins.GafferThreeAPI
LightUIDelegate = UIDelegate.GetUIDelegateClassForPackageClass(
    GafferThreeAPI.PackageClasses.LightPackage)
LightEditUIDelegate = UIDelegate.GetUIDelegateClassForPackageClass(
    GafferThreeAPI.PackageClasses.LightEditPackage)


class SkyDomeUIDelegate(LightUIDelegate):
    """
    The UI delegate for the SkyDome package.

    This class is responsible for exposing the parameters on each of the
    parameter tabs. This is done by creating parameter policies attached to the
    parameters on the package's nodes. We can also modify the appearance of the
    parameter tabs by modifying the hints dictionaries on the policies.
    """

    # The hash used to uniquely identify the action of creating a package
    # This was generated using:
    #     hashlib.md5('SkyDome.AddSkyDome').hexdigest()
    AddPackageActionHash = 'f1765c35808868c77019cebd796f14b7'

    # The keyboard shortcut for creating a package
    DefaultShortcut = 'Alt+S'

    def getTabPolicy(self, tabName):
        """
        The main method of a UIDelegate. This is responsible for returning a
        policy instance for each tab. The policy will contain other policies
        that should drive the actual package node's parameters.
        """
        if tabName == "Object":
            return self.__getObjectTabPolicy()
        elif tabName == "Linking":
            return self.__getLinkingTabPolicy()
        else:
            return LightUIDelegate.getTabPolicy(self, tabName)

    def __getObjectTabPolicy(self):
        """
        Returns the widget that should be displayed under the 'Object' tab.
        """
        # Get the create node in the package, which contains the transform
        # parameter.
        packageNode = self.getPackageNode()
        createNode = NU.GetRefNode(packageNode, "create")
        if createNode is None:
            return None

        # Create a root group policy and add some hints on it
        rootPolicy = QT4FormWidgets.PythonGroupPolicy('object')
        rootPolicy.getWidgetHints()['open'] = True
        rootPolicy.getWidgetHints()['hideTitle'] = True

        transformPolicy = QT4FormWidgets.PythonGroupPolicy('transform')
        transformPolicy.getWidgetHints()['open'] = True

        translatePolicy = FormMaster.CreateParameterPolicy(
            None, createNode.getParameter("transform.translate"))
        rotatePolicy = FormMaster.CreateParameterPolicy(
            None, createNode.getParameter("transform.rotate"))
        scalePolicy = FormMaster.CreateParameterPolicy(
            None, createNode.getParameter("transform.scale"))

        transformPolicy.addChildPolicy(translatePolicy)
        transformPolicy.addChildPolicy(rotatePolicy)
        transformPolicy.addChildPolicy(scalePolicy)

        rootPolicy.addChildPolicy(transformPolicy)

        return rootPolicy

    def __getLinkingTabPolicy(self):
        return LightUIDelegate.GetLightLinkingTabPolicy(
            self.getReferencedNode("node_lightLink_illumination"),
            self.getReferencedNode("node_lightLink_shadow"),
            self.getReferencedNode("node_lightListEdit"))


class SkyDomeEditUIDelegate(LightEditUIDelegate):
    """
    The UI delegate for the SkyDomeEdit package.
    """

    def getTabPolicy(self, tabName):
        """
        The main method of a UIDelegate. This is responsible for returning a
        Value Policy for each tab. The Value Policy will contain other policies
        that should drive the actual package node's parameters.
        """
        if tabName == "Object":
            return self.__getObjectTabPolicy()
        else:
            return LightEditUIDelegate.getTabPolicy(self, tabName)

    def __getObjectTabPolicy(self):
        """
        Returns the widget that should be displayed under the 'Object' tab.
        """
        # Works similarly to SkyDomeUIDelegate.__getObjectTabPolicy, but uses
        # the TransformEdit to modify the transform.
        packageNode = self.getPackageNode()
        transfromEditNode = NU.GetRefNode(packageNode, "transform_edit")

        if transfromEditNode is None:
            return None

        rootPolicy = QT4FormWidgets.PythonGroupPolicy('object')
        rootPolicy.getWidgetHints()['open'] = True
        rootPolicy.getWidgetHints()['hideTitle'] = True

        # Add these three top-level parameters in TransformEdit to the root
        # policy.
        for paramName in ('action', 'rotationOrder', 'args'):
            parameter = transfromEditNode.getParameter(paramName)
            policy = FormMaster.CreateParameterPolicy(rootPolicy,
                                                      parameter)
            rootPolicy.addChildPolicy(policy)

        return rootPolicy


# Register the UI delegates

UIDelegate.RegisterUIDelegateClass(SkyDomePackage, SkyDomeUIDelegate)
UIDelegate.RegisterUIDelegateClass(SkyDomeEditPackage, SkyDomeEditUIDelegate)

