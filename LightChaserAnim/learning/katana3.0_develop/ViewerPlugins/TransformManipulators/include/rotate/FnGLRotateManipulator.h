// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef GLROTATEMANIPULATOR_H_
#define GLROTATEMANIPULATOR_H_

#include "FnGLTransformManipulator.h"

#include <FnAttribute/FnGroupBuilder.h>
#include <FnViewer/plugin/FnMathTypes.h>

using Foundry::Katana::ViewerUtils::GLTransformManipulator;

/**
 * This class contains the set of manipulator handles required to rotate an
 * object in all three axes. The actual work of rotating objects and setting
 * values is done by the handles.
 *
 * This manipulator is composed of 3 axis handles (GLRotateAxisHandle) and a
 * central "ball" handle (GLRotateBallHandle). The GLRotateAxisHandles
 * are oriented according to the rules set by GlTransformManipulatorHandle,
 * which takes the orientation (world, view, object) into account.
 * GLRotateBallHandle allows two degrees of freedom using a camera facing
 * orientation.
 */
class GLRotateManipulator : public GLTransformManipulator
{

public:

    /// @brief Constructor.
    GLRotateManipulator();

    /// @brief Destructor.
    virtual ~GLRotateManipulator();

    /// Creates a new instance of the manipulator
    static Manipulator* create()
    {
        return new GLRotateManipulator();
    }

    /// Flushes any cached data.
    static void flush(){}

    /**
     * Returns whether the manipulator is valid for the passed location
     * attributes. See the Manipulator class.
     */
    static bool matches(FnAttribute::GroupAttribute locationAttrs)
    {
        return locationAttrs.getChildByName("xform.interactive.rotateX").isValid()
            || locationAttrs.getChildByName("xform.interactive.rotateY").isValid()
            || locationAttrs.getChildByName("xform.interactive.rotateZ").isValid();
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
        gb.set(kTagName, FnAttribute::StringAttribute("Rotate"));
        gb.set(kTagShortcut, FnAttribute::StringAttribute("E"));
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


#endif  // GLROTATEMANIPULATOR_H_
