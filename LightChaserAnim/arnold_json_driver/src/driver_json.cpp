#include <ai.h>
#include <ai_drivers.h>
#include <iostream> 
#include <string>
#include <vector>
#include <cstring>
#include <fstream>
#include <tr1/unordered_map>

// This driver will write to a file a list of all the objects in a pointer AOV
 
AI_DRIVER_NODE_EXPORT_METHODS(DriverJsonMtd);

namespace ASTR {
   const AtString name("name");
   const AtString filename("filename");
};

typedef struct 
{
   std::vector<AtString> name;
   std::vector<AtNode*> node;
}DriverData;



//node_parameters
static void Parameters(AtList* params, AtMetaDataStore* mds)
{
   AiParameterSTR(ASTR::filename, "objects.txt");
}

//node_loader
AI_EXPORT_LIB bool NodeLoader(int i, AtNodeLib* node)
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

//static void Initialize(AtNode* node, AtParamValue* params)
node_initialize
{
   DriverData* data = new DriverData();
   // initialize the driver
   AiDriverInitialize(node, false, data);
}

//node_update
static void Update(AtNode* node, AtParamValue* params)
{ }

//node_finish
static void Finish(AtNode* node)
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
{ // this driver is unusual and happens to do all the writing at the end, so this function is
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
   while (AiOutputIteratorGetNext(iterator, NULL, NULL, &bucket_data))
   {
      for (int y = 0; y < bucket_size_y; y++)
      {
         for (int x = 0; x < bucket_size_x; x++)
         {
            // Get source bucket coordinates for pixel
            int sidx = y * bucket_size_x + x;
            // Because of driver_supports_pixel_type, we know pixel is a
            // pointer to an AtNode.
            AtNode* pixel_node = ((AtNode **)bucket_data)[sidx];
            const AtString name = AiNodeGetStr(pixel_node, ASTR::name);
            data->name.push_back(name);
            data->node.push_back(pixel_node);
         }
      }
   }
}
 
//driver_close
static void DriverClose(AtNode* node, struct AtOutputIterator* iterator)
{
   DriverData *driver = (DriverData *)AiDriverGetLocalData(node);
   std::ofstream myfile(AtString("/home/xukai/objects.txt"));
   std::vector<AtString>   list = driver->name;
   //for (unsigned int i = 0;i < list.size();++i)
   //   myfile << i.first << ":\t " <<i.second << std::endl;
   myfile << "Hello Arnold Driver" << std::endl;
   std::cout << "@@@@ Json Driver @@@@" << std::endl;
   myfile.close();
}