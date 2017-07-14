// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef _FileREADER_H
#define _FileREADER_H

#include <map>
#include <string>

/**
 * The data structure that contains the info about all the attributes
 * and their node names that will have to match the ones in the
 * SceneGraph. AttrDataEntries corresponds to a map of attribute
 * name/value pair. AttrData's key is the name of the node to be
 * matched and the value is a AttrDataEntries map.
 **/
typedef std::string AttrDataEntry;
typedef std::map<std::string, AttrDataEntry> AttrDataEntries;
typedef std::map<std::string, AttrDataEntries> AttrData;


/**
 * The abstract class who's children are responsible for parsing
 * an File file with Attribute information and transforms it into
 * an AttrData.
 */
class FileReader
{
public:
    virtual ~FileReader(){}
    virtual AttrData read(std::string filepath) = 0;
};

// The signature of the factory functions that is called by ldopen.
// This should instantiate a FileReader in the heap. It is the user's
// responsibility to delete it afterwards.
typedef FileReader* createAttrFileReader_t();


#endif

