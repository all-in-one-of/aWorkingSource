#ifndef ZMQ_HELPERS_H
#define ZMQ_HELPERS_H

#include <zmq.h>

namespace Foundry
{
  namespace Katana
  {
    namespace Util
    {
      namespace IPC
      {
        int zmqu_msg_send(zmq_msg_t *msg, void *socket, int flags);

        int zmqu_msg_recv(zmq_msg_t *msg, void *socket, int flags);
      }
    }
  }
}


#endif
