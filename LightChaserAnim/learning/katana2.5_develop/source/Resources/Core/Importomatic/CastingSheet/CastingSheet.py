# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

"""
The casting sheet importomatic plugin will open an xml file (casting sheet)
which contains assets and names for those assets.

Below is an example casting sheet file.

<castingsheet>
    <entry assetid="/tmp/Gnome.abc" name="gnomeA"/>
    <entry assetid="/tmp/Pony.xml" name="ponyA"/>
</castingsheet>

The 'asset id' xml attribute is an 'asset id' to be used with whichever asset management
system you are working with.

If you are working with files then the asset id is a file path.

The 'name' xml attribute is the identifier which will be used to
reference an instance of that asset in your katana scene. It is used to determine
the default scene graph location for the asset and to label the node used to import it.

It is important to specify a type mapping for the casting sheet to be able to determine
which plugin loads which asset type.

The type mapping is in CastingSheetConstants.

This plugin wraps the content produced by the importomatic asset plugins.
"""

#/////////////////////////////////////////////////////////////////////////////

import logging

import CastingSheetConstants
import CastingSheetIO
import Katana
from Katana import AssetAPI
from Katana import NodegraphAPI
from Katana import Plugins
from Katana import ResourceFiles
from Katana import Utils

AssetModule = Plugins.ImportomaticAPI.AssetModule
ImportomaticAPI = Plugins.ImportomaticAPI

#/////////////////////////////////////////////////////////////////////////////
#// Globals

CASTINGSHEET_TYPE = "CastingSheet"
CHAR_WIDTH = 10.0
EMPTY_NODE_WIDTH = 48
UNLOCK, LOCK = (0, 1)
ZERO = 0.0

log = logging.getLogger("CastingSheet")

#/////////////////////////////////////////////////////////////////////////////

def Register():
    ImportomaticAPI.AssetModule.RegisterCreateCallback('Add Casting Sheet', FindAndAddCastingSheetCall)
    ImportomaticAPI.AssetModule.RegisterBatchCreateCallback('CastingSheet', AddCastingSheetCall)
    ImportomaticAPI.AssetModule.RegisterType(CASTINGSHEET_TYPE, CastingSheetModule())

#/////////////////////////////////////////////////////////////////////////////

def RegisterGUI():
    """
    Construct all gui related objects here.
    RegisterGUI will not be called in batch mode.
    """
    from Katana import QtGui, QtCore
    
    # Register this class in the global scope
    # but only if the UI is being used.
    
    global SetLockedAction
    class SetLockedAction(QtGui.QAction):
        """
        Locks an asset to its currently chosen version.
        If the casting sheet changes this asset's version will stay the same.
        """
        def __init__(self, castingSheetNode, parent, node, label, state):
            QtGui.QAction.__init__(self, label, parent)
            self.__node = node
            self.__state = state
            self.__castingSheetNode = castingSheetNode
        
            self.connect(self, QtCore.SIGNAL('triggered(bool)'), self.__go)
        
        @undogroup("Lock Asset Version")
        def __go(self, checked=False):
            SetLocked(self.__node, self.__state)

#//////////////////////////////////////////////////////////////////////////////

class CastingSheetModule(ImportomaticAPI.AssetModule):
    def getAssetTreeRoot(self, node):
        return CastingSheetTreeRoot(node)

#/////////////////////////////////////////////////////////////////////////////

class undogroup(object):
    """
    Used as a decorator around node and script item methods.
    This wraps an undo block around the method that uses it.
    
    A string argument must be given to it.
    
    @undogroup("My changes")
    def makeChange():
        ...
        
    This implementation is copied from the ShadowManger super tool.
    """
    def __init__(self, name):
        # Ensure that we are passing a string in --> @undogroup("HERE")
        
        if type(name) != str:
            raise Exception("Decorator only accepts an undo item label as its parameter")
        
        # This will be the label for our undo group
        
        self.__name = name

    def __call__(self, f):
        def wrapped_f(*args):
            
            # Start undo block
            
            Utils.UndoStack.OpenGroup(self.__name)
            
            # Try it
            
            try:
                return f(*args)
            
            # Close the block no matter what
            
            finally:
                Utils.UndoStack.CloseGroup()
                
        return wrapped_f

