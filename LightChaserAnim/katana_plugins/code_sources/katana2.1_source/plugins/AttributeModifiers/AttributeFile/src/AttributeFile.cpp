// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.


#include <iostream>
#include <FnAttributeModifier/plugin/FnAttributeModifier.h>
#include "FnAttribute/FnGroupBuilder.h"
#include "FileReaderFactory.h"


typedef std::map<std::string, AttrData> AttrDataCache;



/* AttributeFile is a plugin which reads a file containing attribute values
 * and applies the values to the scenegraph.
 * By default, the file is in a Katana-specific XML format, but a .so file can be 
 * specified to interpret other file formats; this was implemented so that clients
 * could read in their own formats.
 */

class AttributeFile : public FnKat::AttributeModifier
{
public:
    AttributeFile(FnKat::GroupAttribute args);
    virtual ~AttributeFile() {}
    virtual void cook(FnKat::AttributeModifierInput &input);
    virtual std::string getCacheId();

    static FnKat::AttributeModifier *create(FnKat::GroupAttribute args) {
        return new AttributeFile(args);
    }
    static FnKat::GroupAttribute getArgumentTemplate() {
        FnKat::GroupBuilder gb;
        gb.set("filepath",FnKat::StringAttribute(""));
        gb.set("sofilepath",FnKat::StringAttribute(""));
        gb.set("groupAttr",FnKat::StringAttribute(""));
        return gb.build();
    }

    static void flush() {}

private:
    // the path of the file to read
    std::string m_filepath; 
    // the path of an optional .so file for parsing input formats
    std::string m_sofilepath;
    // if filled, the name of a group attribute to create containing
    // all attributes defined in the file, rather than replacing existing
    // ones.
    std::string m_groupAttr;
    // A filename->data map the plugin keeps in memory
    static AttrDataCache *m_attrDataCache;

    void parseFileIfNeeded();
};

// ------------------------------------------------------------------------

AttrDataCache *AttributeFile::m_attrDataCache = 0;

AttributeFile::AttributeFile(FnKat::GroupAttribute args)
    : FnKat::AttributeModifier(args), m_filepath(""), m_sofilepath(""), m_groupAttr("")
{
    if ( args.isValid() ) {
        FnKat::StringAttribute filepath_attr = args.getChildByName("filepath");
        if(filepath_attr.isValid()) {
            FnKat::StringConstVector strvec = filepath_attr.getNearestSample(0.0);
            m_filepath = *(strvec.data());
        }
        FnKat::StringAttribute sofilepath_attr = args.getChildByName("sofilepath");
        if(sofilepath_attr.isValid()) {
            FnKat::StringConstVector strvec = sofilepath_attr.getNearestSample(0.0);
            m_sofilepath = *(strvec.data());
        }
        FnKat::StringAttribute groupattr_attr = args.getChildByName("groupAttr");
        if(groupattr_attr.isValid()) {
            FnKat::StringConstVector strvec = groupattr_attr.getNearestSample(0.0);
            m_groupAttr = *(strvec.data());
        }
    }

    // create the attribute data cache, if needed
    if(!m_attrDataCache)
        m_attrDataCache = new AttrDataCache;


    // parse the file
    parseFileIfNeeded();

}


void
AttributeFile::parseFileIfNeeded() {
    if((m_attrDataCache->find(m_filepath) == m_attrDataCache->end())) {
        /* If it's not cached, parse it and add it to the chache */

        FileReaderFactory factory;
        FileReader *reader = factory.get(m_sofilepath);

        AttrData attrData  = reader->read(m_filepath);

        m_attrDataCache->insert(AttrDataCache::value_type(m_filepath, attrData));

        //TODO: use a refpointer instead?
        if(reader) 
            delete reader;
    }
}


void
AttributeFile::cook(FnKat::AttributeModifierInput &input) {
    // Update the cache if necessary and get the one for this file
    parseFileIfNeeded();

    const AttrDataCache::iterator attrDataCacheIt = m_attrDataCache->find(m_filepath);
    if(attrDataCacheIt != m_attrDataCache->end()) {
        AttrData &attrData = attrDataCacheIt->second;

        // Check if the current location's name exist in attrData's keys
        AttrData::iterator locationIt = attrData.find(input.getName());

        if(locationIt != attrData.end()) {
            // We build a group attribute and add it to the output


            AttrDataEntries attrEntries = locationIt->second;
            AttrDataEntries::iterator attrIt;

            if(m_groupAttr != "") {
                FnKat::GroupBuilder gb;
                for(attrIt = attrEntries.begin(); attrIt != attrEntries.end(); ++attrIt) {
                    gb.set(attrIt->first.c_str(), FnKat::StringAttribute(attrIt->second.c_str()));
                }
                FnKat::GroupAttribute groupAttr = gb.build();
                input.addOverride(m_groupAttr, groupAttr, false);
            } else {
                for(attrIt = attrEntries.begin(); attrIt != attrEntries.end(); ++attrIt) {
                    input.addOverride(attrIt->first, FnKat::StringAttribute(attrIt->second.c_str()), false);
                }
            }

        }
        
    }
}

std::string
AttributeFile::getCacheId() {
   return "AttributeFile";
}


DEFINE_AMP_PLUGIN(AttributeFile)

void registerPlugins()
{
    REGISTER_PLUGIN(AttributeFile, "AttributeFile", 0, 1);
}

