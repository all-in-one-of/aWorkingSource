#include <ai.h>
#include <cstring>
 
extern AtNodeMethods* UserDataFloatMethods;
extern AtNodeMethods* UserDataIntMethods;
extern AtNodeMethods* UserDataStringMethods;

enum{
    UserDataFloat = 0,
    UserDataInt,
    UserDataString,
};
 
node_loader
{
    switch (i)
    {
    case UserDataFloat:
        node->methods = UserDataFloatMethods;
        node->output_type = AI_TYPE_FLOAT;
        node->name = "lc_user_data_float";
        node->node_type = AI_NODE_SHADER;
        break;
 
    case UserDataInt:
        node->methods = UserDataIntMethods;
        node->output_type = AI_TYPE_INT;
        node->name = "lc_user_data_int";
        node->node_type = AI_NODE_SHADER;
        break;
 
    case UserDataString:
        node->methods = UserDataStringMethods;
        node->output_type = AI_TYPE_STRING;
        node->name = "lc_user_data_string";
        node->node_type = AI_NODE_SHADER;
        break;
    default:
        return false;
    }
 
    strcpy(node->version, AI_VERSION);
    return true;
}