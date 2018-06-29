// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnAttributeModifierInput_H
#define FnAttributeModifierInput_H

#include <string>
#include <vector>

#include <FnAttribute/FnAttribute.h>
#include <FnAttributeModifier/FnAttributeModifierAPI.h>
#include <FnAttributeModifier/suite/FnAttributeModifierSuite.h>

namespace Foundry
{
    namespace Katana
    {

        /**
         * \ingroup AMP
         * @{
         */

        /** @brief Provides functions that inspect and manipulate the Scenegraph
         * that contains the Location being processed by the AMP (the current Location).
         *
         * Once fed into an AMP's AttributeModifier::cook() function, it
         * provides a methods to override/remove Attributes in the current Location
         * and allows the inspection of the Attributes at different Locations.
         *
         */
        class FNATTRIBUTEMODIFIER_API AttributeModifierInput
        {
        public:
            /**
             * Constructor. Only used internally by Katana - an AMP always receives
             * an already instantiated object of this type.
             */
            AttributeModifierInput(FnAttributeModifierHostHandle handle);

            /**
             * Returns the name of the current Location. (ex: "pony" in "/root/world/geo/pony")
             */
            std::string getName() const;

            /**
             * Returns the full path name of the current Location. (ex: "/root/world/geo/pony")
             */
            std::string getFullName() const;

            /**
             * Returns the type of the current Location. (ex: "subdmesh" or "group")
             */
            std::string getType() const;

            /**
             * Returns the current frame time (as selected in Katana's timebar)
             */
            float getFrameTime() const;

            /**
             * Provides a AttributeModifier with a string representation of current
             *  host (i.e. "katana 1.0") for version specific behavior.
             */
            std::string getHostString() const;

            /**
             * When motion blur is being used then several time samples will be
             * processed from a start time to an end time. This function returns the
             * start time of the current frame.
             */
            float getShutterOpen() const;

            /**
             * When motion blur is being used then several time samples will be
             * processed from a start time to an end time. This function returns the
             * end time of the current frame.
             */
            float getShutterClose() const;


            /**
             * When motion blur is being used then several time samples will be processed.
             * This function Returns the number of motion blur samples chosen on the scene.
             */
            int getNumSamples() const;

            /**
             * Function that overrides or creates an Attribute under the current Location.
             * This is the function that must be called from within
             * AttributeModifier::cook() function in order to arr/modify an attribute.
             * In order to delete an attribute an NullAttribute can be used with the
             * name of the attribute to be deleted.
             *
             * @param name Name of the attribute to be modified/created/deleted.
             * @param attr The new attribute that should be added or replace the existing one.
             *             If this is a null pointer (0x0, not a NullAttribute), any existing
             *             attribute with this name will be deleted.
             * @param inheritGroup If the attribute being overridden is a Group Attribute
             *                      this flag defines if this group will be inherited by
             *                      the child Locations of the current Location.
             *
             */
            void addOverride(const std::string &name, Attribute attr, bool inheritGroup);

            /**
             * Returns an existing Attribute from the specified Location, or from the
             * current Location if no location is specified. If there isn't any Attribute
             * with the given name on the given Location then an invalid Attribute is
             * returned (which can be checked with the function Attribute::isValid()).
             *
             * @param name The name of the attribute - can be a child of a GroupAttribute
             *              (ex: "geometry.point.P")
             * @param global If false then the global attributes (inherited from parent
             *               Locations) will be ignored. In this case if the requested
             *               Attribute is global, then an invalid attribute is returned.
             * @param atLocation The location from where the attribute will be retrieved
             *          (optional - if not specified the current Location will be used)
             * @return A valid Attribute if there is an attribute with the given name
             *         on the given location, or an invalid one otherwise. If the <i>global</i>
             *         parameter is set to false and the Attribute is a global attribute
             *         then an invalid attribute is returned.
             */
            Attribute getAttribute(const std::string &name, bool global=false, const std::string &atLocation="") const;

            /**
             * Fills a vector of strings with the names of the top level
             * attributes (which doesn't include the children of Group Attributes)
             * on the current Location.
             *
             * @param names A reference to a vector that will be filled with the Attribute names
             * @param global If false then the global attributes (inherited from parent
             *               Locations) will be ignored.
             */
            void getAttributeNames(std::vector<std::string> &names, bool global=false) const;

            /**
             * Returns a Double Attribute with the world space transform matrix of a given Location.
             * Katana stores the transforms in a structured Group Attribute that contains the translate,
             * rotate and scale values on the three axes. This function crunches these attributes down into a
             * Matrix.
             * @multiSample If true then all the samples of the Transform stored on the Location will
             *              be returned in the resulting Double Attribute (that will return the number of
             *              different matrices using the DoubleAttribute::getNumberOfTuples() function)
             * @param atLocation The location from where the transforms will be retrieved
             *             (optional - if not specified the current Location will be used)
             */
            DoubleAttribute getWorldSpaceXform(bool multiSample=false, const std::string &atLocation="") const;

            /**
             * Returns a Double Attribute with the world space transform matrix of a given Location
             * and for a certain set of sample times.
             * @param samples A vector of float containing the samples for which the transforms will be returned.
             * @param atLocation The location from where the attribute names will be retrieved
             *          (optional - if not specified the current Location will be used)
             */
            DoubleAttribute getWorldSpaceXformForSamples(const std::vector<float> &samples,
                const std::string &atLocation="") const;

            /**
             * Fills a vector with the names of the children of the given Location.
             * @param names A reference to a vector of strings that will be filled with the child names.
             * @param atLocation The location for which the child names will be retrieved
             *          (optional - if not specified the current Location will be used)
             */
            void getChildNames(std::vector<std::string> &names, const std::string &atLocation="") const;






            // HELPER FUNCTIONS, not part of C API
            /**
             * Convenience function for reporting error via attribute overrides.
             * Will set "errorMessage" attribute and optionally, set "type" to "error"
             * (for a fatal error).
             */
            void addErrorOverride(const std::string &errorMessage, bool isFatal);

            /**
             * Set an attribute at this location.  Overrides existing attribute or adds
             * new attribute.  Equivalent to "addOverride(name, attr, inheritGroup)".
             *
             */
            void setAttribute(const std::string &name, Attribute attr, bool inheritGroup);

            /**
             * Delete an attribute at this location.
             * Equivalent to "addOverride(name, 0x0, false)".
             *
             */
            void deleteAttribute(const std::string &name);


            ///@cond FN_INTERNAL_DEV
            static void setHost(FnPluginHost *host);

        private:
            FnAttributeModifierHostHandle _handle;
            static FnAttributeModifierHostSuite_v2 *_suite;
            static FnPluginHost *_host;

             ///@endcond

        };

        /// @}

    }//namespace
}//namespace

namespace FnKat = Foundry::Katana;

#endif // FnAttributeModifier_H
