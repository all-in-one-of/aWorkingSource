#include "shader_utils.h"

AI_SHADER_NODE_EXPORT_METHODS(LcXgenRGBMethods);

enum Params {
   p_passthrough,
   p_shader_paremeter, 
};

node_parameters
{
   AiParameterRGB("passthrough", 1.0f,1.0f,1.0f);   
   AiParameterStr("shader_paremeter", "");
}

node_initialize
{
/*   ShaderData *data = new ShaderData;
   AiNodeSetLocalData(node, data);
*/}

node_finish
{
/*   ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);
   data->ktData.clear();
   std::vector<cell>().swap(data->ktData);
   delete data;
*/}

node_update
{
/*   bool is_override = false;
   std::string config_path;
   std::string xgen_code;
   std::vector<cell> cell_data;
   if(const char* env_p = std::getenv("XGEN_CODE"))
      xgen_code = env_p;
   if(!xgen_code.empty())
   {
      is_override = true;
      // config_path debug
      //config_path = "/home/xukai/Documents/TestProgramFactory/lc_xgen_bridge/src/";
      config_path = xgen_code + "kt_xgen/";
   }
   float my_float[10] = {1.0,0.0,0.0,0.5,0.5,0.5,0.5,0.5,0.5,0.5};
   cell my_cell(0,my_float);
   cell_data.push_back(my_cell);
   //tinyxml2::XMLDocument doc;
   //doc.LoadFile( "/home/xukai/Documents/tinyxml2-5.0.1/resources/dream.xml" );
   AiMsgInfo("YOUR XGEN_CODE IS: [%s]",config_path.c_str());
   ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);
   data->ktOverride = is_override;
   data->ktConfigParh = config_path;
   data->ktParameter = AiNodeGetStr(node, "shader_paremeter");
   data->ktData = cell_data;
*/}


shader_evaluate
{
   //ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);
   AtRGB passthrough = AiShaderEvalParamRGB(p_passthrough);
   AtString shader_parameter = AiShaderEvalParamStr(p_shader_paremeter); 
   AtRGB result = AtRGB(1.0f,1.0f,1.0f);
   std::string file_name = "shader_utils.h";
   std::string file_path;
/*   if(data->ktOverride)
   {
      float faceid;
      std::string config_path = data->ktConfigParh;
      if(AiUDataGetFlt(AtString("xgen_faceid"),faceid))
         {
            for(std::vector<cell>::iterator it = data->ktData.begin() ; it != data->ktData.end(); ++it)
            {
               cell temp_cell = *it;
               if(faceid == temp_cell.getID())
               {
                  float *temp_data = temp_cell.getData();
                  result = AtRGB(temp_data[0],temp_data[1],temp_data[2]);
                  delete temp_data;      
               }
            }
         }
      else
         result = passthrough;
   }
   else
      result = passthrough;*/
   
   result = passthrough;
   sg->out.RGB() = result;

}
