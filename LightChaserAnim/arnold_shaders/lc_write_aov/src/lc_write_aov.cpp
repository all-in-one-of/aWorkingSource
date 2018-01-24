#include <ai.h>


AI_SHADER_NODE_EXPORT_METHODS(LcWriteAovMtd);


struct WriteAovData
{
   AtString aov_name;
   bool blend_opacity;
};


enum Params 
{
   p_passthrough = 0,
   p_aov_input,
   p_aov_name,
   p_blend_opacity,
};

node_parameters
{
   AiParameterClosure("passthrough");
   AiParameterRGB("aov_input", 1.0f,1.0f,1.0f);
   AiParameterStr("aov_name", "");
   AiParameterBool("blend_opacity",true);
}

node_initialize
{
   AiNodeSetLocalData(node, new WriteAovData());
}

node_update
{
   // register AOV
   WriteAovData *data = (WriteAovData*)AiNodeGetLocalData(node);
   data->aov_name = AiNodeGetStr(node, "aov_name");
   data->blend_opacity = AiNodeGetBool(node, "blend_opacity");
   if(data->blend_opacity)
      AiAOVRegister(data->aov_name, AI_TYPE_RGB, AI_AOV_BLEND_OPACITY );
   else
      AiAOVRegister(data->aov_name, AI_TYPE_RGB, AI_AOV_BLEND_NONE);
}

node_finish
{
   WriteAovData *data = (WriteAovData*)AiNodeGetLocalData(node);
   delete data;
}

shader_evaluate
{
   const WriteAovData *data = (WriteAovData*)AiNodeGetLocalData(node);
   AtRGB aov_input = AiShaderEvalParamRGB(p_aov_input);

   // write AOV only if in use
   if ((sg->Rt & AI_RAY_CAMERA) && AiAOVEnabled(data->aov_name, AI_TYPE_RGB))
      AiAOVSetRGB(sg, data->aov_name, aov_input);
   sg->out.CLOSURE() = AiShaderEvalParamClosure(p_passthrough);
}

node_loader
{
   if (i > 0)
     return false;
   node->methods     = LcWriteAovMtd;
   node->output_type = AI_TYPE_CLOSURE;
   node->name        = "lc_write_aov";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}