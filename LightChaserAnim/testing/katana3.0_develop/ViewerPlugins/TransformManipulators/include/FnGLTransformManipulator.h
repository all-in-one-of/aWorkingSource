// Copyright (c) 2017 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef GLTRANSFORMMANIPULATOR_H_
#define GLTRANSFORMMANIPULATOR_H_

#include <stdint.h>
#include <string>

#include <FnViewer/utils/FnGLManipulator.h>
#include <FnViewer/plugin/FnMathTypes.h>

#include <ImathVec.h>
#include <ImathMatrix.h>
#include <ImathEuler.h>

namespace Foundry
{
namespace Katana
{
namespace ViewerUtils
{
using namespace Foundry::Katana::ViewerAPI;

/// @brief Manipulator orientation spaces
enum Orientation { OBJECT = 0, WORLD, VIEW };

/// @brief Manipulator transform modes.
enum TransformMode
{
    kObject,  // Transform applies to objects (default mode).
    kCenterOfInterest,  // Transform applies to the center of interest.
    kAroundCenterOfInterest,  // Transform applies to objects, which will remain
                              // oriented towards the center of interest.
};

/**
 * @brief A base class for a GL based Transform Manipulator.
 * 
 * This is a base class implementation of GLManipulator for transforms
 * (example: Rotate, Translate and Scale manipulators).
 *
 * A transform manipulator can be applied to several manipulated locations.
 *
 * It calculates and caches a transformation for the manipulator as a whole
 * (see Manipulator::setXform() and Manipulator::getXform()).
 *
 * The manipulator's origin is placed at the pivot of the last of these
 * locations (see getLastLocationPath()) and it is scaled in order to maintain
 * its relative size to the Viewport, no matter what projection and view are
 * used to render it. The pivot can be the location's origin (if
 * alwaysAtLocationOrigin constructor argument is set to true) or it can be
 * defined by the transform component being manipulated. For example, in
 * a location in which the transform order dictates that the translation is
 * applied before the rotation, the pivot will be off the location's origin, 
 * since a rotation will imply an orbit around this pivot, rather than around
 * the object's origin.
 *
 * The orientation (rotation) of the manipulator is left to each of its
 * Manipulator Handle to decide, as each one of them can specify a different
 * orientation (see GLTransformManipulatorHandle).
 *
 * The calculation and caching of the manipulator's transform happens in draw()
 * and pickerDraw() because the transform of the location where the manipulator
 * is positioned can change when it is cooked, hence the need to poll for it
 * when drawing.
 *
 * See GLTransformManipulatorHandle for information on how a local manipulation
 * can be applied to the different locations.
 *
 * The handles of a GLTransformManipulator must extend the base class
 * GLTransformManipulatorHandle.
 */
class GLTransformManipulator : public GLManipulator
{

public:

    /** @brief Constructor. */
    GLTransformManipulator();

    /** @brief Destructor. */
    virtual ~GLTransformManipulator();

    /**
     * @brief Draws the manipulator.
     *
     * This must be called by the implementation on child classes. This will
     * guarantee that the handles' draw() functions are called and that the
     * manipulator's transform is calculated and set. Since the transform
     * of the location where the manipulator is positioned can change when it
     * is cooked, this has to be recalculated here.
     */
    virtual void draw();

    /**
     * @brief Draws the manipulator for picking using the passed picker ID.
     *
     * This must be called by the implementation on child classes. This will
     * guarantee that the handles' draw() functions are called and that the
     * manipulator's transform is calculated and set. Since the transform
     * of the location where the manipulator is positioned can change when it
     * is cooked, this has to be recalculated here.
     */
    virtual void pickerDraw(int64_t pickerID);

    /**
     * Returns the order of the axis in which the rotation is applied. This is
     * defined by the order of the xform.interactive.rotate? attributes in the
     * passed location.
     */
    Imath::Eulerd::Order getRotationOrder(const std::string& locationPath);

