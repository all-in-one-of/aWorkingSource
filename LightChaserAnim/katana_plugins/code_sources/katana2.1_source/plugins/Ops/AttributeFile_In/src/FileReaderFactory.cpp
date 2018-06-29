// Copyright (c) 2014 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <tr1/memory>
#include <stdexcept>
#include <sstream>
#include <dlfcn.h>

#include "FileReaderFactory.h"
#include "KatanaAttrFileReader.h"

// RAII wrapper to handle dlclose-ing loaded dynamic libraries.
class DynLibWrapper
{
public:
    DynLibWrapper(const std::string &sofile)
    {
        m_lib = dlopen(sofile.c_str(), RTLD_LAZY);
    }
    ~DynLibWrapper()
    {
        if (m_lib)
        {
            dlclose(m_lib);
        }
    }

    void* get() {return m_lib;}
    void* getSymbol(const std::string &symbol)
    {
        return dlsym(m_lib, symbol.c_str());
    }
    const char* getError()
    {
        return dlerror();
    }

private:
    void* m_lib;

};
typedef std::tr1::shared_ptr<DynLibWrapper> DynLibWrapperPtr;

// Deleter class, used to delay dlclose-ing the loaded dynamic library
// until after the FileReader is no longer needed.
class FileReaderDeleter
{
public:
    FileReaderDeleter(DynLibWrapperPtr so) : m_so(so) {};

    void operator()(FileReader* reader)
    {
        if (reader)
        {
            delete reader;
        }
        if (m_so)
        {
            m_so.reset();
        }
    }

private:
    DynLibWrapperPtr m_so;
};

FileReaderPtr FileReaderFactory::get(std::string soFilename)
{
    // If the .so file is not specified, use the default FileReader
    if (soFilename.empty())
    {
        return getDefault();
    }

    DynLibWrapperPtr fileReaderSo =
            DynLibWrapperPtr(new DynLibWrapper(soFilename));

    if (!fileReaderSo->get())
    {
        std::ostringstream os;
        os << "Error loading FileReader library \"" << soFilename << "\": "
           << fileReaderSo->getError();
        throw std::runtime_error(os.str().c_str());
    }

    // Reset errors
    fileReaderSo->getError();

    // Load the "createAttrFileReader" symbol
    createAttrFileReader_t* create;
    create = (createAttrFileReader_t*) fileReaderSo->getSymbol(
        "createAttrFileReader");
    const char* symbolError = fileReaderSo->getError();
    if (symbolError)
    {
        std::ostringstream os;
        os << "Error loading FileReader library \"" << soFilename << "\": "
           << symbolError;
        throw std::runtime_error(os.str().c_str());
    }

    // Instantiate the FileReader through create(), and keep track of it
    // through a shared_ptr. The deleter is used here to defer the dlclose()
    // operation until whenever the FileReaderPtr goes out of scope and is
    // cleaned up.
    FileReaderPtr fileReader(create(), FileReaderDeleter(fileReaderSo));
    return fileReader;
}

FileReaderPtr FileReaderFactory::getDefault()
{
    return FileReaderPtr(new KatanaAttrFileReader());
}

