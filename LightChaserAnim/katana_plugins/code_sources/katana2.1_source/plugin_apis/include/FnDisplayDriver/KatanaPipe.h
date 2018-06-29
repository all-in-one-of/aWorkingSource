// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef KATANA_PIPE_H
#define KATANA_PIPE_H

#include <queue>
#include <string>

#include <boost/thread.hpp>

#include "Message.h"
#include "NewChannelMessage.h"

#include <iostream>

#define DEFAULT_MAX_QUEUE_SIZE 16384

namespace Foundry
{
  namespace Katana
  {
     /**************************************************************************
      * TSQ Implementation
      *************************************************************************/
      class ConcurrentQueue
      {
      public:
          ConcurrentQueue(const size_t max_size=DEFAULT_MAX_QUEUE_SIZE,
                          const char * name=""):
                          _queue(), _mutex(), _condition(), _empty_q_cond(),
                          _q_not_full_cond(), _max_size(max_size), _name(name),
                          _warned_max_size_reached(false)
          {}

        void push(const Message &msg)
        {
          boost::mutex::scoped_lock lock(_mutex);

          if (_max_size > 0)
          {
            while(_queue.size() > _max_size)
            {
              if (!_warned_max_size_reached)
                {
                  std::cerr << "[KatanaPipe" << (_name.empty() ? "" : " ") <<
                          _name << "] WARNING: limit of " <<
                          _max_size << " messages reached." << std::endl;
                  _warned_max_size_reached = true;
                }
              _q_not_full_cond.wait(lock);
            }
          }

          // The caller may have allocated msg on the heap and immediately
          // cleaned it up, we want a cheap copy containing the data so we can
          // pack it down the pipe.
          Message *m = msg.copy();
          _queue.push(m);
          lock.unlock();
          _condition.notify_one();
        }

        void wait_and_pop(Message **msg)
        {
          boost::mutex::scoped_lock lock(_mutex);
          while(_queue.empty())
          {
            _condition.wait(lock);
          }

          *msg = (_queue.front());
          _queue.pop();

          // Notify listeners if that pop made us empty.
          if( _queue.empty() )
            _empty_q_cond.notify_one();

          // Notify any thread waiting to push.
          _q_not_full_cond.notify_one();
        }

        void wait_till_empty()
        {
          boost::mutex::scoped_lock lock(_mutex);
          while(!_queue.empty())
          {
            _empty_q_cond.wait(lock);
          }

          return;
        }

      private:
        // Disable copy and assignment
        ConcurrentQueue(const ConcurrentQueue &rhs);
        ConcurrentQueue& operator=(const ConcurrentQueue rhs);

        // The queue
        std::queue<Message*> _queue;
        mutable boost::mutex _mutex;
        boost::condition_variable _condition;

        boost::condition_variable _empty_q_cond;
        boost::condition_variable _q_not_full_cond;

        const size_t _max_size;
        const std::string _name;

        // _warned_max_size_reached needn't be atomic, as it is protected
        // by _mutex.
        bool _warned_max_size_reached;
      };

    /**
     * @ingroup DD
     * @brief The KatanaPipe is used to send messages to the specified Katana
     * Catalog Server.
     *
     * It is a uni-directional communication pipe that allows a message to be
     * sent to the end-point specified in the constructor. The end-point will
     * usually be Katana however any process implementing the Render Data
     * Protocol could serve as the end-point.
     *
     * Very important <b>The pipe is not thread safe.</b> If you require
     * communication capabilities from more than a single thread you should
     * create an instance of the KatanaPipe for each thread however you should
     * find this is not necessary.
     */
    class KatanaPipe
    {
    public:

      /** @brief Create a new instance of the KatanaPipe which will be connected
       * to the specified host name and port number.
       *
       * The constructor will not attempt to create the actual connection to the
       * specified host name and port number. To do this you must call connect()
       *
       * @param[in] hostName   a host name that specifies the end point to be
       *                       connected to. This must be network visible from
       *                       the host you are connecting from.
       * @param[in] portNumber the TCP port which the end-point on the specified
       *                       hostName is listening for messages on.
       *
       * @param[in] maxQueueSize the maximum number of messages we can store in
       *                       the outgoing thread safe queue and control queue.
       *
       * @sa connect()
       */
      KatanaPipe(const std::string &hostName, unsigned int portNumber,
                 const size_t maxQueueSize);


      /** @brief Destroy this Katana Pipe.
       *
       * This will close the connection to the end-point, any unsent messages in
       * the buffer will be sent.
       */
      ~KatanaPipe();

      /** @brief Initiate the connection to the end-point specified by the host
       * name and port number provided at construction.
       *
       * connect() must be called before any messages are sent. Calling
       * connect() after the pipe has been connected will not have any effect.
       *
       * @return 0 if the pipe connects successfully to the end-point otherwise
       *           nonzero.
       */
      int connect();

      /** @brief Send a message to the end-point this pipe is connected to.
       *
       * send() pushes a message down the pipe to the end-point this pipe is
       * connected to. send() ensures the message is either sent in its
       * entirety or not at all.
       *
       * @param[in] message   the message to be send.
       *
       * @return 0 if the message is sent successfully - otherwise nonzero.
       */
      int send(const Message &message);
      int sendControl(const Message &message);

      /** @brief This is an important synchronisation function that you should
       * called when your renderer has sent all image data to the driver for
       * a given channel.
       *
       * It will block until all data has been cleared from this end of the
       * pipe.
       */
      void flushPipe(const NewChannelMessage &channel);

      /** @brief This is an important function that you should call from your
       * driver when there is no more data to be sent for this channel, ever.
       *
       * Once this message has been sent Katana will ignore any further image
       * data.
       */
      void closeChannel(const NewChannelMessage &channel);

    private:
      // Disable copy and assignment
      KatanaPipe(const KatanaPipe &rhs);
      KatanaPipe& operator=(const KatanaPipe &rhs);

      // Terminate the pipe connection
      void terminate();

      // Human readable end point specifier
      std::string _hostName;
      unsigned int _portNumber;

      // The zmq context
      void *_context;

      // The zmq PUSH sockets
      void *_socket;
      void *_ctrl_socket;

      // Connection flag.
      bool _isConnected;

      // Thread Safe Queue
      ConcurrentQueue _tsq;
      ConcurrentQueue _tsq_control;

      // Consumer - sender - thread
      boost::thread* _consumerThread;
      static void consumer(KatanaPipe *pipe);

      // Consumer - sender - thread
      boost::thread* _consumerControlThread;
      static void consumerControl(KatanaPipe *pipe);

      // Heartbeat thread
      boost::thread* _heartbeatThread;
      static void heartBeat(KatanaPipe *pipe);
    };
  };
};

#endif
