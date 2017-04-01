// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <math.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <cassert>
#include <stddef.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include <FnViewerModifier/plugin/FnViewerModifier.h>
#include <FnViewerModifier/plugin/FnViewerModifierInput.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnAttribute/FnAttribute.h>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathBox.h>

#include "BatchViewerModifierUtils.h"


class BatchViewerModifier : public FnKat::ViewerModifier
{
    /**
     * @brief The MeshData struct stores attributes and data used to
     *    generate buffers. The attributes could be used to check if data
     *    changed.
     */
    struct MeshData
    {
        FnKat::FloatAttribute verticesDataAttr;
        FnKat::IntAttribute indicesAttr;

        /// The id of a mesh is the index in the command vector.
        size_t index;

        MeshData(FnKat::FloatAttribute vertDataAttr,
                 FnKat::IntAttribute indAttr,
                 size_t i)
            : verticesDataAttr(vertDataAttr),
              indicesAttr(indAttr),
              index(i)
        {
        }
    };

    struct DrawElementsIndirectCommand
    {
        GLuint count;
        GLuint instanceCount;
        GLuint firstIndex;
        GLuint baseVertex;
        GLuint baseInstance;
    };

    typedef std::map<std::string, MeshData> MeshMap;

    static const int MAX_VERTEX_COUNT = 100000000;
    static const int MAX_INDICES_COUNT = 10000000;
    static const size_t MAX_OBJECTS = 100000;

    static const std::string VERTEX_SHADER;
    static const std::string FRAGMENT_SHADER;

    static size_t lastVertexOffset;
    static size_t lastIndexOffset;


    static std::map<std::string, MeshData> locationMap;

    static std::vector<DrawElementsIndirectCommand> commands;

    static GLuint vboId;
    static GLuint iboId;
    static GLuint xformBufferId;
    static GLuint normalsXFormsBufferId;
    static GLuint worldXFormsBufferId;

    static GLuint vertexShader;
    static GLuint fragmentShader;

    static GLuint shaderProgram;

    static GLint mvpLocation;

    /// Model-View-Projection matrix.
    static Matrix44 mvp;
    /// Object to world matrix.
    static Matrix44 world;
    /// View-Projection matrix.
    static Matrix44 vp;

    static Matrix44 MVPS[MAX_OBJECTS];
    static Matrix44 worldTransforms[MAX_OBJECTS];
    static Matrix33 normalTransforms[MAX_OBJECTS];

    static GLint positionID;
    static GLint normalID;
    static GLint xformsID;
    static GLint normalsXFormsID;
    static GLint worldXFormsID;
    static GLint cameraPosID;

    static V3f cameraPositionWorld;
public:
    explicit BatchViewerModifier(FnKat::GroupAttribute args)
        : FnKat::ViewerModifier(args)
        , index(-1)
    {
    }

    static FnKat::ViewerModifier* create(FnKat::GroupAttribute args)
    {
        return (FnKat::ViewerModifier*)new BatchViewerModifier(args);
    }

    static FnKat::GroupAttribute getArgumentTemplate()
    {
        FnKat::GroupBuilder gb;
        return gb.build();
    }

    static const char* getLocationType()
    {
        return "customVMP";
    }

    static void onFrameBegin() { }

    static void HideAllInstances()
    {
        for (std::vector<DrawElementsIndirectCommand>::iterator it =
                commands.begin();
             it != commands.end();
             ++it)
        {
            it->instanceCount = 0;
        }
    }

