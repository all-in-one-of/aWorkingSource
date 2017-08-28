#include <ai.h>
#include <ai_drivers.h>
#include <iostream> 
#include <string>
#include <vector>
#include <cstring>
#include <fstream>
#include <cstdio>

#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filewritestream.h"


AI_DRIVER_NODE_EXPORT_METHODS(DriverJsonMtd);

namespace ASTR {
   const AtString name("name");
   const AtString filename("filename");
};

typedef struct 
{
   std::vector<AtString> names;
   std::vector<AtNode*> nodes;
   std::vector<std::string> keys;
   int count;
}DriverData;



node_parameters
{
   AiParameterSTR(ASTR::filename, "objects.txt");
}

node_loader
{
   if (i>0)
      return false;
   node->methods = (AtNodeMethods*) DriverJsonMtd;
   node->output_type = AI_TYPE_NONE;
   node->name = "driver_json";
   node->node_type = AI_NODE_DRIVER;
   strcpy(node->version, AI_VERSION);
   return true;
}

node_initialize
{
   DriverData* data = new DriverData();
   // initialize the driver
   AiDriverInitialize(node, false, data);
}

node_update
{ }

node_finish
{
   // Free local data
   DriverData *data = (DriverData *)AiDriverGetLocalData(node);
   delete data;
   AiDriverDestroy(node);
}


//driver_needs_bucket
static bool DriverNeedsBucket(AtNode* node, 
   int bucket_xo, 
   int bucket_yo, 
   int bucket_size_x, 
   int bucket_size_y, 
   int tid)
{
   return true;
}

//driver_process_bucket
static void DriverProcessBucket(AtNode* node, 
   struct AtOutputIterator* iterator, 
   struct AtAOVSampleIterator* sample_iterator, 
   int bucket_xo, 
   int bucket_yo, 
   int bucket_size_x, 
   int bucket_size_y, 
   int tid)
{ }


//driver_supports_pixel_type
static bool DriverSupportsPixelType(const AtNode* node, AtByte pixel_type)
{
   // this driver will only support pointer formats
   return pixel_type == AI_TYPE_POINTER || pixel_type == AI_TYPE_NODE;
}
 
//driver_open
static void DriverOpen(AtNode* node, 
   struct AtOutputIterator* iterator, 
   AtBBox2 display_window, 
   AtBBox2 data_window, 
   int bucket_size)
{ 
   // this driver is unusual and happens to do all the writing at the end, so this function is
   // empty.
}
 
//driver_extension
static const char** DriverExtension()
{
   static const char *extensions[] = { "txt", NULL };
   return extensions;
}
 
//driver_prepare_bucket
static void DriverPrepareBucket(AtNode* node, int bucket_xo, int bucket_yo, int bucket_size_x, int bucket_size_y, int tid)
{ }

//driver_write_bucket 
static void DriverWriteBucket(AtNode* node, 
   struct AtOutputIterator* iterator, 
   struct AtAOVSampleIterator* sample_iterator, 
   int bucket_xo, 
   int bucket_yo, 
   int bucket_size_x, 
   int bucket_size_y)
{

   DriverData *data = (DriverData *)AiDriverGetLocalData(node);
   const void *bucket_data;
   // Iterate over all the AOVs hooked up to this driver
   data->count = 0;
   while (AiOutputIteratorGetNext(iterator, NULL, NULL, &bucket_data))
   {
      for (int y = 0; y < bucket_size_y; y++)
      {
         for (int x = 0; x < bucket_size_x; x++)
         {
            int sidx = y * bucket_size_x + x;

            AtNode* pixel_node = ((AtNode **)bucket_data)[sidx];
            const AtString name = AiNodeGetStr(pixel_node, ASTR::name);
            //const AtNodeEntry* pixel_node_entry = AiNodeGetNodeEntry(pixel_node);
            //AtString name_entry = AiNodeEntryGetNameAtString(pixel_node_entry);
            std::cout << name.c_str() << std::endl;
            data->names.push_back(name);
            data->nodes.push_back(pixel_node);
         }
      }
   }
}

//driver_close
static void DriverClose(AtNode* node, struct AtOutputIterator* iterator)
{
   DriverData *data = (DriverData *)AiDriverGetLocalData(node);
   std::ofstream myfile(AiNodeGetStr(node, ASTR::filename));
   std::cout << "=== Json Driver TEST===" << std::endl;
   std::vector<AtString> list_0 = data->names;
   //std::vector<AtNode*> list_1 = data->nodes;
   //std::vector<std::string> list_2 = data->keys;
   for (unsigned int i = 0;i < list_0.size();++i)
   {
      myfile << list_0[i] << std::endl;
   }
      
   myfile.close();
   std::cout << data->count << std::endl;
   std::cout << "=== Json Driver END===" << std::endl;
   
}