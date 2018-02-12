#include <ai.h>

#include "alUtil.h"

AI_SHADER_NODE_EXPORT_METHODS(LcJitterColorMethods);

enum Params
{
   p_input,

   p_face_shell,
   p_plant_area ,   
   p_plant_ao,       
   p_plant_py,       
   p_plant_custom,
   p_plant_custom_default,

   p_rand_intensity,
   p_rand_hue_min,
   p_rand_hue_max,
   p_rand_sat_min,
   p_rand_sat_max,
   p_rand_val_min,
   p_rand_val_max,
   p_rand_seed,

   p_py_enable ,
   p_py_posi_a,
   p_py_posi_b,
   p_py_rand_int,
   p_py_hue_min ,
   p_py_hue_max ,
   p_py_sat_min ,
   p_py_sat_max ,
   p_py_val_min ,
   p_py_val_max ,
   p_py_seed ,

   p_area_enable ,
   p_area_posi_a ,
   p_area_posi_b ,
   p_area_rand_int,
   p_area_hue_min ,
   p_area_hue_max ,
   p_area_sat_min ,
   p_area_sat_max ,
   p_area_val_min ,
   p_area_val_max ,
   p_area_seed ,

   p_ao_enable ,
   p_ao_posi_a ,
   p_ao_posi_b ,
   p_ao_rand_int ,
   p_ao_hue_min ,
   p_ao_hue_max ,
   p_ao_sat_min ,
   p_ao_sat_max ,
   p_ao_val_min ,
   p_ao_val_max ,
   p_ao_seed ,

   p_cus_enable ,
   p_cus_posi_a ,
   p_cus_posi_b ,
   p_cus_rand_int ,
   p_cus_hue_min ,
   p_cus_hue_max ,
   p_cus_sat_min ,
   p_cus_sat_max ,
   p_cus_val_min ,
   p_cus_val_max ,
   p_cus_seed ,

   p_debugMode ,
};

