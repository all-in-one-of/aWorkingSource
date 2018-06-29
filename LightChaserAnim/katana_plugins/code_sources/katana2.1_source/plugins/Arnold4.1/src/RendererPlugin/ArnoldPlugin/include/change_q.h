// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef _CHANGE_QUEUE_H
#define _CHANGE_QUEUE_H

#include <ai.h>
#include <arnold_render.h>

typedef void (*NodePlusAttrCallbackPtr)(AtNode *, FnKat::Attribute attr, ArnoldPluginState* sharedState);

/*
 * a node delta
 */
typedef struct KatParamChange
{
   AtNode       *node;
   char         *param_name;
   int           param_type;
   int           array_type;
   AtParamValue  param_value;
   bool          is_default;
   
   //fnc_ptr-based callback
   FnKat::Attribute attr_data;
   NodePlusAttrCallbackPtr attr_callback;
   
   KatParamChange(): is_default(false) {}

} KatParamChange;

void ApplyNodeChanges(FnKat::FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState);
bool FindNodeChange(const char*);
KatParamChange GetNodeChange(const char*);
void AddNodeChange(KatParamChange *change);
bool NodeChangesAvailable();

void ClearExtraChanges();
bool FindExtraChange(const char*);
KatParamChange GetExtraChange(const char*);
void AddExtraChange(KatParamChange *change);

int    GetIntValue(KatParamChange* change);
float  GetFloatValue(KatParamChange* change);
double GetDoubleValue(KatParamChange* change);

#endif // _CHANGE_QUEUE_H
