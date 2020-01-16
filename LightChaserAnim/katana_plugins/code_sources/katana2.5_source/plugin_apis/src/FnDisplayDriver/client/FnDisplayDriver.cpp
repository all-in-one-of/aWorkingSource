// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <FnDisplayDriver/FnKatanaDisplayDriver.h>

#include <string>
#include <iostream>

#ifdef _WIN32
#include <FnPlatform/Windows.h>
#else
#include <dlfcn.h>
#endif // _WIN32

#include <FnPluginSystem/FnPluginSystem.h>

namespace { // anonymous
  std::string getLastError()
  {
    std::string error;
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

    error = messageBuffer ? messageBuffer : "";
    if (messageBuffer != NULL)
      LocalFree(messageBuffer);
#else
    char *lastError = dlerror();
    if (lastError != NULL)
      error = lastError;
#endif // _WIN32
    return error;
  }
} // namespace anonymous

FNDISPLAYDRIVER_NAMESPACE_ENTER
{
namespace FnDisplayDriver
{

  // --- Bootstrap -----------------------------------------------------------
  bool Bootstrap(const std::string& katanaPath)
  {
    // Open the FnDisplayDriverHost library
    std::string hostLibPath = katanaPath;
#ifdef _WIN32
    hostLibPath += "\\bin\\FnDisplayDriverHost.dll";
    HINSTANCE dsoptr = LoadLibraryA(hostLibPath.c_str());
#else
    hostLibPath += "/bin/libFnDisplayDriverHost.so";
    void *dsoptr = dlopen(hostLibPath.c_str(), RTLD_NOW);
#endif // _WIN32

    if (!dsoptr)
    {
      std::cerr << "Error loading FnDisplayDriverHost: " << getLastError() << std::endl;
      return false;
    }

    // Get the getDisplayDriverHost() function from FnDisplayDriverHost
#ifdef _WIN32
    void *funcptr = GetProcAddress(dsoptr, "getDisplayDriverHost");
#else
    void *funcptr = dlsym(dsoptr, "getDisplayDriverHost");
#endif // _WIN32

    if (!funcptr)
    {
      std::cerr << "Error loading Display Driver: " << getLastError() << std::endl;
#ifdef _WIN32
      FreeLibrary(dsoptr);
#else
      dlclose(dsoptr);
#endif // _WIN32
      return false;
    }

    typedef FnPluginHost* (*GetHostFunc)(void);
    GetHostFunc getDisplayDriverHostFunc = (GetHostFunc)funcptr;

    // Get the host.
    FnPluginHost *host = getDisplayDriverHostFunc();

    // Inject the host (and consequently the suites) into the C++ plugin
    // wrappers.
    KatanaPipe::setHost(host);
    Message::setHost(host);

    return true;
  }

  void Initialize(const FnDisplayDriverHostSuite_v1 *hostSuite)
  {
    KatanaPipe::setSuite(hostSuite);
    Message::setSuite(hostSuite);
  }

} // namespace FnDisplayDriver
}
FNDISPLAYDRIVER_NAMESPACE_EXIT
