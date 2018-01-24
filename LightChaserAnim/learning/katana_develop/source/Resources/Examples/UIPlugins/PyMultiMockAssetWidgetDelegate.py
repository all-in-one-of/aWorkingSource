# Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

from Katana import QtCore, QtGui, QT4FormWidgets, UI4, AssetAPI, logging
from UI4.Util import AssetWidgetDelegatePlugins
import os
import tempfile
import ConfigurationAPI_cmodule as Configuration

#///////////////////////////////////////////////////////////////////////////////

# Set up a Katana logger
log = logging.getLogger("PyMultiMockAssetWidgetDelegate")

#///////////////////////////////////////////////////////////////////////////////

def _logDebug(message):
    """
    Logs additional information for debug purposes.

    This can be turned on by setting the MOCK_DEBUG environment variable.
    """
    if os.environ.get("MOCK_DEBUG", False):
        log.info(message)


def _getBaseDir():
    """
    Returns the default root directory of the MOCK asset database from the
    "MOCK_ASSET_DIR" environment variable.
    """
    return os.environ.get("MOCK_ASSET_DIR", tempfile.gettempdir())


def _listDirs(dir):
    """Returns an ordered list of directories inside a given folder"""
    dirs = []
    if not os.path.exists(dir):
        log.warning('Database directory not found: "%s"' % dir)
        return dirs

    children = os.listdir(dir)
    for child in children:
        path = os.path.join(dir, child)
        if os.path.isdir(path):
            dirs.append(child)

    return sorted(dirs)


def _getShows():
    """Return a list of shows in the mock asset database"""
    return _listDirs(_getBaseDir())


def _getShots(show=None):
    """Return a list of shots in the mock asset database"""
    if not show:
        return []
    showPath = os.path.join(_getBaseDir(), show)
    return _listDirs(showPath)


def _getAssets(show=None, shot=None):
    """Return a list of asset names in the mock asset database"""
    if not show or not shot:
        return []
    shotPath = os.path.join(_getBaseDir(), show, shot)
    return _listDirs(shotPath)


def _getVersions(show=None, shot=None, asset=None):
    """Return a list of versions in the mock asset database"""
    if not show or not shot or not asset:
        return []
    assetPath = os.path.join(_getBaseDir(), show, shot, asset)
    return _listDirs(assetPath)


def _getSandboxFiles():
    """
    Returns the list of filenames currently stored in the sandbox folder
    """
    sandboxFolder = os.getenv("MOCK_SANDBOX_DIR", tempfile.gettempdir())
    sandboxPrefix = "mock_sandbox_"
    filenames = os.listdir(sandboxFolder)
    fileList = []
    for filename in filenames:
        if filename.startswith(sandboxPrefix):
            fileList.append(filename[len(sandboxPrefix):])
    return fileList

#///////////////////////////////////////////////////////////////////////////////

