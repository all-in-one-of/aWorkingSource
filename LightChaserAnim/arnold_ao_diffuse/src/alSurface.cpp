#include <ai.h>
#include <math.h>
#include "utils.h"

#define RR_BOUNCES

AI_SHADER_NODE_EXPORT_METHODS(alSurfaceMethods);

#define GlossyMISBRDF AiCookTorranceMISBRDF
#define GlossyMISPDF AiCookTorranceMISPDF
#define GlossyMISSample AiCookTorranceMISSample

#define GlossyMISBRDF_wrap AiCookTorranceMISBRDF_wrap
#define GlossyMISPDF_wrap AiCookTorranceMISPDF_wrap
#define GlossyMISSample_wrap AiCookTorranceMISSample_wrap

#define GlossyMISCreateData AiCookTorranceMISCreateData

#define NUM_ID_AOVS 8
static const char* id_names[NUM_ID_AOVS] = {
    "id_1", "id_2", "id_3", "id_4", "id_5", "id_6", "id_7", "id_8",
};

inline void flipNormals(AtShaderGlobals* sg)
{
   sg->Nf = -sg->Nf;
   sg->Ngf = -sg->Ngf;
}

static const char* fresnel_mode_names[] = {"dielectric", "metallic", NULL};

enum FresnelModes
{
   FM_DIELECTRIC = 0,
   FM_METALLIC
};

enum SssMode
{
   SSSMODE_CUBIC = 0,
   SSSMODE_DIFFUSION = 1,
   SSSMODE_DIRECTIONAL
};

const char* SssModeNames[] = {"cubic", "diffusion", "directional", NULL};

enum Debug
{
   DB_OFF = 0,
   DB_diffuseStrength,
   DB_diffuseColor,
   DB_diffuseRoughness,
   DB_backlightStrength,
   DB_backlightColor,
   DB_sssMix,
   DB_sssWeight1,
   DB_sssColor1,
   DB_sssWeight2,
   DB_sssColor2,
   DB_sssWeight3,
   DB_sssColor3,
   DB_specular1Strength,
   DB_specular1Color,
   DB_specular1Roughness,
   DB_specular2Strength,
   DB_specular2Color,
   DB_specular2Roughness,
   DB_transmissionStrength,
   DB_transmissionColor,
   DB_transmissionRoughness,
   DB_END
};

const char* debugNames[] = {
    "off", "diffuse-strength", "diffuse-color", "diffuse-roughness",
    "backlight-strength", "backlight-color", "sss-mix", "sss-weight-1",
    "sss-color-1", "sss-weight-2", "sss-color-2", "sss-weight-3", "sss-color-3",
    "specular-1-strength", "specular-1-color", "specular-1-roughness",
    "specular-2-strength", "specular-2-color", "specular-2-roughness",
    "transmission-strength", "transmission-color", "transmission-roughness",
    NULL};

enum alSurfaceParams
{
   // diffuse
   p_diffuseStrength = 0,
   p_diffuseColor,
   p_diffuseRoughness,

   // backlight
   p_backlightStrength,
   p_backlightColor,
   p_backlightIndirectStrength,

   p_emissionStrength,
   p_emissionColor,

   // sss
   p_sssMix,
   p_sssMode,
   p_sssRadius,
   p_sssWeight1,
   p_sssRadiusColor,
   p_sssRadius2,
   p_sssWeight2,
   p_sssRadiusColor2,
   p_sssRadius3,
   p_sssWeight3,
   p_sssRadiusColor3,
   p_sssDensityScale,
   p_sssTraceSet,
   p_sssExtraSamples,

   p_ssInScatteringStrength,
   p_ssAttenuationColor,
   p_ssSpecifyCoefficients,
   p_ssScattering,
   p_ssAbsorption,
   p_ssDensityScale,
   p_ssDirection,

   p_diffuseExtraSamples,
   p_diffuseIndirectStrength,
   p_diffuseIndirectClamp,
   p_diffuseNormal,

   // specular
   p_specular1Strength,
   p_specular1Color,
   p_specular1Roughness,
   p_specular1Anisotropy,
   p_specular1Rotation,
   p_specular1FresnelMode,
   p_specular1Ior,
   p_specular1Reflectivity,
   p_specular1EdgeTint,
   p_specular1RoughnessDepthScale,
   p_specular1ExtraSamples,
   p_specular1Normal,
   p_specular1IndirectStrength,
   p_specular1IndirectClamp,
   p_specular1CausticPaths,
   p_specular1InternalDirect,

