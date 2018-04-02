// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "DrawableMesh.h"

#ifdef _WIN32
#include <FnPlatform/Windows.h>
#endif

#include <FnViewer/utils/FnImathHelpers.h>
#include <FnViewer/utils/FnDrawingHelpers.h>

#include <GL/glew.h>

#include <cstdlib>
#include <iostream>
#include <string>

using namespace Foundry::Katana::ViewerUtils;

DrawableMesh::DrawableMesh() : m_numDrawElements(0), m_customShader(NULL)
{
    m_vbo[VERTEX_BUFFER] = 0;
    m_vbo[INDEX_BUFFER] = 0;
    m_vbo[COLOR_BUFFER] = 0;
    m_vbo[TEXCOORD_BUFFER] = 0;
    m_vbo[NORMAL_BUFFER] = 0;
}

DrawableMesh::~DrawableMesh()
{
    clearGLResources();
}

void DrawableMesh::deleteShader()
{
    if (m_customShader)
        delete m_customShader;
    m_customShader = 0x0;
}

void DrawableMesh::clearGLResources()
{
    if(m_vbo[VERTEX_BUFFER])
    {
        glDeleteBuffers(1, &m_vbo[VERTEX_BUFFER]);
    }

    if(m_vbo[INDEX_BUFFER])
    {
        glDeleteBuffers(1, &m_vbo[INDEX_BUFFER]);
    }

    if(m_vbo[COLOR_BUFFER])
    {
        glDeleteBuffers(1, &m_vbo[COLOR_BUFFER]);
    }

    if(m_vbo[TEXCOORD_BUFFER])
    {
        glDeleteBuffers(1, &m_vbo[TEXCOORD_BUFFER]);
    }

    if(m_vbo[NORMAL_BUFFER])
    {
        glDeleteBuffers(1, &m_vbo[NORMAL_BUFFER]);
    }

    m_vaoCleanupMap = m_vaoMap;
    m_vaoMap.clear();

    m_vbo[VERTEX_BUFFER] = 0;
    m_vbo[INDEX_BUFFER] = 0;
    m_vbo[COLOR_BUFFER] = 0;
    m_vbo[TEXCOORD_BUFFER] = 0;
    m_vbo[NORMAL_BUFFER] = 0;

    m_numDrawElements = 0;

    if(!m_vaoCleanupMap.empty())
        cleanupVAO();

    // Delete the GLShader
    deleteShader();
}

void DrawableMesh::cleanupVAO()
{
#if defined(_WIN32)
    GLContextType context = wglGetCurrentContext();
#else
    GLContextType context = glXGetCurrentContext();
#endif
    std::map<GLContextType, GLuint>::iterator it = m_vaoCleanupMap.find(context);
    if (it != m_vaoCleanupMap.end())
    {
        // Clear the VAO
        const GLuint vao = it->second;
        if(glIsVertexArray(vao))
            glDeleteVertexArrays(1, &vao);

        m_vaoCleanupMap.erase(it);
    }
}

void DrawableMesh::setDrawAttribute(const std::string& name,
    FnAttribute::Attribute attr)
{
    if(name == "meshAttrs" && attr.isValid())
    {
        // The attribute should have "geometry" and "material" child attributes
        // to initialize the geometry and shader program.
        setupGeometry(attr);
        setupMaterials(attr);
    }
    else if(name == "viewMatrix")
    {
        FnAttribute::DoubleAttribute matrixAttr = attr;
        if(matrixAttr.isValid())
        {
            m_viewMatrixAttr = matrixAttr;
        }
    }
    else if(name == "projectionMatrix")
    {
        FnAttribute::DoubleAttribute matrixAttr = attr;
        if(matrixAttr.isValid())
        {
            m_projectionMatrixAttr = matrixAttr;
        }
    }
    else if(name == "worldMatrix")
    {
        FnAttribute::DoubleAttribute matrixAttr = attr;
        if(matrixAttr.isValid())
        {
            m_worldMatrixAttr = matrixAttr;
        }
    }
    else if(name == "color")
    {
        m_colorAttr = attr;
    }
}

