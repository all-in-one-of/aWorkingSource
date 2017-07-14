// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

/*
 * ProceduralArgument.cpp
 *
 *  Created on: Aug 23, 2011
 *      Author: orn
 */

#include "ProceduralArgument.h"


namespace instance_scene
{
    ProceduralArgument::ProceduralArgument(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport)
    : m_channels(channels)
    , m_channelIndex(-1)
    {
        if( element.empty() )
        {
            errorReport.addErrorMessage("ScenegraphXML: Error reading XML element for ProceduralArgument\n");
            return;
        }

        if( PTreeHelper::hasAttr( element, "name" ) )
            name = PTreeHelper::getAttr( element, "name" );
        else
        {
            errorReport.addErrorMessage("ScenegraphXML: Could not find name for arg (ProceduralArgument)\n");
            return;
        }

        if( PTreeHelper::hasAttr( element, "value" ) )
            value = PTreeHelper::getAttr( element, "value" );

        if( PTreeHelper::hasAttr( element, "type" ) )
            type = PTreeHelper::getAttr( element, "type" );
        else
            type = "string";

        if( PTreeHelper::hasAttr( element, "tupleSize" ) )
            tupleSize = atoi(PTreeHelper::getAttr( element, "tupleSize" ).c_str());
        else
            tupleSize = 1;
    }

    std::string ProceduralArgument::getName()
    {
        return "args." + name;
    }

    FnKat::Attribute ProceduralArgument::getValue()
    {
        if( type == "string" )
        {
            return FnKat::StringAttribute(value);
        }
        else if( type == "double" )
        {
            std::vector<double> m_values;
            size_t m_numValues;

            std::vector<std::string> tokens;
            PTreeHelper::split(value, tokens);

            m_numValues = tokens.size();

            if (m_numValues > 0)
            {
                m_values.reserve(m_numValues);

                for (size_t i = 0; i < m_numValues; ++i)
                {
                    m_values.push_back(atof(tokens[i].c_str()));
                }
            }

            return FnKat::DoubleAttribute(&m_values.front(), m_numValues, tupleSize);
        }

        return FnKat::Attribute();
    }
}
