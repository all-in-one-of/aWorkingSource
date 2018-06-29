// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#include "FnRender/plugin/SocketIdSender.h"

#include <stdint.h>
#include <string>

#include "FnPluginManager/FnPluginManager.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{
SocketIdSender::SocketIdSender(const std::string& host, int64_t frameID)
    : _handle(getSuite()->socketIdSenderCreate(host.c_str(), frameID))
{
}

SocketIdSender::~SocketIdSender()
{
    getSuite()->socketIdSenderDispose(_handle);
}

/* static */ const FnRenderServicesSuite_v1* SocketIdSender::getSuite()
{
    static FnPluginManager::LazyHostSuite<FnRenderServicesSuite_v1> suite = {
        "RenderServices", 1
    };
    return suite.get();
}

void SocketIdSender::getIds(int64_t* nextId, int64_t* maxId)
{
    FnSocketIdSenderStatus status;
    status = getSuite()->socketIdSenderGetIds(_handle, nextId, maxId);
}

void SocketIdSender::send(int64_t id, const char* objectName)
{
    FnSocketIdSenderStatus status;
    status = getSuite()->socketIdSenderSendId(_handle, id, objectName);
}
}  // namespace Render
}  // namespace Katana
}  // namespace Foundry
