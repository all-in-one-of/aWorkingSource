// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef INCLUDED_INSTANCE_SCENE_ARBITRARYLIST_H
#define INCLUDED_INSTANCE_SCENE_ARBITRARYLIST_H

#include "Manifest.h"
#include "ErrorReport.h"
#include "Channels.h"

namespace instance_scene
{
    class AttrDataBase
    {
    public:
        AttrDataBase(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport);

        std::string getName();

        //without a vtable, we can't dynamic_cast
        virtual ~AttrDataBase(){}

    protected:
        std::string m_name;
    };

    typedef boost::shared_ptr<AttrDataBase> AttrDataBaseRcPtr;

    ///////////////////////////////////////////////////////////////////////////

    class AttrDataFloat;
    typedef boost::shared_ptr<AttrDataFloat> AttrDataFloatRcPtr;

    class AttrDataFloat : public AttrDataBase
    {
    public:
        AttrDataFloat(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport);

        static AttrDataFloatRcPtr getAs(AttrDataBaseRcPtr p);
        double getValue(int frame);
        bool isAnimated();

    private:
        double m_value;
        ChannelRangeRcPtr m_channels;
        int m_channelIndex;
    };

    ///////////////////////////////////////////////////////////////////////////

    class AttrDataFloatList;
    typedef boost::shared_ptr<AttrDataFloatList> AttrDataFloatListRcPtr;

    class AttrDataFloatList : public AttrDataBase
    {
    public:
        AttrDataFloatList(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport);

        static AttrDataFloatListRcPtr getAs(AttrDataBaseRcPtr p);
        void getValues(int frame, std::vector<double> & values);
        size_t getNumValues();
        bool isAnimated();

    private:
        std::vector<double> m_values;
        size_t m_numValues;
        ChannelRangeRcPtr m_channels;
        int m_channelIndex;
    };


    ///////////////////////////////////////////////////////////////////////////

    class AttrDataString;
    typedef boost::shared_ptr<AttrDataString> AttrDataStringRcPtr;

    class AttrDataString : public AttrDataBase
    {
    public:
        AttrDataString(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport);

        static AttrDataStringRcPtr getAs(AttrDataBaseRcPtr p);
        std::string getValue();
 
    private:
        std::string m_value;
    };

 
    ///////////////////////////////////////////////////////////////////////////

    class ArbitraryList
    {
    public:
        ArbitraryList(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport);

        size_t getNumberOfAttrs();
        AttrDataBaseRcPtr getAttrData(size_t index);

    private:
        std::vector<AttrDataBaseRcPtr> m_attrDataList;
    };

    typedef boost::shared_ptr<ArbitraryList> ArbitraryListRcPtr;
}

#endif
