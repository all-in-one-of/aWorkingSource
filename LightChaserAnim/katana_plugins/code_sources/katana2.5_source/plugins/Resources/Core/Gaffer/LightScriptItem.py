# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.
from Katana import Plugins
from Katana import NodegraphAPI
from Katana import GeoAPI
from Katana import Utils
import Katana
import os
import FnKatImport
import logging


GafferAPI = Plugins.GafferAPI
ScriptItems = GafferAPI.ScriptItems
SA = GafferAPI.ScriptActions
C = GafferAPI.Constants
Resources = GafferAPI.Resources
deprecated = Utils.Decorators.deprecated

"""
Custom Gaffer ScriptItem
-------------------------
    There are 3 classes related to creating a new Gaffer plugin the ScriptItem,
    the AddLocationAction and the UIItem

    The ScriptItem is the main class, which controls how the Gaffer item is set
    up internally & controls the items behaviour

    The AddLocationAction is only used by the GUI and creates the "Add..." menu
    item for the ScriptItem. This includes setting the menu name, and how it
    should be grouped in the menu

    The UIItem is also only used by the UI and controls how the 'Object', 'Material'
    and 'Linking' tabs are set up within the gaffer UI

 - Register()      : Registers the ScriptItem whether in GUI, script or batch mode
 - RegisterGUI()   : Only called when in GUI mode to register the AddLocationAction
                     and UIItem. Anything that uses Qt or UI4 modules, should be set
                     up below RegisterGUI as these will not be available in
                     batch / script mode

"""
#-------------------------------------
# Customisation Constants
PackageName = 'GafferProLightPackage'

# Some other script items like to use the GafferLightUIItem static functions
GafferLightUIItem = None

#//////////////////////////////////////////////////////////////////////////////
#///

log = logging.getLogger("Gaffer.LightScriptItem")

