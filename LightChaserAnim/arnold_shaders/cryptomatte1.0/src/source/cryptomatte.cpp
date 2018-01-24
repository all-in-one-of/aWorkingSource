#include <ai.h>
#include <vector>
#include <string>
#include <cassert>

#include <alUtil.h>
#include <aovs.h>

#include "cryptomatte/cryptomatte.h"


AI_SHADER_NODE_EXPORT_METHODS(cryptomatteMethods)

struct ShaderData
{
   // AOV names
   std::vector<std::string> aovs;
   std::vector<std::string> aovs_rgba;
   bool standardAovs;
   CryptomatteData* cryptomatte;
};

enum cryptomatteParams
{
   p_sourceShader,

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

   p_aov_crypto_asset,
   p_aov_crypto_object,
   p_aov_crypto_material,

   p_crypto_asset_override,
   p_crypto_object_override,
   p_crypto_material_override,

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
   AiParameterRGB("sourceShader", 0.0f, 0.0f, 0.0f);

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

   AiParameterStr("aov_crypto_asset", "crypto_asset");
   AiParameterStr("aov_crypto_object", "crypto_object");
   AiParameterStr("aov_crypto_material", "crypto_material");
   
   AiParameterStr("crypto_asset_override", "");
   AiParameterStr("crypto_object_override", "");
   AiParameterStr("crypto_material_override", "");
   
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
   ShaderData* data = new ShaderData;
   data->cryptomatte = new CryptomatteData();
   AiNodeSetLocalData(node, data);
}

node_finish
{
   if (AiNodeGetLocalData(node))
   {
      ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);
      if (data->cryptomatte)
         delete data->cryptomatte;
      AiNodeSetLocalData(node, NULL);
      delete data;
   }
}

node_update
{
   ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);

   // set up AOVs
   REGISTER_AOVS
   data->standardAovs = params[p_standardAovs].BOOL;
   data->cryptomatte->setup_all(AiNodeGetStr(node, "aov_crypto_asset"), AiNodeGetStr(node, "aov_crypto_object"), AiNodeGetStr(node, "aov_crypto_material"));
}

shader_evaluate
{
   ShaderData* data = (ShaderData*)AiNodeGetLocalData(node);

   AtRGB result = AI_RGB_BLACK;
   AtRGB result_opacity = AI_RGB_WHITE;

   result = AiShaderEvalParamRGB(p_sourceShader);

   sg->out.RGB = result;
   sg->out_opacity = result_opacity;

   data->cryptomatte->do_cryptomattes(sg, node, p_crypto_asset_override, p_crypto_object_override, p_crypto_material_override);
}

node_loader
{
   if (i > 0) return 0;
   node->methods = cryptomatteMethods;
   node->output_type = AI_TYPE_RGB;
   node->name = "cryptomatte";
   node->node_type = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}
