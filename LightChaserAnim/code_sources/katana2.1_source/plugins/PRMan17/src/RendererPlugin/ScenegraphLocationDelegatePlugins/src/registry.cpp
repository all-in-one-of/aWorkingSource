// *******************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Pixar, is intended for
// Katana and PRMan customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// *******************************************************************

#include <PRManSphereScenegraphLocationDelegate.h>
#include <PRManPolymeshScenegraphLocationDelegate.h>
#include <PRManSubdmeshScenegraphLocationDelegate.h>
#include <PRManCurvesScenegraphLocationDelegate.h>
#include <PRManNurbspatchScenegraphLocationDelegate.h>
#include <PRManPointCloudScenegraphLocationDelegate.h>
#include <PRManSpheresScenegraphLocationDelegate.h>
#include <PRManBrickMapScenegraphLocationDelegate.h>
#include <PRManProceduralScenegraphLocationDelegate.h>
#include <PRManRibArchiveScenegraphLocationDelegate.h>
#include <PRManVolumeScenegraphLocationDelegate.h>

DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(PRManSphereScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(PRManPolymeshScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(PRManSubdmeshScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(PRManCurvesScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(PRManNurbspatchScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(PRManPointCloudScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(PRManSpheresScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(PRManBrickMapScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(PRManProceduralScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(PRManRibArchiveScenegraphLocationDelegate)
DEFINE_SCENEGRAPH_LOCATION_DELEGATE_PLUGIN(PRManVolumeScenegraphLocationDelegate)

void registerPlugins() {
    REGISTER_PLUGIN(PRManPolymeshScenegraphLocationDelegate, "PRMan17PolymeshScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(PRManSubdmeshScenegraphLocationDelegate, "PRMan17SubdmeshScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(PRManSphereScenegraphLocationDelegate, "PRMan17SphereScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(PRManCurvesScenegraphLocationDelegate, "PRMan17CurvesScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(PRManNurbspatchScenegraphLocationDelegate, "PRMan17NurbspatchScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(PRManPointCloudScenegraphLocationDelegate, "PRMan17PointCloudScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(PRManSpheresScenegraphLocationDelegate, "PRMan17SpheresScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(PRManBrickMapScenegraphLocationDelegate, "PRMan17BrickMapScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(PRManProceduralScenegraphLocationDelegate, "PRMan17ProceduralScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(PRManRibArchiveScenegraphLocationDelegate, "PRMan17RibArchiveScenegraphLocationDelegate", 0, 1);
    REGISTER_PLUGIN(PRManVolumeScenegraphLocationDelegate, "PRMan17VolumeScenegraphLocationDelegate", 0, 1);
}
