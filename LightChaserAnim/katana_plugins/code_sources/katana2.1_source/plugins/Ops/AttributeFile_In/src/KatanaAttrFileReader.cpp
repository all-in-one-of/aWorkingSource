// Copyright (c) 2014 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <sstream>
#include <stdexcept>

#include "KatanaAttrFileReader.h"
#include "tinyxml.h"

extern "C" FileReader* createAttrFileReader()
{
    return new KatanaAttrFileReader();
}

AttrData KatanaAttrFileReader::read(std::string filename)
{
    AttrData data;

    if (filename == "")
        return data;

    // Use TinyXML to parse the file.
    TiXmlDocument doc(filename);
    if (!doc.LoadFile())
    {
        std::ostringstream os;
        os << "Error parsing XML file \"" << filename << "\".";
        throw std::runtime_error(os.str().c_str());
    }

    TiXmlElement* rootElement = doc.RootElement();

    // Iterate over the child elements, looking for (and skipping anything
    // that isn't) attributeList entries
    for (TiXmlElement* childElement = rootElement->FirstChildElement();
         childElement; childElement = childElement->NextSiblingElement())
    {
        // Make sure that the entry is of type "attributeList"
        const std::string &elementType = childElement->ValueStr();
        if (elementType != "attributeList")
        {
            continue;
        }

        // Get the location name specified on this attributeList entry
        std::string location;
        if (childElement->QueryStringAttribute("location", &location)
                != TIXML_SUCCESS)
        {
            continue;
        }

        // Iterate over the children inside this attributeList, for each
        // one extracting the name and value fields, if they're found
        AttrDataEntries attrDataEntries;
        TiXmlElement* attributeElement;
        for (attributeElement = childElement->FirstChildElement();
             attributeElement;
             attributeElement = attributeElement->NextSiblingElement())
        {
            if (attributeElement->ValueStr() != "attribute")
            {
                // Wrong type - skip
                continue;
            }

            std::string name;
            std::string value;

            if (attributeElement->QueryStringAttribute("name", &name)
                    != TIXML_SUCCESS)
            {
                continue;
            }
            if (attributeElement->QueryStringAttribute("value", &value)
                    != TIXML_SUCCESS)
            {
                continue;
            }

            // Add this attribute to the map
            attrDataEntries[name] = value;
        }

        // Add the attribute map to the location map
        data[location] = attrDataEntries;
    }

    return data;
}

