# Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import NodegraphAPI, Decorators, Plugins
import PackageSuperToolAPI.NodeUtils as NU
from PackageSuperToolAPI import Packages

import os
import textwrap
import logging

log = logging.getLogger("GafferThree.SkyDomePackage")

_iconsDir = os.path.join(os.path.dirname(__file__), 'Icons')

# Get base classes for our packages from the registered GafferThree packages
GafferThreeAPI = Plugins.GafferThreeAPI
LightPackage = GafferThreeAPI.PackageClasses.LightPackage
LightEditPackage = GafferThreeAPI.PackageClasses.LightEditPackage


class SkyDomePackage(LightPackage):
    """
    Implements a Sky Dome package. We inherit LightPackage, and override some
    functions to modify behaviour.
    """

    # Class Variables ---------------------------------------------------------

    # The name of the package type as it should be shown in the UI
    DISPLAY_NAME = 'Sky Dome'

    # The default name of a package when it is created. This also defines the
    # default name of the package's scene graph location
    DEFAULT_NAME = 'skyDome'

    # The icon to use to represent this package type in the UI
    DISPLAY_ICON = os.path.join(_iconsDir, 'skyDome16.png')

    # The default size of the sky dome
    DEFAULT_SIZE = 1000

    # The default location of the baked skydome light
    DEFAULT_BAKED_LIGHT_FILENAME = '/tmp/hdriSkyDomeLight.klf'
    DEFAULT_BAKED_LIGHT_NAME = 'hdriSkyDomeLight'

    # Class Functions ---------------------------------------------------------

    @classmethod
    def create(cls, enclosingNode, locationPath):
        """
        A factory method which returns an instance of the class.

        @type enclosingNode: C{NodegraphAPI.Node}
        @type locationPath: C{str}
        @rtype: L{LightPackage}
        @param enclosingNode: The parent node within which the new
            package's node should be created.
        @param locationPath: The path to the location to be created/managed
            by the package.
        @return: The newly-created package instance.
        """
        # Create the package node
        packageNode = NodegraphAPI.CreateNode('Group', enclosingNode)
        packageNode.addOutputPort('out')

        # Add parameter containing the package type and location path to the
        # package node
        NU.AddPackageTypeAndPath(packageNode, cls.__name__, locationPath)

        # Create an expression to link the name of the sky dome location to the
        # name of the package.
        locExpr = '=^/%s' % NU.GetPackageLocationParameterPath()

        # Create geometry for the light - in this case a sphere
        createNode = NodegraphAPI.CreateNode('PrimitiveCreate', packageNode)
        createNode.getParameter('type').setValue('coordinate system sphere', 0)
        createNode.getParameter('transform.scale.x').setValue(
            cls.DEFAULT_SIZE, 0)
        scaleExpr = "=transform.scale.x"
        createNode.getParameter('transform.scale.y').setExpression(scaleExpr)
        createNode.getParameter('transform.scale.z').setExpression(scaleExpr)
        createNode.getParameters().createChildNumber('forceAsStaticScene', 1)

        createNodeExtraAttrsParameter = \
            createNode.getParameters().createChildGroup('extraAttrs')
        createNodeExtraAttrsParameter.createChildString(
            '__gafferPackage', '').setExpression('@%s' % packageNode.getName())
        createNode.getParameter('name').setExpression(locExpr)
        createNode.addInputPort('masterMaterial')

        # Store the package class as a parameter on the create node
        NU.SetOrCreateDeepScalarParameter(
            createNode.getParameters(), 'extraAttrs.info.gaffer.packageClass',
            cls.__name__)

        # Set the type of the package location to "light", so that the Gaffer
        # recognizes it as such
        typeAttrSetNode = NodegraphAPI.CreateNode('AttributeSet', packageNode)
        typeAttrSetNode.setName("SetTypeAttributeSet")
        typeAttrSetNode.getParameter('paths.i0').setExpression(locExpr)
        typeAttrSetNode.getParameter('attributeName').setValue('type', 0)
        typeAttrSetNode.getParameter('attributeType').setValue('string', 0)
        typeAttrSetNode.getParameter('stringValue.i0').setValue('light', 0)

        # Set the "viewer.locationType" attribute on the package location to
        # "nurbspatch", so that the viewer knows to display it as geometry
        viewerTypeAttrSetNode = NodegraphAPI.CreateNode('AttributeSet',
                                                        packageNode)
        viewerTypeAttrSetNode.setName("SetViewerTypeAttributeSet")
        viewerTypeAttrSetNode.getParameter('paths.i0').setExpression(locExpr)
        viewerTypeAttrSetNode.getParameter('attributeName').setValue(
            'viewer.locationType', 0)
        viewerTypeAttrSetNode.getParameter('attributeType').setValue(
            'string', 0)
        viewerTypeAttrSetNode.getParameter('stringValue.i0').setValue(
            'nurbspatch', 0)

        # Add the package location to the light list at /root
        lightListEditNode = NodegraphAPI.CreateNode('LightListEdit',
                                                     packageNode)
        lightListEditNode.setName("LightListEdit")
        lightListEditNode.getParameter('locations.i0').setExpression(locExpr)

        # Create the Material node
        materialNode = NodegraphAPI.CreateNode('Material', packageNode)
        materialNode.getParameter('action').setValue('edit material', 0)
        materialNode.getParameter('makeInteractive').setValue('Yes', 0)
        materialNode.getParameters().createChildNumber(
            'makeInteractiveSpecificToMaterial', 1)
        materialNode.getParameter('edit.location').setExpression(locExpr)

        # Create an OpScript node to copy the preview texture from the shader
        # to the sphere
        copyPreviewTextureOpScriptNode = cls.__createOpScriptNode(
            packageNode, 'CopyPreviewTextureOpScript',
            cls.__getPreviewTextureOpScript())

        # Create an OpScript node to copy the transform from the sphere to the
        # shader
        copyXformOpScriptNode = cls.__createOpScriptNode(
            packageNode, 'CopyXformOpScript', cls.__getCopyXformOpScript())

        # Create an OpScript node to flip the UVs in the viewer
        viewerUVFlipOpScriptNode = cls.__createOpScriptNode(
            packageNode, 'ViewerUVFlipOpScript',
            cls.__getViewerUVFlipOpScript())

        # Create a DistantPort node to reference the master material
        masterMaterialDistantPortNode = NodegraphAPI.CreateNode('DistantPort',
                                                                packageNode)
        masterMaterialDistantPortNode.setBypassed(True)

        # Create a ViewerObjectSettings node in case we want to customize how
        # the skydome is displayed in the viewer
        viewerObjectSettingsNode = NodegraphAPI.CreateNode(
            'ViewerObjectSettings', packageNode)
        viewerObjectSettingsNode.getParameter('CEL').setExpression(locExpr)

        # Add node references to the package node
        NU.AddNodeRef(packageNode, 'create', createNode)
        NU.AddNodeRef(packageNode, 'lightListEdit', lightListEditNode)
        NU.AddNodeRef(packageNode, 'material', materialNode)
        NU.AddNodeRef(packageNode, 'master', masterMaterialDistantPortNode)
        NU.AddNodeRef(packageNode, 'viewerObjectSettings',
                      viewerObjectSettingsNode)

        # Wire up and position the nodes
        NU.WireInlineNodes(packageNode, (masterMaterialDistantPortNode,
                                         createNode,
                                         typeAttrSetNode,
                                         viewerTypeAttrSetNode,
                                         lightListEditNode,
                                         materialNode,
                                         copyPreviewTextureOpScriptNode,
                                         copyXformOpScriptNode,
                                         viewerUVFlipOpScriptNode,
                                         viewerObjectSettingsNode))

        # Create and append light linking nodes
        linkingNodes = Packages.LinkingMixin.getLinkingNodes(packageNode,
                                                             create=True)
        NU.AppendNodes(packageNode, tuple(linkingNode
                                          for linkingNode in linkingNodes
                                          if linkingNode is not None))
        # Create a package node instance
        result = cls(packageNode)
        Packages.CallbackMixin.executeCreationCallback(result)

        # Create a post-merge stack node for this package
        postMergeNode = result.createPostMergeStackNode()

        # Use an AttributeSet node to set Arnold's background attribute at root
        arnoldBGAttrSetNode = NodegraphAPI.CreateNode('AttributeSet',
                                                      packageNode)
        arnoldBGAttrSetNode.setName("ArnoldBGAttributeSet")
        arnoldBGAttrSetNode.getParameter('paths.i0').setValue('/root', 0)
        arnoldBGAttrSetNode.getParameter('attributeName').setValue(
            'arnoldGlobalStatements.background', 0)
        arnoldBGAttrSetNode.getParameter('attributeType').setValue(
            'string', 0)
        arnoldBGAttrSetNode.getParameter('stringValue.i0').setExpression(
            'getParam("%s.__gaffer.location")' % packageNode.getName())
        postMergeNode.buildChildNode(adoptNode=arnoldBGAttrSetNode)

        # Set our material to point at a baked Look File material
        result.setLookFileMaterial(
            cls.DEFAULT_BAKED_LIGHT_FILENAME,
            "/root/materials/%s" % cls.DEFAULT_BAKED_LIGHT_NAME)

        return result

    @classmethod
    def createShadingNetwork(cls, parentNode=None, xPos=0, yPos=0):
        """
        Convenience function to create the shading network for our SkyDome.

        Can be called from inside Katana using:
        C{Plugins.GafferThreeAPI.PackageClasses.SkyDomePackage.createShadingNetwork()}
        """
        if parentNode is None:
            parentNode = NodegraphAPI.GetRootNode()

        # Create shading network
        imageNode = NodegraphAPI.CreateNode('ArnoldShadingNode', parentNode)
        imageNode.getParameter('name').setValue('skyDomeImage', 0)
        NodegraphAPI.SetNodePosition(imageNode, (xPos, yPos+120))
        imageNode.getParameter('nodeType').setValue('image', 0)
        imageNode.checkDynamicParameters()

        lightNode = NodegraphAPI.CreateNode('ArnoldShadingNode', parentNode)
        lightNode.getParameter('name').setValue('skyDomeLight', 0)
        NodegraphAPI.SetNodePosition(lightNode, (xPos, yPos+80))
        lightNode.getParameter('nodeType').setValue('skydome_light', 0)
        lightNode.checkDynamicParameters()
        lightNode.getInputPort('color').connect(imageNode.getOutputPort('out'))

        hdriLightNode = NodegraphAPI.CreateNode('NetworkMaterial', parentNode)
        hdriLightNode.getParameter('name').setValue('hdriSkyDomeLight', 0)
        NodegraphAPI.SetNodePosition(hdriLightNode, (xPos, yPos+40))
        hdriLightNode.addInputPort('arnoldSurface')
        hdriLightNode.getInputPort('arnoldSurface').connect(
            lightNode.getOutputPort('out'))

        lookFileMaterialsOutNode = NodegraphAPI.CreateNode(
            'LookFileMaterialsOut', parentNode)
        lookFileMaterialsOutNode.setName('hdriSkiDomeLight_bake')
        NodegraphAPI.SetNodePosition(lookFileMaterialsOutNode, (xPos, yPos))
        lookFileMaterialsOutNode.getParameter('saveTo').setValue(
            cls.DEFAULT_BAKED_LIGHT_FILENAME, 0)
        lookFileMaterialsOutNode.getInputPort('in').connect(
            hdriLightNode.getOutputPort('out'))

        # Expose shading node parameters on the network material
        cls.__exposeShaderParameter(imageNode, 'filename', 'HDRI_map',
                                    {'widget': 'fileInput'})
        cls.__exposeShaderParameter(imageNode, 'sflip')
        cls.__exposeShaderParameter(imageNode, 'tflip')
        cls.__exposeShaderParameter(imageNode, 'multiply', 'color')
        cls.__exposeShaderParameter(lightNode, 'matrix')
        cls.__exposeShaderParameter(lightNode, 'intensity')
        cls.__exposeShaderParameter(lightNode, 'exposure')
        cls.__exposeShaderParameter(lightNode, 'samples')

        return lookFileMaterialsOutNode

    # Public Instance Functions -----------------------------------------------

    def setHDRIMap(self, hdriMap, asExpression=False):
        """
        Convenience function for setting the HDRI map on our shader.

        @type hdriMap: C{str}
        @type asExpression: C{bool}
        @param hdriMap: The path to an HDRI map asset, or a parameter
            expression linking to such a path.
        @param asExpression: A flag indicating whether the given C{hdriMap}
            value should be treated as a parameter expression.
        """
        materialNode = self.getMaterialNode()
        hdriMapParameter = materialNode.getParameter(
            "shaders.parameters.HDRI_map")

        if not hdriMapParameter:
            log.warning('"HDRI_map" parameter not found on material node for '
                        'package "%s".' % self.getName())
            return

        if asExpression:
            hdriMapParameter.setExpression(hdriMap)
        else:
            hdriMapParameter.setValue(hdriMap, 0.0)

    # Private Class Functions -------------------------------------------------

    @classmethod
    def __exposeShaderParameter(cls, node, parameterName, exposedName=None,
                                extraHints={}):
        """
        Sets the 'dstName' hint on the specified parameter so that it will be
        exposed in a network material interface. Optionally add extra hints,
        e.g. for selecting widget type.
        """
        parameter = node.getParameter('parameters.%s' % parameterName)
        if not parameter:
            return

        # Extract any existing hints on the parameter, and add any given extra
        # hints
        hintsDict = {}
        hintsParameter = parameter.getChild('hints')
        if hintsParameter:
            hintsParameterValue = hintsParameter.getValue()
            try:
                hintsDict = eval(hintsParameterValue)
            except:
                hintsDict = {}
        else:
            hintsParameter = parameter.createChildString('hints', '')

        hintsDict.update(extraHints)

        # Set the exposed name in the 'dstName' hint
        if exposedName is None:
            exposedName = parameterName
        hintsDict['dstName'] = exposedName

        # Set the hints parameter
        hintsParameter.setValue(hintsDict.__str__(), 0)

    @classmethod
    def __createOpScriptNode(cls, parentNode, nodeName, opScript):
        """
        Creates and returns an OpScript node with the given name, under the
        given parent node, and sets the text of the OpScript to the given Lua
        code. The OpScript node will be set to run at the location of this
        package.

        @type parentNode: C{NodegraphAPI.Node}
        @type nodeName: C{str}
        @type opScript: C{str}
        @rtype: C{NodegraphAPI.Node}
        @param parentNode: The enclosing Group node under which to create the
            new OpScript node.
        @param nodeName: The name to give the new OpScript node.
        @param opScript: The text of the Lua code for the OpScript node.
        @return: The newly created OpScript node.
        """
        opScriptNode = NodegraphAPI.CreateNode('OpScript', parentNode)
        opScriptNode.setName(nodeName)
        opScriptNode.getParameter('CEL').setExpression("=^/__gaffer.location")
        opScriptNode.getParameter('script.lua').setValue(opScript, 0)
        return opScriptNode

    @classmethod
    def __getPreviewTextureOpScript(cls):
        return textwrap.dedent(
            """
            -- Get the path to the HDRI map from the material attributes
            local previewTextureAttr = Interface.GetAttr("material.parameters.HDRI_map")
            local previewTextureValue = (previewTextureAttr ~= nil) and previewTextureAttr:getValue() or ""

            -- Copy the path to the preview material attribute if it isn't blank
            if (previewTextureValue ~= "")
            then
                Interface.SetAttr("previewMaterial.texture", previewTextureAttr)
            end
            """).strip()

    @classmethod
    def __getCopyXformOpScript(cls):
        return textwrap.dedent(
            """
            -- Get the global xform for the current location
            local xformGrp = Interface.GetGlobalXFormGroup()
            local xformMatrix = XFormUtils.CalcTransformMatrixAtTime(xformGrp, 0.0)

            -- Copy the xform to the shader's matrix parameter
            Interface.SetAttr("material.parameters.matrix", xformMatrix)
            """).strip()

    @classmethod
    def __getViewerUVFlipOpScript(cls):
        return textwrap.dedent(
            """
            -- OpScript that sets the geometry.viewerFlipU and
            -- geometry.viewerFlipV values for the representation of the
            -- skydome in the viewer, based on the values of 'sflip' and
            -- 'tflip' from the material.

            -- This is hardcoded to assume that the default values for sflip
            -- and tflip are 0. It might be better (but less efficient) if we
            -- were to get those default values from the material itself.

            -- From comparing the results from using Arnold's skydome_light
            -- shader and what is seen in the viewer, we set the viewer's FlipU
            -- to be the inverse of the shader's sflip values.

            local materialSflipAttr = Interface.GetAttr("material.parameters.sflip")
            local materialSflipValue = (materialSflipAttr ~= nil) and materialSflipAttr:getValue() or 0
            Interface.SetAttr("geometry.viewerFlipU", IntAttribute(1 - materialSflipValue))

            local materialTflipAttr = Interface.GetAttr("material.parameters.tflip")
            Interface.SetAttr("geometry.viewerFlipV", materialTflipAttr)
            """).strip()