#/////////////////////////////////////////////////////////////////////////////

class parameterChange(object):
    """
    Setting the value of a parameter can be done in multiple
    function calls.
    For example:
    
    parameter.setValue(123)
    parameter.setHintString("")
    
    Finalize value is needed to let the rest of the system
    know that the parameter has finished being changed.
    
    parameter.setValue(123)
    parameter.setHintString("")
    parameter.finalizeValue() # <---- I'm done
    
    This context manager ensures that finalizeValue
    is called once everything has been set up.
    
    Below is an example of usage.
    
    with parameterChange(parameter):
        parameter.setValue(123)
        parameter.setHintString("")
    """
    
    def __init__(self, parameter):
        self.__parameter = parameter
    
    def __enter__(self):
        pass
    
    def __exit__(self, type, value, tb):
        self.__parameter.finalizeValue()

#/////////////////////////////////////////////////////////////////////////////

def setCastingStringParamExpression(node, name, expression, hintsDict={}):
    """
    Set an expression on a string parameter in the casting info group parameter.
    If the castingInfo parameter hasn't been created yet, create it.
    If the string parameter hasn't been created yet, create it.
    """
    
    # Lazily construct the casting info
    #
    castingInfo = node.getParameter('castingInfo')
    if not castingInfo:
        parameters = node.getParameters()
        castingInfo = parameters.createChildGroup('castingInfo')
    
    # Lazily construct the string parameter
    #
    param = castingInfo.getChild(name)
    if not param:
        param = NodegraphAPI.Parameter.createChildString(castingInfo, name, "")
    
    # Set up the parameter
    #
    with parameterChange(param):
        # Set contents
        #
        param.setExpression(expression)
    
        # And the hints...
        #
        hintString = str(hintsDict)
        param.setHintString(hintString)

#/////////////////////////////////////////////////////////////////////////////

def setCastingParam(node, name, value, create=NodegraphAPI.Parameter.createChildString, hintsDict={}):
    """
    Set the value of a parameter in the casting info group parameter.
    If the castingInfo parameter hasn't been created yet, create it.
    If the parameter specified by 'name' hasn't been created yet, create it.
    """
    
    # Lazily construct the casting info
    #
    castingInfo = node.getParameter('castingInfo')
    if not castingInfo:
        parameters = node.getParameters()
        castingInfo = parameters.createChildGroup('castingInfo')
        
    # Assign
    #
    param = castingInfo.getChild(name)
    if not param:
        # The parameter's value could have been set here
        # on construction, but this code reads more clearly
        # of it all happens in one place.
        valueType = type(value)
        defaultValue = valueType()
        param = create(castingInfo, name, defaultValue)

    # Set up the parameter
    #
    with parameterChange(param):
        # Value
        param.setValue(value, ZERO)
        # Hint
        hintString = str(hintsDict)
        param.setHintString(hintString)

#/////////////////////////////////////////////////////////////////////////////

def getCastingParam(node, name, default):
    """
    Retrieves the value of a parameter stored in the 'castingInfo' group node.
    Takes a default input which is returned if nothing is found.
    """
    param = node.getParameter('castingInfo.%s' % name)
    if param:
        return param.getValue(ZERO)

    return default

#/////////////////////////////////////////////////////////////////////////////

