// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/IdSenderFactory.h"
#include "FnRender/plugin/SocketIdSender.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{
namespace IdSenderFactory
{

IdSenderInterface* getNewInstance(const std::string & host, int64_t frameID)
{
    return new SocketIdSender(host, frameID);
}

}
}
}
}
