#!/usr/bin/env python

import os
import MaterialX as mx

# Path to your arnold materialx nodedefs
ARNOLD_NODEDEFS="/mnt/work/software/arnold/arnold-5.1.0.0/materialx/arnold/nodedefs.mtlx"
SCRIPT_ROOT_PATH=os.path.dirname(os.path.realpath(__file__))

# Create a document.
doc = mx.createDocument()
# Include Arnold nodedefs.
mx.prependXInclude(doc, ARNOLD_NODEDEFS)

nodeDef = doc.addNodeDef(name="KTColorMap", type="color3", node="KTColorMap")
nodeDef.addInput("input","color3")
nodeDef.setInputValue("input",mx.Color3(1.0,1.0,1.0))
nodeDef.addOutput("out","color3")
# Create a node graph with a single image node and output.
nodeGraph = doc.addNodeGraph("NodeGraph_red")
image = nodeGraph.addNode( 'image', name='image_shading_node')
image.setInputValue('filename', "/mnt/public/home/xukai/sources/checker.png")
color_map=nodeGraph.addNode( 'KTColorMap', name='KTColorMap_shading_node')
color_map.setConnectedNode("input", image)
color_correct = nodeGraph.addNode( 'color_correct', name='cc_shading_node')
color_correct.setConnectedNode("input", color_map)
color_correct.setInputValue("saturation", 1.0)
output_out = nodeGraph.addOutput('out', "color3")
output_out.setConnectedNode(color_map)

# Create a material.
material = doc.addMaterial("M_red")
shaderRef = material.addShaderRef('redMate', 'standard_surface')
baseColorBindInput = shaderRef.addBindInput('base_color')
# baseColorBindInput.setValue(mx.Color3(1.0, 0.0, 0.0))
# baseColorBindInput.setNodeGraphString("NodeGraph_red")
# baseColorBindInput.setConnectedOutput("out_prue_color")
# baseColorBindInput.setOutputString("out")
specularBindInput = shaderRef.addBindInput('specular')
# specularBindInput.setValue(0.5)
specularBindInput.setNodeGraphString("NodeGraph_red")
# baseColorBindInput.setConnectedOutput("out_prue_color")
specularBindInput.setOutputString("out")

# Create a collection.
collection = doc.addCollection("S_pony")
collectionAdd = collection.addCollectionAdd("GlobalAssign")
collectionAdd.setGeom("*")

# Create a look.
lookA = doc.addLook("default")
materialAssign = lookA.addMaterialAssign("default")
materialAssign.setCollection(collection)
materialAssign.setMaterial("M_red")

mx.writeToXmlFile(doc, os.path.join(SCRIPT_ROOT_PATH, "networks.mtlx"))


arnold_doc = mx.createDocument()
mx.readFromXmlFile(arnold_doc, ARNOLD_NODEDEFS)
# Traverse the document tree in depth-first order.
node_defs = arnold_doc.getNodeDefs()
print arnold_doc.getNodeDef("image").getInput("filename").getType()
# for elem in doc.traverseTree():
#     # Display the filename of each image node.
#     print dir(elem)
#     break 
#     if elem.isA(mx.Node, 'image'):
#         print elem
#         param = elem.getParameter('filename')
#         print param.getType()
#         if param:
#             filename = param.getValueString()
#             print 'Image node', elem.getName(), 'references', filename