class CastingEntryTreeRoot(object):
    """
    This dynamically wraps the AssetTreeChild object
    that an importomatic asset plugin will have created for the
    node type that it creates. 
    
    It allows us to override behaviour without inheriting.
    We can't use inheritance because we don't have access to the AssetTreeChild
    object types of the other plugins.
    
    Working in this way makes the Casting Sheet mostly type agnostic.
    """
    def __init__(self, handler, castingSheetEntryNode, castingSheetNode):
        
        # The AssetTreeChild object that we wrap
        self.__handler = handler
        
        # The nodegraph node that the handler represents
        self.__castingSheetEntryNode = castingSheetEntryNode
        
        # The nodegraph node that contains all the casting sheet information
        # This is the top level node that stores the state information for
        # this casting sheet instance
        self.__castingSheetNode = castingSheetNode
        
    def __getattr__(self, name):
        """
        Used to delegate all method calls to the
        handler (AssetTreeChild) object that we wrap.
        """
        
        # If we haven't got our own implementation
        # then delegate
        return getattr(self.__handler, name)
    
    def addToContextMenu(self, menu, importomaticNode):
        """
        Provides a Lock/Unlock mechanism for the assets.
        """
        
        # If we are using the 'File'
        # asset management plugin then there
        # is no versioning information and therefore
        # no assets are lockable
        if self.isLockable():
            menu.addSeparator()
            
            # If the asset is already locked we need to present an 'Unlock' option
            if self.isLocked():
                menu.addAction(SetLockedAction(self.__castingSheetNode, menu, self.__castingSheetEntryNode, "Unlock", UNLOCK))
                
            # If it's not locked then present the option to lock to an asset version
            else:
                menu.addAction(SetLockedAction(self.__castingSheetNode, menu, self.__castingSheetEntryNode, "Lock To Version", LOCK))
        
        # Add the context menu items that the asset plugin
        # provides
        self.__handler.addToContextMenu(menu, importomaticNode)
    
    @undogroup("Set A Casting Sheet Entry AssetID")
    def setAssetId(self, assetId):
        """
        Used by the importomatic to set the version of an asset but it can be used in
        a more generic way to set the entire asset id.
        
        In our case we delegate the asset id change to the asset plugin.
        """
        
        # Delegate
        
        self.__handler.setAssetId(assetId)
    
    def isDeletable(self):
        """
        No assets in the casting sheet can be deleted.
        The casting sheet defines its own contents.
        To remove an item, remove it from the casting sheet.
        Alternatively you can ignore it with the 'Ignore' context menu it.
        """
        
        # The contents of the casting sheet
        # is dictated by its xml file
        
        return False
    
    def isLockable(self):
        """
        An asset is not lockable if there is no version
        information for it.
        """
        
        # An asset is lockable if it has
        # multiple versions
        
        assetId = self.__handler.getAssetId()
        assetPlugin = AssetAPI.GetDefaultAssetPlugin()
        versions = assetPlugin.getAssetVersions(assetId)
        lockable = len(versions) != 0
        
        return lockable
    
    def isLocked(self):
        """
        The lock state is stored as a number parameter on the castingInfo group node.
        """
        return IsLocked(self.__castingSheetEntryNode)
        
    def setItemState(self, item):
        """
        Used to update the lock icon when the asset version is locked
        """
        from Katana import UI4
        if self.isLockable() and self.isLocked():
            item.setIcon(
                    ImportomaticAPI.VERSION_COLUMN,
                    UI4.Util.IconManager.GetIcon('Icons/lock16_active.png')
            )
        
        self.__handler.setItemState(item)

#/////////////////////////////////////////////////////////////////////////////

