// Copyright (c) 2011 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnAttribute/FnDataBuilder.h>

FNATTRIBUTE_NAMESPACE_ENTER
{
    template<> StringAttribute DataBuilder<StringAttribute>::build()
    {
        if ( _data.size() == 0 )
        {
            // Return an attribute representing an empty array of the given
            // tuple size
            return StringAttribute((const char**) 0x0, 0, _tupleSize);
        }

        std::vector<float> timeSample;
        std::vector<const char**> timeSampleData;
        std::vector< std::vector<const char*> > timeSampleStrings;
        size_t timeSampleDataCount = _data.begin()->second.size();
        for ( dataiterator iter = _data.begin(); iter != _data.end(); iter++)
        {
            // Time samples
            timeSample.push_back( iter->first );

            // String data
            std::vector<const char*> currentStrings;
            std::string* stringData = iter->second.data();
            for ( size_t i = 0; i < iter->second.size(); ++i )
            {
                currentStrings.push_back( stringData[i].c_str() );
            }
            timeSampleStrings.push_back( currentStrings );

            // Time-sampled data must all be of the same size
            if ( timeSampleDataCount != iter->second.size() )
            {
                throw std::runtime_error("Error building attribute time samples are different sizes.");
            }
        }

        for ( std::vector< std::vector<const char*> >::iterator iter = timeSampleStrings.begin(); iter != timeSampleStrings.end(); iter++)
        {
            timeSampleData.push_back( iter->data() );
        }
        return StringAttribute(timeSample.data(), (int64_t) timeSample.size(),
                               (const char ***) timeSampleData.data(),
                               (int64_t) timeSampleDataCount, _tupleSize);
    }

}
FNATTRIBUTE_NAMESPACE_EXIT
