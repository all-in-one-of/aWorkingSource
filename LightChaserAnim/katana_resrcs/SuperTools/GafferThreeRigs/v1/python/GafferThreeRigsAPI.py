import logging
import math
from Katana import NodegraphAPI, Nodes3DAPI, AssetAPI, Plugins
from Katana import CacheManager

log = logging.getLogger('GafferThreeRigs')

cRigLayers = [
    "chr_layer",
    "env_layer",
    "vol_layer",
]

cArnoldGafferPackages = {
    "SpotLight":Plugins.GafferThreeAPI.PackageClasses.ArnoldSpotLightGafferPackage,
    "QuadLight":Plugins.GafferThreeAPI.PackageClasses.ArnoldQuadLightGafferPackage,
    "PointLight":Plugins.GafferThreeAPI.PackageClasses.ArnoldPointLightGafferPackage,
    "DistantLight":Plugins.GafferThreeAPI.PackageClasses.ArnoldDistantLightGafferPackage,
    "FilterLight":Plugins.GafferThreeAPI.PackageClasses.ArnoldFilterLightGafferPackage,
    "GoboSpotLight":Plugins.GafferThreeAPI.PackageClasses.ArnoldGoboSpotLightGafferPackage,
    "SkyDomeLight":Plugins.GafferThreeAPI.PackageClasses.ArnoldSkyDomeLightGafferPackage,
}

cLightColors = {
    "key" : (0.9, 0.5, 0.1),
    "fill" : (0.5, 0.5, 0.9),
    "rim" : (0.5, 0.6, 0.5),
    "eyes" : (0.5, 0.1, 0.9),
    "env" : (0.9, 0.9, 0.1),
}

cLightObject = {
    "key" : 
        {
        "translate":(-10.0, 5.0, 10.0),
        "rotate":(-30.0, -45.0, 0.0),
        "scale":(1.0, 1.0, 1.0),
        "color":(0.9, 0.5, 0.1),
        },
    "fill" : 
        {
        "translate":(10.0, 0.0, 10.0),
        "rotate":(0.0, 45.0, 0.0),
        "scale":(1.0, 1.0, 1.0),
        "color":(0.5, 0.5, 0.9),
        },
    "back" : 
        {
        "translate":(8.0, 0.0, -8.0),
        "rotate":(180.0, 45.0, 180.0),
        "scale":(1.0, 1.0, 1.0),
        "color":(0.5, 0.7, 0.5),
        },
    "rim" : 
        {
        "translate":(-8.0, 0.0, -8.0),
        "rotate":(180.0, -45.0, 180.0),
        "scale":(1.0, 1.0, 1.0),
        "color":(0.5, 0.6, 0.5),
        },
    "eyes" : 
        {
        "translate":(-5.0, 5.0, 5.0),
        "rotate":(0.0, 0.0, 0.0),
        "scale":(1.0, 1.0, 1.0),
        "color":(0.5, 0.1, 0.9),
        },
    "env" : 
        {
        "translate":(0.0, 0.0, 0.0),
        "rotate":(-30.0, -45.0, 0.0),
        "scale":(100.0, 100.0, 100.0),
        "color":(0.9, 0.9, 0.1),
        },
    "skydome" : 
        {
        "translate":(0.0, 0.0, 0.0),
        "rotate":(0.0, 0.0, 0.0),
        "scale":(10000.0, 10000, 10000),
        "color":(0.9, 0.9, 0.1),
        },
}

cDefaultRootAssets = "/root/world/geo/assets"

def setMessage(message):
    from Katana import QtCore, QtGui
    layoutsMenus = [x for x in QtGui.qApp.topLevelWidgets() if type(x).__name__ == 'LayoutsMenu']
    KatanaWindow = layoutsMenus[0].parent().parent()

    widget  = QtGui.QDialog(parent=KatanaWindow)
    title_layout = QtGui.QVBoxLayout()
    button_layout = QtGui.QHBoxLayout()
    title = QtGui.QLabel()
    button = QtGui.QPushButton()
    font = QtGui.QFont()
    font.setPixelSize(16)

    title.setText(message)
    title.setFont(font)
    title.setAlignment(QtCore.Qt.AlignHCenter)
    button.setText("Confirm!")
    button.clicked.connect(widget.close)
    button.setFixedHeight(30)

    title_layout.addStretch()
    title_layout.addWidget(title)
    title_layout.addStretch()
    button_layout.addStretch()
    button_layout.addWidget(button)
    button_layout.addStretch()

    mainLayout = QtGui.QVBoxLayout()
    mainLayout.addLayout(title_layout)
    mainLayout.addLayout(button_layout)

    widget.setWindowTitle("GafferThreeRigs.Warning")
    widget.setLayout(mainLayout)
    widget.setFixedHeight(130)
    widget.show()
