// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NEW_CHANNEL_MESSAGE_H
#define NEW_CHANNEL_MESSAGE_H

#include <string>

#include "Message.h"
#include "MessageTypes.h"

namespace Foundry
{
  namespace Katana
  {
    class NewFrameMessage;

    /** @addtogroup DD
     * @{
     */

    /**
     * @brief The NewChannelMessage class encapsulates the message sent to the
     * Katana Catalog Server (KCS) to specify that a new channel should be
     * reserved in the Catalog for a given frame.
     *
     * The NewChannelMessage message should be sent down the KatanaPipe after a
     * NewFrame message has been sent as the NewFrame message will reserve the
     * frame in the Catalog.
     *
     * A channel represents a given output pass by a renderer, such as the
     * primary colour pass, id pass, depth pass P pass etc.
     *
     * Each NewChannelMessage object must be created with with reference to a
     * valid unique frame ID so Katana will know which frame the channel belongs
     * to.
     */
    class NewChannelMessage : public Message
    {
    public:
      /** @brief Create a new instance of NewChannelMessage message using an
       * existing NewFrame object to obtain the unique frame ID.
       *
       * The default constructor will create a new channel belonging to the
       * specified frame with all values set to 0 and an empty string for the
       * channel name.
       *
       * Note: the NewChannelMessage object will NOT maintain a reference to
       * NewFrameMessage or assume ownership of it.
       *
       * @param[in] frame      an existing NewFrameMessage object
       * @param[in] channelID  an ID number to identify this channel within the
       *                       frame. (This can just be sequential from zero).
       * @param[in] height     the height of this channel in pixels.
       * @param[in] width      the width of this channel in pixels.
       * @param[in] xorigin    the x origin of this channel relative to the
       *                       frame's x origin
       * @param[in] yorigin    the y origin of this channel relative to the
       *                       frame's y origin.
       * @param[in] sampleRate1 sampleRate1
       * @param[in] sampleRate2 sampleRate2
       * @param[in] dataSize   the size - in bytes - of a single pixel of data
       *                       for this channel.
       *
       * @sa setChannelName()
       */
      NewChannelMessage(const NewFrameMessage &frame,
                        uint16_t channelID = 0,
                        uint32_t height = 0,
                        uint32_t width = 0,
                        int xorigin = 0,
                        int yorigin = 0,
                        float sampleRate1 = 0.0,
                        float sampleRate2 = 0.0,
                        uint32_t dataSize = 0);

      /** @brief Create a new instance of a NewChannelMessage specifying a
       * pointer to a 16 byte array that will contain a unique frame ID.
       *
       * The default for other values will create a new channel belonging to
       * the specified frame with all values set to 0 and an empty string for
       * the channel name.
       *
       * @param[in] frameID    a pointer to a 16 byte array containing a unique
       *                       128 bit frame ID.
       * @param[in] channelID  an ID number to identify this channel within the
       *                       frame. (This can just be sequential from zero).
       * @param[in] height     the height of this channel in pixels.
       * @param[in] width      the width of this channel in pixels.
       * @param[in] xorigin    the x origin of this channel relative to the
       *                       frame's x origin
       * @param[in] yorigin    the y origin of this channel relative to the
       *                       frame's y origin.
       * @param[in] sampleRate1 sampleRate1
       * @param[in] sampleRate2 sampleRate2
       * @param[in] dataSize   the size - in bytes - of a single pixel of data
       *                       for this channel.
       *
       * @sa setChannelName()
       */
      NewChannelMessage(const byte *frameID,
                        uint16_t channelID = 0,
                        uint32_t height = 0,
                        uint32_t width = 0,
                        int xorigin = 0,
                        int yorigin = 0,
                        float sampleRate1 = 0.0,
                        float sampleRate2 = 0.0,
                        uint32_t dataSize = 0);

      /** @brief Destroy the NewChannelMessage instance.
       */
      virtual ~NewChannelMessage();

      /** @brief Called by KatanaPipe to send this NewChannelMessage down the
       * pipe.
       *
       * @param[in] socket a valid zeromq socket connection
       *
       * @return 0 if the entire message was sent successfully otherwise
       *         non-zero.
       */
      int send(void *socket) const;

