// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef FNVIEWERMODIFIERINPUT__H
#define FNVIEWERMODIFIERINPUT__H

#include <FnViewerModifier/suite/FnViewerModifierSuite.h>
#include <FnPluginSystem/FnPluginSystem.h>
#include <FnPluginSystem/FnPlugin.h>
#include <FnAttribute/FnAttribute.h>

#include <string>

namespace Foundry
{
  namespace Katana
  {

    /**
     * \ingroup VMP
     * @{
     */

    /** @brief Provides functions that inspect and manipulate the Scenegraph
     * that contains the Location being processed by the VMP (the current Location).
     *
     * Contains methods for controlling live attributes in the scenegrapgh (live attributes
     * are those linked to parameters in the nodegraph)
     */

    class ViewerModifierInput
    {
    public:

      /**
       * Constructor. Only used internally by Katana - a VMP always receives
       * an already instantiated object of this type.
       */
      ViewerModifierInput(FnViewerModifierHostHandle handle);

      /**
       * Returns the name of the current Location. (ex: "pony" in "/root/world/geo/pony")
       */
      std::string   getName() const;

      /**
       * Returns the type of the current Location. (ex: "subdmesh" or "group")
       */
      std::string   getType() const;

      /**
       * Returns the full path name of the current Location. (ex: "/root/world/geo/pony")
       */
      std::string   getFullName() const;

      bool          isSelected() const;

      /**
       * Returns an existing Attribute from the specified Location, or from the
       * current Location if no location is specified. If there isn't any Attribute
       * with the given name on the given Location then an invalid Attribute is
       * returned (which can be checked with the function Attribute::isValid()).
       * Global attributes (inherited from parent Locations) will be ignored.
       *
       * @param name The name of the attribute - can be a child of a GroupAttribute
       *              (ex: "geometry.point.P")
       * @param atLocation The location from where the attribute will be retrieved
       *          (optional - if not specified the current Location will be used)
       */
      Attribute       getAttribute(const std::string& name, const std::string& atLocation="") const;

      /**
       * Returns an existing Attribute from the specified Location, or from the
       * current Location if no location is specified. If there isn't any Attribute
       * with the given name on the given Location then an invalid Attribute is
       * returned (which can be checked with the function Attribute::isValid()).
       * Global attributes (inherited from parent Locations) can be accessed.
       *
       * @param name The name of the attribute - can be a child of a GroupAttribute
       *              (ex: "geometry.point.P")
       * @param atLocation The location from where the attribute will be retrieved
       *          (optional - if not specified the current Location will be used)
       */
      Attribute       getGlobalAttribute(const std::string& name, const std::string& atLocation="") const;


      /**
       * Returns a Double Attribute with the worldspace transform matrix of a given Location.
       * Katana stores the transforms in a structured Group Attribute that contains the translate,
       * rotate and scale values on the 3 axis. This function crunches these attributes down into a
       * Matrix.
       * @param atLocation The location from where the tranforms will be retrieved.
       *             (optional - if not specified the current Location will be used)
       */
      DoubleAttribute getWorldSpaceXform(const std::string& atLocation="") const;

      /**
       * Returns true if the specified attribute has changed interactively.
       * @param attribute The name of the attribute.
       * @param atLocation The location of the attribute.
       *             (optional - if not specified the current Location will be used)
       */
      bool            isLiveAttribute(const std::string& attribute, const std::string& atLocation="") const;

      /**
       * Get the value of an attribute including any interactive changes.
       * If there are no interactive changes the result is identical with
       * getAttribute.
       * @param attribute The name of the attribute.
       * @param atLocation The location of the attribute.
       *             (optional - if not specified the current Location will be used)
       */
      Attribute       getLiveAttribute(const std::string& name, const std::string& atLocation="") const;

      /**
       * Returns true if the specified global attribute has changed interactively.
       * @param attribute The name of the attribute.
       * @param atLocation The location of the attribute.
       *             (optional - if not specified the current Location will be used)
       */
      bool            isGlobalLiveAttribute(const std::string& attribute, const std::string& atLocation="") const;

      /**
       * Get the value of an attribute including any interactive changes.
       * If there are no interactive changes the result is identical to
       * getAttribute.
       * @param attribute The name of the attribute.
       * @param atLocation The location of the attribute.
       *             (optional - if not specified the current Location will be used)
       */
      Attribute       getGlobalLiveAttribute(const std::string& name, const std::string& atLocation="") const;

      /**
       * This function is deprecated. getGlobalAttribute() or getAttribute()
       * should be used instead.
       */
      Attribute       getUniversalAttribute(const std::string& name, const std::string& atLocation="") const;

      /**
       * Returns true if the world space transform at the current (or
       * specified) location has changed interactively.
       */
      bool            isLiveWorldSpaceXform(const std::string& atLocation="") const;

      /**
       * Get the world space transform including any interactive changes.
       * @param atLocation The location from where the tranforms will be retrieved.
       *             (optional - if not specified the current Location will be used)
       */
      DoubleAttribute getLiveWorldSpaceXform(const std::string& atLocation="") const;


      /**
       * Fills a vector with the names of the children of the given Location.
       * @param names A reference to a vector of strings that will be filled with the child names.
       * @param atLocation The location for which the child names will be retrieved
       *          (optional - if not specified the current Location will be used)
       */
      void getChildNames(std::vector<std::string> &names, const std::string &atLocation="") const;


      /**
       * Allow Katana's GLSL shaders to run on the geometry created
       * by this GLM
       */
      void            setUseFixedFunctionPipeline(bool);

      /**
       * Tell Katana to draw this GLM in depth-sorted order so that
       * transparency works correctly
       */
      void            setTransparent(bool);

      /**
       * Get access to the various view and projection matrices for the
       * current viewer.
       * @param type The type of display matrix to retrieve.
       */
      DoubleAttribute getDisplayMatrix(MatrixType type) const;

      /**
       * Return the current GL viewport.
       */
      DoubleAttribute getViewport() const;

      /**
       * Query the draw state at the current location. Valid queries are:
       * fillPoints, fillWireframe, fillSolid, shadingFlat, shadingSmooth,
       * lightOff, lightDefault, lightShaded, proxyBBox, proxyGeo
       * proxyBoth, shadows, textures, selected, selectedAncestor,
       * highlight, highlightAncestor, invisible, invisibleAncestor,
       * normals, smoothOff, smoothLines, smoothPoints.
       */
      int                 getDrawOption(const std::string&) const;

      /**
       * Query a float from the draw state at the current location. Valid
       * queries are:
       * pointSize.
       */
      float               getDrawOptionFloat(const std::string&) const;

      /**
       * Query a string from the draw state at the current location.
       * Reserved for future use.
       */
      const std::string   getDrawOptionString(const std::string&) const;

      /**
       * If this is called at some point during deep setup, the GLM
       * host will bypass its own geometry drawing routine for the
       * location to which the GLM is attached.
       */
      void                overrideHostGeometry();

      /**
       * Returns true if the current location is currently being looked through.
       * @param includeChildren If true, consider descendants of looked-through
       * locations as also being looked through.
       */
      bool isLookedThrough(bool includeChildren=false);

      ///@cond FN_INTERNAL_DEV
      static void setHost(FnPluginHost* host);


    private:
      FnViewerModifierHostHandle            _handle;
      static FnViewerModifierHostSuite_v1*  _suite;
      static FnPluginHost*                  _host;

      ///@endcond
    };

    /// @}

  } // namespace Katana
} // namespace Foundry


#endif  // #ifndef FNVIEWERMODIFIERINPUT__H
