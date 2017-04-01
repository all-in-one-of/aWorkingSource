// Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.
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

#include <string>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <vector>

struct Vector3f
{
    Vector3f() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3f(float x, float y, float z) : x(x), y(y), z(z) {}

    float x, y, z;
};

/**
 * Draws a line between two points. This relies on the calling code to have
 * called glBegin(GL_LINES) and to call glEnd().
 */
void drawLine(const Vector3f& pt1, const Vector3f& pt2)
{
    glVertex3f(pt1.x, pt1.y, pt1.z);
    glVertex3f(pt2.x, pt2.y, pt2.z);
}

/**
 * The LightFilterViewerModifier controls how light filter objects are
 * displayed within the viewer. Renderer vendors and shader writers are
 * expected to define how their light filters are displayed, but this VMP
 * should display some sensible default.
 *
 * It is possible to specify the light filter type manually in the
 * "material.lightFilterParams.Type" or "material.viewerLightFilterParams.Type"
 * attributes to one of the following:
 *
 *      - "blocker"
 *      - "gobo"
 *      - "barndoor"
 *      - "decay"
 */
class LightFilterViewerModifier : public FnKat::ViewerModifier
{
public:

    enum LightFilterType
    {
        LIGHTFILTERTYPE_BLOCKER = 0,
        LIGHTFILTERTYPE_GOBO,
        LIGHTFILTERTYPE_BARNDOOR,
        LIGHTFILTERTYPE_DECAY,
    };

    enum FillType
    {
        FILLTYPE_POINTS = 0,
        FILLTYPE_WIREFRAME,
        FILLTYPE_SOLID,
    };

    LightFilterType         m_lightFilterType;
    FillType                m_fillType;
    float                   m_uSize, m_vSize, m_wSize;
    GLUquadric*             m_quadric;

    // The light filter type - stored to avoid computing type for every draw.
    mutable FnKat::StringAttribute  m_typeAttr;
    mutable bool                    m_testedTypeAttr;

    // The light filter type obtained from from the "meta.lightFilterType"
    // attribute.
    std::string m_metaLightFilterType;

    // Used to store the contents of the "meta.defaultKey" attribute, which can
    // be used to specify a child attribute of "meta.lightFilterType" to use as
    // the light filter type.
    std::string m_metaDefaultKey;

private:

    static const char* const s_typeAttrNames[];
    static const int s_numTypeAttrNames = 2;

public:

    LightFilterViewerModifier(FnKat::GroupAttribute args)
        : FnKat::ViewerModifier(args),
          m_lightFilterType(LIGHTFILTERTYPE_BLOCKER),
          m_fillType(FILLTYPE_WIREFRAME),
          m_uSize(1.0f),
          m_vSize(1.0f),
          m_wSize(1.0f),
          m_quadric(0),
          m_testedTypeAttr(false)
    {
        // Empty
    }

    static FnKat::ViewerModifier* create(FnKat::GroupAttribute args)
    {
        return (FnKat::ViewerModifier*) new LightFilterViewerModifier(args);
    }

    static FnKat::GroupAttribute getArgumentTemplate()
    {
        FnKat::GroupBuilder gb;
        return gb.build();
    }

    /**
     * Returns the type of scene graph location that this viewer modifier runs on
     */
    static const char* getLocationType()
    {
        return "light filter";
    }

    /**
     * Called per instance before each draw
     */
    void deepSetup(FnKat::ViewerModifierInput& input)
    {
        // Draw only the VMP representation in the viewer
        input.overrideHostGeometry();
    }

    /**
     * Called once per VMP instance when constructed.
     */
    void setup(FnKat::ViewerModifierInput& input)
    {
        // Used for Spot, sphere and Dome lights
        m_quadric = gluNewQuadric();

        // Ensure that we pick up any changed light filter type attributes
        m_testedTypeAttr = false;

        // Try to get light filter type from the material.meta attribute
        lookupMetaLightFilterType(input);

        // Default to blocker
        m_lightFilterType = LIGHTFILTERTYPE_BLOCKER;

        // Check attributes to try and determine the light filter type
        if(isBlocker(input))
        {
            m_lightFilterType = LIGHTFILTERTYPE_BLOCKER;
        }
        else if(isGobo(input))
        {
            m_lightFilterType = LIGHTFILTERTYPE_GOBO;
        }
        else if(isBarnDoor(input))
        {
            m_lightFilterType = LIGHTFILTERTYPE_BARNDOOR;
        }
        else if(isDecay(input))
        {
            m_lightFilterType = LIGHTFILTERTYPE_DECAY;
        }
    }