def doesExist(node, location):
    producer = Nodes3DAPI.GetRenderProducer(node, 0, False, 0)
    geometryProc = producer.getProducerByPath(location)
    if geometryProc is not None:
        return True
    else:
        return False

def getRootPackage(node):
    group_nodes = NodegraphAPI.GetAllNodesByType("Group", includeDeleted=False)
    root_package_node = None
    for group_node in group_nodes:
        node_mark = group_node.getParameter('__gaffer.packageType')
        if node_mark and node_mark.getValue(0) == "RootPackage":
            parentNode = group_node.getParent()
            if parentNode is node:
                root_package_node = group_node;break
    return root_package_node

def connect(upStreamNode, downStreamNode):
    # connected to Merge node
    input_port = None
    unconnected_inputs = []
    input_ports=downStreamNode.getInputPorts()
    for port in input_ports:
        if not port.getConnectedPorts():
            unconnected_inputs.append(port)
    if unconnected_inputs:
        input_port = unconnected_inputs[0]
    else:
        input_port = downStreamNode.addInputPort("i%i"%len(downStreamNode.getInputPorts()))
    input_port.connect(upStreamNode.getOutputPort('out'))

def getGafferThreeNode(node):
    in_port = node.getInputPort("in")
    out_port = in_port.getConnectedPort(0)
    if out_port and out_port.getNode().getType()=="GafferThree":
        return out_port.getNode()

def getGafferThreeLocation(node):
    gaffer_three_node = getGafferThreeNode(node)
    
    if not gaffer_three_node: return

    root_package_node = getRootPackage(gaffer_three_node)
    return root_package_node.getParameter('__gaffer.location').getValue(0)

def getRigTranslate(node, location):
    translate = [0.0, 0.0, 0.0]
    producer = Nodes3DAPI.GetRenderProducer(node, 0, False, 0)
    geometryproducer = producer.getProducerByPath(location)
    ad=geometryproducer.getDelimitedLocalAttribute('bound')
    if ad :
        data = ad.getData()
        translate = [(data[0]+data[1])/2, (data[2]+data[3])/2, (data[4]+data[5])/2]
    return translate

def getRigScale(node, location):
    scale = [1.0, 1.0, 1.0]
    producer = Nodes3DAPI.GetRenderProducer(node, 0, False, 0)
    geometryproducer = producer.getProducerByPath(location)
    ad = geometryproducer.getAttribute('bound')
    if ad :
        data = ad.getData()
        scale = math.sqrt( math.pow((data[5]-data[4])/2, 2) + math.pow((data[1]-data[0])/2, 2) )
    return scale

def getCommonRigTranslate(node, locationNodes):
    rig_position_list = []
    for location in locationNodes:
        translate = [0.0, 0.0, 0.0]
        location_value = location.getValue(0)
        producer = Nodes3DAPI.GetRenderProducer(node, 0, False, 0)
        geometryproducer = producer.getProducerByPath(location_value)
        ad = geometryproducer.getDelimitedLocalAttribute('bound')
        if ad :
            data = ad.getData()
            translate = [(data[0]+data[1])/2, (data[2]+data[3])/2, (data[4]+data[5])/2]
        rig_position_list.append(translate)
    center_x = center_y = center_z = 0
    for position in rig_position_list:
        center_x += position[0]
        center_y += position[1]
        center_z += position[2]
    length = len(rig_position_list)
    return (center_x/length, center_y/length, center_z/length)


