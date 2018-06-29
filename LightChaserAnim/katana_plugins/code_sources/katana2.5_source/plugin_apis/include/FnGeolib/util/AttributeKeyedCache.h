#ifndef INCLUDED_FNGEOLIBUTIL_ATTRIBUTEKEYEDCACHE_H
#define INCLUDED_FNGEOLIBUTIL_ATTRIBUTEKEYEDCACHE_H

#include "ns.h"

#include <cstdlib>
#include <map>
#include <set>
#include <string>

#include <FnAttribute/FnAttribute.h>
#include <FnPlatform/internal/Mutex.h>
#include <FnPlatform/internal/SharedPtr.h>

FNGEOLIBUTIL_NAMESPACE_ENTER
{
    /**
     * \ingroup Geolib
     * @{
     *
     * @brief The AttributeKeyedCache is a templated class which takes an
     *        Attribute as a key and maps it to an instance of the templated
     *        type.
     *
     *  The cache accepts any Attribute to be used as the key,
     *  internally the cache relies upon the attribute's hash. The use of
     *  the hash as a key allows for efficient storage even with very large
     *  keys such as large CEL statements.
     *
     *  In order to make use of the cache the pure virtual
     *  function createValue() must be implemented. This function will be
     *  called by the cache in response to client requests
     *  for a particular value via getValue(attr). createValue() must return
     *  a valid object instance or a null pointer.
     *
     *  The cache is actually composed of two internal caches.
     *  The main cache contains mappings for all valid key-value mappings.
     *  The 'invalid' cache contains mappings of all invalid hash values.
     *  That is, all attribute hash values that return NULL when passed to your
     *  createValue() implementation. In the following example, all Attributes
     *  whose hash is even will be placed in the main cache, all odd hash
     *  values will be stored in the invalid cache.
     *
     *  \code
     *  class EvenIntegerCache: public FnGeolibUtil::AttributeKeyedCache<int64_t>
     *  {
     *    public:
     *      EvenIntegerCache(std::size_t maxNumEntries,
     *                       std::size_t maxNumInvalidKeys)
     *        : FnGeolibUtil::AttributeKeyedCache<int64_t>(maxNumEntries,
     *                                                     maxNumInvalidKeys)
     *      {}
     *
     *    private:
     *      EvenIntegerCache::IMPLPtr
     *        createValue(const FnAttribute::Attribute &attr)
     *      {
     *        // Only return valid value if the attribute's hash is even.
     *        int64_t hash = attr.getHash().uint64();
     *        if( hash & 0x1 )
     *          return EvenIntegerCache::IMPLPtr();
     *        else
     *          return EvenIntegerCache::IMPLPtr(new int64_t(hash));
     *      }
     *  };
     *  \endcode
     *
     */
    template <class T, class PointerT =
                           typename FnPlatform::internal::SharedPtr<T>::type>
    class AttributeKeyedCache
    {
    public:
        typedef PointerT IMPLPtr;

        /**
         *  Construct new instance of the AttributeKeyedCache with specified
         *  cache size and invalid cache item size.
         *
         *  @param maxNumEntries maximum size of the main cache, once the number
         *         of entries in the cache grows above this value existing
         *         entries will be removed.
         *
         *  @param maxNumInvalidKeys maximum size of the invalid key set,
         *         once the number of entries in the cache grows above this
         *         value existing entries will be removed.
         */
        AttributeKeyedCache(std::size_t maxNumEntries=0xffffffff,
                            std::size_t maxNumInvalidKeys=0xffffffff)
          : maxNumEntries_(maxNumEntries), maxNumInvalidKeys_(maxNumInvalidKeys)
        {}

        virtual ~AttributeKeyedCache() {};

        /**
         *  Returns a pointer to the value that corresponds to the specified
         *  attribute. Returns NULL if the there is no corresponding value for
         *  the specified attribute.
         *
         *  @param iAttr the attribute key
         *  @return a pointer to the value associated with the specified
         *    attribute key or NULL if no such value exists.
         */
        IMPLPtr getValue( const FnAttribute::Attribute & iAttr )
        {
            // For these purposes, good enough to use a simple 64-bit hash.
            // The rationale is that AttributeKeyedCache should not grow to
            // the point where 2**64th hash entries are insufficient.
            //
            // For example, with 65K worth of entries the odds are roughly
            // 1 in 10-billion.
            // For 6 million entries, the odds would still be 1 in a million
            // (If the num entries is greater than ~20 million, then all 128
            // bits of hash should be used)

            uint64_t hash = iAttr.getHash().uint64();

            if (maxNumInvalidKeys_ > 0)
            {
                // lock for our search in the invalid set
                FnPlatform::internal::LockGuard<FnPlatform::internal::Mutex>
                    lock(invalidKeysMutex_);

                if (invalidKeys_.find(hash) != invalidKeys_.end())
                {
                    return IMPLPtr();
                }
            }

            typename std::map< uint64_t, IMPLPtr >::iterator it;

            if (maxNumEntries_ > 0)
            {
                // lock for our search in the cache
                FnPlatform::internal::LockGuard<FnPlatform::internal::Mutex>
                    lock(entriesMutex_);

                it = entries_.find(hash);

                if (it != entries_.end())
                {
                    return it->second;
                }
            }

            IMPLPtr val = createValue(iAttr);
            if (val && maxNumEntries_ != 0)
            {
                FnPlatform::internal::LockGuard<FnPlatform::internal::Mutex>
                    lock(entriesMutex_);
                while (entries_.size() >= maxNumEntries_)
                {
                    it = entries_.begin();

                    std::size_t d = std::rand() % entries_.size();
                    for(; d > 0; --d)
                    {
                        ++it;
                    }
                    entries_.erase(it);
                }
                entries_[hash] = val;
            }
            else if (!val && maxNumInvalidKeys_ != 0)
            {
                FnPlatform::internal::LockGuard<FnPlatform::internal::Mutex>
                    lock(invalidKeysMutex_);
                while (invalidKeys_.size() >= maxNumInvalidKeys_)
                {
                    std::set< uint64_t >::iterator jt = invalidKeys_.begin();

                    // for now it's a random delete
                    std::size_t d = std::rand() % invalidKeys_.size();
                    for(; d > 0; --d)
                    {
                        ++jt;
                    }
                    invalidKeys_.erase(jt);
                }

                invalidKeys_.insert(hash);
            }
            return val;
        }

        /**
         *  Clears all entries from the cache.
         */
        void clear()
        {
            {
                FnPlatform::internal::LockGuard<FnPlatform::internal::Mutex>
                    lock(entriesMutex_);
                entries_.clear();
            }

            {
                FnPlatform::internal::LockGuard<FnPlatform::internal::Mutex>
                    lock(invalidKeysMutex_);
                invalidKeys_.clear();
            }
        }
    protected:
        /**
         *  Called by getValue() to obtain an instance of the templated type
         *  which corresponds to the specified attribute.
         *
         *  Subclasses of AttributeKeyedCache should implement whatever logic
         *  necessary to determine the appropriate value to be returned given
         *  the supplied attribute. If no value is appropriate NULL should be
         *  returned.
         *
         *  Note: The implementation of createValue() <b>WILL</b> be called
         *  from multiple threads and therefore, must be thread safe.
         *
         *  @param iAttr the attribute which should be used
         *  @return pointer to value corresponding to supplied parameter or
         *    NULL.
         */
        virtual IMPLPtr createValue(const FnAttribute::Attribute & iAttr) = 0;
    private:
        std::size_t maxNumEntries_;
        std::size_t maxNumInvalidKeys_;

        std::map< uint64_t, IMPLPtr > entries_;
        std::set< uint64_t > invalidKeys_;

        FnPlatform::internal::Mutex invalidKeysMutex_;
        FnPlatform::internal::Mutex entriesMutex_;

    };

    /**
     * @}
     */
}
FNGEOLIBUTIL_NAMESPACE_EXIT

#endif // INCLUDED_FNGEOLIBUTIL_ATTRIBUTEKEYEDCACHE_H
