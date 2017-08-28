#include <ai.h>
#include <cstring>
 
extern AtNodeMethods* DriverJsonMtd;
extern AtNodeMethods* JsonFilterMtd;
 
enum{
    DriverJson = 0,
    JsonFilter
};
 
node_loader
{
    switch (i)
    {
    case DriverJson:
        node->methods = (AtNodeMethods*) DriverJsonMtd;
        node->output_type = AI_TYPE_NONE;
        node->name = "driver_json";
        node->node_type = AI_NODE_DRIVER;
        break;
    case JsonFilter:
        node->methods      = JsonFilterMtd;
        node->output_type  = AI_TYPE_NONE;
        node->name         = "json_filter";
        node->node_type    = AI_NODE_FILTER;
        break;
 
    default:
        return false;
    }
 
    strcpy(node->version, AI_VERSION);
    return true;
}