    /**
     * This function is called once, at the end of each frame,
     * when all 'draw()' methods have been called on all VMP's instances
     * attached to all expanded and visible locations.
     *
     * This is where we can draw the batches, using the information on
     * geometry and visible objects we accumulated during 'setup()' and
     * 'draw()' callbacks.
     */
    static void onFrameEnd()
    {
        // Setup state
        glPushAttrib(GL_POLYGON_BIT | GL_LIGHTING_BIT | GL_LINE_BIT);
        glPushMatrix();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Bind resources
        glUseProgram(shaderProgram);

        glUniform3f(cameraPosID, cameraPositionWorld.x, cameraPositionWorld.y,
                    cameraPositionWorld.z);

        glBindBuffer(GL_ARRAY_BUFFER, vboId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

        glVertexAttribPointer(positionID, 4, GL_FLOAT, GL_FALSE,
                              sizeof(VertexNormalData),
                              reinterpret_cast<float*>(0));
        glEnableVertexAttribArray(positionID);

        glVertexAttribPointer(normalID, 4, GL_FLOAT, GL_FALSE,
                              sizeof(VertexNormalData),
                              reinterpret_cast<float*>(16));
        glEnableVertexAttribArray(normalID);

        // Setup all per instance MVP transforms.
        glBindBuffer(GL_ARRAY_BUFFER, xformBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(MVPS), &MVPS[0], GL_DYNAMIC_DRAW);

        for (size_t i = 0; i < 4; ++i)
        {
            glVertexAttribPointer(xformsID + i, 4, GL_FLOAT, GL_FALSE,
                                  sizeof(GLfloat) * 16,
                                  reinterpret_cast<GLvoid*>(
                                      sizeof(GLfloat) * 4 * i));
            glEnableVertexAttribArray(xformsID + i);
            glVertexAttribDivisor(xformsID + i, 1);
        }

        // Setup all per instance world transforms.
        glBindBuffer(GL_ARRAY_BUFFER, worldXFormsBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(worldTransforms),
                     &worldTransforms[0], GL_DYNAMIC_DRAW);

        for (size_t i = 0; i < 4; ++i)
        {
            glVertexAttribPointer(worldXFormsID + i, 4, GL_FLOAT, GL_FALSE,
                                  sizeof(GLfloat) * 16,
                                  reinterpret_cast<GLvoid*>(
                                      sizeof(GLfloat) * 4 * i));
            glEnableVertexAttribArray(worldXFormsID + i);
            glVertexAttribDivisor(worldXFormsID + i, 1);
        }

        // Setup all per instance normal transforms.
        glBindBuffer(GL_ARRAY_BUFFER, normalsXFormsBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(normalTransforms),
                     &normalTransforms[0], GL_DYNAMIC_DRAW);

        for (size_t i = 0; i < 3; ++i)
        {
            glVertexAttribPointer(normalsXFormsID + i, 3, GL_FLOAT, GL_FALSE,
                                  sizeof(GLfloat) * 9,
                                  reinterpret_cast<GLvoid*>(
                                      sizeof(GLfloat) * 3 * i));
            glEnableVertexAttribArray(normalsXFormsID + i);
            glVertexAttribDivisor(normalsXFormsID + i, 1);
        }

        // Draw the batch.
        glMultiDrawElementsIndirect(GL_TRIANGLES,
            GL_UNSIGNED_INT,
            commands.data(),
            commands.size(),
            0);

        // Clean
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glDisableVertexAttribArray(positionID);
        glDisableVertexAttribArray(normalID);

        for (size_t i = 0; i < 4; ++i)
        {
            glDisableVertexAttribArray(xformsID + i);
            glDisableVertexAttribArray(worldXFormsID + i);
            glDisableVertexAttribArray(normalsXFormsID + i);
        }
        glDisable(GL_CULL_FACE);

        HideAllInstances();

        glPopMatrix();
        glPopAttrib();
    }

    void deepSetup(FnKat::ViewerModifierInput& input)
    {
        // Draw only the VMP representation in the viewer
        input.overrideHostGeometry();

        mvp = GetMVP(input);
        vp = GetVP(input);
        world = GetWorld(input);

        MVPS[index] = mvp;
        Matrix33 rot = Matrix33(world[0][0], world[0][1], world[0][2],
                                world[1][0], world[1][1], world[1][2],
                                world[2][0], world[2][1], world[2][2]);
        normalTransforms[index] = rot.inverse().transpose();
        worldTransforms[index] = world;

        Matrix44 viewInverse =
            DoubleAttrToFloatMatrix(input.getDisplayMatrix(VIEW)).inverse();

        cameraPositionWorld = viewInverse.translation();
    }

    /**
     * @brief Initializes vertex and index buffer objects.
     */
    void InitBuffers()
    {
        glGenBuffers(1, &vboId);
        glGenBuffers(1, &iboId);
        glGenBuffers(1, &xformBufferId);
        glGenBuffers(1, &normalsXFormsBufferId);
        glGenBuffers(1, &worldXFormsBufferId);

        glBindBuffer(GL_ARRAY_BUFFER, vboId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)* MAX_VERTEX_COUNT,
                     0, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(unsigned int)* MAX_INDICES_COUNT, 0,
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, xformBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16 * MAX_OBJECTS,
                     &MVPS[0], GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, normalsXFormsBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 9 * MAX_OBJECTS,
                     &normalTransforms[0], GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, worldXFormsBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16 * MAX_OBJECTS,
                     &worldTransforms[0], GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void InitResources()
    {
        // we still don't have allocated a vbo
        if (vboId == 0)
        {
            const char * version =
                reinterpret_cast<const char *>(glGetString(GL_VERSION));
            std::cout << "GL_VERSION: " << version << std::endl;

            // Glew init isn't supposed to be called directly by VMP code.
            // Katana is taking care of initializing extensions. Current
            // Glew version shipped with Katana is 1.5 which it doesn't
            // contains glMultiDrawElementsIndirect method that is used in this
            // example.
            glewInit();

            InitShaders();
            InitUniforms();
            InitBuffers();
        }
    }

    void setup(FnKat::ViewerModifierInput& input)
    {
        locationName = input.getFullName();

        InitResources();

        MeshMap::iterator meshIt =
            BatchViewerModifier::locationMap.find(locationName);

        if (meshIt != BatchViewerModifier::locationMap.end())
        {
            // We already loaded data for the given location.
            // We should check if the geometry attribute
            // didn't change and eventually update VBO/IBO.

            // VMP instance may have been deleted and rebuilt.
            if (index < 0)
                index = meshIt->second.index;

            commands[index].instanceCount = 1;

            FnKat::FloatAttribute verticesAttr =
                input.getAttribute("geometry.point.P");
            if (!verticesAttr.isValid())
            {
                return;
            }

            if (meshIt->second.verticesDataAttr != verticesAttr)
            {
                // Vertices data changed. We should update the VBO.
            }
            else
            {
                FnKat::FloatConstVector pVect =
                    verticesAttr.getNearestSample(0.0f);
                CalculateBoundingBox(pVect, localBBox);
            }
        }
        else
        {
            FnKat::FloatAttribute verticesAttr =
                input.getAttribute("geometry.point.P");
            if (!verticesAttr.isValid())
            {
                return;
            }

            FnKat::IntAttribute vertexListAttr =
                input.getAttribute("geometry.poly.vertexList");
            if (!vertexListAttr.isValid())
            {
                return;
            }

            FnKat::IntAttribute startIndexAttr =
                input.getAttribute("geometry.poly.startIndex");
            if (!startIndexAttr.isValid())
            {
                return;
            }

            FnKat::IntConstVector vertexList =
                vertexListAttr.getNearestSample(0.0f);
            FnKat::IntConstVector startIndexList =
                startIndexAttr.getNearestSample(0.0f);

            // Build the index list. We may update the indices if we read quads.
            std::vector<unsigned int> indices;
            BuildIndexData(vertexList, startIndexList, indices);

            // We use the draw command index as id for the given location.
            index = commands.size();
            // We keep track of relevant attributes. It's useful to cache
            // geometry data and see if anything changed.
            locationMap.insert(
                std::make_pair(locationName,
                               MeshData(verticesAttr, vertexListAttr, index)));

            FnKat::FloatConstVector pVect = verticesAttr.getNearestSample(0.0f);

            Matrix44 objToWorld = GetWorld(input);
            MVPS[index] = objToWorld;

            FnKat::FloatAttribute normalsAttr =
                input.getAttribute("geometry.point.N");

            // Build a vector of VertexNormalData with interleaved vertex and
            // normal vectors.
            std::vector<VertexNormalData> interleavedData;
            if (normalsAttr.isValid())
            {
                FnKat::FloatConstVector pNorm =
                    normalsAttr.getNearestSample(0.0f);
                BuildInterleavedVertexBufferData(pVect, pNorm, interleavedData,
                                                 localBBox);
            }
            else
            {
                BuildInterleavedVertexBufferData(pVect, interleavedData,
                                                 localBBox);
            }

            // Update buffers with vertices and indices of the current location.
            glBindBuffer(GL_ARRAY_BUFFER, vboId);
            glBufferSubData(GL_ARRAY_BUFFER,
                            sizeof(VertexNormalData)* lastVertexOffset,
                            sizeof(VertexNormalData)* interleavedData.size(),
                            interleavedData.data());


            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
                            lastIndexOffset * sizeof (unsigned int),
                            sizeof(unsigned int)* indices.size(),
                            indices.data());

            // Add a new command for indirect drawing.
            struct DrawElementsIndirectCommand cmd;
            cmd.count = indices.size();
            cmd.instanceCount = 1;
            cmd.firstIndex = lastIndexOffset;
            cmd.baseVertex = lastVertexOffset;
            cmd.baseInstance = commands.size();

            commands.push_back(cmd);

            // Keep track of how many vertices and indices read so far.
            lastIndexOffset += indices.size();
            lastVertexOffset +=  interleavedData.size();
        }
    }

    void draw(FnKat::ViewerModifierInput& input)
    {
        // Ignore the current location if it hasn't been setup.
        if (index < 0)
            return;

        assert(commands.size() > index);

        // Draw is called if the current instance hasn't been culled.
        commands[index].instanceCount = 1;
    }

    void cleanup(FnKat::ViewerModifierInput& input)
    {
        if (index != -1)
        {
            commands[index].instanceCount = 0;
        }
    }

    void deepCleanup(FnKat::ViewerModifierInput& input)
    {
    }

    FnKat::DoubleAttribute getWorldSpaceBoundingBox(
            FnKat::ViewerModifierInput& input)
    {
        Matrix44 objToWorld = GetWorld(input);

        V3f min_WS = localBBox.min * objToWorld;
        V3f max_WS = localBBox.max * objToWorld;

        double data[6];

        data[0] = min_WS.x;
        data[1] = max_WS.x;

        data[2] = min_WS.y;
        data[3] = max_WS.y;

        data[4] = min_WS.z;
        data[5] = max_WS.z;

        return FnKat::DoubleAttribute(data, 6, 1);
    }

    FnKat::DoubleAttribute getLocalSpaceBoundingBox(
            FnKat::ViewerModifierInput& input)
    {
        double data[6];

        data[0] = localBBox.min.x;
        data[1] = localBBox.max.x;

        data[2] = localBBox.min.y;
        data[3] = localBBox.max.y;

        data[4] = localBBox.min.z;
        data[5] = localBBox.max.z;

        return FnKat::DoubleAttribute(data, 6, 1);
    }

    static void flush() {}

private:
    /**
     * @brief Initialize compiling and linking shaders.
     * @return True if initialization of shaders succeded.
     */
    static bool InitShaders()
    {
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        const GLchar *vertexSource = VERTEX_SHADER.c_str();
        glShaderSource(vertexShader, 1, &vertexSource, 0);
        glCompileShader(vertexShader);

        if (!CheckShaderCompilationStatus(vertexShader))
        {
            return false;
        }

        const GLchar *fragmentSource =  FRAGMENT_SHADER.c_str();

        glShaderSource(fragmentShader, 1, &fragmentSource, 0);
        glCompileShader(fragmentShader);

        if (!CheckShaderCompilationStatus(fragmentShader))
        {
            return false;
        }

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);

        glBindAttribLocation(shaderProgram, 0, "vertexIn");
        glBindAttribLocation(shaderProgram, 1, "normalIn");

        glLinkProgram(shaderProgram);

        return CheckShaderLinking(shaderProgram,
                                  vertexShader,
                                  fragmentShader);
    }

