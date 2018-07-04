// Copyright (c) 2018 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef GLCOIMANIPULATOR_H_
#define GLCOIMANIPULATOR_H_

#include "FnGLTransformManipulator.h"

/**
 * This class contains the set of manipulator handles required to translate or
 * rotate an object around its center of interest.
 *
 * This manipulator is composed of several axis for translate and rotation.
 */
class GLCoiManipulator :
    public Foundry::Katana::ViewerUtils::GLTransformManipulator
{
public:
    /// @brief Constructor.
    GLCoiManipulator();

    /// @brief Destructor.
    virtual ~GLCoiManipulator();

    /// Creates a new instance of the manipulator.
    static Manipulator* create()
    {
        return new GLCoiManipulator();
    }

    /// Flushes any cached data.
    static void flush(){}

    /**
     * Returns whether the manipulator is valid for the passed location
     * attributes.
     */
    static bool matches(FnAttribute::GroupAttribute attrs)
    {
        const FnAttribute::GroupAttribute interactiveAttrs =
            attrs.getChildByName("xform.interactive");
        if (!interactiveAttrs.isValid())
        {
            return false;
        }

        return attrs.getChildByName("geometry.centerOfInterest").isValid() &&
            (interactiveAttrs.getChildByName("translate").isValid()
            || interactiveAttrs.getChildByName("rotateX").isValid()
            || interactiveAttrs.getChildByName("rotateY").isValid()
            || interactiveAttrs.getChildByName("rotateZ").isValid());
    }

    /**
     * Returns a number of tags that can be queried by the UI to set up
     * various settings, such as the visible name of the manipulator,
     * what menu group it should be in, or the keyboard shortcut to
     * activate it.
     */
    static FnAttribute::GroupAttribute getTags()
    {
        FnAttribute::GroupBuilder gb;
        gb.set(kTagName, FnAttribute::StringAttribute("Center of Interest"));
        gb.set(kTagShortcut, FnAttribute::StringAttribute("T"));
        gb.set(kTagGroup, FnAttribute::StringAttribute(kTagGroup_TRANSFORM));
        gb.set(kTagTechnology, FnAttribute::StringAttribute(
            GLTransformManipulator::kTechnology));
        gb.set(kTagAlwaysAvailable, FnAttribute::IntAttribute(1));
        gb.set(kTagExclusiveInGroup, FnAttribute::IntAttribute(1));

        return gb.build();
    }

    /// Adds the child handles. Called when the manipulator is created.
    void setup();

protected:
    /**
     * Function template to help add and initialize a manipulator handle.
     */
    template <typename className>
    className* addManipulatorHandle(
        const char* pluginName,
        const char* manipName,
        bool placeOnCenterOfInterest,
        FnKat::ViewerUtils::TransformMode transformMode)
    {
        Foundry::Katana::ViewerAPI::ManipulatorHandleWrapperPtr handleWrapper =
            Manipulator::addManipulatorHandle(pluginName, manipName);
        className* const handle = handleWrapper->getPluginInstance<className>();
        handle->placeOnCenterOfInterest(placeOnCenterOfInterest);
        handle->setTransformMode(transformMode);

        return handle;
    }
};

#endif  // GLCOIMANIPULATOR_H_
