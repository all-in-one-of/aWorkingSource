
frame = NodegraphAPI.NodegraphGlobals.GetCurrentTime()
root = Nodes3DAPI.GetGeometryProducer(NodegraphAPI.GetViewNode(), frame)
loc = root.getProducerByPath(obj_path)
attr = loc.getAttribute(attr_name)