def createLight(node, name, material, 
                usage = None, linkTarget = None, 
                annotation=None, lightAov=None, 
                parent=None):
    if not parent: parent = getRootPackage(node)

    merge_node_para = parent.getParameter("node_merge")
    merge_node = NodegraphAPI.GetNode(merge_node_para.getValue(0))
    
    location_path = parent.getParameter("__gaffer.location").getValue(0) + "/" + "light"
    package_class = cArnoldGafferPackages[material].create(node, location_path)
    package_class.setName(name, queueEvent=False, selectLocation=False, makeUnique=True)
    package_node = package_class._Package__packageNode
    
    package_node.setParent(parent)    
    
    connect(package_node, merge_node)

    #########################
    # set light preview color
    if usage is not None:
        node_name = package_node.getParameter('node_create').getValue(0)
        light_create_node = NodegraphAPI.GetNode(node_name)
        preview_color = light_create_node.getParameter('previewColor')
        if preview_color:
            para_list = preview_color.getChildren()
            for para in para_list:
                i = para_list.index(para)
                para.setValue(cLightObject[usage]["color"][i],0)
    
    ###########################
    # set light annotation text
    if annotation is not None:
        node_name = package_node.getParameter('node_viewerObjectSettings').getValue(0)
        viewerObjectSettings_node = NodegraphAPI.GetNode(node_name)
        viewerObjectSettings_node.getParameter('args.viewer.default.annotation.text.enable').setValue(1,0)
        viewerObjectSettings_node.getParameter('args.viewer.default.annotation.text.value').setValue(annotation,0)

    #####################
    # set light transform
    if usage is not None:
        node_name = package_node.getParameter('node_create').getValue(0)
        light_create_node = NodegraphAPI.GetNode(node_name)
        translate_list = light_create_node.getParameter('transform.translate').getChildren()
        rotate_list = light_create_node.getParameter('transform.rotate').getChildren()
        scale_list = light_create_node.getParameter('transform.scale').getChildren()
        for para in translate_list:
            i = translate_list.index(para)
            para.setValue(cLightObject[usage]["translate"][i],0)
        for para in rotate_list:
            i = rotate_list.index(para)
            para.setValue(cLightObject[usage]["rotate"][i],0)        
        for para in scale_list:
            i = scale_list.index(para)
            para.setValue(cLightObject[usage]["scale"][i],0)

    ####################
    # set light aov name
    if lightAov is not None:
        node_name = package_node.getParameter('node_material').getValue(0)
        material_node = NodegraphAPI.GetNode(node_name)
        material_node.checkDynamicParameters()
        material_node.getParameter('shaders.arnoldLightParams.aov.enable').setValue(1,0)
        material_node.getParameter('shaders.arnoldLightParams.aov.value').setValue(str(lightAov),0)

    #################
    # set light links
    light_link_node_name = package_node.getParameter('node_lightLink_illumination').getValue(0)
    light_create_node_name = package_node.getParameter('node_create').getValue(0)
    light_link_node = NodegraphAPI.GetNode(light_link_node_name)
    light_create_node = NodegraphAPI.GetNode(light_create_node_name)
    if linkTarget is not None:
        light_link_node.getParameter("on").setValue(linkTarget,0)
    initial_state = light_create_node.getParameter('initialState')
    if initial_state:
        initial_state.setValue(0,0)


    return package_node

def getRootRig(node, name):
    root_package_node = getRootPackage(node)
    for child_node in root_package_node.getChildren():
        location = child_node.getParameter('__gaffer.location')
        if location is not None and location.getValue(0).split("/")[-1] == name:
            return child_node

def createRig(node, name, translate=None, parent=None):
    
    if not parent: parent = getRootPackage(node)

    merge_node_para = parent.getParameter("node_merge")
    merge_node = NodegraphAPI.GetNode(merge_node_para.getValue(0))

    location_path = parent.getParameter("__gaffer.location").getValue(0) + "/" + "rig"
    package_class = Plugins.GafferThreeAPI.PackageClasses.RigPackage.\
        create(node, location_path)
    package_class.setName(name, queueEvent=False, selectLocation=False, makeUnique=True)
    package_node = package_class._Package__packageNode

    package_node.setParent(parent)

    connect(package_node, merge_node)

    ###################
    # set rig transform
    if translate is not None:
        node_name = package_node.getParameter('node_create').getValue(0)
        primitive_node = NodegraphAPI.GetNode(node_name)
        para_list = primitive_node.getParameter('transform.translate').getChildren()
        for para in para_list:
            index = para_list.index(para)
            para.setValue(translate[index],0)
    return package_node

