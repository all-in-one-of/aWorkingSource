#include <ai.h>
 
#include "constants.h"

AI_SHADER_NODE_EXPORT_METHODS(SpeedTreeShaderMethods);

enum SpeedTreeParams 
{
    p_speedtree_roots = 0,
    p_speedtree_trunks,
    p_speedtree_branches,
    p_speedtree_leaves
};
 
node_parameters
{
   AiParameterClosure("speedtree_roots");
   AiParameterClosure("speedtree_trunks");
   AiParameterClosure("speedtree_branches");
   AiParameterClosure("speedtree_leaves");
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
        case P_SPEEDTREE_ROOTS:
            sg->out.CLOSURE() = AiShaderEvalParamClosure(p_speedtree_roots);
            break;
        case P_SPEEDTREE_TRUNKS:
            sg->out.CLOSURE() = AiShaderEvalParamClosure(p_speedtree_trunks);
            break;
        case P_SPEEDTREE_BRANCHES:
            sg->out.CLOSURE() = AiShaderEvalParamClosure(p_speedtree_branches);
            break;
        case P_SPEEDTREE_LEAVES:
            sg->out.CLOSURE() = AiShaderEvalParamClosure(p_speedtree_leaves);
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