    /**
     *  @brief: Gets the location path where the manipulator is placed.
     *
     *  This is the last location returned by Manipulator::getLocationPaths(),
     *  or empty string if Manipulator::getLocationPaths returns an empty
     *  vector.
     */
    std::string getLastLocationPath();

    /**
     * @brief: Sets the option for the Manipulator.
     *
     * This has been overwritten to capture and store manipulator preferences
     * such as scale.
     */
    virtual void setOption(
        Foundry::Katana::ViewerAPI::OptionIdGenerator::value_type optionId,
        FnAttribute::Attribute attr);

    /**
     * @brief Calculates a scale that will compensate for the perspective.
     *
     * This guarantees that the size of an object at the given point will be
     * constant from any camera position, fov, etc. by compensating the effect
     * of the projection and view matrices.
     */
    double getFixedSizeScale(Imath::V3d point);

    /**
     *  @brief These functions get the specific transform for the given
     *  location.
     *
     *  The initial values are stored on startDrag(), whereas the current
     *  values come directly from the viewer delegate.
     */
    Imath::M44d getInitialTranslateXformFromLocation(
        const std::string& locationPath);
    Imath::M44d getTranslateXformFromLocation(
        const std::string& locationPath);
    Imath::M44d getInitialRotateXformFromLocation(
        const std::string& locationPath);
    Imath::M44d getRotateXformFromLocation(
        const std::string& locationPath);
    Imath::M44d getInitialScaleXformFromLocation(
        const std::string& locationPath);
    Imath::M44d getScaleXformFromLocation(
        const std::string& locationPath);

    /**
     * Gets the initial position in world space of the given location.
     * The value is cached on startDrag().
     */
    Imath::V3d getInitialPositionFromLocation(const std::string& locationPath);

    /**
     * Gets the initial center of interest in world space of the given location.
     * The value is cached on startDrag().
     */
    Imath::V3d getInitialCoiFromLocation(const std::string& locationPath);

    /**
     * Gets the orientation xform in world space of the given location. If the
     * given orientation is WORLD, it uses the initial world xform.
     * The value is cached on startDrag().
     */
    Imath::M44d getInitialOrientationXform(
        const std::string& locationPath,
        Orientation orientation);

    /**
     * Gets the orientation xform in world space of the given location. If the
     * given orientation is WORLD, it uses the identity matrix.
     */
    Imath::M44d getOrientationXform(
        const std::string& locationPath,
        Orientation orientation);

    /**
     * Rotates all the selected locations around their center of interest given
     * the rotation angle and the axis.
     */
    void applyRotationAroundCoiToAllLocations(
        double angle,
        const Imath::V3d& axis,
        Orientation orientation,
        bool isFinal);

    /**
     * Applies a "Translate COI" action to all locations.
     *
     * It calculates the new center of interests' positions give the delta, and
     * adjusts the objects' rotations and center of interest distances, so that
     * the objects are oriented towards their respective center of interest.
     */
    void applyCoiTranslationToAllLocations(
        const Imath::V3d& delta,
        double distance,
        Orientation orientation,
        bool isFinal);

    /**
     * Applies a "Translate around COI" action to all locations.
     *
     * This function assumes that the objects have already been translated to
     * their final position. Then, it calculates the new center of interests'
     * positions, and adjusts the objects' rotations and center of interest
     * distances, so that the objects are oriented towards their respective
     * center of interest.
     */
    void applyTranslationAroundCoiToAllLocations(bool isFinal);

    /**
     * Applies a rotation matrix to the location to change its look-at
     * direction.
     */
    void applyOrientationToLocation(
        const std::string& locationPath,
        const Imath::M44d& rotateMatrix,
        bool isFinal);