class PyMockAssetControlWidget(AssetWidgetDelegatePlugins.BaseAssetControlWidget):
    """
    The asset control widget takes control of the display of any string
    parameters that have an asset id 'hint'.
    """

    def buildWidgets(self, hints):
        """Creates most of the UI widgets"""
        label = QtGui.QLabel(" (PyMock)", self)
        label.setProperty('smallFont', True)
        label.setProperty('boldFont', True)
        p = label.palette()
        p.setColor(QtGui.QPalette.WindowText, QtGui.QColor(255, 200, 0))
        p.setColor(QtGui.QPalette.Text, QtGui.QColor(255, 200, 0))
        label.setPalette(p)
        self.layout().addWidget(label)

        self.__protocolLabel = QtGui.QLabel("mock://")
        self.layout().addWidget(self.__protocolLabel)

        self.__showEdit = QT4FormWidgets.InputWidgets.InputLineEdit(self)
        self.layout().addWidget(self.__showEdit)

        self.layout().addWidget(QtGui.QLabel("/"))
        self.__shotEdit = QT4FormWidgets.InputWidgets.InputLineEdit(self)
        self.layout().addWidget(self.__shotEdit)

        self.layout().addWidget(QtGui.QLabel("/"))
        self.__assetEdit = QT4FormWidgets.InputWidgets.InputLineEdit(self)
        self.layout().addWidget(self.__assetEdit)

        self.layout().addWidget(QtGui.QLabel("/"))
        self.__versionEdit = QT4FormWidgets.InputWidgets.InputLineEdit(self)
        self.layout().addWidget(self.__versionEdit)

        self.connect(self.__showEdit, QtCore.SIGNAL("lostFocus"),
                     self.__lostFocus)
        self.connect(self.__shotEdit, QtCore.SIGNAL("lostFocus"),
                     self.__lostFocus)
        self.connect(self.__assetEdit, QtCore.SIGNAL("lostFocus"),
                     self.__lostFocus)
        self.connect(self.__versionEdit, QtCore.SIGNAL("lostFocus"),
                     self.__lostFocus)

        # Add a context menu to the protocol label
        self.__protocolLabel.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.connect(self.__protocolLabel,
                     QtCore.SIGNAL('customContextMenuRequested(const QPoint&)'),
                     self.onContextMenu)

        self.__popupMenu = QtGui.QMenu(self)
        self.__popupMenu.addAction("Swap protocol", self.onSwapProtocol)

    def onContextMenu(self, point):
        """
        Displays a simple context menu to swap the current protocol used to
        resolve asset IDs.
        """
        self.__popupMenu.exec_(self.__protocolLabel.mapToGlobal(point))

    def onSwapProtocol(self):
        """
        Swaps the current protocol used to resolve asset IDs.
        """
        if str(self.__protocolLabel.text()) == "mock://":
            self.__protocolLabel.setText("sandbox://")
        else:
            self.__protocolLabel.setText("mock://")
        self.emitValueChanged()


    def __lostFocus(self):
        """Triggered when the browser loses focus"""
        self.emitValueChanged()

    def setPalette(self, palette):
        self.__showEdit.setPalette(palette)
        self.__shotEdit.setPalette(palette)
        self.__assetEdit.setPalette(palette)
        self.__versionEdit.setPalette(palette)

    def setReadOnly(self, readOnly):
        self.__showEdit.setEnabled(not readOnly)
        self.__shotEdit.setEnabled(not readOnly)
        self.__assetEdit.setEnabled(not readOnly)
        self.__versionEdit.setEnabled(not readOnly)

    def setValue(self, value):
        """
        Given an asset ID. Set the Show, Shot, Asset and version in the UI.
        """

        assetPlugin = AssetAPI.GetDefaultAssetPlugin()
        if not assetPlugin.isAssetId(value):
            return

        assetFields = assetPlugin.getAssetFields(value, True)
        self.__showEdit.setText(assetFields["show"])
        self.__shotEdit.setText(assetFields["shot"])
        self.__assetEdit.setText(assetFields[AssetAPI.kAssetFieldName])
        self.__versionEdit.setText(assetFields[AssetAPI.kAssetFieldVersion])

        if assetFields.get("protocol", "") == "sandbox":
            self.__protocolLabel.setText("sandbox://")
        else:
            self.__protocolLabel.setText("mock://")

    def getValue(self):
        """Get the asset ID from this browser window"""

        show = str(self.__showEdit.text())
        shot = str(self.__shotEdit.text())
        asset = str(self.__assetEdit.text())
        version = str(self.__versionEdit.text())

        protocol = "asset"
        if self.__protocolLabel.text() == "sandbox://":
            protocol = "sandbox"

        assetFields = {"protocol" : protocol,
                       "show" : show,
                       "shot" : shot,
                       AssetAPI.kAssetFieldName : asset,
                       AssetAPI.kAssetFieldVersion : version}
        assetPlugin = AssetAPI.GetDefaultAssetPlugin()

        assetId = assetPlugin.buildAssetId(assetFields)
        return assetId

