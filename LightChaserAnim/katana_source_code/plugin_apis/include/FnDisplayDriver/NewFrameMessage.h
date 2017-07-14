// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef NEWFRAMEMESSAGE_H
#define NEWFRAMEMESSAGE_H

#include <string>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include "Message.h"
#include "MessageTypes.h"

namespace Foundry
{
  namespace Katana
  {
    /**
     * @addtogroup DD
     * @{
     */

    /**
     * @brief The NewFrameMessage class encapsulates the message sent to the
     * Katana Catalog Server (KCS) to specify that a new frame should be
     * reserved in the Catalog.
     *
     * The NewFrameMessage should be sent down the KatanaPipe first to reserve
     * a frame in the Catalog. A frame may contain zero or more channels.
     * Channels then contain the actual data output by the renderer.
     *
     * A frame represents the collection of these channels at a point in time.
     *
     * Each NewFrameMessage object will be created with a unique 128 bit
     * identifier that will identify the frame being rendered to Katana.
     */
    class NewFrameMessage : public Message
    {
    public:

      /** @brief Create a new NewFrameMessage.
       *
       * The default constructor will create a NewFrameMessage object with a
       * unique frame id and all other values initialised to 0.
       *
       * The frame's name will be initialised to an empty string.
       *
       * @param[in] frameTime the point in time this frame represents
       * @param[in] height    the height in pixels of this frame
       * @param[in] width     the width in pixels of this frame
       * @param[in] xorigin   the xorigin relative to the lower left corner of
       *                      the screen
       * @param[in] yorigin   the yorigin relative to the lower left corner of
       *                      the screen.
       *
       * @sa setFrameName()
       */
      NewFrameMessage(float frameTime = 0.0,
                      uint32_t height = 0,
                      uint32_t width = 0,
                      int xorigin = 0,
                      int yorigin = 0);

      /** @brief Destroy the NewFrameMessage object.
       */
      virtual ~NewFrameMessage();

      /** @brief Called by KatanaPipe to send this NewFrameMessage message down
       *  the pipe.
       *
       * @param[in] socket a valid zeromq socket connection
       *
       * @return 0 if the entire message was sent successfully otherwise
       *         non-zero.
       */
      int send(void *socket) const;

      Message* copy() const;

      /** @brief Set the human readable name that will appear in the name column
       * of the Katana Catalog
       *
       * The maximum number of characters that can be specified in name is 8000.
       *
       * @param[in] name the name that will be sent to the KCS and appear in the
       *                 Catalog
       *
       * @return 0 if the name was set successfully, otherwise nonzero, if
       *         nonzero is returned the existing name will remain unchanged.
       */
      int setFrameName(const std::string &name)
      {
        if(name.size() > MAX_FRAME_NAME_LENGTH)
          return -1;

        this->_frameName = name;

        // Set frameName size using zmq convention which is minus the '\0'
        _newFrame.nameLength = uint32_t( this->_frameName.size() );

        return 0;
      }

      /** @brief Set the frame time.
       *
       * @param[in] time the point in time this frame represents.
       */
      void setFrameTime(float time)
      {
        this->_newFrame.frameTime = time;
      }

      /** @brief Set the width and height of this frame.
       *
       * @param[in] width  the width of this frame in pixels
       * @param[in] height the height of this frame in pixels
       */
      void setFrameDimensions(uint32_t width, uint32_t height)
      {
        this->_newFrame.width = width;
        this->_newFrame.height = height;
      }

      /** @brief Set the origin of this frame
       *
       * @param[in] ox  the x origin relative to the lower left corner of the
       *                screen
       * @param[in] oy  the y origin relative to the lower left corner of
       *                the screen.
       */
      void setFrameOrigin(int ox, int oy)
      {
        this->_newFrame.xorigin = ox;
        this->_newFrame.yorigin = oy;
      }

      /** @brief Get the unique identifier for this frame.
       *
       * @return a pointer to the start of the 16 byte array that contains the
       * unique frame ID.
       */
      const byte* frameUUID() const
      {
        return _newFrame.frameUUID;
      }

      uint32_t frameHeight() const
      {
        return _newFrame.height;
      }

      uint32_t frameWidth() const
      {
        return _newFrame.width;
      }

      int frameXOrigin() const
      {
        return _newFrame.xorigin;
      }

      int frameYOrigin() const
      {
        return _newFrame.yorigin;
      }

      float frameTime() const
      {
        return _newFrame.frameTime;
      }

      const std::string & frameName() const
      {
        return _frameName;
      }

      uint32_t frameNameLength() const
      {
        return _newFrame.nameLength;
      }

#ifdef SOCKET_DEBUG
      friend std::ostream& operator<<(std::ostream &out, const NewFrameMessage &frame);
#endif

    private:
      NewFrameMessage(const NewFrameMessage &rhs);
      NewFrameMessage& operator=(const NewFrameMessage &rhs);

#ifdef SOCKET_DEBUG
      void _printFrameID(std::ostream &out) const;
#endif

      boost::uuids::uuid _tag;

      std::string _frameName;

      RDPHeaderStruct _header;
      NewFrameStruct _newFrame;
    };

    std::ostream& operator<<(std::ostream &out,
                             const Foundry::Katana::NewFrameMessage &frame);

    /** @brief This is a utility function to allow the simple encoding of
     *  legacy frameNames and frame numbers to be used with the latest Render
     *  Data Protocol.
     *
     *  @param[in]  legacyFrameName      the frame name to be encoded.
     *  @param[in]  legacyFrameNumber    the frame number to be encoded.
     *  @param[out] buffer               where the encoding can be found.
     */
    void encodeLegacyName(const std::string &legacyFrameName,
                          int legacyFrameNumber,
                          std::string &buffer);

    /**
     * @}
     */
  };
};

#endif
