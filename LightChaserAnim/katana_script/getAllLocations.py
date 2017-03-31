from Katana import NodegraphAPI,FnGeolib,runtime,Nodes3DAPI

def getAll():
	node = NodegraphAPI.GetViewNode()
	runtime = FnGeolib.GetRegisteredRuntimeInstance()
	txn = runtime.createTransaction()
	client = txn.createClient()
	op = Nodes3DAPI.GetOp(txn, node)
	txn.setClientOp(client, op)
	runtime.commit(txn)

	locationPathsAndData = []
	traversal = FnGeolib.Util.Traversal(client, "/root")
	while traversal.valid():
	    locationPathsAndData.append((traversal.getLocationPath(),traversal.getLocationData()))
	    traversal.next()
	for i in locationPathsAndData:
	    print i

getAll()
	