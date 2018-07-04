// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DRAWABLEMESH_H_
#define DRAWABLEMESH_H_

#if defined(_WIN32)
#include <FnPlatform/Windows.h>
#endif

#include "GL/glew.h"
#include "FnViewer/utils/FnGLShaderProgram.h"
#include <FnAttribute/FnAttribute.h>
#include <vector>
#include <map>

#if defined(_WIN32)
    #include <Wingdi.h>
    typedef HGLRC GLContextType;
#else
    #include <GL/glx.h>
    typedef GLXContext GLContextType;
#endif

/**
This class represents an OpenGL mesh and it's associated objects
and settings. Its settings (such as the view matrix to draw with)
as well as the geometry information such as vertex positions,
are specified using the setDrawAttribute() function.

The VBOs (and similar objects) created by this mesh will be shared
by multiple viewports so care must be taken when loading and drawing
geometry to ensure that the data is valid for the current OpenGL
context.
*/
class DrawableMesh
{
public:
    enum BUFFERS {
        VERTEX_BUFFER,
        COLOR_BUFFER,
        INDEX_BUFFER,
        TEXCOORD_BUFFER,
        NORMAL_BUFFER,

        NUM_BUFFERS
    };

    DrawableMesh();
    virtual ~DrawableMesh();

    /// Releases any OpenGL resources held by this mesh.
    void clearGLResources();

    /// Deletes the GLShaderProgram held by this mesh.
    void deleteShader();

    /// Sets generic settings, eg. geometry data, color values, draw mode etc.
    void setDrawAttribute(const std::string& name, FnAttribute::Attribute attr);

    /// Draw this object.
    void draw();

    /**
     * Draw this object for the picking framebuffer using the given shader
     * program. The flat shaded color to draw the mesh with should be set already via setDrawAttribute().
     */
    void pickerDraw(Foundry::Katana::ViewerUtils::GLShaderProgram& shaderProgram);

    /**
     * Inspects the passed attribute for a child called "geometry" and populates
     * the OpenGL buffers as required.
     */
    void setupGeometry(const FnAttribute::GroupAttribute& attributes);

    /**
     * Inspects the passed attribute for a child called "material" and
     * generates any materials, shaders or settings as required.
     * This function allows you to override the GLSL shader program, by
     * passing replacement code in the scene graph.
     */
    void setupMaterials(const FnAttribute::GroupAttribute& attributes);

    /// Returns true if the mesh is fully set up and drawable.
    bool isValid() const;

    /// Returns the custom GLSL shader for this location.
    Foundry::Katana::ViewerUtils::GLShaderProgram* getCustomShader() const { return m_customShader; }

    /// Returns a valid OpenGL Vertex Array Object for the current context.
    GLuint getVAO();

private:
    /// Deletes any VAOs that have been marked for cleaning.
    void cleanupVAO();

    /// The current "geometry.point.P" attribute that the mesh was built with.
    FnAttribute::FloatAttribute m_pointsAttr;
    /// A map of OpenGL VOAs for the various active contexts.
    std::map<GLContextType, GLuint> m_vaoMap;
    /// A map containing a list of VOA's that need to be cleaned up in a
    /// deferred way
    std::map<GLContextType, GLuint> m_vaoCleanupMap;
    /// An array of OpenGL Vertex Buffer Objects.
    GLuint m_vbo[NUM_BUFFERS];
    /// The number of vertices to draw.
    int m_numDrawElements;
    /// The shader program to draw the mesh with.
    Foundry::Katana::ViewerUtils::GLShaderProgram* m_customShader;
    /// The view matrix to pass to the shader program.
    FnAttribute::DoubleAttribute m_viewMatrixAttr;
    /// The projection matrix to pass to the shader program.
    FnAttribute::DoubleAttribute m_projectionMatrixAttr;
    /// The world matrix to pass to the shader program.
    FnAttribute::DoubleAttribute m_worldMatrixAttr;
    /// The color value to pass to the shader program (eg. for manipualtor handles).
    FnAttribute::FloatAttribute m_colorAttr;
};


#endif /* DRAWABLEMESH_H_ */
