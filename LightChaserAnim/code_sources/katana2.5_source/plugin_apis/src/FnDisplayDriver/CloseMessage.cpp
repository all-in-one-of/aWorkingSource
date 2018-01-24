// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnDisplayDriver/CloseMessage.h"
#include "FnDisplayDriver/NewChannelMessage.h"
#include <FnDisplayDriver/suite/FnDisplayDriverSuite.h>

Foundry::Katana::CloseMessage::CloseMessage(const NewChannelMessage &channel)
{
  _handle = getSuite()->createCloseMessage(channel.frameUUID(),
    channel.channelID());
}

Foundry::Katana::CloseMessage::CloseMessage(const uint8_t *frameID,
    uint16_t channelID)
{
  _handle = getSuite()->createCloseMessage(frameID, channelID);
}