GLuint DrawableMesh::getVAO()
{
    // Ensure old VAO's have been cleaned up
    if (!m_vaoCleanupMap.empty())
        cleanupVAO();

#if defined(_WIN32)
    GLContextType context = wglGetCurrentContext();
#else
    GLContextType context = glXGetCurrentContext();
#endif

    std::map<GLContextType, GLuint>::iterator mIt = m_vaoMap.find(context);
    if (mIt != m_vaoMap.end())
    {
        // Use a cached VAO for this context
        return mIt->second;
    }

    // No existing VAO found for this context
    GLuint vao = 0;
    GL_CMD(glGenVertexArrays(1, &vao));
    GL_CMD(glBindVertexArray(vao));

    // Fill the vertex buffer
    GL_CMD(glBindBuffer(GL_ARRAY_BUFFER, m_vbo[VERTEX_BUFFER]));
    GL_CMD(glVertexAttribPointer(VERTEX_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0));
    GL_CMD(glEnableVertexAttribArray(VERTEX_BUFFER)); // VertexPosition

    // Fill the color buffer
    GL_CMD(glBindBuffer(GL_ARRAY_BUFFER, m_vbo[COLOR_BUFFER]));
    GL_CMD(glVertexAttribPointer(COLOR_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0));
    GL_CMD(glEnableVertexAttribArray(COLOR_BUFFER)); // VertexColor

    // Fill the index buffer
    GL_CMD(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[INDEX_BUFFER]));

    // Unbind buffers
    GL_CMD(glBindVertexArray(0));
    GL_CMD(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GL_CMD(glBindBuffer(GL_ARRAY_BUFFER, 0));
    m_vaoMap[context] = vao;

    return vao;
}

void DrawableMesh::draw()
{
    if(!m_pointsAttr.isValid())
    {
        return;
    }

    // Get or create a VAO for this context
    GLuint vao = getVAO();
    if(!vao)
        return;

    if(m_customShader)
    {
        m_customShader->use();
        m_customShader->setUniform("ProjectionMatrix", m_projectionMatrixAttr);
        m_customShader->setUniform("ViewMatrix", m_viewMatrixAttr);
        m_customShader->setUniform("WorldMatrix", m_worldMatrixAttr);
        if(m_colorAttr.isValid())
        {
            m_customShader->setUniform("handleColor", m_colorAttr);
        }
    }

    // Draw the mesh
    GL_CMD(glBindVertexArray(vao));
    GL_CMD(glDrawElements(GL_TRIANGLES, m_numDrawElements, GL_UNSIGNED_INT, 0));
    GL_CMD(glBindVertexArray(0));
}

void DrawableMesh::pickerDraw(
    Foundry::Katana::ViewerUtils::GLShaderProgram& shaderProgram)
{
    if(!m_pointsAttr.isValid())
    {
        return;
    }

    // Get or create a VAO for this context
    GLuint vao = getVAO();
    if(!vao)
        return;

    shaderProgram.use();
    shaderProgram.setUniform("ProjectionMatrix", m_projectionMatrixAttr);
    shaderProgram.setUniform("ViewMatrix", m_viewMatrixAttr);
    shaderProgram.setUniform("WorldMatrix", m_worldMatrixAttr);
    shaderProgram.setUniform("Color", m_colorAttr);

    // Draw the mesh
    GL_CMD(glBindVertexArray(vao));
    GL_CMD(glDrawElements(GL_TRIANGLES, m_numDrawElements, GL_UNSIGNED_INT, 0));
    GL_CMD(glBindVertexArray(0));

}

void DrawableMesh::setupGeometry(const FnAttribute::GroupAttribute& attributes)
{
    if(!attributes.isValid())
    {
        return;
    }

    FnAttribute::FloatAttribute pointsAttr =
            attributes.getChildByName("geometry.point.P");
    if(!pointsAttr.isValid())
    {
        return;
    }

    FnAttribute::IntAttribute vertIndicesAttr =
                attributes.getChildByName("geometry.poly.vertexList");
    FnAttribute::IntAttribute faceStartAttr =
                attributes.getChildByName("geometry.poly.startIndex");

    // Check whether the point data attribute's hash is different from the cached
    // value
    if(m_pointsAttr.getHash() != pointsAttr.getHash())
    {
        m_pointsAttr = pointsAttr;

        clearGLResources();

        FnAttribute::FloatConstVector points = pointsAttr.getNearestSample(0.0f);
        FnAttribute::IntConstVector indices = vertIndicesAttr.getNearestSample(0.0f);
        FnAttribute::IntConstVector startIndices = faceStartAttr.getNearestSample(0.0f);

        std::vector<unsigned int> indexData;
        indexData.reserve(indices.size());
        unsigned int numElements = startIndices.size();

        for(unsigned int i = 0; i < numElements - 1; ++i)
        {
            int startIndex = startIndices[i];
            int faceVertices = startIndices[i+1] - startIndex;

            // Ideally this should support more types of face, but for
            // simplicity here we only support triangles.
            if(faceVertices == 3)
            {
                indexData.push_back(indices[startIndex]);
                indexData.push_back(indices[startIndex + 2]);
                indexData.push_back(indices[startIndex + 1]);
            }
            if(faceVertices == 4)
            {
                // Split the quad into two triangles
                indexData.push_back(indices[startIndex]);
                indexData.push_back(indices[startIndex + 2]);
                indexData.push_back(indices[startIndex + 1]);

                indexData.push_back(indices[startIndex + 0]);
                indexData.push_back(indices[startIndex + 3]);
                indexData.push_back(indices[startIndex + 2]);
            }
        }

        m_numDrawElements = indexData.size();

        // Prepare vertex data
        std::size_t dataSize = points.size() * sizeof(GLfloat);
        float* colorArray = new float[points.size()];
        for(unsigned int i=0; i < points.size() / 3; ++i)
        {
            colorArray[i * 3] = 0.7f;
            colorArray[(i * 3) + 1] = 0.7f;
            colorArray[(i * 3) + 2] = 0.7f;
        }

        GL_CMD(glGenBuffers(NUM_BUFFERS, &m_vbo[VERTEX_BUFFER]));

        // Fill the vertex buffer
        GL_CMD(glBindBuffer(GL_ARRAY_BUFFER, m_vbo[VERTEX_BUFFER]));
        GL_CMD(glBufferData(GL_ARRAY_BUFFER, dataSize, points.data(), GL_STATIC_DRAW));

        // Fill the color buffer
        GL_CMD(glBindBuffer(GL_ARRAY_BUFFER, m_vbo[COLOR_BUFFER]));
        GL_CMD(glBufferData(GL_ARRAY_BUFFER, dataSize, colorArray, GL_STATIC_DRAW));

        // Fill the index buffer
        GL_CMD(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[INDEX_BUFFER]));
        GL_CMD(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), &indexData[0],  GL_STATIC_DRAW));

        // Unbind buffers
        GL_CMD(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        GL_CMD(glBindBuffer(GL_ARRAY_BUFFER, 0));

        delete[] colorArray;
    }
}