class CastingSheetTreeRoot(ImportomaticAPI.AssetTreeChild):
    """
    The Importomatic handler that is used to represent the top level
    casting sheet.
    This is the root of the casting sheet tree item in the Importomatic Tree UI.
    """
    
    def __init__(self, node):
        # We store a reference to the casting sheet root node
        # and a cache of the handlers (AssetTreeChild) for each
        # child asset
        
        self.__node = node
        self.__children = None
    
    def getAssetId(self):
        """
        The asset id is stored as a string parameter in castingInfo.asset
        in the casting sheet node.
        """
        node = self.__node
        
        assetParam = node.getParameter('castingInfo.asset')
        if assetParam != None:
            assetId = assetParam.getValue(ZERO)
            return assetId
        
        return None
    
    @undogroup("Set Casting Sheet Asset ID")
    def setAssetId(self, assetId):
        """
        Setting the asset id of a casting sheet node
        synchronizes its contents to a casting sheet file.
        It happens immediately.
        """
        SyncCastingSheet(self.__node, assetId)
    
    def setItemState(self, item):
        item.setText(ImportomaticAPI.NAME_COLUMN, 'Casting Sheet')
    
    def getChildren(self):
        """
        Get hold of handlers for all the child asset nodes.
        """
        # Lazily construct the list of children
        
        if not self.__children:
            self.__children = []
            merge = _GetMergeNode(self.__node)
            nodeChildren = _GetInputNodes(merge)
            for child in nodeChildren:
                
                handler = AssetModule.GetHandlerForNode(child)
                if handler:
                    
                    childRoot = handler.getAssetTreeRoot(child)
                    if childRoot:
                        
                        # Wrap our own tree handler around
                        # the one provided by the asset plug-in
                        
                        childRoot = CastingEntryTreeRoot(childRoot, child, self.__node)
                        
                        self.__children.append(childRoot)
        
        # Return the list
        
        return self.__children
    
    def addNodeObservers(self, callback):
        callback(self.__node)
    
    def getItemKey(self):
        return self.__node
    
    def isIgnorable(self):
        return True
    
    def isIgnored(self):
        return self.__node.isBypassed()
    
    def setIgnored(self, state):
        # Ignore all the entries in this casting sheet (the assets)
        
        for child in self.__children:
            child.setIgnored(state)
        
        # Ignore the merge node.
        # The graveyard isn't connected to anything
        # so there is no need to ignore it
        
        merge = _GetMergeNode(self.__node)        
        merge.isBypassed()
        
        # Ignore this node too
        
        return self.__node.setBypassed(state)
    
    def isDeletable(self):
        """
        Yes we can delete hte casting sheet itself.
        """
        return True
    
    def delete(self):
        """
        Deletes the casting sheet and everything that comes with it.
        """
        self.__node.delete()
        
    def getEditor(self, widgetParent):
        """
        Only displays the casting sheet scene graph location.
        The asset name isn't editable. You have to add a new casting sheet.
        You can change the casting sheet version though via the importomatic tree view.
        """
        
        # Only use this if we are in UI mode.
        #
        from Katana import UI4, QT4FormWidgets
        node = self.__node
        group = QT4FormWidgets.PythonGroupPolicy("Casting Sheet Information")
        group.getWidgetHints()['hideTitle'] = True
        
        paramPolicy = UI4.FormMaster.ParameterPolicy
        
        # The name policy
        #
        nameParam = node.getParameter("castingInfo.name")
        namePolicy = paramPolicy.CreateParameterPolicy(group, nameParam)
        group.addChildPolicy(namePolicy)
        
        factory = UI4.FormMaster.KatanaFactory.ParameterWidgetFactory
        w = factory.buildWidget(widgetParent, group)
        w.showPopdown(True)
        
        return w

#/////////////////////////////////////////////////////////////////////////////

def FindPluginFromAssetIdType(assetId):
    """
    Given an asset id, find the type of the asset and look for a plugin
    that can open that type. 
    """
    
    # Get hold of the primary asset plugin
    #
    assetPlugin = AssetAPI.GetDefaultAssetPlugin()
    
    # Get hold of the asset type name
    # so that we can match it up with a plug-in
    # for loading that particular type
    #
    attrs = assetPlugin.getAssetAttributes(assetId, "version")
    if not attrs:
        log.error("No attributes for asset '%s'. Ensure you are using the correct asset management plug-in for this asset id." % assetId)
        return None
    
    assetType = attrs["type"]
    
    # Look for the right importomatic plugin
    # for this type
    #
    plugin_name = CastingSheetConstants.TYPE_MAPPING.get(assetType, None)
    
    # Check that there is a plugin for this type
    #
    if not plugin_name:
        log.error("Unable to find a plugin mapping to load the given asset type '%s'" % assetType)
        return None
    
    # Check that there is a plugin for this type
    #
    if not AssetModule.HasBatchCreateCallback(plugin_name):
        log.error("Unable to find a plugin registered to load the given asset type '%s'" % assetType)
        return None

    # Load it
    #
    return plugin_name

