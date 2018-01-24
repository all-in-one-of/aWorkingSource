// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <zmq.h>
#include <string>
#include <cassert>

#include "FnDisplayDriver/Message.h"
#include "FnDisplayDriver/ZmqHelpers.h"

int Foundry::Katana::Message::send_msg_part(const void *data,
                                            std::size_t size,
                                            void *socket,
                                            bool lastMsg) const
{
  // Check we have valid inputs
  assert(socket);

  zmq_msg_t msg;

  // Create a message of the appropriate size
  int rc = zmq_msg_init_size( &msg, size );
  assert( rc == 0 );

  // Copy our data into the zmq message
  memcpy( zmq_msg_data(&msg), data, size );

  // Pack the data off making sure to set the 'more' flag based on what we've
  // been asked to do by the lastMsg flag...
  rc = Util::IPC::zmqu_msg_send(&msg, socket, (lastMsg ? 0 : ZMQ_SNDMORE) );
  assert( rc == (int)size );

  // Free the zmq_msg_t memory allocated during zmq_msg_init_size
  zmq_msg_close( &msg );

  return 0;
}
