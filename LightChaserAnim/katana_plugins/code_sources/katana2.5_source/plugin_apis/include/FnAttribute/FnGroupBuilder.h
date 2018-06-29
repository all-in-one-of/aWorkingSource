// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FoundryKatanaGroupBuilder_H
#define FoundryKatanaGroupBuilder_H

#include <FnAttribute/FnAttribute.h>
#include <FnAttribute/FnAttributeAPI.h>
#include <FnAttribute/ns.h>

FNATTRIBUTE_NAMESPACE_ENTER
{
    /**
     * \ingroup FnAttribute
     * @{
     */

    /**
     * @brief A factory class for constructing GroupAttribute objects.
     *
     * Typical usage involves creating a GroupBuilder, adding attributes to it
     * with the set() method, and, when finished, retrieving a newly
     * constructed GroupAttribute using the GroupBuilder's build() method.
     *
     * \warning There is currently no way to inspect the contents of the
     * builder, other than by calling build() and inspecting the generated
     * GroupAttribute. Note that by default build() clears the contents of the
     * builder; to override this behaviour pass GroupBuilder::BuildAndRetain to
     * build().
     *
     * As a convenience, GroupBuilder has support for creating arbitrarily
     * nested GroupAttribute structures by passing a dot-delimited string to
     * set(), which is referred to as a "path". Note that this implies that the
     * '.' character is <em>not</em> a valid attribute name!
     *
     * Example:
     * \code
     * GroupBuilder gb;
     * gb.set("my.nested.attribute", IntAttribute(2));
     * gb.set("myTopLevelAttribute", StringAttribute("taco"));
     * gb.set("myOtherTopLevelAttribute", FloatAttribute(4.0f));
     *
     * GroupAttribute groupAttribute = gb.build();
     *
     * // Following the call to build(), |gb| is empty and |groupAttribute| has
     * // the following structure:
     * //
     * // {
     * //   "my": {
     * //     "nested": {
     * //       "attribute": IntAttribute(2)
     * //      }
     * //   },
     * //   "myTopLevelAttribute": StringAttribute("taco"),
     * //   "myOtherTopLevelAttribute": FloatAttribute(4.0f)
     * // }
     * \endcode
     */

    class FNATTRIBUTE_API GroupBuilder
    {
    public:
        /**
         * Creates a new, empty GroupBuilder object.
         *
         * \remark The builder is created with GroupBuilder::BuilderModeNormal.
         */
        GroupBuilder() : _handle(0) {}

        typedef enum BuilderMode
        {
            /**
             * The "normal" build mode, which allows the full suite of
             * GroupBuilder functionality. This is the default.
             */
            BuilderModeNormal = kFnKatGroupBuilderModeNormal,

            /**
             * An advanced option that enables a more restrictive but higher
             * performance mode of operation.
             *
             * When working in this mode:
             * - callers must not pass dot-delimited paths to the set() method
             * - callers must not make multiple calls to set() using the same
             *   path
             * - deletions are disallowed: the del() method becomes a no-op.
             */
            BuilderModeStrict = kFnKatGroupBuilderModeStrict
        } BuilderMode;

        /**
         * Creates a new, empty GroupBuilder that uses the specified
         * \link GroupBuilder::BuilderMode BuilderMode\endlink.
         */
        GroupBuilder(BuilderMode builderMode) : _handle(getSuite()->createGroupBuilder2((BuilderModeType) builderMode)) {}

        ~GroupBuilder() {
            if (_handle != 0x0)
                getSuite()->releaseGroupBuilder(_handle);
        }

        bool isValid() const {
            return _handle != 0x0;
        }

        /**
         * Sets the value for the attribute identified by the given path.
         *
         * If \p path refers to an existing attribute in the builder and the
         * builder was created with GroupBuilder::BuilderModeNormal, the
         * attribute is replaced.
         *
         * @param path The path of the attribute whose value to set.
         * @param attr The attribute object to set for the given path.
         * @param groupInherit If \p path is a dot-delimited path, specifies the
         * \e groupInherit flag for any new groups added by this set() call.
         *
         * \remark If the builder was created with
         * GroupBuilder::BuilderModeStrict, note the caveats (documented above)
         * for working in this mode.
         *
         * \remark The empty string ("") is not a valid path component.
         */

        GroupBuilder & set(const std::string &path, const Attribute &attr,
            bool groupInherit=true)
        {
            lazyCreate();
            getSuite()->setGroupBuilder(_handle, path.c_str(),
                int32_t(path.size()), attr.getHandle(), uint8_t(groupInherit));
            return (*this);
        }

        GroupBuilder & set(const char* path, const Attribute &attr,
            bool groupInherit=true)
        {
            lazyCreate();
            getSuite()->setGroupBuilder(_handle, path, int32_t(strlen(path)),
                attr.getHandle(), uint8_t(groupInherit));
            return (*this);
        }

        /**
         * Sets the value for an attribute identified using a unique name
         * derived from the given path.
         *
         * If no attribute exists for the given path, this is equivalent to
         * calling set(). Otherwise, setWithUniqueName() chooses a new path by
         * suffixing the given path with an integer.
         *
         * @param path The base path of the attribute whose value to set.
         * @param attr The attribute object to set for the resulting path.
         * @param groupInherit If \p path is a dot-delimited path, specifies the
         * \e groupInherit flag for any new groups added by this set() call.
         */

        GroupBuilder & setWithUniqueName(const std::string &path, const Attribute &attr,
            bool groupInherit=true)
        {
            lazyCreate();
            getSuite()->setGroupBuilderUnique(_handle, path.c_str(),
                int32_t(path.size()), attr.getHandle(), uint8_t(groupInherit));
            return (*this);
        }

        GroupBuilder & setWithUniqueName(const char* path, const Attribute &attr,
            bool groupInherit=true)
        {
            lazyCreate();
            getSuite()->setGroupBuilderUnique(_handle, path, int32_t(strlen(path)),
                attr.getHandle(), uint8_t(groupInherit));
            return (*this);
        }

        /**
         * Deletes the attribute of the builder specified with the given \p
         * path.
         *
         * \remark This has no effect if the builder was created with
         * GroupBuilder::BuildModeStrict.
         */
        GroupBuilder & del(const std::string &path)
        {
            lazyCreate();
            getSuite()->delGroupBuilder(_handle, path.c_str(), int32_t(path.size()));
            return (*this);
        }

        GroupBuilder & del(const char* path)
        {
            lazyCreate();
            getSuite()->delGroupBuilder(_handle, path, int32_t(strlen(path)));
            return (*this);
        }


        /**
         * Updates the contents of the builder with the attributes
         * from the given GroupAttribute.
         *
         * Any new attributes with the same names as existing attributes
         * replace the old ones. Existing attributes not matching new
         * attributes are left intact. (This is analogous to the Python
         * dictionary's update method.)
         *
         * If setGroupInherit() has not been previously called, the builder
         * will also adopt the incoming GroupAttribute's \e groupInherit flag.
         *
         * @param attr  A GroupAttribute containing new attributes to add
         */
        GroupBuilder & update(const GroupAttribute &attr)
        {
            if (attr.isValid()) {
                lazyCreate();
                getSuite()->updateGroupBuilder(_handle, attr.getHandle());
            }
            return (*this);
        }

        /**
         * Recursively updates the contents of the builder with the attributes
         * held within the provided group attribute.
         *
         * Groups are traversed until set operations can be applied at the
         * leaves which are <em>not</em> GroupAttributes themselves.
         *
         * If setGroupInherit() has not been previously called, the builder
         * will also adopt the incoming GroupAttribute's \e groupInherit flag.
         */
        GroupBuilder & deepUpdate(const GroupAttribute &attr)
        {
            if (attr.isValid()) {
                lazyCreate();
                getSuite()->deepUpdateGroupBuilder(_handle, attr.getHandle());
            }
            return (*this);
        }

        /**
         * Reserves space for \p n attributes.
         *
         * This is an optimisation only. Calling reserve() before adding
         * attributes will avoid having to reallocate internal data structures.
         *
         * @param n The number of attributes the group attribute is to have.
         */
        GroupBuilder & reserve(int64_t n)
        {
            if (n > 0) {
                lazyCreate();
                getSuite()->reserveGroupBuilder(_handle, n);
            }
            return (*this);
        }

        /**
         * Sets a special flag on the builder that determines the value
         * returned by GroupAttribute::getGroupInherit() for the top-level
         * GroupAttribute returned by build().
         *
         * This \p groupInherit flag is <em>sticky</em>, so once it's been set
         * -- either through an explicit call to setGroupInherit(), or
         * indirectly via a call to update()/deepUpdate() -- further calls to
         * setGroupInherit() will have no effect.
         */
        GroupBuilder & setGroupInherit(bool groupInherit)
        {
            if (_handle || !groupInherit) {
                lazyCreate();
                getSuite()->setGroupBuilderInherit(_handle, (uint8_t) groupInherit);
            }
            return (*this);
        }

        /**
         * Sorts the top-level attributes of the builder by name.
         *
         * \remark sort() uses bytewise lexicographic ordering.
         */
        GroupBuilder & sort()
        {
            if (_handle)
                getSuite()->sortGroupBuilder(_handle);
            return (*this);
        }

        /**
         * \enum BuilderBuildMode
         * Flags to control the behaviour of the build() method.
         */
        typedef enum BuilderBuildMode
        {
            /**
             * Specifies that the builder's contents are cleared following a
             * call to build(). This is the default.
             */
            BuildAndFlush = kFnKatGroupBuilderBuildAndFlush,

            /**
             * Specifies that the builder's contents are retained following a
             * call to build().
             */
            BuildAndRetain = kFnKatGroupBuilderBuildAndRetain
        } BuilderBuildMode;

        /**
         * Returns a newly created group attribute with the contents of the
         * builder.
         *
         * @param builderMode An optional argument specifying whether the
         * builder's contents are retained.
         *
         * \warning By default, the contents of the builder are cleared
         * following a call to build(). To reuse the builder while retaining
         * its contents, pass GroupBuilder::BuildAndRetain for the optional
         * \p builderMode argument.
         */
        GroupAttribute build(BuilderBuildMode builderMode = BuildAndFlush)
        {
            if (_handle)
                return Attribute::CreateAndSteal(getSuite()->buildGroupBuilder(
                    _handle, (int32_t) builderMode));
            else
                return GroupAttribute(true);
        }

        ///@cond FN_INTERNAL_DEV

        FnGroupBuilderHandle getHandle() {
            // this function is used to hand over GroupBuilders through C APIs
            // so we need to make sure we do not return NULL
            lazyCreate();
            return _handle;
        }

        static const FnAttributeHostSuite_v2 *getSuite()
        {
            return _attrSuite;
        }

        static void setSuite(const FnAttributeHostSuite_v2 *suite);

        static FnPlugStatus setHost(FnPluginHost *host);


    private:
        // no copy/assign
        GroupBuilder(const GroupBuilder& rhs);
        GroupBuilder& operator=(const GroupBuilder& rhs);

        void lazyCreate() {
            if (_handle == 0x0)
                _handle = getSuite()->createGroupBuilder();
        }

        const static FnAttributeHostSuite_v2 *_attrSuite;

        FnGroupBuilderHandle _handle;

        //@endcond
    };

    /// @}
}
FNATTRIBUTE_NAMESPACE_EXIT

#endif // FoundryKatanaGroupBuilder_H
