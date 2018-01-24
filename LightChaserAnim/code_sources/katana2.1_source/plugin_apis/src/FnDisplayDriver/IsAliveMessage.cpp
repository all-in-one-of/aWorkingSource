// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <cassert>

#include "FnDisplayDriver/IsAliveMessage.h"

Foundry::Katana::IsAliveMessage::IsAliveMessage()
{
  // Setup the protocol header
  _header.magicNumber = 0xAA;
  _header.versionNumber = 0x01;
  _header.msgType = IsAliveMessageType;
  _header.zeroPadding = 0x00;

  // Fill the _isAliveStruct
  _isAliveMessage.aliveAt = time(NULL);
}

int Foundry::Katana::IsAliveMessage::send(void *socket) const
{
  assert(socket);

  // Header
  Message::send_msg_part(&_header, sizeof( _header ), socket, false);

  // Body
  Message::send_msg_part(&_isAliveMessage, sizeof( _isAliveMessage ), socket, true);

  return 0;
}

Foundry::Katana::Message* Foundry::Katana::IsAliveMessage::copy() const
{
  Foundry::Katana::IsAliveMessage *cmCopy = new Foundry::Katana::IsAliveMessage();

  memcpy(&(cmCopy->_header), &_header, sizeof(_header));
  memcpy(&(cmCopy->_isAliveMessage), &_isAliveMessage, sizeof(_isAliveMessage));

  return cmCopy;
}

