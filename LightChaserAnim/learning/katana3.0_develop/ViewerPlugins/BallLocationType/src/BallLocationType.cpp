#include "BallLocationType.h"

#include <ImathMatrix.h>

#include <FnViewer/utils/FnDrawingHelpers.h>

#include "GL/glew.h"

#include <algorithm>
#include <iostream>
#include <vector>

FnLogSetup("Viewer.Example.BallLocationType");

/***** Viewer Delegate Component *****/

BallComponent::BallComponent() {}

BallComponent::~BallComponent() {}

void BallComponent::setup() {}

void BallComponent::cleanup() {}

ViewerDelegateComponent* BallComponent::create()
{
    return new BallComponent();
}

void BallComponent::flush() {}

bool BallComponent::locationEvent(const ViewerLocationEvent& event,
                                   bool locationHandled)
{
    if (event.stateChanges.attributesUpdated)
    {
        FnAttribute::GroupAttribute attrs = event.attributes;

        // If the type is "ball", then we store some information about it in the
        // local data structure.
        FnAttribute::StringAttribute typeAttr = attrs.getChildByName("type");
        if (typeAttr.isValid() && typeAttr.getValue() == "ball")
        {
            FnAttribute::DoubleAttribute radiusAttr =
                attrs.getChildByName("geometry.radius");
            if (radiusAttr.isValid())
            {
                m_balls[event.locationPath] = Ball(radiusAttr.getValue(),
                    getViewerDelegate()->getWorldXform(event.locationPath));
            }
            else
            {
                m_balls.erase(event.locationPath);
            }
        }
    }
    else if (event.stateChanges.locationRemoved)
    {
        m_balls.erase(event.locationPath);
    }
    return false;
}

void* BallComponent::getPrivateData(void* /*inputData*/)
{
    // Return a reference to the local data structure to be accessed by other
    // plugins built into the same binary as this.
    return static_cast<void*>(&m_balls);
}

void BallComponent::locationsSelected(const std::vector<std::string>& locations)
{}


/***** Viewport Layer *****/

BallLayer::BallLayer()
    : ViewportLayer(false /* usePickingOnHover */)
{}

BallLayer::~BallLayer() {}

void BallLayer::setup()
{
    if (!GLEW_INIT())
        return;

    // Cache the Viewport and ViewerDelegate, since they will not change during
    // the lifetime of this object.
    m_viewport = getViewport();
    m_delegate = m_viewport->getViewerDelegate();

    // Get the reference to the ViewerDelegateComponent plugin
    ViewerDelegateComponentWrapperPtr component = m_delegate->getComponent(
        "BallComponent");
    if (!component)
    {
        std::cerr << "Error: BallLayer could not find BallComponent!" << std::endl;
    }

    // Get the ViewerDelegateComponent plugin instance - this only works if the
    // other plugin is built into the same binary as this.
    BallComponent* testComponent = component->getPluginInstance<BallComponent>();

    // Get the ViewerDelegateComponent's private data - this only works if the
    // other plugin is built into the same binary as this.
    // Because the instance of BallComponent::m_balls (the private data) does
    // not change, we can get it here and cache it. Otherwise we would need to
    // call this on every draw() and pickerDraw() call.
    m_balls = static_cast<BallsMap*>(testComponent->getPrivateData(NULL));

    // Set up the shader program
    std::string fullPath = getenv("LCA_DEVKTN_PATH");
    fullPath.append("/resource/3.0/Shaders/GenericViewerPlugins/");
    fullPath.append("n_dot_eyes");
    m_shader.compileShader(fullPath + ".vert", VERTEX);
    m_shader.compileShader(fullPath + ".frag", FRAGMENT);
    m_shader.link();
}

void BallLayer::resize(unsigned int width, unsigned int height) { }

void BallLayer::draw()
{
    std::set<std::string> ignoreLocations; // ignore nothing
    drawBalls(false, ignoreLocations); // draw the balls
}

