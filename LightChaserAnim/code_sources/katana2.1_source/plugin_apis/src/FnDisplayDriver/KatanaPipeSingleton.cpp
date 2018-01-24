// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnDisplayDriver/KatanaPipeSingleton.h"

// Initialise the pipe singleton...
Foundry::Katana::KatanaPipe* Foundry::Katana::PipeSingleton::_pipe = NULL;

Foundry::Katana::KatanaPipe* Foundry::Katana::PipeSingleton::Instance(const std::string &hostName, unsigned int portNumber)
{
  if(!_pipe)
  {
      size_t maxQueueSize = 0;
      const char * maxQueueSizeString = getenv("KATANA_PIPE_MAX_QUEUE_SIZE");
      if (maxQueueSizeString)
      {
          maxQueueSize = static_cast<size_t>(strtoul(maxQueueSizeString, NULL, 0));
      }

    _pipe = new Foundry::Katana::KatanaPipe(hostName, portNumber, maxQueueSize);
  }

  return _pipe;
}
