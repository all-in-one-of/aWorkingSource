// Copyright (c) 2012-2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifdef _WIN32
#include <FnPlatform/Windows.h>
#endif

#include <iostream>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <OpenEXR/ImathFun.h>
#include <OpenEXR/ImathVec.h>

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>
#include <FnViewerModifier/plugin/FnViewerModifier.h>
#include <FnViewerModifier/plugin/FnViewerModifierInput.h>

using namespace Foundry::Katana;

class VolumeViewerModifier : public ViewerModifier
{
public:

    VolumeViewerModifier(GroupAttribute args) : ViewerModifier(args)
    {
    }

    ~VolumeViewerModifier()
    {
    }

    static ViewerModifier* create(GroupAttribute args)
    {
        return (ViewerModifier*)new VolumeViewerModifier(args);
    }

    static GroupAttribute getArgumentTemplate()
    {
        GroupBuilder gb;
        return gb.build();
    }

    static const char* getLocationType()
    {
        return "volume";
    }

    virtual void deepSetup(ViewerModifierInput& input)
    {
    }

    virtual void setup(ViewerModifierInput& input)
    {
        // Save a reference to the 'bound' and 'geometry.voxelResolution' attributes
        m_boundAttr = input.getAttribute("bound");
        m_voxelResAttr = input.getAttribute("geometry.voxelResolution");
        DoubleAttribute radiusAttr = input.getAttribute("geometry.radius");

        // (re)build point data used for drawing
        buildPointData();

        // Get the sphere radius
        m_hasRadius = false;
        if (radiusAttr.isValid())
        {
            m_hasRadius = true;
            m_radius = radiusAttr.getValue();
        }
    }

    virtual void draw(ViewerModifierInput& input)
    {

        glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);

        glDisable(GL_LIGHTING);

        glEnableClientState(GL_VERTEX_ARRAY);

        // Box
        glColor3fv(input.isSelected() ? g_selectedColor : g_boundColor);
        glVertexPointer(3, GL_DOUBLE, 0, m_boxPoints.data());
        glDrawArrays(GL_LINES, 0, m_boxPoints.size() / 3);

        // Ticks
        glColor3fv(input.isSelected() ? g_selectedColor : g_ticksColor);
        glVertexPointer(3, GL_DOUBLE, 0, m_ticksPoints.data());
        glDrawArrays(GL_LINES, 0, m_ticksPoints.size() / 3);

        glDisableClientState(GL_VERTEX_ARRAY);

        // Sphere
        if (m_hasRadius)
        {
            glColor3fv(input.isSelected() ? g_selectedColor : g_boundColor);
            glPushMatrix();
            glRotatef(90, 1, 0, 0);
            GLUquadricObj*  quad;
            quad = gluNewQuadric();
            gluQuadricDrawStyle(quad,GLU_LINE);
            gluQuadricNormals(quad, GLU_NONE);
            gluSphere(quad, m_radius, 16, 16);
            gluDeleteQuadric(quad);
            glPopMatrix();
        }

        glPopAttrib();
    }

    virtual void cleanup(ViewerModifierInput& input)
    {
    }

    virtual void deepCleanup(ViewerModifierInput& input)
    {
    }

    virtual DoubleAttribute getLocalSpaceBoundingBox(ViewerModifierInput& input)
    {
        return m_boundAttr;
    }

    static void flush() {}

    static void onFrameBegin() {}

    static void onFrameEnd() {}