    static void InitUniforms()
    {
        positionID = 0;
        normalID = 1;
        xformsID = 2;
        worldXFormsID = 6;
        normalsXFormsID = 10;

        mvpLocation = glGetUniformLocation(shaderProgram, "MVP");
        cameraPosID =  glGetUniformLocation(shaderProgram, "cameraPos");
    }

    std::string locationName;
    Box3f localBBox;
    int index;
};

GLuint BatchViewerModifier::vboId = 0;
GLuint BatchViewerModifier::iboId = 0;
GLuint BatchViewerModifier::xformBufferId = 0;
GLuint BatchViewerModifier::normalsXFormsBufferId = 0;
GLuint BatchViewerModifier::worldXFormsBufferId = 0;

GLuint BatchViewerModifier::vertexShader = 0;
GLuint BatchViewerModifier::fragmentShader = 0;
GLuint BatchViewerModifier::shaderProgram = 0;


GLint BatchViewerModifier::mvpLocation = 0;
GLint BatchViewerModifier::cameraPosID = 0;
V3f BatchViewerModifier::cameraPositionWorld = V3f(0.0, 0.0, 0.0);

std::map<std::string, BatchViewerModifier::MeshData>
    BatchViewerModifier::locationMap;

std::vector<BatchViewerModifier::DrawElementsIndirectCommand>
    BatchViewerModifier::commands;

