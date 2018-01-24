// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef FNVIEWERMODIFIER__H
#define FNVIEWERMODIFIER__H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnPluginSystem/FnPlugin.h>
#include <FnViewerModifier/suite/FnViewerModifierSuite.h>
#include <FnViewerModifier/plugin/FnViewerModifierInput.h>
#include <FnAttribute/FnAttribute.h>

namespace Foundry
{
  namespace Katana
  {
  /**
   * \defgroup VMP Viewer Modifier Plug-in (VMP) API
   * @{
   *
   * @brief An VMP is overrides the default rendering at a given Scenegraph
   * location type in the viewer.
   *
   * An VMP will execute OpenGL code to render in the viewer for a particular
   * location type. It will not modify the Scenegraph location in anyway.
   *
   * In order to implement a VMP the ViewerModifier abstract class must be
   * extended and registered using the <i>DEFINE_VMP_PLUGIN(class)</i>
   * and the <i>REGISTER_PLUGIN(class, name, major_version, minor_version)</i> macros.
   *
   */


  /**
   * @brief Class that represents a VMP. Must be be extended in an VMP.
   *
   * It is responsible for Rendering specific scene graph location types in the
   * viewer.
   * This class accepts arguments that can define how this processing should happen.
   * These arguments are passed to the constructor as a GroupAttribute
   * with children.
   */
    class ViewerModifier
    {
    public:
      ViewerModifier(GroupAttribute args) : _args(args) {}
      virtual ~ViewerModifier() {}

      static GroupAttribute         getArgumentTemplate();
      static const char*            getLocationType();

      /**
       * Called per VMP instance before each draw
       */
      virtual void                  deepSetup(ViewerModifierInput& input) = 0;

      /**
       * Called once per VMP instance when constructed
       */
      virtual void                  setup(ViewerModifierInput& input) = 0;

      /**
       * Performs the render code.
       */
      virtual void                  draw(ViewerModifierInput& input) = 0;

      /**
       * Called when the GLM's host location is removed/refreshed.
       */
      virtual void                  cleanup(ViewerModifierInput& input) = 0;

      /**
       * Called per VMP instance after each draw
       */
      virtual void                  deepCleanup(ViewerModifierInput& input) = 0;

      /**
       * WARNING: This is a deprecated function! Please use getLocalSpaceBoundingBox() instead
       */
      virtual DoubleAttribute       getWorldSpaceBoundingBox(ViewerModifierInput& input);

      /**
       * Returns a DoubleAttribute representing the local space bounding box
       * for the current location to be used with the viewer scenegraph, and
       * useful for culling.
       *
       * The bounding box is represented by 6 double values, that are
       * the interleaved coordinates of minimum and maximum vertices of the
       * bounding box (xMin, xMax, yMin, yMax, zMin, zMax).
       *
       * @param input The input interface to Katana's scenegraph.
       * @return The double attribute representing the bounding box.
       */
      virtual DoubleAttribute
          getLocalSpaceBoundingBox(ViewerModifierInput& input);

      ///@cond FN_INTERNAL_DEV
      static FnViewerModifierHandle newViewerModifierHandle(ViewerModifier* vm);

      /**
       * Gets the pointer to the FnPluginHost structure.
       */
      static FnPluginHost*          getHost();

      /**
       * Sets the pointer to a FnPluginHost structure.
       */
      static FnPlugStatus           setHost(FnPluginHost* host);

      static FnViewerModifierPluginSuite createSuite(
          FnViewerModifierHandle (*create)(FnAttributeHandle),
          FnAttributeHandle (*getArgumentTemplate)(),
          const char* (*getLocationType)(),
          void (*onFrameBegin)(),
          void (*onFrameEnd)());

      static FnViewerModifierPluginSuite pluginSuite;
      static unsigned int                   _apiVersion;
      static const char*                    _apiName;


    protected:
      GroupAttribute          getArguments() { return _args; }

    private:
      static FnPluginHost*  _host;
      GroupAttribute        _args;

      ///@endcond

    };

    /// @}

  } // namespace Katana
} // namespace Foundry


/// @cond FN_INTERNAL_DEV

// Plugin Registering Macro.

#define DEFINE_VMP_PLUGIN(VMP_CLASS)                                                \
                                                                                    \
    FnPlugin VMP_CLASS##_plugin;                                                    \
                                                                                    \
    FnViewerModifierHandle VMP_CLASS##_create(FnAttributeHandle args)               \
    {                                                                               \
         return Foundry::Katana::ViewerModifier::newViewerModifierHandle(           \
            VMP_CLASS::create(                                                      \
                Foundry::Katana::Attribute::CreateAndRetain(args)) );               \
    }                                                                               \
                                                                                    \
    FnAttributeHandle VMP_CLASS##_getArgumentTemplate()                             \
    {                                                                               \
        Foundry::Katana::GroupAttribute attr = VMP_CLASS::getArgumentTemplate();    \
        return attr.getRetainedHandle();                                            \
    }                                                                               \
                                                                                    \
    const char* VMP_CLASS##_getLocationType()                                       \
    {                                                                               \
      return VMP_CLASS::getLocationType();                                          \
    }                                                                               \
                                                                                    \
    void VMP_CLASS##_onFrameBegin()                                                 \
    {                                                                               \
      return VMP_CLASS::onFrameBegin();                                             \
    }                                                                               \
                                                                                    \
    void VMP_CLASS##_onFrameEnd()                                                   \
    {                                                                               \
      return VMP_CLASS::onFrameEnd();                                               \
    }                                                                               \
                                                                                    \
    FnViewerModifierPluginSuite VMP_CLASS##_suite =                                 \
    Foundry::Katana::ViewerModifier::createSuite( VMP_CLASS##_create,               \
                                          VMP_CLASS##_getArgumentTemplate,          \
                                          VMP_CLASS##_getLocationType,              \
                                          VMP_CLASS##_onFrameBegin,                 \
                                          VMP_CLASS##_onFrameEnd);                  \
                                                                                    \
    const void* VMP_CLASS##_getSuite()                                              \
    {                                                                               \
        return &VMP_CLASS##_suite;                                                  \
    }                                                                               \


///@endcond

#endif  // #ifndef FNVIEWERMODIFIER__H
