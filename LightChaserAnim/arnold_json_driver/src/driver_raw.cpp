#include <ai.h>
 
AI_DRIVER_NODE_EXPORT_METHODS(DriverRAWMtd);
 
typedef struct {
   ...
} DriverRAWStruct;
 
node_loader
{
   if (i>0)
      return FALSE;
   node->methods = (AtNodeMethods*) DriverRAWMtd;
   node->output_type = AI_TYPE_NONE;
   node->name = "driver_raw";
   node->node_type = AI_NODE_DRIVER;
   strcpy(node->version, AI_VERSION);
   return TRUE;
}
 
node_parameters
{
   AiParameterSTR( "filename", "deep.raw");
}
 
node_initialize
{
   DriverRAWStruct *raw = new DriverRAWStruct();
 
   static const char *required_aovs[] = { "FLOAT Z", "FLOAT A", NULL };
 
   // Initialize the driver (set the required AOVs and indicate that
   // we want values at all the depths)
   AiRawDriverInitialize(node, required_aovs, true /* deep driver */, raw);
}
 
node_update
{
   ...
}
 
driver_supports_pixel_type
{
   // This function is not needed for raw drivers
   return TRUE;
}
 
driver_open
{
   ...
}
 
driver_extension
{
   static const char *extensions[] = { "raw", NULL };
   return extensions;
}
 
driver_needs_bucket
{
   return true;
}
 
driver_prepare_bucket
{
   ...
}
 
driver_process_bucket
{
   ...
}
 
driver_write_bucket
{
   DriverRAWStruct *raw = (DriverRAWStruct *)AiDriverGetLocalData(node);
   ...
 
   for (y = bucket_yo; y < bucket_yo + bucket_size_y; y++)
   {
      for (x = bucket_xo; x < bucket_xo + bucket_size_x; x++)
      {
         // Iterator for samples in this pixel
         AiAOVSampleIteratorInitPixel(sample_iterator, x, y);
         while (AiAOVSampleIteratorGetNext(sample_iterator))
         {
            AtPoint2 position = AiAOVSampleIteratorGetOffset(sample_iterator);
            while (AiAOVSampleIteratorGetNextDepth(sample_iterator))
            {
               float a = AiAOVSampleIteratorGetAOVFlt(sample_iterator,"A");
               float z = AiAOVSampleIteratorGetAOVFlt(sample_iterator,"Z");
               ...
            }
         }
      }
   }
   ...
}
 
driver_close
{
   ...
}
 
node_finish
{
   ...
   AiDriverDestroy(node);
}