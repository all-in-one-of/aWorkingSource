// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifdef _WIN32
    #include <FnPlatform/Windows.h>
#endif
#include "ExampleViewport.h"
#include <iostream>
#include <sstream>
#include <string>

#include <FnViewer/utils/FnImathHelpers.h>
#include <FnViewer/utils/FnDrawingHelpers.h>


namespace  // anonymous
{
    int _getIntChildAttr(const FnAttribute::GroupAttribute& groupAttr,
                         const std::string& childName)
    {
        FnAttribute::IntAttribute childAttr(
            groupAttr.getChildByName(childName));
        return (childAttr.getValue(0, false));
    }

    bool _getBoolChildAttr(const FnAttribute::GroupAttribute& groupAttr,
                           const std::string& childName)
    {
        FnAttribute::IntAttribute childAttr(
            groupAttr.getChildByName(childName));
        return (childAttr.getValue(0, false) == 1);
    }

    std::string _getStringChildAttr(
        const FnAttribute::GroupAttribute& groupAttr,
        const std::string& childName)
    {
        FnAttribute::StringAttribute childAttr(
            groupAttr.getChildByName(childName));
        return (childAttr.getValue("", false));
    }
}  // anonymous namespace


ExampleViewport::ExampleViewport()
{
    _backgroundColor[0] = 0.3f;
    _backgroundColor[1] = 0.3f;
    _backgroundColor[2] = 0.3f;
}

ExampleViewport::~ExampleViewport()
{
}

Foundry::Katana::ViewerAPI::Viewport* ExampleViewport::create()
{
    return new ExampleViewport();
}

void ExampleViewport::setup()
{
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        // glewInit failed, something is wrong.
        std::cerr << "glewInit() failed, aborting.\n";
        return;
    }

    createDefaultCameras();

    glShadeModel(GL_SMOOTH);
    enableCap(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    enableCap(GL_CULL_FACE);
}

void ExampleViewport::cleanup()
{
}

bool ExampleViewport::event(const FnEventWrapper& event)
{
    // Get a status string for this event
    const std::string status = _getEventDescription(event);
    FnAttribute::StringAttribute statusAttr(status);

    // Set the status option on the delegate
    static const auto kExampleViewerStatus =
        OptionIdGenerator::GenerateId("ExampleViewerStatus");
    ViewerDelegate* viewerDelegate = _getViewerDelegate();
    viewerDelegate->setOption(kExampleViewerStatus, statusAttr);

    // Get the base class to actually process the event
    return Foundry::Katana::ViewerAPI::Viewport::event(event);
}

void ExampleViewport::createDefaultCameras()
{
    Foundry::Katana::ViewerAPI::ViewportCameraWrapperPtr camera =
        addCamera("PerspectiveCamera", "persp");
    if (camera)
    {
        camera->setup(FnAttribute::GroupAttribute());
        setActiveCamera(camera);
    }

    const double kOrthographicCenterOfInterest = 100000.0;
    Foundry::Katana::ViewerAPI::ViewportCameraWrapperPtr sideCamera =
        addCamera("OrthographicCamera", "side");
    if (sideCamera)
    {
        sideCamera->setCenterOfInterest(kOrthographicCenterOfInterest);
        Imath::Matrix44<double> transform;
        transform.rotate(Imath::V3d(0.0f, FnKat::ViewerUtils::toRadians(90.0f), 0.0f));
        transform.translate(
            Imath::V3d(0.0, 0.0, sideCamera->getCenterOfInterest()));

        sideCamera->setup(FnAttribute::GroupAttribute());
        sideCamera->setViewMatrix(&transform.inverse()[0][0]);
    }

    Foundry::Katana::ViewerAPI::ViewportCameraWrapperPtr frontCamera =
        addCamera("OrthographicCamera", "front");
    if (frontCamera)
    {
        frontCamera->setCenterOfInterest(kOrthographicCenterOfInterest);
        Imath::Matrix44<double> transform;
        transform.translate(
            Imath::V3d(0.0, 0.0, frontCamera->getCenterOfInterest()));

        frontCamera->setup(FnAttribute::GroupAttribute());
        frontCamera->setViewMatrix(&transform.inverse()[0][0]);
    }

    Foundry::Katana::ViewerAPI::ViewportCameraWrapperPtr topCamera =
        addCamera("OrthographicCamera", "top");
    if (topCamera)
    {
        topCamera->setCenterOfInterest(kOrthographicCenterOfInterest);
        Imath::Matrix44<double> transform;
        transform.rotate(Imath::V3d(FnKat::ViewerUtils::toRadians(-90.0f),
            0.0f, 0.0f));
        transform.translate(
            Imath::V3d(0.0, 0.0, topCamera->getCenterOfInterest()));

        topCamera->setup(FnAttribute::GroupAttribute());
        topCamera->setViewMatrix(&transform.inverse()[0][0]);
    }
}

