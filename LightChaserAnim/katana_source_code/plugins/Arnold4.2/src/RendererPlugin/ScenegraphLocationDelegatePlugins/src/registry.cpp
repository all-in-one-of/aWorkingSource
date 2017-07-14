// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#include <ArnoldVolumeScenegraphLocationDelegate.h>
#include <ArnoldSphereScenegraphLocationDelegate.h>
#include <ArnoldPointcloudScenegraphLocationDelegate.h>
#include <ArnoldCurvesScenegraphLocationDelegate.h>
#include <ArnoldNurbspatchScenegraphLocationDelegate.h>
#include <ArnoldPolymeshScenegraphLocationDelegate.h>
#include <ArnoldProceduralScenegraphLocationDelegate.h>
#include <ArnoldGenericScenegraphLocationDelegate.h>

DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(ArnoldVolumeScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(ArnoldSphereScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(ArnoldPointcloudScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(ArnoldCurvesScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(ArnoldNurbspatchScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(ArnoldPolymeshScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(ArnoldProceduralScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(ArnoldGenericScenegraphLocationDelegate)
    
void registerPlugins() {
    REGISTER_PLUGIN(ArnoldVolumeScenegraphLocationDelegate, "ArnoldVolumeScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(ArnoldSphereScenegraphLocationDelegate, "ArnoldSphereScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(ArnoldPointcloudScenegraphLocationDelegate, "ArnoldPointcloudScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(ArnoldCurvesScenegraphLocationDelegate, "ArnoldCurvesScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(ArnoldNurbspatchScenegraphLocationDelegate, "ArnoldNurbspatchScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(ArnoldPolymeshScenegraphLocationDelegate, "ArnoldPolymeshScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(ArnoldProceduralScenegraphLocationDelegate, "ArnoldProceduralScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(ArnoldGenericScenegraphLocationDelegate, "ArnoldGenericScenegraphLocationDelegate", 0, 1);
}
