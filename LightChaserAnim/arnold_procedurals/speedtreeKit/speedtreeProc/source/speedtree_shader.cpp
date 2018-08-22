#include <ai.h>
 
#include "constants.h"

using namespace std;

AI_SHADER_NODE_EXPORT_METHODS(SpeedTreeShaderMethods);

enum SpeedTreeParams 
{
    p_branches = 0,
    p_frond_and_caps,
    p_leaves,
    p_facing_leaves,
    p_rigid_meshes,
    p_grass,
    p_billboard,
};

node_parameters
{
   AiParameterClosure("branches");
   AiParameterClosure("frond_and_caps");
   AiParameterClosure("leaves");
   AiParameterClosure("facing_leaves");
   AiParameterClosure("rigid_meshes");
   AiParameterClosure("grass");
   AiParameterClosure("billboard");
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
    AtString attribute = AtString(SPEEDTREE_SHADER);
    int shader_value;
    if (AiUDataGetInt(attribute, shader_value)){
        switch (shader_value)
        {
        case ST_BRANCHES:
            sg->out.CLOSURE() = AiShaderEvalParamClosure(p_branches);
            break;
        case ST_FRONDANDCAPS:
            sg->out.CLOSURE() = AiShaderEvalParamClosure(p_frond_and_caps);
            break;
        case ST_LEAVES:
            sg->out.CLOSURE() = AiShaderEvalParamClosure(p_leaves);
            break;
        case ST_FACINGLEAVES:
            sg->out.CLOSURE() = AiShaderEvalParamClosure(p_facing_leaves);
            break;
        case ST_RIGIDMESHES:
            sg->out.CLOSURE() = AiShaderEvalParamClosure(p_rigid_meshes);
            break;
        case ST_GRASS:
            sg->out.CLOSURE() = AiShaderEvalParamClosure(p_grass);
            break;
        case ST_BILLBOARD:
            sg->out.CLOSURE() = AiShaderEvalParamClosure(p_billboard);
            break;
        default:
            sg->out.CLOSURE() = AiClosureMatte(sg,AI_RGB_WHITE);
            break;
        }
    }
    else
        sg->out.CLOSURE() = AiClosureMatte(sg,AI_RGB_WHITE);
}

node_loader
{
   if (i > 0)
      return false;
   node->methods     = SpeedTreeShaderMethods;
   node->output_type = AI_TYPE_CLOSURE;
   node->name        = "speedtree_shader";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}