   p_specular2Strength,
   p_specular2Color,
   p_specular2Roughness,
   p_specular2Anisotropy,
   p_specular2Rotation,
   p_specular2FresnelMode,
   p_specular2Ior,
   p_specular2Reflectivity,
   p_specular2EdgeTint,
   p_specular2RoughnessDepthScale,
   p_specular2ExtraSamples,
   p_specular2Normal,
   p_specular2IndirectStrength,
   p_specular2IndirectClamp,
   p_specular2CausticPaths,
   p_specular2InternalDirect,

   // transmission
   p_transmissionStrength,
   p_transmissionColor,
   p_transmissionLinkToSpecular1,
   p_transmissionRoughness,
   p_transmissionIor,
   p_transmissionRoughnessDepthScale,
   p_transmissionEnableCaustics,
   p_transmissionExtraSamples,
   p_transmissionClamp,
   p_transmissionDoDirect,
   p_transmissionNormal,
   p_transmissionCausticPaths,

   p_id1,
   p_id2,
   p_id3,
   p_id4,
   p_id5,
   p_id6,
   p_id7,
   p_id8,

   p_aiEnableMatte,
   p_aiMatteColor,
   p_aiMatteColorA,

   p_aov_ao_color,
   p_aov_diffuse_color,
   p_aov_direct_diffuse,
   p_aov_direct_diffuse_raw,
   p_aov_indirect_diffuse,
   p_aov_indirect_diffuse_raw,
   p_aov_direct_backlight,
   p_aov_indirect_backlight,
   p_aov_direct_specular,
   p_aov_indirect_specular,
   p_aov_direct_specular_2,
   p_aov_indirect_specular_2,
   p_aov_single_scatter,
   p_aov_sss,
   p_aov_refraction,
   p_aov_emission,
   p_aov_uv,
   p_aov_depth,
   p_aov_light_group_1,
   p_aov_light_group_2,
   p_aov_light_group_3,
   p_aov_light_group_4,
   p_aov_light_group_5,
   p_aov_light_group_6,
   p_aov_light_group_7,
   p_aov_light_group_8,
   p_aov_id_1,
   p_aov_id_2,
   p_aov_id_3,
   p_aov_id_4,
   p_aov_id_5,
   p_aov_id_6,
   p_aov_id_7,
   p_aov_id_8,

   p_aov_shadow_group_1,
   p_aov_shadow_group_2,
   p_aov_shadow_group_3,
   p_aov_shadow_group_4,
   p_aov_shadow_group_5,
   p_aov_shadow_group_6,
   p_aov_shadow_group_7,
   p_aov_shadow_group_8,

   p_standardAovs,
   p_transmitAovs,
   p_rrTransmissionDepth,

   p_opacity,

   p_rr,

   p_trace_set_all,
   p_trace_set_shadows,
   p_trace_set_diffuse,
   p_trace_set_backlight,
   p_trace_set_specular1,
   p_trace_set_specular2,
   p_trace_set_transmission,

   p_debug,

   p_aov_diffuse_color_clamp,
   p_aov_direct_diffuse_clamp,
   p_aov_direct_diffuse_raw_clamp,
   p_aov_indirect_diffuse_clamp,
   p_aov_indirect_diffuse_raw_clamp,
   p_aov_direct_backlight_clamp,
   p_aov_indirect_backlight_clamp,
   p_aov_direct_specular_clamp,
   p_aov_indirect_specular_clamp,
   p_aov_direct_specular_2_clamp,
   p_aov_indirect_specular_2_clamp,
   p_aov_single_scatter_clamp,
   p_aov_sss_clamp,
   p_aov_refraction_clamp,
   p_aov_emission_clamp,
   p_aov_light_group_1_clamp,
   p_aov_light_group_2_clamp,
   p_aov_light_group_3_clamp,
   p_aov_light_group_4_clamp,
   p_aov_light_group_5_clamp,
   p_aov_light_group_6_clamp,
   p_aov_light_group_7_clamp,
   p_aov_light_group_8_clamp,

   p_bump
};


