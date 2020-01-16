#ifndef BATCHVIEWERMODIFIERUTILS_H
#define BATCHVIEWERMODIFIERUTILS_H

#include <vector>

#include <FnViewerModifier/plugin/FnViewerModifier.h>
#include <FnViewerModifier/plugin/FnViewerModifierInput.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnAttribute.h>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathBox.h>


namespace
{

typedef Imath::V3f V3f;
typedef Imath::V4f V4f;
typedef Imath::Matrix44<float> Matrix44;
typedef Imath::Matrix33<float> Matrix33;
typedef Imath::Box3f Box3f;

/**
 * @brief Converts a double attribute to 4x4 matrix.
 * @param matrixAttr The double attribute to read matrix data from.
 * @return The matrix built using the given attribute.
 */
Matrix44 DoubleAttrToFloatMatrix(
    const FnAttribute::DoubleAttribute &matrixAttr)
{
    FnAttribute::DoubleAttribute::array_type
        matrixData = matrixAttr.getNearestSample(0.0);

    const double *m = matrixData.data();

    return Matrix44(m[0], m[1], m[2], m[3],
                    m[4], m[5], m[6], m[7],
                    m[8], m[9], m[10], m[11],
                    m[12], m[13], m[14], m[15]);
}

/**
 * @brief Returns the model-view-projection matrix.
 * @param input VMP's input interface.
 * @return The model-view-projection matrix.
 */
Matrix44 GetMVP(const FnKat::ViewerModifierInput& input)
{
    Matrix44 view =
        DoubleAttrToFloatMatrix(input.getDisplayMatrix(VIEW));
    Matrix44 proj =
        DoubleAttrToFloatMatrix(input.getDisplayMatrix(PROJECTION));
    Matrix44 world =
        DoubleAttrToFloatMatrix(input.getLiveWorldSpaceXform());

    return  world * view * proj;
}

/**
 * @brief Returns the view-projection matrix.
 * @param input VMP's input interface.
 * @return The view-projection matrix.
 */
Matrix44 GetVP(const FnKat::ViewerModifierInput& input)
{
    Matrix44 view =
        DoubleAttrToFloatMatrix(input.getDisplayMatrix(VIEW));
    Matrix44 proj =
        DoubleAttrToFloatMatrix(input.getDisplayMatrix(PROJECTION));

    return  view * proj;
}

/**
 * @brief Returns object to world space transformation matrix.
 * @param input VMP's input interface.
 * @return The object to world space matrix.
 */
Matrix44 GetWorld(const FnKat::ViewerModifierInput& input)
{
    return DoubleAttrToFloatMatrix(input.getWorldSpaceXform());
}

/**
 * @brief Checks if any OpenGL error occured and prints it.
 * @return True if an error occured, otherwise false.
 */
bool CheckGLError()
{
    GLenum error = glGetError();

    if (error == GL_NO_ERROR)
        return false;

    switch (error)
    {
    case GL_INVALID_ENUM:
        std::cout << "Error: GL_INVALID_ENUM" << std::endl;
        break;
    case GL_INVALID_VALUE:
        std::cout << "Error: GL_INVALID_VALUE" << std::endl;
        break;
    case GL_INVALID_OPERATION:
        std::cout << "Error: GL_INVALID_OPERATION" << std::endl;
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        std::cout << "Error: GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
        break;
    case GL_OUT_OF_MEMORY:
        std::cout << "Error: GL_OUT_OF_MEMORY" << std::endl;
        break;
    case GL_STACK_UNDERFLOW:
        std::cout << "Error: GL_STACK_UNDERFLOW" << std::endl;
        break;
    case GL_STACK_OVERFLOW:
        std::cout << "Error: GL_STACK_OVERFLOW" << std::endl;
        break;
    default:
        break;
    }
    return true;
}

/**
 * @brief CheckShaderCompilationStatus
 * @param shaderId The shader to check for compile errors.
 * @return True if the given shader has been compiled successfully,
 *     otherwise False and releases the allocated shader.
 */
static bool CheckShaderCompilationStatus(GLuint shaderId)
{
    GLint isCompiled = 0;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled);

    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(shaderId, maxLength, &maxLength, &infoLog[0]);

        glDeleteShader(shaderId);

        std::cout << "Shader compilation error: " << std::endl;
        std::cout << infoLog.data() << std::endl;

        return false;
    }
    return true;
}

/**
 * @brief Checks if the given program is correctly linked. If a link error
 *     occured it cleans the relative allocated resources.
 * @param shaderProgram The program to check for linking errors.
 * @param vertShader The vertex shader linked in the given program.
 * @param fragmentShader The fragment shader linked in the given program.
 * @return True if the given program has been linked successfully,
 *     otherwise False and releases the relative allocated shader resources.
 */
static bool CheckShaderLinking(GLuint shaderProgram, GLuint vertexShader,
                               GLuint fragmentShader)
{
    GLint isLinked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);

    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);

        glDeleteProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        std::cout << "Shader link error:" << std::endl;
        std::cout << infoLog.data() << std::endl;

        return false;
    }
    return true;
}

