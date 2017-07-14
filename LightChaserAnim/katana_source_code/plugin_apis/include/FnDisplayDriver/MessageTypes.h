// Copyright (c) 2012 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef KATANA_RENDER_DATA_PROTOCOL_TYPES
#define KATANA_RENDER_DATA_PROTOCOL_TYPES

#include <ctime>

namespace Foundry
{
  namespace Katana
  {

  /***************************************************************************
   *                              TYPE DEFINITIONS
   **************************************************************************/
    typedef unsigned char byte;
    typedef unsigned short int uint16_t;
    typedef unsigned int uint32_t;
    typedef unsigned long uint64_t;

    const unsigned int FRAME_UUID_LENGTH = 16;

    // Define values for the RDPHeaders msgType field.
    enum MessageTypes
    {
      NewFrameType = 0,
      NewChannelType = 1,
      DataMessageType = 2,
      CloseMessageType = 3,
      EndOfRenderMessageType = 4,
      IsAliveMessageType = 5,
    };

    /***************************************************************************
     *                  ON THE WIRE PACKET DEFINITIONS
     **************************************************************************/

    /*
     * All messages sent from either client or server will start with
     * an RDPHeader that will identify the rest of the message
     */
    typedef struct RDPHeaderStruct
    {
      byte        magicNumber;
      byte        versionNumber;
      byte        msgType;
      byte        zeroPadding;
    } RDPHeaderStruct;

    /*
     * Used to instruct the Katana Catalog that about a new frame.
     */
    typedef struct NewFrameStruct
    {
      byte        frameUUID[FRAME_UUID_LENGTH];
      float       frameTime;
      uint32_t    height;
      uint32_t    width;
      int         xorigin;
      int         yorigin;
      uint32_t    nameLength;
    } NewFrameStruct;

    /*
     * Used to instruct the Katana Catalog about a new channel.
     */
    typedef struct NewChannelStruct
    {
      byte        frameUUID[FRAME_UUID_LENGTH];
      uint16_t    channelID;
      uint16_t    zeroPadding;
      uint32_t    height;
      uint32_t    width;
      int         xorigin;
      int         yorigin;
      float       sampleRate[2];
      uint32_t    dataSize;
      uint32_t    channelNameLength;
      uint32_t    alignmentPadding;
    } NewChannelStruct;

    /*
     * Used to describe a block of data output by a render that is sent to
     * Katana.
     */
    typedef struct DataMessageStruct
    {
      byte        frameUUID[FRAME_UUID_LENGTH];
      uint16_t    channelID;
      uint16_t    zeroPadding;
      uint32_t    xMin;
      uint32_t    width;
      uint32_t    yMin;
      uint32_t    height;
      uint32_t    byteSkip;
      uint32_t    blockSize;
    } DataMessageStruct;

    /*
     * Final message in a given sequence of messages that instructs Katana that
     * no more data for the specified channel will be sent and it can clean
     * its data strucutres up. Any further information sent on this frame/channel
     * will be ignored.
     */
    typedef struct EndOfRenderMessageStruct
    {
      byte        frameUUID[FRAME_UUID_LENGTH];
      uint16_t    channelID;
      uint16_t    zeroPadding;
    } EndOfRenderMessageStruct;

    /*
     * A synchronisation message allowing renderers to wait for an empty image
     * buffer.
     */
    typedef struct CloseMessageStruct
    {
      byte        frameUUID[FRAME_UUID_LENGTH];
      uint16_t    channelID;
      uint16_t    zeroPadding;
    } CloseMessageStruct;

    typedef struct IsAliveMessageStruct
    {
      time_t      aliveAt;
    } IsAliveMessageStruct;

    /***************************************************************************
     * RESERVED FOR FUTURE USE
     **************************************************************************/
    typedef struct IDQuestionStruct
    {
      byte        frameUUID[FRAME_UUID_LENGTH];
      uint16_t    channelID;
      uint16_t    zeroPadding;
      uint32_t    nameLength;
    } IDQuestionStruct;

    typedef struct IDAnswerStruct
    {
      byte        frameUUID[FRAME_UUID_LENGTH];
      uint16_t    channelID;
      uint16_t    zeroPadding;
      uint32_t    alignmentPadding;
      uint64_t    ID;
      uint32_t    nameLength;
      uint32_t    alignmentPadding0;
    } IDAnswerStruct;
   };
};


#endif
