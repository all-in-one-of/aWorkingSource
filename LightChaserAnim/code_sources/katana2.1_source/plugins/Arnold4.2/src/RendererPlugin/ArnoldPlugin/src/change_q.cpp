// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include "change_q.h"

#include <iostream>
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pystring/pystring.h>
#include <buildMaterial.h>

using namespace FnKat;

class KatChangeList 
{
private:
   AtCritSec csection;
   std::queue<KatParamChange>  myChangeList;

public:
   KatChangeList() : myChangeList() 
   { 
      AiCritSecInit(&csection); 
   };

   ~KatChangeList() 
   { 
      AiCritSecClose(&csection); 
   };

   void push_back(KatParamChange &change) 
   { 
      AiCritSecEnter(&csection);
      myChangeList.push(change); 
      AiCritSecLeave(&csection);
   };

   bool           FindChange(const char*);
   KatParamChange GetChange(const char*);
   void           ApplyChanges(FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState);

   bool empty() 
   {
      AiCritSecEnter(&csection);
      bool empty =  myChangeList.empty(); 
      AiCritSecLeave(&csection);
      return empty ? true : false; 
   };

   void Clear();
};

KatParamChange KatChangeList::GetChange(const char* paramname)
{
   AiCritSecEnter(&csection);

   KatParamChange returnChange;
   bool foundIt = false;

   int num = myChangeList.size();
   while (num--)
   {
      KatParamChange change = myChangeList.front();
      myChangeList.pop();
      myChangeList.push(change);

      if (!strcmp(change.param_name,paramname))
      {
         foundIt = true;
         returnChange = change;
      }
   }
   AiCritSecLeave(&csection);

   if (foundIt)
   {
      return returnChange;
   }

   throw -1;
}

bool KatChangeList::FindChange(const char* paramname)
{
   try 
   {
      KatParamChange change = GetChange(paramname);
      return true;
   } 
   catch (...)
   {
      return false;
   }

   return false;
}

