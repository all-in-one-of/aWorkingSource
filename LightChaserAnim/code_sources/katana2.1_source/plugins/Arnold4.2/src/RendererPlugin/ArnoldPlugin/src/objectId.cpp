// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include "arnold_render.h"

#include <string.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <objectId.h>
#include <ArnoldPluginState.h>

using namespace FnKat;

void applyObjectId(AtNode* node, FnScenegraphIterator sgIterator, ArnoldPluginState* sharedState)
{
    if (!sharedState->isIdWritingEnabled()) return;

    int64_t id_value = sharedState->getNextId();
    // Write the id to arnold
    const AtNodeEntry *baseNodeEntry = AiNodeGetNodeEntry(node);
    if (AiNodeEntryLookUpParameter(baseNodeEntry, "object_id"))
    {
        AiNodeSetInt(node, "object_id", id_value);
    }
    else
    {
        AiNodeSetInt(node, "id", id_value);
    }

    std::string objectName = sgIterator.getFullName();
    // Don't bother informing Katana if the id written is zero; this is an error case.
    if (id_value == 0)
        return;

    sharedState->sendIdAssignment(id_value, objectName.c_str());
}

void initObjectId(const char *hostname, long frameID, ArnoldPluginState* sharedState)
{
    sharedState->initializeObjectIdState(hostname, frameID);
}

void shutdownObjectId(struct ArnoldObjectIdConnectionState& state, ArnoldPluginState* sharedState)
{
    sharedState->shutdownObjectId();
}