#//////////////////////////////////////////////////////////////////////////////

def IsLocked(node):
    locked = getCastingParam(node, "locked", False)
    return locked

#//////////////////////////////////////////////////////////////////////////////

def GetCastingName(node):
    castingName = getCastingParam(node, "name", None)
    if not castingName:
        log.error("Found a casting node entry '%s' with no casting name" % node.getName())
    return castingName

#//////////////////////////////////////////////////////////////////////////////

def _GetNodeByType(node, nodeTypeNode):
    children = node.getChildren()
    for c in children:
        typeName = c.getType() 
        if typeName == nodeTypeNode:
            return c
    
    return None

#//////////////////////////////////////////////////////////////////////////////

def _GetGraveyardNode(node):
    return _GetNodeByType(node, "Group")

#//////////////////////////////////////////////////////////////////////////////

def _GetMergeNode(node):
    return _GetNodeByType(node, "Merge")

#//////////////////////////////////////////////////////////////////////////////

def _InputNodeIterator(node):
    inputs = node.getInputPorts()
    for i in inputs:
        ports = i.getConnectedPorts()
        for p in ports:
            inputNode = p.getNode()
            yield inputNode

#//////////////////////////////////////////////////////////////////////////////

def _GetInputNodes(node):
    inputs = list(_InputNodeIterator(node))
    return inputs

#//////////////////////////////////////////////////////////////////////////////

def _ReplaceTagsWithNumbers(assetId):
    """
    Given an asset id that looks like this:
    
    mock://foo/bar/latest
    
    replace the tag with the latest version
    
    mock://foo/bar/3
    """
    assetPlugin = AssetAPI.GetDefaultAssetPlugin()
    version = assetPlugin.resolveAssetVersion(assetId)
    fields = assetPlugin.getAssetFields(assetId, False)
    fields["version"] = version
    assetId = assetPlugin.buildAssetId(fields)
    
    return assetId

#//////////////////////////////////////////////////////////////////////////////

def _PopulatePreCondition(filepath):
    """
    Check that we will be able to load each item in our casting sheet
    before we try to do it.
    """
    
    # Check it is a castingsheet xml file
    #
    if not CastingSheetIO.IsCastingSheet(filepath):
        return False
    
    # Iterate over all the asset ids in the casting sheet file
    # and check that they can be loaded.
    #
    for assetId, name in CastingSheetIO.Iterator(filepath):
        
        # Search for a plug-in for this particular assetId
        #
        plugin_name = FindPluginFromAssetIdType(assetId)
        
        # No plugin found
        #
        if plugin_name == None:
            log.error("We were unable to find a plug-in to load the the asset '%s' in the casting sheet '%s'." % (assetId, filepath))
            return False
    
    # Return None if everything went ok.
    # 
    return True

#//////////////////////////////////////////////////////////////////////////////

def _EstimateNodeWidth(node):
    """
    Get a guaranteed maximum width for the given node. Based on the width of
    the standard font and how many characters are in the node's name
    """
    nodeWidthEstimate = (CHAR_WIDTH * len(node.getName())) + EMPTY_NODE_WIDTH
    return nodeWidthEstimate

#//////////////////////////////////////////////////////////////////////////////

