# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.
from Katana import Plugins
from Katana import NodegraphAPI
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
PackageName = 'GafferProSkyDomePackage'


#//////////////////////////////////////////////////////////////////////////////
#///

from Katana import NodegraphAPI, KatanaFeatures

log = logging.getLogger("Gaffer.SkyDomeScriptItem")

#seems to load differently in batch mode than other forms
if hasattr(KatanaFeatures, 'KatanaFeatures'):
    KF = KatanaFeatures.KatanaFeatures
else:
    KF = KatanaFeatures

class GafferSkyDomeScriptItem(ScriptItems.GafferScriptItem):
    @staticmethod
    def create(gnode, path):
        group = SA.BuildLocationPackageForPath(gnode, path)
        group.setName(os.path.basename(path) + '_package')
        group.setType(PackageName)
        SA.AddNodeReferenceForPath(gnode, path, 'package', group)
        
        nodes = []
        
        primCreate = NodegraphAPI.CreateNode('PrimitiveCreate', group)
        primCreate.addInputPort('in')
        primCreate.getParameter('name').setExpression('getParent().pathName')
        primCreate.getParameter('type').setValue('coordinate system sphere', 0)
        
        for p in primCreate.getParameter('transform.scale').getChildren():
            p.setValue(10000, 0)
        
        nodes.append(primCreate)

        lightListEdit = NodegraphAPI.CreateNode('LightListEdit', group)
        lightListEdit.getParameter('locations.i0').setExpression(
                'str(getParent().pathName)+"/material"')
        nodes.append(lightListEdit)
        
        material = NodegraphAPI.CreateNode('Material', group)
        material.getParameter('action').setValue('edit material', 0)
        material.getParameter('makeInteractive').setValue('Yes', 0)
        material.getParameter('edit.location').setExpression(
                'str(getParent().pathName)+"/material"')
        material.getParameter('makeInteractive').setValue('No', 0)
        nodes.append(material)
        
        primCreate.getParameter('previewTexture').setExpression(
                "getNode(%r).shaders.arnoldSurfaceParams.HDRI_map.value" % \
                        material.getName())
        
        attrsParam = SA.GetAttributesParameterForPath(gnode, path, create=True)
        materialAssignParam = attrsParam.createChildString(
                'materialAssign', '')
        materialAssignParam.setExpression(
                '"/"+getParamRelative(self, "../../../material").'\
                'param.getFullName(False).replace(".", "/")')
        
        SA.BuildLookFileMaterialParametersForPath(gnode,
                path+'/material')
        
        materialAttrsParam = SA.GetAttributesParameterForPath(gnode,
                path+'/material', create=True)
        geoGroup = materialAttrsParam.createChildGroup('geometry')
        geoGroup.createChildNumber('samples', 1)
        geoGroup.createChildString('__type__samples', 'IntAttr')
        geoGroup.createChildNumber('sssSamples', 1)
        geoGroup.createChildString('__type__sssSamples', 'IntAttr')
        geoGroup.createChildNumber('resolution', 1024)
        geoGroup.createChildString('__type__resolution', 'IntAttr')
        
        hc = SA.GetHierarchyCreateNode(gnode)
        metaGroup = hc.getMetaDataParameterForLocation(
                path+'/material', create=True)
        metaGroup.createChildNumber('skydome', 1)
        hcAttrs = hc.getAttrsParameterForLocation(path, create=True)
        gafferAttr = hcAttrs.createChildString('gaffer_type', 'skydome')
        
        viewerFlip = NodegraphAPI.CreateNode('AttributeSet', group)
        viewerFlip.setName('viewerFlip')
        viewerFlip.getParameter('paths.i0').setExpression(
                "getParent().pathName")
        viewerFlip.getParameter('attributeName').setValue(
                'geometry.viewerFlipU', 0)
        viewerFlip.getParameter('attributeType').setValue('integer', 0)
        viewerFlip.getParameter('numberValue.i0').setExpression(
                '1 if getParent().latlongOrientation == "Interior" else 0')
        nodes.append(viewerFlip)
        
        xformScript = NodegraphAPI.CreateNode('AttributeSet', group)


        xformScript.setName('xformScript')
        xformScript.getParameter('attributeName').setValue(
                'material.ops.skydomeXform', 0)
        xformScript.getParameter('paths.i0').setExpression(
                'str(getParent().pathName)+"/material"')
        xformScript.getParameter('attributeType').setValue('group', 0)

        xformScript.getParameter('groupValue').createChildString(
                'opType', 'GafferLegacyApplySkydomeXform')
        opArgs = xformScript.getParameter('groupValue').createChildGroup(
                'opArgs')
        
        flipX = opArgs.createChildNumber('flipX', 0.0)
        
        # # flipX is flipped for OSL only
        flipX.setExpression(
            'getParent().latlongOrientation == '
          + '("Exterior" if getParent().gafferProfile == "osl" else "Interior")')

        opArgs.createChildString('__type__flipX', 'IntAttr')
        



        nodes.append(xformScript)
        
        SA.WireInlineNodes(group, nodes)
        
        postMerge = SA.GetPostMergeStack(gnode)
        
        postMergePackage = NodegraphAPI.CreateNode('GroupStack')
        postMergePackage.setChildNodeType('<multi>')
        postMerge.buildChildNode(adoptNode=postMergePackage)
        
        # reorder to the top of the list so that newly created skydomes
        # won't immediately overwrite existing ones
        postMerge.setExplicitChildOrder(
                [postMergePackage]+postMerge.getChildNodes()[:-1])
        
        setBackground = NodegraphAPI.CreateNode('AttributeSet', group)
        setBackground.getParameter('paths.i0').setValue('/root', 0)
        setBackground.getParameter('attributeName').setValue(
                'arnoldGlobalStatements.background', 0)
        setBackground.getParameter('attributeType').setValue('string', 0)
        setBackground.getParameter('stringValue.i0').setExpression(
                "str(getNode(%r).pathName)+'/material'" % group.getName())
        
        postMergePackage.buildChildNode(adoptNode=setBackground)
        
        defaultLink = metaGroup.createChildNumber('defaultLink', 1)
        defaultShadowLink = metaGroup.createChildNumber('defaultShadowLink', 1)
        
        pathExpr = 'str(getNode(%r).pathName)+"/material"' % group.getName()
        SA.BuildLightLinkNodes(group, postMergePackage, defaultLink, pathExpr)
        SA.BuildLightLinkNodes(group, postMergePackage, defaultShadowLink,
                pathExpr).getParameter('effect').setValue(
                        'shadow visibility', 0)
        
        ll = group.getParameters().createChildString('latlongOrientation', 'Exterior')
        ll.setHintString("""{'widget': 'popup',
                             'options': 'Interior|Exterior',
                             'help': %r}""" % Resources.HELP_LATLONGORIENTATION)
        
        SA.AddNodeReferenceForPath(gnode, path, 'postmerge', postMergePackage)
        
        SA.AddNodeReferenceParam(group, 'node_primCreate', primCreate)
        SA.AddNodeReferenceParam(group, 'node_lightListEdit', lightListEdit)
        SA.AddNodeReferenceParam(group, 'node_material',   material)

        SA.AddNodeReferenceParam(group, 'node_setBackground', setBackground)
    
    def acceptsChild(self, childScriptItem):
        return False

    def setShader(self, shaderName, asExpression=False):
        n = self.getNode()
        m = SA.GetRefNode(n, 'material')
        if shaderName is None:
            m.getParameter('shaders.arnoldSurfaceShader.enable').setValue(0,0)
        else:
            m.getParameter('shaders.arnoldSurfaceShader.enable').setValue(1,0)
            if asExpression:
                m.getParameter('shaders.arnoldSurfaceShader.value').setExpression(
                        shaderName)
            else:
                m.getParameter('shaders.arnoldSurfaceShader.value').setValue(
                        shaderName,0)

    @deprecated('setShader()', log)
    def setArnoldShader(self, shaderName):
        self.setShader(shaderName)
    
    def getArnoldShader(self):
        n = self.getNode()
        m = SA.GetRefNode(n, 'material')
        if (m.getParameter('shaders.arnoldSurfaceShader.enable').getValue(0)):
            return m.getParameter(
                    'shaders.arnoldSurfaceShader.value').getValue(0)
        return ''
    
    
    
    def getMaterialPath(self):
        return self.getFullName()+'/material'
    
    def setBackgroundMapParameter(self, param):
        p = self.getNodeReference('node_primCreate')
        m = self.getNodeReference('node_material')

        p.getParameter('previewTexture').setExpression(
                "getNode(%r).shaders.%s" % (m.getName(), param))
    
    def getLookFileMaterialEnabled(self):
        return bool(SA.GetLookFileMaterialEnabledParameter(self).getValue(0))
    
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
    
    def makeActiveAsBackground(self):
        postMergePackage = SA.GetNodeReferenceForPath(self.getGafferPro(),
                self.getFullName(), 'postmerge')
        
        postMerge = postMergePackage.getParent()
        
        childNodes = postMerge.getChildNodes()
        childNodes.remove(postMergePackage)
        childNodes.append(postMergePackage)
        postMerge.setExplicitChildOrder(childNodes)

    def getBackgroundMappingEnabled(self):
        node = self.getNodeReference('node_setBackground')
        if not node:
            return True
        return not node.isBypassed()
    
    def setBackgroundMappingEnabled(self, state):
        node = self.getNodeReference('node_setBackground')
        if node:
            node.setBypassed(not state)
    
    def getAbstractTransformNode(self):
        return self.getNodeReference('node_primCreate')

    def getMaterialNode(self):
        return self.getNodeReference('node_material')
    
    def supportsMuting(self):
        return "MUTE_ARNOLD_SKYDOMES" in KF
    
    def getDefaultLinkingParameter(self):
        return self.getNodeReference('node_lightListEdit').getParameter(
                'initialState')
    
    def getDefaultShadowLinkingParameter(self):
        #TODO: add support to LightListEdit for arbitrary linking attrs and port
        hc = SA.GetHierarchyCreateNode(self.getGafferPro())
        metaParam = hc.getMetaDataParameterForLocation(
                self.getMaterialPath(), create=False)
        return metaParam.getChild('defaultShadowLink')
    
    def getLinkingExceptionsParameter(self):
        return self.getNodeReference(
                'node_defaultLink').getParameter('objects')
    
    def getShadowLinkingExceptionsParameter(self):
        return self.getNodeReference(
                'node_defaultShadowLink').getParameter('objects')
    
