// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef FnAttributeModifier_H
#define FnAttributeModifier_H

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnPluginSystem/FnPlugin.h>
#include <FnAttributeModifier/suite/FnAttributeModifierSuite.h>
#include <FnAttributeModifier/plugin/FnAttributeModifierInput.h>
#include <FnAttribute/FnAttribute.h>

#include <string>
#include <stdio.h>

namespace Foundry
{
    namespace Katana
    {

        /**
         * \defgroup AMP Attribute Modifier Plug-in (AMP) API
         * @{
         *
         * @brief An AMP is a Scenegraph filter that creates, modifies and/or deletes
         * Attributes on a given Location.
         *
         * An AMP plug-in will never change the
         * Scenegraph Locations topology, it will only affect the Attributes under
         * the given Locations.
         *
         * In order to implement an AMP plugin the AttributeModifier abstract
         * class must be extended and registered using the <i>DEFINE_AMP_PLUGIN(class)</i>
         * and the <i>REGISTER_PLUGIN(class, name, major_version, minor_version)</i> macros.
         *
         * In order to use these plugins a AttributeModifierDefine node should be used. When
         * the <i>applyWhen</i> parameter is set to "immediate" the AMP will execute immediately
         * once the locations are evaluated. If it is set to "deferred" the plugin will execute
         * with an implicit resolver (triggered by a render, for example) or forced to resolve
         * by using the AttributeModifierResolve node.
         *
         */


        /**
         * @brief Class that represents an AMP. Must be be extended in an AMP plugin.
         *
         * It is responsible for processing the Attributes on a given Location.
         * This class accepts arguments that can define how this processing should happen.
         * These arguments are passed to the constructor as a GroupAttribute
         * with children.
         */
        class AttributeModifier
        {
        public:

            /**
             * Constructor. Any initialization should be done here.
             *
             * @param args An GroupAttribute that can contain several Attributes
             *             that can be used as arguments for the AMP. The valuese of
             *             these argument Attributes can be stored in member variables
             *             or can be used to initialize internal data-structures in
             *             AttributeModifier's sub-class. These member variables and
             *             data-structures can be later used by the cook() method.
             *
             */
            AttributeModifier(GroupAttribute args) : _args(args) {}
            virtual ~AttributeModifier() {}

            /**
             * A static function that allows Katana to query the AMP about which arguments
             * it expects. This should return a GroupAttribute with the same structure
             * as the one that is passed to the constructor, in which the Attribute values
             * will be the default values of the correspondent AMP argument.
             */
            static GroupAttribute getArgumentTemplate();

            /**
             * Returns an hash that defines if an AMP should run or not. In its default
             * implementation the hash is automatically generated from the AMP argument
             * values, which means that it will force the AMP to run whenever the value of
             * an argument is changed by the user. This function can also be overriden so
             * that the conditions in which an AMP should execute again can be under the
             * developer's control.
             */
            virtual std::string getCacheId();

            /**
             * The function that does all the Scenegraph Attributes processing on
             * a given location. It receives an AttributeModifierInput which
             * provides all the information related with the location being processed.
             * It should make use of the addOverride() function on AttributeModifierInput
             * to perform the attribute changes.
             *
             */
            virtual void cook(AttributeModifierInput &input) = 0;


            /// @cond FN_INTERNAL_DEV

            static FnAttributeModifierHandle newAttributeModifierHandle(AttributeModifier *am);

            /**
             * Sets the pointer to a FnPluginHost structure.
             */
            static FnPlugStatus setHost(FnPluginHost *host);

            /**
             * Gets the pointer to the FnPluginHost structure.
             */
            static FnPluginHost *getHost();

            static FnAttributeModifierPluginSuite_v1 createSuite(
                        FnAttributeModifierHandle (*create)(FnAttributeHandle),
                        FnAttributeHandle (*getArgumentTemplate)() );

            static FnAttributeModifierPluginSuite_v1 pluginSuite;
            static unsigned int _apiVersion;
            static const char* _apiName;

        private:

            static FnPluginHost *_host;
            GroupAttribute _args;

            /// @endcond
        };

        /// @}

    }//namespace
}//namespace

namespace FnKat = Foundry::Katana;


/// @cond FN_INTERNAL_DEV

// Plugin Registering Macro.

#define DEFINE_AMP_PLUGIN(AMP_CLASS)                                                \
                                                                                    \
    FnPlugin AMP_CLASS##_plugin;                                                    \
                                                                                    \
    FnAttributeModifierHandle AMP_CLASS##_create(FnAttributeHandle args)            \
    {                                                                               \
         return Foundry::Katana::AttributeModifier::newAttributeModifierHandle(     \
                   AMP_CLASS::create(Foundry::Katana::Attribute::CreateAndRetain(args)) );   \
    }                                                                               \
                                                                                    \
    FnAttributeHandle AMP_CLASS##_getArgumentTemplate()                             \
    {                                                                               \
        Foundry::Katana::GroupAttribute attr = AMP_CLASS::getArgumentTemplate();    \
        return attr.getRetainedHandle();                                            \
    }                                                                               \
                                                                                    \
    FnAttributeModifierPluginSuite_v1 AMP_CLASS##_suite =                           \
    Foundry::Katana::AttributeModifier::createSuite( AMP_CLASS##_create,            \
                                          AMP_CLASS##_getArgumentTemplate);         \
                                                                                    \
    const void* AMP_CLASS##_getSuite()                                              \
    {                                                                               \
        return &AMP_CLASS##_suite;                                                  \
    }                                                                               \


///@endcond

#endif // FnAttributeModifier_H
