import os,json

def JobRecord(file, job):
    if os.path.isfile(file):
        infile = open(file, 'r')
        json_data = json.load(infile)
        infile.close()
    else:
        json_data = []
    json_data.append(job)
    outfile = open(file, 'w')
    json.dump(json_data, outfile, ensure_ascii=True, sort_keys = True, indent = 4)
    outfile.close()

SHADERMAPPING = {
    "lc_color_correct":
    [
        "lc_color_correct",
        {
            "input":"lc_input",
            "invert_color":"lc_invert_color",
            "hue_shift":"lc_hue_shift",
            "saturation":"lc_saturation",
            "brighten":"lc_brighten",
            "contrast":"lc_contrast",
            "contrast_pivot":"lc_contrast_pivot",
            "exposure":"lc_exposure",
            "gain":"lc_gain",
            "gamma":"lc_gamma",
            "mask":"lc_mask",
        }
    ],
    "lc_switch_color":
    [
        "lc_switch_color",
        {
            "index" : "lc_index",
            "input0" : "lc_input0",
            "input1" : "lc_input1",
            "input2" : "lc_input2",
            "input3" : "lc_input3",
            "input4" : "lc_input4",
            "input5" : "lc_input5",
            "input6" : "lc_input6",
            "input7" : "lc_input7",
            "input8" : "lc_input8",
            "input9" : "lc_input9",
            "input10" : "lc_input10",
            "input11" : "lc_input11",
            "input12" : "lc_input12",
            "input13" : "lc_input13",
            "input14" : "lc_input14",
            "input15" : "lc_input15",
            "input16" : "lc_input16",
            "input17" : "lc_input17",
            "input18" : "lc_input18",
            "input19" : "lc_input19",
            "input20" : "lc_input20",
            "threshold" : "lc_threshold",
            "smooth" : "lc_smooth",
        }
    ],
    "lc_switch_float":
    [
        "lc_switch_float",
        {
            "index" : "lc_index",
            "input0" : "lc_input0",
            "input1" : "lc_input1",
            "input2" : "lc_input2",
            "input3" : "lc_input3",
            "input4" : "lc_input4",
            "input5" : "lc_input5",
            "input6" : "lc_input6",
            "input7" : "lc_input7",
            "input8" : "lc_input8",
            "input9" : "lc_input9",
            "input10" : "lc_input10",
            "input11" : "lc_input11",
            "input12" : "lc_input12",
            "input13" : "lc_input13",
            "input14" : "lc_input14",
            "input15" : "lc_input15",
            "input16" : "lc_input16",
            "input17" : "lc_input17",
            "input18" : "lc_input18",
            "input19" : "lc_input19",
            "input20" : "lc_input20",
            "threshold" : "lc_threshold",
        }
    ],
    "lc_layer_color":
    [
        "lc_layer_color",
        {
            "layer1" : "lc_layer1", 
            "layer1_blend" : "lc_layer1_blend",
            "layer1_alpha" : "lc_layer1_alpha",
            "layer1_enabled" : "lc_layer1_enabled",
            "layer2" : "lc_layer2",
            "layer2_blend" : "lc_layer2_blend",
            "layer2_alpha" : "lc_layer2_alpha",
            "layer2_enabled" : "lc_layer2_enabled",
            "layer3" : "lc_layer3",
            "layer3_blend" : "lc_layer3_blend",
            "layer3_alpha" : "lc_layer3_alpha",
            "layer3_enabled" : "lc_layer3_enabled",
            "layer4" : "lc_layer4",
            "layer4_blend" : "lc_layer4_blend",
            "layer4_alpha" : "lc_layer4_alpha",
            "layer4_enabled" : "lc_layer4_enabled",
            "layer5" : "lc_layer5",
            "layer5_blend" : "lc_layer5_blend",
            "layer5_alpha" : "lc_layer5_alpha",
            "layer5_enabled" : "lc_layer5_enabled",
            "layer6" : "lc_layer6",
            "layer6_blend" : "lc_layer6_blend",
            "layer6_alpha" : "lc_layer6_alpha",
            "layer6_enabled" : "lc_layer6_enabled",
            "layer7" : "lc_layer7",
            "layer7_blend" : "lc_layer7_blend",
            "layer7_alpha" : "lc_layer7_alpha",
            "layer7_enabled" : "lc_layer7_enabled",
            "layer8" : "lc_layer8",
            "layer8_blend" : "lc_layer8_blend",
            "layer8_alpha" : "lc_layer8_alpha",
            "layer8_enabled" : "lc_layer8_enabled",
            "clamp_result" : "lc_clamp_result",
        }
    ],
    "lc_layer_normal":
    [
        "lc_layer_normal",
        {
            "layer0" : "lc_layer0",
            "layer0_strength" : "lc_layer0_strength",
            "layer1" : "lc_layer1",
            "layer1_blend" : "lc_layer1_blend",
            "layer1_strength" : "lc_layer1_strength",
            "layer1_enabled" : "lc_layer1_enabled",
            "layer2" : "lc_layer2",
            "layer2_blend" : "lc_layer2_blend",
            "layer2_strength" : "lc_layer2_strength",
            "layer2_enabled" : "lc_layer2_enabled",
            "layer3" : "lc_layer3",
            "layer3_blend" : "lc_layer3_blend",
            "layer3_strength" : "lc_layer3_strength",
            "layer3_enabled" : "lc_layer3_enabled",
            "layer4" : "lc_layer4",
            "layer4_blend" : "lc_layer4_blend",
            "layer4_strength" : "lc_layer4_strength",
            "layer4_enabled" : "lc_layer4_enabled",
            "layer5" : "lc_layer5",
            "layer5_blend" : "lc_layer5_blend",
            "layer5_strength" : "lc_layer5_strength",
            "layer5_enabled" : "lc_layer5_enabled",
            "layer6" : "lc_layer6",
            "layer6_blend" : "lc_layer6_blend",
            "layer6_strength" : "lc_layer6_strength",
            "layer6_enabled" : "lc_layer6_enabled",
            "normalize_result" : "lc_normalize_result",
        }
    ],
    "lc_remap_float":
    [
        "lc_remap_float",
        {
            "input" : "lc_input", 
            "RMPinputMin" : "lc_input_min", 
            "RMPinputMax" : "lc_input_max", 
            "RMPcontrast" : "lc_contrast", 
            "RMPcontrastPivot" : "lc_contrast_pivot", 
            "RMPbias" : "lc_bias", 
            "RMPgain" : "lc_gain", 
            "RMPoutputMin" : "lc_output_min", 
            "RMPoutputMax" : "lc_output_max", 
            "RMPclampEnable" : "lc_clamp_enable", 
            "RMPthreshold" : "lc_threshold", 
            "RMPclampMin" : "lc_clamp_min", 
            "RMPclampMax" : "lc_clamp_max", 
            "mask" : "lc_mask",      
        }
    ],
    "lc_remap_color":
    [
        "lc_remap_color",
        {
            "input" : "lc_input",
            "gammaVal" : "lc_gamma",
            "saturationVal" : "lc_saturation",
            "hueOffsetVal" : "lc_hue_offset",
            "contrastVal" : "lc_contrast",
            "contrastPivot" : "lc_contrast_pivot",
            "gainVal" : "lc_gain",
            "exposureVal" : "lc_exposure",
            "maskVal" : "lc_mask",    
        }
    ],
    "lc_remap_normal":
    [
        "lc_remap_normal",
        {
            "input" : "lc_input",
            "intensity" : "lc_intensity",
            "invert_x" : "lc_invert_x",
            "invert_y" : "lc_invert_y",
            "invert_z" : "lc_invert_z",
        }
    ],
    "lc_ramp_p4":
    [
        "lc_ramp_p4",
        {
            "p_type" : "lc_type",
            "p_interp" : "lc_interp",
            "p_positions_1" : "lc_positions_1",
            "p_positions_2" : "lc_positions_2",
            "p_positions_3" : "lc_positions_3",
            "p_positions_4" : "lc_positions_4",
            "p_colors_1" : "lc_colors_1",
            "p_colors_2" : "lc_colors_2",
            "p_colors_3" : "lc_colors_3",
            "p_colors_4" : "lc_colors_4",
        }
    ],
    "lc_Fractal":
    [
        "lc_noise_fractal",
        {
            "mode" : "lc_mode",
            "space" : "lc_space",
            "offest" : "lc_offest",
            "scale" : "lc_scale",
            "rot" : "lc_rot",
            "value" : "lc_value",
            "octaves" : "lc_octaves",
            "gain" : "lc_gain",
            "lacunarity" : "lc_lacunarity",
            "distortion" : "lc_distortion",
            "turbulent" : "lc_turbulent",
            "clamp_mode" : "lc_clamp_mode",
            "Time" : "lc_time",
            "color1" : "lc_color1",
            "color2" : "lc_color2",
            "position" : "lc_position",
        }
    ],
    "lc_user_data_float":
    [
        "user_data_float",
        {
            "attribute" : "attribute",
            "default_value" : "default",
        }
    ],
    "lc_add":
    [
        "add",
        {
            "input1" : "input1",
            "input2" : "input2",
        }
    ],
    "lc_multiply":
    [
        "multiply",
        {
            "input1" : "input1",
            "input2" : "input2",
        }
    ],
    "lc_clamp":
    [
        "clamp",
        {
            "input" : "input",
            "min_value" : "min",
            "max_value" : "max",
        }
    ],
    "lc_range":
    [
        "lc_remap_float",
        {
            "input" : "lc_input",
            "input_min" : "lc_input_min",
            "input_max" : "lc_input_max",
            "output_min" : "lc_output_min",
            "output_max" : "lc_output_max",
            "is_smoothstep" : "lc_clamp_enable",
        }
    ],
    "lc_user_data_int":
    [
        "user_data_int",
        {
            "attribute" : "attribute",
            "default_value" : "default",
        }
    ],
    "lc_user_data_rgb":
    [
        "user_data_rgb",
        {
            "attribute" : "attribute",
            "default_value" : "default",
        }
    ],
    "lc_user_data_float":
    [
        "user_data_float",
        {
            "attribute" : "attribute",
            "default_value" : "default",
        }
    ],
    "lc_histogram_scan":
    [
        "lc_histogram_scan",
        {
            "inputVal" : "lc_input",
            "positionVal" : "lc_position",
            "contrastVal" : "lc_contrast",
            "invertVal" : "lc_invert",
            "clampVal" : "lc_clamp",
        }
    ],
    "lc_int":
    [
        "float_to_int",
        {
            "input" : "input",
        }
    ],
    "lc_flat":
    [
        "flat",
        {
            "pure_color" : "color",
        }
    ],
    "lc_complement":
    [
        "complement",
        {
            "input" : "input",
        }
    ],
    "lc_modulo":
    [
        "modulo",
        {
            "input" : "input",
            "divisor" : "divisor",
        }
    ],
}


