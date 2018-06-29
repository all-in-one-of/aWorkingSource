// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include "arnold_render.h"

#include <string.h>
#include <ArnoldPluginState.h>

using namespace FnKat;

namespace
{
    int init(AtNode* myNode, void **user_ptr)
    {
        ArnoldPluginState* sharedState = (ArnoldPluginState*)AiNodeGetPtr(myNode, "sharedState");
        if (!sharedState)
        {
            AiMsgFatal("[kat] FATAL: (%s) Null sharedState pointer.", AiNodeGetStr(myNode, "name"));
        }

        const char *scenegraphLocationPath = AiNodeGetStr(myNode, "scenegraphLocationPath");
        if (!scenegraphLocationPath)
        {
            AiMsgFatal("[kat]: Scene graph location path not specified for (%s)",
                       AiNodeGetStr(myNode, "name"));
        }

        FnScenegraphIterator *rootIterator =\
            reinterpret_cast<FnScenegraphIterator*>(AiNodeGetPtr(myNode, "userptr"));
        if (!rootIterator)
        {
            AiMsgFatal("[kat]: Null root iterator pointer (%s)",
                       AiNodeGetStr(myNode, "name"));
        }

        FnScenegraphIterator sgIterator;
        bool traverseFromFirstChild = false;
        traverseFromFirstChild = AiNodeGetBool(myNode, "traverseFromFirstChild");
        if(traverseFromFirstChild)
        {
            FnScenegraphIterator parentIterator =\
                rootIterator->getByPath(scenegraphLocationPath);
            sgIterator = parentIterator.getFirstChild(true);
        }
        else
        {
            sgIterator = rootIterator->getByPath(scenegraphLocationPath);
        }

        if (!sgIterator.isValid())
        {
            AiMsgFatal("[kat] FATAL: (%s) Invalid sgIterator.",
                       AiNodeGetStr(myNode, "name"));
        }

        std::vector<AtNode*>* childNodes = new std::vector<AtNode*>;
        *user_ptr = childNodes;

        try
        {
            while (sgIterator.isValid())
            {
                buildLocation(sgIterator, childNodes, false, true, sharedState);
                sgIterator = sgIterator.getNextSibling();
            }
        }
        catch (...)
        {
            delete reinterpret_cast<FnScenegraphIterator*>(rootIterator);
            AiNodeSetPtr(myNode, "userptr", NULL);
            AiMsgError("[kat] Uncaught exception at %s",
                       sgIterator.getFullName().c_str());

            return 0;
        }

        delete reinterpret_cast<FnScenegraphIterator*>(rootIterator);
        AiNodeSetPtr(myNode, "userptr", NULL);

        return 1;
    }

    int cleanup(void *userData)
    {
        std::vector<AtNode*>* childNodes  = reinterpret_cast<std::vector<AtNode*>*>(userData);
        delete childNodes;
        return 1;
    }

    int numNodes(void *userData)
    {
        std::vector<AtNode*>* childNodes  = reinterpret_cast<std::vector<AtNode*>*>(userData);
        return childNodes->size();
    }

    AtNode* getNode(void *userData, int index)
    {
        std::vector<AtNode*>* childNodes  = reinterpret_cast<std::vector<AtNode*>*>(userData);
        return (*childNodes)[index];
    }

    int null_init(AtNode* myNode, void **user_ptr)
    {
        return 1;
    }

    int null_cleanup(void *userData)
    {
        return 1;
    }

    int null_numNodes(void *userData)
    {
        return 0;
    }

    AtNode* null_getNode(void *userData, int index)
    {
        return NULL;
    }
}

int KatanaProcedural_ProcLoader(AtProcVtable* api)
{
    api->Init        = init;
    api->Cleanup     = cleanup;
    api->NumNodes    = numNodes;
    api->GetNode     = getNode;
    strcpy(api->version, AI_VERSION);
    return 1;
}

int KatanaProcedural_ProcNull(AtProcVtable* api)
{
    api->Init        = null_init;
    api->Cleanup     = null_cleanup;
    api->NumNodes    = null_numNodes;
    api->GetNode     = null_getNode;
    strcpy(api->version, AI_VERSION);
    return 1;
}