node_parameters
{
   AiParameterFLT("diffuseStrength", 1.0f);
   AiParameterRGB("diffuseColor", 0.5f, 0.5f, 0.5f);
   AiParameterFLT("diffuseRoughness", 0.0f);

   AiParameterFLT("backlightStrength", 0.0f);
   AiParameterRGB("backlightColor", 0.18f, 0.18f, 0.18f);
   AiParameterFLT("backlightIndirectStrength", 1.0f);

   AiParameterFLT("emissionStrength", 0.0f);
   AiParameterRGB("emissionColor", 1.0f, 1.0f, 1.0f);

   AiParameterFLT("sssMix", 0.0f);
   AiParameterEnum("sssMode", SSSMODE_CUBIC, SssModeNames);
   AiParameterFLT("sssRadius", 1.5f);
   AiParameterFLT("sssWeight1", 1.0f);
   AiParameterRGB("sssRadiusColor", .439, .156, .078);
   AiParameterFLT("sssRadius2", 4.0f);
   AiParameterFLT("sssWeight2", 0.0f);
   AiParameterRGB("sssRadiusColor2", .439, .08, .018);
   AiParameterFLT("sssRadius3", .75f);
   AiParameterFLT("sssWeight3", .0f);
   AiParameterRGB("sssRadiusColor3", .523, .637, .667);
   AiParameterFLT("sssDensityScale", 1.0f);
   AiParameterSTR("sssTraceSet", "");
   AiParameterINT("sssExtraSamples", 0);

   AiParameterFLT("ssInScatteringStrength", 0.0f);
   AiParameterRGB("ssAttenuationColor", 1.0f, 1.0f, 1.0f);
   AiParameterBOOL("ssSpecifyCoefficients", false);
   AiParameterRGB("ssScattering", 1.0f, 1.0f, 1.0f);
   AiParameterRGB("ssAbsorption", 1.0f, 1.0f, 1.0f);
   AiParameterFLT("ssDensityScale", 1.0f);
   AiParameterFLT("ssDirection", 0.0f);

   AiParameterINT("diffuseExtraSamples", 0);
   AiParameterFLT("diffuseIndirectStrength", 1.0f);
   AiParameterFLT("diffuseIndirectClamp", 0.0f);
   AiParameterVec("diffuseNormal", 0, 0, 0);

   AiParameterFLT("specular1Strength", 1.0f);
   AiParameterRGB("specular1Color", 1.0f, 1.0f, 1.0f);
   AiParameterFLT("specular1Roughness", 0.3f);
   AiParameterFLT("specular1Anisotropy", 0.5f);
   AiParameterFLT("specular1Rotation", 0.0f);
   AiParameterENUM("specular1FresnelMode", FM_DIELECTRIC, fresnel_mode_names);
   AiParameterFLT("specular1Ior", 1.4f);
   AiParameterRGB("specular1Reflectivity", 0.548, .549, .570);
   AiParameterRGB("specular1EdgeTint", 0.579, .598, .620);
   AiParameterFLT("specular1RoughnessDepthScale", 1.0f);
   AiParameterINT("specular1ExtraSamples", 0);
   AiParameterVec("specular1Normal", 0, 0, 0);
   AiParameterFLT("specular1IndirectStrength", 1.0f);
   AiParameterFLT("specular1IndirectClamp", 0.0f);
   AiParameterBOOL("specular1CausticPaths", false);
   AiParameterBOOL("specular1InternalDirect", true);

   AiParameterFLT("specular2Strength", 0.0f);
   AiParameterRGB("specular2Color", 1.0f, 1.0f, 1.0f);
   AiParameterFLT("specular2Roughness", 0.5f);
   AiParameterFLT("specular2Anisotropy", 0.5f);
   AiParameterFLT("specular2Rotation", 0.0f);
   AiParameterENUM("specular2FresnelMode", FM_DIELECTRIC, fresnel_mode_names);
   AiParameterFLT("specular2Ior", 1.4f);
   AiParameterRGB("specular2Reflectivity", 0.548, .549, .570);
   AiParameterRGB("specular2EdgeTint", 0.579, .598, .620);
   AiParameterFLT("specular2RoughnessDepthScale", 1.0f);
   AiParameterINT("specular2ExtraSamples", 0);
   AiParameterVec("specular2Normal", 0, 0, 0);
   AiParameterFLT("specular2IndirectStrength", 1.0f);
   AiParameterFLT("specular2IndirectClamp", 0.0f);
   AiParameterBOOL("specular2CausticPaths", false);
   AiParameterBOOL("specular2InternalDirect", true);

   AiParameterFLT("transmissionStrength", 0.0f);
   AiParameterRGB("transmissionColor", 1.0f, 1.0f, 1.0f);
   AiParameterBOOL("transmissionLinkToSpecular1", true);
   AiParameterFLT("transmissionRoughness", 0.f);
   AiParameterFLT("transmissionIor", 1.4f);
   AiParameterFLT("transmissionRoughnessDepthScale", 1.0f);
   AiParameterBOOL("transmissionEnableCaustics", true);
   AiParameterINT("transmissionExtraSamples", 0);
   AiParameterFLT("transmissionClamp", 0.0f);
   AiParameterBOOL("transmissionDoDirect", false);
   AiParameterVec("transmissionNormal", 0, 0, 0);
   AiParameterBOOL("transmissionCausticPaths", false);

   AiParameterRGB("id1", 0.0f, 0.0f, 0.0f);
   AiParameterRGB("id2", 0.0f, 0.0f, 0.0f);
   AiParameterRGB("id3", 0.0f, 0.0f, 0.0f);
   AiParameterRGB("id4", 0.0f, 0.0f, 0.0f);
   AiParameterRGB("id5", 0.0f, 0.0f, 0.0f);
   AiParameterRGB("id6", 0.0f, 0.0f, 0.0f);
   AiParameterRGB("id7", 0.0f, 0.0f, 0.0f);
   AiParameterRGB("id8", 0.0f, 0.0f, 0.0f);

   AiParameterBOOL("aiEnableMatte", false);
   AiParameterRGB("aiMatteColor", 0.0f, 0.0f, 0.0f);
   AiParameterFlt("aiMatteColorA", 0.0f);

   AiParameterStr("aov_ao_color", "ao_color");
   AiParameterStr("aov_diffuse_color", "diffuse_color");
   AiParameterStr("aov_direct_diffuse", "direct_diffuse");
   AiParameterStr("aov_direct_diffuse_raw", "direct_diffuse_raw");
   AiParameterStr("aov_indirect_diffuse", "indirect_diffuse");
   AiParameterStr("aov_indirect_diffuse_raw", "indirect_diffuse_raw");
   AiParameterStr("aov_direct_backlight", "direct_backlight");
   AiParameterStr("aov_indirect_backlight", "indirect_backlight");
   AiParameterStr("aov_direct_specular", "direct_specular");
   AiParameterStr("aov_indirect_specular", "indirect_specular");
   AiParameterStr("aov_direct_specular_2", "direct_specular_2");
   AiParameterStr("aov_indirect_specular_2", "indirect_specular_2");
   AiParameterStr("aov_single_scatter", "single_scatter");
   AiParameterStr("aov_sss", "sss");
   AiParameterStr("aov_refraction", "refraction");
   AiParameterStr("aov_emission", "emission");
   AiParameterStr("aov_uv", "uv");
   AiParameterStr("aov_depth", "depth");
   AiParameterStr("aov_light_group_1", "light_group_1");
   AiParameterStr("aov_light_group_2", "light_group_2");
   AiParameterStr("aov_light_group_3", "light_group_3");
   AiParameterStr("aov_light_group_4", "light_group_4");
   AiParameterStr("aov_light_group_5", "light_group_5");
   AiParameterStr("aov_light_group_6", "light_group_6");
   AiParameterStr("aov_light_group_7", "light_group_7");
   AiParameterStr("aov_light_group_8", "light_group_8");
   AiParameterStr("aov_id_1", "id_1");
   AiParameterStr("aov_id_2", "id_2");
   AiParameterStr("aov_id_3", "id_3");
   AiParameterStr("aov_id_4", "id_4");
   AiParameterStr("aov_id_5", "id_5");
   AiParameterStr("aov_id_6", "id_6");
   AiParameterStr("aov_id_7", "id_7");
   AiParameterStr("aov_id_8", "id_8");
   AiParameterStr("aov_shadow_group_1", "shadow_group_1");
   AiParameterStr("aov_shadow_group_2", "shadow_group_2");
   AiParameterStr("aov_shadow_group_3", "shadow_group_3");
   AiParameterStr("aov_shadow_group_4", "shadow_group_4");
   AiParameterStr("aov_shadow_group_5", "shadow_group_5");
   AiParameterStr("aov_shadow_group_6", "shadow_group_6");
   AiParameterStr("aov_shadow_group_7", "shadow_group_7");
   AiParameterStr("aov_shadow_group_8", "shadow_group_8");

   AiParameterBool("standardCompatibleAOVs", false);
   AiParameterBool("transmitAovs", false);

   AiParameterInt("rrTransmissionDepth", 1);

   AiParameterRGB("opacity", 1.0f, 1.0f, 1.0f);

   AiParameterBool("rr", true);

   AiParameterSTR("traceSetAll", "");
   AiParameterSTR("traceSetShadows", "");
   AiParameterSTR("traceSetDiffuse", "");
   AiParameterSTR("traceSetBacklight", "");
   AiParameterSTR("traceSetSpecular1", "");
   AiParameterSTR("traceSetSpecular2", "");
   AiParameterSTR("traceSetTransmission", "");

   AiParameterENUM("debug", DB_OFF, debugNames);

   AiParameterFlt("aov_diffuse_color_clamp", 0.0f);
   AiParameterFlt("aov_direct_diffuse_clamp", 0.0f);
   AiParameterFlt("aov_direct_diffuse_raw_clamp", 0.0f);
   AiParameterFlt("aov_indirect_diffuse_clamp", 0.0f);
   AiParameterFlt("aov_indirect_diffuse_raw_clamp", 0.0f);
   AiParameterFlt("aov_direct_backlight_clamp", 0.0f);
   AiParameterFlt("aov_indirect_backlight_clamp", 0.0f);
   AiParameterFlt("aov_direct_specular_clamp", 0.0f);
   AiParameterFlt("aov_indirect_specular_clamp", 0.0f);
   AiParameterFlt("aov_direct_specular_2_clamp", 0.0f);
   AiParameterFlt("aov_indirect_specular_2_clamp", 0.0f);
   AiParameterFlt("aov_single_scatter_clamp", 0.0f);
   AiParameterFlt("aov_sss_clamp", 0.0f);
   AiParameterFlt("aov_refraction_clamp", 0.0f);
   AiParameterFlt("aov_emission_clamp", 0.0f);
   AiParameterFlt("aov_light_group_1_clamp", 0.0f);
   AiParameterFlt("aov_light_group_2_clamp", 0.0f);
   AiParameterFlt("aov_light_group_3_clamp", 0.0f);
   AiParameterFlt("aov_light_group_4_clamp", 0.0f);
   AiParameterFlt("aov_light_group_5_clamp", 0.0f);
   AiParameterFlt("aov_light_group_6_clamp", 0.0f);
   AiParameterFlt("aov_light_group_7_clamp", 0.0f);
   AiParameterFlt("aov_light_group_8_clamp", 0.0f);
}

