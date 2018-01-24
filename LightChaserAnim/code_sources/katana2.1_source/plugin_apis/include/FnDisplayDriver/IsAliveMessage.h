// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef IS_ALIVE_MESSAGE_H
#define IS_ALIVE_MESSAGE_H

#include "Message.h"
#include "MessageTypes.h"

#include <ctime>

namespace Foundry
{
  namespace Katana
  {
    /** @addtogroup DD
     *  @{
     */

    /** @brief The IsAliveMessage is sent periodically to inform Katana
     *  that the renderer is still in progress.
     *
     * @sa KatanaPipe.
     */
    class IsAliveMessage : public Message
    {
    public:
      IsAliveMessage();

      virtual ~IsAliveMessage() {};

      int send(void *socket) const;

      Message* copy() const;

    private:
      IsAliveMessage(const IsAliveMessage &rhs);
      IsAliveMessage& operator=(const IsAliveMessage &rhs);

      RDPHeaderStruct _header;
      IsAliveMessageStruct _isAliveMessage;
    };
    /**
     * @}
     */
  };
};

#endif