#///////////////////////////////////////////////////////////////////////////////

class PyMockAssetRenderWidget(AssetWidgetDelegatePlugins.BaseAssetRenderWidget):

    def buildWidgets(self, hints):
        """
        This will construct the UI for the render widget for the asset
        management system.
        """
        label = QtGui.QLabel(" (PyMock)", self)
        label.setProperty('smallFont', True)
        label.setProperty('boldFont', True)
        p = label.palette()
        p.setColor(QtGui.QPalette.WindowText, QtGui.QColor(255, 200, 0))
        p.setColor(QtGui.QPalette.Text, QtGui.QColor(255, 200, 0))
        label.setPalette(p)
        self.layout().addWidget(label)

        assetIdWidget = QtGui.QWidget(self)
        assetIdHBox = QtGui.QHBoxLayout(assetIdWidget)
        assetIdLabel = QtGui.QLabel("Output Asset:", assetIdWidget)
        assetIdLabel.setEnabled(False)
        assetIdHBox.addWidget(assetIdLabel, 0)
        self.__assetIdLabel = QtGui.QLineEdit("", assetIdWidget)
        self.__assetIdLabel.setReadOnly(True)
        assetIdHBox.addWidget(self.__assetIdLabel, 0)
        self.layout().addWidget(assetIdWidget)

        self.updateWidgets()

    def updateWidgets(self):
        """Update the UI to reflect internals"""
        assetId = self.getOutputInfo()["outputLocation"]
        if not assetId:
            assetId = "No output defined..."

        self.__assetIdLabel.setText(assetId)

#///////////////////////////////////////////////////////////////////////////////

