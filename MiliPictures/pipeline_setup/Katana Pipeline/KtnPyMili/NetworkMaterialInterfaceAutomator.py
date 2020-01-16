# Copyright (c) 2016 Mili Pictures. All Rights Reserved.


"""
NAME: Network Material Interface Automator
ICON: icon.png
DROP TYPES:
SCOPE:
A helper untility to aid exposing NetworkMaterial ShadingNode parmameters
in the materials public interface.
"""

from __future__ import with_statement
from Katana import NodegraphAPI, QtCore, QtGui, QT4Widgets, Widgets, UI4
from Katana import Utils
import Katana
import copy

#//////////////////////////////////////////////////////////////////////////////

class AutomatorDialog(QtGui.QDialog):
    
    HEADERNAMES = [
        'Source Name',
        'Public Name',
        'Public Page',
        'Hints',
    ]
    
    for i,name in enumerate(HEADERNAMES):
        locals()['COLUMN_'+ name.upper().replace(' ', '_')] = i
    del i
    del name
    
    
    def __init__(self):
        self.__building = True
        
        self._buildPresets()

        QtGui.QDialog.__init__(self)
        self.setWindowTitle('Public Interface Automator')

        QtGui.QVBoxLayout(self)
        
        self.__splitter = QtGui.QSplitter(QtCore.Qt.Vertical, self)
        self.layout().addWidget(self.__splitter)
        
        self.__splitTop = QtGui.QWidget(None)
        QtGui.QVBoxLayout(self.__splitTop)
        self.__splitter.addWidget(self.__splitTop)
        self.__splitTop.setMinimumHeight(350)

        self.__splitBottom = QtGui.QWidget(None)
        
        QtGui.QVBoxLayout(self.__splitBottom)
        self.__splitter.addWidget(self.__splitBottom)
        
        self.__splitter.setChildrenCollapsible(False)
        
        self.__buildToolbar(self.__splitTop)
        self.__buildTree(self.__splitTop)
        self.__buildScriptArea(self.__splitBottom)

        self._loadPreset( self.__presetNames[0] )
        
        self.__building = False
        
        self.__refresh()
        
        self.setMinimumWidth(600)
        self.setMinimumHeight(550)

    def _buildPresets( self ) :

        # We want to choose the order of these...
        self.__presetNames = []
        self.__presets = {}

        self.__presetNames.append( 'Copy Names')
        self.__presets[ self.__presetNames[-1] ] = \
            'dstName = paramName'

        self.__presetNames.append( 'Copy Names with prefix' )
        self.__presets[ self.__presetNames[-1] ] = \
            'dstName = "change me to a prefix" + paramName'
 
        self.__presetNames.append( 'CopyNames, fixed Page' )
        self.__presets[ self.__presetNames[-1] ] = \
            'dstName = paramName\ndstPage = "change me to page name"'
        
        self.__presetNames.append( 'Copy Names, Page from node name' )
        self.__presets[ self.__presetNames[-1] ] = \
            'dstName = paramName\ndstPage = nodeName'
       
        self.__presetNames.append( 'Copy Names, prefix on conflict' )
        self.__presets[ self.__presetNames[-1] ] = \
