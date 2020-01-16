#ifndef FnGeolibCookInterface_H
#define FnGeolibCookInterface_H

#include <map>
#include <set>
#include <string>
#include <vector>

#include <FnAttribute/FnAttribute.h>

#include <FnGeolib/FnGeolibAPI.h>
#include <FnGeolib/op/ns.h>
#include <FnGeolib/suite/FnGeolibCookInterfaceSuite.h>

#include <FnPlatform/internal/Portability.h>

#include <FnPluginSystem/FnPlugin.h>
#include <FnPluginSystem/FnPluginSystem.h>

//  This is the GeolibOp client side wrapper for FnGeolibInterfaceSuite_v1.  A
//  reference to an instance of this is handed to the GeolibOp::cook() function.
//
//  This class is not meant to be derived from, just compiled into the Op plug-
//  in as a convenience.


FNGEOLIBOP_NAMESPACE_ENTER
{
    /**
     * @ingroup FnGeolibOp
     * @brief Provides a unified interface for querying and manipulating the
     *     scene graph.
     * @details A reference to an instance of GeolibCookInterface is passed
     *     to your Op's \c cook() each time it is called by the
     *     Runtime.
     *
     *     Its methods can broadly be divided into two categories:
     *         - Methods which query the incoming, or upstream, scene graph.
     *           These methods provide \b read-only access to the incoming
     *           scene graph.
     *         - Methods which modify the output scene graph, observable after
     *           executing this Op. This involves creating new scene graph
     *           locations, modifying attributes of incoming scene graph
     *           locations, or entirely deleting locations.
     */
    class FNGEOLIB_API GeolibCookInterface
    {
    public:
        /**
         * @brief Thrown by the Runtime to abort the current \code cook() \endcode
         *     call.
         * @details This exception type will be thrown by methods on
         *     GeolibCookInterface and should never be caught by client code,
         *     <i>consequently, you should avoid the use of \code try{
         *     }catch(...){ } \endcode in your code</i>
         *
         * @cond FN_INTERNAL_DEV
         *
         * This should NOT derive from std::exception, as we prevent client code
         * inadvertently catching it. Therefore, to be as friendly as possible
         * make it a stand-alone type. (only catch(...) will get this one)
         *
         * @endcond
         */
        class QueryAbortException
        {
        };

        GeolibCookInterface(FnGeolibCookInterfaceHandle interfaceHandle,
            FnGeolibCookInterfaceSuite_v1 *interfaceSuite);

        /**
         *  @typedef ResetRoot
         *  @brief The values of ResetRoot control which root scene graph
         *   location will be used for Ops when run at child locations.
         *
         *      - ResetRootTrue the root location of the Op evaluated at the new
         *        location is reset to the new location path.
         *      - ResetRootAuto (the default) - the root location is reset only
         *        if opType is different to the Op calling createChild().
         *      - ResetRootFalse - the root location of the Op evaluated at the
         *        new location is inherited from the Op that called
         *        createChild().
         */
        typedef enum ResetRoot
        {
            ResetRootFalse = kFnKatGeolibCookInterfaceResetRootFalse,
            ResetRootTrue = kFnKatGeolibCookInterfaceResetRootTrue,
            ResetRootAuto = kFnKatGeolibCookInterfaceResetRootAuto
        } ResetRoot;

        /**
         * @brief Returns the leaf name for the output location.
         * @details For example:
         *
         *  \code
         *      "/root/world/geo" returns "geo"
         *   \endcode
         *
         *  <i>This is the successor to the getName() Geolib2.</i>
         *
         * @return The leaf name of the output location.
         */
        std::string getOutputName() const;

        /**
         * @brief Returns the leaf name for the input location.
         * @details This will typically match getOutputName(), but may differ
         *     when copyLocationToChild() is used.
         * @return The leaf name for the input location.
         * @see copyLocationToChild(const std::string,
         *                          const std::string,
         *                          int inputIndex,
         *                          const std::string)
         */
        std::string getInputName() const;

        /**
         * @brief Returns the output scene graph location path in its absolute
         *     form.
         * @details The output scene graph location path is the scene graph
         *     location that is currently being cooked. For example:
         *
         *  \code
         *      "/root/world/geo"
         *  \endcode
         *
         * @return The absolute output scene graph location path.
         */
        std::string getOutputLocationPath() const;

        /**
         * @brief Returns the scene graph location path corresponding to the
         *     input scene graph location that is currently being traversed.
         * @return The input scene graph location that is currently being
         *     traversed.
         */
        std::string getInputLocationPath() const;

        /**
         * @brief Returns the location path relative to the root execution
         *     location currently being traversed.
         *
         * @details For Ops instantiated at document level:
         *  - "/root" returns ""
         *  - "/root/world" returns "world"
         *  - "/root/world/geo" returns "world/geo"
         *
         * For an Op first executed at /root/world/geo (producing "a/b")
         *  - "/root/world/geo" returns ""
         *  - "/root/world/geo/a" returns "a"
         *  - "/root/world/geo/a/b" returns "a/b"
         *
         * @return The location path relative to the root execution location
         *     currently being traversed
         */
        std::string getRelativeOutputLocationPath() const;

        /**
         * @brief Returns the relative location path corresponding to the input
         *     scene graph location that is currently being traversed.
         * @return The relative location path corresponding to the input scene
         *     graph location that is currently being traversed.
         */
        std::string getRelativeInputLocationPath() const;

        /**
         * @brief Returns the scene graph location corresponding to the root of
         *     the Op's execution.
         * @details This may or may not be \c /root, specifically in the case
         *     where \c execOp() is called beneath \c /root.
         * @return The root of the Op's execution.
         * @see execOp(const std::string &, const FnAttribute::GroupAttribute &);
         */
        std::string getRootLocationPath() const;

        /**
         * @brief Returns true if we are at the topmost location this Op has
         *     been cooked at.
         * @details This is often, but not always \c /root. It is equivalent to
         *     the expression:
         *
         *     \code getRootLocationPath() == getOutputLocationPath() \endcode
         *
         * @return true if we are at the topmost location this Op has
         *     been cooked at, otherwise false.
         */
        bool atRoot() const;

        /**
         * @brief Returns the input index this scene graph location was created
         *     in.
         * @details In most cases getInputIndex() will return 0, this reflects
         *     the fact that most Ops operate on the default input or branch.
         *
         *     In specific cases, such as in a Merge Op, getInputIndex() may
         *     return an index > 0: Certain locations may be present only in
         *     additional input scenes, but not in the first (connected) input
         *     scene. Those locations are created in the output scene, and are
         *     then cooked by subsequent cook() calls of the Merge Op. For those
         *     locations, the Runtime keeps track of the index of the scene in
         *     which they were contained. That index is accessible using
         *     getInputIndex(). This allows the Merge Op to know which input
         *     scene a particular location came from, and is used for
         *     performance optimizations when certain circumstances apply.
         * @return The input index this location was created on.
         */
        int getInputIndex() const;

        /**
         * @brief Returns the number of input branches this Op has.
         * @details An Op can have the output from multiple other Ops as its
         *  input. Obvious use cases for this are instances where you wish to
         *  merge multiple scene graphs produced by different OpTrees into a
         *  single scene graph, comparing attribute values in two scene graph
         *  states, or copying one scene graph into another one. The
         *  getNumInputs() function allows you to determine how many Ops you
         *  have as inputs, which is a precursor to interrogating different
         *  branches of the OpTree for scene graph data.
         *
         * @return The number of inputs this Op has.
         */
        int getNumInputs() const;

        /**
         * @brief Returns the type of this Op.
         * @details For user supplied Ops this will be the string that was used
         *     to register the Op in the \c REGISTER_PLUGIN() macro.
         * @return A string containing the Op's type.
         */
        std::string getOpType() const;

        /**
         * @brief Returns the specified Op argument or all Op arguments if an
         *     empty string is provided
         * @details Op arguments are passed to the Op to configure how it should
         *     run. Op arguments are stored in a GroupAttribute. This method
         *     allows you to interrogate the Op arguments available to this Op.
         *
         *     If you supply an argument name this can be in the dot-delimited
         *     form i.e. \c a.b.c or the single name form.
         *
         * @param specificArgName The dot-delimited name of the Op argument.
         * @return An instance of Attribute containing the Op argument.
         */
        FnAttribute::Attribute getOpArg(
            const std::string &specificArgName=std::string()) const;

        /** @name Query Input
          * The following functions refer to queries on the connected
          * \b input scene graph. Client code is free to do scattered
          * queries of the incoming scene graph as required to express the
          * desired processing.
          *
          * @note Input scene graph locations which have not been cooked at the
          *     time of the query may throw an internal exception, aborting
          *     execution of the requesting Op. Client code should not catch
          *     these exceptions, they are a signal to the the runtime to
          *     recook the Op when the inputs are ready.
          *
          * If your Op makes multiple scattered queries in a single cook()
          * call, it is recommended that prefetch() is used.
          *
          * @{
          */

        /**
         * @brief Returns the specified attribute on the Op's input.
         * @details It is often necessary to perform some action or compute a
         *      value based on the result stored in another attribute. The
         *      getAttr() function allows you to interrogate any part of the
         *      incoming scene graph by providing the attribute name and a scene
         *      graph location path (either absolute or relative).
         *
         *      By default getAttr() will return the specified attribute on the
         *      Op's default input. In most cases where the Op only has one
         *      input branch this will suffice. If the Op has multiple inputs
         *      (e.g. the Merge node) the \c inputIndex parameter can be used to
         *      specify a particular branch. When specified, \c inputIndex must
         *      satisfy 0 <= inputIndex < getNumInputs().
         *
         * @note It is important to remember that getAttr() will only
         *     interrogate the Op's input scene graph. Calls made to setAttr()
         *     will not be visible to getAttr(). If you need to see what
         *     attributes have been set during an Op's execution you should call
         *     getOutputAttr().
         * @param attrName The name of the attribute.
         * @param inputLocationPath The input location path the attribute should
         *     be retrieved from.
         * @param inputIndex The input index the attribute should be
         *     retrieved from.
         * @return The attribute at the specified location path and input index.
         * @see getNumInputs(), getOutputAttr()
         */
        FnAttribute::Attribute getAttr(
            const std::string & attrName,
            const std::string & inputLocationPath=std::string(),
            int inputIndex=kFnKatGeolibDefaultInput) const;

        /**
         * @brief Returns true if the specified location exists on the Op's input.
         * @param inputLocationPath The scene graph location path.
         * @param inputIndex The input index that should be searched.
         *
         * @return \c true if the location exists at the specified location and
         *     index otherwise false.
         */
        bool doesLocationExist(
            const std::string & inputLocationPath=std::string(),
            int inputIndex=kFnKatGeolibDefaultInput) const;

        /**
         * @brief Returns a StringAttribute containing list of names of
         *  potential children on input.
         * @details The ability for Ops, via deleteSelf(), to delete themselves
         *     gives rise to a subtle behavior. When an upstream Op is evaluated
         *     and creates children, if downstream Ops have the ability to
         *     delete them, the upstream Op can only go so far as to state that
         *     the children it creates may potentially exist after a downstream
         *     Op has been evaluated at those child locations. This is because
         *     the Op has no knowledge of what a downstream Op may do when
         *     evaluated at such a location. To that extent,
         *     getPotentialChildren() returns a list of all the children of a
         *     given location on the input of an Op.
         *
         * @param inputLocationPath The input location path to obtain the
         *     potential children for.
         * @param inputIndex The input index to be used.
         *
         * @return A StringAttribute containing the potential children at the
         *     specified input location.
         */
        FnAttribute::StringAttribute getPotentialChildren(
            const std::string & inputLocationPath=std::string(),
            int inputIndex=kFnKatGeolibDefaultInput) const;

        /**
         * @brief Returns a raw pointer to the private data stored in this Op or
         *     \c NULL.
         * @return Raw void pointer or NULL if no private data has been set.
         */
        void *getPrivateData() const;

        /**
         * @brief Indicates to the Runtime that the caller depends on the
         * specified location.
         *
         * @details Calling the \c prefetch() function within an Op’s \c cook()
         * function instructs the Runtime that the specified location is
         * required for the Op's processing task.
         *
         * The \c prefetch() function will return immediately, scheduling the
         * computation of the requested location to be cooked concurrently or
         * at some point in the future.
         *
         * \c prefetch() is recommended if your Op makes multiple scattered
         * queries during a cook. Note that it is not necessary to prefetch
         * parent locations of your default input.
         *
         * The use of a prefetch creates a data flow dependency, so do not
         * prefetch locations or inputs inputs you do not need.
         *
         * @param inputLocationPath The scene graph location path to be
         *     prefetched.
         * @param inputIndex The input index from which to request the specified
         *     \c inputLocationPath. In common cases the default input will
         *     suffice, however if your Op has multiple input branches (like a
         *     Merge Op) you will need to specify a particular input index.
         */
        void prefetch(
            const std::string & inputLocationPath=std::string(),
            int inputIndex=kFnKatGeolibDefaultInput) const;

        /**
         * @brief Returns the specified attribute (if set) on the Op's output
         *     for the current location.
         * @details This is useful for inspecting the output attributes set
         *     during an execOp() that may potentially require further
         *     processing after the exec'd Op has returned.
         *
         *     \note This is not recommended for normal usage.
         *
         * @param attrName The name of the attribute to query.
         * @return The attribute set on the Op's output. If not set an invalid
         *     attribute will be returned.
         */
        FnAttribute::Attribute getOutputAttr(const std::string & attrName) const;

        /**
         * @}
         */

        /** @name Modify Output
          * @{
          */

        /**
         * @brief Sets the specified attribute on the Op's output.
         * @details \c attrName can be specified in dot-delimited form e.g.
         *     \c attr1.attr2.attr3, in which case a new GroupAttribute is
         *     created for each level required and the value of \c groupInherit
         *     will determine the GroupAttribute's inheritance state.
         *
         * @note Calls to setAttr() made in the current Op will not be reflected
         *  in subsequent calls to getAttr(). This is because functions which
         *  mutate the scene graph only affect the output of this Op. Methods
         *  such as getAttr() and doesLocationExist() query the input to this
         *  Op. This is also true for non-local queries i.e., when an Op queries
         *  attributes of its parent. In this case it will be querying the input
         *  at its parent, and thus does see the results of its own processing.
         *  If you need to see the attributes set during a cook call you should
         *  use getOutputAttr().
         *
         * @param attrName The name of the attribute to create. This can be
         *     specified in dot-delimited form.
         * @param value The FnAttribute instance to be associated with
         *     \c attrName.
         * @param groupInherit The group inheritance state for newly created
         *     GroupAttributes.
         * @see getOutputAttr()
         */
        void setAttr(const std::string & attrName,
            const FnAttribute::Attribute & value,
            const bool groupInherit=true);

        /**
         * @brief Copies the specified source attribute to the specified
         *     destination attribute.
         * @details The specified source attribute (determined by \c
         *     inputLocationPath and \c inputIndex) will be copied to the output
         *     of this Op as \c dstAttrName. If the source attribute or location
         *     does not exist this call is equivalent to calling deleteAttr()
         *     with the destination attribute.
         *
         * @param dstAttrName The name of the attribute to copy the attribute at
         *     \c srcAttrName to.
         * @param srcAttrName The name of the attribute to copy to \c
         *     dstAttrName.
         * @param groupInherit The group inheritance state of the destination
         *     attribute.
         * @param inputLocationPath The scene graph location path on this Op's
         *     input to copy from.
         * @param inputIndex The input index to copy from (where multiple
         *     branches in the Op tree exist).
         */
        void copyAttr(const std::string & dstAttrName,
            const std::string & srcAttrName,
            const bool groupInherit=true,
            const std::string & inputLocationPath=std::string(),
            int inputIndex=kFnKatGeolibDefaultInput);

        /**
         * @brief Extends the existing input attribute with additional values if
         *     the types match.
         * @details If no input attribute exists with the specified name then
         *     this call is equivalent to setAttr().
         *
         *     Multi-sampling is supported; the new attribute is constructed as
         *     the union of all incoming and new time samples, leveraging the
         *     FnAttribute's existing interpolation.
         *
         *     If there is a type mismatch, it will be reported as a scene graph
         *     error. If more sophisticated or fine-grained type mismatch
         *     handling is required it is recommended to instead manually call
         *     getAttr() / setAttr()
         *
         *     Incoming NullAttribute's are equivalent to the attribute not
         *     being set in the incoming scene.
         *
         * @param dstAttrName The name of the attribute to be extended.
         * @param value The Attributes that will extend the specified attribute.
         * @param srcAttrName The name of the attribute a specified location to
         *     be extended, if empty string is used this is the equivalent of
         *     setAttr().
         * @param groupInherit The value of the group inheritance flag.
         * @param inputLocationPath The input location path from which \c
         *     srcAttrName should be sourced from.
         * @param inputIndex The input index to search for \c inputLocationPath
         *     on.
         * @see setAttr(), getAttr()
         */
        void extendAttr(const std::string & dstAttrName,
            const FnAttribute::Attribute & value,
            const std::string & srcAttrName=std::string(),
            const bool groupInherit=true,
            const std::string & inputLocationPath=std::string(),
            int inputIndex=kFnKatGeolibDefaultInput);

        /**
         * @brief Deletes the attribute specified by \c attrName so it will no
         *  longer on this Ops output.
         *
         * @param attrName The name of the attribute to delete from the Op's
         *     output.
         */
        void deleteAttr(const std::string & attrName);

        /**
         * @brief Deletes all attributes on this Op's output.
         */
        void deleteAttrs();

        /**
         * @brief Replaces the attributes on this Op's output with those
         *     specified by the input scene graph location path.
         *
         * @param inputLocationPath The attributes at inputLocationPath will
         *     replace those on this Op's output.
         * @param inputIndex The input index to use where multiple input
         *     branches exist for this Op.
         */
        void replaceAttrs(
            const std::string & inputLocationPath=std::string(),
            int inputIndex=kFnKatGeolibDefaultInput);
        /**
         * @}
         */

        /**
          * @name Change the Output Topology
          * @{
          */

        /**
         * @brief Creates a new child at the specified location if one didn't
         *     already exist.
         * @details If the child already exists in the incoming scene then the
         *     Op will be run and previously created attribute values and
         *     children will be preserved. To ensure the child will not have any
         *     pre-existing attributes or children it is recommended to first
         *     call deleteChild(), doing so will maintain the original child
         *     index ordering.
         *
         *     If the Op type is not specified or is an empty string the calling
         *     Op's type will be inherited by the child.
         *
         *     To modify the result of getRelativeOutputLocationPath() in calls
         *     beneath this location the \c resetRoot argument can be set. If
         *     ResetRootAuto is specified then the child's root location will be
         *     reset if and only if the Op's type differs from the parent's.
         *
         * @note Multiple calls to createChild() for the same named child
         *  location causes the last specified \c opType to be used, that is to
         *  say, successive calls to createChild() mask prior calls.
         *
         * @param name The name of the child to create.
         * @param opType The type of the Op that will be evaluated at the child
         *     location. If an empty string or nothing is specified then the
         *     current Op's type will be used.
         * @param args The Op arguments that will be passed to the Op when run
         *     at the child location.
         * @param resetRoot Specifies what the root location for Ops run at
         *     child location should be.
         * @param privateData A pointer to user data that can be used inside the
         *     Op.
         * @param deletePrivateData A function pointer of the form <tt> void
         *  (*deletePrivateData)(void* data) </tt> that will be called to delete
         *  the \c privateData.
         *
         * @see ResetRoot
         */
        void createChild(const std::string & name,
            const std::string & opType="",
            const FnAttribute::Attribute & args=FnAttribute::Attribute(),
            ResetRoot resetRoot=ResetRootAuto,
            void *privateData=0x0,
            void (*deletePrivateData)(void *data)=0x0);

        /**
         * @brief Replaces the children under the Op's current scene graph
         *     location with children from an alternate input location and/or
         *     input index.
         *
         * @param inputLocationPath The input scene graph location whose
         *     children will replace this location's children.
         * @param inputIndex The input index on which the where the input
         *     location path should be retrieved.
         */
        void replaceChildren(
            const std::string & inputLocationPath=std::string(),
            int inputIndex=kFnKatGeolibDefaultInput);

        /**
         * @brief Deletes all children on this Op's output, both newly created
         *     by the current Op and any incoming children.
         */
        void deleteChildren();

        /**
         * @brief Deletes the specified child location on the Op's output.
         * @note Calling deleteChild() is more efficient than allowing the child
         *     location to delete itself. Where possible you should prefer
         *     deleteChild() over deleteSelf().
         * @param name The name of the child location to delete.
         */
        void deleteChild(const std::string & name);

        /**
         * @brief Deletes the current output location.
         * @details It is common to return from cook() immediately following
         *     this call.
         *
         * @note Calls to deleteSelf() will not remove the location from the
         *     location's parent \c getPotentialChildren() list. Thus, if there
         *     is a way to structure code to use \c deleteChild() instead, it
         *     is generally preferable.
         */
        void deleteSelf();

        /**
         * @brief Copies the specified location in the input scene, to the
         *     specified child location.
         * @details The specified \c inputLocationPath will not be evaluated
         *     until the new child location is traversed. At this time, if the
         *     input does not exist, neither will the new child.
         *
         *     \c copyLocationToChild() can be used to rename a child location:
         *     \code
         *         rename(dst, src)
         *         {
         *           copyLocationToChild(dst, src, kFnKatGeolibDefaultInput, src);
         *           deleteChild(src);
         *         }
         *     \endcode
         *
         *     The \c inputLocationPath always refers to the input name, and
         *     \c child refers to output name, so multiple
         *     \c copyLocationToChild() calls are unambiguous.
         *
         * @param child The name of the child to be copied to the \b output of
         *     this Op.
         * @param inputLocationPath The input scene graph location path to copy.
         * @param inputIndex The input index to copy the \c inputLocationPath
         *     from.
         * @param orderBefore The child that this should be precede in the scene
         *     graph ordering.
         */
        void copyLocationToChild(const std::string & child,
            const std::string & inputLocationPath=std::string(),
            int inputIndex=kFnKatGeolibDefaultInput,
            const std::string & orderBefore=std::string());

        /**
         * @brief Replaces the Op type and Op arguments for child locations of
         *     this Op.
         * @details Newly created child locations (created with createChild())
         *     only have access to the Op arguments / type and  private data
         *     they were created with originally. Conceptually, this is the type
         *     and Op arguments used when traversing locations in the incoming
         *     scene.
         *
         * @param opType The Op type that will be evaluated at child locations.
         * @param args The arguments that will be passed to the Op when run at
         *     the child locations.
         * @param privateData A pointer to user data that can be used inside the
         *     Op.
         * @param deletePrivateData A function pointer of the form <tt> void
         *  (*deletePrivateData)(void* data) </tt> that will be called to delete
         *  the \c privateData.
         */
        void replaceChildTraversalOp(const std::string & opType,
                                     const FnAttribute::GroupAttribute & args,
                                     void *privateData=0x0,
                                     void (*deletePrivateData)(void *data)=0x0);

        /**
         * @brief Stops this Op from running at descendants of the current
         * location.
         */
        void stopChildTraversal();

        /**
         * @brief Runs the specified Op with the provided arguments at the
         *     current location.
         * @details Children created as a result of the execOp() call will
         *     behave as expected in such that they will retain the type and Op
         *     arguments of the exec'd Op.
         *
         * @param opType The type of Op to be run.
         * @param args A GroupAttribute describing the arguments that will be
         *     passed to the Op when run.
         */
        void execOp(const std::string & opType, const FnAttribute::GroupAttribute & args);

        /**
         * @brief Marks this location as the new root location in the Op's
         *     traversal.
         * @details Subsequent calls to getRelativeOutputLocationPath() at child
         *     locations will return paths relative to this one.
         * @note Calls to getRelativeOutputLocationPath() at the current
         *     location will remain unaffected by this function.
         */
        void resetRoot();

        /**
         * @}
         */

        // Need ability to pass this interface instance to other C APIs.
        FnGeolibCookInterfaceHandle getHandle() const {return _handle;}
        FnGeolibCookInterfaceSuite_v1 * getSuite() const {return _suite;}

    private:
        FnGeolibCookInterfaceHandle _handle;
        FnGeolibCookInterfaceSuite_v1 *_suite;
    };

    /**
     * @ingroup FnGeolibOp
     * @brief Convenience base class that can be derived from to allow users who
     *     require private data in their Ops.
     */
    class FNGEOLIB_API GeolibPrivateData
    {
    public:
        GeolibPrivateData() {}
        virtual ~GeolibPrivateData() {}
        static void Delete(void *data) {delete (GeolibPrivateData*)data;}
    };

    /**
     * @ingroup FnGeolibOp
     * @brief Returns the location type for specified location on the
     *     corresponding GeolibCookInterfaces's input.
     *
     * @param interface A reference to a valid GeolibCookInterface object.
     * @param location The scene graph location path.
     * @param inputIndex The input index on which to retrieve the specified
     *     scene graph location path.
     * @return The location's \c 'type' attribute. If type attribute is invalid
     *     an empty string will be returned, if it is not set then \c group
     *     will be returned by default.
     */
    FNGEOLIB_API
    std::string GetInputLocationType(
        const GeolibCookInterface & interface,
        const std::string & location=std::string(),
        int inputIndex=kFnKatGeolibDefaultInput);

    /**
     * @ingroup FnGeolibOp
     * @brief Resolves a relative \b output scene graph location path to an
     *     absolute path.
     * @details This method cannot be used to query input locations (such as
     *     those used with methods such as GeolibCookInterface::getAttr()) but
     *     those methods that operate on the Op's output such as
     *     GeolibCookInterface::createChild(). If you require the absolute scene
     *     graph location path for an input location you should use
     *     GetAbsInputLocationPath().
     *
     * @param interface A reference to a valid GeolibCookInterface object.
     * @param outputLocationPath The relative output scene graph location path.
     * @return The corresponding absolute location path.
     * @see GetAbsInputLocationPath()
     */
    FNGEOLIB_API
    std::string GetAbsOutputLocationPath(const GeolibCookInterface & interface,
                                  const std::string & outputLocationPath);

    /**
     * @ingroup FnGeolibOp
     * @brief Resolves a relative \b input scene graph location path to an
     *     absolute path.
     * @details This method cannot be used to query output locations (such as
     *     those used with methods such as GeolibCookInterface::createChild())
     *     but those methods that operate on the Op's output such as
     *     GeolibCookInterface::getAttr(). If you require the absolute scene
     *     graph location path for an output location you should use
     *     GetAbsOutputLocationPath().
     *
     * @param interface  A reference to a valid GeolibCookInterface object.
     * @param inputLocationPath The relative input scene graph location path.
     *
     * @return The corresponding absolute location path.
     * @see GetAbsOutputLocationPath()
     */
    FNGEOLIB_API
    std::string GetAbsInputLocationPath(const GeolibCookInterface & interface,
                                 const std::string & inputLocationPath);

    /**
     * @ingroup FnGeolibOp
     * @brief Renames the specified child.
     *
     * @param interface A reference to a valid GeolibCookInterface object.
     * @param src The name of the child to rename.
     * @param dst The new name of the child.
     */
    FNGEOLIB_API
    void RenameChild(GeolibCookInterface &interface,
        const std::string & src, const std::string & dst);

    /**
     * @ingroup FnGeolibOp
     * @brief Returns attribute from the input scene, including inherited
     *     attributes.
     *
     * @param interface A reference to a valid GeolibCookInterface object.
     * @param name The name of the attribute.
     * @param location The location at which the attribute should be found.
     * @param inputIndex The input index of which to search for the input scene
     *     graph location.
     * @return The specified attribute at the given location including inherited
     *     attributes.
     */
    FNGEOLIB_API
    FnAttribute::Attribute GetGlobalAttr(
        const GeolibCookInterface & interface,
        const std::string & name,
        const std::string & location=std::string(),
        int inputIndex=kFnKatGeolibDefaultInput);

    /**
     * @ingroup FnGeolibOp
     * @brief Reports the specified error message to the scene graph at the
     *     current scene graph location.
     *
     * @param interface A reference to a valid GeolibCookInterface object.
     * @param message The error message.
     */
    FNGEOLIB_API
    void ReportError(GeolibCookInterface & interface,
                     const std::string & message);

    /**
     * @ingroup FnGeolibOp
     * @brief Reports the specified warning message to the scene graph at the
     *     current scene graph location.
     *
     * @param interface A reference to a valid GeolibCookInterface object.
     * @param message The warning message.
     */
    FNGEOLIB_API
    void ReportWarning(GeolibCookInterface & interface,
                       const std::string & message);

    /**
     * @ingroup FnGeolibOp
     * @brief Merges the unique children of the specified GroupAttribute.
     * @details The left-most definition for a child attribute "wins", and the
     *     left-most attribute ordering is preserved.
     * @note This matches the combination logic used in Merge for \c lightList,
     *     \c renderSettings.outputs, \c relativeScopedCoordinateSystems, \c
     *     collections, etc.
     *
     * @param attrs A vector of GroupAttribute to be merged.
     * @return A GroupAttribute containing the merged GroupAttributes.
     */
    FNGEOLIB_API
    FnAttribute::GroupAttribute MergeImmediateGroupChildren(
        const std::vector<FnAttribute::GroupAttribute> & attrs);

    /**
     * @ingroup FnGeolibOp
     * @brief Returns the global transform as a GroupAttribute, where the
     *     immediate children represent all the entries from \c /root to the
     *     leaf which have an \c 'xform' attribute present
     * @note If a collapsed 4x4 transform matrix is required pass the result
     *     of this function to the \c
     *     FnGeolibServices::FnXFormUtil::CalcTransformMatrix..() functions.
     * @param interface A reference to a valid GeolibCookInterface object.
     * @param inputLocationPath The input scene graph location to retrieve the
     *     global transform from.
     * @param inputIndex The input index from which to retrieve the scene graph
     *     location.
     * @return The global transform at the specified location.
     */
    FNGEOLIB_API
    FnAttribute::GroupAttribute GetGlobalXFormGroup(
        const GeolibCookInterface & interface,
        const std::string & inputLocationPath=std::string(),
        int inputIndex=kFnKatGeolibDefaultInput);

    /**
     * @ingroup FnGeolibOp
     * @brief Returns the current time from the current Graph State.
     * @param interface A reference to a valid GeolibCookInterface object.
     * @return The current time.
     */
    FNGEOLIB_API
    float GetCurrentTime(const GeolibCookInterface & interface);

    /**
     * @ingroup FnGeolibOp
     * @brief Returns the shutter open time from the current Graph State.
     * @param interface A reference to a valid GeolibCookInterface object.
     * @return The shutter open time.
     */
    FNGEOLIB_API
    float GetShutterOpen(const GeolibCookInterface & interface);

    /**
     * @ingroup FnGeolibOp
     * @brief Returns the shutter close time from the current Graph State.
     * @param interface A reference to a valid GeolibCookInterface object.
     * @return The shutter close time.
     */
    FNGEOLIB_API
    float GetShutterClose(const GeolibCookInterface & interface);

    /**
     * @ingroup FnGeolibOp
     * @brief Returns the number of time samples from the current Graph State.
     * @param interface A reference to a valid GeolibCookInterface object.
     * @return The number of time samples.
     */
    FNGEOLIB_API
    int GetNumSamples(const GeolibCookInterface & interface);

    /**
     * @ingroup FnGeolibOp
     * @brief Returns the specified variable from the current Graph State.
     * @param interface A reference to a valid GeolibCookInterface object.
     * @param variableName The name of the graph state variable.
     * @return The Graph State Variable specified by \c variableName.
     */
    FNGEOLIB_API
    FnAttribute::Attribute GetGraphStateVariable(
        const GeolibCookInterface & interface,
        const std::string & variableName);

    ////////////////////////////////////////////////////////////////////////////
    //
    // WARNING - these are likely to be moved to an alternate library / header
    //
    ////////////////////////////////////////////////////////////////////////////

    /**
     * @ingroup FnGeolibOp
     * @brief Utility structure that can be used to represent a point in 3D
     *     space. Multiple points can be used to describe a bounding box.
     */
    struct BoundPoint
    {
        BoundPoint() : x(0), y(0), z(0) {}
        BoundPoint(double x_, double y_, double z_)
            : x(x_), y(y_), z(z_) {}

        /**
         * @brief x coordinate.
         */
        double x;

        /**
         * @brief y coordinate.
         */
        double y;

        /**
         * @brief z coordinate.
         */
        double z;
    };

    /**
     * @ingroup FnGeolibOp
     * @brief Returns the bound attribute at the specified input scene graph
     *     location.
     * @param interface A reference to a valid GeolibCookInterface object.
     * @param inputLocationPath The scene graph location to obtain the bound
     *     attribute from.
     * @param inputIndex The input index on which to find the scene graph
     *     location.
     * @return A 6-element DoubleAttribute containing the bound attribute or an
     *     invalid DoubleAttribute if bounds do not exist at the specified
     *     location.
     */
    FNGEOLIB_API
    FnAttribute::DoubleAttribute GetBoundAttr(
        const GeolibCookInterface & interface,
        const std::string & inputLocationPath=std::string(),
        int inputIndex=kFnKatGeolibDefaultInput);

    /**
     * @ingroup FnGeolibOp
     * @brief Fills the array's \c outMinBound and \c outMaxBound with the
     *     values stored in the DoubleAttribute \c boundAttr.
     * @param outMinBound A 3-element array that will contain x-min, y-min,
     *     z-min.
     * @param outMaxBound A 3-element array that will contain x-max, y-max,
     *     z-max.
     * @param boundAttr The bounds attribute to process.
     * @param sampleTime The sample time that the bounds attribute \c boundAttr
     *     should be sampled at.
     */
    FNGEOLIB_API
    void GetBoundAttrValue(
        double outMinBound[3], double outMaxBound[3],
        const FnAttribute::DoubleAttribute & boundAttr,
        float sampleTime);

    /**
     * @ingroup FnGeolibOp
     * @brief Returns the bounding box specified by the bound attribute \c
     *     boundAttr after being transformed by the 4x4 transform matrix \c
     *     xform.
     * @param [out] outPoints An empty vector that will filled with the 8
     *     points of the transformed bounding box.
     * @param boundAttr A 6-element DoubleAttribute specifying the bounding box
     *     to be transformed.
     * @param sampleTime The sample time that the bounds attribute \c boundAttr
     *     should be sampled at.
     * @param xform A 4x4 transform matrix by which \c boundAttr will be
     *     transformed.
     * @see BoundPoint
     */
    FNGEOLIB_API
    void GetTransformedBoundAttrValue(
        std::vector<BoundPoint> & outPoints,
        const FnAttribute::DoubleAttribute & boundAttr,
        float sampleTime, const double xform[16]);

    /**
     * @ingroup FnGeolibOp
     * @brief Returns the axis-aligned bounding box specified by the bound
     *     attribute \c boundAttr after being transformed by the 4 x 4
     *     transform matrix \c xform.
     * @param [out] outMinBound A 3-element array that will contain the
     *     transformed x-min, y-min, z-min.
     * @param [out] outMaxBound A 3-element array that will contain the
     *     transformed x-max, y-max, z-max.
     * @param boundAttr A 6-element DoubleAttribute specifying the bounding box
     *     to be transformed.
     * @param sampleTime The sample time that the bounds attribute \c boundAttr
     *     should be sampled at.
     * @param xform A 4x4 transform matrix by which \c boundAttr will be
     *     transformed.
     */
    FNGEOLIB_API
    void GetTransformedBoundAttrValue(
        double outMinBound[3], double outMaxBound[3],
        const FnAttribute::DoubleAttribute & boundAttr,
        float sampleTime, const double xform[16]);

    /**
     * @ingroup FnGeolibOp
     * @brief  Returns a single bound attribute, given a vector of bounds in the
     *     same local coordinate system.
     * @details If any of the bounds are multi-sampled the result will promoted
     *     to a multi-sampled attribute. The resulting time samples will be the
     *     union of the supplied sample times. Missing values will be
     *     synthesized using either DoubleAttribute::fillInterpSample(). This is
     *     the same combination logic used by Merge Op's \c sumBounds option.
     * @param boundAttrs A vector of 6-element bound attributes in the same
     *     local coordinate system to be merged.
     * @return The merged bound attributes.
     */
    FNGEOLIB_API
    FnAttribute::DoubleAttribute MergeLocalBoundAttrs(
        const std::vector<FnAttribute::DoubleAttribute> & boundAttrs);

    /**
     * @ingroup FnGeolibOp
     * @brief Returns a newly constructed GroupAttribute based on \c groupAttr
     *     with all NullAttribute removed.
     * @param groupAttr The GroupAttribute to filter.
     * @return A new instance of type GroupAttribute which contains all values
     *     in \c groupAttr except NullAttributes.
     */
    FNGEOLIB_API
    FnAttribute::GroupAttribute FilterNullAttributes(
        const FnAttribute::GroupAttribute & groupAttr);

    /**
     * @ingroup FnGeolibOp
     * @brief Returns a GroupAttribute which contains the attributes stored at
     *     the location contained in the specified attribute \c "keyAttrName".
     * @note Currently \c attrName is ignored.
     * @param interface  A reference to a valid GeolibCookInterface object.
     * @param keyAttrName The attribute name on the input scene graph location
     *     that contains the reference location(s).
     * @param attrName The attribute(s) that will be merged with those at the
     *     referenced location. If an empty string is specified then all
     *     attributes will be used.
     * @param inputLocationPath The input scene graph location path at which \c
     *     attrName will be found.
     * @param inputIndex The input index on which \c inputLocationPath can be
     *     found.
     * @return A GroupAttribute which is a combination of those attributes at
     *     the location specified in \c keyAttrName and those attributes stored
     *     under the attribute specified at \c attrName.
     */
    FNGEOLIB_API
    FnAttribute::GroupAttribute ResolveReferentialInheritance(
        const FnGeolibOp::GeolibCookInterface & interface,
        const std::string & keyAttrName,
        const std::string & attrName,
        const std::string & inputLocationPath=std::string(),
        int inputIndex=kFnKatGeolibDefaultInput);

    ////////////////////////////////////////////////////////////////////////////
    //
    // DEPRECATED - Do not use! Pending removal
    //
    ////////////////////////////////////////////////////////////////////////////

    struct CreateLocationInfo
    {
        bool atLeaf;
        bool canMatchChildren;
    };

    FNGEOLIB_API
    void CreateLocation(CreateLocationInfo & createLocationInfo,
                        GeolibCookInterface & interface,
                        const std::string & location);

    FNGEOLIB_API
    void CopyTree(
        GeolibCookInterface & interface,
        const std::string & dstLocation,
        const std::string & srcLocation,
        int srcInput=kFnKatGeolibDefaultInput);
}
FNGEOLIBOP_NAMESPACE_EXIT

#endif // FnGeolibCookInterface_H