    /**
     * Applies a translation matrix to the location.
     */
    void applyTranslationToLocation(
        const std::string& locationPath,
        const Imath::V3d& deltaWorldSpace,
        Orientation orientation,
        bool isFinal);

protected:
    /// @brief Utility function: Get the ViewerDelegate.
    ViewerDelegateWrapperPtr getViewerDelegate();

private:
    /**
     * @brief: The global scale of the manipulator. This value is set via
     * "setOption()".
     *
     * This defines the size of every transform manipulator. The size is always
     * the same, no matter what camera and point of view they are rendered
     * from. This value is a multiplier that defines the size of the
     * manipulator in relation to the Viewport. It defines the ratio between
     * the Viewport vertical size and a distance of 1 in manipulator space.
     */
    double m_globalScale;
};


/**
 * @brief A base class for a GL based Transform ManipulatorHandle.
 * 
 * This is a base class implementation of GLManipulatorHandle for editing
 * transform component parameters/attributes.
 *
 * The handle can specify its own orientation (object, world, view) by
 * implementing getOrientation() in the subclass. The orientation will affect
 * the Local Transform of the handle. This is calculated in
 * calculateAndSetLocalXform(), which (like in GLTransformManipulator) is
 * called in draw() and pickerDraw(). This guarantees that the latest cooked
 * information is used in cases like in object space orientation, which depends
 * on the rotation of the location where the manipulator is placed.
 *
 * Each manipulator affects a specific transform component (rotation, scale,
 * translation), and the corresponding attribute name should be specified by
 * extending getComponentName().
 *
 * The manipulator class of a GLTransformManipulatorHandle should extend
 * GLTransformManipulator.
 *
 *       ** The dragging-to-location values transformation algorithm **
 *
 * The Orientation Transform (getOrientationXform()) is then used in two ways:
 *  - To draw and detect the dragging of the handle in the correct orientation
 *  - To transmit a local dragging value to the every manipulated location's
 *    local space, in order to correctly set thei new tranform values.
 *
 * A location's Component Transform is defined by its parent location's
 * transform and all the transform components (rotation, translation, scaling)
 * that are applied after the one being manipulated (see
 * ViewerDelegate::getPartialXform()). This is essential to allow the correct
 * manipulation of the component values in different transform orders (SRT,
 * RTS, TSR, etc).
 *
 * The way a local dragging is transferred to each of the location's local
 * space is via the Manipulation Transform (applyManipulationXform()). This is
 * different for each location and is defined by the its Orientation
 * Transform and by the location's Component Transform. This allows a drag on
 * a given orientation to be correctly applied to each location, even if
 * they contain different transformation orders and when the orientation type
 * depends on the location's transform (like in object space).
 *
 * This is achieved by implementing the GLManipulator::drag() function in the
 * subclass. This should detect a dragging in the handle's Local Space,
 * because when dragging a handle the manipulator's position and scale
 * calculated by the Manipulator Transform is irrelevant. The delta value
 * should then be converted into a delta matrix that applies that
 * transformation in local space. For example, in a translate manipulator axis
 * handle, this delta matrix should contain just a translation on that axis.
 * Then this matrix should be passed to the applyXformToAllLocations() function
 * as the 'xform' argument. This, in turn, will iterate through each location,
 * apply the corresponding Manipulation Matrix, that turns the local space
 * transform into the location's correct component space and pass it to the
 * virtual function applyXformToLocation(). This function should then be
 * implemented by the subclass, which will apply this transformation to the
 * location's values via the Manipulator::setValue() call.
 * 
 * The transformation matrix calculated in drag() can be relative to either
 * the previous dragging value or to the initial dragging value, and
 * applyXformToLocation() needs to follow that decision in order to produce
 * the correct values.
 *
 * Summary of the algorithm:
 *
 *  1 - When a mouse drag starts the initial location transforms are cached via
 *      get____XformFromLocation(), which should use Manipulator::getValue() to
 *      collect the transformation for the component being manipulated
 *
 *  2 - In the function drag() transform a dragged delta value into a local
 *      delta transform and there are two options:
 *      2.1 - Apply the delta transform to all the location via
 *            applyXformToAllLocations() (go to 3); 
 *      2.2 - Apply the delta transform to only one location. Make sure the
 *            Manipulation Transform to it via applyManipulationXform() and
 *            pass the result to applyXformToLocation() (go to 4)
 *
 *  3 - applyXformToAllLocations() iterates through each location and applies
 *      the Manipulation Transform to the local delta transform, passing the
 *      resulting location component's space delta matrix to
 *      applyXformToLocation() on each location;
 * 
 *  4 - applyXformToLocation() will then apply the delta transform to either
 *      the initial transform or to the current one (which can be retrieved
 *      via get____XformFromLocation()).
 *
 *  5 - Finally, applyXformToLocation() will extract the xform component values
 *      from the resulting transformation and set them back into Katan via
 *      Manipulator:setValue()
 *
 */
class GLTransformManipulatorHandle : public GLManipulatorHandle
{
public:
    /** @brief Constructor.
     *
     * @param alwaysAtLocationOrigin: Specifies if this manipulator should
     *    always be placed at the location's origin. Some manipulators
     *    (example: rotate, scale) have to be placed in a pivot position in
     *    some transform orders. Others, like the translate, can always be
     *    placed wherever the manipulated object is.
     */
    explicit GLTransformManipulatorHandle(bool alwaysAtLocationOrigin);

