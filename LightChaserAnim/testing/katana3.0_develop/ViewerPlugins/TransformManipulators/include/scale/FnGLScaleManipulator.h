// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef GLSCALEMANIPULATOR_H_
#define GLSCALEMANIPULATOR_H_

#include "FnGLTransformManipulator.h"

#include <FnAttribute/FnGroupBuilder.h>

using Foundry::Katana::ViewerUtils::GLTransformManipulator;

/**
 * This class contains the set of manipulator handles required to scale an
 * object along all three axis. The actual work of scaling objects and
 * setting values is done by the handles.
 *
 * This manipulator is composed of 3 axis handles (GLScaleAxisHandle) and a
 * central "cube" handle (GLScaleUniformHandle). The GLScaleAxisHandle
 * are oriented according to the rules set by GlTransformManipulatorHandle,
 * which takes the orientation (world, view, object) into account.
 * GLScaleUniformHandle allows to uniformly scale all 3 dimensions at once.
 */
class GLScaleManipulator : public GLTransformManipulator
{

public:

    /// @brief Constructor.
    GLScaleManipulator();

    /// @brief Destructor.
    virtual ~GLScaleManipulator();

    /// Creates a new instance of the manipulator.
    static Manipulator* create()
    {
        return new GLScaleManipulator();
    }

    /// Flushes any cached data.
    static void flush(){}

    /**
     * Returns whether the manipulator is valid for the passed location
     * attributes. See the Manipulator class.
     */
    static bool matches(FnAttribute::GroupAttribute locationAttrs)
    {
        return locationAttrs.getChildByName("xform.interactive.scale").isValid();
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
        gb.set(kTagName, FnAttribute::StringAttribute("Scale"));
        gb.set(kTagShortcut, FnAttribute::StringAttribute("R"));
        gb.set(kTagGroup, FnAttribute::StringAttribute(kTagGroup_TRANSFORM));
        gb.set(kTagTechnology, FnAttribute::StringAttribute(
            GLTransformManipulator::kTechnology));
        gb.set(kTagAlwaysAvailable, FnAttribute::IntAttribute(1));
        gb.set(kTagExclusiveInGroup, FnAttribute::IntAttribute(1));

        return gb.build();
    }

    /// Adds the child handles. Called when the manipulator is created.
    void setup();

};

#endif  // GLSCALEMANIPULATOR_H_