def setCharacterRig(node, nodeRigs, chrRigNode):
    def setEyesRig(node, parent, annotation=None):
        eyes_rig_node = createRig(node, "eyes_rig", parent=parent)
        createLight(node, "eye_L", "PointLight", usage="eyes", 
            lightAov="chr_eyes_light", annotation=annotation, parent=eyes_rig_node)
        createLight(node, "eye_R", "PointLight", usage="eyes", 
            lightAov="chr_eyes_light", annotation=annotation, parent=eyes_rig_node)
    def setDefaultRig(node, nodeRigs, chrRigNode):
        locations = nodeRigs.getParameter('rootLocations').getChildren()
        for location in locations:
            location_value = location.getValue(0)
            chr_name = location_value.split("/")[-1]
            translate = getRigTranslate(node, location_value)
            rig_location = chrRigNode.getParameter("__gaffer.location").getValue(0)\
                 + "/" + chr_name
            if doesExist(node, rig_location):
                continue
            chr_rig_node = createRig(node, chr_name, translate, parent=chrRigNode)
            key_material = nodeRigs.getParameter('LightTypes.KeyLight').getValue(0)
            fill_material = nodeRigs.getParameter('LightTypes.FillLight').getValue(0)
            back_material = nodeRigs.getParameter('LightTypes.BackLight').getValue(0)
            rim_material = nodeRigs.getParameter('LightTypes.RimLight').getValue(0)
            links_target = cDefaultRootAssets + "/chr/%s"%chr_name
            createLight(node, "key_light", key_material, 
                linkTarget= links_target, annotation=chr_name, usage="key", 
                lightAov="chr_key_light", parent=chr_rig_node)
            createLight(node, "fill_light", fill_material, 
                linkTarget= links_target, annotation=chr_name, usage="fill", 
                lightAov="chr_fill_light", parent=chr_rig_node)
            createLight(node, "back_light", back_material,
                linkTarget= links_target, annotation=chr_name, usage="back", 
                lightAov="chr_back_light", parent=chr_rig_node)
            createLight(node, "rim_light", rim_material,
                linkTarget= links_target, annotation=chr_name, usage="rim", 
                lightAov="chr_rim_light", parent=chr_rig_node)
            setEyesRig(node, chr_rig_node, annotation=chr_name)
    def setCommonKey(node, nodeRigs, chrRigNode):
        locations = nodeRigs.getParameter('rootLocations').getChildren()
        translate = getCommonRigTranslate(node, locations)
        common_rig_location = chrRigNode.getParameter("__gaffer.location").getValue(0)\
             + "/" + "common"
        if not doesExist(node, common_rig_location):        
            common_rig_node = createRig(node, "common", translate, parent=chrRigNode)
            key_material = nodeRigs.getParameter('LightTypes.KeyLight').getValue(0)
            links_target = cDefaultRootAssets + "/chr"
            createLight(node, "key_light", key_material, usage="key",
                linkTarget= links_target, annotation="chr_common", 
                lightAov="chr_key_light", parent=common_rig_node)
        for location in locations:
            location_value = location.getValue(0)
            chr_name = location_value.split("/")[-1]
            translate = getRigTranslate(node, location_value)
            rig_location = chrRigNode.getParameter("__gaffer.location").getValue(0)\
                 + "/" + chr_name
            if doesExist(node, rig_location):
                continue
            chr_rig_node = createRig(node, chr_name, translate, parent=chrRigNode)
            fill_material = nodeRigs.getParameter('LightTypes.FillLight').getValue(0)
            back_material = nodeRigs.getParameter('LightTypes.BackLight').getValue(0)
            rim_material = nodeRigs.getParameter('LightTypes.RimLight').getValue(0)
            links_target = cDefaultRootAssets + "/chr/%s"%chr_name
            createLight(node, "fill_light", fill_material, 
                linkTarget= links_target, annotation=chr_name, usage="fill", 
                lightAov="chr_fill_light", parent=chr_rig_node)
            createLight(node, "back_light", back_material,
                linkTarget= links_target, annotation=chr_name, usage="back", 
                lightAov="chr_back_light", parent=chr_rig_node)
            createLight(node, "rim_light", rim_material,
                linkTarget= links_target, annotation=chr_name, usage="rim", 
                lightAov="chr_rim_light", parent=chr_rig_node)        
            setEyesRig(node, chr_rig_node, annotation=chr_name)

    def setCommonRig(node, nodeRigs, chrRigNode):
        common_rig_location = chrRigNode.getParameter("__gaffer.location").getValue(0)\
             + "/" + "common"
        locations = nodeRigs.getParameter('rootLocations').getChildren()
        if not doesExist(node, common_rig_location):        
            translate = getCommonRigTranslate(node, locations)
            common_rig_node = createRig(node, "common", translate, parent=chrRigNode)
            key_material = nodeRigs.getParameter('LightTypes.KeyLight').getValue(0)
            fill_material = nodeRigs.getParameter('LightTypes.FillLight').getValue(0)
            back_material = nodeRigs.getParameter('LightTypes.BackLight').getValue(0)
            rim_material = nodeRigs.getParameter('LightTypes.RimLight').getValue(0)
            links_target = cDefaultRootAssets + "/chr"
            createLight(node, "key_light", key_material, usage="key", annotation="chr_common", 
                linkTarget= links_target, lightAov="chr_key_light", parent=common_rig_node)
            createLight(node, "fill_light", fill_material, usage="fill", annotation="chr_common",
                linkTarget= links_target, lightAov="chr_fill_light", parent=common_rig_node)
            createLight(node, "back_light", back_material, usage="back", annotation="chr_common",
                linkTarget= links_target, lightAov="chr_back_light", parent=common_rig_node)
            createLight(node, "rim_light", rim_material, usage="rim", annotation="chr_common",
                linkTarget= links_target, lightAov="chr_rim_light", parent=common_rig_node)
        for location in locations:
            location_value = location.getValue(0)
            chr_name = location_value.split("/")[-1]
            translate = getRigTranslate(node, location_value)
            rig_location = chrRigNode.getParameter("__gaffer.location").getValue(0)\
                 + "/" + chr_name
            if doesExist(node, rig_location):
                continue
            chr_rig_node = createRig(node, chr_name, translate, parent=chrRigNode)       
            setEyesRig(node, chr_rig_node, annotation=chr_name)

    use_common_key = nodeRigs.getParameter('Setting.useCommonKeyLight').getValue(0)
    use_common_rig = nodeRigs.getParameter('Setting.useCommonLightRig').getValue(0)
    if use_common_rig:
        setCommonRig(node, nodeRigs, chrRigNode)
    elif use_common_key:
        setCommonKey(node, nodeRigs, chrRigNode)
    else:
        setDefaultRig(node, nodeRigs, chrRigNode)