protected:

    // Utility method to add points to a vector
    void addPoint(std::vector<double>& vec, double x, double y, double z)
    {
        vec.push_back(x);
        vec.push_back(y);
        vec.push_back(z);
    }

    // Utility method to add points to a vector
    void addPoint(std::vector<double>& vec, const Imath::V3d& p)
    {
        vec.push_back(p.x);
        vec.push_back(p.y);
        vec.push_back(p.z);
    }

    static bool isValidVoxelResolutionAttr(const IntAttribute attr)
    {
        if (!attr.isValid()) return false;
        if (attr.getNumberOfValues() != 3) return false;

        IntAttribute::array_type values = attr.getNearestSample(0.0f);
        if (values[0] < 1 || values[1] < 1 || values[2] < 1) return false;

        return true;
    }

    void buildPointData()
    {
        m_boxPoints.clear();
        m_ticksPoints.clear();

        if (m_boundAttr.isValid() && m_boundAttr.getNumberOfValues() == 6)
        {
            DoubleConstVector bound = m_boundAttr.getNearestSample(0.0f);

            const double & xMin = bound[0];
            const double & xMax = bound[1];
            const double & yMin = bound[2];
            const double & yMax = bound[3];
            const double & zMin = bound[4];
            const double & zMax = bound[5];

            // 2 * 12 points x 3 comps
            m_boxPoints.reserve(24 * 3);

            addPoint(m_boxPoints, xMin, yMin, zMax);
            addPoint(m_boxPoints, xMax, yMin, zMax);
            addPoint(m_boxPoints, xMax, yMin, zMax);
            addPoint(m_boxPoints, xMax, yMax, zMax);
            addPoint(m_boxPoints, xMax, yMax, zMax);
            addPoint(m_boxPoints, xMin, yMax, zMax);
            addPoint(m_boxPoints, xMin, yMax, zMax);
            addPoint(m_boxPoints, xMin, yMin, zMax);

            addPoint(m_boxPoints, xMin, yMin, zMin);
            addPoint(m_boxPoints, xMax, yMin, zMin);
            addPoint(m_boxPoints, xMax, yMin, zMin);
            addPoint(m_boxPoints, xMax, yMax, zMin);
            addPoint(m_boxPoints, xMax, yMax, zMin);
            addPoint(m_boxPoints, xMin, yMax, zMin);
            addPoint(m_boxPoints, xMin, yMax, zMin);
            addPoint(m_boxPoints, xMin, yMin, zMin);

            addPoint(m_boxPoints, xMax, yMax, zMax);
            addPoint(m_boxPoints, xMax, yMax, zMin);
            addPoint(m_boxPoints, xMax, yMin, zMax);
            addPoint(m_boxPoints, xMax, yMin, zMin);
            addPoint(m_boxPoints, xMin, yMax, zMax);
            addPoint(m_boxPoints, xMin, yMax, zMin);
            addPoint(m_boxPoints, xMin, yMin, zMax);
            addPoint(m_boxPoints, xMin, yMin, zMin);

            if (isValidVoxelResolutionAttr(m_voxelResAttr))
            {
                IntConstVector voxRes = m_voxelResAttr.getNearestSample(0.0f);

                const Imath::V3d dim(xMax - xMin, yMax - yMin, zMax - zMin);
                const double tickLen = 0.15 * std::min(std::min(dim.x, dim.y), dim.z);

                const Imath::V3d axes[3] = {
                    Imath::V3d(tickLen, 0.0, 0.0),
                    Imath::V3d(0.0, tickLen, 0.0),
                    Imath::V3d(0.0, 0.0, tickLen )
                };

                // (res - 1) ticks x 4 points x 3 comp per axis
                m_ticksPoints.reserve((voxRes[0] + voxRes[1] + voxRes[2] - 3) * 12);

                // For each axis
                for (int i = 0; i < 3; ++i)
                {
                    const int & res = voxRes[i];
                    if (res > 1)
                    {
                        const double delta = dim[i] / res;

                        Imath::V3d pt0(xMin, yMin, zMin);
                        const Imath::V3d v1 = axes[(i + 1) % 3];
                        const Imath::V3d v2 = axes[(i + 2) % 3];

                        // For each tick
                        for (int j = 0; j < res - 1; ++j)
                        {
                            // 10*n-th ticks are drawn full scale, others are half-sized
                            const double scale = ((j + 1) % 10 == 0) ? 1.0 : 0.5;

                            pt0[i] += delta;

                            Imath::V3d pt1 = pt0 + v1 * scale;
                            Imath::V3d pt2 = pt0 + v2 * scale;

                            addPoint(m_ticksPoints, pt0);
                            addPoint(m_ticksPoints, pt1);

                            addPoint(m_ticksPoints, pt0);
                            addPoint(m_ticksPoints, pt2);
                        }
                    }
                }
            }
        }
    }

    static const float g_selectedColor[3];
    static const float g_boundColor[3];
    static const float g_ticksColor[3];

    DoubleAttribute m_boundAttr;
    IntAttribute m_voxelResAttr;
    std::vector<double> m_boxPoints;
    std::vector<double> m_ticksPoints;
    bool m_hasRadius;
    double m_radius;

};

// Draw color
const float VolumeViewerModifier::g_selectedColor[3] = { 1.0f, 1.0f, 1.0f };
const float VolumeViewerModifier::g_boundColor[3] = { 0.3f, 0.3f, 0.3f };
const float VolumeViewerModifier::g_ticksColor[3] = { 0.35f, 0.35f, 0.35f };


// Plugin definition and registration
DEFINE_VMP_PLUGIN(VolumeViewerModifier)

void registerPlugins()
{
    std::cout << "[LCA PLUGIN]: Register VolumeViewerModifier v0.2" << std::endl;
    REGISTER_PLUGIN(VolumeViewerModifier, "VolumeViewerModifier", 0, 2);
}