node_parameters
{
      AiParameterRGB("input", 1.0f,1.0f,1.0f);

      AiParameterStr("face_shell","lca_face_shell");
      AiParameterStr("plant_area","lca_shell_area");
      AiParameterStr("plant_ao","lca_shell_ao");
      AiParameterStr("plant_py","lca_shell_py");
      AiParameterStr("plant_custom","type your own float data");
      AiParameterFlt("plant_custom_default", 0.0f);

      AiParameterFlt("rand_intensity", 0.0f);
      AiParameterFlt("rand_hue_min", 0.0f);
      AiParameterFlt("rand_hue_max", 0.0f);
      AiParameterFlt("rand_sat_min", 0.0f);
      AiParameterFlt("rand_sat_max", 0.0f);
      AiParameterFlt("rand_val_min", 0.0f);
      AiParameterFlt("rand_val_max", 0.0f);
      AiParameterInt("rand_seed",0)

      AiParameterInt("py_enable",1);
      AiParameterFlt("py_posi_a", 0.0f);
      AiParameterFlt("py_posi_b", 1.0f);
      AiParameterFlt("py_rand_int", 0.0f);
      AiParameterFlt("py_hue_min", 0.0f);
      AiParameterFlt("py_hue_max", 0.0f);
      AiParameterFlt("py_sat_min", 0.0f);
      AiParameterFlt("py_sat_max", 0.0f);
      AiParameterFlt("py_val_min", 0.0f);
      AiParameterFlt("py_val_max", 0.0f);
      AiParameterInt("py_seed",0);


      AiParameterInt("area_enable",1);
      AiParameterFlt("area_posi_a", 0.0f);
      AiParameterFlt("area_posi_b", 1.0f);
      AiParameterFlt("area_rand_int", 0.0f);
      AiParameterFlt("area_hue_min", 0.0f);
      AiParameterFlt("area_hue_max", 0.0f);
      AiParameterFlt("area_sat_min", 0.0f);
      AiParameterFlt("area_sat_max", 0.0f);
      AiParameterFlt("area_val_min", 0.0f);
      AiParameterFlt("area_val_max", 0.0f);
      AiParameterInt("py_seed",0);

      AiParameterInt("ao_enable",1);
      AiParameterFlt("ao_posi_a", 0.0f);
      AiParameterFlt("ao_posi_b", 1.0f);
      AiParameterFlt("ao_rand_int", 0.0f);
      AiParameterFlt("ao_hue_min", 0.0f);
      AiParameterFlt("ao_hue_max", 0.0f);
      AiParameterFlt("ao_sat_min", 0.0f);
      AiParameterFlt("ao_sat_max", 0.0f);
      AiParameterFlt("ao_val_min", 0.0f);
      AiParameterFlt("ao_val_max", 0.0f);
      AiParameterInt("ao_seed",0);

      AiParameterInt("cus_enable",1);
      AiParameterFlt("cus_posi_a", 0.0f);
      AiParameterFlt("cus_posi_b", 1.0f);
      AiParameterFlt("cus_rand_int", 0.0f);
      AiParameterFlt("cus_hue_min", 0.0f);
      AiParameterFlt("cus_hue_max", 0.0f);
      AiParameterFlt("cus_sat_min", 0.0f);
      AiParameterFlt("cus_sat_max", 0.0f);
      AiParameterFlt("cus_val_min", 0.0f);
      AiParameterFlt("cus_val_max", 0.0f);
      AiParameterInt("cus_seed",0);

      AiParameterStr("debugMode","render");
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
   AtRGB intput = AiShaderEvalParamRGB(p_input);

   AtString face_shell = AiShaderEvalParamStr(p_face_shell);
   AtString plant_area = AiShaderEvalParamStr(p_plant_area);
   AtString plant_ao = AiShaderEvalParamStr(p_plant_ao);
   AtString plant_py = AiShaderEvalParamStr(p_plant_py);
   AtString plant_custom = AiShaderEvalParamStr(p_plant_custom);
   float plant_custom_default = AiShaderEvalParamFlt(p_plant_custom_default);

   float rand_intensity = AiShaderEvalParamFlt(p_rand_intensity);
   float rand_hue_min = AiShaderEvalParamFlt(p_rand_hue_min);
   float rand_hue_max = AiShaderEvalParamFlt(p_rand_hue_max);
   float rand_sat_min = AiShaderEvalParamFlt(p_rand_sat_min);
   float rand_sat_max = AiShaderEvalParamFlt(p_rand_sat_max);
   float rand_val_min = AiShaderEvalParamFlt(p_rand_val_min);
   float rand_val_max = AiShaderEvalParamFlt(p_rand_val_max);
   int rand_seed = AiShaderEvalParamInt(p_rand_seed);

   int py_enable = AiShaderEvalParamInt(p_py_enable);
   float py_posi_a = AiShaderEvalParamFlt(p_py_posi_a);
   float py_posi_b = AiShaderEvalParamFlt(p_py_posi_b);
   float py_rand_int = AiShaderEvalParamFlt(p_py_rand_int);
   float py_hue_min = AiShaderEvalParamFlt(p_py_hue_min);
   float py_hue_max = AiShaderEvalParamFlt(p_py_hue_max);
   float py_sat_min = AiShaderEvalParamFlt(p_py_sat_min);
   float py_sat_max = AiShaderEvalParamFlt(p_py_sat_max);
   float py_val_min = AiShaderEvalParamFlt(p_py_val_min);
   float py_val_max = AiShaderEvalParamFlt(p_py_val_max);
   int py_seed = AiShaderEvalParamInt(p_py_seed);

   int area_enable = AiShaderEvalParamInt(p_area_enable);
   float area_posi_a = AiShaderEvalParamFlt(p_area_posi_a);
   float area_posi_b = AiShaderEvalParamFlt(p_area_posi_b);
   float area_rand_int = AiShaderEvalParamFlt(p_area_rand_int);
   float area_hue_min = AiShaderEvalParamFlt(p_area_hue_min);
   float area_hue_max = AiShaderEvalParamFlt(p_area_hue_max);
   float area_sat_min = AiShaderEvalParamFlt(p_area_sat_min);
   float area_sat_max = AiShaderEvalParamFlt(p_area_sat_max);
   float area_val_min = AiShaderEvalParamFlt(p_area_val_min);
   float area_val_max = AiShaderEvalParamFlt(p_area_val_max);
   int area_seed = AiShaderEvalParamInt(p_area_seed);

   int ao_enable = AiShaderEvalParamInt(p_ao_enable);
   float ao_posi_a = AiShaderEvalParamFlt(p_ao_posi_a);
   float ao_posi_b = AiShaderEvalParamFlt(p_ao_posi_b);
   float ao_rand_int = AiShaderEvalParamFlt(p_ao_rand_int);
   float ao_hue_min = AiShaderEvalParamFlt(p_ao_hue_min);
   float ao_hue_max = AiShaderEvalParamFlt(p_ao_hue_max);
   float ao_sat_min = AiShaderEvalParamFlt(p_ao_sat_min);
   float ao_sat_max = AiShaderEvalParamFlt(p_ao_sat_max);
   float ao_val_min = AiShaderEvalParamFlt(p_ao_val_min);
   float ao_val_max = AiShaderEvalParamFlt(p_ao_val_max);
   int ao_seed = AiShaderEvalParamInt(p_ao_seed);

   int cus_enable = AiShaderEvalParamInt(p_cus_enable);
   float cus_posi_a = AiShaderEvalParamFlt(p_cus_posi_a);
   float cus_posi_b = AiShaderEvalParamFlt(p_cus_posi_b);
   float cus_rand_int = AiShaderEvalParamFlt(p_cus_rand_int);
   float cus_hue_min = AiShaderEvalParamFlt(p_cus_hue_min);
   float cus_hue_max = AiShaderEvalParamFlt(p_cus_hue_max);
   float cus_sat_min = AiShaderEvalParamFlt(p_cus_sat_min);
   float cus_sat_max = AiShaderEvalParamFlt(p_cus_sat_max);
   float cus_val_min = AiShaderEvalParamFlt(p_cus_val_min);
   float cus_val_max = AiShaderEvalParamFlt(p_cus_val_max);
   int cus_seed = AiShaderEvalParamInt(p_cus_seed);

   AtString debugMode = AiShaderEvalParamStr(p_debugMode);
   
   // Continue....







   AtRGB result = AtRGB(1.0f,1.0f,1.0f);

   sg->out.RGB() = result;

}


node_loader
{
   if (i > 0)
      return false;

   node->methods     = LcJitterColorMethods;
   node->output_type = AI_TYPE_RGB;
   node->name        = "lc_jitter_color";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}