def setEnvrionmentRig(node, nodeRigs, envRigNode):
    createLight(node, "key_light", "DistantLight", usage="env", annotation="env",
        lightAov="env_key_light", parent=envRigNode)
    createLight(node, "skydome", "SkyDomeLight", usage="skydome",
        lightAov="env_skydome_light", parent=envRigNode)

def importRigsSet(node):
    gaffer_three_node = getGafferThreeNode(node)
    root_locations = node.getParameter('rootLocations').getChildren()
    producer = Nodes3DAPI.GetRenderProducer(node, 0, False, 0)
    if not gaffer_three_node:
        message = "GafferThree node have not be connected to GafferThreeRigs!"
        setMessage(message)
        log.warning(message)
        return

    if not root_locations:
        message = "GafferThreeRigs node has no rootLocation!"
        setMessage(message)
        log.warning(message)
        return
    else:
        if root_locations and root_locations[0].getValue(0) is "":
            message = "GafferThreeRigs has no rootLocation"
            log.warning(message)
            return


    if not producer.getProducerByPath(getGafferThreeLocation(node) + "/" + cRigLayers[0]):
        chr_layer_rig_node = createRig(gaffer_three_node, cRigLayers[0])        
        setCharacterRig(gaffer_three_node, node, chr_layer_rig_node)
    else:
        chr_layer_rig_node = getRootRig(gaffer_three_node, cRigLayers[0])
        setCharacterRig(gaffer_three_node, node, chr_layer_rig_node)


    if not producer.getProducerByPath(getGafferThreeLocation(node) + "/" + cRigLayers[1]):
        env_layer_rig_node = createRig(gaffer_three_node, cRigLayers[1])
        setEnvrionmentRig(gaffer_three_node, node, env_layer_rig_node)
    else:
        message = "Light rig env_layer already created!"
        log.warning(message)

    if not producer.getProducerByPath(getGafferThreeLocation(node) + "/" + cRigLayers[2]):
        vol_layer_rig_node = createRig(gaffer_three_node, cRigLayers[2])
    else:
        message = "Light rig vol_layer already created!"
        log.warning(message)

    message = "Light rigs have been created into {%s} node!"%gaffer_three_node.getName()
    setMessage(message)
    CacheManager.flush(True)

