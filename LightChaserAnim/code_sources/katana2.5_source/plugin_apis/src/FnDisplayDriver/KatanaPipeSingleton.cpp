// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnDisplayDriver/KatanaPipeSingleton.h"

#include <cstdlib>
#include <string>

#include <FnDisplayDriver/FnDisplayDriver.h>

// Initialise the pipe singleton...
Foundry::Katana::KatanaPipe* Foundry::Katana::PipeSingleton::_pipe = NULL;

Foundry::Katana::KatanaPipe* Foundry::Katana::PipeSingleton::Instance(
  const std::string &hostName, unsigned int portNumber,
  const std::string& katanaPath)
{
#if !defined(_WIN32)
  char *env;
#else
  size_t returnValue = 0;
  char env[4096];
#endif  // _WIN32

  if(!_pipe)
  {
    // Bootstrap Display Driver host
    std::string path(katanaPath);
    if (path.empty())
    {
#if !defined(_WIN32)
      env = ::getenv("KATANA_ROOT");
      if (env != NULL)
#else
      ::getenv_s(&returnValue, env, "KATANA_ROOT");
      if (returnValue != 0)
#endif  // _WIN32
      {
        path = env;
      }
      else
      {
        return _pipe;
      }
    }
    if (!FNDISPLAYDRIVER_NAMESPACE::FnDisplayDriver::Bootstrap(path))
    {
      return _pipe;
    }

    // Initialize KatanaPipe instance
    size_t maxQueueSize = 0;

#if !defined(_WIN32)
    env = ::getenv("KATANA_PIPE_MAX_QUEUE_SIZE");
    if (env != NULL)
#else
    returnValue = 0;
    ::getenv_s(&returnValue, env, "KATANA_PIPE_MAX_QUEUE_SIZE");
    if (returnValue != 0)
#endif  // _WIN32
    {
      maxQueueSize = static_cast<size_t>(strtoul(env, NULL, 0));
    }

    _pipe = new Foundry::Katana::KatanaPipe(hostName, portNumber, maxQueueSize);
  }

  return _pipe;
}