Matrix44 BatchViewerModifier::mvp;
Matrix44 BatchViewerModifier::world;
Matrix44 BatchViewerModifier::vp;
Matrix44 BatchViewerModifier::MVPS[BatchViewerModifier::MAX_OBJECTS];
Matrix33 BatchViewerModifier::normalTransforms
    [BatchViewerModifier::MAX_OBJECTS];
Matrix44 BatchViewerModifier::worldTransforms
    [BatchViewerModifier::MAX_OBJECTS];


size_t BatchViewerModifier::lastVertexOffset = 0;
size_t BatchViewerModifier::lastIndexOffset = 0;

GLint BatchViewerModifier::positionID;
GLint BatchViewerModifier::normalID;
GLint BatchViewerModifier::xformsID;
GLint BatchViewerModifier::normalsXFormsID;
GLint BatchViewerModifier::worldXFormsID;

const std::string BatchViewerModifier::VERTEX_SHADER =
        "#version 420\n"
        "layout(location = 0) in vec4 vertexIn;"
        "layout(location = 1) in vec4 normalIn;"
        "layout(location = 2) in mat4 MVP;"
        "layout(location = 6) in mat4 model2World;"
        "layout(location = 10) in mat3 normal2World;"
        "uniform vec3 cameraPos = vec3(1,1,0);"
        "out vec3 normal_WS;"
        "out vec3 pos_WS;"
        "in int gl_InstanceID;"

        "void main(void)"
        "{"
        "gl_Position = MVP * vec4(vertexIn.xyz,1.0);"
        "normal_WS = normalize(normal2World * normalIn.xyz);"
        "pos_WS = (model2World * vec4(vertexIn.xyz,1.0)).xyz;"
        "float id = float(gl_InstanceID);"
        "}";