void KatChangeList::ApplyChanges(FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState)
{
   AiCritSecEnter(&csection);

   while (myChangeList.size())
   {
      KatParamChange change = myChangeList.front();
      myChangeList.pop();

      if (change.attr_data.isValid())
      {
        (*change.attr_callback)(change.node, change.attr_data, sharedState);
      }
      else if (change.array_type)
      {
         if (change.param_type == AI_TYPE_POINTER)
         {
             AtArray* array = change.param_value.ARRAY;
             std::vector<void*> ptr;

             for(unsigned int i = 0; i < array->nelements; i++)
             {
                 std::string loc(((char**)array->data)[i]);
                 if (loc.empty() ||
                      loc.find("UNIMPLEMENTED PARAM TYPE") != std::string::npos)  continue;
                 AtNode* shaderNode = NULL;
                 shaderNode = getShaderByPath( loc, rootIterator, NULL, true, sharedState, "arnoldSurface");

                 if(shaderNode) 
                     ptr.push_back(shaderNode);
                 else
                 {
                     AtNode* node = AiNodeLookUpByName(loc.c_str());
                     if (node) 
                        ptr.push_back(node);
                     else
                     {
                        std::cout << AiNodeGetStr(change.node,"name");
                        std::cout << ": Cannot find node " << loc << " ignored." << std::endl;
                     }
                 }
              }
              AiArrayDestroy(array);
              change.param_value.ARRAY = AiArrayConvert(ptr.size(), 1, AI_TYPE_POINTER, &ptr[0]);
         }

         AiNodeSetArray(change.node, change.param_name, change.param_value.ARRAY);
      }
      else
      {
         switch(change.param_type)
         {
            case AI_TYPE_BOOLEAN:
               AiNodeSetBool(change.node, change.param_name, change.param_value.BOOL);
               break;
            case AI_TYPE_BYTE:
               AiNodeSetByte(change.node, change.param_name, change.param_value.BYTE);
               break;
            case AI_TYPE_ENUM:
            case AI_TYPE_INT:
               AiNodeSetInt(change.node,  change.param_name, change.param_value.INT);
               break;
            case AI_TYPE_UINT:
               AiNodeSetUInt(change.node, change.param_name, change.param_value.UINT);
               break;
            case AI_TYPE_FLOAT:
               AiNodeSetFlt(change.node,  change.param_name, change.param_value.FLT);
               break;
            case AI_TYPE_RGB:
               AiNodeSetRGB(change.node,  change.param_name, 
                            change.param_value.RGB.r, change.param_value.RGB.g, change.param_value.RGB.b);
               break;
            case AI_TYPE_RGBA:
               AiNodeSetRGBA(change.node,  change.param_name, 
                             change.param_value.RGBA.r, change.param_value.RGBA.g, change.param_value.RGBA.b,change.param_value.RGBA.a);
               break;
            case AI_TYPE_VECTOR:
               AiNodeSetVec(change.node,  change.param_name, 
                            change.param_value.VEC.x, change.param_value.VEC.y, change.param_value.VEC.z);
               break;
            case AI_TYPE_POINT:
               AiNodeSetPnt(change.node,  change.param_name, 
                            change.param_value.PNT.x, change.param_value.PNT.y, change.param_value.PNT.z);
               break;
            case AI_TYPE_POINT2:
               AiNodeSetPnt2(change.node,  change.param_name, 
                             change.param_value.PNT2.x, change.param_value.PNT2.y);
               break;
            case AI_TYPE_STRING:
               AiNodeSetStr(change.node,  change.param_name, change.param_value.STR);
               // If the value was set to be the default then its param_value
               // will be the one returned by Arnold as the parameter default
               // value. In that case this should not be deleted.
               if (!change.is_default)
               {
                   free((void *)change.param_value.STR);
               }
               break;
            case AI_TYPE_MATRIX:
                AiNodeSetMatrix(change.node, change.param_name, *change.param_value.pMTX);
                // If the value was set to be the default then its param_value
                // will be the one returned by Arnold as the parameter default
                // value. In that case this should not be deleted.
                if(!change.is_default)
                {
                    free((void *)change.param_value.pMTX);
                }
                break;
            case AI_TYPE_POINTER:
            {
               std::string loc(change.param_value.STR);
               std::vector<std::string> optionparams;
               pystring::split(loc, optionparams, ",");

               for (std::vector<std::string>::iterator it = optionparams.begin();
                    it != optionparams.end(); ++it)
               {
                  if ((*it).empty() ||
                      (*it).find("UNIMPLEMENTED PARAM TYPE") != std::string::npos)  continue;
                  std::string path = (*it);
                  AtNode* shaderNode = NULL;
                  shaderNode = getShaderByPath(path, rootIterator, NULL, true, sharedState, "arnoldSurface");

                  if(shaderNode) 
                     AiNodeSetPtr(change.node,  change.param_name, shaderNode);
                  else
                  {
                     AtNode *node = AiNodeLookUpByName(path.c_str());
                     if (node)
                        AiNodeSetPtr(change.node,  change.param_name, node);
                     else
                     {
                        std::cout << AiNodeGetStr(change.node,"name");
                        std::cout << ": Cannot find node " << (*it) << " ignored." << std::endl;
                     }
                  }
               }

               free((void *)change.param_value.STR);
               break;
            }
            default:
               AiMsgWarning("[change-q]  could not determine %s's parameter type '%s'!!!",
                     AiNodeGetStr(change.node,"name"), change.param_name);
         }
      }

      free(change.param_name);
   }
   AiCritSecLeave(&csection);
}

void KatChangeList::Clear()
{
   AiCritSecEnter(&csection);
   while (myChangeList.size())
   {
      KatParamChange change = myChangeList.front();
      myChangeList.pop();
      if (change.array_type)
      {
         AiArrayDestroy(change.param_value.ARRAY);
      }
      else
      {
         switch(change.param_type)
         {
            case AI_TYPE_STRING:
               free((void *)change.param_value.STR);
            break;
         }
      }
      free(change.param_name);
   }
   AiCritSecLeave(&csection);
};

KatChangeList ChangeNodeQ;
KatChangeList ChangeExtraQ;

//extern "C" 
//{
/*
 * Apply any outstanding node changes
 */
