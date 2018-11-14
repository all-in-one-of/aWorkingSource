#include <ai.h>

#include <cstdio>

extern const AtNodeMethods* MayaMultiplyDivideMtd;
extern const AtNodeMethods* MayaClampMtd;
extern const AtNodeMethods* MayaGammaCorrectMtd;
extern const AtNodeMethods* MayaConditionMtd;
extern const AtNodeMethods* MayaReverseMtd;
extern const AtNodeMethods* MayaBlendColorsMtd;
extern const AtNodeMethods* MayaPlusMinusAverage1DMtd;
extern const AtNodeMethods* MayaPlusMinusAverage2DMtd;
extern const AtNodeMethods* MayaPlusMinusAverage3DMtd;
extern const AtNodeMethods* MayaRemapValueToValueMtd;
extern const AtNodeMethods* MayaRemapValueToColorMtd;
extern const AtNodeMethods* MayaRemapColorMtd;
extern const AtNodeMethods* MayaFileMtd;
extern const AtNodeMethods* MayaPlace2DTextureMtd;
extern const AtNodeMethods* MayaRampMtd;
extern const AtNodeMethods* MayaProjectionMtd;
extern const AtNodeMethods* MayaLuminanceMtd;
extern const AtNodeMethods* MayaSetRangeMtd;
extern const AtNodeMethods* MayaCheckerMtd;
extern const AtNodeMethods* MayaBulgeMtd;
extern const AtNodeMethods* MayaClothMtd;
extern const AtNodeMethods* MayaGridMtd;
extern const AtNodeMethods* MayaFractalMtd;
extern const AtNodeMethods* MayaNoiseMtd;
extern const AtNodeMethods* MayaPlace3DTextureMtd;
extern const AtNodeMethods* MayaEnvSphereMtd;
extern const AtNodeMethods* MayaRgbToHsvMtd;
extern const AtNodeMethods* MayaHsvToRgbMtd;
extern const AtNodeMethods* MayaLuminanceMtd;
extern const AtNodeMethods* MayaCloudMtd;
extern const AtNodeMethods* MayaCloudAlphaMtd;
extern const AtNodeMethods* MayaSnowMtd;
extern const AtNodeMethods* MayaContrastMtd;
extern const AtNodeMethods* MayaLayeredTextureMtd;
extern const AtNodeMethods* MayaLayeredShaderMtd;
extern const AtNodeMethods* MayaSolidFractalMtd;
extern const AtNodeMethods* MayaVolumeNoiseMtd;
extern const AtNodeMethods* MayaBrownianMtd;
extern const AtNodeMethods* MayaStuccoMtd;
extern const AtNodeMethods* MayaRemapHsvMtd;
extern const AtNodeMethods* MayaImagePlaneMtd;
extern const AtNodeMethods* MayaSurfaceShaderMtd;
extern const AtNodeMethods* ColorToFloatMtd;
extern const AtNodeMethods* VectorToFloatMtd;
extern const AtNodeMethods* PointToFloatMtd;
extern const AtNodeMethods* Point2ToFloatMtd;
extern const AtNodeMethods* WriteColorMtd;
extern const AtNodeMethods* WriteFloatMtd;
extern const AtNodeMethods* WriteColorInlineMtd;
extern const AtNodeMethods* WriteFloatInlineMtd;
extern const AtNodeMethods* WriteVectorInlineMtd;
extern const AtNodeMethods* WriteVector2InlineMtd;
extern const AtNodeMethods* AnimFloatMtd;
extern const AtNodeMethods* AnimPointMtd;
extern const AtNodeMethods* AnimVectorMtd;
extern const AtNodeMethods* AnimColorMtd;
extern const AtNodeMethods* AnimMatrixMtd;
extern const AtNodeMethods* UserDataVectorMtd;
extern const AtNodeMethods* UserDataColorMtd;
extern const AtNodeMethods* UserDataBoolMtd;
extern const AtNodeMethods* MayaShadingEngineMtd;
extern const AtNodeMethods* MayaSamplerInfo1DMtd;
extern const AtNodeMethods* MayaSamplerInfo2DMtd;
extern const AtNodeMethods* MayaSamplerInfo3DMtd;
extern const AtNodeMethods* MayaVectorDisplacementMtd;
extern const AtNodeMethods* MayaNormalDisplacementMtd;
extern const AtNodeMethods* ShadowCatcherMtd;
extern const AtNodeMethods* MayaHairMtd;
extern const AtNodeMethods* MeshLightMaterialMtd;
extern const AtNodeMethods* UserDataVec2Mtd;
extern const AtNodeMethods* MayaBump2DMtd;
extern const AtNodeMethods* MayaFluidMtd;
extern const AtNodeMethods* MayaMarbleMtd;
extern const AtNodeMethods* MayaMountainMtd;
extern const AtNodeMethods* MayaWaterMtd;
extern const AtNodeMethods* MayaCraterMtd;
extern const AtNodeMethods* MayaLeatherMtd;
extern const AtNodeMethods* MayaGraniteMtd;
extern const AtNodeMethods* MayaRockMtd;
extern const AtNodeMethods* MayaSingleShadingSwitchMtd;
extern const AtNodeMethods* MayaDoubleShadingSwitchMtd;
extern const AtNodeMethods* MayaTripleShadingSwitchMtd;
extern const AtNodeMethods* MayaQuadShadingSwitchMtd;
extern const AtNodeMethods* MayaFluidDataMtd;
extern const AtNodeMethods* MayaFluidTexture2DMtd;
extern const AtNodeMethods* SkinMtd;
extern const AtNodeMethods* MayaSurfaceLuminanceMtd;
extern const AtNodeMethods* VolumeCollectorMtd;
extern const AtNodeMethods* CameraUvMapperMtd;
extern const AtNodeMethods* MPlayDriverMtd;
extern const AtNodeMethods* VolumeSampleRgbMethods;
extern const AtNodeMethods* VolumeSampleFloatMethods;
extern const AtNodeMethods* CurvatureMethods;
extern const AtNodeMethods* MayaFlatClosureMtd;
extern const AtNodeMethods* ThinFilmMethods;
extern const AtNodeMethods* ComplexIORMethods;

