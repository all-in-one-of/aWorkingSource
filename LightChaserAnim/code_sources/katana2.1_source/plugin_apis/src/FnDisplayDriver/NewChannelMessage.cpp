// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.
#include "FnDisplayDriver/NewChannelMessage.h"
#include "FnDisplayDriver/NewFrameMessage.h"

Foundry::Katana::NewChannelMessage::NewChannelMessage(const byte *frameID,
                                                      uint16_t channelID,
                                                      uint32_t height,
                                                      uint32_t width,
                                                      int xorigin,
                                                      int yorigin,
                                                      float sampleRate1,
                                                      float sampleRate2,
                                                      uint32_t dataSize)
{
  init(frameID, channelID, height, width, xorigin, yorigin, sampleRate1, sampleRate2, dataSize);
}

Foundry::Katana::NewChannelMessage::NewChannelMessage(const NewFrameMessage &frame,
                                                      uint16_t channelID,
                                                      uint32_t height,
                                                      uint32_t width,
                                                      int xorigin,
                                                      int yorigin,
                                                      float sampleRate1,
                                                      float sampleRate2,
                                                      uint32_t dataSize)
{
  init(frame.frameUUID(), channelID, height, width, xorigin, yorigin, sampleRate1, sampleRate2, dataSize);
}

Foundry::Katana::NewChannelMessage::NewChannelMessage()
{

}

void Foundry::Katana::NewChannelMessage::init(const byte *frameID,
                                              uint16_t channelID,
                                              uint32_t height,
                                              uint32_t width,
                                              int xorigin,
                                              int yorigin,
                                              float sampleRate1,
                                              float sampleRate2,
                                              uint32_t dataSize)
{
  // Setup the protocol header
  _header.magicNumber = 0xAA;
  _header.versionNumber = 0x01;
  _header.msgType = 0x01;
  _header.zeroPadding = 0x00;

  // Setup the body of the Message
  memcpy(&(_newChannel.frameUUID), frameID, 16);
  _newChannel.channelID = channelID;
  _newChannel.zeroPadding = 0;
  _newChannel.height = height;
  _newChannel.width = width;
  _newChannel.xorigin = xorigin;
  _newChannel.yorigin = yorigin;
  _newChannel.dataSize = dataSize;
  _newChannel.sampleRate[0] = sampleRate1;
  _newChannel.sampleRate[1] = sampleRate2;
  _newChannel.channelNameLength = 0;
  _newChannel.alignmentPadding = 0;
}

Foundry::Katana::NewChannelMessage::~NewChannelMessage()
{
  // No dynamically allocated memory to worry about...
#ifdef SOCKET_DEBUG
  std::cerr << "NewChannelMessage "; _printFrameID(std::cerr); std::cerr << " destroyed " << std::endl;
#endif
}

int Foundry::Katana::NewChannelMessage::send(void *socket) const
{
  assert(socket);

  // Header
  Message::send_msg_part( &_header, sizeof( _header ), socket, false );

  // Body
  Message::send_msg_part(&_newChannel, sizeof( _newChannel ), socket, false);

  // Channel Name
  Message::send_msg_part(_channelName.c_str(), _newChannel.channelNameLength, socket, true);

  return 0;
}

Foundry::Katana::Message* Foundry::Katana::NewChannelMessage::copy() const
{
  Foundry::Katana::NewChannelMessage* ncCopy = new Foundry::Katana::NewChannelMessage();

  memcpy(&(ncCopy->_header), &_header, sizeof(_header));
  memcpy(&(ncCopy->_newChannel), &_newChannel, sizeof(_newChannel));
  ncCopy->_channelName = _channelName;

  return ncCopy;
}

#ifdef SOCKET_DEBUG
std::ostream& Foundry::Katana::operator<<(std::ostream &out, const Foundry::Katana::NewChannelMessage &channel)
{
  out << "********************************************************************************" << "\n";
  out << "Channel" << "\n";

  out << "Header - Magic Number   : " << std::hex << static_cast<int>(channel._header.magicNumber) << "\n";
  out << "Header - Version Number : " << std::hex << static_cast<int>(channel._header.versionNumber) << "\n";
  out << "Header - Message Type   : " << std::hex << static_cast<int>(channel._header.msgType) << "\n";
  out << "Header - Zero Padding   : " << std::hex << static_cast<int>(channel._header.zeroPadding) << "\n\n";

  out << "Body   - Frame UUID    : ";
  channel._printFrameID(out);
  out << "\n";

  out << std::dec;
  out << "Body   - Channel ID    : " << channel._newChannel.channelID << "\n";
  out << "Body   - Zero Padding  : " << channel._newChannel.zeroPadding << "\n";
  out << "Body   - Height        : " << channel._newChannel.height << "\n";
  out << "Body   - Width         : " << channel._newChannel.width << "\n";
  out << "Body   - X Origin      : " << channel._newChannel.xorigin << "\n";
  out << "Body   - Y Origin      : " << channel._newChannel.yorigin << "\n";
  out << "Body   - Data Size     : " << channel._newChannel.dataSize << "\n";
  out << "Body   - Sample Rate 1 : " << channel._newChannel.sampleRate[0] << "\n";
  out << "Body   - Sample Rate 2 : " << channel._newChannel.sampleRate[1]<< "\n";
  out << "Body   - Chnl Name Len : " << channel._newChannel.channelNameLength << "\n";
  out << "Body   - Alignment Pad : " << channel._newChannel.alignmentPadding << "\n\n";

  out << "Tail   - Channel Name  : " << channel._channelName << "\n";
  out << std::endl;

  return out;
}

void Foundry::Katana::NewChannelMessage::_printFrameID(std::ostream &out) const
{
  for(int x = 0; x < 16; x++)
  {
    out << std::hex << static_cast<int>(_newChannel.frameUUID[x]);
    if(((x + 1) % 4) == 0 &&  (x + 1) < 16)
      out << " - ";
  }
}
#endif
