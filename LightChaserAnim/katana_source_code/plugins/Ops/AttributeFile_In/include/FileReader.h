// Copyright (c) 2014 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FILEREADER_H
#define FILEREADER_H

#include <map>
#include <string>

// The data structure that contains the info about all the attributes and their
// node names that will have to match the ones in the scene graph.
// AttrDataEntries corresponds to a map of attribute name/value pairs.
// AttrData's key is the name of the node to be matched and the value is an
// AttrDataEntries map.
typedef std::string AttrDataEntry;
typedef std::map<std::string, AttrDataEntry> AttrDataEntries;
typedef std::map<std::string, AttrDataEntries> AttrData;


// The abstract base class for classes that are responsible for parsing a file
// with attribute information and transforming it into an AttrData object.
class FileReader
{
public:
    virtual ~FileReader(){}
    virtual AttrData read(std::string filename) = 0;
};

// The signature of the factory function that is called by dlopen(). The
// function should instantiate a FileReader on the heap. It is the user's
// responsibility to delete it afterwards.
typedef FileReader* createAttrFileReader_t();


#endif

