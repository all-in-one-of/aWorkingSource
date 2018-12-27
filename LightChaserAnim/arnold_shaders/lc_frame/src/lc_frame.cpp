#include <ai.h>

#include <stdio.h>

AI_SHADER_NODE_EXPORT_METHODS(LcFrameMtd);

enum Params
{
   p_offset = 0,
   p_mutiply
};

struct ShaderData
{
    float frame;
    float offset;
    float mutiply;
};

node_parameters
{
   AiParameterFlt("offset", 0.0f);
   AiParameterFlt("mutiply", 1.0f);
}

node_initialize
{
   ShaderData *data = new ShaderData;

   AtNodeIterator* iter = AiUniverseGetNodeIterator(AI_NODE_ALL);
   while (!AiNodeIteratorFinished(iter))
   {
      AtNode* node = AiNodeIteratorGetNext(iter);
      // const char *name = AiNodeGetStr(node, "name");
      // if (AiNodeGetNodeEntry(node) == mytype && AiNodeGetPtr(node, "camera") != cam)
      // {
      //     //AiNodeSetInt(node, "visibility", 0);
      // }
      // if (node != NULL)
      //    printf("%s\n", "Yes");   
      // else
      //    printf("%s\n", "Noo");
   }

    // AiNodeIteratorDestroy(iter);



   AiNodeSetLocalData(node, data);
}

node_update
{
   ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);
   // get current frame & fps, the global render settings are stored in the options node,(Maya)
   // AtNode* options = AiUniverseGetOptions();
   // printf("%s\n", "@HI THERE");
   // if (options == NULL)
   //    printf("%s\n", "Yes");   
   // else
   //    printf("%s\n", "Noo");


   // printf("%s\n", "@BYE THERE");
   data->frame = 0.1f;
   data->offset = AiNodeGetFlt(node, "offset");
   data->mutiply = AiNodeGetFlt(node, "mutiply");
}

node_finish
{
   ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);
   delete data;
}

shader_evaluate
{
   ShaderData *data = (ShaderData*)AiNodeGetLocalData(node);
   sg->out.FLT() = (data->frame + data->offset)*data->mutiply;
}

node_loader
{
   if (i > 0)
     return false;
   node->methods     = LcFrameMtd;
   node->output_type = AI_TYPE_FLOAT;
   node->name        = "lc_frame";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}