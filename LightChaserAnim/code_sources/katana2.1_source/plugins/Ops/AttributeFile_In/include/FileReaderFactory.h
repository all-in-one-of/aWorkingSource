// Copyright (c) 2014 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FILEREADERFACTORY_H
#define FILEREADERFACTORY_H

#include <tr1/memory>
#include "FileReader.h"

typedef std::tr1::shared_ptr<FileReader> FileReaderPtr;

class FileReaderFactory
{
public:
    FileReaderFactory() {};
    virtual ~FileReaderFactory() {};
    FileReaderPtr get(std::string soFilename);
    FileReaderPtr getDefault();
};

#endif