class MockAssetListsWidget(QtGui.QFrame):
    """
    Asset browser using lists. This is used for asset selection when loading
    assets.
    """

    def __init__(self):
        QtGui.QFrame.__init__(self)

        QtGui.QHBoxLayout(self)

        self.__tabWidget = QtGui.QTabWidget(self)
        self.layout().addWidget(self.__tabWidget)

        assetTab = QtGui.QFrame(self)
        QtGui.QHBoxLayout(assetTab)
        self.__tabWidget.addTab(assetTab, "Asset")

        self.__showList = self.__buildListWidget("Show", assetTab.layout())
        self.__shotList = self.__buildListWidget("Shot", assetTab.layout())
        self.__nameList = self.__buildListWidget("Asset", assetTab.layout())
        self.__versionList = self.__buildListWidget("Version", assetTab.layout())

        self.connect(self.__showList, QtCore.SIGNAL("itemSelectionChanged()"),
                     self.__updateShow)
        self.connect(self.__shotList, QtCore.SIGNAL("itemSelectionChanged()"),
                     self.__updateShot)
        self.connect(self.__nameList, QtCore.SIGNAL("itemSelectionChanged()"),
                     self.__updateAsset)
        self.connect(self.__versionList, QtCore.SIGNAL("itemSelectionChanged()"),
                     self.__updateVersion)

        sandboxTab = QtGui.QFrame(self)
        QtGui.QHBoxLayout(sandboxTab)
        self.__tabWidget.addTab(sandboxTab, "Sandbox")

        self.__sandboxList = self.__buildListWidget("Sandbox", sandboxTab.layout())

        self.__widgetsDict = {
            "show": self.__showList,
            "shot": self.__shotList,
            "name": self.__nameList,
            "version": self.__versionList,
            "sandbox": self.__sandboxList,
        }

        self.__showList.clear()
        self.__showList.addItems(_getShows())
        self.__sandboxList.addItems(_getSandboxFiles())

    def __buildListWidget(self, name, parentLayout):

        layout = QtGui.QVBoxLayout()

        layout.addWidget(QtGui.QLabel(name))
        listWidget = QtGui.QListWidget()
        layout.addWidget(listWidget)

        parentLayout.addLayout(layout)

        return listWidget

    def __updateShow(self):
        show = self.__getListSelectionText("show")
        self.__shotList.clear()
        self.__shotList.addItems(_getShots(show))

    def __updateShot(self):
        show = self.__getListSelectionText("show")
        shot = self.__getListSelectionText("shot")
        self.__nameList.clear()
        self.__nameList.addItems(_getAssets(show, shot))

    def __updateAsset(self):
        show = self.__getListSelectionText("show")
        shot = self.__getListSelectionText("shot")
        name = self.__getListSelectionText("name")
        self.__versionList.clear()
        self.__versionList.addItems(_getVersions(show, shot, name))

    def __updateVersion(self):
        pass

    def __getListSelectionText(self, name):
        currSel = ""
        w = self.__widgetsDict.get(name, None)
        if w:
            sel = w.selectedItems()
            currSel = sel[0].text() if sel else ""

        return str(currSel)

    def __selectItem(self, listName, itemName):
        w = self.__widgetsDict.get(listName, None)
        if w:
            items = w.findItems(itemName, QtCore.Qt.MatchExactly)

            if items and items[0]:
                w.setCurrentItem(items[0], QtGui.QItemSelectionModel.Select)

    def setAssetId(self, assetId):
        """
        Given an asset id decomposes in to a show, shot, name and
        version fields and updates the UI to reflect that
        """
        assetPlugin = AssetAPI.GetDefaultAssetPlugin()
        if not assetPlugin.isAssetId(assetId):
            return

        assetFields = assetPlugin.getAssetFields(assetId, True)
        for k in ("show", "shot", "name",  "version"):
            self.__selectItem(k, assetFields[k])

        currIndex = 0
        if assetFields.get("protocol", "") == "sandbox":
            currIndex = 1
        self.__tabWidget.setCurrentIndex(currIndex)

    def getAssetFields(self):
        fields = {}
        if self.__tabWidget.currentIndex() == 0: # Asset
            fields["protocol"] = "asset"
            for k in ("show", "shot", "name", "version"):
                fields[k] = self.__getListSelectionText(k)
        else: # Sandbox
            selectedItems = self.__sandboxList.selectedItems()
            if not selectedItems:
                return fields
            fields["protocol"] = "sandbox"
            assetId = str(selectedItems[0].text())
            parts = assetId.split(".")
            for i, k in enumerate(("show", "shot", "name", "version")):
                fields[k] = parts[i]

        return fields

#///////////////////////////////////////////////////////////////////////////////

