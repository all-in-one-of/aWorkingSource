// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_FNRENDERPLUGINSUITE_H
#define FNRENDER_FNRENDERPLUGINSUITE_H

extern "C"
{
    typedef struct FnRenderPluginStruct* FnRenderPluginHandle;

    #include "FnScenegraphIterator/suite/FnScenegraphIteratorSuite.h"
    #include "FnAttribute/suite/FnAttributeSuite.h"

    #define FnRenderPluginSuite_version 1

    struct FnRenderPluginSuite_v1
    {
        FnRenderPluginHandle (*create)(
            FnSgIteratorHandle sgHandle,
            FnAttributeHandle argsHandle);

        void (*destroy)(FnRenderPluginHandle handle);

        int (*start)(
            FnRenderPluginHandle handle);

        int (*pause)(
            FnRenderPluginHandle handle);

        int (*resume)(
            FnRenderPluginHandle handle);

        int (*stop)(
            FnRenderPluginHandle handle);

        int (*startLiveEditing)(
            FnRenderPluginHandle handle);

        int (*stopLiveEditing)(
            FnRenderPluginHandle handle);

        int (*processControlCommand)(
            FnRenderPluginHandle handle,
            const char* command);

        int (*queueDataUpdates)(
            FnRenderPluginHandle handle,
            FnAttributeHandle commandAttribute);

        bool (*hasPendingDataUpdates)(
            FnRenderPluginHandle handle);

        int (*applyPendingDataUpdates)(
            FnRenderPluginHandle handle);

        FnAttributeHandle (*configureDiskRenderOutputProcess)(
            FnRenderPluginHandle handle,
            const char* outputName,
            const char* outputPath,
            const char* renderMethodName,
            const float* frameTime);
    };
}

#endif
