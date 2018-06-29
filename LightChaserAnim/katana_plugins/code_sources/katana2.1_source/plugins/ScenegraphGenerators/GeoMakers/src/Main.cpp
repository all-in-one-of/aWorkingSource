// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <SphereMaker.h>
#include <CubeMaker.h>

DEFINE_SGG_PLUGIN(CubeMaker)
DEFINE_SGG_PLUGIN(SphereMaker)


void registerPlugins()
{
    REGISTER_PLUGIN(SphereMaker, "SphereMaker", 0, 1);
    REGISTER_PLUGIN(CubeMaker, "CubeMaker", 0, 1);
}

