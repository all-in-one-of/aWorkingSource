#coding:utf-8
__author__ = 'kai.xu'


import maya.cmds as cmds

class ArnoldSettingUI(object):
    def __init__(self):
        pass
    def showWindows(self):
        if cmds.window('ArnoldSettingUI', q=1, exists=1):
            cmds.deleteUI('ArnoldSettingUI')
        Mytool = cmds.window('ArnoldSettingUI',title='Mili Arnold Setting',widthHeight=(420,600),sizeable=False)
        cmds.rowColumnLayout( numberOfColumns=1, columnWidth=[(1, 420)])
        cmds.text(label='Arnold Global Setting',height=30, backgroundColor=[0.5, 0.5, 0.6])
        cmds.separator(style='out')
        tabs = cmds.tabLayout('tabsNameSpace')

        #################################
        #Common
        child1 = cmds.rowColumnLayout(numberOfColumns=1, columnWidth=[(1, 420)])
        cmds.scrollLayout('scrollLayoutUnique1', height=600)


        #File Output
        cmds.frameLayout(label='File Output', collapsable=True, collapse=False,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.separator(style='in')
        cmds.attrControlGrp('defaultResolutionWidth',attribute="defaultRenderGlobals.imageFilePrefix ",label='File name prefix:')
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultRenderGlobals.animation",label='Frame/Animation ext')
        cmds.attrControlGrp(attribute="defaultRenderGlobals.outFormatControl",label='Out Format Control:')
        cmds.attrControlGrp(attribute="defaultRenderGlobals.extensionPadding",label='Frame padding:')
        cmds.setParent('..')
        cmds.setParent('..')


        #Image Size
        cmds.frameLayout(label='Image Size', collapsable=True, collapse=False,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.separator(style='in')
        cmds.attrControlGrp('defaultResolutionWidth',attribute="defaultResolution.width",label='Width in pixels:')
        cmds.attrControlGrp('defaultResolutionHeight',attribute="defaultResolution.height",label='Height in pixels:')
        cmds.attrControlGrp(attribute="defaultResolution.imageSizeUnits",label='Size units:')
        cmds.attrControlGrp(attribute="defaultResolution.dpi",label='Resolution:')
        cmds.attrControlGrp(attribute="defaultResolution.pixelDensityUnits",label='Resolution units:')
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultResolution.deviceAspectRatio",label='Device aspect ratio:')
        cmds.attrControlGrp(attribute="defaultResolution.pixelAspect",label='Pixel aspect ratio:')
        cmds.setParent('..')
        cmds.setParent('..')


        #Render Option
        cmds.frameLayout(label='Render Option', collapsable=True, collapse=False,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultRenderGlobals.preMel",label='Pre render MEL:')
        cmds.attrControlGrp(attribute="defaultRenderGlobals.postMel",label='Post render MEL')
        cmds.attrControlGrp(attribute="defaultRenderGlobals.prlm",label='Pre render layer MEL')
        cmds.attrControlGrp(attribute="defaultRenderGlobals.polm",label='Post render layer MEL')
        cmds.attrControlGrp(attribute="defaultRenderGlobals.prm",label='Post render frame MEL')
        cmds.attrControlGrp(attribute="defaultRenderGlobals.pom",label='Post render frame MEL')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        #################################
        #Arnold Renderer
        child2 = cmds.rowColumnLayout(numberOfColumns=1, columnWidth=[(1, 420)])
        cmds.scrollLayout('scrollLayoutUnique2', height=600)

        #Sampling
        cmds.frameLayout(label='Sampling', collapsable=True, collapse=False,borderStyle='etchedIn',width=410)
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp( attribute='defaultArnoldRenderOptions.AASamples',label='Camera(AA)')
        cmds.attrControlGrp( attribute='defaultArnoldRenderOptions.GIDiffuseSamples',label='Diffuse')
        cmds.attrControlGrp( attribute='defaultArnoldRenderOptions.GIGlossySamples',label='Glossy')
        cmds.attrControlGrp( attribute='defaultArnoldRenderOptions.GIRefractionSamples',label='Refraction')
        cmds.attrControlGrp( attribute='defaultArnoldRenderOptions.sssBssrdfSamples',label='SSS')
        cmds.attrControlGrp( attribute='defaultArnoldRenderOptions.volumeIndirectSamples',label='Volume Indirect')
        cmds.separator(style='in')
        cmds.attrControlGrp( attribute="defaultArnoldRenderOptions.lock_sampling_noise",label='Lock Sampling Pattern')
        cmds.attrControlGrp( attribute="defaultArnoldRenderOptions.sssUseAutobump",label='Use Autobump in SSS')
        cmds.setParent('..')
        cmds.columnLayout(adjustableColumn=True)
        cmds.frameLayout(label='Clamping', collapsable=True, collapse=True,borderStyle='out')
        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 145),(2,200)])
        cmds.text('Clamp Sample Values ',align='right',height=25)
        cmds.checkBox('Clamp_Sample_Values',value=cmds.getAttr("defaultArnoldRenderOptions.use_sample_clamp"),label='',onCommand=lambda *arg:self.Clamp_Sample_Values_On(),offCommand=lambda *arg:self.Clamp_Sample_Values_Off())
        cmds.text('Affect_AOVs_text',enable=cmds.getAttr("defaultArnoldRenderOptions.use_sample_clamp"),label='Affect AOVs ',align='right')
        cmds.checkBox('Affect_AOVs',enable=cmds.getAttr("defaultArnoldRenderOptions.use_sample_clamp"),value=cmds.getAttr("defaultArnoldRenderOptions.use_sample_clamp_AOVs"),label='',onCommand=lambda *arg:cmds.setAttr("defaultArnoldRenderOptions.use_sample_clamp_AOVs",1),offCommand=lambda *arg:cmds.setAttr("defaultArnoldRenderOptions.use_sample_clamp_AOVs",0))
        cmds.setParent('..')
        cmds.attrControlGrp('Max_Value',attribute="defaultArnoldRenderOptions.AASampleClamp",label='Max Value',enable=cmds.getAttr("defaultArnoldRenderOptions.use_sample_clamp"))
        cmds.setParent('..')
        cmds.frameLayout(label='Filter', collapsable=True, collapse=True,borderStyle='out')
        cmds.columnLayout(adjustableColumn=True)
        cmds.text(label=u'很抱歉，这项功能暂不支持。如有需要请联系RnD！',align='center',backgroundColor=(0.6,0.5,0.5),height=40)
        '''
        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 115),(2,120)])
        cmds.text(' ')
        cmds.optionMenu('Filter_optionMenu',label='Type')
        cmds.menuItem(label='gaussian')
        cmds.menuItem(label='blackman_harris')
        cmds.menuItem(label='box')
        cmds.menuItem(label='catrom')
        cmds.menuItem(label='catrom2d')
        cmds.menuItem(label='closest')
        cmds.menuItem(label='cone')
        cmds.menuItem(label='cook')
        cmds.menuItem(label='cubic')
        cmds.menuItem(label='disk')
        cmds.menuItem(label='farthest')
        cmds.menuItem(label='heatmap')
        cmds.menuItem(label='mitnet')
        cmds.menuItem(label='sinc')
        cmds.menuItem(label='triangle')
        cmds.menuItem(label='variance')
        cmds.menuItem(label='video')
        cmds.optionMenu('Filter_optionMenu',value='gaussian',e=True)
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldFilter.width",label='Width')
        cmds.setParent('..')
        '''
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        #Ray Depth
        cmds.frameLayout(label='Ray Depth', collapsable=True, collapse=True,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.GITotalDepth",label='Total')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.GIDiffuseDepth",label='Diffuse')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.GIGlossyDepth",label='Glossy')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.GIReflectionDepth",label='Reflection')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.GIRefractionDepth",label='Refraction')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.GIVolumeDepth",label='Volume')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.autoTransparencyDepth",label='Transparency Depth')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.autoTransparencyThreshold",label='Transparency Threshold')
        cmds.setParent('..')
        cmds.setParent('..')

        #Environment
        cmds.frameLayout(label='Environment', collapsable=True, collapse=True,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.text(label=u'很抱歉，这项功能暂不支持。如有需要请联系RnD！',align='center',backgroundColor=(0.6,0.5,0.5),height=80)
        cmds.setParent('..')
        cmds.setParent('..')

        #Motion Blur
        cmds.frameLayout(label='Motion Blur', collapsable=True, collapse=True,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 145),(2,255)])
        cmds.text('Enable  ',height=18,align='right')
        cmds.checkBox('Enable_MB',value=cmds.getAttr("defaultArnoldRenderOptions.motion_blur_enable"),label='',onCommand=lambda *args:self.Enable_MB_On(),offCommand=lambda *args:self.Enable_MB_Off())
        cmds.text('Deformation_Text', enable=cmds.getAttr("defaultArnoldRenderOptions.motion_blur_enable"),label='Deformation  ',height=18,align='right')
        cmds.checkBox('Deformation_MB',value=cmds.getAttr("defaultArnoldRenderOptions.mb_object_deform_enable"),label='',enable=cmds.getAttr("defaultArnoldRenderOptions.motion_blur_enable"),onCommand=lambda *args:self.Deformation_MB_On(),offCommand=lambda *args:self.Deformation_MB_Off())
        cmds.text('Camera_MB',label='Camera  ',height=18,align='right',enable=cmds.getAttr("defaultArnoldRenderOptions.motion_blur_enable"))
        cmds.checkBox('Camera_MB',value=cmds.getAttr("defaultArnoldRenderOptions.mb_camera_enable"),label='',enable=cmds.getAttr("defaultArnoldRenderOptions.motion_blur_enable"),onCommand=lambda *args:self.Camera_MB_On(),offCommand=lambda *args:self.Camera_MB_Off())
        cmds.setParent('..')
        cmds.attrControlGrp('Keys_MB',attribute="defaultArnoldRenderOptions.motion_steps",label='Keys',enable=cmds.getAttr("defaultArnoldRenderOptions.motion_blur_enable"))
        cmds.separator(style='in')
        cmds.rowColumnLayout(numberOfColumns=2, columnWidth=[(1, 145),(2,100)])
        cmds.text('Position_MB',label='Position  ',align='right',enable=cmds.getAttr("defaultArnoldRenderOptions.motion_blur_enable"))
        flag_MB = cmds.getAttr("defaultArnoldRenderOptions.range_type")
        if flag_MB == 0:
            optionValue = 'Start On Frame'
            enableFlag1 = True
            enableFlag2 = False
        elif flag_MB == 1:
            optionValue = 'Center On Frame'
            enableFlag1 = True
            enableFlag2 = False
        elif flag_MB == 2:
            optionValue = 'End On Frame'
            enableFlag1 = True
            enableFlag2 = False
        else:
            optionValue = 'Custom'
            enableFlag1 = False
            enableFlag2 = True
        cmds.optionMenu('Position_optionMenu_MB',label='',enable=cmds.getAttr("defaultArnoldRenderOptions.motion_blur_enable"),changeCommand=lambda *args:self.Position_optionMenu_MB())
        cmds.menuItem(label='Start On Frame')
        cmds.menuItem(label='Center On Frame')
        cmds.menuItem(label='End On Frame')
        cmds.menuItem(label='Custom')
        cmds.optionMenu('Position_optionMenu_MB',value=optionValue,e=True)
        cmds.setParent('..')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp('Length_MB',attribute="defaultArnoldRenderOptions.motion_frames",label='Length ',enable=enableFlag1)
        cmds.attrControlGrp('Start_MB',attribute="defaultArnoldRenderOptions.motion_start",label='Start ',enable=enableFlag2)
        cmds.attrControlGrp('End_MB',attribute="defaultArnoldRenderOptions.motion_end",label='End ',enable=enableFlag2)
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        #Lights
        cmds.frameLayout(label='Lights', collapsable=True, collapse=True,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.lowLightThreshold",label='Low Light Threshold')
        cmds.separator(style='in')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.lightLinking",label='Light Linking')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.shadowLinking" ,label='Shadow Linking')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        #Gamma Correction
        cmds.frameLayout(label='Gamma Correction', collapsable=True, collapse=True,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.display_gamma",label='Display Driver Gamma')
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.light_gamma",label='Lights')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.shader_gamma",label='Shaders')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.texture_gamma",label='Textures')
        cmds.setParent('..')
        cmds.setParent('..')

        #Textures
        cmds.frameLayout(label='Textures', collapsable=True, collapse=True,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.textureAutomip",label='Auto-mipmap')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.textureAcceptUnmipped",label='Accept Unmipped')
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.autotile",label='Auto-tile',changeCommand=lambda *args:self.Auto_tile())
        cmds.attrControlGrp('Tile_Size_T',attribute="defaultArnoldRenderOptions.textureAutotile",label='Tile Size',enable=cmds.getAttr("defaultArnoldRenderOptions.autotile"))
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.textureAcceptUntiled",label='Accept Untiled')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.use_existing_tiled_textures",label='Use Existing .tx Textures')
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.textureMaxMemoryMB",label='Max Cache Size(MB)')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.textureMaxOpenFiles",label='Max Open Files')
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.textureDiffuseBlur",label='Diffuse Blur')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.textureGlossyBlur",label='Glossy Blur')
        cmds.setParent('..')
        cmds.setParent('..')

        cmds.setParent('..')
        cmds.setParent('..')

        #################################
        #Systrm
        child3 = cmds.rowColumnLayout(numberOfColumns=1, columnWidth=[(1, 420)])
        cmds.scrollLayout('scrollLayoutUnique3', height=600)

        #Maya Integration
        cmds.frameLayout(label='Maya Integration', collapsable=True, collapse=False,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.progressive_rendering",label='Progressive Refinement')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.progressive_initial_level",label='Initial Sampling Level')
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.clear_before_render",label='Clear Before Render')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.force_scene_update_before_IPR_refresh",label='Force Scene Update On IPR Refresh')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.force_texture_cache_flush_after_render",label='Force Texture Cache Flush After Render')
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.enable_swatch_render",label='Enable Swatch Render')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.standinDrawOverride",label='StandIn Viewport Override')
        cmds.setParent('..')
        cmds.setParent('..')

        #Render Settings
        cmds.frameLayout(label='Render Settings', collapsable=True, collapse=False,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.renderType",label='Render Type')
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.bucketScanning",label='Bucket Scanning')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.bucketSize",label='Bucket Size')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.outputOverscan",label='Overscan')
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.threads_autodetect",label='Autodetect Threads',changeCommand=lambda *args:self.Autodetect_Threads())
        key = cmds.getAttr("defaultArnoldRenderOptions.threads_autodetect")
        if key == True:
            flag = False
        else:
            flag = True
        cmds.attrControlGrp('Threads_S',attribute="defaultArnoldRenderOptions.threads",label='Threads',enable=flag)
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.binaryAss",label='Binary-encode ASS Files')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.outputAssBoundingBox",label='Export Bounding Box(.asstoc)')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.expandProcedurals",label='Expand Procedurals')
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.kickRenderFlags",label='Kick Render Flags')
        cmds.setParent('..')
        cmds.setParent('..')

        #Search Paths
        cmds.frameLayout(label='Search Paths', collapsable=True, collapse=False,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.absoluteTexturePaths",label='Absolute Texture Paths')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.absoluteProceduralPaths",label='Absolute Procedural Paths')
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.procedural_searchpath",label='Procedural Search Path')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.shader_searchpath",label='Shader Search Path')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.texture_searchpath",label='Texture Search Path')
        cmds.setParent('..')
        cmds.setParent('..')

        #Licensing
        cmds.frameLayout(label='Licensing', collapsable=True, collapse=False,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.abortOnLicenseFail",label='About On License Fail')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.skipLicenseCheck",label='Skip License Check')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        #################################
        #AOVs
        child4 = cmds.rowColumnLayout(numberOfColumns=1, columnWidth=[(1, 420)])
        cmds.rowColumnLayout(numberOfColumns=1, columnWidth=[(1, 420)])
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.aovMode",label='Mode',changeCommand=lambda *args:self.Mode_A())
        cmds.rowColumnLayout(numberOfColumns=3, columnWidth=[(1, 143),(2,200),(3,50)])
        cmds.text('Render View AOV ',align='right')
        cmds.optionMenu('RenderViewAOVs_optionMenu',label='',changeCommand=lambda *args:self.renderViewAov())
        aiAOVs = cmds.listConnections('defaultArnoldRenderOptions.aovList')
        cmds.menuItem(label='beauty')
        if aiAOVs != None:
            for aiAOV in aiAOVs:
                name = cmds.getAttr('%s.name'%aiAOV)
                cmds.menuItem(label=name)
        cmds.text(' ')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')
        key = cmds.getAttr("defaultArnoldRenderOptions.aovMode")
        if key == 0:
            flag = False
        else:
            flag = True
        cmds.rowColumnLayout(numberOfColumns=1, columnWidth=[(1, 420)])
        cmds.scrollLayout('scrollLayoutUnique', height=550)

        #Default Drivers
        cmds.frameLayout('Default_Drivers_frameLayout',label='Default Drivers', collapsable=True, collapse=True,width=410,borderStyle='etchedIn',enable=flag)
        cmds.columnLayout(adjustableColumn=True)
        cmds.rowColumnLayout(numberOfColumns=3, columnWidth=[(1, 140),(2,100),(3,50)])
        cmds.text('Default Arnold Display Driver ',align='right')
        cmds.optionMenu(label='')
        cmds.menuItem(label='exr')
        cmds.text(' ')
        cmds.setParent('..')
        cmds.frameLayout(collapsable=False, labelVisible=False,collapse=True,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldDisplayDriver.exrCompression",label='Compression')
        cmds.attrControlGrp(attribute="defaultArnoldDisplayDriver.halfPrecision",label='Half Precision')
        cmds.attrControlGrp(attribute="defaultArnoldDisplayDriver.preserveLayerName",label='Preserve Layer Name')
        cmds.attrControlGrp(attribute="defaultArnoldDisplayDriver.tiled",label='Tiled')
        cmds.attrControlGrp(attribute="defaultArnoldDisplayDriver.autocrop",label='Autocrop')
        cmds.attrControlGrp(attribute="defaultArnoldDisplayDriver.append",label='Append')
        cmds.frameLayout(label='Default Drivers', collapsable=True, collapse=True,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.text(label=u'很抱歉，这项功能暂不支持。如有需要请联系RnD！',align='center',backgroundColor=(0.6,0.5,0.5),height=40)
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        #AOV Browser
        cmds.frameLayout('AOV_Browser_frameLayout',label='AOV Browser', collapsable=True, collapse=False,width=410,borderStyle='etchedIn',height=200,enable=flag)
        cmds.columnLayout(adjustableColumn=True)
        cmds.text(label=u'很抱歉，这项功能正在开发，请耐心等待。',align='center',backgroundColor=(0.6,0.5,0.5),height=30)
        cmds.rowColumnLayout(numberOfColumns=3, columnWidth=[(1, 135),(2,135),(3,135)])
        cmds.columnLayout(adjustableColumn=True)
        cmds.text('AOV Groups',align='center')
        cmds.textScrollList('AOV_Groups_A', numberOfRows=8, allowMultiSelection=True,
                            append=['<builtin>', 'aiHair', 'aiSkin', 'aiStandard', 'alCel', 'alHair','alHairX','alLayer','alSurface','alSurfaceX'],height=160)
        cmds.setParent('..')
        cmds.columnLayout(adjustableColumn=True)
        cmds.text('Available AOVs',align='center')
        cmds.textScrollList('Available_AOVs_A', numberOfRows=8, allowMultiSelection=True,append=[],height=135)
        cmds.button(label='>>')
        cmds.setParent('..')
        cmds.columnLayout(adjustableColumn=True)
        cmds.text('Active AOVs',align='center')
        cmds.textScrollList('Active_AOVs_A', numberOfRows=8, allowMultiSelection=True,
                        append=[],height=135)

        cmds.button(label='<<')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        #AOVs
        cmds.frameLayout('AOVs_frameLayout',label='AOVs', collapsable=True, collapse=False,width=410,borderStyle='in')
        cmds.columnLayout(adjustableColumn=True)
        cmds.rowColumnLayout(numberOfColumns=4, columnWidth=[(1, 140),(2,100),(3,100),(4,80)])
        cmds.text(' ')
        cmds.button(label='Add Custom',command=lambda *args:self.addCustom_AOVS())
        cmds.button(label='Delete All',command=lambda *args:self.deleteAll_AOVs())
        cmds.text(' ')
        cmds.setParent('..')
        cmds.separator(style='in')
        cmds.rowColumnLayout(numberOfColumns=4, columnWidth=[(1, 140),(2,60),(3,60),(4,80)])
        cmds.text(label='name',align='center')
        cmds.text(label='data',align='center')
        cmds.text(label='driver',align='center')
        cmds.text(label='filter',align='center')
        cmds.setParent('..')
        cmds.frameLayout('aiAOV_nodeList',labelVisible=False,width=420,borderStyle='in')
        aiAOVs = cmds.listConnections('defaultArnoldRenderOptions.aovList')
        if aiAOVs != None:
            for aiAOV in aiAOVs:
                self.cmdsGuiGroup(aiAOV)
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        #################################
        #Diagnostics
        child5 = cmds.rowColumnLayout(numberOfColumns=1, columnWidth=[(1, 420)])
        cmds.scrollLayout('scrollLayoutUnique5', height=600)

        #Log
        cmds.frameLayout(label='Log', collapsable=True, collapse=False,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.log_verbosity",label='Verbosity Level')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.log_to_console",label='Console')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.log_to_file",label='File',changeCommand=lambda *args:self.File_D())
        cmds.attrControlGrp('Filename_D',attribute="defaultArnoldRenderOptions.log_filename",label='Filename',enable=cmds.getAttr("defaultArnoldRenderOptions.log_to_file"))
        cmds.attrControlGrp('Max_Warnings_D',attribute="defaultArnoldRenderOptions.log_max_warnings",label='Max.Warnings',enable=cmds.getAttr("defaultArnoldRenderOptions.log_to_file"))
        cmds.setParent('..')
        cmds.setParent('..')

        #Error Handing
        cmds.frameLayout(label='Error Handing', collapsable=True, collapse=False,width=410,borderStyle='etchedIn')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.abortOnError",label='About On Error')
        cmds.separator(style='in')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.errorColorBadTexture",label='Texture Error Color')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.errorColorBadPixel",label='NaN Error Color')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        #################################
        #Override
        child6 = cmds.rowColumnLayout(numberOfColumns=1, columnWidth=[(1, 420)])
        cmds.scrollLayout('scrollLayoutUnique6', height=600)

        #User Options
        cmds.frameLayout(label='User Options', collapsable=True, collapse=False,width=410,borderStyle='out')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.aiUserOptions",label='Option')
        cmds.setParent('..')
        cmds.setParent('..')

        #Feature Overrides
        cmds.frameLayout(label='Feature Overrides', collapsable=True, collapse=False,width=410,borderStyle='out')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.ignoreTextures",label='Ignore Textures')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.ignoreShaders",label='Ignore Shaders')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.ignoreAtmosphere",label='Ignore Atmosphere')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.ignoreLights",label='Ignore Lights')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.ignoreShadows",label='Ignore Shadows')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.ignoreSubdivision",label='Ignore Subdivision')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.ignoreDisplacement",label='Ignore Displacement')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.ignoreBump",label='Ignore Bump')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.ignoreSmoothing",label='Ignore Normal Smoothing')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.ignoreMotionBlur",label='Ignore Motion Blur')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.ignoreDof",label='Ignore Depth of Field')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.ignoreSss",label='Ignore Sub-Surface-Scattering')
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.forceTranslateShadingEngines",label='Force Translation of Shading Engines')
        cmds.setParent('..')
        cmds.setParent('..')

        #Subdivision
        cmds.frameLayout(label='Subdivision', collapsable=True, collapse=False,width=410,borderStyle='out')
        cmds.columnLayout(adjustableColumn=True)
        cmds.attrControlGrp(attribute="defaultArnoldRenderOptions.maxSubdivisions",label='Max.Subdvisions')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.setParent('..')

        cmds.tabLayout(tabs, edit=True, tabLabel=((child1, 'Common'),(child2,'Arnold Renderer'),(child3,'Systrm'),(child4,'AOVs'),(child5,'Diagnosics'),(child6,'Override')) )
        cmds.setParent('..')
        cmds.button(label='Close',height=30,command=lambda *args:cmds.deleteUI('ArnoldSettingUI'))
        cmds.showWindow(Mytool)
    def Clamp_Sample_Values_On(self):
        cmds.setAttr("defaultArnoldRenderOptions.use_sample_clamp",1)
        cmds.text('Affect_AOVs_text',enable=True,e=True)
        cmds.checkBox('Affect_AOVs',enable=True,e=True)
        cmds.attrControlGrp('Max_Value',enable=True,e=True)
    def Clamp_Sample_Values_Off(self):
        cmds.setAttr("defaultArnoldRenderOptions.use_sample_clamp",0)
        cmds.text('Affect_AOVs_text',enable=False,e=True)
        cmds.checkBox('Affect_AOVs',enable=False,e=True)
        cmds.attrControlGrp('Max_Value',enable=False,e=True)
    def Background_E(self):
        pass
    def Atmosphere_E(self):
        pass
    def Enable_MB_On(self):
        cmds.setAttr("defaultArnoldRenderOptions.motion_blur_enable",1)
        cmds.text('Deformation_Text',enable=True,e=True)
        cmds.checkBox('Deformation_MB',enable=True,e=True)
        cmds.text('Camera_MB',enable=True,e=True)
        cmds.checkBox('Camera_MB',enable=True,e=True)
        cmds.attrControlGrp('Keys_MB',enable=True,e=True)
        cmds.text('Position_MB',enable=True,e=True)
        cmds.optionMenu('Position_optionMenu_MB',enable=True,e=True)
        cmds.attrControlGrp('Length_MB',enable=True,e=True)
        cmds.attrControlGrp('Start_MB',enable=False,e=True)
        cmds.attrControlGrp('End_MB',enable=False,e=True)
    def Enable_MB_Off(self):
        cmds.setAttr("defaultArnoldRenderOptions.motion_blur_enable",0)
        cmds.text('Deformation_Text',enable=False,e=True)
        cmds.checkBox('Deformation_MB',enable=False,e=True)
        cmds.text('Camera_MB',enable=False,e=True)
        cmds.checkBox('Camera_MB',enable=False,e=True)
        cmds.attrControlGrp('Keys_MB',enable=False,e=True)
        cmds.text('Position_MB',enable=False,e=True)
        cmds.optionMenu('Position_optionMenu_MB',enable=False,e=True)
        cmds.attrControlGrp('Length_MB',enable=False,e=True)
        cmds.attrControlGrp('Start_MB',enable=False,e=True)
        cmds.attrControlGrp('End_MB',enable=False,e=True)
    def Deformation_MB_On(self):
        cmds.setAttr("defaultArnoldRenderOptions.mb_object_deform_enable",1)
    def Deformation_MB_Off(self):
        cmds.setAttr("defaultArnoldRenderOptions.mb_object_deform_enable",0)
    def Camera_MB_On(self):
        cmds.setAttr("defaultArnoldRenderOptions.mb_camera_enable",1)
    def Camera_MB_Off(self):
        cmds.setAttr("defaultArnoldRenderOptions.mb_camera_enable",0)
    def Position_optionMenu_MB(self):
        key = cmds.optionMenu('Position_optionMenu_MB',value=True,q=True)
        if key == 'Start On Frame':
            cmds.setAttr("defaultArnoldRenderOptions.range_type",0)
            cmds.attrControlGrp('Length_MB',enable=True,e=True)
            cmds.attrControlGrp('Start_MB',enable=False,e=True)
            cmds.attrControlGrp('End_MB',enable=False,e=True)
        elif key == 'Center On Frame':
            cmds.setAttr("defaultArnoldRenderOptions.range_type",1)
            cmds.attrControlGrp('Length_MB',enable=True,e=True)
            cmds.attrControlGrp('Start_MB',enable=False,e=True)
            cmds.attrControlGrp('End_MB',enable=False,e=True)
        elif key == 'End On Frame':
            cmds.setAttr("defaultArnoldRenderOptions.range_type",2)
            cmds.attrControlGrp('Length_MB',enable=True,e=True)
            cmds.attrControlGrp('Start_MB',enable=False,e=True)
            cmds.attrControlGrp('End_MB',enable=False,e=True)
        else:
            cmds.setAttr("defaultArnoldRenderOptions.range_type",3)
            cmds.attrControlGrp('Length_MB',enable=False,e=True)
            cmds.attrControlGrp('Start_MB',enable=True,e=True)
            cmds.attrControlGrp('End_MB',enable=True,e=True)
    def Auto_tile(self):
        if cmds.getAttr("defaultArnoldRenderOptions.autotile") == True:
            print 'Yes'
            cmds.attrControlGrp('Tile_Size_T',enable=True,e=True)
        else:
            print 'No'
            cmds.attrControlGrp('Tile_Size_T',enable=False,e=True)
    def Autodetect_Threads(self):
        key = cmds.getAttr("defaultArnoldRenderOptions.threads_autodetect")
        if key == True:
            flag = False
        else:
            flag = True
        cmds.attrControlGrp('Threads_S',enable=flag,e=True)
    def File_D(self):
        key = cmds.getAttr("defaultArnoldRenderOptions.log_to_file")
        if key == True:
            cmds.attrControlGrp('Filename_D',enable=True,e=True)
            cmds.attrControlGrp('Max_Warnings_D',enable=True,e=True)
        else:
            cmds.attrControlGrp('Filename_D',enable=False,e=True)
            cmds.attrControlGrp('Max_Warnings_D',enable=False,e=True)
    def Mode_A(self):
        key = cmds.getAttr("defaultArnoldRenderOptions.aovMode")
        if key == 0:
            flag = False
        else:
            flag = True
        cmds.frameLayout('Default_Drivers_frameLayout',enable=flag,e=True)
        cmds.frameLayout('AOV_Browser_frameLayout',enable=flag,e=True)
        cmds.frameLayout('AOVs_frameLayout',enable=flag,e=True)
    def refreshRenderViewAovOptionMenu(self):
        tmpList = cmds.optionMenu('RenderViewAOVs_optionMenu',itemListLong=True,q=True )
        for tmp in tmpList:
            cmds.deleteUI(tmp)
        aiAOVs = cmds.listConnections('defaultArnoldRenderOptions.aovList')
        cmds.menuItem(parent='RenderViewAOVs_optionMenu',label='beauty')
        if aiAOVs != None:
            for aiAOV in aiAOVs:
                name = cmds.getAttr('%s.name'%aiAOV)
                cmds.menuItem(parent='RenderViewAOVs_optionMenu',label=name)
        cmds.setAttr("defaultArnoldRenderOptions.displayAOV", 'beauty',type="string")
    def renderViewAov(self):
        displayAOV = cmds.optionMenu('RenderViewAOVs_optionMenu',value=True,q=True)
        cmds.setAttr("defaultArnoldRenderOptions.displayAOV", displayAOV,type="string")
    def addCustom_AOVS(self):
        self.initCreateAovWindow('aiAOV_nodeList')
    def deleteAll_AOVs(self):
        aiAOVs = cmds.listConnections('defaultArnoldRenderOptions.aovList')
        if aiAOVs != None:
            for aiAOV in aiAOVs:
                cmds.deleteUI(aiAOV)
                cmds.delete(aiAOV)
        self.refreshRenderViewAovOptionMenu()
    def initCreateAovWindow(self,parent):
        self.parent=parent
        if cmds.window('AddCustomWindows', q=1, exists=1):
            cmds.deleteUI('AddCustomWindows')
        Mytool = cmds.window('AddCustomWindows',title='New AOV',widthHeight=(190,80),sizeable=False,minimizeButton=False,maximizeButton=False)
        cmds.rowColumnLayout( numberOfColumns=3, columnWidth=[(1, 10),(2,170),(3,10)])
        cmds.columnLayout(adjustableColumn=True)
        cmds.setParent('..')
        cmds.rowColumnLayout( numberOfColumns=1, columnWidth=[(1,170)],rowSpacing=(1,5))
        cmds.text(label='AOV Name:',align='left')
        cmds.textField('NewAovName')
        cmds.rowColumnLayout( numberOfColumns=3, columnWidth=[(1, 80),(2,10),(3,80)])
        cmds.button(label='Create', command=lambda *arg:self.create())
        cmds.text(' ')
        cmds.button(label='Cancel', command=lambda *arg:self.cancel())
        cmds.setParent('..')
        cmds.columnLayout(adjustableColumn=True)
        cmds.setParent('..')
        cmds.setParent('..')
        cmds.showWindow(Mytool)
    def getAovName(self):
        return cmds.textField('NewAovName',text=True,q=True)
    def create(self):
        customAOVName = self.getAovName()
        if customAOVName != '':
            node = cmds.createNode("aiAOV",name='aiAOV_%s'%customAOVName)
            cmds.setAttr(node+'.name',customAOVName,type='string')
            cmds.connectAttr('defaultArnoldDriver.message','%s.outputs[0].driver'%node,force=True)
            cmds.connectAttr('defaultArnoldFilter.message','%s.outputs[0].filter'%node,force=True)
            i = 0
            while True:
                try:
                    cmds.connectAttr('%s.message'%node,'defaultArnoldRenderOptions.aovList[%s]'%i)
                    break
                except RuntimeError:
                    i += 1
            cmds.deleteUI('AddCustomWindows')
            self.cmdsGuiGroup(node)
            self.refreshRenderViewAovOptionMenu()
    def cmdsGuiGroup(self,aiAOV):
        name = cmds.getAttr('%s.name'%aiAOV)
        layout = cmds.frameLayout(aiAOV,parent='aiAOV_nodeList',labelVisible=False,width=340,borderStyle='in')
        cmds.rowColumnLayout(numberOfColumns=7, columnWidth=[(1, 140),(2,60),(3,60),(4,80),(5,5),(6,25),(7,25)])
        nameLayout = cmds.rowColumnLayout(numberOfColumns=4, columnWidth=[(1, 5),(2,25),(3,105),(4,5)])
        cmds.text(' ')
        cmds.checkBox(label='',value=True,align='center')
        cmds.textField(text = name)
        cmds.text(' ')
        cmds.setParent('..')
        dataOptionMenu = cmds.optionMenu('dataOptionMenu',label='')
        cmds.menuItem(label='int')
        cmds.menuItem(label='bool')
        cmds.menuItem(label='float')
        cmds.menuItem(label='rgb')
        cmds.menuItem(label='rgba')
        cmds.menuItem(label='vector')
        cmds.menuItem(label='point')
        cmds.menuItem(label='point2')
        cmds.menuItem(label='pointer')
        cmds.optionMenu(label='',height=20)
        cmds.menuItem(label='<exr>')
        cmds.optionMenu(label='',height=20)
        cmds.menuItem(label='gaussian')
        data = cmds.getAttr('%s.type'%aiAOV)
        if data == 1:
            dataname = 'int'
        elif data == 3:
            dataname = 'bool'
        elif data == 4:
            dataname = 'float'
        elif data == 5:
            dataname = 'rgb'
        elif data == 6:
            dataname = 'rgba'
        elif data == 7:
            dataname = 'vector'
        elif data == 8:
            dataname = 'point'
        elif data == 9:
            dataname = 'point2'
        elif data == 11:
            dataname = 'pointer'
        else:
            pass
        cmds.optionMenu('dataOptionMenu',e=True,value=dataname,changeCommand=lambda *args:self.dataOptionMenuChange(aiAOV,dataOptionMenu))
        cmds.text(' ')
        cmds.iconTextButton( style='iconOnly', image1='selectByHierarchy.png', label='',height=25,c=lambda:cmds.select(aiAOV))
        cmds.iconTextButton( style='iconOnly', image1='deleteActive.png', label='',height=25,c=lambda:(cmds.delete(aiAOV),cmds.deleteUI(layout),self.refreshRenderViewAovOptionMenu()))
        cmds.setParent('..')
        cmds.setParent('..')
    def dataOptionMenuChange(self,aiAOV,dataOptionMenu):
        dataname = cmds.optionMenu(dataOptionMenu,q=True,value=True)
        if dataname == 'int':
            data = 1
        elif dataname == 'bool':
            data = 3
        elif dataname == 'float':
            data = 4
        elif dataname == 'rgb':
            data = 5
        elif dataname == 'rgba':
            data = 6
        elif dataname == 'vector':
            data = 7
        elif dataname == 'point':
            data = 8
        elif dataname == 'point2':
            data = 9
        elif dataname == 'pointer':
            data = 11
        else:
            pass
        cmds.setAttr("%s.type"%aiAOV,data)
    def cancel(self):
        cmds.deleteUI('AddCustomWindows')
        pass
    def isArnold(self):
        try:
            cmds.getAttr("defaultArnoldRenderOptions.AASamples")
            return True
        except:
            return False
    def dialog(self):
        cmds.confirmDialog( title=u'提示', message=u'请转换到 Arnold 渲染器！', button=u'了解')


def main():
    app = ArnoldSettingUI()
    if app.isArnold():
        app.showWindows()
    else:
        app.dialog()

if __name__ == '__main__':
    main()