void
ApplyNodeChanges(FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState)
{
   ChangeNodeQ.ApplyChanges(rootIterator, sharedState);
   return;
}
/*
 * Find any outstanding node changes
 */
bool
FindNodeChange(const char* paramname)
{
   return ChangeNodeQ.FindChange(paramname) ? true : false;
}
/*
 * Get a node change
 * If parameter cannot find this function will throw exception.
 */
KatParamChange
GetNodeChange(const char* paramname)
{
   return ChangeNodeQ.GetChange(paramname);
}

/*
 * Add a new node change to our change-queue
 */
void 
AddNodeChange(KatParamChange *change)
{
  ChangeNodeQ.push_back(*change);
}

/*
 * Are there changes available?
 */
bool
NodeChangesAvailable()
{
   return ChangeNodeQ.empty()&ChangeExtraQ.empty() ? false: true;
}

/*
 * clear any outstanding extra changes
 */
void
ClearExtraChanges()
{
   ChangeExtraQ.Clear();
   return;
}
/*
 * Find any outstanding extra changes
 */
bool
FindExtraChange(const char* paramname)
{
   return ChangeExtraQ.FindChange(paramname) ? true : false;
}
/*
 * Get a extra change
 * If parameter cannot find this function will throw exception.
 */
KatParamChange
GetExtraChange(const char* paramname)
{
   return ChangeExtraQ.GetChange(paramname);
}

/*
 * Add a new extra change to our change-queue
 */
void 
AddExtraChange(KatParamChange *change)
{
   ChangeExtraQ.push_back(*change);
}

int
GetIntValue(KatParamChange* change)
{
   if (change->array_type)
      throw -1;

   switch(change->param_type)
   {
      case AI_TYPE_BOOLEAN:
         return (int)change->param_value.BOOL;
      case AI_TYPE_BYTE:
         return (int)change->param_value.BYTE;
      case AI_TYPE_ENUM:
      case AI_TYPE_INT:
         return (int)change->param_value.INT;
      case AI_TYPE_UINT:
         return (int)change->param_value.UINT;
      case AI_TYPE_FLOAT:
         return (int)change->param_value.FLT;
      case AI_TYPE_STRING:
         return (int)atoi(change->param_value.STR);
      case AI_TYPE_RGB:
      case AI_TYPE_VECTOR:
      case AI_TYPE_POINT:
      case AI_TYPE_POINTER:
      default:
         throw -1;
   }
}

float
GetFloatValue(KatParamChange* change)
{
   if (change->array_type)
      throw -1;

   switch(change->param_type)
   {
      case AI_TYPE_BOOLEAN:
         return (float)change->param_value.BOOL;
      case AI_TYPE_BYTE:
         return (float)change->param_value.BYTE;
      case AI_TYPE_ENUM:
      case AI_TYPE_INT:
         return (float)change->param_value.INT;
      case AI_TYPE_UINT:
         return (float)change->param_value.UINT;
      case AI_TYPE_FLOAT:
         return (float)change->param_value.FLT;
      case AI_TYPE_STRING:
         return (float)atof(change->param_value.STR);
      case AI_TYPE_RGB:
      case AI_TYPE_VECTOR:
      case AI_TYPE_POINT:
      case AI_TYPE_POINTER:
      default:
         throw -1;
   }
}

double
GetDoubleValue(KatParamChange* change)
{
   if (change->array_type)
      throw -1;

   switch(change->param_type)
   {
      case AI_TYPE_BOOLEAN:
         return (double)change->param_value.BOOL;
      case AI_TYPE_BYTE:
         return (double)change->param_value.BYTE;
      case AI_TYPE_ENUM:
      case AI_TYPE_INT:
         return (double)change->param_value.INT;
      case AI_TYPE_UINT:
         return (double)change->param_value.UINT;
      case AI_TYPE_FLOAT:
         return (double)change->param_value.FLT;
      case AI_TYPE_STRING:
         return (double)atof(change->param_value.STR);
      case AI_TYPE_RGB:
      case AI_TYPE_VECTOR:
      case AI_TYPE_POINT:
      case AI_TYPE_POINTER:
      default:
         throw -1;
   }
}

//}