def getMaterialXJsonDir(file):
    file_dir = file[:-len(file.split("/")[-1])]
    file_path = os.path.join(file_dir, "matetialx")
    if not os.path.exists(file_path):
        os.makedirs(file_path)
    return file_path

def getVersoonUpKatanaDir(file):
    file_dir = file[:-len(file.split("/")[-1])]
    return file_dir

#~ Check out if this port connected with any other ports.
def isPortConnected(node, parameter_name):
    port = node.getInputPort(parameter_name)
    #~ if parameter is checkbox
    if port is None:
        return False
    ports = port.getConnectedPorts()
    if ports:
        return True
    else:
        return False

def writeScript(katanaFile, jsonFile):
    from Katana import KatanaFile,NodegraphAPI

    # if katanaFile is not None:
    KatanaFile.Load(katanaFile)
    json_data = {}
    print "\n\n########################"
    for node in NodegraphAPI.GetAllNodes():
        if node.getType() == "ArnoldShadingNode":
            node_name = node.getName()
            shader_type = node.getParameter('nodeType').getValue(0)
            if shader_type.startswith("lc_"):
                #~ check if shader_type in SHADERMAPPING
                if not shader_type in SHADERMAPPING:
                    print "#ERROR#",shader_type;continue
                json_data[node_name] = []
                #~ read paramter value and record it!
                json_data[node_name].append(shader_type)
                shader_data = []
                for para_name in SHADERMAPPING[shader_type][1]:
                    node.checkDynamicParameters()
                    k_parameter_value = node.getParameter('parameters.%s.value'%para_name)
                    k_parameter_enable = node.getParameter('parameters.%s.enable'%para_name)
                    #~ if parameter connected to a node
                    if isPortConnected(node, para_name):
                        connected_node = node.getInputPort(para_name).getConnectedPorts()[0].getNode()
                        # if k_parameter_value.getType() == "numberArray":
                        #     if node_name == "bg_remap_color":
                        #         print node.getInputPort(para_name).getConnectedPorts()[0].getNode()
                        # else:
                        #     connected_node = node.getInputPort(para_name).getConnectedPorts()[0].getNode()                        
                        shader_data.append( (False, para_name, True, connected_node.getName()) )
                    #~ if parameter is a value
                    else:
                        value = []
                        enable = False
                        if k_parameter_enable.getValue(0):
                            enable = True
                        if k_parameter_value.getType() == "numberArray":
                            for child in k_parameter_value.getChildren():
                                value.append(child.getValue(0))
                        else:
                            value.append(k_parameter_value.getValue(0))
                        shader_data.append( (enable, para_name, False, value) )
                json_data[node_name].append(shader_data)

    print "#$~",jsonFile
    outfile = open(jsonFile, 'w')
    json.dump(json_data, outfile, ensure_ascii=False, sort_keys = True, indent = 4)
    outfile.close()
    print "########################\n"