def _LayoutContents(node):
    '''
    Produce a layout for the internals of the CastingSheet node that
    looks a bit like the diagram below...

                                         graveyard
                                          
    AlembicAsset(Entry[0])        ScenegraphXMLAsset(Entry[1])        AlembicAsset(Entry[2]) ...
        |                                    |                                  |
        -------------------------------------------------------------------------
                                             |
                                           merge
                                             |
                                            out

    '''
    try: from Katana import DrawingModule
    except: return

    # Casting sheet internals
    #
    children = node.getChildren()
    
    # All our casting sheet internal nodes
    #
    graveyard = _GetGraveyardNode(node)
    merge = _GetMergeNode(node)
    
    entries = _GetInputNodes(merge)
    entryCount = len(entries)
    
    OUT_OF_WAY = (0, 500)
    
    # Padding between nodes
    #
    xPadding = 50.0
    yPadding = 100.0
    
    # Merge width
    #
    X, Y = (0,1)
    AX, AY, BX, BY = range(4)
    mergeWidth = _EstimateNodeWidth(merge)
    
    ORIGIN = (0, -100)
    
    # Merge position
    #
    NodegraphAPI.SetNodePosition(merge, ORIGIN)
    
    # Graveyard position.
    # Placed at the top center
    #
    graveyardXY = (ORIGIN[X], ORIGIN[Y] + (yPadding * 2))
    NodegraphAPI.SetNodePosition(graveyard, graveyardXY)
    
    # Get the maximum width our child nodes could have
    #
    maxWidth = max(( _EstimateNodeWidth(entry) for entry in entries ))
    cellWidth = maxWidth + xPadding
    
    # Work out the bounds for our children
    #
    totalWidth = (cellWidth * (entryCount - 1))
    halfTotalWidth = int(totalWidth / 2.0)
    
    # The nodes are drawn from left to right
    # between the merge node and the graveyard node
    #
    xCursor = ORIGIN[X] - halfTotalWidth
    yCursor = ORIGIN[Y] + yPadding
    
    for entry in entries:
        xy = (xCursor, yCursor)
        
        NodegraphAPI.SetNodePosition(entry, xy)
        
        xCursor += cellWidth
        
    # Organize the graveyard in to a flat list of nodes
    # sorted alphabetically so we can see what they are
    #
    yCursor = ORIGIN[Y]
    graveyardChildren = graveyard.getChildren()
    graveyardChildren = sorted(graveyardChildren, lambda a, b: a < b)
    for child in graveyardChildren:
        xy = ( ORIGIN[X], yCursor)
        NodegraphAPI.SetNodePosition(child, xy)
        yCursor -= yPadding

#//////////////////////////////////////////////////////////////////////////////

def _Populate(filepath, node, graveyard={}):
    """
    Populate the casting sheet node with the contents of the casting sheet file
    pointed to by 'filepath'.
    
    Use the graveyard parameter to bring back locked nodes that were not part of the last
    set casting sheet version.  
    """
    
    # Create a new merge node
    #
    merge = NodegraphAPI.CreateNode('Merge')
    merge.setName('MergeCastingSheet')
    merge.setParent(node)
    mergeOut = merge.getOutputPort("out")
    
    # Connect the merge node
    # to this nodes output port
    #
    returnOut = node.getReturnPort("out")
    mergeOut.connect(returnOut)
    
    # Resolve the asset file path
    #
    assetPlugin = AssetAPI.GetDefaultAssetPlugin()
    
    # Iterate over all the entries
    # in the casting sheet loading
    # their respective nodes
    #
    GRID_WIDTH = 200
    GRID_Y_OFFSET = 200
    FIRST = 0
    ID, NAME = (0, 1)
    for index, asset in enumerate(CastingSheetIO.Iterator(filepath)):
        assetId = asset[ID]
        assetName = asset[NAME]
        
        # If it exists in the 'graveyard'.
        # bring it back to life
        #
        child = graveyard.get(assetName, None)
        
        if not child:
        
            # Get hold of a plug-in that can load this asset
            #
            plugin_name = FindPluginFromAssetIdType(assetId)
            
            # Ensure that a matching plug-in was found
            # for the asset type
            #
            if plugin_name == None:
                log.error("We were unable to find a plug-in to load the the asset '%s'." % (assetId))
            
            else:
                
                # The location of the casting sheet asset in the scene graph
                #
                locationExpression = 'getParam("%s.castingInfo.name") + "/" + getNode(nodeName).getParent().castingInfo.name' % (node.getName())
                
                # Load it in
                #
                child = AssetModule.TriggerBatchCreateCallback(plugin_name, node, assetId, locationExpression)
                
                # We can try to set the name of the casting sheet item
                # but if there is already an item that exists in the scene
                # with that same name then we will be given a different name.
                #
                child.setName(assetName)
                
                # So store the casting sheet name on the node as a parameter.
                # We will use this later for synching.
                #
                setCastingParam(child, "name", assetName)
                setCastingParam(child, "locked", UNLOCK, NodegraphAPI.Parameter.createChildNumber)
        
        if child:
            # Re-parent the node
            #
            child.setParent(node)
            
            # Merge this input in
            #
            entry = merge.addInputPort(assetName)
            main = child.getOutputPortByIndex(FIRST)
            
            main.connect(entry)
            
        else:
            log.warning("We were unable to create a new casting node for the casting sheet entry '%s' at '%s'." % (assetName, assetId))

