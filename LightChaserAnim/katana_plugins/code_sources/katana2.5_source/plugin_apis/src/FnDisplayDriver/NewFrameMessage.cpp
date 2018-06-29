// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnDisplayDriver/NewFrameMessage.h"

#include <sstream>

Foundry::Katana::NewFrameMessage::NewFrameMessage(float frameTime,
                                                  uint32_t height,
                                                  uint32_t width,
                                                  int xorigin,
                                                  int yorigin)
: _frameName()
{
  _handle = getSuite()->createNewFrameMessage(frameTime, height, width,
    xorigin, yorigin);
}

int Foundry::Katana::NewFrameMessage::setFrameName(const std::string &name)
{
  return getSuite()->setFrameName(_handle, name.c_str());
}

void Foundry::Katana::NewFrameMessage::setFrameTime(float time)
{
  getSuite()->setFrameTime(_handle, time);
}

void Foundry::Katana::NewFrameMessage::setFrameDimensions(uint32_t width,
  uint32_t height)
{
  getSuite()->setFrameDimensions(_handle, width, height);
}

void Foundry::Katana::NewFrameMessage::setFrameOrigin(int ox, int oy)
{
  getSuite()->setFrameOrigin(_handle, ox, oy);
}

const uint8_t* Foundry::Katana::NewFrameMessage::frameUUID() const
{
  return getSuite()->frameUUID(_handle);
}

uint32_t Foundry::Katana::NewFrameMessage::frameHeight() const
{
  return getSuite()->frameHeight(_handle);
}

uint32_t Foundry::Katana::NewFrameMessage::frameWidth() const
{
  return getSuite()->frameWidth(_handle);
}

int Foundry::Katana::NewFrameMessage::frameXOrigin() const
{
  return getSuite()->frameXOrigin(_handle);
}

int Foundry::Katana::NewFrameMessage::frameYOrigin() const
{
  return getSuite()->frameYOrigin(_handle);
}

float Foundry::Katana::NewFrameMessage::frameTime() const
{
  return getSuite()->frameTime(_handle);
}

const std::string & Foundry::Katana::NewFrameMessage::frameName() const
{
  const char *frameName = getSuite()->frameName(_handle);
  if (frameName)
  {
    _frameName = frameName;
  }
  else
  {
    _frameName.clear();
  }
  return _frameName;
}

uint32_t Foundry::Katana::NewFrameMessage::frameNameLength() const
{
  return getSuite()->frameNameLength(_handle);
}

void Foundry::Katana::encodeLegacyName(const std::string &legacyFrameName,
                                       int legacyFrameNumber,
                                       std::string &buffer)
{
  // Encode in the JSON format using the following object format
  // framenameEncoding {
  //    id : <id number>,
  //  name : <frame name>
  // }
  std::stringstream ss;
  ss << "{ \"framenameEncoding\" : { \"id\" : \"" << legacyFrameNumber;
  ss << "\", \"name\" : \"" << legacyFrameName << "\" } }";
  buffer = ss.str();
}
