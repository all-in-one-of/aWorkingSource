// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnDisplayDriver/Message.h"

#include <iostream>

const FnDisplayDriverHostSuite_v1 *Foundry::Katana::Message::_suite = 0;

Foundry::Katana::Message::~Message()
{
  if (_suite && _handle)
  {
    _suite->releaseMessage(_handle);
  }
}

FnPlugStatus Foundry::Katana::Message::setHost(FnPluginHost *host)
{
  if (host)
  {
    _suite = reinterpret_cast<const FnDisplayDriverHostSuite_v1*>(
      host->getSuite("DisplayDriverHost", 1));
    if (_suite)
    {
      return FnPluginStatusOK;
    }
  }
  return FnPluginStatusError;
}

void Foundry::Katana::Message::setSuite(
  const FnDisplayDriverHostSuite_v1 *suite)
{
  if (suite)
  {
    _suite = suite;
  }
}

const FnDisplayDriverHostSuite_v1 *Foundry::Katana::Message::getSuite()
{
  if (!_suite)
  {
    std::cerr << "Error using an invalid Display Driver message suite."
              << std::endl;
  }
  return _suite;
}
