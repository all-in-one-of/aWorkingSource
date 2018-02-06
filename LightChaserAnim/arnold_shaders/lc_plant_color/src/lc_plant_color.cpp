#include <ai.h>

#include "alUtil.h"

AI_SHADER_NODE_EXPORT_METHODS(LcPlantColorMethods);

enum Params
{
      p_intputColor,
      p_color_data,
      p_color_default,
      p_intensity_data,
      p_intensity_default,
      p_lca_h_mask_data,
      p_lca_h_mask_default,
      p_lca_s_mask_data,
      p_lca_s_mask_default,
      p_lca_v_mask_data,
      p_lca_v_mask_default,
      p_vaule_data,
      p_value_default,
};

node_parameters
{
      AiParameterRGB("intputColor", 1.0f,1.0f,1.0f);
      AiParameterStr("color_data","lca_ground_color");
      AiParameterRGB("color_default", 1.0f,1.0f,0.0f);
      AiParameterStr("intensity_data","lca_ground_mask");
      AiParameterFlt("intensity_default", 0.0f);
      AiParameterStr("lca_h_mask_data","lca_h_mask");
      AiParameterFlt("lca_h_mask_default", 1.0f);
      AiParameterStr("lca_s_mask_data","lca_s_mask");
      AiParameterFlt("lca_s_mask_default", 1.0f);
      AiParameterStr("lca_v_mask_data","lca_v_mask");
      AiParameterFlt("lca_v_mask_default", 0.0f);
      AiParameterStr("vaule_data","lca_ground_value");
      AiParameterFlt("value_default", 1.0f);
}


node_initialize
{

}

node_update
{
   
}

node_finish
{

}

shader_evaluate
{
   AtRGB intputColor = AiShaderEvalParamRGB(p_intputColor);

   AtString color_data = AiShaderEvalParamStr(p_color_data);
   AtString intensity_data = AiShaderEvalParamStr(p_intensity_data);
   AtString lca_h_mask_data = AiShaderEvalParamStr(p_lca_h_mask_data);
   AtString lca_s_mask_data = AiShaderEvalParamStr(p_lca_s_mask_data);
   AtString lca_v_mask_data = AiShaderEvalParamStr(p_lca_v_mask_data);
   AtString vaule_data = AiShaderEvalParamStr(p_vaule_data);

   AtRGB color_default = AiShaderEvalParamRGB(p_color_default);
   float intensity_default = AiShaderEvalParamFlt(p_intensity_default);
   float lca_h_mask_default = AiShaderEvalParamFlt(p_lca_h_mask_default);
   float lca_s_mask_default = AiShaderEvalParamFlt(p_lca_s_mask_default);
   float lca_v_mask_default = AiShaderEvalParamFlt(p_lca_v_mask_default);
   float value_default = AiShaderEvalParamFlt(p_value_default);

   AtRGB result = AtRGB(1.0f,1.0f,1.0f);

   AtRGB color_data_rgb;
   float intensity_data_float ;
   float lca_h_mask_data_float ;
   float lca_s_mask_data_float ;
   float lca_v_mask_data_float ;
   float vaule_data_float ; 

   // read all user datas
   bool is_color_data = AiUDataGetRGB(color_data,color_data_rgb);
   if ( ! is_color_data)
      color_data_rgb = color_default;

   bool is_intensity_data = AiUDataGetFlt(intensity_data,intensity_data_float);
   if ( ! is_intensity_data)
      intensity_data_float = intensity_default;

   bool is_lca_h_mask_data = AiUDataGetFlt(lca_h_mask_data,lca_h_mask_data_float);
   if ( ! is_lca_h_mask_data)
      lca_h_mask_data_float = lca_h_mask_default;

   bool is_lca_s_mask_data = AiUDataGetFlt(lca_s_mask_data,lca_s_mask_data_float);
   if ( ! is_lca_s_mask_data)
      lca_s_mask_data_float = lca_s_mask_default;

   bool is_lca_v_mask_data = AiUDataGetFlt(lca_v_mask_data,lca_v_mask_data_float);
   if ( ! is_lca_v_mask_data)
      lca_v_mask_data_float = lca_v_mask_default;

   bool is_vaule_data = AiUDataGetFlt(vaule_data,vaule_data_float);
   if ( ! is_vaule_data)
      vaule_data_float = value_default;


   AtRGB imput_hsv = rgb2hsv(color_data_rgb);
   AtRGB imputColor_hsv = rgb2hsv(intputColor);
   AtRGB Temp;
   Temp[0] = lerp(imputColor_hsv[0],imput_hsv[0],lca_h_mask_data_float);
   Temp[1] = lerp(imputColor_hsv[1],imput_hsv[1],lca_s_mask_data_float);
   Temp[2] = lerp(imputColor_hsv[2],imput_hsv[2],lca_v_mask_data_float);

   result = lerp(intputColor,hsv2rgb(Temp),intensity_data_float)*vaule_data_float;
   sg->out.RGB() = result;

}


node_loader
{
   if (i > 0)
      return false;

   node->methods     = LcPlantColorMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = "lc_plant_color";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}