const std::string BatchViewerModifier::FRAGMENT_SHADER =
        "#version 420\n"
        "out vec4 gl_FragColor;"
        "in vec3 normal_WS;"
        "in vec3 pos_WS;"
        "uniform vec3 cameraPos = vec3(1,1,0);"
        "uniform vec3 lightDir = vec3(10.0, 0.0, 0.0);"
        "uniform vec3 lightColor = vec3(0.6, 0.6, 0.6);"
        "uniform vec3 color = vec3(0.215, 0.639, 0.313);"
        "uniform vec3 ambientColor = vec3(1.0, 1.0, 1.0);"
        "uniform vec3 kA = vec3(0.2, 0.2, 0.2);"
        "uniform vec3 kD = vec3(0.5, 0.5, 0.5);"
        "uniform vec3 kS = vec3(0.8, 0.8, 0.8);"
        "uniform float shininess = 20.0;"
        "void main (void) "
        "{"
        "vec3 n = normalize(normal_WS);"
        "vec3 lightDir_WS = normalize(lightDir);"
        "vec3 ambient = color * ambientColor * kA;"
        "float n_dot_l = max(dot(lightDir_WS, n),0);"
        "vec3 diffuse = color * kD * lightColor * n_dot_l;"
        "vec3 viewDir = normalize(cameraPos - pos_WS);"
        "vec3 h = normalize(lightDir_WS + viewDir);"
        "float n_dot_h = dot(n,h);"
        "vec3 specular = kS * lightColor * pow(max(n_dot_h,0.0), shininess);"
        "gl_FragColor = vec4(ambient+ diffuse + specular, 1.0);"
        "}";

DEFINE_VMP_PLUGIN(BatchViewerModifier)


void registerPlugins()
{
    REGISTER_PLUGIN(BatchViewerModifier, "BatchViewerModifier", 0, 1);
}
