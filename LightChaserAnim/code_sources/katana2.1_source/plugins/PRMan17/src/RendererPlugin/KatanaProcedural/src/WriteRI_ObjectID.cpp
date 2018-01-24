// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <fstream>

#include <PRManProcedural.h>
#include <rx.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <iostream>

#include <FnRender/plugin/IdSenderFactory.h>

#include <ErrorReporting.h>

// Using pthread library here to keep boost library dependencies to a minimum.
#include <pthread.h>

using namespace FnKat;

namespace PRManProcedural
{
namespace
{
    void writeObjectID(const std::string& objectName, PRManPluginState* sharedState)
    {
        int64_t id_value = sharedState->getNextId();

        // Write the id to prman
        RiAttribute(const_cast<char *>("identifier"), "uniform int id", &id_value, RI_NULL);

        // Don't bother informing Katana if the id written is zero; this is an error case.
        if (id_value == 0)
            return;

        sharedState->sendIdAssignment(id_value, objectName.c_str());
    }
}

// Write object IDs for all objects (used to feed the id output pass)
// Note: Only tag leafs items
//
// Also note that IDs will only be written out if a user option is
// specified in the rib stream:
//
// Option "user:Procedural" "IDFile" ["/tmp/foo"]

void WriteRI_Object_Id(FnScenegraphIterator sgIterator, PRManPluginState* sharedState)
{
    Report_Debug("WriteRI_Object_Id called", sharedState, sgIterator);

    if(sharedState->proceduralSettings.useIDPass)
    {
        RiArchiveRecord( "comment", "Attribute \"identifier\" \"uniform int id\" [ID]" );
        return;
    }
    
    // Initialize the ID writing system.
    sharedState->initializeObjectIdState();

    if(!sharedState->isIdWritingEnabled()) return;

    std::string type = sgIterator.getType();

    if (type == "group") return;
    if (type == "level-of-detail group") return;
    if (type == "assembly") return;
    if (type == "assembly instance") return;
    if (type == "component") return;
    if (type == "component instance") return;

    writeObjectID(sgIterator.getFullName(), sharedState);
}
}
