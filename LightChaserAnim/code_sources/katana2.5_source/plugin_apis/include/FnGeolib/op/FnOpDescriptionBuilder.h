#ifndef FnOpDescriptionBuilder_H
#define FnOpDescriptionBuilder_H

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnGroupBuilder.h>

#include <vector>
#include <string>
#include <iostream>

#include "ns.h"

/* This file defines the FnOpDescriptionBuilder and associated functionality.
 *
 * The FnOpDescriptionBuilder is intended to be used to aid developers in
 * documenting their Ops - describing what inputs and outputs the Op has, etc.
 * Whilst the FnOpDescriptionBuilder hides away underlying conventions, the
 * end result is a GroupAttribute. As such, you can bypass the
 * FnOpDescriptionBuilder and create a GroupAttribute manually yourself. Below
 * is a description of the attribute conventions expected in the GroupAttribute.
 *
 * Please note that these conventions are subject to change and using the
 * FnOpDescriptionBuilder is the preferred/recommended way of generating this
 * GroupAttribute.
 *
 * The top-level children of the GroupAttribute can be:
 *     * 'opSummary': A StringAttribute that gives a *brief* description of the
 *           Op
 *     * 'opHelp': A StringAttribute that gives a more detailed, potentially
 *           multi-line description of the Op.
 *     * 'opNumInputs': An IntAttribute representing the number of inputs the Op
 *           requires.
 *     * 'opArgs': A GroupAttribute whose children represent Attributes
 *           expected/required as opArgs to the Op (see below for structure)
 *     * 'inputAttrs': A GroupAttribute whose children represent Attributes
 *           that the Op will read from the input scene graph (see below for
 *           structure)
 *     * 'outputAttrs': A GroupAttribute whose children represent Attributes
 *           that the Op will write to the output scene graph (see below for
 *           structure)
 *     * 'meta': A GroupAttribute that is used to maintain ordering of opArgs,
 *           inputAttrs and outputAttrs. This GroupAttribute should have three
 *           children: 'opArgs', 'inputAttrs' and 'outputAttrs'. Each of these
 *           is expected to be a multi-sampled StringAttribute, with the names
 *           of the opArgs/inputAttrs/outputAttrs respectively, ordered
 *           appropriately. This ordering will be maintained in any UI display
 *           generated from this description.
 *
 * For the opArgs, inputAttrs and outputAttrs GroupAttributes, it is expected
 * that each of their children is another GroupAttribute, named as the name
 * of the respective attribute (including any "."s). Underneath that name is
 * then expected to be a special GroupAttribute named __FnOpDescription. This
 * is to help separate the description from the naming, when the attribute name
 * contains children (has "."s). The __FnOpDescription group can have the
 * following children (unless otherwise specified, these children are optional):
 *
 *     * 'description': (Required) A StringAttribute that describes the usage
 *           of the attribute in as much detail as necessary
 *     * 'type': (Required) An IntAttribute that describes the type of the
 *           attribute. This numeric value should correspond to one of the type
 *           values defined by the AttrTypeDescription enum below
 *     * 'optional': An IntAttribute that is 1 if the attribute isn't required
 *           for the Ops correct operation, 0 if it is required.
 *     * 'default': (Required iff optional=1) An Attribute that is only used if
 *           'optional' is 1. This represents the default value that is used if
 *           the (optional) attribute is not found by the Op
 *     * 'defaultDescription': (Required iff optional=1 and default is not
 *           specified) A StringAttribute that is only used if 'optional' is 1.
 *           This can be used in place of the 'default' value if it's not
 *           possible for you to provide a concrete default value, and instead
 *           wish to provide a description of what happens in the default case
 *     * 'inputIndex': An IntAttribute that defines which input index on the Op
 *           that the attribute will be read from. This is only used/expected
 *           in the inputAttrs GroupAttribute.
 *     * 'widgetHint': A StringAttribute that corresponds to a UI widget hint.
 *           This can be used by UI code to guess at an appropriate widget to
 *           use when providing a UI for the Op. Examples include 'cel',
 *           'scenegraphLocation', etc.
 *     * 'atLocation': A StringAttribute that can be used to specify a static
 *           scene graph location that the attribute will always be read
 *           from/written to. This is only used (and is optional) in the
 *           inputAttrs and outputAttrs GroupAttributes. This is useful if, for
 *           example, the attribute is always read from /root/world (for
 *           example for constraint or globals lists).
 *
 * There are some conventions that can be used when naming things:
 *
 *     * opArgs that are intended to be internal/private to the Op should be
 *       prefixed with an underscore. Often, Ops will call themselves as
 *       children and pass down modified and extra opArgs. In this case, if
 *       the opArg is not intended to ever be provided by external users of the
 *       Op (other than the Op itself), the opArg name should be prefixed with
 *       a "_".
 *
 *     * Wildcards ("*") can be used in attribute names. Some Ops may have a
 *       flexible interface, not specifically requiring a particular named
 *       attribute, but instead handling any attribute under a certain parent.
 *       One example of this is the StaticSceneCreate Op. This Op has three
 *       main expected opArgs: a, c and x. Whilst these are known values, the
 *       structure and nature of their children isn’t known, and so cannot be
 *       completely described. But we can describe what we do know and do want
 *       to restrict using wildcards: a.*, c.*, x.* and also x.*.opType and
 *       x.*.opArgs can be used as names when creating the opArgs
 *       GroupAttribute.
 */