void DrawableMesh::setupMaterials(const FnAttribute::GroupAttribute& attributes)
{
    if(!attributes.isValid())
    {
        return;
    }

    // The below attributes are the only type of material handling that the
    // example viewer employs at the moment. These attributes on a location
    // can be set to contain the source for the various GLSL shader stages,
    // or left empty and the defaults will be used.
    FnAttribute::StringAttribute fragShaderAttr =
            attributes.getChildByName("material.viewerShaderSource");
    FnAttribute::StringAttribute vertexShaderAttr =
            attributes.getChildByName("material.viewerVertexShaderSource");
    FnAttribute::StringAttribute geometryShaderAttr =
            attributes.getChildByName("material.viewerGeometryShaderSource");
    FnAttribute::StringAttribute tessControlShaderAttr =
            attributes.getChildByName("material.viewerTessControlShaderSource");
    FnAttribute::StringAttribute tessEvalShaderAttr =
            attributes.getChildByName("material.viewerTessEvalShaderSource");

    std::string shadersPath = getenv("KATANA_ROOT");
    if (!shadersPath.empty())
        shadersPath.append("/plugin_apis/Resources/Shaders/FnViewer/");

    // Delete any existing shader program
    if(m_customShader)
    {
        delete m_customShader;
    }
    m_customShader = new Foundry::Katana::ViewerUtils::GLShaderProgram();

    // Set up the shader program either using defaults or those specified from
    // the attributes
    if(vertexShaderAttr.isValid())
    {
        m_customShader->compileShader("CustomVertexShader",
            Foundry::Katana::ViewerUtils::VERTEX,
            vertexShaderAttr.getValue("", false));
    }
    else
    {
        m_customShader->compileShader(shadersPath + "basic.vert",
            Foundry::Katana::ViewerUtils::VERTEX);
    }

    if(fragShaderAttr.isValid())
    {
        m_customShader->compileShader("CustomFragmentShader",
            Foundry::Katana::ViewerUtils::FRAGMENT,
            fragShaderAttr.getValue("", false));
    }
    else
    {
        m_customShader->compileShader(shadersPath + "basic.frag",
            Foundry::Katana::ViewerUtils::FRAGMENT);

    }

    if(geometryShaderAttr.isValid())
    {
        m_customShader->compileShader("CustomGeometryShader",
            Foundry::Katana::ViewerUtils::GEOMETRY,
            geometryShaderAttr.getValue("", false));
    }
    if(tessControlShaderAttr.isValid())
    {
        m_customShader->compileShader("CustomTessControlShader",
            Foundry::Katana::ViewerUtils::TESS_CONTROL,
            tessControlShaderAttr.getValue("", false));
    }
    if(tessEvalShaderAttr.isValid())
    {
        m_customShader->compileShader("CustomTessEvalShader",
            Foundry::Katana::ViewerUtils::TESS_EVALUATION,
            tessEvalShaderAttr.getValue("", false));
    }

    m_customShader->link();
}

bool DrawableMesh::isValid() const
{
    return m_pointsAttr.isValid();
}