      Message* copy() const;

      /** @brief Set the human readable name that will be seen in the Monitor.
       *
       * The maximum number of characters that can be specified in name is 8000.
       *
       * @param[in] name the name that will be sent to the KCS and appear in the
       *                 Monitor
       *
       * @return 0 if the name was set successfully, otherwise nonzero, if
       *         nonzero is returned the existing name will remain unchanged.
       */
      int setChannelName(const std::string &name)
      {
        if(name.size() > MAX_FRAME_NAME_LENGTH)
          return -1;

        this->_channelName = name;

        _newChannel.channelNameLength = uint32_t( this->_channelName.size() );

        return 0;
      }

      /** @brief Set the width and height of this channel.
       *
       * @param[in] width  the width of this channel in pixels
       * @param[in] height the height of this channel in pixels
       */
      void setChannelDimensions(uint32_t width, uint32_t height)
      {
        this->_newChannel.width = width;
        this->_newChannel.height = height;
      }

      /** @brief Set the origin of this channel
       *
       * @param[in] ox  the x origin relative to the x origin of the frame to
       *                which this channel belongs to.
       * @param[in] oy  the y origin relative to the y origin of the frame to
       *                which this channel belongs to.
       */
      void setChannelOrigin(int ox, int oy)
      {
        this->_newChannel.xorigin = ox;
        this->_newChannel.yorigin = oy;
      }

      /** @brief Set the size - in bytes - of a pixels worth of data in this
       * channel.
       *
       * For example - if for each pixel this channel contains RGBA values
       * stored as float values the size would be 16 and thus setDataSize(16).
       *
       * @param[in] dataSize the size in bytes of a pixel of data.
       */
      void setDataSize(uint32_t dataSize)
      {
        this->_newChannel.dataSize = dataSize;
      }

      /** @brief Set the channel ID to identify this channel within a frame.
       *
       * @param[in] channelID the channel ID for this channel.
       */
      void setChannelID(uint16_t channelID)
      {
        this->_newChannel.channelID = channelID;
      }

      void setSampleRate(const float *sampleRate)
      {
        this->_newChannel.sampleRate[0] = sampleRate[0];
        this->_newChannel.sampleRate[1] = sampleRate[1];
      }

      /** @brief Get the unique 128 bit identifier that is assigned to the frame
       * this channel is a member of.
       *
       * @return a pointer to the start of the 16 byte array that contains the
       * unique frame ID.
       */
      const byte* frameUUID() const
      {
        return _newChannel.frameUUID;
      }

      uint16_t channelID() const
      {
        return _newChannel.channelID;
      }

      uint32_t channelHeight() const
      {
        return _newChannel.height;
      }

      uint32_t channelWidth() const
      {
        return _newChannel.width;
      }

      int channelXOrigin() const
      {
        return _newChannel.xorigin;
      }

      int channelYOrigin() const
      {
        return _newChannel.yorigin;
      }

      uint32_t channelDataSize() const
      {
        return _newChannel.dataSize;
      }

      const std::string & channelName() const
      {
        return _channelName;
      }

      uint32_t channelNameLength() const
      {
        return _newChannel.channelNameLength;
      }

      const float *sampleRate() const
      {
        return _newChannel.sampleRate;
      }

#ifdef SOCKET_DEBUG
      friend std::ostream& operator<<(std::ostream &out,
                                      const NewChannelMessage &frame);
#endif

    private:
      NewChannelMessage();
      NewChannelMessage(const NewChannelMessage& rhs);
      NewChannelMessage& operator=(const NewChannelMessage &rhs);

      void init(const byte *frameID,
                uint16_t channelID,
                uint32_t height,
                uint32_t width,
                int xorigin,
                int yorigin,
                float sampleRate1,
                float sampleRate2,
                uint32_t dataSize);

#ifdef SOCKET_DEBUG
      void _printFrameID(std::ostream &out) const;
#endif

      RDPHeaderStruct _header;
      NewChannelStruct _newChannel;

      std::string _channelName;
    };

#ifdef SOCKET_DEBUG
    std::ostream& operator<<(std::ostream &out, const Foundry::Katana::NewChannelMessage &frame);
#endif
    /**
     * @}
     */
};
};


#endif
