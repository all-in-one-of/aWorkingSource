// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnDisplayDriver/EndOfRenderMessage.h"
#include "FnDisplayDriver/NewChannelMessage.h"
#include <FnDisplayDriver/suite/FnDisplayDriverSuite.h>

Foundry::Katana::EndOfRenderMessage::EndOfRenderMessage(
    const NewChannelMessage &channel)
{
  _handle = getSuite()->createEndOfRenderMessage(channel.frameUUID(),
    channel.channelID());
}

Foundry::Katana::EndOfRenderMessage::EndOfRenderMessage(const uint8_t *frameID,
    uint16_t channelID)
{
  _handle = getSuite()->createEndOfRenderMessage(frameID, channelID);
}

