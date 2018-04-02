// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef MANIPULATOR_MESH_H_
#define MANIPULATOR_MESH_H_

#include <vector>
#include <FnViewer/plugin/FnGLStateHelper.h>
#include <FnViewer/plugin/FnMathTypes.h>
#include <FnViewer/utils/FnDrawingHelpers.h>
#include <FnViewer/utils/FnImathHelpers.h>
#include <GL/glew.h>

using Foundry::Katana::ViewerAPI::Vec3f;

namespace Foundry
{
namespace Katana
{
namespace ViewerUtils
{


/**
 * @brief: An object that can be drawn on the Viewport.
 *
 * This can be used to draw Manipulator handle elements, such as lines, points
 * or meshes. It maintains a VAO and VBOs internally.
 */
class Drawable
{
public:

    /// Buffer types
    enum BUFFER {
        VERTEX_BUFFER = 0,
        NORMAL_BUFFER,
        INDEX_BUFFER,
        NUM_BUFFERS
    };

    /// Constructor
    Drawable()
    : m_isReady(false),
      m_vao(0),
      m_numVertices(0),
      m_numIndices(0),
      m_lineWidth(1),
      m_pointSize(1)
    {
        m_vbo[VERTEX_BUFFER] = 0;
        m_vbo[NORMAL_BUFFER] = 0;
        m_vbo[INDEX_BUFFER] = 0;
    }

    /// Destructor
    ~Drawable()
    {   
        if (m_vbo[VERTEX_BUFFER])
        {
            glDeleteBuffers(1, &m_vbo[VERTEX_BUFFER]);
        }

        if (m_vbo[NORMAL_BUFFER])
        {
            glDeleteBuffers(1, &m_vbo[NORMAL_BUFFER]);
        }

        if (m_vbo[INDEX_BUFFER])
        {
            glDeleteBuffers(1, &m_vbo[INDEX_BUFFER]);
        }

        if (glIsVertexArray(m_vao))
        {
            glDeleteVertexArrays(1, &m_vao);
        }
    }

    /// Tells if the Drawable is ready to be drawn (if setup has been called).
    bool isReady()
    {
        return m_isReady;
    }

    /// Sets up the vertex buffers
    void setup(
        const std::vector<Vec3f>& vertices,
        const std::vector<Vec3f>& normals,
        const std::vector<unsigned int>& indices)
    {
        if (isReady())
        {
            // Do not load it again
            return;
        }

        // No existing VAO found for this context
        GL_CMD(glGenVertexArrays(1, &m_vao));
        GL_CMD(glBindVertexArray(m_vao));

        GL_CMD(glGenBuffers(NUM_BUFFERS, &m_vbo[0]));

        // Fill the vertex buffer
        GL_CMD(glBindBuffer(GL_ARRAY_BUFFER, m_vbo[VERTEX_BUFFER]));
        GL_CMD(glBufferData(
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(Vec3f),
            vertices.data(),
            GL_STATIC_DRAW));
        GL_CMD(glVertexAttribPointer(
            VERTEX_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0));
        GL_CMD(glEnableVertexAttribArray(VERTEX_BUFFER));

        // Fill the normal buffer
        GL_CMD(glBindBuffer(GL_ARRAY_BUFFER, m_vbo[NORMAL_BUFFER]));
        GL_CMD(glBufferData(
            GL_ARRAY_BUFFER,
            normals.size() * sizeof(Vec3f),
            normals.data(),
            GL_STATIC_DRAW));
        GL_CMD(glVertexAttribPointer(
            NORMAL_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0));
        GL_CMD(glEnableVertexAttribArray(NORMAL_BUFFER));

        // Fill the index buffer
        GL_CMD(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[INDEX_BUFFER]));
        GL_CMD(glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int),
            indices.data(),
            GL_STATIC_DRAW));

        // Unbind buffers
        GL_CMD(glBindVertexArray(0));
        GL_CMD(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        GL_CMD(glBindBuffer(GL_ARRAY_BUFFER, 0));

        m_numVertices = vertices.size();
        m_numIndices = indices.size();
        m_isReady = true;
    }

    /// Update mesh vertices positions
    void updateVertices(const std::vector<Vec3f>& vertices)
    {
        if (!isReady())
        {
            // Do not update if it hasn't been created yet
            return;
        }

        // Update the vertex data
        GL_CMD(glBindBuffer(GL_ARRAY_BUFFER, m_vbo[VERTEX_BUFFER]));
        GL_CMD(glBufferData(
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(Vec3f),
            vertices.data(),
            GL_STATIC_DRAW));
        GL_CMD(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }

    /// Sets the width of the lines to be drawn.
    void setLineWidth(unsigned int width)
    {
        m_lineWidth = width;
    }

    /// Sets the width of the points to be drawn.
    void setPointWidth(unsigned int size)
    {
        m_pointSize = size;
    }

    /// Draws as a mesh
    void draw(bool withTransparency=true, bool cull=true)
    {
        if (!isReady())
        {
            // Not loaded yet, don't render
            return;
        }

        ViewerUtils::GLStateRestore glStateRestore(ViewerUtils::ColorBuffer |
                                                   ViewerUtils::Polygon);

        if (withTransparency)
        {
            // Enable alpha
            glEnable(GL_BLEND);             // ViewerUtils::ColorBuffer
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        // Update culling
        if (cull)
            glEnable(GL_CULL_FACE);         // ViewerUtils::Polygon
        else
            glDisable(GL_CULL_FACE);        // ViewerUtils::Polygon


        // Draw the VAO
        GL_CMD(glBindVertexArray(m_vao));
        GL_CMD(glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0));
        GL_CMD(glBindVertexArray(0));
    }

    /// Draws as lines
    void drawLines(bool withTransparency=true)
    {
        if (!isReady())
        {
            // Not loaded yet, don't render
            return;
        }

        ViewerUtils::GLStateRestore glStateRestore(ViewerUtils::ColorBuffer |
                                                   ViewerUtils::Line);

        if (withTransparency)
        {
            // Enable alpha
            glEnable(GL_BLEND);             // // ViewerUtils::ColorBuffer
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        glLineWidth(m_lineWidth);

        GL_CMD(glBindVertexArray(m_vao));
        GL_CMD(glDrawElements(GL_LINES, m_numIndices, GL_UNSIGNED_INT, 0));
        GL_CMD(glBindVertexArray(0));
    }

    /// Draws as a points
    void drawPoints(bool withTransparency=true)
    {
        if (!isReady())
        {
            // Not loaded yet, don't render
            return;
        }

        ViewerUtils::GLStateRestore glStateRestore(ViewerUtils::ColorBuffer |
                                                   ViewerUtils::Point);

        if (withTransparency)
        {
            // Enable alpha
            glEnable(GL_BLEND);             // ViewerUtils::ColorBuffer
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        glPointSize(m_pointSize);

        GL_CMD(glBindVertexArray(m_vao));
        GL_CMD(glDrawElements(GL_POINTS, m_numIndices, GL_UNSIGNED_INT, 0));
        GL_CMD(glBindVertexArray(0));
    }


private:
    bool m_isReady;
    GLuint m_vao;
    GLuint m_vbo[NUM_BUFFERS];
    unsigned int m_numVertices;
    unsigned int m_numIndices;
    unsigned int m_lineWidth;
    unsigned int m_pointSize;
};

}
}
}

#endif /* MANIPULATOR_MESH_H_ */