void ExampleViewport::draw()
{
    glClearColor(_backgroundColor[0], _backgroundColor[1], _backgroundColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // We mark it as not dirty here because the layers can mark it again as
    // dirty in their draw() calls.
    setDirty(false);

    // Use base class implementation for drawing layers
    Viewport::draw();
}

void ExampleViewport::resize(unsigned int width, unsigned int height)
{
    Viewport::resize(width, height);

    setOption(OptionIdGenerator::GenerateId("ViewportWidth"),
        FnAttribute::IntAttribute(width));
    setOption(OptionIdGenerator::GenerateId("ViewportHeight"),
        FnAttribute::IntAttribute(height));

    Foundry::Katana::ViewerAPI::ViewportCameraWrapperPtr camera =
        getActiveCamera();
    if (camera)
    {
        camera->setViewportDimensions(getWidth(), getHeight());
    }

    glViewport(0, 0, width, height);
}

void ExampleViewport::setOption(OptionIdGenerator::value_type optionId,
    FnAttribute::Attribute attr)
{
    static const OptionIdGenerator::value_type kViewMatrixHash =
        OptionIdGenerator::GenerateId("ViewMatrix");
    static const OptionIdGenerator::value_type kProjectionMatrixHash =
        OptionIdGenerator::GenerateId("ProjectionMatrix");
    static const OptionIdGenerator::value_type kBackgroundColorHash =
        OptionIdGenerator::GenerateId("Viewport.BackgroundColor");
    if (optionId == kViewMatrixHash)
    {
        FnAttribute::DoubleAttribute data = attr;
        if (!data.isValid())
            return;

        if (data.getNumberOfValues() != 16)
            return;

        const double* mtx = getViewMatrix();
        if (!mtx)
            return;

        FnAttribute::DoubleConstVector newMatrix = data.getNearestSample(0);
        getActiveCamera()->setViewMatrix(newMatrix.data());
    }
    else if (optionId == kProjectionMatrixHash)
    {
        FnAttribute::DoubleAttribute data = attr;
        if (!data.isValid())
            return;

        if (data.getNumberOfValues() != 16)
            return;

        const double* mtx = getProjectionMatrix();
        if (!mtx)
            return;

        FnAttribute::DoubleConstVector newMatrix = data.getNearestSample(0);
        getActiveCamera()->setProjectionMatrix(newMatrix.data());
    }
    else if (kBackgroundColorHash == optionId)
    {
        FnAttribute::FloatAttribute attribute(attr);
        if (attribute.isValid())
        {
            FnAttribute::FloatConstVector values = attribute.getNearestSample(0.0f);
            size_t numComponents = values.size();
            if (4 == numComponents)
            {
                _backgroundColor[0] = values[0];
                _backgroundColor[1] = values[1];
                _backgroundColor[2] = values[2];
            }
        }
    }

    m_options[optionId] = attr;

    setDirty(true);
}

FnAttribute::Attribute ExampleViewport::getOption(OptionIdGenerator::value_type optionId)
{
    static const OptionIdGenerator::value_type kViewMatrixHash =
        OptionIdGenerator::GenerateId("ViewMatrix");
    static const OptionIdGenerator::value_type kProjectionMatrixHash =
        OptionIdGenerator::GenerateId("ProjectionMatrix");

    if (optionId == kViewMatrixHash)
    {
        return FnAttribute::DoubleAttribute(getViewMatrix(), 16, 4);
    }
    else if (optionId == kProjectionMatrixHash)
    {
        return FnAttribute::DoubleAttribute(getProjectionMatrix(), 16, 4);
    }

    AttrMap::iterator it = m_options.find(optionId);

    if (it != m_options.end())
    {
        return it->second;
    }

    return FnAttribute::Attribute();
}

void ExampleViewport::flush()
{
}

void ExampleViewport::freeze()
{
}

void ExampleViewport::thaw()
{

    /*
    TODO
    Foundry::Katana::ViewerAPI::ViewerDelegate* viewerDelegate = getViewerDelegate();
    viewerDelegate->thaw();
    */

    setDirty(true);
}

ExampleViewerDelegate* ExampleViewport::_getViewerDelegate()
{
    Foundry::Katana::ViewerAPI::ViewerDelegateWrapperPtr delegatePtr =
        getViewerDelegate();
    ExampleViewerDelegate* viewerDelegate =
        delegatePtr->getPluginInstance<ExampleViewerDelegate>();

    return viewerDelegate;
}

std::string
ExampleViewport::_getEventDescription(const FnEventWrapper& event) const
{
    // Generate a status string to report back to the tab via the delegate.
    FnAttribute::GroupAttribute eventData = event.getData();
    const std::string eventType = event.getType();

    std::stringstream str;
    str << "[" << eventType << "] ";

    if (eventType.substr(0, 3) == "Key")
    {
        // Add the key sequence string + auto-repeat
        str << _getStringChildAttr(eventData, "keySequence");

        if (_getBoolChildAttr(eventData, "isAutoRepeat"))
        {
            str << " (Auto-Repeat)";
        }
    }
    else
    {
        // Add modifiers
        if (_getBoolChildAttr(eventData, "ShiftModifier"))
        {
            str << "[Shift]";
        }
        if (_getBoolChildAttr(eventData, "CtrlModifier"))
        {
            str << "[Ctrl]";
        }
        if (_getBoolChildAttr(eventData, "AltModifier"))
        {
            str << "[Alt]";
        }
        if (_getBoolChildAttr(eventData, "MetaModifier"))
        {
            str << "[Meta]";
        }

        // Add mouse buttons
        if (_getBoolChildAttr(eventData, "LeftButton"))
        {
            str << "[LeftButton]";
        }
        if (_getBoolChildAttr(eventData, "MidButton"))
        {
            str << "[MidButton]";
        }
        if (_getBoolChildAttr(eventData, "RightButton"))
        {
            str << "[RightButton]";
        }

        if (eventType.substr(0, 5) == "Mouse")
        {
            // Add position
            str << " : (" << _getIntChildAttr(eventData, "x") << ", "
                          << _getIntChildAttr(eventData, "y") << ")";
        }
        else if (eventType.substr(0, 5) == "Wheel")
        {
            // Add delta
            str << " : delta = " << _getIntChildAttr(eventData, "delta");
        }
    }
    return str.str();
}
