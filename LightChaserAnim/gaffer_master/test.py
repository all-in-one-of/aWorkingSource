
def getItemPath(item):
    if item and hasattr(item, 'getItemData'):
        data = item.getItemData()
        if isinstance(data, tuple):
            return data[0]

gafferNode = NodegraphAPI.GetNode("Gaffer")  
gafferPaths = list()

gafferTreeWidgets = [x for x in QtGui.qApp.allWidgets() if isinstance(x, QtGui.QTreeWidget) and x.__class__.__name__ == 'SortableTreeWidget']
rootPath = gafferNode.getRootPath()
for widget in gafferTreeWidgets:
    print widget.selectedItems()
    if rootPath == getItemPath(widget.topLevelItem(0)):
        treeWidget = widget
if not treeWidget:
        treeWidget =  gafferPaths

for item in treeWidget.selectedItems():
    itemPath = getItemPath(item)
    if itemPath:
        gafferPaths.append(itemPath)
    
print gafferPaths