    /** @brief Destructor. */
    virtual ~GLTransformManipulatorHandle();

    /**
    * @brief Cancels the manipulation.
    *
    * Typically called before the handle is destroyed to discard any pending
    * transform that hasn't been made persistent while dragging the handle.
    */
    virtual void cancelManipulation();

    /**
     * @brief Draws the handle.
     *
     * This must be called by the implementation on child classes in order to
     * bind the standard shader (see GLManipulator::draw()) and to calculate
     * and set the handle's local xform. Since the transform of the location
     * where the manipulator is positioned can change when it is cooked, this
     * has to be recalculated here.
     */
    virtual void draw();

    /**
     * @brief Draws the handle for picking using the passed picker ID.
     *
     * This must be called by the implementation on child classes in order to
     * bind the standard shader (see GLManipulator::draw()) and to calculate
     * and set the handle's local xform. Since the transform of the location
     * where the manipulator is positioned can change when it is cooked, this
     * has to be recalculated here.
     */
    virtual void pickerDraw(int64_t pickerID);

    /**
     * @brief Defines the orientation space of the manipulator.
     *
     * This uses a "ManipulatorOrientation" option set on the Viewport that
     * specifies the orientation for the manipulator. This option's value is an
     * integer with the correspondent Orientation enum value.
     */
    virtual Orientation getOrientation();

    /**
     *  @brief Gets the display color for handles
     *
     *  This will return the display color for the handles based on if is
     *  hovered or active, the color passed as argument is the color to use
     *  if the handle is neither hovered or active.
     */
    Vec4f getDisplayColor(const Vec4f& color);

    /**
     * @brief The transform component (trans, rot, scale) attribute full name.
     *
     * This is used when calculating the manipulator's transform, and is fed to
     * ViewerDelegate::getPartialXform() as the componentName argument.
     *
     * Examples:
     *   xform.interactive.translate
     *   xform.interactive.rotate?
     */
    virtual std::string getComponentName() = 0;

    /**
     *  @brief: Gets the uniform scale of the manipulator in world space.
     *
     * This returns the manipulator's scale value cached in the previous draw()
     * call, which calculates the manipulator's transform. If draw() hasn't
     * been called recently this value can be out of date. This is the scale
     * value that keeps the manipulator with constant size on the Viewport.
     */
    double getScale() { return m_manipScale; }

    /**
     *  @brief: Gets the origin of the manipulator in world space.
     *
     * This returns the origin cached in the previous draw() call, which
     * calculates the manipulator's transform. If draw() hasn't been called
     * recently this value can be out of date.
     */
    Imath::V3d getOrigin() { return m_manipOrigin; }