FNGEOLIBOP_NAMESPACE_ENTER
{
    namespace FnOpDescription
    {
        /// An enumeration of attribute types that are valid for
        /// getOpArg/getAttr/setAttr calls. These can be OR'd together
        /// if multiple types are valid.
        typedef enum AttrTypeDescription {
            kTypeIntAttribute = 1,
            kTypeFloatAttribute = 2,
            kTypeDoubleAttribute = 4,
            kTypeStringAttribute = 8,
            kTypeGroupAttribute = 16,
            kTypeDynamic = 32, // type is inferred from some other value/thing
            kTypeNumericAttribute = kTypeIntAttribute |
                                    kTypeFloatAttribute |
                                    kTypeDoubleAttribute,
            kTypeAnyAttribute = kTypeIntAttribute |
                                kTypeFloatAttribute |
                                kTypeDoubleAttribute |
                                kTypeStringAttribute |
                                kTypeGroupAttribute
        } AttrTypeDescription;

        /// @brief Description of a generic Attribute.
        ///
        /// Description of a generic Attribute, including name, type and
        ///     a description of its usage.
        class AttrDescription
        {
            friend class FnOpDescriptionBuilder;

        public:
            virtual ~AttrDescription() {}

            /// @brief Get the description of the Attribute's usage.
            const std::string& getDescription() const { return m_description; }
            /// @brief Get the name of the Attribute.
            const std::string& getName() const { return m_name; }
            /// @brief Get the type (AttrTypeDescription) of the Attribute.
            AttrTypeDescription getAttrType() const { return m_type; }

            /// @brief Set the description of the Attribute's usage.
            void setDescription(const std::string& description)
            {
                m_description = description;
            }

        protected:
            virtual FnAttribute::GroupAttribute build() const
            {
                FnAttribute::GroupBuilder attrGb;

                attrGb.set("description",
                           FnAttribute::StringAttribute(m_description));
                attrGb.set("type",
                           FnAttribute::IntAttribute(m_type));

                return attrGb.build();
            }

            /// @param type The type (AttrTypeDescription) of the Attribute.
            /// @param name The name of the Attribute.
            AttrDescription(AttrTypeDescription type, const std::string& name)
                : m_type(type), m_name(name), m_description("") {}

        private:
            AttrTypeDescription m_type;
            std::string m_name;
            std::string m_description;
        };

        /// @brief Description of an Attribute that can be optional.
        ///
        /// Description of an Attribute that can be optional, and what its
        ///     default value is (if it is optional).
        class OptionalAttrDescription : public AttrDescription
        {
            friend class FnOpDescriptionBuilder;

        public:
            /// @brief Gets whether the Attribute is optional or not.
            bool isOptional() const { return m_optional; }
            /// @brief Gets the default value of the Attribute, if it's
            ///     optional.
            const FnAttribute::Attribute getDefaultValue() const
            {
                return m_defaultValue;
            }

            /// @brief Set whether the Attribute is optional.
            void setOptional(bool optional)
            {
                m_optional = optional;
            }
            /// @brief Set the default value of the Attribute. By calling this
            ///     with an attribute that isn't a NullAttribute, you implicitly
            ///     mark this Attribute as optional (so a call to setOptional()
            ///     is ... optional).
            void setDefaultValue(const FnAttribute::Attribute& defaultValue)
            {
                if (defaultValue.getType() != kFnKatAttributeTypeNull)
                {
                    setOptional(true);
                }
                m_defaultValue = defaultValue;
            }

            /// @brief If you can't give a concrete default value (via a call
            ///     to setDefaultValue()), you can use this to provide a
            ///     description of how not providing this Attribute will
            ///     affect the execution of the Op.
            void setDefaultDescription(const std::string& defaultDescription)
            {
                m_defaultDescription = defaultDescription;
            }

        protected:
            virtual FnAttribute::GroupAttribute build() const
            {
                FnAttribute::GroupBuilder attrGb;
                attrGb.update(AttrDescription::build());

                if (m_optional)
                {
                    attrGb.set("optional", FnAttribute::IntAttribute(1));

                    if (m_defaultValue.getType() != kFnKatAttributeTypeNull)
                    {
                        attrGb.set("default", m_defaultValue);
                    }
                    else if (!m_defaultDescription.empty())
                    {
                        attrGb.set("defaultDescription",
                                   FnAttribute::StringAttribute(
                                       m_defaultDescription));
                    }
                    else
                    {
                        throw std::runtime_error(
                            "Attribute declared as optional, but no default "
                            "value or description was given.");
                    }

                    attrGb.set("default", m_defaultValue);
                }

                return attrGb.build();
            }

            /// @param type The type (AttrTypeDescription) of the Attribute.
            /// @param name The name of the Attribute.
            OptionalAttrDescription(AttrTypeDescription type,
                    const std::string& name)
                : AttrDescription(type, name),
                  m_optional(false),
                  m_defaultValue(FnAttribute::NullAttribute()) {}

        private:
            bool m_optional;
            FnAttribute::Attribute m_defaultValue;
            std::string m_defaultDescription;
        };

        /// @brief Description of an Attribute used as an opArg.
        ///
        /// Description of an Attribute used as an opArg, including name,
        ///     type, whether it's optional or not, and what its widget hint
        ///     is (if any). The widgetHint should correspond to the widget
        ///     hint naming/conventions as used in Node parameters UIs.
        class OpArgDescription : public OptionalAttrDescription
        {
            friend class FnOpDescriptionBuilder;

        public:
            /// @param type The type (AttrTypeDescription) of the Attribute.
            /// @param name The name of the Attribute.
            OpArgDescription(AttrTypeDescription type, const std::string& name)
                : OptionalAttrDescription(type, name),
                  m_widgetHint("") {}

            /// @brief Get the widget hint of the Attribute. The widget hint
            ///     should correspond to the named values and syntax that is
            ///     used in Node parameter UIs.
            const std::string& getWidgetHint() const { return m_widgetHint; }

            /// @brief Set the widget hint for the Attribute. The widget hint
            ///     should correspond to the named values and syntax that is
            ///     used in Node parameter UIs.
            void setWidgetHint(const std::string& widgetHint)
            {
                m_widgetHint = widgetHint;
            }

        protected:
            virtual FnAttribute::GroupAttribute build() const
            {
                FnAttribute::GroupBuilder attrGb;
                attrGb.update(OptionalAttrDescription::build());

                if (!m_widgetHint.empty())
                {
                    attrGb.set("widgetHint",
                               FnAttribute::StringAttribute(m_widgetHint));
                }

                return attrGb.build();
            }

        private:
            std::string m_widgetHint;
        };

        /// @brief Description of an Attribute queried as an input through a
        ///     call to interface.getAttr().
        ///
        /// Description of an Attribute used as an input from the scene graph.
        class InputAttrDescription : public OptionalAttrDescription
        {
            friend class FnOpDescriptionBuilder;

        public:
            /// @param type The type (AttrTypeDescription) of the Attribute.
            /// @param name The name of the Attribute.
            InputAttrDescription(AttrTypeDescription type,
                                 const std::string& name)
                : OptionalAttrDescription(type, name),
                  m_inputIndex(0),
                  m_atLocation("") {}

            /// @brief Get the input index for the input scene graph that the
            ///     Attribute will be read from.
            int getInputIndex() const { return m_inputIndex; }
            /// @brief Get the specific location that Attribute will be read
            ///     from, if any.
            std::string getAtLocation() const { return m_atLocation; }

            /// @brief Set the input index for the input scene graph that the
            ///     Attribute will be read from.
            void setInputIndex(int inputIndex)
            {
                m_inputIndex = inputIndex;
            }
            /// @brief Set a specific location that the Attribute will be read
            ///     from, if any. This should be used if the Attribute is
            ///     always read from a specific location (e.g. /root/world),
            ///     rather than dynamically, based on where the Op is run.
            void setAtLocation(const std::string& atLocation)
            {
                m_atLocation = atLocation;
            }

        protected:
            virtual FnAttribute::GroupAttribute build() const
            {
                FnAttribute::GroupBuilder attrGb;
                attrGb.update(OptionalAttrDescription::build());

                attrGb.set("inputIndex",
                           FnAttribute::IntAttribute(m_inputIndex));

                if (!m_atLocation.empty())
                {
                    attrGb.set("atLocation",
                               FnAttribute::StringAttribute(m_atLocation));
                }

                return attrGb.build();
            }

        private:
            int m_inputIndex;
            std::string m_atLocation;
        };

        /// @brief Description of an Attribute output as a result to the output
        ///     scene graph through a call to interface.setAttr().
        ///
        /// Description of an Attribute used as an output to the scene graph.
        class OutputAttrDescription : public AttrDescription
        {
            friend class FnOpDescriptionBuilder;

        public:
            /// @param type The type (AttrTypeDescription) of the Attribute.
            /// @param name The name of the Attribute.
            OutputAttrDescription(AttrTypeDescription type,
                                  const std::string& name)
                : AttrDescription(type, name),
                  m_atLocation("") {}

            /// @brief Get the specific location that Attribute will be written
            ///     to, if any.
            std::string getAtLocation() const { return m_atLocation; }

            /// @brief Set a specific location that the Attribute will be
            ///     written to, if any. This should be used if the Attribute is
            ///     always written to a specific location (e.g. /root/world),
            ///     rather than dynamically, based on where the Op is run.
            void setAtLocation(const std::string& atLocation)
            {
                m_atLocation = atLocation;
            }

        protected:
            virtual FnAttribute::GroupAttribute build() const
            {
                FnAttribute::GroupBuilder attrGb;
                attrGb.update(AttrDescription::build());

                if (!m_atLocation.empty())
                {
                    attrGb.set("atLocation",
                               FnAttribute::StringAttribute(m_atLocation));
                }

                return attrGb.build();
            }

        private:
            std::string m_atLocation;
        };

        /// @brief Helper class for building attributes in Op describe()
        ///     functions.
        ///
        /// This class contains helper functionality for building the
        /// GroupAttribute that is required as a result of an Op's describe()
        /// function.
        class FnOpDescriptionBuilder
        {
        public:

            FnOpDescriptionBuilder() {}
            ~FnOpDescriptionBuilder() {}

            /// @brief Converts from AttrTypeDescription types into
            ///        FnKatAttributeType.
            ///
            /// Returns a vector of FnKatAttributeTypes that the given
            /// AttrTypeDescription represents.
            static std::vector<FnKatAttributeType>
            AttrTypeDescriptionToFnKatAttributeTypes(
                AttrTypeDescription attrType)
            {
                std::vector<FnKatAttributeType> result;
                if (attrType & kTypeIntAttribute)
                    result.push_back(kFnKatAttributeTypeInt);
                if (attrType & kTypeFloatAttribute)
                    result.push_back(kFnKatAttributeTypeFloat);
                if (attrType & kTypeDoubleAttribute)
                    result.push_back(kFnKatAttributeTypeDouble);
                if (attrType & kTypeStringAttribute)
                    result.push_back(kFnKatAttributeTypeString);
                if (attrType & kTypeGroupAttribute)
                    result.push_back(kFnKatAttributeTypeGroup);
                return result;
            }

            /// @brief Sets the short/brief one-line summary of the Op.
            ///
            /// The summary should be a short single sentence describing the
            /// Op's function.
            void setSummary(const std::string& summary)
            {
                m_gb.set("opSummary", FnAttribute::StringAttribute(summary));
            }

            /// @brief Sets the longer, more detailed help text of the Op.
            ///
            /// The help text/description should build on the summary, and go
            /// into more detail about the Ops functionality.
            void setHelp(const std::string& help)
            {
                m_gb.set("opHelp", FnAttribute::StringAttribute(help));
            }

            /// @brief The number of inputs to the Op.
            void setNumInputs(unsigned int numInputs)
            {
                m_gb.set("opNumInputs", FnAttribute::IntAttribute(numInputs));
            }

            /// @brief Build and return the result GroupAttribute.
            FnAttribute::GroupAttribute build()
            {
                // Add details of the ordering of the opArgs/inattrs/outattrs
                m_gb.set("meta.ordering.opArgs",
                         FnAttribute::StringAttribute(m_opArgOrder));
                m_gb.set("meta.ordering.inputAttrs",
                         FnAttribute::StringAttribute(m_inputAttrOrder));
                m_gb.set("meta.ordering.outputAttrs",
                         FnAttribute::StringAttribute(m_outputAttrOrder));

                // Reset state. m_gb.build() will reset the state of the
                // internal GroupBuilder, so reset our state too.
                m_opArgOrder.clear();
                m_inputAttrOrder.clear();
                m_outputAttrOrder.clear();

                return m_gb.build();
            }

            /// @brief Get the order that the opArg keys were specified in.
            std::vector<std::string> getOpArgOrder()
            {
                return m_opArgOrder;
            }

            /// @brief Get the order that the input attribute keys were
            ///     specified in.
            std::vector<std::string> getInputAttrOrder()
            {
                return m_inputAttrOrder;

            }

            /// @brief Get the order that the output attribute keys were
            ///     specified in.
            std::vector<std::string> getOutputAttrOrder()
            {
                return m_outputAttrOrder;
            }

            /// @brief Describe an Op arg that the Op may request via
            ///        getOpArg().
            ///
            /// Describe any opArgs that are requested through getOpArg.
            ///
            /// @param opArg The OpArgDescription detailing the opArg.
            void describeOpArg(const OpArgDescription& opArg)
            {
                set(std::string("opArgs.") + opArg.getName(), opArg.build());

                // Store the name in a vector to maintain order
                m_opArgOrder.push_back(opArg.getName());
            }

            /// @brief Describe an Attribute that may be queried via getAttr().
            ///
            /// Describe any Attributes that are queried from the scene
            /// graph through a call to getAttr() on the Op's interface.
            ///
            /// @param inputAttr The InputAttrDescription detailing the
            ///     attribute.
            void describeInputAttr(const InputAttrDescription& inputAttr)
            {
                set(std::string("inputAttrs.") + inputAttr.getName(),
                    inputAttr.build());

                // Store the name in a vector to maintain order
                m_inputAttrOrder.push_back(inputAttr.getName());
            }

            /// @brief Describe an Attribute that may be set via setAttr().
            ///
            /// Describe any Attributes that are set in the scene
            /// graph through a call to setAttr() on the Op's interface.
            ///
            /// @param outputAttr The OutputAttrDescription detailing the
            ///     attribute.
            void describeOutputAttr(const OutputAttrDescription& outputAttr)
            {
                set(std::string("outputAttrs.") + outputAttr.getName(),
                    outputAttr.build());

                // Store the name in a vector to maintain order
                m_outputAttrOrder.push_back(outputAttr.getName());
            }

        private:

            void set(const std::string& name,
                     const FnAttribute::Attribute& attr)
            {
                const std::string keyedName = name + ".__FnOpDescription";
                m_gb.set(keyedName, attr);
            }

            FnAttribute::GroupBuilder m_gb;

            std::vector<std::string> m_opArgOrder;
            std::vector<std::string> m_inputAttrOrder;
            std::vector<std::string> m_outputAttrOrder;
        };
    }
}
FNGEOLIBOP_NAMESPACE_EXIT

#endif
