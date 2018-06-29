// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnDisplayDriver/DataMessage.h"
#include "FnDisplayDriver/NewChannelMessage.h"
#include <FnDisplayDriver/suite/FnDisplayDriverSuite.h>

Foundry::Katana::DataMessage::DataMessage(const NewChannelMessage &channel,
                                          uint32_t xMin,
                                          uint32_t width,
                                          uint32_t yMin,
                                          uint32_t height,
                                          uint32_t byteSkip)
{
  _handle = getSuite()->createDataMessage(channel.frameUUID(),
    channel.channelID(), xMin, width, yMin, height, byteSkip);
}

Foundry::Katana::DataMessage::DataMessage(const uint8_t *frameID,
                                          uint16_t channelID,
                                          uint32_t xMin,
                                          uint32_t width,
                                          uint32_t yMin,
                                          uint32_t height,
                                          uint32_t byteSkip)
{
  _handle = getSuite()->createDataMessage(frameID, channelID, xMin, width, yMin,
    height, byteSkip);
}

void Foundry::Katana::DataMessage::setData(const void *data, uint32_t size)
{
  getSuite()->setData(_handle, data, size);
}

void Foundry::Katana::DataMessage::setStartCoordinates(uint32_t x, uint32_t y)
{
  getSuite()->setStartCoordinates(_handle, x, y);
}

void Foundry::Katana::DataMessage::setDataDimensions(uint32_t width,
    uint32_t height)
{
  getSuite()->setDataDimensions(_handle, width, height);
}

void Foundry::Katana::DataMessage::setByteSkip(uint32_t byteSkip)
{
  getSuite()->setByteSkip(_handle, byteSkip);
}

const uint8_t* Foundry::Katana::DataMessage::frameUUID() const
{
  return getSuite()->dataFrameUUID(_handle);
}

uint16_t Foundry::Katana::DataMessage::channelID() const
{
  return getSuite()->dataChannelID(_handle);
}

uint32_t Foundry::Katana::DataMessage::xMin() const
{
  return getSuite()->xMin(_handle);
}

uint32_t Foundry::Katana::DataMessage::width() const
{
  return getSuite()->width(_handle);
}

uint32_t Foundry::Katana::DataMessage::yMin() const
{
  return getSuite()->yMin(_handle);
}

uint32_t Foundry::Katana::DataMessage::height() const
{
  return getSuite()->height(_handle);
}

uint32_t Foundry::Katana::DataMessage::bufferSize() const
{
  return getSuite()->bufferSize(_handle);
}

const void * Foundry::Katana::DataMessage::dataBuffer() const
{
  return getSuite()->dataBuffer(_handle);
}

uint32_t Foundry::Katana::DataMessage::byteSkip() const
{
  return getSuite()->byteSkip(_handle);
}
