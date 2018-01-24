// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef FNRENDER_FNRENDERSERVICESSUITE_H_
#define FNRENDER_FNRENDERSERVICESSUITE_H_
#include <stdint.h>
#if !defined(_MSC_VER) || _MSC_VER >= 1800 /* VS 2013 */
#include <stdbool.h>
#endif

typedef struct FnSocketIdSenderStruct* FnSocketIdSenderHandle;

#define FnRenderServicesSuite_version 1

enum
{
    kFnSocketIdSenderStatus_OK = 0,
    kFnSocketIdSenderStatus_SocketError = 1,
    kFnSocketIdSenderStatus_UnknownError = 2,
};
typedef int FnSocketIdSenderStatus;

struct FnRenderServicesSuite_v1
{
    // SocketIDSender
    FnSocketIdSenderHandle (*socketIdSenderCreate)(const char* host,
                                                   int64_t frameId);
    void (*socketIdSenderDispose)(FnSocketIdSenderHandle handle);
    FnSocketIdSenderStatus (*socketIdSenderSendId)(
        FnSocketIdSenderHandle handle,
        int64_t id,
        const char* objectName);
    FnSocketIdSenderStatus (*socketIdSenderGetIds)(
        FnSocketIdSenderHandle handle,
        int64_t* nextId,
        int64_t* maxId);

    // RendererInfo
    bool (*shaderTypeTagMatchesFilter)(const char* shaderTypeTag,
                                       const char* filter);
};

#endif  // FNRENDER_FNRENDERSERVICESSUITE_H_
