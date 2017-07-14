#from Katana import NodegraphAPI,FnGeolib,runtime,Nodes3DAPI
import json
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
	result = {}
	j = 0
	for i in locationPathsAndData:
	    print i
	    result[i[0]] = j
	    j+=1
	return result
def writeJson(inFile,outPath):
	file = open(outPath, "w")
	json.dump(inFile, file, indent=3)
	file.close()

inFile = getAll()
writeJson(inFile,"/home/xukai/Documents/TestProgramFactory/ktn_op_shapeID/out/output.json")
	