"""if paramName not in persistentData :
    dstName = paramName
else :
    # For any node, params have unique names
    dstName = "%s_%s" % ( paramName, nodeName )
persistentData[ dstName ] = True
"""

    #//////////////////////////////////////////////////////////////////////////
    
    def __buildToolbar(self, parent):
        self.__toolbarLayout = QtGui.QHBoxLayout()
        parent.layout().addLayout(self.__toolbarLayout)
        
        self.__scopePopup = QtGui.QComboBox(parent)
        
        self.connect(self.__scopePopup, QtCore.SIGNAL(
                'currentIndexChanged(const QString &)'), self.__refresh)
        
        self.__scopePopup.addItems(
                ['Selected ShadingNodes', 'All ShadingNodes',
                        'Upstream of NetworkMaterial'])
                        
        self.__toolbarLayout.addWidget(self.__scopePopup)
        
        
        self.__shadingNetworkMaterialsPopup = QtGui.QComboBox(parent)
        
        self.__shadingNetworkMaterialsPopup.addItems([x.getName() for x in
                self.__getNetworkMaterials()])
        
        self.connect(self.__shadingNetworkMaterialsPopup, QtCore.SIGNAL(
                'currentIndexChanged(const QString &)'), self.__refresh)
        
        self.__shadingNetworkMaterialsPopup.hide()
        
        self.__toolbarLayout.addWidget(self.__shadingNetworkMaterialsPopup)
        
        self.__toolbarLayout.addStretch()
        
        self.__filterField = Widgets.FilterFieldWidget(parent)
        self.connect(self.__filterField, QtCore.SIGNAL('filterUpdate'),
                self.__filterUpdate)
        
        self.__toolbarLayout.addWidget(self.__filterField)
        
        
        self.__statusBar = QtGui.QLabel('', parent)
        parent.layout().addWidget(self.__statusBar)
    
    def __buildTree(self, parent):
        self.__tree = QT4Widgets.SortableTreeWidget(parent)
        parent.layout().addWidget(self.__tree)
        
        self.__tree.setHeaderLabels(self.HEADERNAMES)
        self.__tree.header().setClickable(False)
        self.__tree.header().setResizeMode(QtGui.QHeaderView.ResizeToContents)
        self.__tree.setSelectionMode(self.__tree.ExtendedSelection)
    
    def __buildScriptArea(self, parent):
        self.__presetToolbar = QtGui.QHBoxLayout()
        parent.layout().addLayout(self.__presetToolbar)

        self.__presetToolbar.addWidget(QtGui.QLabel('Presets:', parent))
         
        self.__presetsMenu = QtGui.QComboBox(parent)
        self.__presetsMenu.addItems( self.__presetNames )

        self.__presetToolbar.addWidget(self.__presetsMenu)

        self.__presetLoadButton = QtGui.QPushButton('Load', parent)
        self.connect(self.__presetLoadButton, QtCore.SIGNAL('clicked()'),
                self._loadPreset)

        self.__presetToolbar.addWidget(self.__presetLoadButton)

        self.__presetToolbar.addStretch()    
        
        self.__scriptText = TabbableTextEdit(parent)
        parent.layout().addWidget(self.__scriptText)

 
        self.__scriptToolbar = QtGui.QHBoxLayout()
        parent.layout().addLayout(self.__scriptToolbar)
        
        
        self.__scriptHelp = QT4FormWidgets.InputWidgets.HelpButton(
                parent)
        self.__scriptHelp.setAutoDefault(False)
        self.__scriptHelp.setHelpText("""
        local variables: <br>
        <b>nodeName</b>, <b>paramName</b> these are inputs, changing has no result<br>
        <b>dstName</b>, <b>dstPage</b> change these values sets exposure<br>
        <b>hints</b> set fields of this dict to assign widget hints<br>
        <b>persistentData</b>, a dictionary that is reset each time 'Run Script' is 
        pressed, rather than just for each parameter. Useful if you wish to store data 
        about previously visited parameters, for example.
        """)
        self.__scriptToolbar.addWidget(self.__scriptHelp)
        
        
        self.__scriptScope = QtGui.QComboBox(parent)
        
        self.__scriptToolbar.addWidget(QtGui.QLabel('Apply Script To', parent))
        
        self.__scriptScope.addItems(
                ['All Visible Items', 'Selected Visible Items', 'All Items'])
        
        self.__scriptToolbar.addWidget(self.__scriptScope)
        self.__scriptToolbar.addStretch()
        
        self.__undoDepth = 0
        self.__scriptUndoButton = QtGui.QPushButton('Undo', parent)
        self.__scriptUndoButton.setEnabled(False)
        self.connect(self.__scriptUndoButton, QtCore.SIGNAL('clicked()'),
                self.__undo)
        self.__scriptToolbar.addWidget(self.__scriptUndoButton)
        
        
        
        self.__scriptRunButton = QtGui.QPushButton('Run Script', parent)
        #self.__scriptRunButton.setEnabled(False)
        self.connect(self.__scriptRunButton, QtCore.SIGNAL('clicked()'),
                self.__evaluateScript)
        
        
        self.__scriptToolbar.addWidget(self.__scriptRunButton)
        
        
    
    #//////////////////////////////////////////////////////////////////////////
     
    def _loadPreset( self, presetName=None ) :

      if not presetName :
            presetName = str(self.__presetsMenu.currentText())
      
      if presetName in self.__presets :
            self.__scriptText.setText(self.__presets[presetName])


    def __getScopedNodes(self):

        nodes = self.__getAllShadingNetworkNodes() 
        
        if str(self.__scopePopup.currentText()).startswith('Upstream'):
            snmNodeName = str(self.__shadingNetworkMaterialsPopup.currentText())
            if not snmNodeName :
                return []

            snmNode = NodegraphAPI.GetNode(snmNodeName)
            if not snmNode:
                raise RuntimeError, "Unable to find the node '%s'" % snmNodeName

            upstreamSet = set(NodegraphAPI.Util.GetAllConnectedInputs(
                    set((snmNode,))))
            
            selectedNodes = []

            for node in nodes:
                if node in upstreamSet:
                    selectedNodes.append(node)
                else:
                    #any of my parents in there?
                    p = node.getParent()
                    while p:
                        if p in upstreamSet:
                            selectedNodes.append(node)
                            break
                        
                        p = p.getParent()
            
            nodes = selectedNodes
        else:
            
            if str(self.__scopePopup.currentText()).startswith('Selected'):
                selectedNodes = []
                allSelected = set(NodegraphAPI.GetAllSelectedNodes())
                for node in nodes:
                    p = node
                    while p:
                        if p in allSelected:
                            selectedNodes.append(node)
                            break
                        p = p.getParent()
                
                nodes = selectedNodes
        
        return nodes

    def __getAllShadingNetworkNodes(self) :

        nodes = NodegraphAPI.GetAllNodesByType('ArnoldShadingNode')
        nodes.extend( NodegraphAPI.GetAllNodesByType('PrmanShadingNode') )
        return nodes 
       
    def __getNetworkMaterials(self) :
        return NodegraphAPI.GetAllNodesByType('NetworkMaterial')
    
    def __refresh(self):
        if self.__building:
            return
        
        if str(self.__scopePopup.currentText()).startswith('Upstream'):
            self.__shadingNetworkMaterialsPopup.show()
        else:
            self.__shadingNetworkMaterialsPopup.hide()
        
        
        sm = Widgets.ScrollAreaMemory.ScrollAreaMemory(self.__tree)
        
        selectedItems = set(x.getItemData()['key'] \
                for x in self.__tree.selectedItems())
        closedItems = set()
        for i in xrange(self.__tree.topLevelItemCount()):
            item = self.__tree.topLevelItem(i)
            if not item.isExpanded():
                closedItems.add(item.getItemData()['key'])
        
        
        self.__tree.clear()
        
        with self.__tree.getUpdateSuppressor():
            for node in self.__getScopedNodes():
                item = QT4Widgets.SortableTreeWidgetItem(self.__tree,
                        node.getName())
                item.setIcon(self.COLUMN_SOURCE_NAME,
                        UI4.Util.IconManager.GetIcon('Icons/node16.png'))
                
                key = hash(node)
                
                item.setItemData({'name': node.getName(), 'key':key})
                
                if key in selectedItems:
                    item.setSelected(True)
                
                self.__buildNodeChildren(item, selectedItems)
                item.setExpanded(key not in closedItems)
                
                item.setText(self.COLUMN_PUBLIC_NAME, node.getParameter(
                        'publicInterface.namePrefix').getValue(0))
                item.setText(self.COLUMN_PUBLIC_PAGE, node.getParameter(
                        'publicInterface.pagePrefix').getValue(0))
                
                item.setHiliteColor(QtGui.QColor(52, 64, 64))
            self.__filterUpdate()
    
    
    def __buildNodeChildren(self, nodeItem, selectedItems):
        node = NodegraphAPI.GetNode(nodeItem.getItemData()['name'])
        
        for param in node.getParameter('parameters').getChildren():
            name = param.getName()
            if name == '__unused':
                continue
            
            if param.getType() != 'group':
                continue
            
            
            item = QT4Widgets.SortableTreeWidgetItem(nodeItem,
                    name)
            
            key = hash(param)
            item.setItemData({'name': param.getName(), 'key':key})
            
            if key in selectedItems:
                item.setSelected(True)
            
            self.__updateParamItem(item, param)
            
    
    
    def __updateParamItem(self, item, param):
        hints = self.__getHints(param)
        
        publicName = hints.get('dstName')
        if publicName:
            item.setText(self.COLUMN_PUBLIC_NAME, publicName)
        else:
            item.setText(self.COLUMN_PUBLIC_NAME, '')
        
        publicPage = hints.get('dstPage')
        if publicPage:
            item.setText(self.COLUMN_PUBLIC_PAGE, publicPage)
        else:
            item.setText(self.COLUMN_PUBLIC_PAGE, '')
    
    
    def __getHints(self, param):
        result = {}
        if not param:
            return result
        
        hintsParam = param.getChild('hints')
        if not hintsParam:
            return result
        
        try:
            hints = eval(hintsParam.getValue(0))
        except:
            hints = {}
        
        if not isinstance(hints, dict):
            hints = {}
        
        return hints
    
    
    def __filterUpdate(self, text=None, filterType=None):
        if text is None:
            text = self.__filterField.getFilterText()
            filterType = self.__filterField.getFilterType()
        
        totalShown = 0
        totalHidden = 0
        
        for i in xrange(self.__tree.topLevelItemCount()):
            nodeItem = self.__tree.topLevelItem(i)
            
            oneVisible = False
            for i in xrange(nodeItem.childCount()):
                childItem = nodeItem.child(i)
                name = str(childItem.text(0))
                
                if text in name:
                    oneVisible = True
                    childItem.setHidden(False)
                    totalShown += 1
                else:
                    childItem.setHidden(True)
                    totalHidden += 1
            
            nodeItem.setHidden(not oneVisible)
        
        statusText = '%i shown' % totalShown
        
        if totalHidden:
            statusText += ' <i>(%i hidden by filter)</i>' % totalHidden
        
        self.__statusBar.setText(statusText)
    
    
    def __evaluateScript(self):
        Utils.UndoStack.OpenGroup('Public Interface Automator Script')
        
        failed = False
        self.__changed = False
        try:
            scope = str(self.__scriptScope.currentText())
            
            I = QtGui.QTreeWidgetItemIterator(self.__tree)

            # make some persistant storage for the
            # lifetime of the script
            #import imp
            #persistentData = imp.new_module('user' )
            persistentData = {}


            while I.value():
                item = I.value()
                if item.parent():
                    withinScope = True
                    if scope == 'All Visible Items':
                        if item.isHidden():
                            withinScope = False
                        elif not item.parent().isExpanded():
                            withinScope = False
                    
                    if scope == 'Selected Visible Items':
                        if item.isHidden():
                            withinScope = False
                        elif not item.parent().isExpanded():
                            withinScope = False
                        
                        if not item.isSelected():
                            withinScope = False
                    
                    if withinScope:
                        self.__evaluateScriptForItem(item, persistentData)
                
                I+=1

        except:
            failed = True
            import traceback
            traceback.print_exc()
        finally:
            Utils.UndoStack.CloseGroup()
            
            if self.__changed:
                self.__undoDepth += 1
            
            if failed:
                #undo?
                
                if self.__changed:
                    if Widgets.MessageBox.Warning('Script Error',
                            'Check the shell for traceback',
                                    'Cancel', 'Undo') == 1:
                        self.__undo()
                else:
                    Widgets.MessageBox.Warning('Script Error',
                            'Check the shell for traceback',
                                    'Ok')
            
            
            self.__scriptUndoButton.setEnabled(self.__undoDepth > 0)
    
    def __undo(self):
        if self.__undoDepth:
            self.__undoDepth -= 1
        
        Utils.UndoStack.Undo()
        
        self.__refresh()
        self.__scriptUndoButton.setEnabled(self.__undoDepth > 0)
    
    
    def __evaluateScriptForItem(self, item, persistentData ):
        if not item.parent():
            return
        
        node = NodegraphAPI.GetNode(item.parent().getItemData()['name'])
        param = node.getParameter('parameters.'+item.getItemData()['name'])
        
        hints = self.__getHints(param)
        
        if 'dstName' in hints:
            dstName = hints['dstName']
            del hints['dstName']
        else:
            dstName = ''
        
        if 'dstPage' in hints:
            dstPage = hints['dstPage']
            del hints['dstPage']
        else:
            dstPage = ''
        

        localsDict = {
            'Katana':Katana,
            'nodeName': node.getName(),
            'paramName': param.getName(),
            'dstName':dstName,
            'dstPage':dstPage,
            'hints':copy.deepcopy(hints), #this is mutable so we must copy it
            'persistentData': persistentData,
            '__builtins__':__builtins__,
        }
        
        exec str(self.__scriptText.toPlainText()) in localsDict
        
        changed = False
        
        for key in ('dstName', 'dstPage', 'hints'):
            if localsDict[key] != locals()[key]:
                changed = True
                break
        
        if changed:
            self.__changed = True
            dstName = localsDict['dstName']
            dstPage = localsDict['dstPage']
            hints = localsDict['hints']
            
            hintsParam = param.getChild('hints')
            
            if not dstName and not dstPage:
                if hintsParam:
                    hintsParam.setValue('', 0)
                    param.deleteChild(hintsParam)
            else:
                if not hintsParam:
                    hintsParam = param.createChildString('hints', '')
                
                if dstName:
                    hints['dstName'] = dstName
                
                if dstPage:
                    hints['dstPage'] = dstPage
                
                hintsParam.setValue(repr(hints), 0)
            
            self.__updateParamItem(item, param)

    def __applyPreset( presetName ) :
        pass

#//////////////////////////////////////////////////////////////////////////////

class TabbableTextEdit(QtGui.QTextEdit):
    def __init__(self, *args):
        QtGui.QTextEdit.__init__(self,*args)
        font = QtGui.QFont(self.font())
        font.setFamily("Courier")
        self.setFont(font)
        self.setLineWrapMode(self.NoWrap)
    
    #TODO, handle delete?
    def keyPressEvent(self, keyEvent):
        if (keyEvent.key() == QtCore.Qt.Key_Tab):
            keyEvent.accept()
            self.insertPlainText('   ')
        else:
            QtGui.QTextEdit.keyPressEvent(self, keyEvent)

#//////////////////////////////////////////////////////////////////////////////


def go():
    AutomatorDialog().exec_()

#single shot so we can track undo events directly instead of grouped together
#QtCore.QTimer.singleShot(0.1, go)
app = AutomatorDialog()
app.show()

