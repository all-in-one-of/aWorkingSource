#ifndef _BALL_LOCATION_TYPE_H_
#define _BALL_LOCATION_TYPE_H_

#include <FnViewer/plugin/FnViewerDelegate.h>
#include <FnViewer/plugin/FnViewport.h>
#include <FnViewer/plugin/FnViewerDelegateComponent.h>
#include <FnViewer/plugin/FnViewportLayer.h>
#include <FnViewer/utils/FnGLShaderProgram.h>

#include <string>
#include <set>


using namespace Foundry::Katana::ViewerAPI;
using namespace Foundry::Katana::ViewerUtils;

// Represents a Ball location
struct Ball
{
    Ball() : radius(1), visible(true) {}
    Ball(double _radius, const Matrix44d& _worldXform)
        : radius(_radius),
          worldXform(_worldXform)
    {}

    double radius;
    bool visible;
    Matrix44d worldXform;
};

// Data structure shared between the ViewerDelegatComponent and the ViewportLayer
typedef std::map<std::string, Ball> BallsMap;


/***** Viewer Delegate Component *****/

class BallComponent : public ViewerDelegateComponent
{
public:

    BallComponent();

    ~BallComponent();

    void setup();

    void cleanup();

    static ViewerDelegateComponent* create();

    static void flush();

    bool locationEvent(
        const Foundry::Katana::ViewerAPI::ViewerLocationEvent& event,
        bool locationHandled);

    void locationsSelected(const std::vector<std::string>& locations);

    void* getPrivateData(void* inputData);

private:
    BallsMap m_balls;
};


/***** Viewport Layer *****/

class BallLayer : public ViewportLayer
{
public:
    BallLayer();
    virtual ~BallLayer();

    /// Returns a new instance of BallLayer.
    static ViewportLayer* create()
    {
        return new BallLayer();
    }

    void setup();

    void cleanup() { }

    void draw();

    void pickerDraw(unsigned int x, unsigned int y,
        unsigned int w, unsigned int h,
        const PickedAttrsMap& ignoreAttrs);

    bool usesPickingOnHover() { return true; }

    void resize(unsigned int width, unsigned int height);

    void freeze() {}

    void thaw() {}

private:

    void drawBalls(bool isPicking,
        const std::set<std::string>& ignoreLocations);

    BallsMap* m_balls;

    ViewerDelegateWrapperPtr m_delegate;
    ViewportWrapperPtr m_viewport;

    GLShaderProgram m_shader;
};

#endif
