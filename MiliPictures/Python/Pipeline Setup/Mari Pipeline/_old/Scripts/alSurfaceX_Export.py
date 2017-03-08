#coding:utf-8
__author__ = 'kai.xu'
import mari
import os,shutil
from PySide import QtGui,QtCore
class MayaBridge(QtGui.QDialog):
    #all tools dealing  with parameter is here
    def __init__(self):
        super(MayaBridge,self).__init__()
        self.COLOR_PARAMETER_LIST = ['BacklightColor','DiffuseColor','EmissionColor','Opacity','SSSColor','SSSRadius','Specular1Color','Specular1EdgeTint','Specular1Reflectivity',
                      'Specular2Color','Specular2EdgeTint','Specular2Reflectivity','TransmissionColor',]
        self.G_EIGHT_BIT_TYPES = ['bmp', 'jpg', 'jpeg', 'png', 'ppm', 'psd', 'tga', 'tif', 'tiff', 'xbm', 'xpm']
        self.G_HIGHER_BIT_TYPES = ['exr', 'psd', 'tif', 'tiff']
        self.textures_path_map = {}
        self.input_map = {}
        self.parameter_map = {}
        self.getShaderInfo()
    def showWindows(self):
        self.setModal(True)
        self.setWindowTitle('Mili Tools')
        folderTitle = QtGui.QLabel()
        folderTitle.setText("Out Folder:")
        folderTitle.setFont(QtGui.QFont("Roman times",10,QtGui.QFont.Bold))
        folderTitle.setAlignment(QtCore.Qt.AlignTop)
        self.folderLineEdit = QtGui.QLineEdit()
        self.folderLineEdit.setText("Select Directory for Export")
        folderBrowseButton = QtGui.QPushButton()
        search_pixmap = QtGui.QPixmap(mari.resources.path(mari.resources.ICONS) + '/Folder.png')
        folderBrowseButton.setIcon(search_pixmap)
        folderBrowseButton.clicked.connect(self.getFolderButton)
        folderBrowseButton.setFixedWidth(80)
        floderPathHLayout = QtGui.QHBoxLayout()
        floderPathHLayout.addWidget(self.folderLineEdit)
        floderPathHLayout.addWidget(folderBrowseButton)
        folderMainLayout = QtGui.QVBoxLayout()
        folderMainLayout.addWidget(folderTitle)
        folderMainLayout.addLayout(floderPathHLayout)
        folderMainLayout.setAlignment(QtCore.Qt.AlignTop)
        self.channalsList = QtGui.QListWidget()
        for channel in self.getCurrentChannels():
            self.channalsList.addItem(channel.name())
        self.channalsList.itemClicked.connect(self.channalsChange)
        channalsTitle = QtGui.QLabel()
        channalsTitle.setText("Channals:")
        channalsTitle.setFont(QtGui.QFont("Roman times",10,QtGui.QFont.Bold))
        channalsTitle.setAlignment(QtCore.Qt.AlignLeft)
        channalsLayout = QtGui.QVBoxLayout()
        channalsLayout.addWidget(channalsTitle)
        channalsLayout.addWidget(self.channalsList)
        self.layerList = QtGui.QListWidget()
        layerTitle = QtGui.QLabel()
        layerTitle.setText("Layers:")
        layerTitle.setFont(QtGui.QFont("Roman times",10,QtGui.QFont.Bold))
        layerTitle.setAlignment(QtCore.Qt.AlignLeft)
        layerLayout = QtGui.QVBoxLayout()
        layerLayout.addWidget(layerTitle)
        layerLayout.addWidget(self.layerList)
        self.exportItemList = QtGui.QListWidget()
        exportItemTitle = QtGui.QLabel()
        exportItemTitle.setText("For Export:")
        exportItemTitle.setFont(QtGui.QFont("Roman times",10,QtGui.QFont.Bold))
        exportItemTitle.setAlignment(QtCore.Qt.AlignLeft)
        exportItemLayout = QtGui.QVBoxLayout()
        exportItemLayout.addWidget(exportItemTitle)
        exportItemLayout.addWidget(self.exportItemList)
        channelsMainLayout = QtGui.QHBoxLayout()
        middle_button_layout = QtGui.QVBoxLayout()
        add_button = QtGui.QPushButton("+")
        add_button.clicked.connect(self.plus)
        remove_button = QtGui.QPushButton("-")
        remove_button.clicked.connect(self.subtract)
        middle_button_layout.addStretch()
        middle_button_layout.addWidget(add_button)
        middle_button_layout.addWidget(remove_button)
        middle_button_layout.addStretch()
        arrows = QtGui.QLabel()
        arrows.setText("-->")
        arrows.setFont(QtGui.QFont("Roman times",10,QtGui.QFont.Bold))
        channelsMainLayout.addLayout(channalsLayout)
        channelsMainLayout.addWidget(arrows)
        channelsMainLayout.addLayout(layerLayout)
        channelsMainLayout.addLayout(middle_button_layout)
        channelsMainLayout.addLayout(exportItemLayout)


        bottom_layout = QtGui.QHBoxLayout()

        #Add res options
        eight_bit_combo_text = QtGui.QLabel('8-bit Files :')
        self.eight_bit_combo = QtGui.QComboBox()
        for bit_type in self.G_EIGHT_BIT_TYPES :
            self.eight_bit_combo.addItem(bit_type)
        self.eight_bit_combo.setCurrentIndex(self.eight_bit_combo.findText('png'))

        higher_bit_combo_text = QtGui.QLabel('    16/32-bit Files :')
        self.higher_bit_combo = QtGui.QComboBox()
        for bit_type in self.G_HIGHER_BIT_TYPES :
            self.higher_bit_combo.addItem(bit_type)
        self.higher_bit_combo.setCurrentIndex(self.higher_bit_combo.findText('tif'))

        self.force_overwrite_box = QtGui.QCheckBox('Force Overwrite')
        self.force_overwrite_box.setCheckState(QtCore.Qt.Checked)
        okButtom = QtGui.QPushButton()
        okButtom.setText("OK")
        okButtom.clicked.connect(self.okButton)
        cancelButton = QtGui.QPushButton()
        cancelButton.setText("Cancel")
        cancelButton.clicked.connect(self.cancelButton)

        bottom_layout.addWidget(eight_bit_combo_text)
        bottom_layout.addWidget(self.eight_bit_combo)
        bottom_layout.addWidget(higher_bit_combo_text)
        bottom_layout.addWidget(self.higher_bit_combo)
        bottom_layout.addStretch()
        bottom_layout.addWidget(self.force_overwrite_box)
        bottom_layout.addStretch()

        bottom_layout.addWidget(okButtom)
        bottom_layout.addWidget(cancelButton)

        self.mainLayout = QtGui.QVBoxLayout()
        self.mainLayout.addLayout(folderMainLayout)
        self.mainLayout.addLayout(channelsMainLayout)
        self.mainLayout.addLayout(bottom_layout)
        self.setLayout(self.mainLayout)
        self.resize(600,300)
        self.show()

    def plus(self):
        current_channel = self.channalsList.currentItem()
        current_layer = self.layerList.currentItem()
        all_exportItem_list = []
        if self.exportItemList.count() !=0:
            for i in range( self.exportItemList.count()):
                all_exportItem_list.append(self.exportItemList.item(i).text())
        if current_layer:
            layer = current_layer.text()
            channel =  current_channel.text()
            exportItem = "%s->%s"%(channel,layer)
            #print self.exportItemList.findItems("%s->%s"%(channel,layer),QtCore.Qt.MatchFlags)
            if not exportItem in all_exportItem_list:
                self.exportItemList.addItem(exportItem)

    def subtract(self):
        current_layer = self.exportItemList.currentItem()
        if current_layer:
            row = self.exportItemList.row(current_layer)
            self.exportItemList.takeItem(row)

    def channalsChange(self):
        USER_ROLE = 32
        current_channel = self.channalsList.currentItem().text()
        current_geo = mari.geo.current()
        channels = current_geo.channelList()
        self.layerList.clear()
        for channel in channels:
            if channel.name() == current_channel:
                for layer in self.getPaintableLayers(channel):
                    self.layerList.addItem(layer.name())
                    self.layerList.item(self.layerList.count() - 1).setData(USER_ROLE, layer)
                for layer in self.getProceduralLayers(channel):
                    self.layerList.addItem(layer.name())
                    self.layerList.item(self.layerList.count() - 1).setData(USER_ROLE, layer)

    def getMatchingLayers(self,layer_stack, criterionFn):
        #Returns a list of all of the layers in the stack that match the given criterion function, including substacks.
        matching = []
        for layer in layer_stack.layerList():
            if criterionFn(layer):
                matching.append(layer)
            elif hasattr(layer, 'layerStack'):
                matching.extend(self.getMatchingLayers(layer.groupStack(), criterionFn))
            if layer.hasMaskStack():
                matching.extend(self.getMatchingLayers(layer.maskStack(), criterionFn))
            if hasattr(layer, 'hasAdjustmentStack') and layer.hasAdjustmentStack():
                matching.extend(self.getMatchingLayers(layer.adjustmentStack(), criterionFn))
        return matching

    def getProceduralLayers(self,layer_stack):
        #Returns a list of all of the procedural layers in the layer stack, including in substacks.
        return self.getMatchingLayers(layer_stack, mari.Layer.isProceduralLayer)


    def getPaintableLayers(self,layer_stack):
        #Returns a list of all of the paintable layers in the layer stack, including in substacks.
        return self.getMatchingLayers(layer_stack, mari.Layer.isPaintableLayer)

    def getFolderButton(self):
        #Get Folder
        dirname = str(QtGui.QFileDialog.getExistingDirectory())

        if dirname:
            self.folderLineEdit.setText(dirname)
    def okButton(self):
        folder_path = self.folderLineEdit.text()
        if folder_path not in "Select Directory for Export" and folder_path != "" and folder_path != None:
            if os.path.exists(folder_path) :
                #if it does, get existing folders/files
                dir_contents = os.listdir(folder_path)
                if self.force_overwrite_box.checkState():
                    for fname in dir_contents:
                        os.remove(folder_path+"/"+fname)
            else:
                os.makedirs(folder_path)

            for i in range(self.exportItemList.count()):
                list_text = self.exportItemList.item(i).text()
                key_channel = list_text.split("->")[0]
                key_layer = list_text.split("->")[1]
                for channel in self.getCurrentChannels():
                    if channel.name() == key_channel:
                        for layer in self.getPaintableLayers(channel):
                            if layer.name() == key_layer:
                                path = self.export(layer)
                                self.textures_path_map[key_channel] = path
            self.mayaFileContent()
            self.close()
    def export(self,layer):
        imgset = layer.imageSet()
        output = ""
        for uv in imgset.uvIndices():
            image = imgset.image(uv)
            if image.depth() in [16, 32]:
                format = self.higher_bit_combo.currentText()
            else :
                format = self.eight_bit_combo.currentText()
            uv_fixed = str(uv + 1001)
            folder_path = self.folderLineEdit.text()
            #Set out filename
            out_fname = folder_path + "/" + layer.name() + "." + uv_fixed + "." + format
            output += out_fname + ": "
            try:
                #do the export
                image.saveAs(out_fname)
            except IOError:
                raise
            path = folder_path + "/" + layer.name() + ".<udim>." + format
        return path
    def cancelButton(self):
        self.close()
    def FL_Upper(self,string):
        #make first letter upper
        string = string.replace(string[0],string[0].upper(),1)
        return string

    def FL_Lower(self,string):
        #make first letter lower
        string = string.replace(string[0],string[0].lower(),1)
        return string

    def getCurrentChannels(self):
        current_geo = mari.geo.current()
        channels = current_geo.channelList()
        return channels

    def getShaderInfo(self):
        #objects = mari.geo.list()
        current_geo = mari.geo.current()
        shaders = current_geo.shaderList()
        for shader in shaders:
            if shader.name() == "alSurfaceX":
                for input_channel in shader.inputList():
                    try:
                        #print input_channel[0],'  :  ',input_channel[1].name()
                        self.input_map[self.FL_Upper(input_channel[0])] = input_channel[1].name()
                    except:
                        #print input_channel[0],'  :  ',input_channel[1]
                        self.input_map[self.FL_Upper(input_channel[0])] = input_channel[1]
                for parameter_name in shader.parameterNameList():
                    if parameter_name in self.COLOR_PARAMETER_LIST:
                        #print parameter_name,':',shader.getParameter(parameter_name).rgb()
                        self.parameter_map[self.FL_Lower(parameter_name)] = shader.getParameter(parameter_name).rgb()
                    elif parameter_name == "Specular1FresnelMode" or parameter_name == "Specular2FresnelMode":
                        if shader.getParameter(parameter_name) == "dielectric":
                            self.parameter_map[self.FL_Lower(parameter_name)] = 0
                        elif shader.getParameter(parameter_name) == "metallic":
                            self.parameter_map[self.FL_Lower(parameter_name)] = 1
                        else:
                            raise
                    elif parameter_name == "ConvertToGlossiness":
                        if shader.getParameter(parameter_name) == True:
                            self.parameter_map[self.FL_Lower(parameter_name)] = 1
                        else:
                            self.parameter_map[self.FL_Lower(parameter_name)] = 0
                    else:
                        #print self.FL_Lower(parameter_name),':',shader.getParameter(parameter_name)
                        self.parameter_map[self.FL_Lower(parameter_name)] = shader.getParameter(parameter_name)

    def inputs(self):
        return self.input_map

    def parameters(self):
        return self.parameter_map

    def mayaFileContent(self):
        creation_content = \
            'currentUnit -l centimeter -a degree -t film;\n'\
            'fileInfo "application" "maya";\n'\
            'fileInfo "product" "Maya 2014";\n'\
            'fileInfo "version" "2014";\n'\
            'createNode alSurfaceX -n "alSurfaceX_mari";\n'\
                'setAttr ".diffuseStrength" %s;\n'\
                'setAttr ".diffuseColor" -type "float3" %s %s %s ;\n'\
                'setAttr ".diffuseRoughness" %s;\n'\
                'setAttr ".backlightStrength" %s;\n'\
                'setAttr ".backlightColor" -type "float3" %s %s %s ;\n'\
                'setAttr ".backlightIndirectStrength" %s;\n'\
                'setAttr ".emissionStrength" %s;\n'\
                'setAttr ".emissionColor" -type "float3" %s %s %s ;\n'\
                'setAttr ".specular1Strength" %s;\n'\
                'setAttr ".specular1Color" -type "float3" %s %s %s ;\n'\
                'setAttr ".specular1Roughness" %s;\n'\
                'setAttr ".specular1Anisotropy" %s;\n'\
                'setAttr ".specular1Rotation" %s;\n'\
                'setAttr ".specular1FresnelMode" %s;\n'\
                'setAttr ".specular1Ior" %s;\n'\
                'setAttr ".specular1Reflectivity" -type "float3" %s %s %s ;\n'\
                'setAttr ".specular1EdgeTint" -type "float3" %s %s %s ;\n'\
                'setAttr ".specular2Strength" %s;\n'\
                'setAttr ".specular2Color" -type "float3" %s %s %s ;\n'\
                'setAttr ".specular2Roughness" %s;\n'\
                'setAttr ".specular2Anisotropy" %s;\n'\
                'setAttr ".specular2Rotation" %s;\n'\
                'setAttr ".specular2FresnelMode" %s;\n'\
                'setAttr ".specular2Ior" %s;\n'\
                'setAttr ".specular2Reflectivity" -type "float3" %s %s %s ;\n'\
                'setAttr ".specular2EdgeTint" -type "float3" %s %s %s ;\n'\
                'setAttr ".transmissionStrength" 0.31012657284736633;\n'\
                'setAttr ".transmissionColor" -type "float3" 0.25949493 0.25949493 0.25949493 ;\n'\
                'setAttr ".transmissionLinkToSpecular1" no;\n'\
                'setAttr ".transmissionRoughness" 0.39240506291389465;\n'\
                'setAttr ".transmissionIor" 1.4700000286102295;\n'\
                'setAttr ".opacity" -type "float3" %s %s %s ;\n'\
                'setAttr ".convertToGlossiness" %s;\n'\
            'createNode aiOptions -s -n "defaultArnoldRenderOptions";\n'\
                'setAttr ".version" -type "string" "1.2.4.2";\n'\
            'createNode aiAOVFilter -s -n "defaultArnoldFilter";\n'\
                'setAttr ".ai_translator" -type "string" "gaussian";\n'\
            'createNode aiAOVDriver -s -n "defaultArnoldDriver";\n'\
                'setAttr ".ai_translator" -type "string" "exr";\n'\
            'createNode aiAOVDriver -s -n "defaultArnoldDisplayDriver";\n'\
                'setAttr ".output_mode" 0;\n'\
                'setAttr ".ai_translator" -type "string" "maya";\n'\
            %(
                self.parameter_map["diffuseStrength"],
                self.parameter_map["diffuseColor"][0],self.parameter_map["diffuseColor"][1],self.parameter_map["diffuseColor"][2],
                self.parameter_map["diffuseRoughness"],
                self.parameter_map["backlightStrength"],
                self.parameter_map["backlightColor"][0],self.parameter_map["backlightColor"][1],self.parameter_map["backlightColor"][2],
                self.parameter_map["backlightIndirectStrength"],
                self.parameter_map["emissionStrength"],
                self.parameter_map["emissionColor"][0],self.parameter_map["emissionColor"][1],self.parameter_map["emissionColor"][2],
                self.parameter_map["specular1Strength"],
                self.parameter_map["specular1Color"][0],self.parameter_map["specular1Color"][1],self.parameter_map["specular1Color"][2],
                self.parameter_map["specular1Roughness"],
                self.parameter_map["specular1Anisotropy"],
                self.parameter_map["specular1Rotation"],
                self.parameter_map["specular1FresnelMode"],
                self.parameter_map["specular1Ior"],
                self.parameter_map["specular1Reflectivity"][0],self.parameter_map["specular1Reflectivity"][1],self.parameter_map["specular1Reflectivity"][2],
                self.parameter_map["specular1EdgeTint"][0],self.parameter_map["specular1EdgeTint"][1],self.parameter_map["specular1EdgeTint"][2],
                self.parameter_map["specular2Strength"],
                self.parameter_map["specular2Color"][0],self.parameter_map["specular2Color"][1],self.parameter_map["specular2Color"][2],
                self.parameter_map["specular2Roughness"],
                self.parameter_map["specular2Anisotropy"],
                self.parameter_map["specular2Rotation"],
                self.parameter_map["specular2FresnelMode"],
                self.parameter_map["specular2Ior"],
                self.parameter_map["specular2Reflectivity"][0],self.parameter_map["specular2Reflectivity"][1],self.parameter_map["specular2Reflectivity"][2],
                self.parameter_map["specular2EdgeTint"][0],self.parameter_map["specular2EdgeTint"][1],self.parameter_map["specular2EdgeTint"][2],
                self.parameter_map["opacity"][0],self.parameter_map["opacity"][1],self.parameter_map["opacity"][2],
                self.parameter_map["convertToGlossiness"],
            )
        connection_content = \
            'connectAttr ":defaultArnoldDisplayDriver.msg" ":defaultArnoldRenderOptions.drivers" -na;\n'\
            'connectAttr ":defaultArnoldFilter.msg" ":defaultArnoldRenderOptions.filt";\n'\
            'connectAttr ":defaultArnoldDriver.msg" ":defaultArnoldRenderOptions.drvr";\n'\
            'connectAttr "alSurfaceX_mari.msg" ":defaultShaderList1.s" -na;\n'\
            'connectAttr "defaultRenderLayer.msg" ":defaultRenderingList1.r" -na;\n'
        channel_list = []
        for i in range(self.exportItemList.count()):
            list_text = self.exportItemList.item(i).text()
            key_channel = list_text.split("->")[0]
            channel_list.append(key_channel)
        display_content = ''
        for item in self.input_map:
            if self.input_map[item] != None and item != "View_Current_Channel" and self.input_map[item] in channel_list:
                path = self.textures_path_map[self.input_map[item]]
                display_content += self.createFileNode(item,self.FL_Lower(item),path)

        content = \
            '\n' \
            '\n' \
            '//Maya ASCII 2010 scene\n' \
            '//Codeset: UTF-8\n' \
            '//Generated by The Foundry\'s Mari\n' \
            '\n' \
            'requires maya "2010";\n' \
            'fileInfo "application" "maya";\n' \
            'fileInfo "product" "Maya 2010";\n' \
            '\n' \
            'requires maya "2014";\n'\
            'requires -nodeType "aiStandIn" -nodeType "aiVolume" -nodeType "SphereLocator" -nodeType "aiOptions"\n'\
                     '-nodeType "aiAOV" -nodeType "aiAOVDriver" -nodeType "aiAOVFilter" -nodeType "aiSkyDomeLight"\n'\
                     '-nodeType "aiAreaLight" -nodeType "aiPhotometricLight" -nodeType "aiLightBlocker"\n'\
                     '-nodeType "aiSky" -nodeType "aiAmbientOcclusion" -nodeType "aiBarndoor" -nodeType "aiBump2d"\n'\
                     '-nodeType "aiBump3d" -nodeType "aiDensity" -nodeType "aiFog" -nodeType "aiGobo" -nodeType "aiHair"\n'\
                     '-nodeType "aiImage" -nodeType "aiLightDecay" -nodeType "aiMotionVector" -nodeType "aiNoise"\n'\
                     '-nodeType "aiPhysicalSky" -nodeType "aiRaySwitch" -nodeType "aiStandard" -nodeType "aiUtility"\n'\
                     '-nodeType "aiVolumeScattering" -nodeType "aiWireframe" -nodeType "aiShadowCatcher"\n'\
                     '-nodeType "aiSkin" -nodeType "aiUserDataBool" -nodeType "aiUserDataColor" -nodeType "aiUserDataFloat"\n'\
                     '-nodeType "aiUserDataInt" -nodeType "aiUserDataPnt2" -nodeType "aiUserDataString"\n'\
                     '-nodeType "aiUserDataVector" -nodeType "aiVolumeCollector" -nodeType "aiVolumeSampleFloat"\n'\
                     '-nodeType "aiVolumeSampleRgb" -nodeType "aiWriteColor" -nodeType "aiWriteFloat" -nodeType "aiHotOcean"\n'\
                     '-nodeType "alBlackbody" -nodeType "alCache" -nodeType "alCel" -nodeType "alCellNoise"\n'\
                     '-nodeType "alColorSpace" -nodeType "alCombineColor" -nodeType "alCombineFloat" -nodeType "alCurvature"\n'\
                     '-nodeType "alFlake" -nodeType "alFlowNoise" -nodeType "alFractal" -nodeType "alFresnelConductor"\n'\
                     '-nodeType "alGaborNoise" -nodeType "alHair" -nodeType "alHairX" -nodeType "alInputScalar"\n'\
                     '-nodeType "alInputVector" -nodeType "alJitterColor" -nodeType "alLayer" -nodeType "alLayerColor"\n'\
                     '-nodeType "alLayerFloat" -nodeType "alMeasured" -nodeType "alPattern" -nodeType "alRemapColor"\n'\
                     '-nodeType "alRemapFloat" -nodeType "alSurface" -nodeType "alSurfaceX" -nodeType "alSwitchColor"\n'\
                     '-nodeType "alSwitchFloat" -nodeType "alTriplanar" "mtoa" "1.2.4.2";\n'\
                    + creation_content + '\n' \
                    + connection_content + '\n' \
                    + display_content + '\n'
        obj = mari.geo.current()
        obj_name = obj.name()
        folder_path = self.folderLineEdit.text()
        target_maya_file_name = "%s/%s.ma" % (folder_path, obj_name)
        file = open(target_maya_file_name,'w')
        file.writelines(content)
        file.close()

    def createFileNode(self,file_name,attr,path):
        connect_string ='\n'\
                        'createNode file -n "%s_file";\n' \
                        'createNode place2dTexture -n "%s_place2dTexture";\n'\
                        %(file_name,file_name)
        connect_string += 'setAttr "%s_file.ftn" -type "string" "%s";\n'%(file_name,path)
        #generate connect string
        for channel in ("coverage", "translateFrame", "rotateFrame", "mirrorU", "mirrorV", "stagger", "wrapU", "wrapV",
                        "repeatUV", "offset", "rotateUV", "noiseUV", "vertexUvOne", "vertexUvTwo", "vertexUvThree", "vertexCameraOne"):
            connect_string += '\nconnectAttr "%s_place2dTexture.%s" "%s_file.%s";' % (file_name, channel, file_name, channel)

        connect_string += 'connectAttr "%s_place2dTexture.outUV" "%s_file.uvCoord";' % (file_name, file_name)
        connect_string += 'connectAttr "%s_place2dTexture.outUvFilterSize" "%s_file.uvFilterSize";' % (file_name, file_name)
        connect_string += 'connectAttr "%s_file.msg" ":defaultTextureList1.tx" -na;\n'%file_name
        connect_string += 'connectAttr "%s_place2dTexture.msg" ":defaultRenderUtilityList1.u" -na;\n'%file_name
        connect_string += 'connectAttr "%s_file.oc" "alSurfaceX_mari.%s";\n'%(file_name,attr)
        return connect_string

def main():
    global app
    app = MayaBridge()
    app.showWindows()

def _registerAction():
    menu_path = "MainWindow/P&ython"
    mari.menus.addAction(mari.actions.create('Mari alSurface for Maya', 'alSurfaceX_Export.main()'), menu_path)

if mari.app.isRunning():
    _registerAction()
