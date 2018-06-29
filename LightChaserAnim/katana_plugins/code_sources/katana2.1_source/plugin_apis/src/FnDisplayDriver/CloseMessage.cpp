// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <cassert>

#include "FnDisplayDriver/CloseMessage.h"
#include "FnDisplayDriver/NewChannelMessage.h"

Foundry::Katana::CloseMessage::CloseMessage(const NewChannelMessage &channel)
{
  this->init(channel.frameUUID(), channel.channelID());
}

Foundry::Katana::CloseMessage::CloseMessage(const byte *frameID, uint16_t channelID)
{
  this->init(frameID, channelID);
}

Foundry::Katana::CloseMessage::CloseMessage()
{

}

void Foundry::Katana::CloseMessage::init(const byte *frameID, uint16_t channelID)
{
  // Setup the protocol header
  _header.magicNumber = 0xAA;
  _header.versionNumber = 0x01;
  _header.msgType = 0x03;
  _header.zeroPadding = 0x00;

  // Setup the body of the message.
  memcpy( &(_closeMessage.frameUUID), frameID, 16);
  _closeMessage.channelID = channelID;
  _closeMessage.zeroPadding = 0;
}

int Foundry::Katana::CloseMessage::send(void *socket) const
{
  assert(socket);

  // Header
  Message::send_msg_part(&_header, sizeof( _header ), socket, false);

  // Body
  Message::send_msg_part(&_closeMessage, sizeof( _closeMessage ), socket, true);

  return 0;
}

Foundry::Katana::Message* Foundry::Katana::CloseMessage::copy() const
{
  Foundry::Katana::CloseMessage *cmCopy = new Foundry::Katana::CloseMessage();

  memcpy(&(cmCopy->_header), &_header, sizeof(_header));
  memcpy(&(cmCopy->_closeMessage), &_closeMessage, sizeof(_closeMessage));

  return cmCopy;
}

