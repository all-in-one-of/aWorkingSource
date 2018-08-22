#include <ai.h>
#include <cstring>
#include <cstdlib>
#include <vector>

#include "constants.h"
#include "geometry.h"

using namespace std;

AI_PROCEDURAL_NODE_EXPORT_METHODS(SpeedTreeMethods);


bool SortFloatArray(AtArray* array, vector<float> & cvector){
   unsigned int nElements = (array) ? AiArrayGetNumElements(array) : 0;
   if (nElements > 0)
   {
      for (unsigned int i; i < nElements; ++i)
      {
         cvector.push_back(AiArrayGetFlt(array, i));      
      }
      return true;
   }
   else
   {
      return false;
   }
}

bool SortVec3Array(AtArray* array, vector<Vec3> & cvector){
   unsigned int nElements = (array) ? AiArrayGetNumElements(array) : 0;
   if (nElements > 0)
   {
      for (unsigned int i; i < nElements; ++i)
      {
            AtVector tempVec = AiArrayGetVec(array, i);
            cvector.push_back(Vec3(tempVec.x, tempVec.y, tempVec.z));
      }
   }
   else
   {
      return false;  
   }
}


node_parameters
{
   AiParameterStr("srtFile", "");

   AiParameterInt("useGlobalMotion", 1);
   AiParameterFlt("currentFrame", 1001.0f);
   AiParameterInt("enableMotionBlur", 0);
   AiParameterArray("motionSamples", AiArray(-0.25f, 0.25f, AI_TYPE_FLOAT));

   AiParameterFlt("fps", 24.0);
   AiParameterFlt("globalFrequency", 0.0);
   AiParameterFlt("gustFrequency", 0.0);
   AiParameterFlt("windSpeed", 0.0);

   AiParameterFlt("windDirection_x", 1.0);
   AiParameterFlt("windDirection_y", 0.0);
   AiParameterFlt("windDirection_z", 0.0);

   AiParameterInt("windType", 6);
   AiParameterInt("LODType", 0);
   AiParameterInt("LODSmoothType", 0);

   AiParameterArray("speedKeyFrame", AiArray(1, 1, AI_TYPE_FLOAT, 1.0f));
   AiParameterArray("speedResponseTime", AiArray(1, 1, AI_TYPE_FLOAT, 1.0f));
   AiParameterArray("speedKeyValue", AiArray(1, 1, AI_TYPE_FLOAT, 0.3f));
   AiParameterArray("direKeyFrame", AiArray(1, 1, AI_TYPE_FLOAT, 1.0f));
   AiParameterArray("direResponseTime", AiArray(1, 1, AI_TYPE_FLOAT, 1.0f));
   AiParameterArray("direKeyValue", AiArray(1, 1, AI_TYPE_VECTOR, 1.0f, 0.0f, 0.0f));
}

