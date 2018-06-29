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

# Create a material.
material = doc.addMaterial("M_red")
shaderRef = material.addShaderRef('redMate', 'standard_surface')
baseColorBindInput = shaderRef.addBindInput('base_color')
baseColorBindInput.setValue(mx.Color3(1.0, 0.0, 0.0))
specularBindInput = shaderRef.addBindInput('specular')
specularBindInput.setValue(0.5)

# Create a material.
material = doc.addMaterial("M_red")
shaderRef = material.addShaderRef('redMate', 'standard_surface')
baseColorBindInput = shaderRef.addBindInput('base_color')
baseColorBindInput.setValue(mx.Color3(1.0, 0.0, 0.0))
specularBindInput = shaderRef.addBindInput('specular')
specularBindInput.setValue(0.5)


material = doc.addMaterial("M_blue")
shaderRef = material.addShaderRef('blueMate', 'standard_surface')
baseColorBindInput = shaderRef.addBindInput('base_color')
baseColorBindInput.setValue(mx.Color3(0.0, 0.0, 1.0))
specularBindInput = shaderRef.addBindInput('specular')
specularBindInput.setValue(0.5)

# Create a collection.
collection = doc.addCollection("S_pony")
collectionAdd = collection.addCollectionAdd("GlobalAssign")
collectionAdd.setGeom("*")

# Create a lookA.
lookA = doc.addLook("lookA")
materialAssign = lookA.addMaterialAssign("redMA")
materialAssign.setCollection(collection)
materialAssign.setMaterial("M_red")

# Create a lookB.
lookB = doc.addLook("lookB")
materialAssign = lookB.addMaterialAssign("blueMA")
materialAssign.setCollection(collection)
materialAssign.setMaterial("M_blue")

# Create a lookC.
lookC = doc.addLook("lookC")
visibilityAssign = lookC.addVisibility("ponyVisibility")
visibilityAssign.setCollection(collection)
visibilityAssign.setVisible(False)

mx.writeToXmlFile(doc, os.path.join(SCRIPT_ROOT_PATH, "looks.mtlx"))