void BallLayer::pickerDraw(
        unsigned int x, unsigned int y,
        unsigned int w, unsigned int h,
        const PickedAttrsMap& ignoreAttrs)
{
    // Get the locations to ignore
    std::set<std::string> ignoreLocations;
    PickedAttrsMap::const_iterator it;
    for (it = ignoreAttrs.begin(); it != ignoreAttrs.end(); ++it)
    {
        FnAttribute::GroupAttribute child = it->second;
        if (child.isValid())
        {
            // The convention for a location will be a GroupAttribute with
            // a child called "location" and the value must be a
            // StringAttribtue containing the location path in its value
            FnAttribute::StringAttribute locationAttr =
                child.getChildByName("location");
            if (locationAttr.isValid())
            {
                ignoreLocations.insert(locationAttr.getValue());
            }
        }
    }

    // Y is flipped in relation to the scissor
    y = getViewport()->getHeight() - y - h;
    glScissor(x, y, w, h); // Render only the necessary part
    glEnable(GL_SCISSOR_TEST);

    drawBalls(true, ignoreLocations); // Draw the balls

    glDisable(GL_SCISSOR_TEST);
}



void BallLayer::drawBalls(bool isPicking,
    const std::set<std::string>& ignoreLocations)
{

    // Get the list of selected locations, so that we can highlight them
    std::vector<std::string> selectedLocations;
    m_delegate->getSelectedLocations(selectedLocations);

    // Get the camera and prepare the shader
    ViewportCameraWrapperPtr camera = m_viewport->getActiveCamera();
    m_shader.use();
    m_shader.setUniform("ProjectionMatrix", camera->getProjectionMatrix44d());
    m_shader.setUniform("ViewMatrix", camera->getViewMatrix44d());

    // Iterate through the balls
    BallsMap::const_iterator it;
    for ( it = m_balls->begin(); it != m_balls->end(); it++ )
    {
        std::string locationPath = it->first;
        const Ball& ball = it->second;

        // Ignore location if in ignore set
        if (ignoreLocations.find(locationPath) != ignoreLocations.end())
        {
            continue;
        }

        Vec4f color; // the color to be used in the render

        // If this is a pickerDraw() call
        if (isPicking)
        {
            // Build a location entry attribute:
            // The convention for a location will be a GroupAttribute with
            // a child called "location" and the value must be a
            // StringAttribtue containing the location path in its value
            FnAttribute::GroupAttribute locationAttr =
                FnAttribute::GroupBuilder()
                .set("location", FnAttribute::StringAttribute(locationPath))
                .build();

            // Add the pickable object
            FnPickId pickId = addPickableObject(locationAttr);

            // Get the color for this ID (see FnPickingTypes.h)
            pickIdToColor(pickId, color);
        }
        else
        {
            // Check if this location is selected - in reality we could have
            // tracked the selection in the locationSelected() callback and
            // update the data balls structure with that information.
            if (std::find(selectedLocations.begin(), selectedLocations.end(),
                    locationPath) == selectedLocations.end())
            {
                color = Vec4f(0.25f, 0.5f, 0.75f, 1.0f);
            }
            else
            {
                color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
            }
        }

        // Set the rest of the shader args
        m_shader.setUniform("WorldMatrix", ball.worldXform);
        m_shader.setUniform("Color", color.x, color.y, color.z, color.w);

        // Draw a sphere with the correct radius
        GLUquadricObj* quad;
        quad = gluNewQuadric();
        gluQuadricNormals(quad, GLU_SMOOTH);
        gluSphere(quad, ball.radius, 32, 32);
        gluDeleteQuadric(quad);
    }
}

/******  Plugin Registration ********/

namespace // anonymous
{

    DEFINE_VIEWER_DELEGATE_COMPONENT_PLUGIN(BallComponent);
    DEFINE_VIEWPORT_LAYER_PLUGIN(BallLayer);

} // namespace anonymous

void registerPlugins()
{
    REGISTER_PLUGIN(BallComponent, "BallComponent", 0, 1);
    REGISTER_PLUGIN(BallLayer, "BallLayer", 0, 1);
}
