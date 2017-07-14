// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef COMMON_OBJECT_ID_H
#define COMMON_OBJECT_ID_H
#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <ai.h>
#include <ArnoldPluginState.h>
// Apply object id to a node and pass information back to Katana.
void initObjectId(const char *hostname, long frameId, ArnoldPluginState* sharedState);
void applyObjectId(AtNode *node, FnKat::FnScenegraphIterator sgIterator, ArnoldPluginState* sharedState);
void shutdownObjectId(ArnoldPluginState* sharedState);
#endif
