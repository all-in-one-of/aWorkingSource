// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef FnScenegraphIterator_H
#define FnScenegraphIterator_H

#include <string>

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnAttribute/FnAttribute.h>
#include <FnScenegraphIterator/suite/FnScenegraphIteratorSuite.h>

namespace Foundry
{

namespace Katana
{

/**
 * \defgroup SI Scenegraph Iterator (SI) API
 * @{
 *
 *@brief API that allows to iterate through the locations and attributes on a Scenegraph.
 *
 * This API allows to iterate through the locations on a scenegraph and query
 * their attributes. This is an auxiliary API that can be used when implementing,
 * for example, a connection of Katana to a Renderer. In this specific case the
 * renderer will have at some point to discover which locations and attributes
 * exist on the Katana scene being rendered.
 *
 * Given the lazy initialization nature of Katana the scenegraph locations and
 * attributes will be procedurally generated under the hood as the iterators
 * go through the scene. Whenever a FnScenegraphIterator is instantiated Katana
 * will generate its corresponding location if needed on that moment.
 *
 */




/**
 * @brief The Scenegraph Iterator that 'points' at a location in a scenegraph.
 *
 * Each instance of this class will be conceptually pointing at a specific
 * location of the scenegraph. It presents methods that return other iterators
 * that point at child, sibling, parent and other arbitraty locations on the
 * scenegraph. The location pointed by the instance is referred here as "current
 * location".
 */
class FnScenegraphIterator
{
public:
    FnScenegraphIterator(FnSgIteratorHandle handle);
    FnScenegraphIterator();

    ~FnScenegraphIterator();

    FnScenegraphIterator(const FnScenegraphIterator& rhs);
    FnScenegraphIterator& operator=(const FnScenegraphIterator& rhs);

    /**
     * @brief Returns true if this Scenegraph Iterator is valid.
     */
    bool isValid() const {return _handle != 0x0;}

    /**
     * @brief Returns the name of the current Location. (ex: "pony" in "/root/world/geo/pony")
     */
    std::string getName() const;

    /**
     * @brief Returns the full path name of the current Location. (ex: "/root/world/geo/pony")
     */
    std::string getFullName() const;

    /**
     * @brief Returns the type of the current Location. (ex: "subdmesh" or "group")
     */
    std::string getType() const;

    /**
     * @brief Returns an iterator that poins at the first child of the current location.
     */
    FnScenegraphIterator getFirstChild(bool evict=false) const;


    /**
     * @brief Returns an iterator that poins at the next sibling of the current location.
     */
    FnScenegraphIterator getNextSibling(bool evict=false) const;

    /**
     * @brief Returns an iterator that poins at the parent of the current location.
     */
    FnScenegraphIterator getParent() const;

    /**
     * @brief Returns an iterator that poins at the root location of the scenegraph where
     * the current locations lives.
     */
    FnScenegraphIterator getRoot() const;

    /**
     * @return a list of immediate potential child names
     */

    FnAttribute::StringAttribute getPotentialChildren() const;

    /**
     * @brief Returns an iterator which is child of the current one
     *
     * Returns an iterator that poins at the a child location of the current one
     * with the specified name.
     *
     * @param name: the name of the child location
     */
    FnScenegraphIterator getChildByName(const std::string &name, bool evict=false) const;

    /**
     * @brief Returns an iterator pointint at a specified location.
     *
     * Returns an iterator that poins at a location on the scenegraph where the
     * current location lives specified by the forward slash-separated
     * scenegraph path.
     *
     * @param name: the forward slash-separated path of the location
     */
    FnScenegraphIterator getByPath(const std::string &path, bool evict=false) const;


    /**
     * @brief Get a specific attribute from the location pointed by the iterator.
     *
     * Returns an existing Attribute from the current Location if no location is
     * specified. If there isn't any Attribute with the given name on the given
     * Location then an invalid FnAttribute is returned (which can be checked
     * with the function FnAttribute::isValid()).
     *
     * @param name The name of the attribute - can be a child of a GroupAttribute
     *              (ex: "geometry.point.P")
     * @param global If false then the global attributes (inherited from parent
     *               Locations) will be ignored. In this case if the requested
     *               Attribute is global, then an invalid attribute is returned.
     *
     */
    FnAttribute::Attribute getAttribute(const std::string &name, bool global=false) const;

    /**
     * @brief Return the names of all the attributes on the location pointed by
     * the iterator.
     *
     * @return A valid StringAttribute
     */
    FnAttribute::StringAttribute getAttributeNames() const;

    /**
     * @return a GroupAttribute that represents the global transform for this
     *     location.
     */
    FnAttribute::GroupAttribute getGlobalXFormGroup() const;

    ///@cond FN_INTERNAL_DEV

    static FnPlugStatus setHost(FnPluginHost *host);

    static FnSgIteratorHostSuite_v2 *_suite;

    FnSgIteratorHandle getHandle() const
    {
        return _handle;
    }

    static FnScenegraphIterator getIteratorFromFile(
            const std::string & opTreeFileName);

protected:
    void acceptHandle(const FnScenegraphIterator &rhs);

private:

    FnSgIteratorHandle _handle;

    ///@endcond
};



/// @}

} // namespace Katana

} // namespace Foundry

namespace FnKat = Foundry::Katana;

#endif // FnScenegraphIteratorSuite_H
