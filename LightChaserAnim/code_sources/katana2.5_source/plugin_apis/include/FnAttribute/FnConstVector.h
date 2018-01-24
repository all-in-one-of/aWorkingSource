// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FoundryKatanaConstVector_H
#define FoundryKatanaConstVector_H

#include <stdexcept>
#include <vector>
#include <string>

#include "ns.h"

FNATTRIBUTE_NAMESPACE_ENTER
{
    /**
     * \ingroup FnAttribute
     * @{
     */

    /**
     *  @brief Helper class used to wrap raw attribute data contained
     *  on the host side, with a stl::vector style interface.
     *
     *  The data is immutable so only const references are avaliable.
     *
     *  There are several convenience typedefs of this class for the
     *  different Attribute types, which remove the template from ConstVector:
     *  <b>IntConstVector</b>, <b>FloatConstVector</b>, <b>DoubleConstVector</b>, <b>StringConstVector</b>
     */
    template<class T> class ConstVector
    {
        private:
        const T* elems;    // fixed-size array of elements of type T
        std::size_t N;
        public:

        /**
         * Basic type of the data in ConstVector
         */
        typedef T              value_type;
        /**
         * Const iterator for the data type
         */
        typedef const T*       const_iterator;
        /**
         * Const reference for the data type
         */
        typedef const T&       const_reference;
        /**
         * Typedef of the sizetype
         */
        typedef std::size_t    size_type;
        typedef std::ptrdiff_t difference_type;
        /**
         * Constructor used to create ConstVector from a pointer to the
         * data elements and the number of elements.
         */
        ConstVector( const T* elemsPointer, const std::size_t elemsSize )
            : elems(elemsPointer), N(elemsSize) {}

        ConstVector()
            : elems(NULL), N(0) {}

        ConstVector<T>& operator=(const ConstVector<T>& rhs)
        {
            elems = rhs.elems;
            N = rhs.N;
            return *this;
        }

        /**
         * Returns an iterator at the beginning of the data
         */
        const_iterator begin() const { return elems; }

        /**
         * Returns an iterator at the end of the data
         */
        const_iterator end() const { return elems+N; }

        /**
         * Individual element access read only
         */
        const_reference operator[](size_type i) const
        {
            return elems[i];
        }

        /**
         * Individual element access read only
         */
        const_reference at(size_type i) const
        {
            return elems[i];
        }

        /**
         * Returns a const reference to the first element
         */
        const_reference front() const
        {
            return elems[0];
        }

        /**
         * Returns a const reference to the last element
         */
        const_reference back() const
        {
            return elems[N-1];
        }

        /**
         * Returns the number of data elements
         */
        size_type size() const
        {
            return N;
        }

        /**
         * Returns false if the ConstVector contains any data
         */
        bool empty() const
        {
            return (N==0);
        }

        /**
         * Returns the number of data elements
         */
        size_type max_size() const { return N; }

        /**
         * Pointer to the raw data (read-only)
         */
        const T* data() const { return elems; }

    };

    typedef ConstVector<int32_t> IntConstVector;
    typedef ConstVector<float> FloatConstVector;
    typedef ConstVector<double> DoubleConstVector;
    typedef ConstVector<const char *> StringConstVector;

    /// @}

}
FNATTRIBUTE_NAMESPACE_EXIT

#endif // FoundryKatanaConstVector_H
