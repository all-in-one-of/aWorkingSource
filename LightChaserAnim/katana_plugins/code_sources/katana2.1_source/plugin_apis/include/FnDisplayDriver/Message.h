// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef MESSAGE_H
#define MESSAGE_H

#include <cstring>

#define MAX_FRAME_NAME_LENGTH 8000

namespace Foundry
{
  namespace Katana
  {
    /** @addtogroup DD
     *  @{
     */

    /** @brief This is the base class against which all Messages sent to Katana
     *  should inherit from.
     */
    class Message
    {
    public:
      virtual ~Message() { /* Empty */ };

      /** @brief Called by the KatanaPipe to send the Message down the pipe
       *  socket will be a valid zmq socket.
       *
       *  send() may be called multiple times on the same object (this diverges
       *  from zmq messages where a zmq_msg_t is only valid for 1 send() call.
       *
       *  @param[in] socket - valid zmq socket handle.
       *
       *  @return 0 if the entire message was sent successfully otherwise
       *  non-zero.
       */
      virtual int send(void *socket) const = 0;

      /** Return a pointer to a valid Message type capable of being sent down
       *  the pipe.
       *
       *  <b>The memory will be the responsibility of the caller to clean up.</b>
       *
       *  @return pointer to a valid Message.
       */
      virtual Message* copy() const = 0;

    protected:
      /** send_msg_part() is used to send part of a zmq message. The following
       * arguments are valid:
       *
       * @param[in] data    pointer to the data to be written down the pipe.
       * @param[in] size    number of bytes to be written down the pipe.
       * @param[in] socket  valid handle to zero mq socket.
       * @param[in] lastMsg set to true if this is the last message in a
       *                    multi-part zero mq message otherwise set to true.
       */
      virtual int send_msg_part(const void *data,
                                std::size_t size,
                                void *socket,
                                bool lastMsg = false) const;
    };
    /**
     * @}
     */
  };
};


#endif