    /**
     * @brief Indicates whether the manipulator should be placed on the
     * center of interest.
     *
     * The property is used in calculateAndSetXform().
     */
    void placeOnCenterOfInterest(bool placeOnCoi)
    {
        m_placeOnCoi = placeOnCoi;
    }

    /**
     * @brief Indicates whether the manipulator is placed on the center of
     * interest.
     */
    bool isPlacedOnCenterOfInterest() const
    {
        return m_placeOnCoi;
    }

    /**
     * @brief Sets the mode for the manipulator's handle.
     *
     * Manipulators such as the Translate or Rotate manipulator make use of this
     * property to correctly tweaks the objects' xform.
     */
    void setTransformMode(TransformMode transformMode)
    {
        m_transformMode = transformMode;
    }

    /**
     * @brief Gets the mode for the manipulator's handle.
     *
     * Manipulators such as the Translate or Rotate manipulator make use of this
     * property to correctly tweaks the objects' xform.
     */
    TransformMode getTransformMode() { return m_transformMode; }

protected:

    /// @brief Utility function: Gets the GLTransformManipulator instance.
    GLTransformManipulator* getGLTransformManipulator();

    /// Determines whether the axis is close to parallel to the camera.
    bool isParallelToCamera(const Imath::V3d& axis);

    /// Determines whether the axis is close to perpendicular to the camera.
    bool isPerpendicularToCamera(const Imath::V3d& axis);

    /// Returns the direction between the manipulator's projection onto window
    /// space and the camera origin (i.e. the direction of the ray towards the
    /// manipulator's origin).
    /// Used in isParallelToCamera() and isPerpendicularToCamera().
    Imath::V3d getCameraRayDirection();

    /**
     * @brief Called when a mouse dragging occurs.
     *
     * This must be called by the implementation on child classes. It caches
     * the initial Local Transform matrices at drag start for each manipulated
     * location, which are then provided to applyXformToLocation() in order to
     * calculate the resulting transform for those locations.
     *
     * See GLManipulator::startDrag()
     */
    void startDrag(const Vec3d& initialPointOnPlane
        , const Vec2i& initialMousePosition);

     /**
      * @brief Called when a mouse dragging occurs.
      *
      * This must be implemented by the child classes. This must transform a
      * dragging value into a local transform. This should then be passed to
      * applyXformToAllLocations() in order to be applied to every location.
      * If only one location is meant to be transformed, then
      * applyXformToLocation() should be used, and xform should be transformed
      * by getOrientationXform() and the result should be passed to
      * applyXformToLocation().
      */
    virtual void drag(const Vec3d& initialPointOnPlane,
                      const Vec3d& previousPointOnPlane,
                      const Vec3d& currentPointOnPlane,
                      const Vec2i& initialMousePosition,
                      const Vec2i& previousMousePosition,
                      const Vec2i& currentMousePosition,
                      bool isFinal) {}

    /**
     * @brief Called when a mouse dragging ends.
     *
     * This must be called by the implementation on child classes. It clears
     * the cache with the initial Local Transform matrices at drag start for
     * each manipulated location.
     *
     * See GLManipulator::endDrag()
     */
    void endDrag();

    /**
     *  @brief Applies a transformation to all the manipulated locations.
     *
     * This calls applyXformToLocation() on each of the manipulated locations.
     * The transformation is in manipulator's local space, and is transformed
     * by the Manipulation Transform (see applyManipulationXform()) in order to
     * be correctly applied to the locations' transform component attributes or
     * node parameters via applyXformToLocation().
     *
     * See Manipulator::setValue() for the definition of the isFinal flag.
     */
    void applyXformToAllLocations(const Imath::M44d& xform, bool isFinal);

