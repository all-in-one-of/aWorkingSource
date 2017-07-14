#include "FnDisplayDriver/ZmqHelpers.h"

int Foundry::Katana::Util::IPC::zmqu_msg_send(zmq_msg_t *msg, void *socket, int flags)
{
  // Refer to http://api.zeromq.org/3-2:zmq-msg-send for possible return codes.
  // The logic here is such that:
  // 1). If EINTR retry the call
  // 2). If EAGAIN - see if we're a non-blocking socket and if so return otherwise
  //     retry the call.
  // 3). Any other error return error to the user.
  // 4). If zmq_msg_send > 0 then then success, return number of bytes sent.
  int r_val = -1;
  while( (r_val = zmq_msg_send(msg, socket, flags)) < 0)
  {
    if(errno == EINTR)
    {
      continue;
    }
    else if(errno == EAGAIN)
    {
      if(flags & ZMQ_NOBLOCK)
      {
        return -1;
      }
      else
      {
        continue;
      }
    }
    else
    {
      return -1;
    }
  }

  return r_val;
}

int Foundry::Katana::Util::IPC::zmqu_msg_recv(zmq_msg_t *msg, void *socket, int flags)
{
  // Refer to http://api.zeromq.org/3-2:zmq-msg-recv for possible return codes.
  // The logic here is such that:
  // 1). If EINTR retry the call
  // 2). If EAGAIN - see if we're a non-blocking socket and if so return otherwise
  //     retry the call.
  // 3). Any other error return error to the user.
  // 4). If zmq_msg_recv > 0 then then success, return number of bytes sent.
  int r_val = -1;
  while( (r_val = zmq_msg_recv(msg, socket, flags)) < 0)
  {
    if(errno == EINTR)
    {
      continue;
    }
    else if(errno == EAGAIN)
    {
      if(flags & ZMQ_NOBLOCK)
      {
        return -1;
      }
      else
      {
        continue;
      }
    }
    else
    {
      return -1;
    }
  }

  return r_val;
}

