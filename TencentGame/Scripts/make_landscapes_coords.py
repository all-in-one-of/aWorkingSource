




def createCoordNodes(filename=None, offset=[0, 0]):
    import hou
    heightfield_file_node = hou.node("/obj/Swamp_VIS/subnet1/").createNode("heightfield_file")
    heightfield_xform_node = hou.node("/obj/Swamp_VIS/subnet1/").createNode("heightfield_xform")
    volume_wrangle_node = hou.node("/obj/Swamp_VIS/subnet1/").createNode("volumewrangle")

    heightfield_file_node.setPosition([0, 0])
    heightfield_xform_node.setPosition([0, -1])
    heightfield_xform_node.setInput(0, heightfield_file_node)
    
    volume_wrangle_node.setPosition([0, -2])
    volume_wrangle_node.setInput(0, heightfield_xform_node)
    volume_wrangle_node.parm("snippet").set("@mask=1.0;")
    heightfield_xform_node.parm("tx").set(offset[0])
    heightfield_xform_node.parm("tz").set(offset[1])
    heightfield_file_node.parm("filename").set(filename)
    heightfield_file_node.parm("size").set(505)
    heightfield_file_node.parm("heightscale").set(512)

def generateNetworks():
    main_heightfield = hou.node("/obj/Swamp_VIS/subnet1/").createNode("heightfield_file")


import hou

node = hou.pwd()
