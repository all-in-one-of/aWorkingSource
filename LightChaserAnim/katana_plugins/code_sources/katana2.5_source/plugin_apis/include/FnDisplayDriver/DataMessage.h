// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef DATA_MESSAGE_H
#define DATA_MESSAGE_H

#include <stdint.h>

#include "Message.h"

namespace Foundry
{
  namespace Katana
  {
    /** @addtogroup DD
     *  @{
     */

    class NewChannelMessage;

    /** @brief The DataMessage class encapsulates the message sent to the Katana
     * Catalog Server (KCS) which actually contains image data for a given
     * channel.
     *
     * The DataMessage message should be sent down the KatanaPipe after a
     * NewFrame & NewChannel messages have been sent.
     *
     * A DataMessage represents a region of pixel data within a particular
     * channel. The region may be as small as 1 pixel or as large as the whole
     * channel. i.e. you can send the entire channel's data in on DataMessage.
     *
     * Any chunks of data passed to the DataMessage will be copied by the
     * instance and sent asynchronously on an I/O thread.
     */
    class DataMessage : public Message
    {
    public:

      /** @brief Create a new instance of a DataMessage message using an existing
       * NewChannel object to obtain the unique frame ID and channel ID.
       *
       * The default constructor will create a DataMessage belonging to the
       * specified channel with all values set to 0 and an empty data buffer.
       *
       * @param[in] channel    an existing NewChannel object
       * @param[in] xMin       the starting x coordinate in the channel relative
       *                       to the channel's origin
       * @param[in] width      the number of pixels wide this data extends for.
       * @param[in] yMin       the starting y coordinate in the channel relative
       *                       to the channel's origin.
       * @param[in] height     the number of pixels high this data extends for.
       *
       * @param[in] byteSkip   number of bytes to skip over to get to next pixel
       *                       data (this is also stored in the channel data but,
       *                       added here if the data packets differ for any
       *                       reason).
       *
       * @sa setData()
       */
      DataMessage(const NewChannelMessage &channel,
                  uint32_t xMin = 0,
                  uint32_t width = 0,
                  uint32_t yMin = 0,
                  uint32_t height = 0,
                  uint32_t byteSkip = 0);

      /** @brief Create a new instance of a DataMessage message using a pointer
       * to a 16 byte array containing a unique frame ID.
       *
       * The default constructor will create a DataMessage belonging to the
       * specified channel with all values set to 0 and an empty data buffer.
       *
       * @param[in] frameID    a pointer to a 16 byte array containing a unique
       *                       frameID
       * @param[in] channelID  the channel ID for the channel this data belongs
       *                       to.
       * @param[in] xMin       the starting x coordinate in the channel relative
       *                       to the channel's origin
       * @param[in] width      the number of pixels wide this data extends for.
       * @param[in] yMin       the starting y coordinate in the channel relative
       *                       to the channel's origin.
       * @param[in] height     the number of pixels high this data extends for.
       * @param[in] byteSkip   number of bytes to skip over to get to next pixel
       *                       data (this is also stored in the channel data but,
       *                       added here if the data packets differ for any
       *                       reason).
       *
       * @sa setData()
       */
      DataMessage(const uint8_t *frameID,
                  uint16_t channelID = 0,
                  uint32_t xMin = 0,
                  uint32_t width = 0,
                  uint32_t yMin = 0,
                  uint32_t height = 0,
                  uint32_t byteSkip = 0);

      /** @brief Destroy the DataMessage instance and free any resources it
       * holds.
       */
      virtual ~DataMessage() {}

      /** @brief Set the data payload for this DataMessage.
       *
       * Sets the data payload for this message to the buffer pointed to by data
       * by copying it into an internal buffer.
       *
       * Data pointed to by data will remain under the control of the caller.
       *
       * Note: 4-channel color data is expected to be encoded as ARGB.
       *
       * @param[in] data - pointer to the array data
       * @param[in] size - size of the data to be sent.
       */
      void setData(const void *data, uint32_t size);
      void setStartCoordinates(uint32_t x, uint32_t y);
      void setDataDimensions(uint32_t width, uint32_t height);
      void setByteSkip(uint32_t byteSkip);

      /** @brief Get the unique 128 bit identifier that is assigned to the frame
       * this data message is a member of.
       *
       * @return a pointer to the start of the 16 byte array that contains the
       * unique frame ID.
       */
      const uint8_t* frameUUID() const;
      uint16_t channelID() const;
      uint32_t xMin() const;
      uint32_t width() const;
      uint32_t yMin() const;
      uint32_t height() const;
      uint32_t bufferSize() const;
      const void * dataBuffer() const;
      uint32_t byteSkip() const;

    private:
      DataMessage();
      DataMessage(const DataMessage &rhs);
      DataMessage& operator=(const DataMessage &rhs);
    };
    /**
     * @}
     */
  };
};

#endif
