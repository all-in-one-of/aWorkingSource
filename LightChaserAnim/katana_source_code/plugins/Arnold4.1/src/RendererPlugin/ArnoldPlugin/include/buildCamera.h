// ********************************************************************
// This file contains copyrighted work from The Foundry,
// Sony Pictures Imageworks and Solid Angle, is intended for
// Katana and Solid Angle customers, and is not for distribution
// outside the terms of the corresponding EULA(s).
// ********************************************************************

#ifndef BUILDCAMERA_H
#define BUILDCAMERA_H

#include <ArnoldPluginState.h>

// Apply the camera default settings
void setCameraDefault(AtNode*);


// Get the default render camera name (renderSettings.cameraName)
std::string getDefaultRenderCameraName(FnKat::FnScenegraphIterator rootIterator);
// Build the render camera.
void buildRenderCamera(FnKat::FnScenegraphIterator rootIterator, ArnoldPluginState* sharedState, int *overscan=0,
    int *displayWindowSize=0);
// Build all the cameras in the scene.
void buildCameras(FnKat::FnScenegraphIterator worldIterator, ArnoldPluginState* sharedState);
// Build a specific camera location.
AtNode* buildCameraLocation(FnKat::FnScenegraphIterator cameraIterator, ArnoldPluginState* sharedState, int *overscan=0,
    int *displayWindowSize=0);
// Build a specific camera location given a root prodpucer and a path.  Results are cached
// by path name so calling multiple times with same path is cheap, and doesn't traverse scene.
AtNode *buildCameraLocationByPath(const std::string &path, FnKat::FnScenegraphIterator rootIterator,
    int *overscan, ArnoldPluginState* sharedState, int *displayWindowSize=0);
// Get node of a previously built camera location (error if it hasn't been built)
AtNode *getCameraLocationByPath(const std::string &path, ArnoldPluginState* sharedState);


#endif

