// Copyright (c) 2014 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifdef _WIN32
#include <FnPlatform/Windows.h>
#else
#include <dlfcn.h>
#endif // _WIN32

#include <stdexcept>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "FileReaderFactory.h"
#include "KatanaAttrFileReader.h"

// RAII wrapper to handle dlclose-ing loaded dynamic libraries.
class DynLibWrapper
{
public:
    DynLibWrapper(const std::string &sofile)
    {
#ifdef _WIN32
        m_lib = LoadLibraryA(sofile.c_str());
#else
        m_lib = dlopen(sofile.c_str(), RTLD_LAZY);
#endif // _WIN32
    }
    ~DynLibWrapper()
    {
        if (m_lib)
        {
#ifdef _WIN32
            FreeLibrary(m_lib);
#else
            dlclose(m_lib);
#endif // _WIN32
        }
    }

    void* get() {return m_lib;}
    void* getSymbol(const std::string &symbol)
    {
#ifdef _WIN32
        return GetProcAddress(m_lib, symbol.c_str());
#else
        return dlsym(m_lib, symbol.c_str());
#endif // _WIN32
    }
    const char* getError()
    {
#ifdef _WIN32
        char* messageBuffer = NULL;
        DWORD err = GetLastError();

        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                       FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL,
                       err,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       reinterpret_cast<LPSTR>(&messageBuffer),
                       0, NULL);

        m_error = messageBuffer ? messageBuffer : "";
        if (messageBuffer != NULL)
            LocalFree(messageBuffer);
#else
        char *lastError = dlerror();
        if (lastError != NULL)
            m_error = lastError;
#endif // _WIN32
        return m_error.c_str();
    }

private:
#ifdef _WIN32
    HINSTANCE m_lib;
#else
    void* m_lib;
#endif // _WIN32
    std::string m_error;

};
typedef boost::shared_ptr<DynLibWrapper> DynLibWrapperPtr;

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