enum{
   SHADER_MULTIPLYDIVIDE = 0,
   SHADER_CLAMP,
   SHADER_GAMMACORRECT,
   SHADER_CONDITION,
   SHADER_REVERSE,
   SHADER_BLENDCOLORS,
   SHADER_SAMPLERINFO1D,
   SHADER_SAMPLERINFO2D,
   SHADER_PLUSMINUSAVERAGE1D,
   SHADER_PLUSMINUSAVERAGE2D,
   SHADER_PLUSMINUSAVERAGE3D,
   SHADER_REMAPVALUETOVALUE,
   SHADER_REMAPVALUETOCOLOR,
   SHADER_REMAPCOLOR,
   SHADER_FILE,
   SHADER_PLACE2DTEXTURE,
   SHADER_RAMP,
   SHADER_PROJECTION,
   SHADER_CHECKER,
   SHADER_BULGE,
   SHADER_CLOTH,
   SHADER_GRID,
   SHADER_FRACTAL,
   SHADER_NOISE,
   SHADER_PLACE3DTEXTURE,
   SHADER_RGBTOHSV,
   SHADER_HSVTORGB,
   SHADER_LUMINANCE,
   SHADER_CLOUD,
   SHADER_CLOUDALPHA,
   SHADER_SNOW,
   SHADER_CONTRAST,
   SHADER_LAYEREDTEXTURE,
   SHADER_LAYEREDSHADER,
   SHADER_SOLIDFRACTAL,
   SHADER_VOLUMENOISE,
   SHADER_BROWNIAN,
   SHADER_STUCCO,
   SHADER_REMAPHSV,
   SHADER_SETRANGE,
   SHADER_IMAGEPLANE,
   SHADER_SURFACESHADER,
   SHADER_COLORTOFLOAT,
   SHADER_VECTORTOFLOAT,
   SHADER_POINTTOFLOAT,
   SHADER_POINT2TOFLOAT,
   SHADER_WRITECOLOR,
   SHADER_WRITECOLORINLINE,
   SHADER_WRITEFLOAT,
   SHADER_WRITEFLOATINLINE,
   SHADER_ENVSPHERE,
   SHADER_ANIMFLOAT,
   SHADER_ANIMPOINT,
   SHADER_ANIMVECTOR,
   SHADER_ANIMCOLOR,
   SHADER_ANIMMATRIX,
   SHADER_USERDATAVECTOR,
   SHADER_USERDATACOLOR,
   SHADER_SHADINGENGINE,   
   SHADER_SAMPLERINFO3D,
   SHADER_WRITEVECTORINLINE,
   SHADER_WRITEVECTOR2INLINE,
   SHADER_NORMALDISPLACEMENT,
   SHADER_VECTORDISPLACEMENT,
   SHADER_USERDATABOOL,
   SHADER_HAIR,
   SHADER_LIGHTMATERIAL,
   SHADER_USERDATAVEC2,
   SHADER_BUMP2D,
   SHADER_MAYAFLUID,
   SHADER_MAYAMARBLE,
   SHADER_MAYAMOUNTAIN,
   SHADER_MAYAWATER,
   SHADER_MAYACRATER,
   SHADER_MAYALEATHER,
   SHADER_MAYAGRANITE,
   SHADER_MAYAROCK,
   SHADER_MAYASINGLESHADINGSWITCH,
   SHADER_MAYADOUBLESHADINGSWITCH,
   SHADER_MAYATRIPLESHADINGSWITCH,
   SHADER_MAYAQUADSHADINGSWITCH,
   SHADER_MAYAFLUIDDATA,
   SHADER_MAYAFLUIDTEXTURE2D,
   SHADER_MAYASURFACELUMINANCE,
   SHADER_CAMERAUVMAPPER,
   SHADER_FLAT_CLOSURE
   
};

