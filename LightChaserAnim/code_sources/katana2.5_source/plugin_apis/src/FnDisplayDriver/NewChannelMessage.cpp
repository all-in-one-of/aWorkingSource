// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <string>

#include "FnDisplayDriver/NewChannelMessage.h"
#include "FnDisplayDriver/NewFrameMessage.h"
#include <FnDisplayDriver/suite/FnDisplayDriverSuite.h>

Foundry::Katana::NewChannelMessage::NewChannelMessage(const uint8_t *frameID,
                                                      uint16_t channelID,
                                                      uint32_t height,
                                                      uint32_t width,
                                                      int xorigin,
                                                      int yorigin,
                                                      float sampleRate1,
                                                      float sampleRate2,
                                                      uint32_t dataSize)
: _channelName()
{
  _handle = getSuite()->createNewChannelMessage(frameID, channelID,
    height, width, xorigin, yorigin, sampleRate1, sampleRate2, dataSize);
}

Foundry::Katana::NewChannelMessage::NewChannelMessage(
  const NewFrameMessage &frame,
  uint16_t channelID,
  uint32_t height,
  uint32_t width,
  int xorigin,
  int yorigin,
  float sampleRate1,
  float sampleRate2,
  uint32_t dataSize)
: _channelName()
{
  _handle = getSuite()->createNewChannelMessage(frame.frameUUID(), channelID,
    height, width, xorigin, yorigin, sampleRate1, sampleRate2, dataSize);
}

int Foundry::Katana::NewChannelMessage::setChannelName(const std::string &name)
{
  return getSuite()->setChannelName(_handle, name.c_str());
}

void Foundry::Katana::NewChannelMessage::setChannelDimensions(uint32_t width,
  uint32_t height)
{
  getSuite()->setChannelDimensions(_handle, width, height);
}

void Foundry::Katana::NewChannelMessage::setChannelOrigin(int ox, int oy)
{
  getSuite()->setChannelOrigin(_handle, ox, oy);
}

void Foundry::Katana::NewChannelMessage::setDataSize(uint32_t dataSize)
{
  getSuite()->setDataSize(_handle, dataSize);
}

void Foundry::Katana::NewChannelMessage::setChannelID(uint16_t channelID)
{
  getSuite()->setChannelID(_handle, channelID);
}

void Foundry::Katana::NewChannelMessage::setSampleRate(const float *sampleRate)
{
  getSuite()->setSampleRate(_handle, sampleRate);
}

const uint8_t* Foundry::Katana::NewChannelMessage::frameUUID() const
{
  return getSuite()->channelFrameUUID(_handle);
}

uint16_t Foundry::Katana::NewChannelMessage::channelID() const
{
  return getSuite()->channelID(_handle);
}

uint32_t Foundry::Katana::NewChannelMessage::channelHeight() const
{
  return getSuite()->channelHeight(_handle);
}

uint32_t Foundry::Katana::NewChannelMessage::channelWidth() const
{
  return getSuite()->channelWidth(_handle);
}

int Foundry::Katana::NewChannelMessage::channelXOrigin() const
{
  return getSuite()->channelXOrigin(_handle);
}

int Foundry::Katana::NewChannelMessage::channelYOrigin() const
{
  return getSuite()->channelYOrigin(_handle);
}

uint32_t Foundry::Katana::NewChannelMessage::channelDataSize() const
{
  return getSuite()->channelDataSize(_handle);
}

const std::string & Foundry::Katana::NewChannelMessage::channelName() const
{
  const char *channelName = getSuite()->channelName(_handle);
  if (channelName)
  {
    _channelName = channelName;
  }
  else
  {
    _channelName.clear();
  }
  return _channelName;
}

uint32_t Foundry::Katana::NewChannelMessage::channelNameLength() const
{
  return getSuite()->channelNameLength(_handle);
}

const float *Foundry::Katana::NewChannelMessage::sampleRate() const
{
  return getSuite()->sampleRate(_handle);
}
