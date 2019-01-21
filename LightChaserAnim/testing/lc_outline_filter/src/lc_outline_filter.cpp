#include <ai.h>
#include <string.h>
 
AI_FILTER_NODE_EXPORT_METHODS(CustomGaussianFilterMtd);
 
struct FilterData
{
   float width;
};
 
node_parameters
{
   AiParameterFlt("width", 2.0f);
}

node_initialize
{
   AiFilterInitialize(node, false, NULL);
   AiNodeSetLocalData(node, new FilterData());
}
 
node_update
{
   FilterData* filter_data = (FilterData*)AiNodeGetLocalData(node);
   filter_data->width = AiNodeGetFlt(node, "width");
   AiFilterUpdate(node, filter_data->width);
}

node_finish
{
   FilterData* filter_data = (FilterData*)AiNodeGetLocalData(node);
   delete filter_data;
}

filter_output_type
{
   switch (input_type)
   {
      case AI_TYPE_RGBA:
         return AI_TYPE_RGBA;
      default:
         return AI_TYPE_NONE;
   }
}

filter_pixel
{
/* static void FilterPixel(AtNode* node, 
*                          AtAOVSampleIterator* 
*                          iterator, 
*                          void* data_out, 
*                          uint8_t data_type)
*/
   FilterData* filter_data = (FilterData*)AiNodeGetLocalData(node);
   const float width = filter_data->width;

   float aweight = 0.0f;
   AtRGBA avalue = AI_RGBA_ZERO;
 
   while (AiAOVSampleIteratorGetNext(iterator))
   {
      // take into account adaptive sampling
      float inv_density = AiAOVSampleIteratorGetInvDensity(iterator);
      if (inv_density <= 0.f)
         continue;

      // determine distance to filter center
      const AtVector2& offset = AiAOVSampleIteratorGetOffset(iterator);
      const float r = AiSqr(2 / width) * (AiSqr(offset.x) + AiSqr(offset.y));
      if (r > 1.0f)
         continue;
 
      // gaussian filter weight
      const float weight = AiFastExp(2 * -r) * inv_density;
 
      // accumulate weights and colors
      avalue += weight * AiAOVSampleIteratorGetRGBA(iterator);
      aweight += weight;
   }

   // compute final filtered color
   if (aweight != 0.0f)
      avalue /= aweight;
   *((AtRGBA*)data_out) = avalue;
}

node_loader
{
   if (i>0)
      return false;
 
   node->methods      = CustomGaussianFilterMtd;
   node->output_type  = AI_TYPE_NONE;
   node->name         = "lc_outline_filter";
   node->node_type    = AI_NODE_FILTER;
   strcpy(node->version, AI_VERSION);
   return true;
}