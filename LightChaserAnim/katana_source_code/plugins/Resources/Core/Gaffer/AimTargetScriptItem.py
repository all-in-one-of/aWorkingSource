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
PackageName = 'GafferProAimTargetPackage'

#//////////////////////////////////////////////////////////////////////////////
#///

class GafferAimTargetScriptItem(ScriptItems.GafferScriptItem):
    @staticmethod
    def create(gnode, path):
        rigGroup = SA.BuildLocationPackageForPath(gnode, path)
        rigGroup.setName(os.path.basename(path) + '_package')
        rigGroup.setType(PackageName)
        SA.AddNodeReferenceForPath(gnode, path, 'package', rigGroup)
        
        primCreate = NodegraphAPI.CreateNode('PrimitiveCreate', rigGroup)
        primCreate.addInputPort('in')
        primCreate.getParameter('name').setExpression('getParent().pathName')
        primCreate.getParameter('type').setValue('locator', 0)
        
        SA.WireInlineNodes(rigGroup, [primCreate])

        SA.AddNodeReferenceParam(rigGroup, 'node_primCreate',
                primCreate)
        
        yPos=0
        NodegraphAPI.SetNodePosition(primCreate,    (0, yPos))
    
    def acceptsChild(self, childScriptItem):
        return False

    def supportsMuting(self):
        return False

    def supportsLocking(self):
        return True

    def isLocked(self):
        node = self.getNodeReference('node_primCreate')
        return node.getParameter('makeInteractive').getValue(0.0) != 'Yes'

    def setLocked(self, lock):
        node = self.getNodeReference('node_primCreate')
        node.getParameter('makeInteractive').setValue(
                "No" if lock else "Yes", 0.0)
    
    def getAbstractTransformNode(self):
        return self.getNodeReference('node_primCreate')
    
#//////////////////////////////////////////////////////////////////////////////
#///
def Register():
    """ Register the GafferScriptItem (none GUI) classes """ 
    GafferAPI.ScriptItems.RegisterPackageClass(PackageName, GafferAimTargetScriptItem)

def RegisterGUI():
    """ Register the GafferUIItem and GafferAddLocationAction classes that are used by the GUI.
    
    GafferAddLocationAction  - Controls how this item appears in the "Add..." menu
    GafferUIItem             - Controls the tabs in the Gaffer UI 
    """
    from Katana import UI4, QT4FormWidgets,FormMaster
    from PluginAPI.BaseGafferAction import BaseGafferAddLocationAction
    UndoGrouping = UI4.Util.UndoGrouping.UndoGrouping
    FnKatImport.FromObject(GafferAPI.UIItems, merge = ['*'])
        
    class GafferAimTargetUIItem(GafferUIItem):
    
        def getObjectTabPolicy(self):
            editorNode = self.getReferencedNode('node_primCreate')
            if not editorNode: return
    
    
            scriptItem = self.getScriptItem()
            
            rootPolicy = QT4FormWidgets.PythonGroupPolicy('object')
            rootPolicy.getWidgetHints()['hideTitle'] = True
    
            nodePolicy = FormMaster.CreateParameterPolicy(None,
                    editorNode.getParameter("transform"))
            rootPolicy.addChildPolicy(nodePolicy)
    
            return rootPolicy
    
    
        def getLinkingTabPolicy(self):
            editorNode = self.getReferencedNode('node_aimConstraint')
            if not editorNode: return
    
            scriptItem = self.getScriptItem()
    
            rootPolicy = QT4FormWidgets.PythonGroupPolicy('object')
            rootPolicy.getWidgetHints()['hideTitle'] = True
    
            nodePolicy = FormMaster.CreateParameterPolicy(None,
                    editorNode.getParameter("basePath"))
            rootPolicy.addChildPolicy(nodePolicy)
    
            return rootPolicy
    
    class GafferAimTargetScriptAction(BaseGafferAddLocationAction):
        
        def __init__(self, parent, parentPath, nodeName, preselectMethod):
            BaseGafferAddLocationAction.__init__(self, parent, self.getActionName() ,
                    parentPath, nodeName, preselectMethod)
            self.setIcon(LocalIcon('coordsys16'))
        
        def getPackageName(self):
            return PackageName
        
        @staticmethod
        def getActionName():
            return 'Add Aim Target'
        
        @staticmethod
        def getMenuGroup():
            return 'Lighting'
        
        def go(self, checked=None):
            node = self.getNode()
            if not node: return
            
            u = UndoGrouping(self.getActionName())
            
            newPath = node.addAimTarget(self.getParentPath(), 'aim1', scriptItemClassName = self.getPackageName())
            self.preselectPath(newPath)
    
    # Register the ScriptAction
    GafferAPI.GafferPluginsModule.RegisterCreateCallback('GafferAimTargetScriptAction', GafferAimTargetScriptAction)
    GafferAPI.GafferPluginsModule.RegisterGafferUIItem(PackageName, GafferAimTargetUIItem)
