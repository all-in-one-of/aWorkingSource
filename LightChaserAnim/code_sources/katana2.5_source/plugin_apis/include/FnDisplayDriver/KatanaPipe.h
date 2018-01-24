// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef KATANA_PIPE_H
#define KATANA_PIPE_H

#include <string>

#include <FnDisplayDriver/suite/FnDisplayDriverSuite.h>
#include <FnPluginSystem/FnPluginSystem.h>

#include "Message.h"
#include "NewChannelMessage.h"

namespace Foundry
{
  namespace Katana
  {
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

      /** @brief Set the Display Driver Host to delegate all communication
       * details to the suite.
       *
       * The KatanaPipeSingleton bootstraps and sets the host suite, and is
       * recommended to initialize a communication pipe.
       */
      static FnPlugStatus setHost(FnPluginHost *host);

      static void setSuite(const FnDisplayDriverHostSuite_v1 *suite);

      /** @brief Get the Display Driver host suite for communication pipes. If
       * invalid, an error message is output to the standard error stream.
       *
       * @return the Display Driver host suite.
       */
      static const FnDisplayDriverHostSuite_v1 *getSuite();

    private:
      // Disable copy and assignment
      KatanaPipe(const KatanaPipe &rhs);
      KatanaPipe& operator=(const KatanaPipe &rhs);

      FnKatanaPipeHandle _handle;

      static const FnDisplayDriverHostSuite_v1 *_suite;
    };
  };
};

#endif