    /**
     * Draws the light filter representation.
     */
    void draw(FnKat::ViewerModifierInput& input)
    {
        glPushAttrib(GL_POLYGON_BIT | GL_LIGHTING_BIT | GL_LINE_BIT);
        glDisable(GL_LIGHTING);

        // To simplify selecting the light filter, when we are "picking",
        // increase the line width.
        const bool isPicking = input.getDrawOption("isPicking");
        if (isPicking)
        {
            glLineWidth(10);
        }
        else
        {
            glLineWidth(1);
        }

        // Get the fill type - default is wireframe
        m_fillType = FILLTYPE_WIREFRAME;
        FnAttribute::StringAttribute fillTypeAttr =
            input.getGlobalAttribute("viewer.default.drawOptions.fill");

        const std::string fillType = fillTypeAttr.getValue("", false);

        if (fillType == "points")
        {
            m_fillType = FILLTYPE_POINTS;
        }
        else if (fillType == "solid")
        {
            m_fillType = FILLTYPE_SOLID;
        }

        if (isPicking || m_fillType == FILLTYPE_SOLID)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        // Draw color: white if selected, otherwise yellow
        if (input.isSelected())
        {
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
        else
        {
            glColor4f(1, 1, 0, 1);
        }

        // Draw light filter depending on light filter type
        switch (m_lightFilterType)
        {
        case LIGHTFILTERTYPE_BLOCKER:
            drawBlocker(input);
            break;
        case LIGHTFILTERTYPE_GOBO:
            drawGobo(input);
            break;
        case LIGHTFILTERTYPE_BARNDOOR:
            drawBarnDoor(input);
            break;
        case LIGHTFILTERTYPE_DECAY:
            drawDecay(input);
            break;
        };

        // Restore the original options
        if(isPicking)
        {
            glLineWidth(1);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        // Restore draw state
        glPopAttrib();
    }

    /**
     * Called when the location is removed/refreshed.
     */
    void cleanup(FnKat::ViewerModifierInput& input)
    {
        // Empty
    }

    /**
     * Called per instance after each draw
     */
    void deepCleanup(FnKat::ViewerModifierInput& input)
    {
        // Empty
    }

    /**
     * Returns a bounding box for the current location for use with the viewer
     * scene graph.
     */
    FnKat::DoubleAttribute getLocalSpaceBoundingBox(FnKat::ViewerModifierInput& input)
    {
        double bounds[6] = {-1, 1, -1, 1, -1, 1};
        return FnKat::DoubleAttribute(bounds, 6, 1);
    }

    static void flush() {}

    static void onFrameBegin() {}

    static void onFrameEnd() {}

private:

    //=======================================================================
    // Light filter type identification
    //=======================================================================

    void lookupMetaLightFilterType(FnKat::ViewerModifierInput& input)
    {
        FnKat::GroupAttribute materialAttr = input.getGlobalAttribute("material");

        FnKat::StringAttribute defaultKeyAttr = materialAttr.getChildByName("meta.defaultKey");
        m_metaDefaultKey = defaultKeyAttr.getValue("", false);

        if (!m_metaDefaultKey.empty())
        {
            FnKat::StringAttribute lightFilterTypeAttr =
                materialAttr.getChildByName("meta.lightFilterType." + m_metaDefaultKey);
            m_metaLightFilterType = lightFilterTypeAttr.getValue("", false);
        }
        else
        {
            FnKat::GroupAttribute lightFilterTypeGroupAttr =
                materialAttr.getChildByName("meta.lightFilterType");

            for (int64_t i = 0, e = lightFilterTypeGroupAttr.getNumberOfChildren();
                 i != e; ++i)
            {
                FnKat::StringAttribute lightFilterTypeAttr =
                    lightFilterTypeGroupAttr.getChildByIndex(i);

                if (lightFilterTypeAttr.isValid())
                {
                    m_metaDefaultKey = lightFilterTypeGroupAttr.getChildName(i);
                    m_metaLightFilterType = lightFilterTypeAttr.getValue("", false);
                    break;
                }
            }
        }
    }

    FnKat::Attribute lookupMetaShaderParam(FnKat::ViewerModifierInput& input,
                                           const std::string & shaderName)
    {
        FnKat::StringAttribute nameAttr = input.getGlobalAttribute(
            "material.meta." + shaderName + "." + m_metaDefaultKey);
        if (nameAttr.isValid())
        {
            FnKat::StringAttribute::array_type values = nameAttr.getNearestSample(0.0);

            for (FnKat::StringAttribute::array_type::const_iterator I =
                    values.begin(), E = values.end(); I != E; ++I)
            {
                FnKat::Attribute shaderAttr =
                    input.getGlobalAttribute(std::string("material.") + (*I));
                if (shaderAttr.isValid())
                {
                    return shaderAttr;
                }
            }
        }

        return FnKat::Attribute();
    }

    /**
     * Determines whether this is a blocker light filter.
     */
    bool isBlocker(FnKat::ViewerModifierInput& input)
    {
        return isLightFilterOfType(input, "blocker");
    }

    /**
     * Determines whether this is a gobo light filter.
     */
    bool isGobo(FnKat::ViewerModifierInput& input) const
    {
        return isLightFilterOfType(input, "gobo");
    }

    /**
     * Determines whether this is a barn door light filter.
     */
    bool isBarnDoor(FnKat::ViewerModifierInput& input) const
    {
        return isLightFilterOfType(input, "barnDoor");
    }

    /**
     * Determines whether this is a decay light filter.
     */
    bool isDecay(FnKat::ViewerModifierInput& input)
    {
        return isLightFilterOfType(input, "decay");
    }

    /**
     * Checks certain attributes to determine whether the type of light filter
     * has been manually specified.
     */
    bool isLightFilterOfType(FnKat::ViewerModifierInput& input, std::string lightFilterType) const
    {
        if (m_metaLightFilterType == lightFilterType)
        {
            return true;
        }

        if (m_testedTypeAttr)
        {
            return m_typeAttr.getValue("", false) == lightFilterType;
        }

        // Optimization to ensure that we only try to get the type once per draw
        m_testedTypeAttr = true;

        // Loop through the list of attributes where the type of light filter
        // can be specified directly
        FnKat::StringAttribute typeAttr;
        for (int i = 0; i < s_numTypeAttrNames; ++i)
        {
            typeAttr = input.getGlobalAttribute(std::string(s_typeAttrNames[i]));
            if(typeAttr.isValid())
            {
                // Store the valid attribute to use in successive queries
                m_typeAttr = typeAttr;
                return typeAttr.getValue("", false) == lightFilterType;
            }
        }

        return false;
    }

    //=======================================================================
    // Drawing helpers
    //=======================================================================

    void drawBlocker(FnKat::ViewerModifierInput& input)
    {
        // TODO - implement drawing for this light filter type
        drawBox(input);
    }

    void drawGobo(FnKat::ViewerModifierInput& input)
    {
        // TODO - implement drawing for this light filter type
        drawCard(input);
    }

    void drawBarnDoor(FnKat::ViewerModifierInput& input)
    {
        // TODO - implement drawing for this light filter type
        drawBox(input);
    }

    void drawDecay(FnKat::ViewerModifierInput& input)
    {
        // TODO - implement drawing for this light filter type
        drawBox(input);
    }

    /**
     * Draws a box
     */
    void drawBox(FnKat::ViewerModifierInput& input)
    {
        glPushMatrix();

        const GLenum drawMode = (m_fillType == FILLTYPE_POINTS) ? GL_POINTS : GL_POLYGON;

        const Vector3f vertex1(-m_uSize, -m_vSize, -m_wSize);
        const Vector3f vertex2(-m_uSize, m_vSize, -m_wSize);
        const Vector3f vertex3(m_uSize, m_vSize, -m_wSize);
        const Vector3f vertex4(m_uSize, -m_vSize, -m_wSize);
        const Vector3f vertex5(-m_uSize, -m_vSize, m_wSize);
        const Vector3f vertex6(-m_uSize, m_vSize, m_wSize);
        const Vector3f vertex7(m_uSize, m_vSize, m_wSize);
        const Vector3f vertex8(m_uSize, -m_vSize, m_wSize);

        // Two polygons for lids
        glBegin(drawMode);
        drawLine(vertex1, vertex2);
        drawLine(vertex3, vertex4);
        glEnd();

        glBegin(drawMode);
        drawLine(vertex5, vertex6);
        drawLine(vertex7, vertex8);
        glEnd();

        // Draw the sides if necessary
        if (m_fillType != FILLTYPE_POINTS)
        {
            glBegin(GL_QUAD_STRIP);
            drawLine(vertex5, vertex1);
            drawLine(vertex8, vertex4);
            drawLine(vertex7, vertex3);
            drawLine(vertex6, vertex2);
            drawLine(vertex5, vertex1);
            glEnd();
        }

        glPopMatrix();
    }

    /*
     * Draws a card.
     */
    void drawCard(FnKat::ViewerModifierInput& input)
    {
        glPushMatrix();

        const GLenum drawMode = (m_fillType == FILLTYPE_POINTS) ? GL_POINTS : GL_POLYGON;

        glBegin(drawMode);
        glVertex3f(-m_uSize, -m_vSize, 0);
        glVertex3f(m_uSize, -m_vSize, 0);
        glVertex3f(m_uSize, m_vSize, 0);
        glVertex3f(-m_uSize, m_vSize, 0);
        glEnd();

        glPopMatrix();
    }

};

const char* const LightFilterViewerModifier::s_typeAttrNames[] = {
    "material.lightFilterParams.Type",
    "material.viewerLightFilterParams.Type"
};

DEFINE_VMP_PLUGIN(LightFilterViewerModifier)

void registerPlugins()
{
    REGISTER_PLUGIN(LightFilterViewerModifier, "LightFilterViewerModifier", 0, 1);
}
