#include <ai.h>

#include "alUtil.h"

AI_SHADER_NODE_EXPORT_METHODS(LcJitterColorMethods);


enum DebugModes { kRender = 0, kRand, kPY, kArea, kAO, kCus };

static const char* debugModeNames[] = {"render", "rand", "py", "area", "ao", "cus", NULL};


AtVector compute_rand(float data,
               float intensity,
               float h_min,
               float h_max,
               float s_min,
               float s_max,
               float v_min,
               float v_max,
               int seed,
               int seed_global)
{
   AtVector resout = AtVector(0,0,0);
   float h_int = 0;
   float h_value = 0;
   float s_int = 0;
   float s_value = 0;
   float v_int = 0;
   float v_value = 0;

   float signal = data+seed+seed_global;
   h_int = AiCellNoise2(AtVector2(signal, 51731.132151));
   h_value = lerp(h_min,h_max,h_int);

   s_int = AiCellNoise2(AtVector2(signal, 173.1231));
   s_value = lerp(s_min,s_max,s_int);

   v_int = AiCellNoise2(AtVector2(signal, 413.7254));
   v_value = lerp(v_min,v_max,v_int);

   //  // float saturation = fmodf((signal + 0.9311) * 51731.13215, 1.f);
   //  float saturation = AiCellNoise2(AtVector2(signal, 51731.132151));
   //  saturation = lerp(minSat, maxSat, saturation);
   //  if (saturation != 1.0f) {
   //      float l = luminance(result);
   //      result = lerp(rgb(l), result, saturation);
   //  }

   //  // hue
   //  // float hueOffset = fmodf((signal + 1.3311) * 173.1231, 1.f);
   //  float hueOffset = AiCellNoise2(AtVector2(signal, 173.1231));
   //  hueOffset = lerp(minHue, maxHue, hueOffset);
   //  if (hueOffset != 0.0f) {
   //      AtRGB hsv = rgb2hsv(result);
   //      hsv.r += hueOffset;
   //      result = hsv2rgb(hsv);
   //  }

   //  // float gain = fmodf((signal + 0.65416) * 413.7254, 1.f);
   //  float gain = AiCellNoise2(AtVector2(signal, 413.7254));
   //  gain = lerp(minGain, maxGain, gain);
   //  result *= gain;


   // h_int = noise("cell",(data+seed+seed_global+2563));
   // h_value = lerp(h_min,h_max,h_int);

   // s_int = noise("cell",(data+seed+seed_global+2014));
   // s_value = lerp(s_min,s_max,s_int);

   // v_int = noise("cell",(data+seed+seed_global+3321));
   // v_value = lerp(v_min,v_max,v_int);
   return resout = AtVector(h_value,s_value,v_value)*intensity;
}

float data_Ramp(float positions[],
            int lca_face_shell,
            float ramp_rand_int,
            float data,
            int seed,
            int seed_global)
{  
   float resout = 0.0;
   float int_rand = lerp(1,AiCellNoise2(AtVector2(lca_face_shell+seed+seed_global, 8740)),ramp_rand_int);
   float value = (data-positions[0])/(positions[1]-positions[0]);
   resout = clamp(float(value), 0.0f, 1.0f)*int_rand;
   return resout;
}

AtVector value_byRange(float intensity,
               float h_min,
               float h_max,
               float s_min,
               float s_max,
               float v_min,
               float v_max)
{
   AtVector resout = AtVector(0, 0, 0);
   resout[0] = lerp(h_min,h_max,intensity);
   resout[1] = lerp(s_min,s_max,intensity);
   resout[2] = lerp(v_min,v_max,intensity);
   return resout;
}


enum Params
{
   p_input,
   p_debugMode ,

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

};