node_loader
{
   switch (i)
   {
   case SHADER_MULTIPLYDIVIDE:
      node->methods     = MayaMultiplyDivideMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaMultiplyDivide";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_CLAMP:
      node->methods     = MayaClampMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaClamp";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_GAMMACORRECT:
      node->methods     = MayaGammaCorrectMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaGammaCorrect";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_CONDITION:
      node->methods     = MayaConditionMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaCondition";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_REVERSE:
      node->methods     = MayaReverseMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaReverse";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_BLENDCOLORS:
      node->methods     = MayaBlendColorsMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaBlendColors";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_SAMPLERINFO1D:
      node->methods     = MayaSamplerInfo1DMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MayaSamplerInfo1D";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_SAMPLERINFO2D:
      node->methods     = MayaSamplerInfo2DMtd;
      node->output_type = AI_TYPE_VECTOR2;
      node->name        = "MayaSamplerInfo2D";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_PLUSMINUSAVERAGE1D:
      node->methods     = MayaPlusMinusAverage1DMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MayaPlusMinusAverage1D";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_PLUSMINUSAVERAGE2D:
      node->methods     = MayaPlusMinusAverage2DMtd;
      node->output_type = AI_TYPE_VECTOR2;
      node->name        = "MayaPlusMinusAverage2D";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_PLUSMINUSAVERAGE3D:
      node->methods     = MayaPlusMinusAverage3DMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaPlusMinusAverage3D";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_REMAPVALUETOVALUE:
      node->methods     = MayaRemapValueToValueMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MayaRemapValueToValue";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_REMAPVALUETOCOLOR:
      node->methods     = MayaRemapValueToColorMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaRemapValueToColor";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_REMAPCOLOR:
      node->methods     = MayaRemapColorMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaRemapColor";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_FILE:
      node->methods     = MayaFileMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaFile";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_PLACE2DTEXTURE:
      node->methods     = MayaPlace2DTextureMtd;
      node->output_type = AI_TYPE_VECTOR2;
      node->name        = "MayaPlace2DTexture";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_RAMP:
      node->methods     = MayaRampMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaRamp";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_PROJECTION:
      node->methods     = MayaProjectionMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaProjection";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_CHECKER:
      node->methods     = MayaCheckerMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaChecker";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_BULGE:
      node->methods     = MayaBulgeMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaBulge";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_CLOTH:
      node->methods     = MayaClothMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaCloth";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_GRID:
      node->methods     = MayaGridMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaGrid";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_FRACTAL:
      node->methods     = MayaFractalMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaFractal";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_NOISE:
      node->methods     = MayaNoiseMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaNoise";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_PLACE3DTEXTURE:
      node->methods     = MayaPlace3DTextureMtd;
      node->output_type = AI_TYPE_MATRIX;
      node->name        = "MayaPlace3DTexture";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_RGBTOHSV:
      node->methods     = MayaRgbToHsvMtd;
      node->output_type = AI_TYPE_VECTOR;
      node->name        = "MayaRgbToHsv";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_HSVTORGB:
      node->methods     = MayaHsvToRgbMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaHsvToRgb";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_LUMINANCE:
      node->methods     = MayaLuminanceMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MayaLuminance";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_CLOUD:
      node->methods     = MayaCloudMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaCloud";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_CLOUDALPHA:
      node->methods     = MayaCloudAlphaMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MayaCloudAlpha";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_SNOW:
      node->methods     = MayaSnowMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaSnow";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_CONTRAST:
      node->methods     = MayaContrastMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaContrast";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_LAYEREDTEXTURE:
      node->methods     = MayaLayeredTextureMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaLayeredTexture";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_LAYEREDSHADER:
      node->methods     = MayaLayeredShaderMtd;
      node->output_type = AI_TYPE_CLOSURE;
      node->name        = "MayaLayeredShader";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_SOLIDFRACTAL:
      node->methods     = MayaSolidFractalMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaSolidFractal";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_VOLUMENOISE:
      node->methods     = MayaVolumeNoiseMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaVolumeNoise";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_BROWNIAN:
      node->methods     = MayaBrownianMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaBrownian";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_STUCCO:
      node->methods     = MayaStuccoMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaStucco";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_REMAPHSV:
      node->methods     = MayaRemapHsvMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaRemapHsv";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_SETRANGE:
      node->methods     = MayaSetRangeMtd;
      node->output_type = AI_TYPE_VECTOR;
      node->name        = "MayaSetRange";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_IMAGEPLANE:
      node->methods     = MayaImagePlaneMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaImagePlane";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_SURFACESHADER:
      node->methods     = MayaSurfaceShaderMtd;
      node->output_type = AI_TYPE_CLOSURE;
      node->name        = "MayaSurfaceShader";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_COLORTOFLOAT:
      node->methods     = ColorToFloatMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MtoaColorToFloat";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_VECTORTOFLOAT:
      node->methods     = VectorToFloatMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MtoaVectorToFloat";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_POINTTOFLOAT:
      node->methods     = PointToFloatMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MtoaPointToFloat";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_POINT2TOFLOAT:
      node->methods     = Point2ToFloatMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MtoaPoint2ToFloat";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_WRITECOLOR:
      node->methods     = WriteColorMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MtoaWriteColor";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_WRITECOLORINLINE:
      node->methods     = WriteColorInlineMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MtoaAovWriteColor";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_WRITEFLOAT:
      node->methods     = WriteFloatMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MtoaWriteFloat";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_WRITEFLOATINLINE:
      node->methods     = WriteFloatInlineMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MtoaAovWriteFloat";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_ENVSPHERE:
      node->methods     = MayaEnvSphereMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaEnvSphere";
      node->node_type   = AI_NODE_SHADER;
      break;      

   case SHADER_ANIMFLOAT:
      node->methods     = AnimFloatMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MtoaAnimFloat";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_ANIMPOINT:
      node->methods     = AnimPointMtd;
      node->output_type = AI_TYPE_VECTOR;
      node->name        = "MtoaAnimPoint";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_ANIMVECTOR:
      node->methods     = AnimVectorMtd;
      node->output_type = AI_TYPE_VECTOR;
      node->name        = "MtoaAnimVector";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_ANIMCOLOR:
      node->methods     = AnimColorMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MtoaAnimColor";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_ANIMMATRIX:
      node->methods     = AnimMatrixMtd;
      node->output_type = AI_TYPE_MATRIX;
      node->name        = "MtoaAnimMatrix";
      node->node_type   = AI_NODE_SHADER;
   break;
   case SHADER_USERDATAVECTOR:
      node->methods     = UserDataVectorMtd;
      node->output_type = AI_TYPE_VECTOR;
      node->name        = "MtoaUserDataVector";
      node->node_type   = AI_NODE_SHADER;
      break;
   case SHADER_USERDATACOLOR:
      node->methods     = UserDataColorMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MtoaUserDataColor";
      node->node_type   = AI_NODE_SHADER;
      break;


   case SHADER_SHADINGENGINE:
      node->methods     = MayaShadingEngineMtd;
      node->output_type = AI_TYPE_CLOSURE;
      node->name        = "MayaShadingEngine";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_SAMPLERINFO3D:
      node->methods     = MayaSamplerInfo3DMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaSamplerInfo3D";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_WRITEVECTORINLINE:
      node->methods     = WriteVectorInlineMtd;
      node->output_type = AI_TYPE_VECTOR;
      node->name        = "MtoaAovWriteVector";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_WRITEVECTOR2INLINE:
      node->methods     = WriteVector2InlineMtd;
      node->output_type = AI_TYPE_VECTOR2;
      node->name        = "MtoaAovWriteVector2";
      node->node_type   = AI_NODE_SHADER;
      break;
      
   case SHADER_NORMALDISPLACEMENT:
      node->methods     = MayaNormalDisplacementMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MayaNormalDisplacement";
      node->node_type   = AI_NODE_SHADER;
      break;
      
   case SHADER_VECTORDISPLACEMENT:
      node->methods     = MayaVectorDisplacementMtd;
      node->output_type = AI_TYPE_VECTOR;
      node->name        = "MayaVectorDisplacement";
      node->node_type   = AI_NODE_SHADER;
      break;
      
   case SHADER_USERDATABOOL:
      node->methods     = UserDataBoolMtd;
      node->output_type = AI_TYPE_BOOLEAN;
      node->name        = "MtoaUserDataBool";
      node->node_type   = AI_NODE_SHADER;
      break;
     
   
   case SHADER_HAIR:
      node->methods     = MayaHairMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaHair";
      node->node_type   = AI_NODE_SHADER;
      break;
      
   case SHADER_LIGHTMATERIAL:
      node->methods     = MeshLightMaterialMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MtoaMeshLightMaterial";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_USERDATAVEC2:
      node->methods     = UserDataVec2Mtd;
      node->output_type = AI_TYPE_VECTOR2;
      node->name        = "MtoaUserDataVec2";
      node->node_type   = AI_NODE_SHADER;
      break;
      
   case SHADER_BUMP2D:
      node->methods     = MayaBump2DMtd;
      node->output_type = AI_TYPE_VECTOR;
      node->name        = "MayaBump2D";
      node->node_type   = AI_NODE_SHADER;
      break;
      
   case SHADER_MAYAFLUID:
      node->methods     = MayaFluidMtd;
      node->output_type = AI_TYPE_CLOSURE;
      node->name        = "MayaFluid";
      node->node_type   = AI_NODE_SHADER;
      break;
      
   case SHADER_MAYAMARBLE:
      node->methods     = MayaMarbleMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaMarble";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_MAYAMOUNTAIN:
       node->methods = MayaMountainMtd;
       node->output_type = AI_TYPE_RGBA;
       node->name = "MayaMountain";
       node->node_type = AI_NODE_SHADER;
       break;

   case SHADER_MAYAWATER:
       node->methods = MayaWaterMtd;
       node->output_type = AI_TYPE_RGBA;
       node->name = "MayaWater";
       node->node_type = AI_NODE_SHADER;
       break;

   case SHADER_MAYACRATER:
       node->methods = MayaCraterMtd;
       node->output_type = AI_TYPE_RGBA;
       node->name = "MayaCrater";
       node->node_type = AI_NODE_SHADER;
       break;

   case SHADER_MAYALEATHER:
       node->methods = MayaLeatherMtd;
       node->output_type = AI_TYPE_RGBA;
       node->name = "MayaLeather";
       node->node_type = AI_NODE_SHADER;
       break;

   case SHADER_MAYAGRANITE:
       node->methods = MayaGraniteMtd;
       node->output_type = AI_TYPE_RGBA;
       node->name = "MayaGranite";
       node->node_type = AI_NODE_SHADER;
       break;

   case SHADER_MAYAROCK:
       node->methods = MayaRockMtd;
       node->output_type = AI_TYPE_RGBA;
       node->name = "MayaRock";
       node->node_type = AI_NODE_SHADER;
       break;

   case SHADER_MAYASINGLESHADINGSWITCH:
      node->methods     = MayaSingleShadingSwitchMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MayaSingleShadingSwitch";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_MAYADOUBLESHADINGSWITCH:
      node->methods     = MayaDoubleShadingSwitchMtd;
      node->output_type = AI_TYPE_VECTOR2;
      node->name        = "MayaDoubleShadingSwitch";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_MAYATRIPLESHADINGSWITCH:
      node->methods     = MayaTripleShadingSwitchMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaTripleShadingSwitch";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_MAYAQUADSHADINGSWITCH:
      node->methods     = MayaQuadShadingSwitchMtd;
      node->output_type = AI_TYPE_RGBA;
      node->name        = "MayaQuadShadingSwitch";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_MAYAFLUIDDATA:
      node->methods     = MayaFluidDataMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaFluidData";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_MAYAFLUIDTEXTURE2D:
      node->methods     = MayaFluidTexture2DMtd;
      node->output_type = AI_TYPE_RGB;
      node->name        = "MayaFluidTexture2D";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_MAYASURFACELUMINANCE:
      node->methods     = MayaSurfaceLuminanceMtd;
      node->output_type = AI_TYPE_FLOAT;
      node->name        = "MayaSurfaceLuminance";
      node->node_type   = AI_NODE_SHADER;
      break;

   case SHADER_CAMERAUVMAPPER:
      node->methods     = CameraUvMapperMtd;
      node->output_type = AI_TYPE_UNDEFINED;
      node->name        = "MtoaCameraUvMapper";
      node->node_type   = AI_NODE_CAMERA;
      break;

   case SHADER_FLAT_CLOSURE:
       node->methods = MayaFlatClosureMtd;
      node->output_type = AI_TYPE_CLOSURE;
      node->name =      "MayaFlatClosure";
      node->node_type = AI_NODE_SHADER;
      break;
   
   default:
      return false;
   }

   sprintf(node->version, AI_VERSION);

   return true;
}
