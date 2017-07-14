// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <cassert>

#include "FnDisplayDriver/DataMessage.h"
#include "FnDisplayDriver/NewChannelMessage.h"

Foundry::Katana::DataMessage::DataMessage(const NewChannelMessage &channel,
                                          uint32_t xMin,
                                          uint32_t width,
                                          uint32_t yMin,
                                          uint32_t height,
                                          uint32_t byteSkip)
{
  this->init(channel.frameUUID(), channel.channelID(), xMin, width, yMin, height, byteSkip);
}

Foundry::Katana::DataMessage::DataMessage(const byte *frameID,
                                          uint16_t channelID,
                                          uint32_t xMin,
                                          uint32_t width,
                                          uint32_t yMin,
                                          uint32_t height,
                                          uint32_t byteSkip)
{
  this->init(frameID, channelID, xMin, width, yMin, height, byteSkip);
}

Foundry::Katana::DataMessage::DataMessage()
:_data(NULL)
{

}

void Foundry::Katana::DataMessage::init(const byte *frameID,
                                        uint16_t channelID,
                                        uint32_t xMin,
                                        uint32_t width,
                                        uint32_t yMin,
                                        uint32_t height,
                                        uint32_t byteSkip)
{
  // Setup the protocol header
  _header.magicNumber = 0xAA;
  _header.versionNumber = 0x01;
  _header.msgType = 0x02;
  _header.zeroPadding = 0x00;

  // Setup the body of the Message
  memcpy( &(_dataMessage.frameUUID), frameID, 16);
  _dataMessage.channelID = channelID;
  _dataMessage.zeroPadding = 0;
  _dataMessage.xMin = xMin;
  _dataMessage.width = width;
  _dataMessage.yMin = yMin;
  _dataMessage.height = height;
  _dataMessage.byteSkip = byteSkip;

  _dataMessage.blockSize = 0;
  _data = NULL;
}

Foundry::Katana::DataMessage::~DataMessage()
{
  // Make sure to free up the payload if we're carying one..
  if(_data)
  {
    delete [] _data;
    _data = NULL;

    _dataMessage.blockSize = 0;
  }

#ifdef SOCKET_DEBUG
  std::cerr << "DataMessage "; _printFrameID(std::cerr); std::cerr << " destroyed" << std::endl;
#endif
}

int Foundry::Katana::DataMessage::send(void *socket) const
{
  assert(socket);

  // Header
  Message::send_msg_part( &_header, sizeof( _header ), socket, false );

  // Body
  Message::send_msg_part( &_dataMessage, sizeof( _dataMessage ), socket, false);

  // Payload
  Message::send_msg_part( _data, _dataMessage.blockSize, socket, true );

  return 0;
}

Foundry::Katana::Message* Foundry::Katana::DataMessage::copy() const
{
  Foundry::Katana::DataMessage* dmCopy = new Foundry::Katana::DataMessage();

  memcpy(&(dmCopy->_header), &_header, sizeof(_header) );
  memcpy(&(dmCopy->_dataMessage), &_dataMessage, sizeof(_dataMessage) );
  dmCopy->setData(dataBuffer(), bufferSize() );

  return dmCopy;
}

#ifdef SOCKET_DEBUG
std::ostream& Foundry::Katana::operator<<(std::ostream &out, const Foundry::Katana::DataMessage &dataMsg)
{
  out << "********************************************************************************" << "\n";
  out << "Data" << "\n";

  out << "Header - Magic Number   : " << std::hex << static_cast<int>(dataMsg._header.magicNumber) << "\n";
  out << "Header - Version Number : " << std::hex << static_cast<int>(dataMsg._header.versionNumber) << "\n";
  out << "Header - Message Type   : " << std::hex << static_cast<int>(dataMsg._header.msgType) << "\n";
  out << "Header - Zero Padding   : " << std::hex << static_cast<int>(dataMsg._header.zeroPadding) << "\n\n";

  out << "Body   - Frame UUID     : ";
  dataMsg._printFrameID(out);
  out << "\n";

  out << std::dec;
  out << "Body   - Channel ID    : " << dataMsg._dataMessage.channelID << "\n";
  out << "Body   - Zero Padding  : " << dataMsg._dataMessage.zeroPadding << "\n";
  out << "Body   - xMin          : " << dataMsg._dataMessage.xMin << "\n";
  out << "Body   - Width         : " << dataMsg._dataMessage.width << "\n";
  out << "Body   - yMin          : " << dataMsg._dataMessage.yMin << "\n";
  out << "Body   - Height        : " << dataMsg._dataMessage.height << "\n";
  out << "Body   - Byte Skip     : " << dataMsg._dataMessage.byteSkip << "\n";
  out << "Body   - Block Size    : " << dataMsg._dataMessage.blockSize << "\n\n";
  out << std::endl;

  return out;
}

void Foundry::Katana::DataMessage::_printFrameID(std::ostream &out) const
{
  for(int x = 0; x < 16; x++)
  {
    out << std::hex << static_cast<int>(_dataMessage.frameUUID[x]);
    if(((x + 1) % 4) == 0 &&  (x + 1) < 16)
      out << " - ";
  }
}
#endif