#//////////////////////////////////////////////////////////////////////////////

def SetLocked(node, state):
    '''
    Lock an asset to a particular version.
    '''
    
    int_state = int(state)
    setCastingParam(node, "locked", state, NodegraphAPI.Parameter.createChildNumber)
    
    # If we've unlocked the casting sheet entry
    # then we want to go back to the version
    # it is set to in the casting sheet.
    #
    # So re-sync the casting sheet.
    #
    castingSheet = node.getParent()
    assetId = getCastingParam(castingSheet, "asset", None)
    if assetId:
        SyncCastingSheet(castingSheet, assetId)

#//////////////////////////////////////////////////////////////////////////////

def FindAndAddCastingSheetCall(importomaticNode):
    # Dynamically reference the function by name.
    #
    # This allows us to reload the CastingSheet module at run time
    # and get a new implementation.
    #
    return FindAndAddCastingSheet(importomaticNode)

#//////////////////////////////////////////////////////////////////////////////

def AddCastingSheetCall(importomaticNode, assetId, locationExpression):
    # Dynamically reference the function by name.
    #
    # This allows us to reload the CastingSheet module at run time
    # and get a new implementation.
    #
    return AddCastingSheet(importomaticNode, assetId, locationExpression)

#//////////////////////////////////////////////////////////////////////////////

@undogroup("Add Casting Sheet")
def FindAndAddCastingSheet(importomaticNode):
    """
    Open a browser and load the chosen casting sheet. 
    """
    
    # Open up a browser to choose a casting sheet
    
    from Katana import UI4
    hints = {'fileTypes': 'cast',
             'context': AssetAPI.kAssetContextCastingSheet}
    assetId = UI4.Util.AssetId.BrowseForAsset('', 'Select file', False, hints)

    # Nothing found from the browser
    
    if assetId == None:
        return None
    
    return AddCastingSheet(importomaticNode, assetId)

#//////////////////////////////////////////////////////////////////////////////

@undogroup("Add Casting Sheet")
def AddCastingSheet(importomaticNode, assetId, locationExpression=None):
    """
    Load the chosen casting sheet specified by assetId to the location specified by locationExpression.
    """
    
    # We can't do anything without a casting sheet asset id
    
    if assetId == None:
        log.error("No asset id given for new casting sheet")
        return

    # Resolve the asset file path
    #
    # Get hold of the primary asset plugin
    # so that we can resolve the casting sheet asset.
    
    assetPlugin = AssetAPI.GetDefaultAssetPlugin()
    filepath = assetPlugin.resolveAsset(assetId)
    if not filepath:
        log.error("Unable to resolve the casting sheet asset ID for '%s'" % assetId)
        return
    
    # Check if we will be able to load
    # the asset ids in the casting sheet on file
    
    if not _PopulatePreCondition(filepath):
        return
    
    # Store everything inside of the casting sheet group
    
    node = NodegraphAPI.CreateNode('Group')
    node.setName('CastingSheet')
    node.setType(CASTINGSHEET_TYPE)
    node.addOutputPort('out')
    returnOut = node.getReturnPort("out")
    
    # CastingSheet parameters - Scene graph location
    
    if locationExpression:
        setCastingStringParamExpression(node, "name", locationExpression, hintsDict={ 'widget' : 'scenegraphLocation' })
        
    # If a locationExpression wasn't given
    # default to a sensible place
    
    else:
        location = '/root/world/geo/%s' % node.getName()
        setCastingParam(node, "name", location, hintsDict={ 'widget' : 'scenegraphLocation' })
    
    # Create our graveyard node
    # This should be created first because it
    # never gets deleted
    
    graveyard = NodegraphAPI.CreateNode('Group')
    graveyard.setName('Graveyard')
    graveyard.setParent(node)
    
    # Read the casting sheet and populate
    # the casting sheet node.
    
    _Populate(filepath, node)
    
    # Now store the asset id so that we know where this all came from
    
    setCastingParam(node, "asset", assetId, hintsDict={ 'widget' : 'assetIdInput' })
    
    # Finally adjust the layout in the nodegraph ui
    
    _LayoutContents(node)
    
    return node

