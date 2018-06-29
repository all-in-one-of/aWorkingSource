// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#include <zmq.h>
#include <netdb.h>
#include <cassert>
#include <sstream>

#include "FnDisplayDriver/KatanaPipe.h"
#include "FnDisplayDriver/CloseMessage.h"
#include "FnDisplayDriver/EndOfRenderMessage.h"
#include "FnDisplayDriver/IsAliveMessage.h"

//#define SOCKET_DEBUG 1

#ifdef SOCKET_DEBUG
#include <iostream>
#include <iomanip>
#endif


static int g_numActiveConnections = 0;

#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS) || defined(_MSC_VER)
#error "incNumActiveConnections, decNumActiveConnections and getNumActiveConnections not implemented on Windows"
#else
inline static void incNumActiveConnections()
{
  __sync_add_and_fetch(&g_numActiveConnections, 1);
}

inline static int decNumActiveConnections()
{
  return __sync_add_and_fetch(&g_numActiveConnections, -1);
}

inline static int getNumActiveConnections()
{
  return __sync_add_and_fetch(&g_numActiveConnections, 0);
}
#endif

Foundry::Katana::KatanaPipe::KatanaPipe(const std::string &hostName,
                                        unsigned int portNumber,
                                        size_t maxQueueSize)
: _hostName(hostName),
  _portNumber(portNumber),
  _context(NULL),
  _socket(NULL),
  _ctrl_socket(NULL),
  _isConnected(false),
  _tsq(maxQueueSize),
  _tsq_control(maxQueueSize, "Control"),
  _consumerThread(NULL),
  _consumerControlThread(NULL),
  _heartbeatThread(NULL)
{
    // First get a handle on the ZMQ Context
    // Here we take care of creating the ZeroMq context directly without
    // going through ContextSingleton since the context will be deleted in
    // the terminate method.
    _context = zmq_ctx_new();
    assert(_context);

    // Create the socket for data related to the image we're going to send.
    _socket = zmq_socket( _context, ZMQ_PUSH );
    assert(_socket);

    // Create the control socket for sending information related to our
    // communication channel with Katana.
    _ctrl_socket = zmq_socket( _context, ZMQ_PUSH );
    assert(_ctrl_socket);

    int HWM = 1;
    int result = zmq_setsockopt(_socket, ZMQ_SNDHWM, &HWM, sizeof(int));
    assert(result == 0);

    int SOCKETTIMEOUT = 2000;
    result = zmq_setsockopt(_socket, ZMQ_SNDTIMEO, &SOCKETTIMEOUT, sizeof(int));
    assert(result == 0);

    HWM = 1;
    result = zmq_setsockopt(_ctrl_socket, ZMQ_SNDHWM, &HWM, sizeof(int));
    assert(result == 0);

    SOCKETTIMEOUT = 2000;
    result = zmq_setsockopt(_ctrl_socket, ZMQ_SNDTIMEO, &SOCKETTIMEOUT, sizeof(int));
    assert(result == 0);
}

Foundry::Katana::KatanaPipe::~KatanaPipe()
{
  assert(getNumActiveConnections() == 0);
  terminate();
}

int Foundry::Katana::KatanaPipe::connect()
{
  if(_isConnected)
  {
    incNumActiveConnections();
    return 0;
  }

  // Make sure the given host name can be resolved, otherwise use 'localhost'
  struct hostent * h = gethostbyname(_hostName.c_str());
  if (!h)
  {
    std::cerr << "[KatanaPipe] Warning: unable to resolve host name '"
      << _hostName << "', 'localhost' will be used instead." << std::endl;
    _hostName = "localhost";
  }

  // Build the string that we'll pass to the ZMQ socket to specify the
  // endpoint
  std::stringstream ss;
  ss << "tcp://" << _hostName << ":" << _portNumber;

#ifdef SOCKET_DEBUG
  std::cerr << "Connecting to " << ss.str() << std::endl;
#endif

  int rc = zmq_connect( _socket, ss.str().c_str() );

  if(rc == -1)
    return -1;

  // Build the string that we'll pass to the ZMQ socket to specify the
  // endpoint
  ss.str("");

  // The control port sits +100 to the data port.
  ss << "tcp://" << _hostName << ":" << (_portNumber + 100);

  rc = zmq_connect( _ctrl_socket, ss.str().c_str() );

  if(rc == -1)
    return -1;

  // It's at this point that we can kick off our consumer thread...
  if(_consumerThread == NULL)
  {
    _consumerThread = new boost::thread(consumer, this);
  }

  // It's at this point that we can kick off our consumer control thread...
  if(_consumerControlThread == NULL)
  {
    _consumerControlThread = new boost::thread(consumerControl, this);
  }

  incNumActiveConnections();

  // Kick off the heart beat thread also.
  if(_heartbeatThread == NULL)
  {
    _heartbeatThread = new boost::thread(heartBeat, this);
  }

  _isConnected = true;

  return 0;
}

