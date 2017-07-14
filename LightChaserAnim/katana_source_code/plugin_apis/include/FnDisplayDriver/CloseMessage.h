// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef CLOSE_MESSAGE_H
#define CLOSE_MESSAGE_H

#include "Message.h"
#include "MessageTypes.h"

namespace Foundry
{
  namespace Katana
  {
    class NewChannelMessage;
    /** @addtogroup DD
     *  @{
     */

    /** @brief The CloseMessage class encapsulates the message sent to Katana
     *  instructing it that there is no more data for the specified channel.
     *
     * The CloseMessage message should only be sent down the pipe when no
     * more data is to be sent for the channel.
     *
     * It is un-likely you will ever need to directly instantiate this class as
     * it is handled for you in the closeChannel method of the KatanaPipe.
     *
     * @sa KatanaPipe.
     */
    class CloseMessage : public Message
    {
    public:
      /**
       * @brief Create a new instance of a CloseMessage message that, when sent
       * to Katana will let the Catalog Server know that there is no more data
       * to be sent through for this specified channel.
       *
       * @param[in] channel     the channel object that will be closed
       */
      CloseMessage(const NewChannelMessage &channel);

      /** @brief Create a new instance of a CloseMessage message that, when sent
       * to Katana will let the Catalog Server know that there is no more data
       * to be sent through for this specified channel.
       *
       * @param[in] frameID    a pointer to a 16 byte array containing a unique
       *                       frameID
       * @param[in] channelID  the channel ID for the channel that should be
       *                       closed.
       */
      CloseMessage(const byte *frameID,
                   uint16_t channelID = 0);

      virtual ~CloseMessage() {};

      int send(void *socket) const;

      Message* copy() const;

    private:
      CloseMessage();
      CloseMessage(const CloseMessage &rhs);
      CloseMessage& operator=(const CloseMessage &rhs);

      void init(const byte *frameID,
                uint16_t channelID);

      RDPHeaderStruct _header;
      CloseMessageStruct _closeMessage;
    };
    /**
     * @}
     */
  };
};

#endif