class GafferLightScriptItem(ScriptItems.GafferScriptItem):
    @staticmethod
    def create(gnode, path):
        lightGroup = SA.BuildLocationPackageForPath(gnode, path)

        SA.AddNodeReferenceForPath(gnode, path, 'package', lightGroup)

        lightGroup.setName(os.path.basename(path) + '_package')
        lightGroup.setType(PackageName)

        nodes = []

        lightCreate = NodegraphAPI.CreateNode('LightCreate', lightGroup)
        lightCreate.addInputPort('in')

        lightCreate.getParameter('name').setExpression('getParent().pathName')

        nodes.append(lightCreate)

        material = NodegraphAPI.CreateNode('Material', lightGroup)
        material.getParameter('action').setValue('edit material', 0)
        material.getParameter('makeInteractive').setValue('Yes', 0)
        material.getParameter('edit.location').setExpression('str(getParent().pathName)+"/master/material"')

        nodes.append(material)

        aimConstraint = NodegraphAPI.CreateNode('AimConstraint', lightGroup)
        aimConstraint.getParameter('basePath').setExpression('getParent().pathName')
        p = aimConstraint.getParameter('addToConstraintList')
        if p: p.setValue(1, 0)
        aimConstraint.setBypassed(True)

        nodes.append(aimConstraint)

        viewerSettings = None
        try:
            # This node is optional - if we're in an older version
            # of Katana it doesn't matter if it's not created
            viewerSettings = NodegraphAPI.CreateNode('ViewerObjectSettings',
                                                     lightGroup)
            viewerSettings.getParameter('CEL').setExpression(
                                                    'getParent().pathName')
            colorParam = viewerSettings.getParameter(
                                'args.viewer.default.annotation.color.value')
            colorParam.getChild('i0').setExpression(
                 'getParam("%s.previewColor.red")' % lightCreate.getName())
            colorParam.getChild('i1').setExpression(
                 'getParam("%s.previewColor.green")' % lightCreate.getName())
            colorParam.getChild('i2').setExpression(
                 'getParam("%s.previewColor.blue")' % lightCreate.getName())
            enableParam = viewerSettings.getParameter(
                                'args.viewer.default.annotation.color.enable')
            enableParam.setExpression(
            '(getParamRelative(self, "../value.i0"), getParamRelative(self, "../value.i1"), getParamRelative(self, "../value.i2")) != (1.0, 1.0, 1.0)')
            nodes.append(viewerSettings)

        except RuntimeError:
            pass

        SA.WireInlineNodes(lightGroup, nodes)

        SA.AddNodeReferenceParam(lightGroup, 'node_lightCreate', lightCreate)
        SA.AddNodeReferenceParam(lightGroup, 'node_material', material)
        SA.AddNodeReferenceParam(lightGroup, 'node_aimConstraint', aimConstraint)
        if viewerSettings:
            SA.AddNodeReferenceParam(lightGroup,
                                'node_viewerObjectSettings', viewerSettings)

        SA.BuildLookFileMaterialParametersForPath(gnode,
                path+'/master/material')

        attrsParam = SA.GetAttributesParameterForPath(gnode, path, create=True)

        materialAssignParam = attrsParam.createChildString(
                'materialAssign', '')
        materialAssignParam.setExpression(
                '"/"+getParamRelative(self, "../../../master/material").'\
                'param.getFullName(False).replace(".", "/")')

        geoGroup = attrsParam.createChildGroup('geometry')

        if not lightCreate.getParameter('radius'):
            geoGroup.createChildNumber('radius', 1)

        postMerge = SA.GetPostMergeStack(gnode)

        postStack = postMerge.buildChildNode(
                adoptNode=NodegraphAPI.CreateNode('GroupStack', postMerge))

        postStack.setName(os.path.basename(path) + '_postPackage')


        defaultLink = lightCreate.getParameter('initialState')

        defaultShadowLink = lightCreate.getParameters().createChildGroup(
                'lightListDefaultsIfDisabled').createChildNumber(
                        'geoShadowEnable', 1)

        pathExpr = 'getNode(%r).pathName' % lightGroup.getName()
        SA.BuildLightLinkNodes(
            lightGroup, postStack, defaultLink, pathExpr,
            nodeReferenceParameterName='node_enable',
            lightLinkNodeName='Enable')
        SA.BuildLightLinkNodes(lightGroup, postStack, defaultShadowLink,
                pathExpr).getParameter('effect').setValue(
                        'shadow visibility', 0)

        SA.AddNodeReferenceForPath(gnode, path, 'postmerge', postStack)

    def getCreateNode(self):
        n = self.getNode()
        lc = SA.GetRefNode(n, 'lightCreate')
        return lc

    def getAbstractTransformNode(self):
        return self.getCreateNode()

    def getMaterialNode(self):
        n = self.getNode()
        m = SA.GetRefNode(n, 'material')
        return m

    def getAimConstraintNode(self):
        n = self.getNode()
        ac = SA.GetRefNode(n, 'aimConstraint')
        return ac

    def setAimTarget(self, path):
        ac = self.getAimConstraintNode()
        if not ac: return
        exp = SA.GetScenegraphPathExpression(self, path)
        if exp: ac.getParameter('targetPath.i0').setExpression(exp)
        else:   ac.getParameter('targetPath.i0').setValue(path, 0.0)
        ac.setBypassed(False)

    def getMasterMaterial(self):
        n = self.getNode()
        mmOs = SA.GetRefNode(n, 'mmOs')

        if not mmOs:
            return

        if mmOs.isBypassed():
            return

        return mmOs.getParameter('user.mmOs').getValue(0)

    def setMasterMaterial(self, masterMaterialPath):
        self.setLookFileMaterialEnabled(False)
        n = self.getNode()
        mmOs = SA.GetRefNode(n, 'mmOs')

        if not mmOs:
            if not masterMaterialPath:
                return

            # TODO: switch this to OpScript and upgrade existing Gaffer nodes

            mmOs = NodegraphAPI.CreateNode('OpScript', n)
            mmOs.setName('mmos')
            SA.AddNodeReferenceParam(n, 'node_mmOs', mmOs)

            material = SA.GetRefNode(n, 'material')

            x,y = NodegraphAPI.GetNodePosition(material)
            NodegraphAPI.SetNodePosition(mmOs, (x-100, y))

            mmOs.getInputPortByIndex(0).connect(
                    material.getInputPortByIndex(0).getConnectedPort(0))
            material.getInputPortByIndex(0).connect(
                    mmOs.getOutputPortByIndex(0))

            mmOs.getParameter('CEL').setExpression(
                    """str(getParent().pathName) + '/master'""")
            mmOs.getParameter('script.lua').setValue(
                    Resources.MASTERMATERIAL_OPSCRIPT, 0)

            NodegraphAPI.UserParameters.CreateString(mmOs, 'mmOs')

        if not masterMaterialPath:
            mmOs.setBypassed(True)
            return

        gp = self.getGafferPro()
        mmItem = gp.getScriptItemForPath(masterMaterialPath)

        mmScriptItemClass = ScriptItems.GetGafferScriptItemClass(
                'GafferProMasterMaterialPackage')
        if not isinstance(mmItem, mmScriptItemClass):
            raise TypeError("%r doesn't exist as master material" % (
                    masterMaterialPath))

        mmOs.getParameter('user.mmOs').setExpression(
                """getNode(%r).pathName""" % mmItem.getNode().getName())

        mmOs.setBypassed(False)

    def getMaterialPath(self):
        return self.getFullName()+'/master/material'

    def setShader(self, shaderName, asExpression=False):
        self.setMasterMaterial(None)
        self.setLookFileMaterialEnabled(False)
        n = self.getNode()
        m = SA.GetRefNode(n, 'material')
        profile = self.getProfile()
        shaderParam = profile.getMaterialParameterName()

        shadersParam = m.getParameter('shaders')
        if shadersParam.getChild(shaderParam) is None:
            m.addShaderType(shaderParam[:-6])

        if shaderName is None:
            m.getParameter('shaders.%s.enable' % shaderParam).setValue(0,0)
        else:
            m.getParameter('shaders.%s.enable' % shaderParam).setValue(1,0)
            if asExpression:
                m.getParameter('shaders.%s.value' % shaderParam).setExpression(shaderName)
            else:
                m.getParameter('shaders.%s.value' % shaderParam).setValue(shaderName,0)

        profile.shaderSelected(self)

    @deprecated('setShader()', log)
    def setArnoldShader(self, shaderName):
        self.setShader(shaderName)

    def getLookFileMaterialEnabled(self):
        lookFileParam = SA.GetLookFileMaterialEnabledParameter(self)
        if lookFileParam:
            return bool(lookFileParam.getValue(0))

        return False

    def setLookFileMaterialEnabled(self, state):
        SA.GetLookFileMaterialEnabledParameter(self).setValue(bool(state), 0)

    def setLookFileMaterial(self, spref, path, asExpression=False):
        if hasattr(self, 'setMasterMaterial'):
            self.setMasterMaterial(None)
        self.setLookFileMaterialEnabled(True)
        refParam = SA.GetLookFileMaterialReferenceParameter(self)
        if asExpression:
            refParam.getChild('asset').setExpression(spref)
            refParam.getChild('materialPath').setExpression(path)
        else:
            refParam.getChild('asset').setValue(spref, 0)
            refParam.getChild('materialPath').setValue(path, 0)

    def getLookFileMaterial(self):
        refParam = SA.GetLookFileMaterialReferenceParameter(self)
        return tuple(x.getValue(0) for x in refParam.getChildren())

    def acceptsChild(self, childScriptItem):
        return type(childScriptItem).__name__ == 'GafferAimTargetScriptItem'

    def supportsMuting(self):
        return True

    def supportsLocking(self):
        return True

    def isLocked(self):
        node = self.getNodeReference('node_lightCreate')
        interactiveParam = node.getParameter('makeInteractive')
        if interactiveParam:
            return node.getParameter('makeInteractive').getValue(0.0) != 'Yes'

        return False

    def setLocked(self, lock):
        node = self.getNodeReference('node_lightCreate')
        node.getParameter('makeInteractive').setValue(
                "No" if lock else "Yes", 0.0)

    def getLinkingExceptionsEnabled(self):
        return not self.getNodeReference('node_enable').isBypassed()

    def getShadowLinkingExceptionsEnabled(self):
        return not self.getNodeReference('node_geoShadowEnable').isBypassed()

    def setLinkingExceptionsEnabled(self, state):
        self.getNodeReference('node_enable').setBypassed(not state)

    def setShadowLinkingExceptionsEnabled(self, state):
        self.getNodeReference('node_geoShadowEnable').setBypassed(not state)

    def getColor(self, rootProducer = None):
        names = self.getProfile().getConstant('SHADER_COLOR_PARAM_NAMES')
        if names:
            attr = self.__findAttribute(names, rootProducer)
            if attr:
                return attr.getNearestSample(0.0)

    def getColorPolicy(self):
        from Katana import FormMaster, Utils
        names = self.getProfile().getConstant('SHADER_COLOR_PARAM_NAMES')
        param = self.__searchShaderParameters(names, edit=True)
        if param:
            policy = FormMaster.CreateParameterPolicy(None, param)

            #WORKAROUND ALERT!
            #
            # For a newly created material without its Material tab visible,
            # the policy we're about to create is frozen because it has
            # no callbacks (which would typically be added by thawing widgets)
            # But, the policies HAVE been created by the Material tab-- so the relevant
            # ScenegraphIncomingValueSource already exists but is in a frozen
            # state since none of those policies are thawed.
            #
            # That means that if we've changed the shader value since that
            # ScenegraphIncomingValueSource was created, it will have outdated results
            #
            # Policy creation doesn't care about that but as soon as something
            # using the policy queries the incoming attr value, it'll fail.
            #
            # By adding a dummy callback to our policy, we force it to thaw
            # temporarily. This indirectly wakes up ScenegraphIncomingValueSource
            # which will registered for ProducerObserver events. By processing
            # all events, we ensure that the ScenegraphIncomingValueSource has
            # accurate results when a recook is required. We can then remove
            # the callback as anything which cares about future changes to the
            # policy's incoming value will end up thawing it.
            def callback(event): pass
            policy.addCallback(callback)
            Utils.EventModule.ProcessAllEvents()
            policy.delCallback(callback)

            return policy

    def getIntensity(self, rootProducer = None):
        names = self.getProfile().getConstant('SHADER_INT_PARAM_NAMES')
        if names:
            attr = self.__findAttribute(names, rootProducer)
            if attr:
                return attr.getNearestSample(0.0)[0]

    def getExposure(self, rootProducer = None):
        names = self.getProfile().getConstant('SHADER_EXP_PARAM_NAMES')
        if names:
            attr = self.__findAttribute(names, rootProducer)
            if attr:
                return attr.getNearestSample(0.0)[0]

    def __findAttribute(self, names, rootProducer):
        if not rootProducer: return None
        producer = rootProducer.getProducerByPath(self.getFullName())
        if not producer: return None
        #material = producer.getGlobalAttribute('materialAssign')
        #if not material: return None
        #materialProducer = rootProducer.getProducerByPath(material.getNearestSample(0.0)[0])

        # GEOLIB3 We're resolving materials here, no need to walk to
        # the material location
        materialProducer = producer

        for name in names:
            attr = materialProducer.getGlobalAttribute('material.%s' % name)
            if attr: return attr

    def canEditColor(self):
        names = self.getProfile().getConstant('SHADER_COLOR_PARAM_NAMES')
        return bool(self.__searchShaderParameters(names, edit=True))

    def setColor(self, (r, g, b)):
        from Katana import FormMaster
        names = self.getProfile().getConstant('SHADER_COLOR_PARAM_NAMES')
        param = self.__searchShaderParameters(names, edit=True)
        if param:
            policy = FormMaster.CreateParameterPolicy(None, param)
            policy.getArrayChild(0).setValue(r)
            policy.getArrayChild(1).setValue(g)
            policy.getArrayChild(2).setValue(b)

    def canEditIntensity(self):
        names = self.getProfile().getConstant('SHADER_INT_PARAM_NAMES')
        return bool(self.__searchShaderParameters(names, edit=True))

    def setIntensity(self, value, createKeyframe=False):
        return self.__setParameterValue('SHADER_INT_PARAM_NAMES', value, createKeyframe)

    def canEditExposure(self):
        names = self.getProfile().getConstant('SHADER_EXP_PARAM_NAMES')
        return bool(self.__searchShaderParameters(names, edit=True))

    def setExposure(self, value, createKeyframe=False):
        return self.__setParameterValue('SHADER_EXP_PARAM_NAMES', value, createKeyframe)

    def __setParameterValue(self, constant, value, createKeyframe):
        from Katana import FormMaster
        names = self.getProfile().getConstant(constant)
        param = self.__searchShaderParameters(names, edit=True)
        if param:
            frame = NodegraphAPI.GetCurrentTime()
            valueParam = param.getChild('value')
            oldValue = valueParam.getValue(NodegraphAPI.GetCurrentTime())
            if valueParam.isExpression():
                # Cannot keyframe an expression
                raise ScriptItems.AnimatingExpressionException(oldValue)

            elif param.isAnimated() and not NodegraphAPI.GetAutoKeyAll():
                if not valueParam.hasKey(frame) and createKeyframe:
                    valueParam.setKey(frame)
                elif valueParam.hasKey(frame) is False and param.getAutoKey() is False:
                    raise ScriptItems.AnimationKeyNotFoundException(oldValue)

            FormMaster.CreateParameterPolicy(None, param).setValue(value)

    def __searchShaderParameters(self, names, edit=False):
        mn = self.getMaterialNode()
        if not mn:
            return

        if edit: mn._cookAndWaitToUpdateParameters()

        for name in names:
            param = mn.getParameter('shaders.%s' % name)
            if param:
                return param

    def getLocalSpaceCOI(self, rootProducer = None):

        if not rootProducer:
            rootProducer = SA.GetInternalProducer(self.getGafferPro())

        producer = rootProducer.getProducerByPath(self.getFullName())
        if not producer: return None

        xform = GeoAPI.ProducerLocalTransform(producer)
        coiAttr = producer.getAttribute(
                    'geometry.centerOfInterest')
        coi = coiAttr.getNearestSample(0.0)[0]
        translate = GeoAPI.Util.Matrix.transformPoint4ByMatrix4x4(
                        (0.0, 0.0, -coi, 1.0), xform)
        return translate

    def getDefaultLinkingParameter(self):
        return self.getNodeReference('node_lightCreate').getParameter(
            'initialState')

    def getDefaultShadowLinkingParameter(self):
        return self.getNodeReference('node_lightCreate').getParameter(
                'lightListDefaultsIfDisabled.geoShadowEnable')

    def getLinkingExceptionsParameter(self):
        return self.getNodeReference(
                'node_enable').getParameter('objects')

    def getShadowLinkingExceptionsParameter(self):
        return self.getNodeReference(
                'node_geoShadowEnable').getParameter('objects')