    /**
     *  @brief Applies a transformation to a given manipulated location.
     *
     * This is meant to be implemented by subclasses and should use the method
     * Manipulator::setValue() to set the new transform attributes/parameters
     * back into Katana. This new transform can be calculated by applying the
     * given xform to either the initial transform at drag start (initialXform)
     * or to the current location's transform, via Manipulator::getValue() or
     * get____XformFromLocation(). The decision of using a transform relative to
     * the initial or current values is made in the implementation of the
     * drag() function.
     *
     * See Manipulator::setValue() for the definition of the isFinal flag.
     *
     * This will be typically called by applyXformToAllLocations().
     */
    virtual void applyXformToLocation(const std::string& locationPath,
        const Imath::M44d& xform, bool isFinal) = 0;

    /**
     * @brief Transform from manipulation space to transform component space.
     *
     * This defines the transformation from manipulation space to the
     * transform component that is being edited (scale, translate, rotate).
     *
     * The transform order will affect the component space.
     *
     * Manipulation space is defined by the amount that each of the axis is
     * affected during dragging. The position of the manipulator is irrelevant,
     * so only its orientation is involved in the calculation of this space.
     *
     * This is used to transform dragged values in local space into values that
     * can be set in the component's node graph parameters.
     */
    virtual Imath::M44d applyManipulationXform(
        const Imath::M44d& xform, const std::string& locationPath);

    /**
    * The priority of the handle for selection
    */
    int m_handlePriority;

    /**
     * @brief Calculates the manipulator transform.
     *
     * Used by the GLTransformManipulatorHandle::draw() and pickerDraw(). This
     * also caches the manipulator scale and origin returned by getScale() and
     * getOrigin().
     */
    Imath::M44d calculateXform(const std::string& locationPath);

    /**
     * @brief Updates the handle's Local Transform.
     *
     * Called by draw() and pickerDraw() and uses the transform of the last
     * manipulated location, where the manipulator is placed. This internally
     * calls GLManipulatorHandle::setLocalXform() in order to cache the local
     * transform.
     *
     * In the default implementation if this method, the Local Transform is
     * defined by the Orientation Transform given by getOrientationXform().
     */
    virtual void calculateAndSetLocalXform(const std::string& locationPath);

private:
    /**
     * @brief The Rotation portion of the Drawing Transform.
     *
     * This calculates the rotation of the handle and is used both in the local
     * transform of the handle (see calculateAndSetLocalXform()) and in the
     * calculation of the manipulation transform (see applyManipulationXform()).
     */
    virtual Imath::M44d getOrientationXform(const std::string& location);

    /**
     * @brief Restores the initial xform of a location.
     *
     * On `startDrag`, the initial transform of every location are stored
     * as part of the manipulator. If the user cancels the manipulation, those
     * transforms will be restored.
     */
    void restoreLocationXform(const std::string& locationPath);

    /**
     * @brief Gets the attribute name (attrSrc) from the manipulator and resets
     * it with a different name (attrDst).
     *
     * This is used by `restoreLocationXform`, to reset the xform of a location.
     */
    void copyLocationAttr(const std::string& locationPath,
                             const char* attrSrc,
                             const char* attrDst);
    /**
     * @brief Specifies if this manipulator should always be placed at the
     *        location's origin.
     *
     * Some manipulators (example: rotate, scale) have to be placed in a pivot
     * position in some transform orders. Others, like the translate, can
     * always be placed wherever the manipulated object is.
     */
    bool m_alwaysAtObjectOrigin;

    // Caches the scale of the manipulator. Set by calculateXform().
    double m_manipScale;

    // Caches the origin of the manipulator. Set by calculateXform().
    Imath::V3d m_manipOrigin;

    /** Determines whether the handle has to be placed on the COI. */
    bool m_placeOnCoi;

    /** Determines the handle's mode: default, COI or around COI. */
    TransformMode m_transformMode;
};


}
}
}

#endif /* GLTRANSFORMMANIPULATOR_H_ */
