// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.
#ifdef _WIN32
#include <FnPlatform/Windows.h>
#endif

#include <FnViewerModifier/plugin/FnViewerModifier.h>
#include <FnViewerModifier/plugin/FnViewerModifierInput.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnAttribute.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <iostream>




class SphereRender : public FnKat::ViewerModifier
{
public:
  SphereRender(FnKat::GroupAttribute args) : FnKat::ViewerModifier(args)
  {
    // Empty
  }

  static FnKat::ViewerModifier* create(FnKat::GroupAttribute args)
  {
    return (FnKat::ViewerModifier*)new SphereRender(args);
  }

  static FnKat::GroupAttribute getArgumentTemplate()
  {
    FnKat::GroupBuilder gb;
    return gb.build();
  }

  static const char* getLocationType()
  {
    return "sphere";
  }

  void deepSetup(FnKat::ViewerModifierInput& input) {};
  void setup(FnKat::ViewerModifierInput& input) {};
  void draw(FnKat::ViewerModifierInput& input) 
  {
    FnKat::DoubleAttribute radiusAttr = input.getAttribute("geometry.radius");
    double radius = radiusAttr.getValue(0.0, false);
    
    FnKat::DoubleAttribute xformAttr = input.getWorldSpaceXform();
    FnKat::DoubleConstVector xform = xformAttr.getNearestSample(0.f);

    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
    glPushMatrix();

    GLUquadricObj*  quad;
    quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);

    gluSphere(quad, radius, 32, 32);
    gluDeleteQuadric(quad);

    glPopMatrix();
    glPopAttrib();
  }

  void cleanup(FnKat::ViewerModifierInput& input) {};
  void deepCleanup(FnKat::ViewerModifierInput& input) {};


  FnKat::DoubleAttribute getLocalSpaceBoundingBox(FnKat::ViewerModifierInput& input)
  {
    FnKat::DoubleAttribute radiusAttr = input.getAttribute("geometry.radius");
    double radius = radiusAttr.getValue(0.0, false);
    double bounds[6] = {-radius, radius, -radius, radius, -radius, radius};
    return FnKat::DoubleAttribute(bounds, 6, 1);
  }

  static void flush() {}

  static void onFrameBegin() {}

  static void onFrameEnd() {}

};

DEFINE_VMP_PLUGIN(SphereRender)

void registerPlugins()
{
  REGISTER_PLUGIN(SphereRender, "SphereRender", 0, 1);
}