#//////////////////////////////////////////////////////////////////////////////

@undogroup("Sync Casting Sheet To File")
def SyncCastingSheet(node, assetId):
    """
    Sync the casting sheet node to the given asset.
    
    -------------------------------------------------------------------
    Syncing steps
    
    Delete the merge node.
    
    Iterate over all of child nodes and ...
       - If they are locked:
           - Move the current node to the 'graveyard' group node.
               We are guaranteed that there will not be a node with the same casting name
               in this group node because if there had been it would have been in the
               casting sheet.
    
       - If they are not locked
           - Delete them
    
    For every node in the 'graveyard' node create an entry in a graveyard map.
    
    Create a new merge node.
    
    Iterate over all of the entries in the casting sheet:
        - If an asset of the given instance name exists in the graveyard map then:
           - re-parent the node under the casting sheet and use it
           
        - Otherwise:
           - Create a new node with the plug-in callback system
    
        Then connect the node to the merge node.
        
    Save the casting sheet asset id in to the parameter castingInfo.asset
    """
    
    # Type check
    #
    nodeType = node.getType()
    if nodeType != "CastingSheet":
        log.error("You are trying to delete a node that is not a casting sheet node")
        return
    
    # For the casting sheet we always
    # resolve the asset version in the asset id
    # immediately.
    #
    assetPlugin = AssetAPI.GetDefaultAssetPlugin()
    assetId = _ReplaceTagsWithNumbers(assetId)
    filepath = assetPlugin.resolveAsset(assetId)
    if not filepath:
        log.error("Unable to resolve the casting sheet asset ID for '%s' in the casting sheet node '%s'" % (assetId, node.getName()))
        return
    
    # Check if we will be able to load
    # the asset ids casting sheet on file
    #
    if not _PopulatePreCondition(filepath):
        return
    
    # Ensure that the casting sheet node has a valid internal state
    #
    children = list(node.getChildren())
    if not children:
        log.error("Casting sheet node '%s' is in an invalid state it has no merge node or graveyard node, remove it and create a new one" % node.getName())
        return
    
    # The graveyard node is always first, then the merge node.
    #
    graveyardNode = _GetGraveyardNode(node)
    mergeNode = _GetMergeNode(node)
    
    # This is done to support an assertion only.
    # It does not contribute to the functionality of this code.
    # 
    takenGraveyardNames = set((GetCastingName(child) for child in graveyardNode.getChildren()))
    
    # Get hold of the children before the
    # merge node is deleted
    #
    entries = _GetInputNodes(mergeNode)
    
    # In case the merge node doesn't exist
    #
    if mergeNode.getType() == "Merge":
        mergeNode.delete()
    
    # Everything else is a casting sheet entry
    #
    for child in entries:
        if IsLocked(child):
            assert GetCastingName(child) not in takenGraveyardNames
            child.setParent(graveyardNode)
        else:
            child.delete()
    
    # Create a casting name to node mapping
    #
    graveyard = [ (GetCastingName(n), n) for n in graveyardNode.getChildren() ]
    graveyard = dict(graveyard)
    
    # Populate the node with casting sheet entries
    #
    _Populate(filepath, node, graveyard)
    
    # Now store the asset id so that we know where this all came from
    #
    setCastingParam(node, "asset", assetId, hintsDict={ 'widget' : 'assetIdInput' })
    
    # Finally rebuild the layout
    #
    _LayoutContents(node)
