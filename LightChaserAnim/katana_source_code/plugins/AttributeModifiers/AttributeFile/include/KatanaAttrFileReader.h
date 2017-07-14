// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef INCLUDED_GEOLIB_ATTRIBUTES_File_KATANAREADER_H
#define INCLUDED_GEOLIB_ATTRIBUTES_File_KATANAREADER_H

#include "FileReader.h"

/**
 * An implementation of an FileReader that reads an xml file that follows
 * Katana's Standard for the Attribute Xml File format.
 */
class KatanaAttrFileReader : public FileReader
{
public:
    AttrData read(std::string filepath);
};

#endif

