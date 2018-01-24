// Copyright (c) 2016 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FnDisplayDriverSuite_H
#define FnDisplayDriverSuite_H

#include <stdint.h>

extern "C" {

/** @brief Blind declaration of a katana pipe handle
*/
typedef struct FnKatanaPipeStruct * FnKatanaPipeHandle;
/** @brief Blind declaration of a message handle
*/
typedef struct FnMessageStruct * FnMessageHandle;

typedef int32_t FnKatMessageType;
#define kFnKatNewFrameType 0
#define kFnKatNewChannelType 1
#define kFnKatDataMessageType 2
#define kFnKatCloseMessageType 3
#define kFnKatEndOfRenderMessageType 4
#define kFnKatIsAliveMessageType 5

#define FnDisplayDriverHostSuite_version 1

/** @brief Display Driver suite

This suite provides functions needed by a display driver plugin.

Any FnDisplayDriverHandle instance returned from a function in this suite will
already have a single reference. It is the responsibility of the caller to
release the message by calling releaseMessage().
*/
struct FnDisplayDriverHostSuite_v1
{
    // KatanaPipe
    FnKatanaPipeHandle (*createKatanaPipe)(const char * hostName,
        uint16_t portNumber, const uint64_t maxQueueSize);
    uint8_t (*connect)(FnKatanaPipeHandle handle);
    uint8_t (*send)(FnKatanaPipeHandle handle, FnMessageHandle message);
    void (*flushPipe)(FnKatanaPipeHandle handle, FnMessageHandle channel);
    void (*closeChannel)(FnKatanaPipeHandle handle, FnMessageHandle channel);
    void (*terminate)(FnKatanaPipeHandle handle);

    // Message
    void (*retainMessage)(FnMessageHandle handle);
    void (*releaseMessage)(FnMessageHandle handle);

    // CloseMessage
    FnMessageHandle (*createCloseMessage)(const uint8_t *frameID,
        uint16_t channelID);

    // EndOfRenderMessage
    FnMessageHandle (*createEndOfRenderMessage)(const uint8_t *frameID,
        uint16_t channelID);

    // DataMessage
    FnMessageHandle (*createDataMessage)(const uint8_t *frameID,
        uint16_t channelID,
        uint32_t xMin, uint32_t width,
        uint32_t yMin, uint32_t height,
        uint32_t byteSkip);
    void (*setData)(FnMessageHandle handle, const void *data, uint32_t size);
    void (*setStartCoordinates)(FnMessageHandle handle, uint32_t x, uint32_t y);
    void (*setDataDimensions)(FnMessageHandle handle,
        uint32_t width, uint32_t height);
    void (*setByteSkip)(FnMessageHandle handle, uint32_t byteSkip);
    const uint8_t * (*dataFrameUUID)(FnMessageHandle handle);
    uint16_t (*dataChannelID)(FnMessageHandle handle);
    uint32_t (*xMin)(FnMessageHandle handle);
    uint32_t (*width)(FnMessageHandle handle);
    uint32_t (*yMin)(FnMessageHandle handle);
    uint32_t (*height)(FnMessageHandle handle);
    uint32_t (*bufferSize)(FnMessageHandle handle);
    const void * (*dataBuffer)(FnMessageHandle handle);
    uint32_t (*byteSkip)(FnMessageHandle handle);

    // NewChannelMessage
    FnMessageHandle (*createNewChannelMessage)(const uint8_t *frameID,
        uint16_t channelID,
        uint32_t height, uint32_t width,
        int xorigin, int yorigin,
        float sampleRate1, float sampleRate2,
        uint32_t dataSize);
    int8_t (*setChannelName)(FnMessageHandle handle, const char * name);
    void (*setChannelDimensions)(FnMessageHandle handle,
        uint32_t width, uint32_t height);
    void (*setChannelOrigin)(FnMessageHandle handle, int32_t ox, int32_t oy);
    void (*setDataSize)(FnMessageHandle handle, uint32_t dataSize);
    void (*setChannelID)(FnMessageHandle handle, uint16_t channelID);
    void (*setSampleRate)(FnMessageHandle handle, const float *sampleRate);
    const uint8_t * (*channelFrameUUID)(FnMessageHandle handle);
    uint16_t (*channelID)(FnMessageHandle handle);
    uint32_t (*channelHeight)(FnMessageHandle handle);
    uint32_t (*channelWidth)(FnMessageHandle handle);
    int32_t (*channelXOrigin)(FnMessageHandle handle);
    int32_t (*channelYOrigin)(FnMessageHandle handle);
    uint32_t (*channelDataSize)(FnMessageHandle handle);
    const char * (*channelName)(FnMessageHandle handle);
    uint32_t (*channelNameLength)(FnMessageHandle handle);
    const float * (*sampleRate)(FnMessageHandle handle);

    // NewFrameMessage
    FnMessageHandle (*createNewFrameMessage)(float frameTime,
        uint32_t height, uint32_t width,
        int32_t xorigin, int32_t yorigin);
    int8_t (*setFrameName)(FnMessageHandle handle, const char * name);
    void (*setFrameTime)(FnMessageHandle handle, float time);
    void (*setFrameDimensions)(FnMessageHandle handle,
        uint32_t width, uint32_t height);
    void (*setFrameOrigin)(FnMessageHandle handle, int32_t ox, int32_t oy);
    const uint8_t * (*frameUUID)(FnMessageHandle handle);
    uint32_t (*frameHeight)(FnMessageHandle handle);
    uint32_t (*frameWidth)(FnMessageHandle handle);
    int32_t (*frameXOrigin)(FnMessageHandle handle);
    int32_t (*frameYOrigin)(FnMessageHandle handle);
    float (*frameTime)(FnMessageHandle handle);
    const char * (*frameName)(FnMessageHandle handle);
    uint32_t (*frameNameLength)(FnMessageHandle handle);
};

}
#endif // FnDisplayDriverSuite_H
