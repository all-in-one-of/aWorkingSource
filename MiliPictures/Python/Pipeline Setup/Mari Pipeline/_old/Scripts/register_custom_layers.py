import getpass
user_name = getpass.getuser()
ROOTPATH = "C:/Users/%s/Documents"%user_name
mari.gl_render.registerCustomAdjustmentLayerFromXMLFile("Posterize",ROOTPATH+"/Mari/CustomShaders/Posterize.xml")
mari.gl_render.registerCustomStandaloneShaderFromXMLFile("alSurfaceX",ROOTPATH+"/Mari/CustomShaders/alSurfaceX.xml") 
mari.gl_render.registerCustomStandaloneShaderFromXMLFile("customShader",ROOTPATH+"/Mari/CustomShaders/customStandaloneShader.xml") 
mari.gl_render.registerCustomStandaloneShaderFromXMLFile("myBRDF",ROOTPATH+"/Mari/CustomShaders/myBRDF.xml") 
mari.gl_render.registerCustomHeaderFile("ALSURFACEX_GLSLH",ROOTPATH+"/Mari/include/alSurfaceX.glslh")
mari.gl_render.registerCustomCodeFile("ALSURFACEX_GLSLC",ROOTPATH+"/Mari/Modules/alSurfaceX.glslc")