class MockAssetCombosWidget(QtGui.QFrame):
    """
    Asset browser using combo boxes. This is used for asset selection when
    saving.
    """

    def __init__(self):
        """Builds the initial combose box UI"""

        QtGui.QFrame.__init__(self)

        QtGui.QVBoxLayout(self)

        assetFrame = QtGui.QFrame(self)
        QtGui.QHBoxLayout(assetFrame)
        self.layout().addWidget(assetFrame)

        assetFrame.layout().addWidget(QtGui.QLabel("show:"))
        self.__showCombobox = QtGui.QComboBox()
        assetFrame.layout().addWidget(self.__showCombobox)

        assetFrame.layout().addWidget(QtGui.QLabel("shot:"))
        self.__shotCombobox = QtGui.QComboBox()
        assetFrame.layout().addWidget(self.__shotCombobox)

        assetFrame.layout().addWidget(QtGui.QLabel("asset:"))
        self.__assetCombobox = QtGui.QComboBox()
        assetFrame.layout().addWidget(self.__assetCombobox)

        assetFrame.layout().addWidget(QtGui.QLabel("version:"))
        self.__versionCombobox = QtGui.QComboBox()
        assetFrame.layout().addWidget(self.__versionCombobox)

        self.connect(self.__showCombobox, QtCore.SIGNAL("currentIndexChanged(QString)"),
                     self.__updateShow)
        self.connect(self.__shotCombobox, QtCore.SIGNAL("currentIndexChanged(QString)"),
                     self.__updateShot)
        self.connect(self.__assetCombobox, QtCore.SIGNAL("currentIndexChanged(QString)"),
                     self.__updateAsset)
        self.connect(self.__versionCombobox, QtCore.SIGNAL("currentIndexChanged(QString)"),
                     self.__updateVersion)

        # Modify the integrated QListViews to lay out the items every time the
        # view is resized, so that changes in the application font preferences
        # are correctly reflected
        for combobox in (self.__showCombobox, self.__shotCombobox,
                         self.__assetCombobox, self.__versionCombobox):
            combobox.view().setResizeMode(QtGui.QListView.Adjust)

        self.__sandboxCheckBox = QtGui.QCheckBox('Save to Sandbox', self)
        self.layout().addWidget(self.__sandboxCheckBox)

        self.__showCombobox.addItems(_getShows())

    def __updateShow(self):
        """Updates the value in the 'show' combo box UI"""

        show = str(self.__showCombobox.currentText())
        self.__shotCombobox.clear()
        self.__shotCombobox.addItems(_getShots(show))

    def __updateShot(self):
        """Updates the value in the 'shot' combo box UI"""
        show = str(self.__showCombobox.currentText())
        shot = str(self.__shotCombobox.currentText())
        self.__assetCombobox.clear()
        self.__assetCombobox.addItems(_getAssets(show, shot))

    def __updateAsset(self):
        """Updates the value in the 'asset' combo box UI"""
        show = str(self.__showCombobox.currentText())
        shot = str(self.__shotCombobox.currentText())
        asset = str(self.__assetCombobox.currentText())
        self.__versionCombobox.clear()
        self.__versionCombobox.addItems(_getVersions(show, shot, asset))

    def __updateVersion(self):
        """Updates the value in the 'versions' combo box UI"""
        pass

    def setEditable(self, editable):
        """Propagate editability down to child widgets"""
        self.__showCombobox.setEditable(editable)
        self.__shotCombobox.setEditable(editable)
        self.__assetCombobox.setEditable(editable)
        self.__versionCombobox.setEditable(editable)

    def setAssetId(self, assetId):
        """Given an asset ID, decompose in to a show, shot, asset name
        and asset version and update the UI to reflect that.
        """
        assetPlugin = AssetAPI.GetDefaultAssetPlugin()
        if not assetPlugin.isAssetId(assetId):
            return

        assetFields = assetPlugin.getAssetFields(assetId, True)
        self.__showCombobox.setEditText(assetFields["show"])
        self.__shotCombobox.setEditText(assetFields["shot"])
        self.__assetCombobox.setEditText(assetFields[AssetAPI.kAssetFieldName])
        self.__versionCombobox.setEditText(assetFields[AssetAPI.kAssetFieldVersion])

        if assetId.startswith("sandbox://"):
            self.__sandboxCheckBox.setCheckState(QtCore.Qt.Checked)

    def getAssetFields(self):
        """Get hold of the individual asset fields from the UI"""
        show = str(self.__showCombobox.currentText())
        shot = str(self.__shotCombobox.currentText())
        asset = str(self.__assetCombobox.currentText())
        version = str(self.__versionCombobox.currentText())

        protocol = "asset"
        if self.__sandboxCheckBox.checkState() == QtCore.Qt.Checked:
            protocol = "sandbox"

        return {"protocol" : protocol,
                "show" : show,
                "shot" : shot,
                AssetAPI.kAssetFieldName : asset,
                AssetAPI.kAssetFieldVersion : version}

#///////////////////////////////////////////////////////////////////////////////

