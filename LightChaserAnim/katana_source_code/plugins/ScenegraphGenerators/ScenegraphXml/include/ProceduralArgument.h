// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#ifndef PROCEDURALARGUMENT_H_
#define PROCEDURALARGUMENT_H_

#include "Manifest.h"
#include "ErrorReport.h"
#include "Channels.h"

#include <FnAttribute/FnAttribute.h>


namespace instance_scene
{
    class ProceduralArgument
    {
        public:
            ProceduralArgument(ptree element, ChannelRangeRcPtr channels, ErrorReport &errorReport);

            std::string getName();
            FnKat::Attribute getValue();

        private:
            std::string name;
            std::string value;
            std::string type;
            int tupleSize;

            ChannelRangeRcPtr m_channels;
            int m_channelIndex;
    };

    typedef boost::shared_ptr<ProceduralArgument> ProceduralArgumentRcPtr;
}

#endif /* PROCEDURALARGUMENT_H_ */