/**
 * @brief The VertexNormalData struct holds vertex position and normals data.
 */
struct VertexNormalData
{
    float vertex[4];
    float normal[4];

    VertexNormalData(float vX, float vY, float vZ,
                     float nX, float nY, float nZ)
    {
        vertex[0] = vX;
        vertex[1] = vY;
        vertex[2] = vZ;
        vertex[3] = 0.0f;

        normal[0] = nX;
        normal[1] = nY;
        normal[2] = nZ;
        normal[3] = 0.0f;
    }

    VertexNormalData(float vX, float vY, float vZ)
    {
        vertex[0] = vX;
        vertex[1] = vY;
        vertex[2] = vZ;
        vertex[3] = 0.0f;

        normal[0] = normal[1] = normal[2] = normal[3] = 0.0f;
    }
};

/**
 * @brief Builds a vector of vertex attributes with interleaved positions and
 *     normals. The relative local space bounding box is updated accordingly.
 *
 * @param vertices List of vertices in local space.
 * @param normals List of normals in local space.
 * @param data The reference to vertex attribute vector to be filled with
 *     interleaved data.
 * @param localBoundingBox The local bounding box to be updated while reading
 *     vertices.
 */
void BuildInterleavedVertexBufferData(FnKat::FloatConstVector vertices,
                                      FnKat::FloatConstVector normals,
                                      std::vector<VertexNormalData>& data,
                                      Box3f& localBoundingBox
                                      )
{
    assert(vertices.size() == normals.size());

    for (size_t i = 0; i < vertices.size(); i += 3)
    {
        localBoundingBox.extendBy(
            V3f(vertices[i], vertices[i + 1], vertices[i + 2]));

        data.push_back(
            VertexNormalData(
                vertices[i], vertices[i + 1], vertices[i + 2],
                normals[i], normals[i + 1], normals[i + 2]));
    }
}

/**
 * @brief Builds a vector of vertex attributes with interleaved positions and
 *     normals. The normals are currently not calculated but initialized with
 *     a default value.
 *     The relative local space bounding box is updated accordingly.
 *
 * @param vertices List of vertices in local space.
 * @param data The reference to vertex attribute vector to be filled with
 *     interleaved data.
 * @param localBoundingBox The local bounding box to be updated while reading
 *     vertices.
 */
void BuildInterleavedVertexBufferData(
     FnKat::FloatConstVector vertices,
     std::vector<VertexNormalData>& data,
     Box3f& localBoundingBox)
{
    for (size_t i = 0; i < vertices.size(); i += 3)
    {
        // Let's use a place-holder normal for now. This should be calculated
        // properly.
        V3f fakeNormal(vertices[i], vertices[i + 1], vertices[i + 2]);
        fakeNormal.normalize();

        localBoundingBox.extendBy(
            V3f(vertices[i], vertices[i + 1], vertices[i + 2]));
        data.push_back(
            VertexNormalData(
                vertices[i], vertices[i + 1], vertices[i + 2],
                fakeNormal.x, fakeNormal.y, fakeNormal.z));
    }
}

/**
 * Fills the indices vector with indices suitable for drawing a
 * buffer of triangles.
 *
 * @param vertexList The original index list of triangles or quads.
 * @param startIndexList The vector of faces.
 * @param indices The vector to be filled with the triangles indices.
 */
void BuildIndexData(FnKat::IntConstVector vertexList,
                    FnKat::IntConstVector startIndexList,
                    std::vector<unsigned int>& indices)
{
    size_t iboIndex = 0;
    for (size_t polyIndex = 0;
         polyIndex < startIndexList.size() - 1;
         ++polyIndex)
    {
        size_t faceNum = startIndexList[polyIndex + 1] -
                         startIndexList[polyIndex];

        assert(faceNum == 3 || faceNum == 4);

        if (faceNum == 4)
        {
            // If reading a quad face we should split it into 2
            // triangles.
            indices.push_back(vertexList[iboIndex + 1]);
            indices.push_back(vertexList[iboIndex]);
            indices.push_back(vertexList[iboIndex + 2]);

            indices.push_back(vertexList[iboIndex]);
            indices.push_back(vertexList[iboIndex + 3]);
            indices.push_back(vertexList[iboIndex + 2]);
        }
        else
        {
            indices.push_back(vertexList[iboIndex + 1]);
            indices.push_back(vertexList[iboIndex]);
            indices.push_back(vertexList[iboIndex + 2]);
        }

        iboIndex += faceNum;
    }
}

/**
 * Calculates the bounding box using the given vertices.
 * @param vertices A vector of vertices to calculate the bounding box for.
 * @param localBoundingBox The calculated bounding box.
 */
void CalculateBoundingBox(
     FnKat::FloatConstVector vertices,
     Box3f& localBoundingBox)
{
    for (size_t i = 0; i < vertices.size(); i += 3)
    {
        localBoundingBox.extendBy(
            V3f(vertices[i], vertices[i + 1], vertices[i + 2]));
    }
}

}  // namespace

#endif  // BATCHVIEWERMODIFIERUTILS_H
