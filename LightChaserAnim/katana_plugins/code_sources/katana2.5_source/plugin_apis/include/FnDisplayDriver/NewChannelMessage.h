// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NEW_CHANNEL_MESSAGE_H
#define NEW_CHANNEL_MESSAGE_H

#include <stdint.h>
#include <string>

#include "Message.h"

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
      NewChannelMessage(const uint8_t *frameID,
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
      virtual ~NewChannelMessage() {}

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
      int setChannelName(const std::string &name);

      /** @brief Set the width and height of this channel.
       *
       * @param[in] width  the width of this channel in pixels
       * @param[in] height the height of this channel in pixels
       */
      void setChannelDimensions(uint32_t width, uint32_t height);

      /** @brief Set the origin of this channel
       *
       * @param[in] ox  the x origin relative to the x origin of the frame to
       *                which this channel belongs to.
       * @param[in] oy  the y origin relative to the y origin of the frame to
       *                which this channel belongs to.
       */
      void setChannelOrigin(int ox, int oy);

      /** @brief Set the size - in bytes - of a pixels worth of data in this
       * channel.
       *
       * For example - if for each pixel this channel contains 4-channel color
       * values stored as float values the size would be 4 * sizeof(float) and
       * thus setDataSize(16).
       *
       * @param[in] dataSize the size in bytes of a pixel of data.
       */
      void setDataSize(uint32_t dataSize);

      /** @brief Set the channel ID to identify this channel within a frame.
       *
       * @param[in] channelID the channel ID for this channel.
       */
      void setChannelID(uint16_t channelID);

      void setSampleRate(const float *sampleRate);

      /** @brief Get the unique 128 bit identifier that is assigned to the frame
       * this channel is a member of.
       *
       * @return a pointer to the start of the 16 uint8_t array that contains
       * the unique frame ID.
       */
      const uint8_t* frameUUID() const;

      uint16_t channelID() const;
      uint32_t channelHeight() const;
      uint32_t channelWidth() const;
      int channelXOrigin() const;
      int channelYOrigin() const;
      uint32_t channelDataSize() const;
      const std::string & channelName() const;
      uint32_t channelNameLength() const;
      const float *sampleRate() const;

    private:
      NewChannelMessage();
      NewChannelMessage(const NewChannelMessage& rhs);
      NewChannelMessage& operator=(const NewChannelMessage &rhs);

      mutable std::string _channelName;
    };
    /**
     * @}
     */
};
};

#endif
