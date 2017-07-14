// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef COMMON_OBJECTSETTINGS_H
#define COMMON_OBJECTSETTINGS_H

#include <FnScenegraphIterator/FnScenegraphIterator.h>
#include <ai.h>
#include <vector>
#include <ArnoldPluginState.h>


void applyObjectSettings(FnKat::FnScenegraphIterator sgIterator, AtNode * leafNode,
        std::vector<AtNode*>* childNodes, ArnoldPluginState* sharedState);

#endif
