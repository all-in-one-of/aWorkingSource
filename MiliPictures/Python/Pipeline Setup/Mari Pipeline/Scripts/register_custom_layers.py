import mari
import globalSet
ROOTPATH = globalSet.ROOTPATH
mari.gl_render.registerCustomAdjustmentLayerFromXMLFile("Posterize",ROOTPATH+"/Shaders/Posterize.xml")
mari.gl_render.registerCustomStandaloneShaderFromXMLFile("alSurfaceX",ROOTPATH+"/Shaders/alSurfaceX.xml") 
mari.gl_render.registerCustomStandaloneShaderFromXMLFile("customShader",ROOTPATH+"/Shaders/customStandaloneShader.xml") 
mari.gl_render.registerCustomStandaloneShaderFromXMLFile("myBRDF",ROOTPATH+"/Shaders/myBRDF.xml") 
mari.gl_render.registerCustomHeaderFile("ALSURFACEX_GLSLH",ROOTPATH+"/Include/alSurfaceX.glslh")
mari.gl_render.registerCustomCodeFile("ALSURFACEX_GLSLC",ROOTPATH+"/Modules/alSurfaceX.glslc")