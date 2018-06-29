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
            AiMsgError("[kat] FATAL: (%s) Null sharedState pointer.", AiNodeGetStr(myNode, "name"));
            exit(-1);
        }

        void* proc_userptr = AiNodeGetPtr(myNode, "userptr");
        if (!proc_userptr)
        {
            AiMsgError("[kat] FATAL: (%s) Null user pointer.", AiNodeGetStr(myNode, "name"));
            exit(-1);
        }

        FnScenegraphIterator sgIterator = *reinterpret_cast<FnScenegraphIterator*>(proc_userptr);
        if (!sgIterator.isValid())
        {
            AiMsgError("[kat] FATAL: (%s) Invalid sgIterator.", AiNodeGetStr(myNode, "name"));
            exit(-1);
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
            delete reinterpret_cast<FnScenegraphIterator*>(proc_userptr);
            AiNodeSetPtr(myNode, "userptr",NULL);
            AiMsgError("[kat] Uncaught exception at %s", sgIterator.getFullName().c_str());
            
            return 0;
        }

        delete reinterpret_cast<FnScenegraphIterator*>(proc_userptr);
        AiNodeSetPtr(myNode, "userptr",NULL);
        
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
