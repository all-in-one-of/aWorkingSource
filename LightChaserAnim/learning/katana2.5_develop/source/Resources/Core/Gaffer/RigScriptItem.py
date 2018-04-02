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
PackageName = 'GafferProRigPackage'

#//////////////////////////////////////////////////////////////////////////////
#///

class GafferRigScriptItem(ScriptItems.GafferScriptItem):
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
       
        pointConstraint = NodegraphAPI.CreateNode('PointConstraint', rigGroup)
        pointConstraint.getParameter('basePath').setExpression('getParent().pathName')
        p = pointConstraint.getParameter('addToConstraintList')
        if p: p.setValue(1, 0)
        pointConstraint.setBypassed(True)

        orientConstraint = NodegraphAPI.CreateNode('OrientConstraint', rigGroup)
        orientConstraint.getParameter('basePath').setExpression('getParent().pathName')
        p = orientConstraint.getParameter('addToConstraintList')
        if p: p.setValue(1, 0)
        orientConstraint.setBypassed(True)

        orientConstraint.getOutputPortByIndex(0).connect(rigGroup.getReturnPort(
                rigGroup.getOutputPortByIndex(0).getName()))
        
        SA.WireInlineNodes(rigGroup, [primCreate, pointConstraint, orientConstraint])

        SA.AddNodeReferenceParam(rigGroup, 'node_primCreate',
                primCreate)
        SA.AddNodeReferenceParam(rigGroup, 'node_pointConstraint',
                pointConstraint)
        SA.AddNodeReferenceParam(rigGroup, 'node_orientConstraint',
                orientConstraint)
        
        yPos=0
        NodegraphAPI.SetNodePosition(primCreate,       (0, yPos))
        NodegraphAPI.SetNodePosition(pointConstraint,  (0, yPos-50))
        NodegraphAPI.SetNodePosition(orientConstraint, (0, yPos-100))
    
    def supportsMuting(self):
        return True

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
    
    def getPointConstraintNode(self):
        return self.getNodeReference('node_pointConstraint')

    def getOrientConstraintNode(self):
        return self.getNodeReference('node_orientConstraint')

#//////////////////////////////////////////////////////////////////////////////
#///

def Register():
    """ Register the GafferScriptItem (none GUI) classes """ 
    GafferAPI.ScriptItems.RegisterPackageClass(PackageName, GafferRigScriptItem)


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
    class GafferRigUIItem(GafferUIItem):
        def getObjectTabPolicy(self):
            editorNode = self.getReferencedNode('node_primCreate')
            if not editorNode: return
    
            scriptItem = self.getScriptItem()
            
            rootPolicy = QT4FormWidgets.PythonGroupPolicy('object')
            rootPolicy.getWidgetHints()['hideTitle'] = True
    
            nodePolicy = FormMaster.CreateParameterPolicy(None,
                    editorNode.getParameter("transform"))
            rootPolicy.addChildPolicy(nodePolicy)
    
            AddConstraintPolicy(scriptItem, rootPolicy,
                                scriptItem.getPointConstraintNode(), 'point',
                                ['targetPath.i0', 'allowMissingTargets',
                                 'baseOrigin', 'targetOrigin'])
    
            AddConstraintPolicy(scriptItem, rootPolicy,
                                scriptItem.getOrientConstraintNode(), 'orient',
                                ['targetPath', 'targetOrientation', 'targetFaceIndex',
                                 'allowMissingTargets', 'xAxis', 'yAxis', 'zAxis'])
    
            return rootPolicy
        
        def fillContextMenu(self, menu):
            GafferUIItem.fillContextMenu(self, menu)
            locked = self.getReferencedNode('node_primCreate').isLocked()
            
            scriptItem = self.getScriptItem()
            def Ungroup():
                u = UI4.Util.UndoGrouping.UndoGrouping(
                        'Ungroup Rig %s' % scriptItem.getFullName())
                
                gafferNode = scriptItem.getGafferPro()
                hcNode = SA.GetHierarchyCreateNode(gafferNode)
                treeEntry = hcNode.getLocationTree(
                        atLocation=scriptItem.getFullName())
                childPaths = [scriptItem.getFullName() + '/' + x.name
                        for x in treeEntry.children]
                
                cookedRoot = Nodes3DAPI.GetGeometryProducer(gafferNode)
                
                itemXforms = []
                
                for childPath in childPaths:
                    childScriptItem = gafferNode.getScriptItemForPath(childPath)
                    
                    xform = None
                    xformNode = None
                    if childScriptItem and hasattr(
                            childScriptItem, 'getAbstractTransformNode'):
                        xformNode = childScriptItem.getAbstractTransformNode()
                        if xformNode:
                            locProducer = cookedRoot.getProducerByPath(childPath)
                            if locProducer:
                                xform = locProducer.getFlattenedGlobalXform()
                    
                    newPath = hcNode.reparentLocation(childPath,
                            scriptItem.getFullName().rsplit('/', 1)[0])
                    if xform and xformNode:
                        t, s, r = GeoAPI.Util.Matrix.explodeMatrix4x4(xform)
                        xformNode.setInteractiveTransform(newPath, s, r, t,
                                NodegraphAPI.GetCurrentTime())
                
                scriptItem.delete()
            
            
            menu.addSeparator()
            menu.addAction('Ungroup Rig', Ungroup).setEnabled(not locked)
        
    

    class AddRigAction(BaseGafferAddLocationAction):
        def __init__(self, parent, parentPath, nodeName, preselectMethod):
            BaseGafferAddLocationAction.__init__(self, parent, self.getActionName(),
                     parentPath, nodeName, preselectMethod)
            self.setIcon(UI4.Util.ScenegraphIconManager.GetIcon('locator'))
        
        def go(self, checked=None):
            node = self.getNode()
            if not node: return
            
            u = UndoGrouping(self.getActionName())
            desiredPath = self.getParentPath() + '/rig1'
            newPath = node.addRig(desiredPath,
                                  alwaysCreateWithOptionalLeafRename = True, 
                                  scriptItemClassName = self.getPackageName())
            self.preselectPath(newPath)
        
        def getPackageName(self):
            return PackageName
        
        @staticmethod
        def getActionName():
            return 'Add Rig'
        
        @staticmethod
        def getMenuGroup():
            return 'Lighting'
        
    # Register the ScriptAction
    GafferAPI.GafferPluginsModule.RegisterCreateCallback('AddRigAction', AddRigAction)
    GafferAPI.GafferPluginsModule.RegisterGafferUIItem(PackageName, GafferRigUIItem)
