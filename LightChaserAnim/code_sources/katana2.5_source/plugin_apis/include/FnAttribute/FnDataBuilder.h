// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FoundryKatanaDataBuilder_H
#define FoundryKatanaDataBuilder_H

#include <set>
#include <map>
#include <vector>
#include <FnAttribute/FnAttribute.h>

#include "ns.h"

FNATTRIBUTE_NAMESPACE_ENTER
{
    /**
     * \ingroup FnAttribute
     * @{
     */

    /**
     *  @brief Helper class used for creating attributes.
     *
     *  Data is pushed into the DataBuilder, when populated with all the required data
     *  the builder build() can be called to generate a new Attribute.
     *
     *  The API provides several convenience typedefs of this class for the
     *  different Attribute types, which remove the template from DataBuilder:
     *  <b>IntBuilder</b>, <b>FloatBuilder</b>, <b>DoubleBuilder</b>, <b>StringBuilder</b>
     */
    template<class ATTR> class DataBuilder {
    public:
        /**
         * Creates an empty DataBuilder with a given tuple size
         */
        DataBuilder( int64_t tupleSize = 1) : _tupleSize(tupleSize) {}

        /**
         * Copies the data from the attribute attr into the DataBuilder
         */
        DataBuilder( ATTR attr );

        /**
         * Reserves timeSampleDataCount entries in the given timesample data
         */
        void reserve(int64_t timeSampleDataCount, float timeSample = 0.f);

        /**
         *Gets the current size tuple size of the builder
         */
        int64_t getTupleSize() const;

        /**
         * Get the current time samples in the builder
         */
        std::vector<float> getTimeSamples() const;

        /**
         * Get a vector at the given data sample
         */
        std::vector<typename ATTR::value_type>& get(const float timeSample = 0.f);

        /**
         * Copies timeSampleData into the builder at the given time sample
         */
        void set(const std::vector<typename ATTR::value_type>& timeSampleData, const float timeSample = 0.f);

        /**
         * Removes the time sample data from the builder
         */
        void del(const float timeSample = 0.f);

        /**
         * Push back a single data element onto a given timesample
         */
        void push_back( typename ATTR::value_type data, float timeSample = 0.f);

        /**
         * Copies the attribute data into the builder
         */
        void update(ATTR &attr);

        /**
         * Create an attribute from the current data in the builder
         */
        ATTR build();

    private:
        typedef ConstVector<typename ATTR::value_type> DataConstVector;
        typedef typename DataConstVector::const_iterator DataConstVectorItereator;
        typedef std::map<float,std::vector<typename ATTR::value_type> > datacontainer;
        typedef typename datacontainer::iterator dataiterator;
        typedef typename datacontainer::const_iterator dataconstiterator;
        datacontainer _data;
        int64_t _tupleSize;
    };

    template<class ATTR> DataBuilder<ATTR>::DataBuilder( ATTR attr )
    {
        update( attr );
    }

    template<class ATTR> void DataBuilder<ATTR>::reserve(int64_t timeSampleDataCount, float timeSample)
    {
        _data[timeSample].reserve(timeSampleDataCount);
    }

    template<class ATTR> int64_t DataBuilder<ATTR>::getTupleSize() const
    {
        return _tupleSize;
    }

    template<class ATTR> std::vector<float> DataBuilder<ATTR>::getTimeSamples() const
    {
        std::vector<float> timeSample;
        for ( dataconstiterator iter = _data.begin(); iter != _data.end(); iter++)
        {
            timeSample.push_back( iter->first );
        }
        return timeSample;
    }

    template<class ATTR> std::vector<typename ATTR::value_type>& DataBuilder<ATTR>::get(const float timeSample)
    {
        return _data[timeSample];
    }

    template<class ATTR> void DataBuilder<ATTR>::set(const std::vector<typename ATTR::value_type>& timeSampleData,const float timeSample)
    {
        _data[timeSample] = timeSampleData;
    }

    template<class ATTR> void DataBuilder<ATTR>::del(const float timeSample)
    {
        _data.erase( timeSample );
    }

    template<class ATTR> void DataBuilder<ATTR>::push_back( typename ATTR::value_type data, float timeSample)
    {
        _data[timeSample].push_back(data);
    }

    template<class ATTR> ATTR DataBuilder<ATTR>::build()
    {
        if ( _data.size() == 0 )
        {
            // Return an attribute representing an empty array of the given
            // tuple size
            return ATTR(0x0, 0, _tupleSize);
        }

        std::vector<float> timeSample;
        std::vector<const typename ATTR::value_type*> timeSampleData;
        size_t timeSampleDataCount = _data.begin()->second.size();
        for ( dataiterator iter = _data.begin(); iter != _data.end(); iter++)
        {
            timeSample.push_back( iter->first );
            timeSampleData.push_back( iter->second.data() );
            // Time-sampled data must all be of the same size
            if ( timeSampleDataCount != iter->second.size() )
            {
                throw std::runtime_error("Error building attribute time samples are different sizes.");
            }
        }

        return ATTR( timeSample.data(),
                     (int64_t) timeSample.size(),
                     (const typename ATTR::value_type **) timeSampleData.data(),
                     (int64_t) timeSampleDataCount, (int64_t) _tupleSize);
    }

    template<class ATTR> void DataBuilder<ATTR>::update(ATTR &attr)
    {
        DataConstVector sampleTimes = attr.getSampleTimes();
        for ( DataConstVectorItereator iter = sampleTimes.begin(); iter != sampleTimes.end(); iter++ )
        {
            DataConstVector currentData = attr.getNearestSample(*iter);
            _data[*iter] = std::vector<typename ATTR::value_type>(currentData.begin(),currentData.end() );
        }
        _tupleSize = attr.getTupleSize();
    }



    typedef DataBuilder<IntAttribute> IntBuilder;
    typedef DataBuilder<FloatAttribute> FloatBuilder;
    typedef DataBuilder<DoubleAttribute> DoubleBuilder;

    // StringBuilder is typically not recommended for use.
    // As strings are not generally multisampled, a builder is not appropriate.
    // (or efficient) If you'd like the convenience of array initialization,
    // consider using the std::vector<std::string> constructor directly.

    typedef DataBuilder<StringAttribute> StringBuilder;

    template<>
    FNATTRIBUTE_API StringAttribute DataBuilder<StringAttribute>::build();


    // This makes it convenient to construct a StringAttr where the entries
    // are unique

    class UniqueStringBuilder
    {
    private:
        std::vector<StringAttribute> attrs;

    public:
        UniqueStringBuilder()
        {
        }

        void add(const StringAttribute & attr)
        {
            if (!attr.isValid()) return;
            // This catches the case where you add a bunch of
            // the same stringattrs together in a row
            if (attrs.size()>0 &&
               (attr.getHash() == attrs[attrs.size()-1].getHash())) return;
            attrs.push_back(attr);
        }

        StringAttribute build()
        {
            if (attrs.empty())
            {
                return StringAttribute();
            }

            if (attrs.size() == 1)
            {
                return attrs[0];
            }

            std::set<std::string> usedValues;
            std::vector<std::string> newValues;

            for (std::vector<StringAttribute>::iterator I = attrs.begin();
                I != attrs.end(); ++I)
            {
                StringAttribute::array_type values = (*I).getNearestSample(0);

                for (StringAttribute::array_type::const_iterator vI =
                        values.begin(), vE = values.end(); vI != vE; ++vI)
                {
                    std::string value(*vI);

                    if (usedValues.find(value) == usedValues.end())
                    {
                        usedValues.insert(value);
                        newValues.push_back(value);
                    }
                }
            }

            if (!newValues.empty())
            {
                return StringAttribute(newValues);
            }
            return StringAttribute();
        }
    };

    /// @}
}
FNATTRIBUTE_NAMESPACE_EXIT

#endif // FoundryKatanaDataBuilder_H