def recoverScript(katanaFile=None, jsonFile=None, logsFile=None):
    from Katana import KatanaFile,NodegraphAPI

    if katanaFile is not None:
        KatanaFile.Load(katanaFile)
        katana_file = katanaFile    
        logs_file = logsFile
        file_name = NodegraphAPI.GetRootNode().getParameter('katanaSceneName').getValue(0)
        katana_file = os.path.join(getVersoonUpKatanaDir(katana_file), file_name)
        #~ version up
        katana_file = katana_file[:-3]+"{:03}".format(int(katana_file[-3:])+1)

    if jsonFile is None:
        json_file = os.path.join(NodegraphAPI.NodegraphGlobals.GetProjectDir(), "matetialx", "translate.json")
    else:
        json_file = jsonFile

    infile = open(json_file, 'r')
    json_data = json.load(infile)
    infile.close()

    #~ loops all nodes
    for node_name in json_data:
        node = NodegraphAPI.GetNode(node_name)
        node_data = json_data[node_name]
        mx_shader_name = node_data[0]
        mx_shader_data = node_data[1]
        #~ loops all parameters
        for para_data in mx_shader_data:
            #~ read data from json data
            enable = para_data[0]
            name = para_data[1]
            connected = para_data[2]
            value = para_data[3]

            #~ translate data
            translate_shader = SHADERMAPPING[mx_shader_name][0]
            translate_para_list = SHADERMAPPING[mx_shader_name][1]
            translate_name = translate_para_list[name]

            #~ set new shader type
            node.checkDynamicParameters()
            if translate_shader != mx_shader_name:
                node.getParameter('nodeType').setValue(translate_shader, 0)
                node.checkDynamicParameters()
                if translate_shader != mx_shader_name:
                    from Katana import CacheManager
                    CacheManager.flush(isNodeGraphLoading=False)

            #~ if node connected to other
            if connected:
                if not mx_shader_name in SHADERMAPPING:
                    continue
                connected_node = NodegraphAPI.GetNode(value)
                out_port = connected_node.getOutputPorts()[0]
                
                in_port = node.getInputPort(translate_name)
                node.getInputPort(translate_name).connect(out_port)
            #~ recover parameter value
            else:
                if not enable:
                    continue
                node.checkDynamicParameters()
                para_enable = node.getParameter('parameters.%s.enable'%translate_name)
                para_enable.setValue(1, 0)
                para_value = node.getParameter('parameters.%s.value'%translate_name)
                node.checkDynamicParameters()
                if para_value.getType() == "numberArray":
                    children = para_value.getChildren()
                    for i in range(0,len(value)):
                        children[i].setValue(value[i], 0)
                elif para_value.getType() == "number":
                    para_value.setValue(float(value[0]),0)
                else:
                    para_value.setValue(str(value[0]),0)

    if katanaFile is not None:
        KatanaFile.Save(katana_file)

    if logsFile is not None:
        JobRecord(logs_file, katana_file)

def check(file):
    from Katana import KatanaFile,NodegraphAPI
    KatanaFile.Load(file)

    print ""
    for node in NodegraphAPI.GetAllNodes():
        if node.getType() == "ArnoldShadingNode":
            node_name = node.getName()
            shader_type = node.getParameter('nodeType').getValue(0)
            if shader_type.startswith("lc_"):
                #~ check if shader_type in SHADERMAPPING
                print "#ERROR#", node_name, shader_type;continue



if __name__ == "__main__":
    print "scriptKatana"