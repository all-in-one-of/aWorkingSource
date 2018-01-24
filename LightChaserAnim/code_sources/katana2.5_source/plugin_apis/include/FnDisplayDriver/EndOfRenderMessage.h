// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef END_OF_RENDER_MESSAGE_H
#define END_OF_RENDER_MESSAGE_H

#include <stdint.h>

#include "Message.h"

namespace Foundry
{
  namespace Katana
  {
    class NewChannelMessage;
    /** @addtogroup DD
     *  @{
     */

    /** @brief The EndOfRenderMessage class encapsulates the message sent
     *  to Katana instructing it that there is no more data for the specified
     *  channel and it can clear up any data structures that it maintains
     *  for this channel.
     *
     * The EndOfRender message should only be sent down the pipe when no
     * more data is to be sent for the channel.
     *
     * @sa KatanaPipe.
     */
    class EndOfRenderMessage : public Message
    {
    public:
      /**
       * @brief Create a new instance of a EndOfRenderMessage message that, when sent
       * to Katana will let the Catalog Server know that there is no more data
       * to be sent through for this specified channel.
       *
       * @param[in] channel     the channel object that will be closed
       */
      EndOfRenderMessage(const NewChannelMessage &channel);

      /** @brief Create a new instance of a EndOfRenderMessage message that, when sent
       * to Katana will let the Catalog Server know that there is no more data
       * to be sent through for this specified channel.
       *
       * @param[in] frameID    a pointer to a 16 byte array containing a unique
       *                       frameID
       * @param[in] channelID  the channel ID for the channel that should be
       *                       closed.
       */
      EndOfRenderMessage(const uint8_t *frameID,
                   uint16_t channelID = 0);

      virtual ~EndOfRenderMessage() {};

    private:
      EndOfRenderMessage();
      EndOfRenderMessage(const EndOfRenderMessage &rhs);
      EndOfRenderMessage& operator=(const EndOfRenderMessage &rhs);
    };
    /**
     * @}
     */
  };
};

#endif
