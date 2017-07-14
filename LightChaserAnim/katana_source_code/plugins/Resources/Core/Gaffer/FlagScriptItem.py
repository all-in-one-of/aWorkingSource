# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.
from Katana import Plugins
from Katana import NodegraphAPI

import Katana
import os
import FnKatImport



GafferAPI = Plugins.GafferAPI
ScriptItems = GafferAPI.ScriptItems
SA = GafferAPI.ScriptActions
C = GafferAPI.Constants
Resources = GafferAPI.Resources


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
PackageName = 'GafferProFlagPackage'




#//////////////////////////////////////////////////////////////////////////////
#///

class GafferFlagScriptItem(ScriptItems.GafferScriptItem):
    
    @staticmethod
    def create(gnode, path):
        flagGroup = SA.BuildLocationPackageForPath(gnode, path)
        flagGroup.setType(PackageName)
        flagGroup.setName(os.path.basename(path) + '_package')
        SA.AddNodeReferenceForPath(gnode, path, 'package', flagGroup)
        
        nodes = []
        
        primCreate = NodegraphAPI.CreateNode('PrimitiveCreate', flagGroup)
        primCreate.addInputPort('in')
        primCreate.getParameter('name').setExpression('getParent().pathName')
        primCreate.getParameter('type').setValue('poly plane', 0)
        
        nodes.append(primCreate)
        
        material = NodegraphAPI.CreateNode('Material', flagGroup)
        material.getParameter('action').setValue('edit material', 0)
        material.getParameter('makeInteractive').setValue('Yes', 0)
        material.getParameter('edit.location').setExpression(
                'str(getParent().pathName)+"/material"')
        material.getParameter('makeInteractive').setValue('Yes', 0)
        
        nodes.append(material)
        
        attrsParam = SA.GetAttributesParameterForPath(gnode, path, create=True)
        materialAssignParam = attrsParam.createChildString(
                'materialAssign', '')
        materialAssignParam.setExpression(
                '"/"+getParamRelative(self, "../../../material").'\
                'param.getFullName(False).replace(".", "/")')
        
        
        arnoldObjectSettings = NodegraphAPI.CreateNode('ArnoldObjectSettings',
                flagGroup)
        arnoldObjectSettings.getParameter('CEL').setExpression(
                'getParent().pathName')
        nodes.append(arnoldObjectSettings)
        
        
        
        SA.AddNodeReferenceParam(flagGroup, 'node_primCreate', primCreate)
        SA.AddNodeReferenceParam(flagGroup, 'node_material', material)
        SA.AddNodeReferenceParam(flagGroup, 'node_arnoldObjectSettings', arnoldObjectSettings)
        
        
        SA.WireInlineNodes(flagGroup, nodes)
    
    def acceptsChild(self, childScriptItem):
        return False
    
    def getAbstractTransformNode(self):
        return self.getNodeReference('node_primCreate')
    
#//////////////////////////////////////////////////////////////////////////////
#///

def Register():
    """ Register the GafferScriptItem (none GUI) classes """ 
    GafferAPI.ScriptItems.RegisterPackageClass(PackageName, GafferFlagScriptItem)


def RegisterGUI():
    """ Register the GafferUIItem and GafferAddLocationAction classes that are used by the GUI.
    
    GafferAddLocationAction  - Controls how this item appears in the "Add..." menu
    GafferUIItem             - Controls the tabs in the Gaffer UI 
    """
    from Katana import UI4
    from PluginAPI.BaseGafferAction import BaseGafferAddLocationAction
    UndoGrouping = UI4.Util.UndoGrouping.UndoGrouping
    FnKatImport.FromObject(GafferAPI.UIItems, merge = ['*'])
    
    #//////////////////////////////////////////////////////////////////////////////
    #///
    class GafferFlagUIItem(GafferUIItem):
        # decorateChildTreeItem no longer required to hide children, left
        # here to retain correct behavior in Katana <= 2.5.5
        def decorateChildTreeItem(self, rootProducer, item, childItem):
            childItem.setHidden(True)
        
        def isChildVisible(self, name, type):
            """Flag doesn't show children of any type."""
            return False
    
    class AddWhiteFlagAction(BaseGafferAddLocationAction):
        def __init__(self, parent, parentPath, nodeName, preselectMethod):
            BaseGafferAddLocationAction.__init__(self, parent, self.getActionName(),
                     parentPath, nodeName, preselectMethod)
            self.setIcon(UI4.Util.ScenegraphIconManager.GetIcon('polymesh'))
        
        def go(self, checked=None):
            node = self.getNode()
            if not node: return
            
            u = UndoGrouping(self.getActionName())
            
            newPath = node.addWhiteFlag('whiteFlag1', self.getParentPath(), scriptItemClassName = self.getPackageName())
            self.preselectPath(newPath)
        
        def getPackageName(self):
            return PackageName
    
        @staticmethod
        def getActionName():
            return 'Add White Flag'
        
        
        @staticmethod
        def getMenuGroup():
            return None
    
    # Register the ScriptAction
    GafferAPI.GafferPluginsModule.RegisterCreateCallback('AddWhiteFlagAction', AddWhiteFlagAction)
    GafferAPI.GafferPluginsModule.RegisterGafferUIItem(PackageName, GafferFlagUIItem)
