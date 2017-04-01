# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.
from Katana import Plugins
from Katana import NodegraphAPI
from Katana import Utils

import Katana
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
PackageName = 'GafferProMasterMaterialPackage'

#//////////////////////////////////////////////////////////////////////////////
#///

log = logging.getLogger("Gaffer.MasterMaterialScriptItem")

class GafferMasterMaterialScriptItem(ScriptItems.GafferScriptItem):
    @staticmethod
    def create(gnode, path):
        hc = SA.GetHierarchyCreateNode(gnode)
        mms = SA.GetMasterMaterials(gnode)
        
        m = NodegraphAPI.CreateNode('Material')
        m.getParameters().createChildString('paramName', '')
        m.getParameters().createChildString('pathName', '')
        
        m.setType(PackageName)
        
        hcParam = hc.getParameterForLocation(path)
        m.getParameter('paramName').setExpression(
                'getParam(%r).param.getFullName()' % hcParam.getFullName())
        
        m.getParameter('pathName').setExpression(
                "'/%s' % str(paramName).split('.', 1)[-1].replace('.', '/')")
        
        m.getParameter('action').setValue('edit material', 0)
        m.getParameter('edit.location').setExpression('pathName')
        m.getParameter('makeInteractive').setValue('Yes', 0)

        hcAttrs = hc.getAttrsParameterForLocation(path, create=True)
        gafferAttr = hcAttrs.createChildString('gaffer_type', 'master material')
        
        mms.buildChildNode(adoptNode=m)
        SA.ReorderMasterMaterials(gnode)
        
        
        SA.BuildLookFileMaterialParametersForPath(gnode, path)
        SA.AddNodeReferenceParam(m, 'node_material', m)
        
        SA.AddNodeReferenceForPath(gnode, path, 'package', m)
        return m
    
    def getMaterialNode(self):
        n = self.getNode()
        m = SA.GetRefNode(n, 'material')
        return m
    
    def setShader(self, shaderName):
        m = self.getNode()
        
        profile = self.getProfile()
        shaderParam = profile.getMaterialParameterName()
        
        shadersParam = m.getParameter('shaders')
        if shadersParam.getChild(shaderParam) is None:
            m.addShaderType(shaderParam[:-6])
                
        m.getParameter('shaders.%s.enable' % shaderParam).setValue(1,0)
        m.getParameter('shaders.%s.value'  % shaderParam).setValue(shaderName,0)

    @deprecated('setShader()', log)
    def setArnoldShader(self, shaderName):
        self.setShader(shaderName)
    
    def acceptsChild(self, childScriptItem):
        return isinstance(childScriptItem, self.__class__)
    
    def getMaterialPath(self):
        return self.getFullName()
    
    def getLookFileMaterialEnabled(self):
        return bool(SA.GetLookFileMaterialEnabledParameter(self).getValue(0))
    
    def setLookFileMaterialEnabled(self, state):
        SA.GetLookFileMaterialEnabledParameter(self).setValue(bool(state), 0)
    
    def setLookFileMaterial(self, spref, path):
        if hasattr(self, 'setMasterMaterial'):
            self.setMasterMaterial(None)
        self.setLookFileMaterialEnabled(True)
        refParam = SA.GetLookFileMaterialReferenceParameter(self)
        refParam.getChild('asset').setValue(spref, 0)
        refParam.getChild('materialPath').setValue(path, 0)
    
    def getLookFileMaterial(self):
        refParam = SA.GetLookFileMaterialReferenceParameter(self)
        return tuple(x.getValue(0) for x in refParam.getChildren())
    

#//////////////////////////////////////////////////////////////////////////////
#///

def Register():
    """ Register the GafferScriptItem (none GUI) classes """ 
    GafferAPI.ScriptItems.RegisterPackageClass(PackageName, GafferMasterMaterialScriptItem)

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
    class GafferMasterMaterialUIItem(GafferUIItem):
        
        def getMaterialTabPolicy(self):
            from LightScriptItem import GafferLightUIItem
            return GafferLightUIItem.GetMaterialTabPolicy(self)
        
        
        def decorateTreeItem(self, rootProducer, item):
            #from LightScriptItem import GafferLightUIItemClass as GafferLightUIItem
            from LightScriptItem import GafferLightUIItem
            log.debug("GafferLightUIItem = %s" % GafferLightUIItem)
            GafferUIItem.decorateTreeItem(self, rootProducer, item)
            GafferLightUIItem.DecorateShaderColumn(self, rootProducer, item)
        
        
        
        def mousePressEvent(self, event, item, column):
            from LightScriptItem import GafferLightUIItem
            if not item: return
            
            if column == C.COLUMN_SHADER:
                return GafferLightUIItem.ShaderColumnClickEvent(
                        self, event, item, column)
                
            
            return GafferUIItem.mousePressEvent(self, event, item, column)
        
        
    class AddMasterMaterialAction(BaseGafferAddLocationAction):
        def __init__(self, parent, parentPath, nodeName, preselectMethod):
            BaseGafferAddLocationAction.__init__(self, parent, self.getActionName(),
                     parentPath, nodeName, preselectMethod)
            self.setIcon(UI4.Util.ScenegraphIconManager.GetIcon('light material'))
        
        def go(self, checked=None):
            node = self.getNode()
            if not node: return
            
            u = UndoGrouping(self.getActionName() )
            
            desiredPath = self.getParentPath() + '/material1'
            newPath = node.addMasterMaterial(desiredPath,
                                             alwaysCreateWithOptionalLeafRename = True,
                                             scriptItemClassName = self.getPackageName())
            self.preselectPath(newPath)
        
        def getPackageName(self):
            return PackageName
        
        @staticmethod
        def getActionName():
            return 'Add Master Material'
        
        @staticmethod
        def getMenuGroup():
            return 'Materials'
        
    # Register the ScriptAction
    GafferAPI.GafferPluginsModule.RegisterCreateCallback('AddMasterMaterialAction', AddMasterMaterialAction)
    GafferAPI.GafferPluginsModule.RegisterGafferUIItem(PackageName, GafferMasterMaterialUIItem)
