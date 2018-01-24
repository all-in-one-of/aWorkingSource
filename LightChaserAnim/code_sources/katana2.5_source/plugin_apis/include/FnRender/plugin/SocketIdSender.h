// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef FNRENDER_SOCKETIDSENDER_H_
#define FNRENDER_SOCKETIDSENDER_H_
#include <stdint.h>
#include <string>
#include <stdexcept>

#include "FnRender/FnRenderAPI.h"
#include "FnRender/plugin/IdSenderInterface.h"
#include "FnRender/suite/FnRenderServicesSuite.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{
class FNRENDER_API SocketIdSender : public IdSenderInterface
{
public:
    SocketIdSender(const std::string& host, int64_t frameID);
    virtual ~SocketIdSender();

    void getIds(int64_t* nextId, int64_t* maxId);
    void send(int64_t id, const char* const objectName);

    class FNRENDER_API SocketIdSenderException : public std::runtime_error
    {
    public:
        explicit SocketIdSenderException(const char* message)
            : std::runtime_error(message)
        {
        }
    };

private:
    const FnRenderServicesSuite_v1* getSuite();

    FnSocketIdSenderHandle _handle;
    std::string _host;
    int64_t _frameID;
    int _socketId;

    void connect_if_first_time();

    SocketIdSender(const SocketIdSender&);
    SocketIdSender& operator=(const SocketIdSender&);
};
}  // namespace Render
}  // namespace Katana
}  // namespace Foundry
#endif  // FNRENDER_SOCKETIDSENDER_H_
