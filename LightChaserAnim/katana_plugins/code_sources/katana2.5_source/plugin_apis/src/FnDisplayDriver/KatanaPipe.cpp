// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <iostream>

#include "FnDisplayDriver/KatanaPipe.h"
#include "FnDisplayDriver/CloseMessage.h"
#include "FnDisplayDriver/EndOfRenderMessage.h"

const FnDisplayDriverHostSuite_v1 *Foundry::Katana::KatanaPipe::_suite = NULL;

Foundry::Katana::KatanaPipe::KatanaPipe(const std::string &hostName,
                                        unsigned int portNumber,
                                        size_t maxQueueSize)
: _handle(NULL)
{
  _handle = getSuite()->createKatanaPipe(hostName.c_str(), (uint16_t)portNumber,
    (uint64_t)maxQueueSize);
}

Foundry::Katana::KatanaPipe::~KatanaPipe()
{
  getSuite()->terminate(_handle);
}

int Foundry::Katana::KatanaPipe::connect()
{
  return getSuite()->connect(_handle);
}

int Foundry::Katana::KatanaPipe::send(const Message &message)
{
  return getSuite()->send(_handle, message._handle);
}

void Foundry::Katana::KatanaPipe::flushPipe(const NewChannelMessage &channel)
{
  // Send out a close message to let Katana clean up.
  CloseMessage closeMsg(channel);
  getSuite()->flushPipe(_handle, closeMsg._handle);
}

void Foundry::Katana::KatanaPipe::closeChannel(const NewChannelMessage &channel)
{
  // Send out a close message to let Katana clean up.
  EndOfRenderMessage eorMessage(channel);
  getSuite()->closeChannel(_handle, eorMessage._handle);
}

FnPlugStatus Foundry::Katana::KatanaPipe::setHost(FnPluginHost *host)
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

void Foundry::Katana::KatanaPipe::setSuite(
    const FnDisplayDriverHostSuite_v1 *suite)
{
  if (suite)
  {
    _suite = suite;
  }
}

const FnDisplayDriverHostSuite_v1 *Foundry::Katana::KatanaPipe::getSuite()
{
  if (!_suite)
  {
    std::cerr
      << "Error using an invalid Display Driver communication pipe suite."
      << std::endl;
  }
  return _suite;
}
