// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef FNVIEWERMODIFIERSUITE__H
#define FNVIEWERMODIFIERSUITE__H

#include <FnAttribute/suite/FnAttributeSuite.h>

extern "C"
{
  enum MatrixType
  {
    VIEW              = 0,
    VIEW_LEFT         = 1,
    VIEW_RIGHT        = 2,
    PROJECTION        = 3,
    PROJECTION_LEFT   = 4,
    PROJECTION_RIGHT  = 5,
  };

  typedef struct FnViewerModifierStruct*      FnViewerModifierHandle;
  typedef struct FnViewerModifierHostStruct*  FnViewerModifierHostHandle;

  #define FnViewerModifierPluginSuite_version 1

  struct FnViewerModifierPluginSuite_v1
  {
    FnViewerModifierHandle  (*create)(FnAttributeHandle args);
    FnAttributeHandle       (*getArgumentTemplate)();
    const char*             (*getLocationType)();

    void                    (*destroy)(FnViewerModifierHandle handle);
    void                    (*deepSetup)(FnViewerModifierHandle handle, FnViewerModifierHostHandle host);
    void                    (*setup)(FnViewerModifierHandle handle, FnViewerModifierHostHandle host);
    void                    (*draw)(FnViewerModifierHandle handle, FnViewerModifierHostHandle host);
    void                    (*cleanup)(FnViewerModifierHandle handle, FnViewerModifierHostHandle host);
    void                    (*deepCleanup)(FnViewerModifierHandle handle, FnViewerModifierHostHandle host);
    FnAttributeHandle       (*getWorldSpaceBoundingBox)(FnViewerModifierHandle handle, FnViewerModifierHostHandle host); // deprecated!
    FnAttributeHandle       (*getLocalSpaceBoundingBox)(FnViewerModifierHandle handle, FnViewerModifierHostHandle host);
  };

  #define FnViewerModifierHostSuite_version 1

  struct FnViewerModifierHostSuite_v1
  {
    const char* (*getName) (FnViewerModifierHostHandle handle);
    const char* (*getType) (FnViewerModifierHostHandle handle);
    const char* (*getFullName) (FnViewerModifierHostHandle handle);

    bool              (*isSelected)(FnViewerModifierHostHandle handle);

    FnAttributeHandle (*getAttribute)(FnViewerModifierHostHandle handle, const char* name, const char* atLocation);
    FnAttributeHandle (*getGlobalAttribute)(FnViewerModifierHostHandle handle, const char* name, const char* atLocation);
    FnAttributeHandle (*getUniversalAttribute)(FnViewerModifierHostHandle handle, const char* name, const char* atLocation);
    FnAttributeHandle (*getWorldSpaceXform)(FnViewerModifierHostHandle handle, const char* atLocation);

    bool              (*isLiveAttribute)(FnViewerModifierHostHandle handle, const char* attribute, const char* atLocation);
    FnAttributeHandle (*getLiveAttribute)(FnViewerModifierHostHandle handle, const char* name, const char* atLocation);
    bool              (*isGlobalLiveAttribute)(FnViewerModifierHostHandle handle, const char* attribute, const char* atLocation);
    FnAttributeHandle (*getGlobalLiveAttribute)(FnViewerModifierHostHandle handle, const char* name, const char* atLocation);
    bool              (*isLiveWorldSpaceXform)(FnViewerModifierHostHandle handle, const char* atLocation);
    FnAttributeHandle (*getLiveWorldSpaceXform)(FnViewerModifierHostHandle handle, const char* atLocation);

    const char**      (*getChildNames)(FnViewerModifierHostHandle handle, unsigned int *nameCount, const char * atLocation);

    void              (*setUseFixedFunctionPipeline)(FnViewerModifierHostHandle handle, bool useFixed);
    void              (*setTransparent)(FnViewerModifierHostHandle handle, bool useTransparent);
    FnAttributeHandle (*getDisplayMatrix)(FnViewerModifierHostHandle handle, MatrixType matrix);
    FnAttributeHandle (*getViewport)(FnViewerModifierHostHandle handle);

    int               (*getDrawOption)(FnViewerModifierHostHandle handle, const char* option);
    float             (*getDrawOptionFloat)(FnViewerModifierHostHandle handle, const char* option);
    const char*       (*getDrawOptionString)(FnViewerModifierHostHandle handle, const char* option);
    void              (*overrideHostGeometry)(FnViewerModifierHostHandle handle);
  };
}

#endif  // #ifndef FNVIEWERMODIFIERSUITE__H