#//////////////////////////////////////////////////////////////////////////////
#///

def Register():
    """ Register the GafferScriptItem (none GUI) classes """ 
    GafferAPI.ScriptItems.RegisterPackageClass(PackageName, GafferSkyDomeScriptItem)


def RegisterGUI():
    """ Register the GafferUIItem and GafferAddLocationAction classes that are used by the GUI.
    
    GafferAddLocationAction  - Controls how this item appears in the "Add..." menu
    GafferUIItem             - Controls the tabs in the Gaffer UI 
    """
    from Katana import UI4, QT4FormWidgets,FormMaster
    from PluginAPI.BaseGafferAction import BaseGafferAddLocationAction
    UndoGrouping = UI4.Util.UndoGrouping.UndoGrouping
    FnKatImport.FromObject(GafferAPI.UIItems, merge = ['*'])
        
        
    #//////////////////////////////////////////////////////////////////////////////
    #///
    class GafferSkyDomeUIItem(GafferUIItem):
        # decorateChildTreeItem no longer required to hide children, left
        # here to retain correct behavior in Katana <= 2.5.5
        def decorateChildTreeItem(self, rootProducer, item, childItem):
            childItem.setHidden(True)
        
        def isChildVisible(self, name, type):
            """SkyDome doesn't show children of any type."""
            return False
    
        def decorateTreeItem(self, rootProducer, item):
            from LightScriptItem import GafferLightUIItem
            
            GafferUIItem.decorateTreeItem(self, rootProducer, item)
            
            GafferLightUIItem.DecorateShaderColumn(
                    self, rootProducer, item)

            if not rootProducer:
                item.setIcon(C.COLUMN_NAME, LocalIcon('skydome16'))
                return 
            
            activeSkyDomePathAttr = rootProducer.getAttribute(
                    'arnoldGlobalStatements.background')
            activeSkyDomePath = activeSkyDomePathAttr and \
                    activeSkyDomePathAttr.getData()[0]
            if (self.getNode().getParameter('pathName').getValue(0) +'/material' ==
                    activeSkyDomePath):
                item.setIcon(C.COLUMN_NAME, LocalIcon('skydome16'))
            else:
                #TODO, inactive
                item.setIcon(C.COLUMN_NAME, LocalIcon('skydomeInactive16'))
        
        def mousePressEvent(self, event, item, column):
            if not item: return
            if column == C.COLUMN_SHADER:
                from LightScriptItem import GafferLightUIItem
                return GafferLightUIItem.ShaderColumnClickEvent(
                        self, event, item, column)
            
            return GafferUIItem.mousePressEvent(self, event, item, column)
        
        def getMaterialTabPolicy(self):
            from LightScriptItem import GafferLightUIItem
            return GafferLightUIItem.GetMaterialTabPolicy(self)
        
        def getObjectTabPolicy(self):
            editorNode = self.getReferencedNode('node_primCreate')
            if not editorNode: return
            
            rootPolicy = QT4FormWidgets.PythonGroupPolicy('object')
            rootPolicy.getWidgetHints()['hideTitle'] = True
            
            scriptItem = self.getScriptItem()
            
            attrsParam = SA.GetAttributesParameterForPath(
                    scriptItem.getGafferPro(), scriptItem.getMaterialPath(),
                            create=False)
            if attrsParam:
                for name in (
                        'resolution',
                    ):
                    
                    param = attrsParam.getChild("geometry."+name)
                    if not param:
                        continue
                    
                    policy = QT4FormWidgets.ValuePolicyProxy(
                            FormMaster.CreateParameterPolicy(rootPolicy, param))
                    policy.getWidgetHints()['int'] = True
                    rootPolicy.addChildPolicy(policy)
            
            for name in (
                    'previewAlpha',
                    'transform',
                    'viewerPickable',
                    ):
                rootPolicy.addChildPolicy(
                        FormMaster.CreateParameterPolicy(rootPolicy,
                                editorNode.getParameter(name)))
    
            ll = scriptItem.getNode().getParameter('latlongOrientation')
            if ll:
                llp = FormMaster.CreateParameterPolicy(rootPolicy, ll)
                llp.getWidgetHints()['widget']  = 'popup'
                llp.getWidgetHints()['options'] = 'Interior|Exterior'
                llp.getWidgetHints()['help'] = Resources.HELP_LATLONGORIENTATION
                rootPolicy.addChildPolicy(llp)
            
            return rootPolicy
            
        def getLinkingTabPolicy(self):
            from LightScriptItem import GafferLightUIItem
            return GafferLightUIItem.GetLinkingTabPolicy(self)
        
        
        
        def fillContextMenu(self, menu):
            menu.addSeparator()
            
            scriptItem = self.getScriptItem()
            def go():
                u = UI4.Util.UndoGrouping.UndoGrouping('Activate Skydome')
                scriptItem.makeActiveAsBackground()
            
            menu.addAction('Make Active As Background', go)
    

    class AddSkyDomeAction(BaseGafferAddLocationAction):
        def __init__(self, parent, parentPath, nodeName, preselectMethod):
            BaseGafferAddLocationAction.__init__(self, parent, self.getActionName(),
                     parentPath, nodeName, preselectMethod)
            self.setIcon(LocalIcon('skydome16'))
        
        def go(self, checked=None):
            node = self.getNode()
            if not node: return
            
            u = UndoGrouping(self.getActionName())
            
            newPath = node.addSkyDome('sky', self.getParentPath(), scriptItemClassName = self.getPackageName())
            self.preselectPath(newPath)
        
        def getPackageName(self):
            return PackageName
        
        @staticmethod
        def getActionName():
            return 'Add Sky Dome'
        
        @staticmethod
        def getMenuGroup():
            return 'Sky'
        
    # Register the ScriptAction
    GafferAPI.GafferPluginsModule.RegisterCreateCallback('AddSkyDomeAction', AddSkyDomeAction)
    GafferAPI.GafferPluginsModule.RegisterGafferUIItem(PackageName, GafferSkyDomeUIItem)