#//////////////////////////////////////////////////////////////////////////////
#///
def Register():
    """ Register the GafferScriptItem (none GUI) classes """
    GafferAPI.ScriptItems.RegisterPackageClass(PackageName, GafferLightScriptItem)

def RegisterGUI():
    """ Register the GafferUIItem and GafferAddLocationAction classes that are used by the GUI.

    GafferAddLocationAction  - Controls how this item appears in the "Add..." menu
    GafferUIItem             - Controls the tabs in the Gaffer UI
    """
    from Katana import UI4, QT4FormWidgets,FormMaster
    from PluginAPI.BaseGafferAction import BaseGafferAddLocationAction
    UndoGrouping = UI4.Util.UndoGrouping.UndoGrouping
    FnKatImport.FromObject(GafferAPI.UIItems, merge = ['*'])

    global GafferLightUIItem
    class GafferLightUIItem(GafferUIItem):

        def getObjectTabPolicy(self):
            editorNode = self.getReferencedNode('node_lightCreate')
            if not editorNode: return

            scriptItem = self.getScriptItem()


            rootPolicy = QT4FormWidgets.PythonGroupPolicy('object')
            rootPolicy.getWidgetHints()['hideTitle'] = True


            geoPolicy = QT4FormWidgets.PythonGroupPolicy(
                    'geometry', rootPolicy)
            geoPolicy.getWidgetHints()['open'] = True

            rootPolicy.addChildPolicy(geoPolicy)


            attrsParam = SA.GetAttributesParameterForPath(
                    scriptItem.getGafferPro(), scriptItem.getFullName(),
                            create=False)

            radiusParam = editorNode.getParameter('radius')

            if attrsParam and attrsParam.getChild('geometry'):
                geoParam = attrsParam.getChild('geometry')

                if not radiusParam:
                    radiusParam = geoParam.getChild('radius')


            if radiusParam:
                radiusPolicy = QT4FormWidgets.ValuePolicyProxy(
                        FormMaster.CreateParameterPolicy(geoPolicy,
                                radiusParam))
                radiusPolicy.getWidgetHints()['min'] = '0'
                geoPolicy.addChildPolicy(radiusPolicy)


            for name in (
                    'fov',
                    'centerOfInterest',
                    'near',
                    'far',
                    'screenWindow'):
                geoPolicy.addChildPolicy(
                        FormMaster.CreateParameterPolicy(geoPolicy,
                                editorNode.getParameter(name)))

            transformPolicy = FormMaster.CreateParameterPolicy(rootPolicy,
                    editorNode.getParameter('transform'))

            rootPolicy.addChildPolicy(transformPolicy)

            constraintNode = self.getReferencedNode('node_aimConstraint')
            if not constraintNode: return rootPolicy

            AddConstraintPolicy(scriptItem, rootPolicy, constraintNode, 'aim',
                                ['targetPath.i0', 'targetOrigin', 'baseAimAxis',
                                 'baseUpAxis', 'targetUpAxis'])


            displayPolicy = QT4FormWidgets.PythonGroupPolicy(
                    'display', rootPolicy)

            rootPolicy.addChildPolicy(displayPolicy)

            vosNode = self.getReferencedNode('node_viewerObjectSettings')
            if vosNode:
    #            textPolicy = FormMaster.NodeBypassPolicy(displayPolicy, vosNode)
    #            hints = textPolicy.getWidgetHints()
    #            hints['label'] = 'display annotation'
    #            hints['widget'] = 'checkBox'
    #            displayPolicy.addChildPolicy(textPolicy)
                displayPolicy.addChildPolicy(
                        FormMaster.CreateParameterPolicy(displayPolicy,
                            vosNode.getParameter(
                                'args.viewer.default.annotation.text')))


            previewColorPolicy = QT4FormWidgets.ValuePolicyProxy(
                    FormMaster.CreateParameterPolicy(displayPolicy,
                                        editorNode.getParameter('previewColor')))
            displayPolicy.addChildPolicy(previewColorPolicy)



            return rootPolicy


        #static to allow other non-inherited types to call
        @staticmethod
        def GetMaterialTabPolicy(self):
            editorNode = self.getReferencedNode('node_material')
            if not editorNode: return

            scriptItem = self.getScriptItem()

            rootPolicy = QT4FormWidgets.PythonGroupPolicy('material')

            rootPolicy.getWidgetHints()['open'] = True
            lookFileEnablePolicy = QT4FormWidgets.ValuePolicyProxy(
                    FormMaster.CreateParameterPolicy(rootPolicy,
                            SA.GetLookFileMaterialEnabledParameter(scriptItem)))
            lookFileEnablePolicy.setWidgetHints({'widget':'checkBox'})

            rootPolicy.addChildPolicy(lookFileEnablePolicy)

            refPolicy = QT4FormWidgets.PythonGroupPolicy('reference', rootPolicy)
            refPolicy.getWidgetHints().update({'open':True,
                    'conditionalVisOp':'notEqualTo',
                    'conditionalVisPath':'../%s' % lookFileEnablePolicy.getName(),
                    'conditionalVisValue':'0',
                    })

            rootPolicy.addChildPolicy(refPolicy)

            refParam = SA.GetLookFileMaterialReferenceParameter(scriptItem)

            remap = {
                'asset':'lookfile',
            }
            for param in refParam.getChildren():
                policy = QT4FormWidgets.ValuePolicyProxy(
                        FormMaster.CreateParameterPolicy(refPolicy, param))
                hints = {}
                editorNode.addParameterHints('Material.lookfile.'+ remap.get(policy.getName(), policy.getName()),
                        hints)
                if policy.getName() == 'materialPath':
                    hints = dict(hints)
                    hints['assetPolicy'] = '../asset'

                policy.setWidgetHints(hints)
                refPolicy.addChildPolicy(policy)

            rootPolicy.addChildPolicy(
                    FormMaster.CreateParameterPolicy(rootPolicy,
                            editorNode.getParameter('addShaderType')))

            rootPolicy.addChildPolicy(
                    FormMaster.CreateParameterPolicy(rootPolicy,
                            editorNode.getParameter('shaders')))

            return rootPolicy


        def getMaterialTabPolicy(self):
            return self.__class__.GetMaterialTabPolicy(self)

        def getLinkingTabPolicy(self):
            return self.__class__.GetLinkingTabPolicy(self)

        @staticmethod
        def GetLinkingTabPolicy(self):
            scriptItem = self.getScriptItem()
            gnode = scriptItem.getGafferPro()
            if not gnode:
                return

            rootPolicy = QT4FormWidgets.PythonGroupPolicy('linking')
            rootPolicy.getWidgetHints()['hideTitle'] = True

            def addLinkingPolicy(defaultPolicy, exceptionsPolicy, name):
                if not defaultPolicy: return

                group = QT4FormWidgets.PythonGroupPolicy(name, rootPolicy)
                group.getWidgetHints()['open'] = True
                rootPolicy.addChildPolicy(group)


                policy = QT4FormWidgets.ValuePolicyProxy(
                            FormMaster.CreateParameterPolicy(
                                    group,defaultPolicy))
                policy.setWidgetHints({'widget':'mapper', 'options':'on:1|off:0'})
                group.addChildPolicy(policy)

                policy = FormMaster.NodeBypassPolicy(group,
                    exceptionsPolicy.getNode())
                hints = policy.getWidgetHints()
                hints['label'] = 'enable exceptions'
                hints['widget'] = 'checkBox'
                hints['headerChild'] = 'objects'
                group.addChildPolicy(policy)


                cel = QT4FormWidgets.ValuePolicyProxy(
                        FormMaster.CreateParameterPolicy(
                                group,exceptionsPolicy))
                hints = {}
                hints['widget'] = 'cel'
                hints['conditionalVisOps'] = {
                    'conditionalVisOp':'notEqualTo',
                    'conditionalVisPath':'../'+policy.getName(),
                    'conditionalVisValue':'0',
                }
                hints['widget'] = 'cel'
                hints['help'] = \
                        'Exceptions will not work unless scene' \
                        ' is connected into Gaffer'
                hints['helpAlert'] = 'warning'

                hints['open'] = True
                cel.setWidgetHints(hints)

                group.addChildPolicy(cel)

            addLinkingPolicy(
                    scriptItem.getDefaultLinkingParameter(),
                    scriptItem.getLinkingExceptionsParameter(),
                    'light linking')

            addLinkingPolicy(
                    scriptItem.getDefaultShadowLinkingParameter(),
                    scriptItem.getShadowLinkingExceptionsParameter(),
                    'shadow linking')



            if rootPolicy.getNumChildren():
                return rootPolicy




        @staticmethod
        def DecorateShaderColumn(self, rootProducer, item):
            path, paramKey = item.getItemData()
            scriptItem = self.getScriptItem()

            if not rootProducer:
                rootProducer = SA.GetInternalProducer(scriptItem.getGafferPro())

            if rootProducer:
                #materialProducer = rootProducer.getProducerByPath(
                #        scriptItem.getMaterialPath())

                # GEOLIB3 We're resolving materials here, no need to walk to
                # the material location

                materialProducer = rootProducer.getProducerByPath(
                        scriptItem.getFullName())
            else:
                materialProducer = None


            if hasattr(scriptItem, 'getMasterMaterial'):
                masterMaterial = scriptItem.getMasterMaterial()
            else:
                masterMaterial = None


            localLookFileMaterialSet = scriptItem.getLookFileMaterialEnabled()

            if masterMaterial:
                item.setText(C.COLUMN_SHADER, masterMaterial)
            elif materialProducer:
                item.setText(C.COLUMN_SHADER, "<none>")

                if localLookFileMaterialSet:
                    #local icon
                    item.setIcon(C.COLUMN_SHADER,
                            UI4.Util.ScenegraphIconManager.GetIcon(
                                    'katana standard'))

                materialAttr = materialProducer.getGlobalAttribute('material')
                if materialAttr:
                    refPathAttr = materialAttr.childByName('info.reference.materialPath')
                    if refPathAttr:
                        if not localLookFileMaterialSet:
                            #inherited
                            item.setIcon(C.COLUMN_SHADER,
                                    UI4.Util.ScenegraphIconManager.GetIcon(
                                            'katana standard'))

                        #set to base name of material path
                        item.setText(C.COLUMN_SHADER,
                                refPathAttr.getData()[0].rsplit('/',1)[-1])
                    else:
                        attrName = scriptItem.getMaterialParameterName()

                        shaderAttr = materialAttr.childByName(attrName)
                        if shaderAttr:
                            item.setText(C.COLUMN_SHADER,
                                    str(shaderAttr.getData()[0]))
            else:
                item.setText(C.COLUMN_SHADER, "<none>")



        def decorateTreeItem(self, rootProducer, item):
            GafferUIItem.decorateTreeItem(self, rootProducer, item)
            self.__class__.DecorateShaderColumn(self, rootProducer, item)

            scriptItem = self.getScriptItem()

            color = scriptItem.getColor(rootProducer)
            if not color is None:
                item.setIcon(C.COLUMN_COLOR, ColorIcon(tuple(color)))

            intensity = scriptItem.getIntensity(rootProducer)
            if not intensity is None:
                item.setText(C.COLUMN_INT, '%.2f' % intensity)

            exposure = scriptItem.getExposure(rootProducer)
            if not exposure is None:
                item.setText(C.COLUMN_EXP, '%.2f' % exposure)

            link = scriptItem.getDefaultLinkingParameter().getValue(0.0)
            exc  = scriptItem.getLinkingExceptionsEnabled()

            item.setText(C.COLUMN_LINK, ("on" if link else "off")
                                      + ("*"  if exc  else ""))

        def prePaint(self, painter, option, item, column):
            if column == C.COLUMN_SHADER:
                scriptItem = self.getScriptItem()
                if scriptItem.getMasterMaterial():
                    return GetInheritedColorOption(option)


            return GafferUIItem.prePaint(self, painter, option, item, column)

        # decorateChildTreeItem no longer required to hide children, left
        # here to retain correct behavior in Katana <= 2.5.5
        def decorateChildTreeItem(self, rootProducer, item, childItem):
            childType = SA.GetHierarchyCreateNode(
                    self.getScriptItem().getGafferPro()).getTypeOfLocation(
                            childItem.getItemData()[0])

            childItem.setHidden(childType not in frozenset(('locator',)))

        def isChildVisible(self, name, type):
            """Light only shows locators."""
            return type == 'locator'

        @staticmethod
        def ShaderColumnClickEvent(self, event, item, column):
            scriptItem = self.getScriptItem()

            def go(popupItem, meta):
                u = UI4.Util.UndoGrouping.UndoGrouping('Set Light Shader')
                #log.debug("popupItem = %s" % popupItem)
                if meta == MASTER_MATERIAL_KEY:
                    scriptItem.setMasterMaterial(popupItem)
                else:
                    scriptItem.setShader(popupItem)

            def setLookFileMaterial(spref, path):
                u = UI4.Util.UndoGrouping.UndoGrouping('Set Look File Material')
                scriptItem.setLookFileMaterial(spref, path)

            lookFileValue = scriptItem.getLookFileMaterial()
            lookFileActive=False
            if scriptItem.getLookFileMaterialEnabled():
                lookFileActive = True


            getMasterMaterialPaths = None

            if hasattr(scriptItem, 'setMasterMaterial'):
                getMasterMaterialPaths = scriptItem.getGafferPro().getMasterMaterialPaths

            profile = scriptItem.getProfile()

            if hasattr(profile, 'getLightShaderType'):
                shaderType = profile.getLightShaderType()

            StandAloneShaderPopup.ShowForTreeItem(
                            item, column, go,
                            getMasterMaterialPaths,
                            setLookFileCallback=setLookFileMaterial,
                            lookFileValue=lookFileValue,
                            lookFileActive=lookFileActive,
                            shaderType=shaderType,
                            shouldIncludeShaderCallback=profile.shouldIncludeShader,
                            profile=profile
                            )

            return True


        def mousePressEvent(self, event, item, column):
            if not item: return
            if column == C.COLUMN_SHADER:
                return self.__class__.ShaderColumnClickEvent(
                        self, event, item, column)
            elif column == C.COLUMN_COLOR:

                button = event.button()


                if button == QtCore.Qt.LeftButton:
                    policy = self.getScriptItem().getColorPolicy()
                    if policy:

                        isAnimationCurve = False

                        needsKeyFrames = False
                        autoKeyEnabled = NodegraphAPI.GetAutoKeyAll()

                        for child in policy.getArrayChildren():

                            if child.isCurveEnabled():
                                isAnimationCurve = True

                            if not NodegraphAPI.GetAutoKeyAll() and not child.getCurveAutoKey():
                                autoKeyEnabled = False

                            if not child.getCurveKey():
                                needsKeyFrames = True


                        if isAnimationCurve and needsKeyFrames and autoKeyEnabled is False:
                            QtGui.QMessageBox.question(None, "Changing animated parameter", "Warning, one or more of the color channels are animated without auto-key frame enabled. You may need to add key frames when changing this color or activate auto-keying" ,"OK")

                        UI4.Util.Color.ShowColorPicker(policy)

                    #TODO, handle multiselection case


                elif button == QtCore.Qt.MidButton:

                    policy = self.getScriptItem().getColorPolicy()
                    if policy is None:
                        return GafferUIItem.mousePressEvent(self, event, item,
                                                            column)

                    dragDict = QT4Color.GetColorPolicyDragDict(policy)
                    dragDict['itemhash'] = hash(item)
                    dragData = QT4Color.BuildDragDataFromInfo(**dragDict)
                    mimeData = QtCore.QMimeData()
                    mimeData.setData(QT4Color.GetColorDragType(), dragData)

                    pixmap = QT4Widgets.InteractiveDrag.buildDragPixmap(
                            QT4Color.BuildDragPixmapFromColor(dragDict['color']), 1)
                    drag = QT4Widgets.InteractiveDrag(item.treeWidget(),
                            mimeData, pixmap)
                    event.accept()

                    drag.exec_()


            else:
                return GafferUIItem.mousePressEvent(self, event, item, column)

        def dragMoveEvent(self, event, item, column):
            if column == C.COLUMN_COLOR:
                if event.provides(QT4Color.GetColorDragType()):
                    colorInfo = QT4Color.GetColorInfoFromDragObject(event)
                    if hash(item) == colorInfo.get('itemhash'):
                        return False

                    return True
        def dropEvent(self, event, item, column):
            if column == C.COLUMN_COLOR:
                if event.provides(QT4Color.GetColorDragType()):
                    colorInfo = QT4Color.GetColorInfoFromDragObject(event)
                    u = UI4.Util.UndoGrouping.UndoGrouping('Drop Color')
                    self.getScriptItem().setColor(colorInfo['color'][0:3])

        def replaceEditor(self, editor, option, item, column):
            if column == C.COLUMN_INT:
                editor.setValidator(QtGui.QDoubleValidator(editor))
            elif column == C.COLUMN_EXP:
                editor.setValidator(QtGui.QDoubleValidator(editor))

            return editor

        def isItemColumnEditable(self, item, column):
            if column == C.COLUMN_INT:
                return self.getScriptItem().canEditIntensity()
            elif column == C.COLUMN_EXP:
                return self.getScriptItem().canEditExposure()
            else:
                return GafferUIItem.isItemColumnEditable(self, item, column)

        def itemEdited(self, item, column, value, createKeyframe=False):
            try:
                if column == C.COLUMN_INT:
                    u = UI4.Util.UndoGrouping.UndoGrouping('Edit Intensity')
                    if item.isSelected():
                        for si in self._getScriptItemsForSelectedItems(item):
                            if si.canEditIntensity():
                                si.setIntensity(value, createKeyframe)
                    else:
                        si = self.getScriptItem()
                        si.setIntensity(value, createKeyframe)
                    return True
                elif column == C.COLUMN_EXP:
                    si = self.getScriptItem()
                    si.setExposure(value, createKeyframe)
                    return True
                else:
                    return GafferUIItem.itemEdited(self, item, column, value)

            except ScriptItems.AnimationKeyNotFoundException, ex:
                if QtGui.QMessageBox.question(None, "Changing animated parameter", "You are attempting to change an animated parameter without a key frame at the current frame. Would you like to add a new key frame?" , "Cancel", "Create Key" ) != 0:
                    if createKeyframe is False:
                        self.itemEdited(item, column, value, True) # Call function again but setting key frame if necessary
                else:
                    item.setText(column, '%.2f' % ex.getValue()) # Reset the value in the tree

            except ScriptItems.AnimatingExpressionException, ex:
                if QtGui.QMessageBox.question(None, "Changing expression parameter", "Cannot change a parameter that is set by an expression.", "OK") == 0:
                    item.setText(column, '%.2f' % ex.getValue())

        def fillMenu(self, menu):
            pass


    #GafferLightUIItemClass = GafferLightUIItem
#//////////////////////////////////////////////////////////////////////////////

    class AddLightAction(BaseGafferAddLocationAction):
        def __init__(self, parent, parentPath, nodeName, preselectMethod):
            BaseGafferAddLocationAction.__init__(self, parent, self.getActionName() ,
                     parentPath, nodeName, preselectMethod)

            self.setIcon(UI4.Util.ScenegraphIconManager.GetIcon('light'))

        def go(self, checked=None):
            node = self.getNode()
            if not node: return

            u = UndoGrouping(self.getActionName() )
            desiredPath = self.getParentPath() + '/light1'
            newPath = node.addLight(desiredPath,
                                    alwaysCreateWithOptionalLeafRename = True,
                                    scriptItemClassName = self.getPackageName())
            self.preselectPath(newPath)

        def getPackageName(self):
            return PackageName

        @staticmethod
        def getActionName():
            return 'Add Light'

        @staticmethod
        def getMenuGroup():
            return 'Lighting'




    """ Register the AddLocationAction and UIItem """
    GafferAPI.GafferPluginsModule.RegisterCreateCallback('AddLightAction', AddLightAction)
    GafferAPI.GafferPluginsModule.RegisterGafferUIItem(PackageName, GafferLightUIItem)
