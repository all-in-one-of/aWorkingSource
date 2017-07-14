// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FoundryKatanaAttribute_H
#define FoundryKatanaAttribute_H

#include <string>
#include <cstring>
#include <inttypes.h>

#include <FnPluginSystem/FnPluginSystem.h>
#include <FnAttribute/suite/FnAttributeSuite.h>
#include <FnAttribute/FnConstVector.h>

#include "ns.h"

FNATTRIBUTE_NAMESPACE_ENTER
{
    /**
     * Bootstraps the API without having to link against the internal libraries
     * that implement the Attributes host. This can be useful when implementing
     * an executable that needs to use the Attributes API via the plug-in system,
     * without having to bootstrap Geolib or link against any internal library.
     *
     * Returns true if the bootstrap succeeds.
     */
    bool Bootstrap(const std::string& katanaPath);

    /**
     * Initializes the API with the given Attribute Host suite.
     */
    void Initialize(const FnAttributeHostSuite_v2 *);

    /** \defgroup FnAttribute Attributes API
     * @{
     *
     * @brief API that allows to manipulate Katana Attributes.
     *
     *
     * Attributes are the basic data storage classes used by Katana's scenegraph.
     * Data types are int, float, double, string, and each attribute contains a
     * map of time samples, with each sample being an array of the base data type.
     * Group attributes contain a list of named child attributes.
     *
     * This API should used in every plug-in that reads/manipulates Katana attributes.
     *
     * All attributes, once constructed, are immutable.  Attributes can be (and
     * often are) referenced in many locations.  Their lifetime is managed internally by
     * ref-counted smart pointers.
     *
     * The API provides several utility classes to build Attributes in more friendly way
     *  (DataBuilder, and its attribute type specific typedefs: <b>IntBuilder</b>,
     *  <b>FloatBuilder</b>, <b>DoubleBuilder</b>).
     *
     *\internal
     * All attributes created via the C API are allocated by the host.  The C++
     * wrappers manage references to the attributes via the retainAttr()/releaseAttr()
     * C API calls.  Copy/assignment of a C++ wrapper instance simply retains a new
     * reference to the attribute handle, so is a very lightweight operation.
     * Copy/assignment also checks the attribute type and will only accept a reference
     * to the handle if it is the proper type (copy/assignment, followed by isValid()
     * can be used for RTTI).
     * \endinternal
     */
     
    class Attribute;

    struct Hash
    {
        uint64_t hash1;
        uint64_t hash2;

        std::string str() const;
        uint64_t uint64() const {
            /* Per spooky hash docs,
             * It can produce 64-bit and 32-bit hash values too, at the same speed,
             * just use the bottom n bits.
             */
            return hash1;
        }

        Attribute attr() const;

        Hash(): hash1(0), hash2(0)
        { }

        Hash(uint64_t hash1_, uint64_t hash2_): hash1(hash1_), hash2(hash2_)
        { }

        Hash(FnAttributeHash h) : hash1(h.hash1), hash2(h.hash2)
        { }

        bool operator==(const Hash & rhs) const
        {
            return ((hash1 == rhs.hash1) && (hash2 == rhs.hash2));
        }

        bool operator!=(const Hash & rhs) const
        {
            return ((hash1 != rhs.hash1) || (hash2 != rhs.hash2));
        }

        bool operator<(const Hash & rhs) const
        {
            return (hash1 == rhs.hash1) ? (hash2 < rhs.hash2) : (hash1 < rhs.hash1);
        }
    };

    /**
     *  @brief The base class of all Attributes.
     */
    class Attribute
    {
    public:
        /**
         * Create empty attribute class (isValid() == false).
         */
        Attribute() : _handle(0x0) {}

        ~Attribute() {
            if (_handle)
                getSuite()->releaseAttr(_handle);
        }

        /**
         * Returns true if this attribute is valid.
         */
        bool isValid() const {return _handle != 0x0;}

        /**
         * Returns the Type of Attribute
         */
        FnKatAttributeType getType() const {
            return getSuite()->getType(getHandle());
        }

        /**
         * Returns an xml representation of the attribute.
         */
        std::string getXML() const;

        /**
         * Returns an Attribute from an XML representation.
         */
        static Attribute parseXML(const char * xml);

         /**
         * Returns an binary representation of the attribute.
         */
        void getBinary(std::vector<char> * buffer) const;

        /**
         * Returns an Attribute from a binary representation.
         */
        static Attribute parseBinary(const char * buffer, size_t size);

        /**
         * Writes an Attribute to the stream provided.
         * Returns true for success, false for failure.
         *  @param stream Opaque pointer to stream object.
         *  @param func Function to write bytes to stream object.
         *  @param streamType How to write the stream either as XML or binary.
         */
        bool writeAttributeStream(void *stream, FnAttributeWriteStreamFunc func, FnKatStreamType streamType) const;

        /**
         * Returns an Attribute read from the stream provided.
         *  @param stream Opaque pointer to stream object.
         *  @param func Function to read bytes from stream object.
         *  @param streamType How to read the stream either as XML or binary.
         */
        static Attribute readAttributeStream(void *stream, FnAttributeReadStreamFunc func, FnKatStreamType streamType);

        Hash getHash() const {
            return Hash(getSuite()->getHash(getHandle()));
        }

        Attribute(const Attribute& rhs) : _handle(0x0)
        {
            acceptHandle(rhs);
        }

        Attribute& operator=(const Attribute& rhs)
        {
            acceptHandle(rhs);
            return *this;
        }


        friend bool operator==(const Attribute & lhs, const Attribute & rhs);
        friend bool operator!=(const Attribute & lhs, const Attribute & rhs);

        ///@cond FN_INTERNAL_DEV

        FnAttributeHandle getHandle() const { return _handle; }

        FnAttributeHandle getRetainedHandle() const
        {
            if (_handle!=0x0) getSuite()->retainAttr(_handle);
            return _handle;
        }


        static const FnAttributeHostSuite_v2 *getSuite() {
            return _attrSuite;
        }

        static FnPlugStatus setHost(FnPluginHost *host);
        static void setSuite(const FnAttributeHostSuite_v2 *suite);

        static Attribute CreateAndSteal(FnAttributeHandle handle) {
            return Attribute(handle);
        }

        static Attribute CreateAndRetain(FnAttributeHandle handle) {
            if(handle)
                getSuite()->retainAttr(handle);
            return Attribute(handle);
        }

    protected:
        Attribute(FnAttributeHandle handle) : _handle(handle) {}

        // set internal _handle and grab a reference
        void acceptHandle(const Attribute &rhs)
        {
            getSuite()->retainAttr(rhs._handle);
            if (_handle!=0x0) getSuite()->releaseAttr(_handle);
            _handle = rhs._handle;
        }

        // steal the specified handle, and do not increment it.
        void stealHandle(const FnAttributeHandle & handle)
        {
            if (_handle!=0x0) getSuite()->releaseAttr(_handle);
            _handle = handle;
        }

        void clearHandle()
        {
            if (_handle!=0x0) getSuite()->releaseAttr(_handle);
            _handle = 0x0;
        }
        void checkAndAcceptHandle(const Attribute &rhs, FnKatAttributeType type)
        {
            if (rhs.isValid() && rhs.getType() == type)
            {
                acceptHandle(rhs);
            }
            else
            {
                clearHandle();
            }
        }

    private:
        static const FnAttributeHostSuite_v2 *_attrSuite;
        FnAttributeHandle _handle;

        /// @endcond
    };

    inline bool operator==(const Attribute & lhs, const Attribute & rhs)
    {
        return lhs.getSuite()->isEqual(lhs.getHandle(), rhs.getHandle());
    }

    inline bool operator!=(const Attribute & lhs, const Attribute & rhs)
    {
        return (!(lhs==rhs));
    }


    /**
     *  @brief The base class of attributes containing data, possibly at multiple samples in time.
     *
     *  The data are organised in tuples of a fixed size of 1 or greater per attribute.
     *  DataAttribute implements common, data-agnostic functionality, such as querying the
     *  number and orientation of values and time samples.
     */

    class DataAttribute : public Attribute
    {
    public:
        /**
         * Create empty attribute class (isValid() == false).
         */
        DataAttribute() {}

        /**
         * Return the number of data values per tuple for this attribute.
         */
        int64_t getTupleSize() const
        {
            return getSuite()->getTupleSize(getHandle());
        }
        /**
         * Return the total number of data values for this attribute. This will be equal to
         * getNumberOfTuples() * getTupleSize()
         */
        int64_t getNumberOfValues() const
        {
            return getSuite()->getNumberOfValues(getHandle());
        }
        /**
         * Return the number of tuples in this attribute.
         */
        int64_t getNumberOfTuples() const
        {
            return getSuite()->getNumberOfTuples(getHandle());
        }
        /**
         * Return the number of time samples at which data is recorded in this attribute.
         */
        int64_t getNumberOfTimeSamples() const
        {
            return getSuite()->getNumberOfTimeSamples(getHandle());
        }
        /**
         * Returns a float value containing the time at a particular index
         * for this attribute.  If the index is not valid 0.0 is returned.
         */
        float getSampleTime(int64_t index) const
        {
            return getSuite()->getSampleTime(getHandle(), index);
        }

        /**
         * For the specified sampletime, return the two bounding samples.
         * (left bounds, right bounds).
         * If the sampletime exactly falls on a single sample, ltime == rtime
         * (this is also the case at extrema, or for attrs with only a single
         * sample.
         * For attrs with 0 samples, the return value is false (signifying failure)
         * For attrs >0 samples, return value is true
         */

        bool getBoundingSampleTimes(float *ltime, float *rtime, float sampletime) const
        {
            return bool(getSuite()->getBoundingSampleTimes(getHandle(), ltime, rtime, sampletime));
        }

        DataAttribute(const Attribute& rhs) : Attribute(0x0)
        {
            checkAndAcceptDataHandle(rhs);
        }

        DataAttribute& operator=(const Attribute& rhs)
        {
            checkAndAcceptDataHandle(rhs);
            return *this;
        }

        ///@cond FN_INTERNAL_DEV

    protected:
        DataAttribute(FnAttributeHandle handle) : Attribute(handle) {}

    private:
        void checkAndAcceptDataHandle(const Attribute &rhs)
        {
            if (rhs.isValid())
            {
                FnKatAttributeType type = rhs.getType();
                if (type == kFnKatAttributeTypeInt    || type == kFnKatAttributeTypeFloat ||
                    type == kFnKatAttributeTypeDouble || type == kFnKatAttributeTypeString)
                {
                    acceptHandle(rhs);
                    return;
                }
            }

            clearHandle();
        }
        //@endcond
    };

    /**
     * @brief A class representing a null value.
     */

    class NullAttribute : public Attribute
    {
    public:
        NullAttribute() : Attribute(getSuite()->createNullAttr()) {}
        NullAttribute(const Attribute& rhs) : Attribute(0x0)
        {
            checkAndAcceptHandle(rhs, kFnKatAttributeTypeNull);
        }
        
        NullAttribute& operator=(const Attribute& rhs)
        {
            checkAndAcceptHandle(rhs, kFnKatAttributeTypeNull);
            return *this;
        }
        
        static FnKatAttributeType getKatAttributeType()
        {
            return kFnKatAttributeTypeNull;
        }
    private:
        NullAttribute(FnAttributeHandle handle); // NOTE: not implemented
    };

    /**
     * @brief A class representing a data attribute containing integers.
     */
    class IntAttribute : public DataAttribute
    {
    public:
        typedef int32_t value_type;
        typedef IntConstVector array_type;

        /**
         *  Create empty attribute class (isValid() == false).
         */
        IntAttribute() : DataAttribute() {}

        /**
         *  Creates an integer attribute containing one value.
         *  @param value The integer value for the new attribute
         */
        IntAttribute(value_type value) : DataAttribute(getSuite()->createIntAttr1(value)) {}

        /**
         *  Creates an integer attribute containing a number of values, grouped
         *  into tuples of a given size.
         *  @param values An array of integers containing the values
         *  @param valueCount  The total number of values. Must be a multiple of tupleSize
         *  @param tupleSize  The size of tuples to group values into.
         */
        IntAttribute(const value_type* values, int64_t valueCount, int64_t tupleSize) :
            DataAttribute(getSuite()->createIntAttr2(values, valueCount, tupleSize)) {}

        /**
         *  Creates an integer attribute containing several timed samples, each containing
         *  a number of values, grouped into tuples of a given size.
         *  @param times An array of floats, giving the times of the samples.
         *  @param timeCount The number of samples for which there is data
         *  @param values An array of pointers to integer arrays containing values. There must be timeCount arrays, each of which has valueCount values.
         *  @param valueCount  The total number of values in each sample. Must be a multiple of tupleSize
         *  @param tupleSize  The size of tuples to group values into.
         */
        IntAttribute(const float *times, int64_t timeCount,
            const value_type **values, int64_t valueCount, int64_t tupleSize) :
            DataAttribute(getSuite()->createIntAttr3(times, timeCount,
                values, valueCount, tupleSize)) {}

        /**
         * Returns the intended type of this Attribute, without needing a valid
         * handle.
         */
        static FnKatAttributeType getKatAttributeType()
        {
            return kFnKatAttributeTypeInt;
        }

        /**
         * Returns the unmodified values of the sample nearest to a given time.
         * This returns a ConstVector to the data at the sample.
         */
        array_type getNearestSample(float time) const
        {
            int64_t valueCount;
            const value_type* data = getSuite()->getIntNearestSample(getHandle(), time, &valueCount);
            return IntConstVector(data, valueCount);
        }

        /**
         * Returns first value from the time sample nearest 0.0.  This is a convenience
         * for the extremely common case of an attribute that stores a single sample
         * of a single value at time 0.0.
         * By default, throws std::runtime_error if there are no time samples or
         * no values available.
         *  @param defValue  The value to return if an error occurs and throwOnError is false.
         *  @param throwOnError  When error occurs, if true, throw std::runtime_error.  If false, return defValue.
         */
        value_type getValue(const value_type defValue = 0, bool throwOnError=true) const
        {
            if (isValid())
            {
                IntConstVector values = getNearestSample(0.f);
                if ( values.size() )
                {
                    return values[0];
                }
            }
            if (throwOnError)
            {
                throw std::runtime_error("Error getting int value from IntAttribute.");
            }
            else
            {
                return defValue;
            }
        }

        IntAttribute(const Attribute& rhs) : DataAttribute(0x0)
        {
            checkAndAcceptHandle(rhs, kFnKatAttributeTypeInt);
        }

        IntAttribute& operator=(const Attribute& rhs)
        {
            checkAndAcceptHandle(rhs, kFnKatAttributeTypeInt);
            return *this;
        }
    private:
        IntAttribute(FnAttributeHandle handle); // NOTE: not implemented
    };

    /**
     * @brief A class representing a data attribute containing single-precision floats.
     */

    class FloatAttribute : public DataAttribute
    {
    public:
        typedef float value_type;
        typedef FloatConstVector array_type;

        /**
         *  Create empty attribute class (isValid() == false).
         */
        FloatAttribute() : DataAttribute() {}
        /**
         *  Creates a float attribute containing one value.
         *  @param value The float value for the new attribute
         */
        FloatAttribute(value_type value) : DataAttribute(getSuite()->createFloatAttr1(value)) {}
        /**
         *  Creates a float attribute containing a number of values, grouped
         *  into tuples of a given size.
         *  @param values An array of floats containing the values
         *  @param valueCount  The total number of values. Must be a multiple of tupleSize
         *  @param tupleSize  The size of tuples to group values into.
         */
        FloatAttribute(const value_type* values, int64_t valueCount, int64_t tupleSize) :
            DataAttribute(getSuite()->createFloatAttr2(values, valueCount, tupleSize)) {}
        /**
         *  Creates a float attribute containing several timed samples, each containing
         *  a number of values, grouped into tuples of a given size.
         *  @param times An array of floats, giving the times of the samples.
         *  @param timeCount The number of samples for which there is data
         *  @param values An array of pointers to float arrays containing values. There must be timeCount arrays, each of which has valueCount values.
         *  @param valueCount  The total number of values in each sample. Must be a multiple of tupleSize
         *  @param tupleSize  The size of tuples to group values into.
         */
        FloatAttribute(const float *times, int64_t timeCount,
            const value_type **values, int64_t valueCount, int64_t tupleSize) :
            DataAttribute(getSuite()->createFloatAttr3(times, timeCount,
                values, valueCount, tupleSize)) {}

        /**
         * Returns the intended type of this Attribute, without needing a valid
         * handle.
         */
        static FnKatAttributeType getKatAttributeType()
        {
            return kFnKatAttributeTypeFloat;
        }

        /**
         * Returns the unmodified values of the sample nearest to a given time.
         * This returns a ConstVector to the data at the sample.
         */
        array_type getNearestSample(float time) const
        {
            int64_t valueCount;
            const value_type* data = getSuite()->getFloatNearestSample(getHandle(), time, &valueCount);
            return FloatConstVector(data, valueCount);
        }

        /**
         * Fills the interpolated values for an exact time. If the time falls
         * between two samples, the values of the samples are interpolated to
         * make the result. If you wish to interpolate all values in the
         * attr, valueCount should equal attr.getNumberOfValues()
         */
        void fillInterpSample(value_type * array, int64_t valueCount,
            float sampleTime,
            const value_type defValue = 0.f, bool throwOnError=true) const;

        /**
         * Returns first value from the time sample nearest 0.0.  This is a convenience
         * for the extremely common case of an attribute that stores a single sample
         * of a single value at time 0.0.
         * By default, throws std::runtime_error if there are no time samples or
         * no values available.
         *  @param defValue  The value to return if an error occurs and throwOnError is false.
         *  @param throwOnError  When error occurs, if true, throw std::runtime_error.  If false, return defValue.
         */
        value_type getValue(const value_type defValue = 0.f, bool throwOnError=true) const
        {
            if (isValid())
            {
                array_type values = getNearestSample(0.f);
                if ( values.size() )
                {
                    return values[0];
                }
            }
            if (throwOnError)
            {
                throw std::runtime_error("Error getting float value from FloatAttribute.");
            }
            else
            {
                return defValue;
            }
        }

        FloatAttribute(const Attribute& rhs) : DataAttribute(0x0)
        {
            checkAndAcceptHandle(rhs, kFnKatAttributeTypeFloat);
        }

        FloatAttribute& operator=(const Attribute& rhs)
        {
            checkAndAcceptHandle(rhs, kFnKatAttributeTypeFloat);
            return *this;
        }

    private:
        FloatAttribute(FnAttributeHandle handle); // NOTE: not implemented
    };

    /**
     * @brief A class representing a data attribute containing double-precision floats.
     */
    class DoubleAttribute : public DataAttribute
    {
    public:
        typedef double value_type;
        typedef DoubleConstVector array_type;

        /**
         *  Create empty attribute class (isValid() == false).
         */
        DoubleAttribute() : DataAttribute() {}
        /**
         *  Creates a double attribute containing one value.
         *  @param value The double value for the new attribute
         */
        DoubleAttribute(value_type value) :DataAttribute(getSuite()->createDoubleAttr1(value)) {}

        /**
         *  Creates a double attribute containing a number of values, grouped
         *  into tuples of a given size.
         *  @param values An array of doubles containing the values
         *  @param valueCount  The total number of values. Must be a multiple of tupleSize
         *  @param tupleSize  The size of tuples to group values into.
         */
        DoubleAttribute(const value_type* values, int64_t valueCount, int64_t tupleSize) :
            DataAttribute(getSuite()->createDoubleAttr2(values, valueCount, tupleSize)) {}
        /**
         *  Creates a double attribute containing several timed samples, each containing
         *  a number of values, grouped into tuples of a given size.
         *  @param times An array of floats, giving the times of the samples.
         *  @param timeCount The number of samples for which there is data
         *  @param values An array of pointers to double arrays containing values. There must be timeCount arrays, each of which has valueCount values.
         *  @param valueCount  The total number of values in each sample. Must be a multiple of tupleSize
         *  @param tupleSize  The size of tuples to group values into.
         */
        DoubleAttribute(const float *times, int64_t timeCount,
            const value_type **values, int64_t valueCount, int64_t tupleSize) :
            DataAttribute(getSuite()->createDoubleAttr3(times, timeCount,
                values, valueCount, tupleSize)) {}

        /**
         * Returns the intended type of this Attribute, without needing a valid
         * handle.
         */
        static FnKatAttributeType getKatAttributeType()
        {
            return kFnKatAttributeTypeDouble;
        }

        /**
         * Returns the unmodified values of the sample nearest to a given time.
         * This returns a ConstVector to the data at the sample.
         */
        array_type getNearestSample(float time) const
        {
            int64_t valueCount;
            const value_type* data = getSuite()->getDoubleNearestSample(getHandle(), time, &valueCount);
            return DoubleConstVector(data,valueCount);
        }


        /**
         * Fills the interpolated values for an exact time. If the time falls
         * between two samples, the values of the samples are interpolated to
         * make the result. If you wish to interpolate all values in the
         * attr, valueCount should equal attr.getNumberOfValues()
         */
        void fillInterpSample(value_type * array, int64_t valueCount,
            float sampleTime,
            const value_type defValue = 0.0, bool throwOnError=true) const;

        /**
         * Returns first value from the time sample nearest 0.0.  This is a convenience
         * for the extremely common case of an attribute that stores a single sample
         * of a single value at time 0.0.
         * By default, throws std::runtime_error if there are no time samples or
         * no values available.
         *  @param defValue  The value to return if an error occurs and throwOnError is false.
         *  @param throwOnError  When error occurs, if true, throw std::runtime_error.  If false, return defValue.
         */
        value_type getValue(const value_type defValue = 0.0, bool throwOnError=true) const
        {
            if (isValid())
            {
                array_type values = getNearestSample(0.f);
                if ( values.size() )
                {
                    return values[0];
                }
            }
            if (throwOnError)
            {
                throw std::runtime_error("Error getting double value from DoubleAttribute.");
            }
            else
            {
                return defValue;
            }
        }

        DoubleAttribute(const Attribute& rhs) : DataAttribute(0x0)
        {
            checkAndAcceptHandle(rhs, kFnKatAttributeTypeDouble);
        }

        DoubleAttribute& operator=(const Attribute& rhs)
        {
            checkAndAcceptHandle(rhs, kFnKatAttributeTypeDouble);
            return *this;
        }
    private:
        DoubleAttribute(FnAttributeHandle handle); // NOTE: not implemented
    };

    /**
     * @brief A class representing a data attribute containing strings.
     */
    class StringAttribute : public DataAttribute
    {
    public:
        typedef std::string value_type;
        typedef StringConstVector array_type;

        /**
         *  Create empty attribute class (isValid() == false).
         */
        StringAttribute() : DataAttribute() {}

        /**
         *  Creates a string attribute containing one value.
         *  @param value The value for the new attribute, in the form of a std::string.
         */
        StringAttribute(const std::string & value) : DataAttribute(getSuite()->createStringAttr1(value.c_str())) {}

        /**
         *  Creates a string attribute containing one value.
         *  @param value The value for the new attribute, in the form of a C string.
         */
        StringAttribute(const char * value) : DataAttribute(getSuite()->createStringAttr1(value)) {}

        /**
         *  Creates a string attribute containing a number of values, grouped
         *  into tuples of a given size.
         *  @param values An array of std::strings containing the values
         *  @param valueCount  The total number of values. Must be a multiple of tupleSize
         *  @param tupleSize  The size of tuples to group values into.
         */
        StringAttribute(const value_type* values, int64_t valueCount, int64_t tupleSize);

        /**
         *  Creates a string attribute containing a number of values, grouped
         *  into tuples of a given size.
         *  @param values An array of C strings containing the values
         *  @param valueCount  The total number of values. Must be a multiple of tupleSize
         *  @param tupleSize  The size of tuples to group values into.
         */
        StringAttribute(const std::vector<std::string> & stringvec, int64_t tupleSize=1);

        StringAttribute(const char ** values, int64_t valueCount, int64_t tupleSize) :
            DataAttribute(getSuite()->createStringAttr2(values, valueCount, tupleSize)) {}

        /**
         * Returns the intended type of this Attribute, without needing a valid
         * handle.
         */
        static FnKatAttributeType getKatAttributeType()
        {
            return kFnKatAttributeTypeString;
        }


        /**
         *  Creates a string attribute containing several timed samples, each containing
         *  a number of values, grouped into tuples of a given size.
         *  @param times An array of floats, giving the times of the samples.
         *  @param timeCount The number of samples for which there is data
         *  @param values An array of pointers to C string arrays containing values. There must be timeCount arrays, each of which has valueCount values.
         *  @param valueCount  The total number of values in each sample. Must be a multiple of tupleSize
         *  @param tupleSize  The size of tuples to group values into.
         */
        StringAttribute(const float *times, int64_t timeCount,
            const char ***values, int64_t valueCount, int64_t tupleSize) :
            DataAttribute(getSuite()->createStringAttr3(times, timeCount,
                values, valueCount, tupleSize)) {}

        /**
         * Returns the unmodified values of the sample nearest to a given time.
         * This returns a ConstVector to the data at the sample.
         */
        array_type getNearestSample(float time) const
        {
            int64_t valueCount;
            const char** data = getSuite()->getStringNearestSample(getHandle(), time, &valueCount);
            return StringConstVector(data, valueCount);
        }


        /**
         * Returns first value from the time sample nearest 0.0.  This is a convenience
         * for the extremely common case of an attribute that stores a single sample
         * of a single value at time 0.0.
         * By default, throws std::runtime_error if there are no time samples or
         * no values available.
         *  @param defValue  The value to return if an error occurs and throwOnError is false.
         *  @param throwOnError  When error occurs, if true, throw std::runtime_error.  If false, return defValue.
         */
        value_type getValue(const value_type &defValue=value_type(), bool throwOnError=true) const
        {
            if (isValid())
            {
                StringConstVector values = getNearestSample(0.f);
                if ( values.size() )
                {
                    return values[0];
                }
            }
            if (throwOnError)
            {
                throw std::runtime_error("Error getting string value from StringAttribute.");
            }
            else
            {
                return defValue;
            }
        }

        /**
         * Returns first value from the time sample nearest 0.0.  This is a convenience
         * for the extremely common case of an attribute that stores a single sample
         * of a single value at time 0.0.
         * By default, throws std::runtime_error if there are no time samples or
         * no values available.
         *  @param defValue  The value to return if an error occurs and throwOnError is false.
         *  @param throwOnError  When error occurs, if true, throw std::runtime_error.  If false, return defValue.
         */
        const char * getValueCStr(const char * defValue="", bool throwOnError=true) const
        {
            if (isValid())
            {
                StringConstVector values = getNearestSample(0.f);
                if ( values.size() )
                {
                    return values[0];
                }
            }

            if (throwOnError)
            {
                throw std::runtime_error("Error getting string value from StringAttribute.");
            }
            else
            {
                return defValue;
            }
        }

        bool operator==(const char* str) const {
            if (isValid()) {
                int64_t valueCount=0;
                const char** data = getSuite()->getStringNearestSample(
                    getHandle(), 0.f, &valueCount);
                if (valueCount==1) // len>1 arrays should not equal scalars
                    return strcmp(data[0], str) == 0;
            }
            return false;
        }

        bool operator!=(const char* str) {
            return !operator==(str);
        }

        bool operator==(const std::string& str) const {
            if (isValid()) {
                int64_t valueCount=0;
                const char** data = getSuite()->getStringNearestSample(
                    getHandle(), 0.f, &valueCount);
                if (valueCount==1) // len>1 arrays should not equal scalars
                    return strcmp(data[0], str.c_str()) == 0;
            }
            return false;
        }

        bool operator!=(const std::string& str) {
            return !operator==(str);
        }

        StringAttribute(const Attribute& rhs) : DataAttribute(0x0)
        {
            checkAndAcceptHandle(rhs, kFnKatAttributeTypeString);
        }

        StringAttribute& operator=(const Attribute& rhs)
        {
            checkAndAcceptHandle(rhs, kFnKatAttributeTypeString);
            return *this;
        }

    private:
        StringAttribute(FnAttributeHandle handle); // NOTE: not implemented
    };

    /**
     * @brief A class representing a group attribute, used for hierarchically encapsulating
     * other attributes.
     */

    class GroupAttribute : public Attribute
    {
    public:

        typedef std::pair<std::string, Attribute> NamedAttr_Type;
        typedef std::vector<NamedAttr_Type> NamedAttrVector_Type;

        /**
         *  Create empty attribute class (isValid() == false).
         */
        GroupAttribute() : Attribute() {}

        explicit GroupAttribute(const bool groupInherit)
            : Attribute(getSuite()->createGroupAttr(NULL, NULL, 0,
                                                    (uint8_t)groupInherit))
        {
        }

        // Note: These constructors do not support delimited child names
        // (i.e., no dots in the name). If you wish to construct
        // a GroupAttribute with a deep hierarchy, use GroupBuilder instead.

        GroupAttribute(const std::string & name1, const Attribute & attr1,
                       const bool groupInherit)  : Attribute(NULL)
        {
            const char * names[] = { name1.c_str() };
            FnAttributeHandle attrs[] = { attr1.getHandle() };
            stealHandle(getSuite()->createGroupAttr(
                names, attrs, 1, (uint8_t) groupInherit));
        }

        GroupAttribute(const char * name1, const Attribute & attr1,
                       const bool groupInherit)  : Attribute(NULL)
        {
            const char * names[] = { name1 };
            FnAttributeHandle attrs[] = { attr1.getHandle() };
            stealHandle(getSuite()->createGroupAttr(
                names, attrs, 1, (uint8_t) groupInherit));
        }

        GroupAttribute(const std::string & name1, const Attribute & attr1,
                       const std::string & name2, const Attribute & attr2,
                       const bool groupInherit) : Attribute(NULL)
        {
            const char * names[] = { name1.c_str(), name2.c_str() };
            FnAttributeHandle attrs[] = { attr1.getHandle(), attr2.getHandle() };
            stealHandle(getSuite()->createGroupAttr(
                names, attrs, 2, (uint8_t) groupInherit));
        }

        GroupAttribute(const char * name1, const Attribute & attr1,
                       const char * name2, const Attribute & attr2,
                       const bool groupInherit) : Attribute(NULL)
        {
            const char * names[] = { name1, name2 };
            FnAttributeHandle attrs[] = { attr1.getHandle(), attr2.getHandle() };
            stealHandle(getSuite()->createGroupAttr(
                names, attrs, 2, (uint8_t) groupInherit));
        }

        GroupAttribute(const std::string & name1, const Attribute & attr1,
                       const std::string & name2, const Attribute & attr2,
                       const std::string & name3, const Attribute & attr3,
                       const bool groupInherit) : Attribute(NULL)
        {
            const char * names[] = { name1.c_str(), name2.c_str(),
                name3.c_str() };
            FnAttributeHandle attrs[] = { attr1.getHandle(), attr2.getHandle(),
                attr3.getHandle() };
            stealHandle(getSuite()->createGroupAttr(
                names, attrs, 3, (uint8_t) groupInherit));
        }

        GroupAttribute(const char * name1, const Attribute & attr1,
                       const char * name2, const Attribute & attr2,
                       const char * name3, const Attribute & attr3,
                       const bool groupInherit) : Attribute(NULL)
        {
            const char * names[] = { name1, name2, name3 };
            FnAttributeHandle attrs[] = { attr1.getHandle(), attr2.getHandle(),
                attr3.getHandle() };
            stealHandle(getSuite()->createGroupAttr(
                names, attrs, 3, (uint8_t) groupInherit));
        }

        GroupAttribute(const std::string & name1, const Attribute & attr1,
                       const std::string & name2, const Attribute & attr2,
                       const std::string & name3, const Attribute & attr3,
                       const std::string & name4, const Attribute & attr4,
                       const bool groupInherit) : Attribute(NULL)
        {
            const char * names[] = { name1.c_str(), name2.c_str(),
                name3.c_str(), name4.c_str() };
            FnAttributeHandle attrs[] = { attr1.getHandle(), attr2.getHandle(),
                attr3.getHandle(), attr4.getHandle() };
            stealHandle(getSuite()->createGroupAttr(
                names, attrs, 4, (uint8_t) groupInherit));
        }

        GroupAttribute(const char * name1, const Attribute & attr1,
                       const char * name2, const Attribute & attr2,
                       const char * name3, const Attribute & attr3,
                       const char * name4, const Attribute & attr4,
                       const bool groupInherit) : Attribute(NULL)
        {
            const char * names[] = { name1, name2, name3, name4 };
            FnAttributeHandle attrs[] = { attr1.getHandle(), attr2.getHandle(),
                attr3.getHandle(), attr4.getHandle() };
            stealHandle(getSuite()->createGroupAttr(
                names, attrs, 4, (uint8_t) groupInherit));
        }

        GroupAttribute(const NamedAttrVector_Type &children,
                       const bool inheritChildren);

        /**
         * Returns the intended type of this Attribute, without needing a valid
         * handle.
         */
        static FnKatAttributeType getKatAttributeType()
        {
            return kFnKatAttributeTypeGroup;
        }

        /**
         * Returns the number of child attributes under the group attribute.
         */
        int64_t getNumberOfChildren() const
        {
            return getSuite()->getNumberOfChildren(getHandle());
        }

        /**
         *  Returns the name of child at index under a group attribute.
         *  This is returned as a std::string.
         *  @param index  The index of the child name to return.
         */
        std::string getChildName(int64_t index) const
        {
            int32_t nameSize = 0;
            const char *name = getSuite()->getChildNameByIndex(getHandle(), index, &nameSize);
            if (nameSize > 0)
            {
                return std::string(name, nameSize);
            }
            return std::string();
        }

        /**
         *  Returns a child attribute by index within a group attribute.
         *  If index is out of range, an invalid Attribute object is returned
         *  (returnValue.isValid() == false).
         *  @param index  The index of the child attribute to return.
         */
        Attribute getChildByIndex(int64_t index) const
        {
            return Attribute::CreateAndSteal(getSuite()->getChildByIndex(getHandle(), index));
        }

        /**
         *  Looks up a child attribute by name, returning it
         *  If named child does not exist, an invalid Attribute object
         *  is returned  (returnValue.isValid() == false).
         *  @param name  The name of attribute to look up.
         */
        Attribute getChildByName(const std::string &name) const
        {
            return Attribute::CreateAndSteal(
                getSuite()->getChildByName(getHandle(), name.c_str(),
                    (int32_t) name.size()));
        }

        /**
         *  Looks up a child attribute by name, returning it
         *  If named child does not exist, an invalid Attribute object
         *  is returned  (returnValue.isValid() == false).
         *  @param name  The name of attribute to look up.
         */
        Attribute getChildByName(const char* name) const
        {
            return Attribute::CreateAndSteal(
                getSuite()->getChildByName(getHandle(), name,
                    (int32_t) strlen(name)));
        }


        bool getGroupInherit() const
        {
            return static_cast<bool>(getSuite()->getGroupInherit(getHandle()));
        }

        void fillChildVector(NamedAttrVector_Type * children) const;

        GroupAttribute(const Attribute& rhs) : Attribute(0x0)
        {
            checkAndAcceptHandle(rhs, kFnKatAttributeTypeGroup);
        }

        GroupAttribute& operator=(const Attribute& rhs)
        {
            checkAndAcceptHandle(rhs, kFnKatAttributeTypeGroup);
            return *this;
        }

    private:
        GroupAttribute(FnAttributeHandle handle); // NOTE: not implemented
    };

    // Name encoding utils. Useful to manually work around the case where
    // one wants to have a groupattr, where the immediate children have
    // dots in their names.
    //
    // 0x2E -> 0xB0 Period -> Degree sign
    // 0x2F -> 0xB1 Slash -> Plus-or-minus sign
    //
    // Both of these swaps correspond to a 0x82 offset

    std::string DelimiterEncode(const std::string & str);

    std::string DelimiterDecode(const std::string & str);

    /// @}
}
FNATTRIBUTE_NAMESPACE_EXIT

#endif // FoundryKatanaAttribute_H
