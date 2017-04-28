def getItemPath(item):
    if item and hasattr(item, 'getItemData'):
        data = item.getItemData()
        if isinstance(data, tuple):
            return data[0]

gafferNode = NodegraphAPI.GetNode("Gaffer")  
gafferPaths = list()

#gafferTreeWidgets = [x for x in QtGui.qApp.allWidgets() if isinstance(x, QtGui.QTreeWidget) and x.__class__.__name__ == 'SortableTreeWidget']
gafferTreeWidgets = [x for x in QtGui.qApp.allWidgets() if isinstance(x, QtGui.QTreeWidget)]

for tree in gafferTreeWidgets:
    if tree.__class__.__name__ == "SortableTreeWidget" or tree.__class__.__name__ == "TreeWidget":
        print tree.__class__.__name__
        print "--",tree.selectedItems()
        root = tree.invisibleRootItem()
        child_count = root.childCount()
        print child_count
        print "tease this:",root.child(0)

ScenegraphManager.getActiveScenegraph().setLocationSelected("/root/world/lgt/gafferthere/fuck")