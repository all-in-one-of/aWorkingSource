// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include "KatanaAttrFileReader.h"
#include "PTreeHelper.h"
#include <iostream>


extern "C" FileReader* createAttrFileReader() {
    return new KatanaAttrFileReader();
}

AttrData KatanaAttrFileReader::read(std::string filepath)
{
    AttrData data;

    if(filepath == "")
        return data;

    // Parse the file
    PTreeHelper parser;

    ptree root;
    try {
        root = parser.parse(filepath);
    } catch (const std::exception &) {
        std::cerr << "ERROR: failed parsing XML file '" << filepath << "' for attributes data\n";
    }

    if (!root.empty()) {
        // if it parsed correctly, then iterate through the attributesList tags
        for(ptree::const_iterator ci=root.begin(); ci!=root.end(); ci++) {
            std::string const & type = (*ci).first;
            ptree attributesList = (*ci).second;
            if (type != "attributeList")
                // here, if type is "<xmlattr>", it's just the XML attribute
                // container, which is OK
                continue; //TODO: error message?

            std::string location = PTreeHelper::getAttr(attributesList, "location");
            AttrDataEntries attrDataEntries;

            for(ptree::const_iterator cj=attributesList.begin(); cj != attributesList.end(); ++cj) 
            {
                std::string const & type2 = (*cj).first;
                ptree attribute = (*cj).second;
                if (type2 != "attribute") 
                    continue; //TODO: error message if not <xmlattr>?

                std::string name = PTreeHelper::getAttr(attribute, "name");
                std::string type = PTreeHelper::getAttr(attribute, "type");
                std::string value = PTreeHelper::getAttr(attribute, "value");

                attrDataEntries[name] = value;
            }

            data[location] = attrDataEntries;
        }
    }

    return data;
}