void Foundry::Katana::KatanaPipe::heartBeat(Foundry::Katana::KatanaPipe *pipe)
{
  while(1)
  {
    IsAliveMessage *isAlive = new IsAliveMessage();

    pipe->sendControl(*isAlive);
    delete isAlive;
    isAlive = 0;

    bool stop = false;
    for (int i = 0; i < 5 && !stop; ++i)
    {
      if (getNumActiveConnections() == 0)
      {
        stop = true;
        break;
      }
      boost::this_thread::sleep(boost::posix_time::milliseconds(200));
    }
    if (stop)
      break;
  }
}

void Foundry::Katana::KatanaPipe::consumerControl(Foundry::Katana::KatanaPipe *pipe)
{
  while(1)
  {
    Message *m = NULL;
    pipe->_tsq_control.wait_and_pop(&m);

#ifdef SOCKET_DEBUG
    std::cout << "Got message " << m << std::endl;
#endif

    m->send(pipe->_ctrl_socket);

    delete m;
  }
}

void Foundry::Katana::KatanaPipe::consumer(Foundry::Katana::KatanaPipe *pipe)
{
  while(1)
  {
    Message *m = NULL;
    pipe->_tsq.wait_and_pop(&m);

#ifdef SOCKET_DEBUG
    std::cout << "Got message " << m << std::endl;
#endif

    m->send(pipe->_socket);

    delete m;
  }
}

int Foundry::Katana::KatanaPipe::send(const Message &message)
{
  if(_isConnected)
  {
    // Push into the TSQ
    _tsq.push(message);
    return 0;
  }
  else
  {
    return -1;
  }
}

int Foundry::Katana::KatanaPipe::sendControl(const Message &message)
{
  if(_isConnected)
  {
    // Push into the TSQ
    _tsq_control.push(message);
    return 0;
  }
  else
  {
    return -1;
  }
}

void Foundry::Katana::KatanaPipe::flushPipe(const NewChannelMessage &channel)
{
  // Send out a close message to let Katana clean up.
  CloseMessage *closeMsg = new CloseMessage(channel);

  this->send(*closeMsg);

  delete closeMsg;
  closeMsg = 0;

  return;
}

void Foundry::Katana::KatanaPipe::closeChannel(const NewChannelMessage &channel)
{
  // Send out a close message to let Katana clean up.
  EndOfRenderMessage *eorMessage = new EndOfRenderMessage(channel);

  this->send(*eorMessage);

  delete eorMessage;
  eorMessage = 0;

  // Decrement the number of active connections and check if we're done.
  if (decNumActiveConnections() == 0)
  {
    // Block until the tsq is empty...
    _tsq.wait_till_empty();

    // If the number of active connections is zero then we need to clean-up
    // the threads previously created.
    terminate();
  }

  return;
}

void Foundry::Katana::KatanaPipe::terminate()
{
  if (_consumerThread)
  {
    // Interrupt the consumer thread.
    _consumerThread->interrupt();
    // Wait for the thread to finish.
    _consumerThread->join();

    delete _consumerThread;
    _consumerThread = NULL;
  }

  if (_heartbeatThread)
  {
    // We don't need to interrupt _heartbeatThread,
    // we only need to wait  for it to finish.
    _heartbeatThread->join();

    delete _heartbeatThread;
    _heartbeatThread = NULL;
  }

  if (_consumerControlThread)
  {
    // Interrupt the consumerControl thread.
    _consumerControlThread->interrupt();
    // Wait for the thread to finish.
    _consumerControlThread->join();

    delete _consumerControlThread;
    _consumerControlThread = NULL;
  }

  int result = 0;
  if (_socket)
  {
    result = zmq_close(_socket);
    assert(result == 0);
    _socket = NULL;
  }

  if (_ctrl_socket)
  {
    result = zmq_close(_ctrl_socket);
    assert(result == 0);
    _ctrl_socket = NULL;
  }

  if (_context)
  {
    // Terminate the zeromq context.
    zmq_term(_context);
    _context = NULL;
  }

  _isConnected = false;
}