def updateParameters(node):
    def copyParam(param, params_list):
        param_name = param.getName()
        param_enable = param.getChild("enable")
        for p in params_list:
            if p.getName() == param_name and param_enable is not None:
                master_param_enable = p.getChild("enable").getValue(0)
                input_param_enable = param.getChild("enable").getValue(0)
                if master_param_enable and not input_param_enable:
                    children = p.getChild("value").getChildren()
                    if children is None:
                        value = p.getChild("value").getValue(0)
                        param.getChild("enable").setValue(1.0, 0)
                        param.getChild("value").setValue(value, 0)
                    else:
                        for child in children:
                            name = child.getName()
                            value = child.getValue(0)
                            param.getChild("enable").setValue(1.0, 0)
                            param_child = param.getChild("value").getChild(name)
                            if param_child is not None:
                                param_child.setValue(value, 0)

    gaffer_three_node = getGafferThreeNode(node)
    root_locations = node.getParameter('rootLocations').getChildren()
    producer = Nodes3DAPI.GetRenderProducer(node, 0, False, 0)
    if not gaffer_three_node:
        message = "GafferThree node have not be connected to GafferThreeRigs!"
        setMessage(message)
        log.warning(message)
        return

    if not root_locations:
        message = "GafferThreeRigs node has no rootLocation!"
        setMessage(message)
        log.warning(message)
        return
    else:
        if root_locations and root_locations[0].getValue(0) is "":
            message = "GafferThreeRigs has no rootLocation"
            log.warning(message)
            return

    master_chr_name = root_locations[0].getValue(0).split("/")[-1]
    chr_layer_rig_node = getRootRig(gaffer_three_node, cRigLayers[0])
    master_chr_rig = None
    rest_rig_set = []
    for chr_rig in chr_layer_rig_node.getChildren():
        if chr_rig.getType() != "Group":
            continue
        location = chr_rig.getParameter('__gaffer.location').getValue(0)
        current_chr_name = location.split("/")[-1]
        if current_chr_name == master_chr_name:
            master_chr_rig = chr_rig
        elif current_chr_name == "common":
            pass
        else:
            rest_rig_set.append(chr_rig)

    master_params_map = {}
    for light_group in master_chr_rig.getChildren():
        material_node_param = light_group.getParameter('node_material')
        location = light_group.getParameter('__gaffer.location')
        if material_node_param is not None:
            light_name = location.getValue(0).split("/")[-1]
            material_node_name = material_node_param.getValue(0)
            material_node = NodegraphAPI.GetNode(material_node_name)
            params_list = material_node.getParameter\
                ('shaders.arnoldLightParams').getChildren()
            master_params_map[light_name] = params_list

    for rig in rest_rig_set:
        for light_group in rig.getChildren():
            material_node_param = light_group.getParameter('node_material')
            location = light_group.getParameter('__gaffer.location')
            if material_node_param is not None:
                light_name = location.getValue(0).split("/")[-1]
                material_node_name = material_node_param.getValue(0)
                material_node = NodegraphAPI.GetNode(material_node_name)
                params_list = material_node.getParameter\
                    ('shaders.arnoldLightParams').getChildren()
                for param in params_list:
                    param_enable = param.getChild("enable")
                    if param_enable is not None:
                        # if this material param is not enable, we copy master 
                        # params to here and set it enable.
                        if light_name in master_params_map:
                            master_params_list = master_params_map[light_name]
                            copyParam(param, master_params_list)
    message = "Parameters in {%s} node have been updated!"%gaffer_three_node.getName()

    setMessage(message)

if __name__ == "__main__":
    print "kevin tsui"