// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.
#ifdef _WIN32
#include <windows.h>
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



class CameraViewerModifier : public FnKat::ViewerModifier
{
public:

    CameraViewerModifier(FnKat::GroupAttribute args) : FnKat::ViewerModifier(args)
    {
        // Empty
    }


    static FnKat::ViewerModifier* create(FnKat::GroupAttribute args)
    {
        return (FnKat::ViewerModifier*)new CameraViewerModifier(args);
    }


    static FnKat::GroupAttribute getArgumentTemplate()
    {
        FnKat::GroupBuilder gb;
        return gb.build();
    }


    static const char* getLocationType()
    {
        return "camera";
    }


    void deepSetup(FnKat::ViewerModifierInput& input)
    {
        input.overrideHostGeometry();

    }


    void setup(FnKat::ViewerModifierInput& input)
    {
        //std::cout <<"Log print test!"<< std::endl;
        // Empty
    }


    void draw(FnKat::ViewerModifierInput& input)
    {
        glPushAttrib(GL_POLYGON_BIT | GL_LIGHTING_BIT | GL_LINE_BIT);
        glDisable(GL_LIGHTING);
        // Lock global render context
        // UTIL::RenderContextLock localRenderLock(0);
        if (input.isSelected())
            glColor3f(1,1,1);
        else
            glColor3f(0,0,1);

        glLineWidth(1);
        glPushMatrix();
        
        glBegin(GL_LINE_LOOP);
        glVertex3f(-0.375f,-0.5f,0.5f);
        glVertex3f(0.375f,-0.5f,0.5f);
        glVertex3f(0.375f,0.5f,0.5f);
        glVertex3f(-0.375f,0.5f,0.5f);
        glEnd();
        
        glBegin(GL_LINE_LOOP);
        glVertex3f(-0.375f,-0.5f,-0.5f);
        glVertex3f(0.375f,-0.5f,-0.5f);
        glVertex3f(0.375f,0.5f,-0.5f);
        glVertex3f(-0.375f,0.5f,-0.5f);
        glEnd();

        glBegin(GL_LINE_LOOP);
        glVertex3f(-0.5f,-0.5f,-0.75f);
        glVertex3f(0.5f,-0.5f,-0.75f);
        glVertex3f(0.5f,0.5f,-0.75f);
        glVertex3f(-0.5f,0.5f,-0.75f);
        glEnd();
        
        glBegin(GL_LINE_LOOP);
        glVertex3f(-0.25f,-0.25f,-0.5f);
        glVertex3f(0.25f,-0.25f,-0.5f);
        glVertex3f(0.25f,0.25f,-0.5f);
        glVertex3f(-0.25f,0.25f,-0.5f);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(0.375f, 0.5f, 0.5f);
        glVertex3f(0.375f, 0.5f, -0.5f);
        glVertex3f(0.375f, -0.5f, 0.5f);
        glVertex3f(0.375f, -0.5f, -0.5f);
        glVertex3f(-0.375f, 0.5f, 0.5f);
        glVertex3f(-0.375f, 0.5f, -0.5f);
        glVertex3f(-0.375f, -0.5f, 0.5f);
        glVertex3f(-0.375f, -0.5f, -0.5f);
        glEnd();
        
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.6f, 0.5f);
        glVertex3f(0.0f, 0.6f, -0.5f);
        glVertex3f(0.0f, 0.6f, 0.5f);
        glVertex3f(0.0f, 0.5f, 0.5f);
        glVertex3f(0.0f, 0.5f, -0.5f);
        glVertex3f(0.0f, 0.6f, -0.5f);
        glEnd();

        glBegin(GL_LINES);
        glVertex3f(-0.5f,-0.5f,-0.75f);
        glVertex3f(-0.25f,-0.25f,-0.5f);
        glVertex3f(0.5f,-0.5f,-0.75f);
        glVertex3f(0.25f,-0.25f,-0.5f);
        glVertex3f(0.5f,0.5f,-0.75f);
        glVertex3f(0.25f,0.25f,-0.5f);
        glVertex3f(-0.5f,0.5f,-0.75f);
        glVertex3f(-0.25f,0.25f,-0.5f);

        //glVertex3f(0.25f,0.25f,-0.5f);
        //glVertex3f(0.25f,-0.25f,-0.5f);

        //glVertex3f(-0.25f,-0.25f,-0.5f);
        //glVertex3f(-0.25f,0.25f,-0.5f);
        glEnd();

        glPopMatrix();
        glPopAttrib();
    }

    void cleanup(FnKat::ViewerModifierInput& input)
    {
        // Empty
    }

    void deepCleanup(FnKat::ViewerModifierInput& input)
    {
        // Empty
    };

    FnKat::DoubleAttribute getLocalSpaceBoundingBox(FnKat::ViewerModifierInput& input)
    {
        double bounds[6] = {-1, 1, -1, 1, -1, 1};
        return FnKat::DoubleAttribute(bounds, 6, 1);
    }

    static void flush() {}
};

DEFINE_VMP_PLUGIN(CameraViewerModifier)

void registerPlugins()
{
    REGISTER_PLUGIN(CameraViewerModifier, "CameraViewerModifier", 0, 1);
    std::cout << "Register CameraViewerModifier v1.0" << std::endl;
}