class SkyDomeEditPackage(LightEditPackage):
    """
    The edit package that allows a GafferThree to edit an existing SkyDome
    package present in the input Scenegraph.

    This package uses a TransformEdit node to edit the SkyDome's transform.
    """

    # Class Variables ---------------------------------------------------------

    DISPLAY_ICON = os.path.join(_iconsDir, "skyDome16.png")

    # Class Functions ---------------------------------------------------------

    @classmethod
    def create(cls, enclosingNode, locationPath):
        """
        Creates the contents of the EditStackNode that contains the edit nodes.
        This could be any other kind of node with at least one input and one
        output, but the createPackageEditStackNode() helper function does all
        of the configuration boilerplate code of an EditStackNode for you.
        The return value is a SkyDomeEditPackage instance.

        This particular package node will contain a TransformEdit node on it,
        which will allow to edit the transform of a skyDome.
        """
        # Create the package node. Since this is an edit package we want to use
        # an EditStackNode instead of a GroupNode, since it already has an
        # input and an output by default. This also adds some necessary
        # parameters to this node.
        packageNode = cls.createPackageEditStackNode(enclosingNode,
                                                     locationPath)

        # Build material edit node
        materialNode = NodegraphAPI.CreateNode('Material', packageNode)
        actionParam = materialNode.getParameter('action')
        actionParam.setValue('edit material', 0)

        editLocationParam = materialNode.getParameter('edit.location')
        editLocationParam.setExpression('=^/__gaffer.location')
        editLocationParam.setExpressionFlag(True)
        NU.AddNodeRef(packageNode, 'material_edit', materialNode)

        packageNode.buildChildNode(adoptNode=materialNode)

        # Build transform edit node
        transformEditNode = NodegraphAPI.CreateNode('TransformEdit',
                                                    packageNode)
        actionParam = transformEditNode.getParameter('action')
        actionParam.setValue('override interactive transform', 0)

        pathParam = transformEditNode.getParameter('path')
        pathParam.setExpression('=^/__gaffer.location')
        pathParam.setExpressionFlag(True)

        # Adds reference parameters to the transform edit node
        NU.AddNodeRef(packageNode, 'transform_edit', transformEditNode)

        # Add the transform edit node into the package node using
        # EditStackNode's buildChildNode().
        packageNode.buildChildNode(adoptNode=transformEditNode)

        # Instantiate a package with the package node
        return cls.createPackage(packageNode)

    @classmethod
    def getAdoptableLocationTypes(cls):
        """
        Returns the set of location types adoptable by this Package. In this
        case, the package can edit locations created by SkyDomePackages, which
        are of the type light.
        """
        return set(('light',))

    # Instance Functions ------------------------------------------------------

    @Decorators.undogroup('Delete SkyDomeEdit Package')
    def delete(self):
        LightEditPackage.delete(self)


# Register the package classes, and associate the edit package class with the
# create package class
GafferThreeAPI.RegisterPackageClass(SkyDomePackage)
GafferThreeAPI.RegisterPackageClass(SkyDomeEditPackage)
SkyDomePackage.setEditPackageClass(SkyDomeEditPackage)
