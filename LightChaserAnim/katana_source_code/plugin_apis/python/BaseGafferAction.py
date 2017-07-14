
try:
    from Katana import QtGui
    guiMode = not QtGui.qApp.startingUp() and not QtGui.qApp.type() == 0
except ImportError:
    guiMode = False

if guiMode:
    from Katana import QtCore, QtGui, UI4, Plugins, Utils, PyXmlIO as XmlIO, AssetAPI
    from Katana import NodegraphAPI


    # Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.
    class BaseGafferAddLocationAction(QtGui.QAction):
        def __init__(self, parent, label, parentPath,
                nodeName, preselectMethod):
            QtGui.QAction.__init__(self, label, parent)
            self.__parentPath = parentPath
            self.__nodeName = nodeName
            self.__preselectMethod = preselectMethod
            self.connect(self, QtCore.SIGNAL('triggered(bool)'), self.go)

        def getNode(self):
            return NodegraphAPI.GetNode(self.__nodeName)

        def preselectPath(self, path):
            self.__preselectMethod(path)

        def getParentPath(self):
            newItem = Plugins.GafferAPI.ScriptItems.GetGafferScriptItemClass(self.getPackageName())

            fields = self.__parentPath.split('/')

            while fields:
                item = self.getNode().getScriptItemForPath('/'.join(fields))
                if not item or item.acceptsChild(newItem):
                    break

                del fields[-1]

            return '/'.join(fields)

        def go(self, checked=None):
            pass

        @staticmethod
        def getActionName():
            raise NotImplementedError, "getActionName() must be implemented by children of GafferActions."

        @staticmethod
        def getMenuGroup():
            return None

        def getPackageName(self):
            return None