procedural_init
{
   SpeedTreeData *proc_data = new SpeedTreeData();
   *user_ptr = proc_data;
   proc_data -> srtFile = AiNodeGetStr(node, "srtFile");

   proc_data->useGlobalMotion = (bool)AiNodeGetInt(node, "useGlobalMotion");
   proc_data -> currentFrame = AiNodeGetFlt(node, "currentFrame");
   proc_data->enableMotionBlur = (bool)AiNodeGetInt(node, "enableMotionBlur");
   proc_data -> motionSamples = AiNodeGetArray(node, "motionSamples");
   proc_data->GlobalFrequency = AiNodeGetFlt(node, "globalFrequency");
   proc_data->GustFrequency = AiNodeGetFlt(node, "gustFrequency");
   proc_data->WindSpeed = AiNodeGetFlt(node, "windSpeed");

   proc_data->WindDirection = Vec3(
      AiNodeGetFlt(node, "windDirection_x"),
      AiNodeGetFlt(node, "windDirection_y"),
      AiNodeGetFlt(node, "windDirection_z"));

   proc_data->WindType = AiNodeGetInt(node, "windType");
   proc_data->LODType = AiNodeGetInt(node, "LODType");
   proc_data->LODSmoothType = AiNodeGetInt(node, "LODSmoothType");

   AtArray* speedKeyFrame = AiNodeGetArray(node, "speedKeyFrame");
   SortFloatArray(speedKeyFrame, proc_data->SpeedKeyFrames);

   AtArray* SpeedResponseTimes = AiNodeGetArray(node, "speedResponseTime");
   SortFloatArray(SpeedResponseTimes, proc_data->SpeedResponseTimes);

   AtArray* SpeedKeyValues = AiNodeGetArray(node, "speedKeyValue");
   SortFloatArray(SpeedKeyValues, proc_data->SpeedKeyValues);

   AtArray* DireKeyFrames = AiNodeGetArray(node, "direKeyFrame");
   SortFloatArray(DireKeyFrames, proc_data->DireKeyFrames);

   AtArray* DireResponseTimes = AiNodeGetArray(node, "direResponseTime");
   SortFloatArray(DireResponseTimes, proc_data->DireResponseTimes);

   AtArray* DireKeyValues = AiNodeGetArray(node, "direKeyValue");
   SortVec3Array(DireKeyValues, proc_data->DireKeyValues);

   proc_data -> id = 0;

   AiMsgInfo("[speedtree] Reading srt file : %s", proc_data -> srtFile.c_str());

   // get current frame & fps, the global render settings are stored in the options node,(Maya)
   AtNode* options = AiUniverseGetOptions();
   if (proc_data->useGlobalMotion)
   {
      proc_data->frame = AiNodeGetFlt(options, "frame");
      AiMsgInfo("[speedtree] \t enable global motion frame!");
   }
   else
   {
      proc_data->frame = AiNodeGetFlt(node, "currentFrame");
      AiMsgInfo("[speedtree] \t using currentFrame as option frame!");
   }
   if (proc_data->enableMotionBlur)
      AiMsgInfo("[speedtree] \t enable motion blur!");
   else
      AiMsgInfo("[speedtree] \t disable motion blur!");
   
   proc_data->fps = AiNodeGetFlt(node, "fps");

   AiMsgInfo("[speedtree] \t frame: %f", proc_data->frame);
   AiMsgInfo("[speedtree] \t fps: %f", proc_data->fps);
   
   *(proc_data->sLod) = proc_data->readSTS->readSRT( proc_data->srtFile.c_str(), 
                                                       proc_data->WindSpeed, 
                                                       proc_data->GustFrequency,
                                                       proc_data->WindDirection,
                                                       true,
                                                       proc_data->LODType);

   proc_data->readSTS->setKeyframes( proc_data->SpeedKeyFrames, 
                                       proc_data->SpeedResponseTimes, 
                                       proc_data->SpeedKeyValues,
                                       proc_data->DireKeyFrames, 
                                       proc_data->DireResponseTimes, 
                                       proc_data->DireKeyValues);
   bool state = proc_data->readSTS->returnState();
   if (state)
      AiMsgInfo("[speedtree] Successed!");
   else
      AiMsgError("[speedtree] Failed!");
   proc_data -> sum = (int)proc_data->readSTS->meshCount;

   return true;
}

procedural_cleanup
{
   SpeedTreeData *proc_data = (SpeedTreeData*)user_ptr;
   delete proc_data; 
   return true;
}

procedural_num_nodes
{
   SpeedTreeData *proc_data = (SpeedTreeData*)user_ptr;
   return proc_data->sum;
}

procedural_get_node
{
   SpeedTreeData *proc_data = (SpeedTreeData*)user_ptr;

   float* windShaderValues = proc_data->readSTS->readWindData(proc_data->frame, 1.0, proc_data->fps);
   proc_data->speedtreeData = proc_data->readSTS->assignWindData(windShaderValues, proc_data->frame);
   if (proc_data->enableMotionBlur)
   {
     float motion_end = AiArrayGetFlt(proc_data -> motionSamples, AiArrayGetNumElements(proc_data -> motionSamples)-1);
     float* motionWindShaderValues = proc_data->readSTS->readMotionWindData(proc_data->frame, motion_end, proc_data->fps);
     proc_data->speedtreeMotionData = proc_data->readSTS->assignWindData(motionWindShaderValues, proc_data->frame + motion_end);
   }

   AtNode* polymesh = createNode(node, proc_data);
   return polymesh;
}

node_loader
{
   if (i>0)
      return false;

   node->methods      = SpeedTreeMethods;
   node->output_type  = AI_TYPE_NONE;
   node->name         = "speedtree_procedural";
   node->node_type    = AI_NODE_SHAPE_PROCEDURAL;
   strcpy(node->version, AI_VERSION);

   return true;
}