node_parameters
{
      AiParameterRGB("input", 1.0f,1.0f,1.0f);
      AiParameterEnum("debugMode", kRender, debugModeNames);

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
      AiParameterInt("area_seed",0);

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

      // AiParameterStr("debugMode","render");
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
   AtRGB input = AiShaderEvalParamRGB(p_input);

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

   int debugMode = AiShaderEvalParamEnum(p_debugMode);
   // Continue....
   int lca_face_shell = 0;
   float plant_py_data = 0.0f;
   float plant_area_data = 0.0f;
   float plant_ao_data = 0.0f;
   float plant_custom_data = 0.0f;
   AtRGB tmp = rgb2hsv(input);
   AtRGB outColor = AtRGB(1.0f,0.0f,0.0f);

   // get user data
   AiUDataGetInt(face_shell,lca_face_shell);
   AiUDataGetFlt(plant_py,plant_py_data);
   AiUDataGetFlt(plant_area,plant_area_data);
   AiUDataGetFlt(plant_ao,plant_ao_data);

   bool arg = AiUDataGetFlt(plant_custom,plant_custom_data);
   if (!arg)
      plant_custom_data = plant_custom_default;

   //rand via face shell attribute
   float rand_int = ((lca_face_shell+rand_seed)%100 < rand_intensity*100)?0.0:1.0;
   AtVector rand_resout = compute_rand(lca_face_shell,rand_int,rand_hue_min,rand_hue_max,rand_sat_min,rand_sat_max,rand_val_min,rand_val_max,3521,rand_seed);

   //rand via position Y
   float py_positions[2] = {py_posi_a,py_posi_b};
   float py_int = data_Ramp(py_positions,lca_face_shell,py_rand_int,plant_py_data,py_seed,rand_seed);
   AtVector py_resout = value_byRange(py_int,py_hue_min,py_hue_max,py_sat_min,py_sat_max,py_val_min,py_val_max);

   //rand via area
   float area_positions[2] = {area_posi_a,area_posi_b};
   float area_int = data_Ramp(area_positions,lca_face_shell,area_rand_int,plant_area_data,area_seed,rand_seed);
   AtVector area_resout = value_byRange(area_int,area_hue_min,area_hue_max,area_sat_min,area_sat_max,area_val_min,area_val_max);

   //rand via ao
   float ao_positions[2] = {ao_posi_a,ao_posi_b};
   float ao_int = data_Ramp(ao_positions,lca_face_shell,ao_rand_int,plant_ao_data,ao_seed,rand_seed);
   AtVector ao_resout = value_byRange(ao_int,ao_hue_min,ao_hue_max,ao_sat_min,ao_sat_max,ao_val_min,ao_val_max);

   //rand via custom attribute
   float cus_positions[2] = {cus_posi_a,cus_posi_b};
   float cus_int = data_Ramp(cus_positions,lca_face_shell,cus_rand_int,plant_custom_data,cus_seed,rand_seed);
   AtVector cus_resout = value_byRange(cus_int,cus_hue_min,cus_hue_max,cus_sat_min,cus_sat_max,cus_val_min,cus_val_max);

   tmp[0] = tmp[0]+rand_resout[0]+py_resout[0]*py_enable+area_resout[0]*area_enable+ao_resout[0]*ao_enable+cus_resout[0]*cus_enable;
   tmp[1] = tmp[1]+rand_resout[1]+py_resout[1]*py_enable+area_resout[1]*area_enable+ao_resout[1]*ao_enable+cus_resout[1]*cus_enable;
   tmp[2] = tmp[2]+rand_resout[2]+py_resout[2]*py_enable+area_resout[2]*area_enable+ao_resout[2]*ao_enable+cus_resout[2]*cus_enable;

   switch(debugMode)
   {
      case kRender:
      {
         outColor = clamp(hsv2rgb(tmp),AtRGB(0.0f),AtRGB(1.0f));
         break;
      }
      case kRand:
      {
          outColor = AtRGB(rand_int);
          break;
      }
      case kPY:
      {
          outColor = AtRGB(py_int);
          break;
      }
      case kArea:
      {
          outColor = AtRGB(area_int);
          break;
      }
      case kAO:
      {
          outColor = AtRGB(ao_int);
          break;
      }
      case kCus:
      {
          outColor = AtRGB(cus_int);
          break;
      }
      default:
      {
          outColor = AtRGB(1.0f, 1.0f, 1.0f);
          break;
      }
   }

   //outColor = clamp(hsv2rgb(tmp),AtRGB(0.0f),AtRGB(1.0f));

   sg->out.RGB() = outColor;
// enum DebugModes { kRender = 0, kRand, kPY, kArea, kAO, kCus};

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