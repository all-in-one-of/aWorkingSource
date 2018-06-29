// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <cassert>

#include "FnDisplayDriver/EndOfRenderMessage.h"
#include "FnDisplayDriver/NewChannelMessage.h"

Foundry::Katana::EndOfRenderMessage::EndOfRenderMessage(const NewChannelMessage &channel)
{
  this->init(channel.frameUUID(), channel.channelID());
}

Foundry::Katana::EndOfRenderMessage::EndOfRenderMessage(const byte *frameID, uint16_t channelID)
{
  this->init(frameID, channelID);
}

Foundry::Katana::EndOfRenderMessage::EndOfRenderMessage()
{

}

void Foundry::Katana::EndOfRenderMessage::init(const byte *frameID, uint16_t channelID)
{
  // Setup the protocol header
  _header.magicNumber = 0xAA;
  _header.versionNumber = 0x01;
  _header.msgType = EndOfRenderMessageType;
  _header.zeroPadding = 0x00;

  // Setup the body of the message.
  memcpy( &(_eorMessage.frameUUID), frameID, 16);
  _eorMessage.channelID = channelID;
  _eorMessage.zeroPadding = 0;
}

int Foundry::Katana::EndOfRenderMessage::send(void *socket) const
{
  assert(socket);

  // Header
  Message::send_msg_part(&_header, sizeof( _header ), socket, false);

  // Body
  Message::send_msg_part(&_eorMessage, sizeof( _eorMessage ), socket, true);

  return 0;
}

Foundry::Katana::Message* Foundry::Katana::EndOfRenderMessage::copy() const
{
  Foundry::Katana::EndOfRenderMessage *cmCopy = new Foundry::Katana::EndOfRenderMessage();

  memcpy(&(cmCopy->_header), &_header, sizeof(_header));
  memcpy(&(cmCopy->_eorMessage), &_eorMessage, sizeof(_eorMessage));

  return cmCopy;
}

