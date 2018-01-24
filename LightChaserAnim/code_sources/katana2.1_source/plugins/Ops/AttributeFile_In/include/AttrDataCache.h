// Copyright (c) 2014 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef ATTRIBUTEDATACACHE_H
#define ATTRIBUTEDATACACHE_H

#include <sstream>
#include <tr1/memory>

#include <FnGeolib/util/AttributeKeyedCache.h>

#include <FileReader.h>
#include <FileReaderFactory.h>

// This cache maps GroupAttributes containing two StringAttribute children
// (named 'filename' and 'soFilename') to AttrData instances, and performs
// queries (get()) in a thread-safe manner.
class AttrDataCache: public FnGeolibUtil::AttributeKeyedCache<AttrData>
{
public:
    typedef std::tr1::shared_ptr<AttrDataCache> Ptr;

    AttrDataCache() :
        FnGeolibUtil::AttributeKeyedCache<AttrData>(1000, 1000)
    {
    }

private:
    AttrDataCache::IMPLPtr createValue(const FnAttribute::Attribute & iAttr)
    {
        // createValue() is called when there is no previously cached data
        // associated with the given iAttr attribute. Deconstruct the iAttr
        // into filename/soFilename strings, and use the FileReaderFactory
        // to load the attribute data from disk.

        FnAttribute::GroupAttribute groupAttr(iAttr);
        if (!groupAttr.isValid())
        {
            return AttrDataCache::IMPLPtr();
        }

        const std::string filename =
            FnAttribute::StringAttribute(
                groupAttr.getChildByName("filename")).getValue("", false);

        const std::string soFilename =
            FnAttribute::StringAttribute(
                groupAttr.getChildByName("soFilename")).getValue("", false);

        if (filename.empty())
        {
            return AttrDataCache::IMPLPtr();
        }

        // Parse it and add it to the cache
        FileReaderFactory factory;
        if (FileReaderPtr reader = factory.get(soFilename))
        {
            // Ask the reader plugin to parse the file and return
            // the attribute data
            const AttrData attrData = reader->read(filename);
            return AttrDataCache::IMPLPtr(new AttrData(attrData));
        }
        else
        {
            std::ostringstream os;
            os << "Error loading plug-in \"" << soFilename << "\".";
            throw std::runtime_error(os.str().c_str());
        }
    }
};

#endif