class PyMockBrowser(QtGui.QFrame):
    """
    The mock asset browser class. This is added to the browser as a tab in the
    configureAssetBrowser function of the PyMultiMockAssetWidgetDelegate.

    The mock browser uses two different UIs depending on the save mode. For
    saving, the asset ID is specified using combo boxes. For loading assets,
    assets are chosen from a list view (see MockAssetCombosWidget and
    MockAssetListsWidget respectively).
    """

    def __init__( self , *args , **kargs ) :
        QtGui.QFrame.__init__( self , *args )

        QtGui.QVBoxLayout(self)

        self.__assetIdLayout = QtGui.QHBoxLayout()
        self.__widget = None
        self.__saveMode = False
        self.__context = ""
        self.__requestedLocation = ""

    def showEvent(self, event):
        if not self.__widget:
            if self.__saveMode:
                self.__widget = MockAssetCombosWidget()
                self.__widget.setEditable(True)
            else:
                self.__widget = MockAssetListsWidget()
            self.layout().addWidget(self.__widget)

        if self.__requestedLocation and self.__widget:
            self.__widget.setAssetId(self.__requestedLocation)

    def setContext(self, context):
        self.__context = context

    def getExtraOptions(self):
        return {"context": self.__context}

    def setSaveMode(self, saveMode):
        self.__saveMode = saveMode

    def setLocation(self, assetId):
        self.__requestedLocation = assetId

    def selectionValid(self):
        return True

    def getResult(self):
        assetFields = self.__widget.getAssetFields()
        assetPlugin = AssetAPI.GetDefaultAssetPlugin()

        return assetPlugin.buildAssetId(assetFields)

#///////////////////////////////////////////////////////////////////////////////

class PyMultiMockAssetWidgetDelegate(AssetWidgetDelegatePlugins.BaseAssetWidgetDelegate):
    """
    The widget delegate class that implements BaseAssetWidgetDelegate.

    This class is registered to be associated with PyMultiMockAsset and represents
    the entry point for configuring the render widget, control widget and
    asset browser. 
    """

    def createAssetControlWidget(self, parent):
        """The hook in to katana that creates the asset control widget"""
        w = PyMockAssetControlWidget(parent, self.getWidgetHints())
        parent.layout().addWidget(w)
        return w

    def createAssetRenderWidget(self, parent, outputInfo):
        """The hook in to katana that creates the asset render control widget"""

        w = PyMockAssetRenderWidget(parent, self.getWidgetHints(), outputInfo)
        parent.layout().addWidget(w)
        return w

    def configureAssetBrowser(self, browser):
        """
        Configure the asset browser.

        For example, to disable an asset browser for shader lookups on the
        Material node, check for a specific context and return early:

         if context == AssetAPI.kAssetContextShader:
             _logDebug("Do not show asset browser for shaders.")
             return
        """
        AssetWidgetDelegatePlugins.BaseAssetWidgetDelegate.\
                                       configureAssetBrowser(self, browser)
        valuePolicy = self.getValuePolicy()
        hints = valuePolicy.getWidgetHints()
        context = hints.get("context")

        _logDebug("configureAssetBrowser hints: %s" % hints)

        index = browser.addBrowserTab(PyMockBrowser, "PyMock")
        inputPath = str(valuePolicy.getValue())
        if inputPath.startswith("mock://") or not inputPath:
            browser.setCurrentIndex(index)
            browser.getBrowser(index).setLocation(inputPath)
            browser.getBrowser(index).setContext(context)

    def shouldAddFileTabToAssetBrowser(self):
        """Yes we want to keep the file tab in the asset browser"""
        return True

    def shouldAddStandardMenuItems(self):
        """Yes we want asset ids to have a standard dropdown menu"""
        return True

    def getQuickLinkPathsForContext(self, context):
        return AssetWidgetDelegatePlugins.BaseAssetWidgetDelegate.\
                   getQuickLinkPathsForContext(self, context)


#///////////////////////////////////////////////////////////////////////////////

# Register the widget delegate to be associated with PyMultiMockAsset
PluginRegistry = [
    ("AssetWidgetDelegate", 1, "PyMultiMockAsset",
     PyMultiMockAssetWidgetDelegate),
]
