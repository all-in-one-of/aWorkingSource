// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <ArbitraryList.h>
#include <iostream>

namespace instance_scene
{
namespace
{
double getIndexedValue(double defaultValue, ChannelRangeRcPtr channels,
                       int channelIndex, int channelOffset, int frame)
{
    if (!channels || channelIndex < 0)
    {
        return defaultValue;
    }

    return channels->getValue(channelIndex+channelOffset, frame);
}
}

///////////////////////////////////////////////////////////////////////////

AttrDataBase::AttrDataBase(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport)
{
}

std::string AttrDataBase::getName()
{
    return m_name;
}

///////////////////////////////////////////////////////////////////////////

AttrDataFloat::AttrDataFloat(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport)
    : AttrDataBase(element, channels, errorReport)
    , m_channels(channels)
    , m_channelIndex(-1)
{
    if( element.empty() )
    {
        errorReport.addErrorMessage("ScenegraphXML: Error parsing XML for float attribute data\n");
        return;
    }

    if( PTreeHelper::hasAttr( element, "name" ) )
    {
        m_name = PTreeHelper::getAttr( element, "name" );
    }

    if( PTreeHelper::hasAttr( element, "value" ) )
    {
        m_value = atof(PTreeHelper::getAttr( element, "value" ).c_str());
    }

    if( PTreeHelper::hasAttr( element, "channelIndex" ) )
    {
        m_channelIndex = atoi(PTreeHelper::getAttr( element, "channelIndex" ).c_str());
    }
}

AttrDataFloatRcPtr AttrDataFloat::getAs(AttrDataBaseRcPtr p)
{
    return boost::dynamic_pointer_cast<AttrDataFloat>(p);
}

double AttrDataFloat::getValue(int frame)
{
    return getIndexedValue(m_value, m_channels, m_channelIndex, 0, frame);
}

bool AttrDataFloat::isAnimated()
{
    return (m_channels && m_channelIndex >= 0);
}

///////////////////////////////////////////////////////////////////////////

AttrDataFloatList::AttrDataFloatList(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport)
    : AttrDataBase(element, channels, errorReport)
    , m_channels(channels)
    , m_channelIndex(-1)
{
    if( element.empty() )
    {
        errorReport.addErrorMessage("ScenegraphXML: Error parsing XML for floatList attribute data\n");
        return;
    }

    if( PTreeHelper::hasAttr( element, "name" ) )
    {
        m_name = PTreeHelper::getAttr( element, "name" );
    }

    if( PTreeHelper::hasAttr( element, "value" ) )
    {
        std::vector<std::string> tokens;
        PTreeHelper::split(PTreeHelper::getAttr( element, "value" ), tokens);

        m_numValues = tokens.size();

        if (m_numValues > 0)
        {
            m_values.reserve(m_numValues);

            for (size_t i = 0; i < m_numValues; ++i)
            {
                m_values.push_back(atof(tokens[i].c_str()));
            }
        }
    }

    if( PTreeHelper::hasAttr( element, "channelIndex" ) )
    {
        m_channelIndex = atoi(PTreeHelper::getAttr( element, "channelIndex" ).c_str());

        if( PTreeHelper::hasAttr( element, "numValues" ) )
        {
            m_numValues = atoi(PTreeHelper::getAttr( element, "numValues" ).c_str());
        }
        else
        {
            errorReport.addErrorMessage("Error reading XML data: numChannels not declared for animating floatList attribute data\n");
        }
    }
}

AttrDataFloatListRcPtr AttrDataFloatList::getAs(AttrDataBaseRcPtr p)
{
    return boost::dynamic_pointer_cast<AttrDataFloatList>(p);
}

void AttrDataFloatList::getValues(int frame, std::vector<double> & values)
{
    values.clear();

    if (!m_channels || m_channelIndex < 0)
    {
        if (m_values.size() > 0)
        {
            values.reserve(m_values.size());
            values = m_values;
        }
    }
    else
    {
        if (m_numValues > 0)
        {
            values.reserve(m_numValues);
            
            for (size_t i = 0; i < m_numValues; ++i)
            {
                values.push_back(
                            m_channels->getValue(m_channelIndex+i, frame));
            }
        }
    }
}

size_t AttrDataFloatList::getNumValues()
{
    return m_numValues;
}

bool AttrDataFloatList::isAnimated()
{
    return (m_channels && m_channelIndex >= 0);
}

///////////////////////////////////////////////////////////////////////////

AttrDataString::AttrDataString(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport)
    : AttrDataBase(element, channels, errorReport)
{
    if( element.empty() )
    {
        errorReport.addErrorMessage("ScenegraphXML: Error parsing XML for string attribute data\n");
        return;
    }

    if( PTreeHelper::hasAttr( element, "name" ) )
    {
        m_name = PTreeHelper::getAttr( element, "name" );
    }

    if( PTreeHelper::hasAttr( element, "value" ) )
    {
        m_value = PTreeHelper::getAttr( element, "value" );
    }
}

AttrDataStringRcPtr AttrDataString::getAs(AttrDataBaseRcPtr p)
{
    return boost::dynamic_pointer_cast<AttrDataString>(p);
}

std::string AttrDataString::getValue()
{
    return m_value;
}

///////////////////////////////////////////////////////////////////////////

ArbitraryList::ArbitraryList(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport)
{
    m_attrDataList.clear();

    if( element.empty() )
    {
        errorReport.addErrorMessage("ScenegraphXML: Error parsing XML for string arbitrary attribute data\n");
        return;
    }

    unsigned int numChildren = element.size();

    m_attrDataList.reserve(numChildren);

    for ( ptree::const_iterator iter = element.begin(); iter != element.end(); ++iter )
    {
        std::string const & type = (*iter).first;
        ptree const & child = (*iter).second;

        if( type == "attribute" )
        {
            if( !PTreeHelper::hasAttr( child, "type" ) )
            {
                errorReport.addErrorMessage("ScenegraphXML: Error parsing XML: attribute data does not declare type\n");
                continue;
            }

            AttrDataBaseRcPtr newAttrData;

            std::string dataType = PTreeHelper::getAttr( child, "type" );

            if (dataType == "float")
            {
                newAttrData = AttrDataBaseRcPtr(
                            new AttrDataFloat(child, channels, errorReport));
            }
            else if (dataType == "floatList")
            {
                newAttrData = AttrDataBaseRcPtr(
                            new AttrDataFloatList(child, channels, errorReport));
            }
            else if (dataType == "string")
            {
                newAttrData = AttrDataBaseRcPtr(
                            new AttrDataString(child, channels, errorReport));
            }
            else
            {
                errorReport.addErrorMessage("ScenegraphXML: Error reading XML data: attribute data with unsupported type\n");
            }

            if (newAttrData)
            {
                m_attrDataList.push_back(newAttrData);
            }
        }
    }
}

size_t ArbitraryList::getNumberOfAttrs()
{
    return m_attrDataList.size();
}

AttrDataBaseRcPtr ArbitraryList::getAttrData(size_t index)
{
    if (index < m_attrDataList.size())
    {
        return m_attrDataList[index];
    }

    return AttrDataBaseRcPtr();
}
}
