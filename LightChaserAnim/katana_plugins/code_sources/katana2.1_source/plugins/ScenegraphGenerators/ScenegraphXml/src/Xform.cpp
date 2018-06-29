// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <Xform.h>



namespace instance_scene
{
    Xform::Xform(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport)
    : m_channels(channels)
    , m_channelIndex(-1)
    {
        if( element.empty() )
        {
            errorReport.addErrorMessage("ScenegraphXML: error reading XML element for Xform\n");
            return;
        }
        
        if( PTreeHelper::hasAttr( element, "value" ) )
        {
            std::vector<std::string> tokens;
            PTreeHelper::split(PTreeHelper::getAttr( element, "value" ), tokens);

            if (tokens.size() == 16)
            {
                m_values.reserve(16);
                
                for (size_t i = 0; i < 16; ++i)
                {
                    m_values.push_back(atof(tokens[i].c_str()));
                }
            }
            else
            {
                m_values.resize(16);    // NOTE: To avoid Katana crashing
            }
        }
        
        if( PTreeHelper::hasAttr( element, "channelIndex" ) )
        {
            m_channelIndex = atoi(PTreeHelper::getAttr( element, "channelIndex" ).c_str());
        }
    }
    
    
    void Xform::getValues(int frame, std::vector<double> & values)
    {
        values.clear();
        
        if (!m_channels || m_channelIndex < 0 )
        {
            if (m_values.size() == 16)
            {
                values.reserve(16);
                values = m_values;
            }
        }
        else
        {
            values.reserve(16);
            
            for (size_t i = 0; i < 16; ++i)
            {
                values.push_back(
                        m_channels->getValue(m_channelIndex+i, frame));
            }
        }
    }
    
    bool Xform::isAnimated()
    {
        return (m_channels && m_channelIndex >= 0);
    }
    
}
