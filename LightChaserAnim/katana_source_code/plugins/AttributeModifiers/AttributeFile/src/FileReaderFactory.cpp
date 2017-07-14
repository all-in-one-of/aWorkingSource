// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FileReaderFactory.h"
#include "KatanaAttrFileReader.h"

#include <iostream>
#ifdef _WIN32
#include "windowshelpers.h"
#endif
#include <dlfcn.h>

FileReaderFactory::FileReaderFactory() { }

FileReaderFactory::~FileReaderFactory() { }


FileReader* FileReaderFactory::get(std::string soFilepath)
{
    // If the so file is not specify, then use the default FileReader
    if (soFilepath == "")
        return getDefault();

    //otherwise load it with ldopen
    void* fileReaderSo = dlopen(soFilepath.c_str(), RTLD_LAZY);
    if (!fileReaderSo) {
        std::cerr << "  * Cannot load attributes FileReader lib: " << dlerror() << std::endl;
        return 0;
    }

    // reset errors
    dlerror();

     // load the "createAttrFileReader" symbol
    createAttrFileReader_t* create;
    create = (createAttrFileReader_t*) dlsym(fileReaderSo, "createAttrFileReader");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "  * Cannot load attributes FileReader lib: " << dlsym_error << std::endl;
        return 0;
    }

    // Instantiate the FileReader
    FileReader* fileReader = create();

    return fileReader;
}


FileReader* FileReaderFactory::getDefault()
{
    return (FileReader*) new KatanaAttrFileReader();
}
