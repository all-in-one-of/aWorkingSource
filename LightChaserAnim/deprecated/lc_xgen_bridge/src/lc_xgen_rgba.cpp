#include "shader_utils.h"

AI_SHADER_NODE_EXPORT_METHODS(LcXgenRGBAMethods);

enum Params { 
};

node_parameters
{
}

node_initialize
{
}

node_finish
{
}

node_update
{
}


shader_evaluate
{

   sg->out.RGB() = AtRGB(1.0f,1.0f,1.0f);

}
