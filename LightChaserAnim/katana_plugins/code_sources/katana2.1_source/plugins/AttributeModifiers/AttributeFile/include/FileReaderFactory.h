// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef _FileREADERFACTORY_H
#define _FileREADERFACTORY_H

#include "FileReader.h"

class FileReaderFactory
{
public:
    FileReaderFactory();
    virtual ~FileReaderFactory();
    FileReader* get(std::string soFilepath);
    FileReader* getDefault();

private:
    FileReader* m_default;
};

#endif

