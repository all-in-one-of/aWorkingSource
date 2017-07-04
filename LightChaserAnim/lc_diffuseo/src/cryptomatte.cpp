#include <ai.h>

#include "utils.h"

AI_SHADER_NODE_EXPORT_METHODS(cryptomatteMethods);


enum cryptomatteParams
{
   p_sourceShader,

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

   p_crypto_asset_override,
   p_crypto_object_override,
   p_crypto_material_override,
   p_aov_crypto_asset,
   p_aov_crypto_object,
   p_aov_crypto_material,

   p_aov_shadow_group_1,
   p_aov_shadow_group_2,
   p_aov_shadow_group_3,
   p_aov_shadow_group_4,
   p_aov_shadow_group_5,
   p_aov_shadow_group_6,
   p_aov_shadow_group_7,
   p_aov_shadow_group_8,

   p_standardAovs,

   p_aiEnableMatte,
   p_aiMatteColor,
   p_aiMatteColorA
};

node_parameters
{
   AiParameterRGB("sourceShader", 1.0f, 0.0f, 0.0f);

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

   AiParameterStr("crypto_asset_override", "");
   AiParameterStr("crypto_object_override", "");
   AiParameterStr("crypto_material_override", "");
   AiParameterStr("aov_crypto_asset", "crypto_asset");
   AiParameterStr("aov_crypto_object", "crypto_object");
   AiParameterStr("aov_crypto_material", "crypto_material");

   AiParameterStr("aov_shadow_group_1", "shadow_group_1");
   AiParameterStr("aov_shadow_group_2", "shadow_group_2");
   AiParameterStr("aov_shadow_group_3", "shadow_group_3");
   AiParameterStr("aov_shadow_group_4", "shadow_group_4");
   AiParameterStr("aov_shadow_group_5", "shadow_group_5");
   AiParameterStr("aov_shadow_group_6", "shadow_group_6");
   AiParameterStr("aov_shadow_group_7", "shadow_group_7");
   AiParameterStr("aov_shadow_group_8", "shadow_group_8");

   AiParameterBool("standardCompatibleAOVs", false);

   AiParameterBOOL("aiEnableMatte", false);
   AiParameterRGB("aiMatteColor", 0.0f, 0.0f, 0.0f);
   AiParameterFlt("aiMatteColorA", 0.0f);
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
   REGISTER_AOVS_CUSTOM
   
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

   AtRGB result = AI_RGB_BLACK;
   AtRGB result_opacity = sg->out_opacity;

   AtVector N = sg->N;
   AtVector Ng = sg->Ng;
   float mint = 0.1f;
   float maxt = 100000.0f;
   float spread = 1.0f;
   float falloff = 0.0f;
   AtSampler * sampler = AiSampler(10,2);
   AtVector Nbent;
   AtColor Black = {0.0036f, 0.0036f, 0.0036f};
   AtColor White = {1.20f,1.20f,1.20f};
   AtColor OriBlack = {0.0f, 0.0f, 0.0f};
   AtColor OriWhite = {1.0f,1.0f,1.0f};
   AtColor ao = AiOcclusion(&N, &Ng, sg, mint, maxt, spread, falloff, sampler, &Nbent);    
   ao = kt::invertColor(ao);
   ao = kt::maxh(ao);
   ao = kt::clamp(ao, Black, White);
   AtColor diffuse = AiShaderEvalParamRGB(p_sourceShader);
   if (diffuse != AI_RGB_BLACK)
      AiAOVSetRGB(sg, data->aovs_custom[k_diffuse_color].c_str(), diffuse);
   if (ao != AI_RGB_BLACK)
      AiAOVSetRGB(sg, data->aovs_custom[k_ao_color].c_str(), ao);

   result = diffuse*ao;
   
   sg->out.RGB = result;
   sg->out_opacity = result_opacity;

}


/*node_loader
{
   if (i > 0)
      return false;

   node->methods     = cryptomatteMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = "cryptomatte";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}*/