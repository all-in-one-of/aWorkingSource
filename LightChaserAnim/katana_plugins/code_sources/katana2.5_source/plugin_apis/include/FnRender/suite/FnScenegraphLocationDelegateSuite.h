// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FNRENDER_FNSCENEGRAPHLOCATIONDELEGATESUITE_H
#define FNRENDER_FNSCENEGRAPHLOCATIONDELEGATESUITE_H

#include "FnScenegraphIterator/suite/FnScenegraphIteratorSuite.h"

#include <vector>
#include <string>

extern "C"
{
    typedef struct FnScenegraphLocationDelegateStruct* FnLocationHandle;

    #define FnScenegraphLocationDelegatePluginSuite_version 1

    struct FnScenegraphLocationDelegatePluginSuite_v1
    {
        FnLocationHandle (*create)();

        void (*destroy)(
            FnLocationHandle handle);

        void* (*process)(
            FnLocationHandle handle,
            FnSgIteratorHandle it,
            void* optionalInput);

        FnAttributeHandle (*getSupportedRenderer)(
            FnLocationHandle handle);

        FnAttributeHandle (*fillSupportedLocationList)(
            FnLocationHandle handle);
    };
}

#endif