node_initialize
{
   ShaderData* data = new ShaderData();
   AiNodeSetLocalData(node, data);
}

node_update
{
   ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);

   // set up AOVs
   REGISTER_AOVS

}

node_finish
{
   if (AiNodeGetLocalData(node))
   {
      ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);

      AiNodeSetLocalData(node, NULL);
      delete data;
   }
}

shader_evaluate
{
   ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);

   AtVector N = sg->N;
   AtVector Ng = sg->Ng;
   float mint = 0.1f;
   float maxt = 100000.0f;
   float spread = 1.0f;
   float falloff = 0.0f;
   AtSampler * sampler = AiSampler(10,2);
   AtVector Nbent;
   AtColor temp_ao = AiOcclusion(&N, &Ng, sg, mint, maxt, spread, falloff, sampler, &Nbent);    
   AtColor ao = kt::invertColor(temp_ao);
   AtColor diffuse = AiShaderEvalParamRGB(p_diffuseColor);

   if (diffuse != AI_RGB_BLACK)
      AiAOVSetRGB(sg, data->aovs[k_diffuse_color].c_str(), diffuse);
   if (ao != AI_RGB_BLACK)
      AiAOVSetRGB(sg, data->aovs[k_ao_color].c_str(), ao);

   AtPoint alsPreviousIntersection;
   AtRGB als_sigma_t; 
   AtRGB outOpacity = AI_RGB_WHITE;
   AiStateGetMsgPnt("alsPreviousIntersection",&alsPreviousIntersection);
   AiStateGetMsgRGB("alsPrevious_sigma_t", &als_sigma_t);
   float z = AiV3Dist(sg->P, alsPreviousIntersection);
   outOpacity.r = fast_exp(-z * als_sigma_t.r);
   outOpacity.g = fast_exp(-z * als_sigma_t.g);
   outOpacity.b = fast_exp(-z * als_sigma_t.b);
   outOpacity = -kt::log(outOpacity);


   sg->out.RGB = diffuse*ao;
   sg->out_opacity = outOpacity;
}

/*node_loader
{
   if (i > 0)
      return false;

   node->methods     = alSurfaceMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = "alSurface";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}*/