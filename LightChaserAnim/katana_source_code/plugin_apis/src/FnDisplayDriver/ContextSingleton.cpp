// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnDisplayDriver/ContextSingleton.h"

#include <cassert>

void* Foundry::Katana::ContextSingleton::_context = NULL;

void* Foundry::Katana::ContextSingleton::Instance()
{
  if(!_context)
  {
    // No need to check for errors returned by zmq_ctx_new() as it doesn't
    // return any.
    Foundry::Katana::ContextSingleton::_context = zmq_ctx_new();
    assert(_context);
  }

  return _context;
}
