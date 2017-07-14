// Copyright (c) 2013 The Foundry Visionmongers Ltd. All Rights Reserved.

#include "FnRender/plugin/SocketIdSender.h"
#include "FnRender/plugin/SocketConnection.h"

#include <unistd.h>
#include <sstream>

namespace Foundry
{
namespace Katana
{
namespace Render
{

SocketIdSender::SocketIdSender(const std::string & host, int64_t frameID): _host(host), _frameID(frameID), _socketId(0)
{
}

SocketIdSender::~SocketIdSender()
{
    if(_socketId)
    {
        close(_socketId);
    }
}

void SocketIdSender::connect_if_first_time()
{
    if(_socketId == 0)
    {
        _socketId = SocketConnection::connect_socket(_host.c_str());

        if (!_socketId)
        {
            std::ostringstream os;
            os << "Error creating socket connection to host \"" << _host << "\"";
            throw SocketIdSenderException(os.str().c_str());
        }

        int64_t protocolId = 2;   // = CatalogSocketClient::EProtocol::eIdNames

        if(sizeof(protocolId) != SocketConnection::socket_write(_socketId, &protocolId, sizeof(protocolId)))
        {
            throw SocketIdSenderException("Error writing protocol ID to socket");
        }

        if(sizeof(_frameID) != SocketConnection::socket_write(_socketId, &_frameID, sizeof(_frameID)))
        {
            throw SocketIdSenderException("Error writing frame ID to socket");
        }
    }
}

void SocketIdSender::getIds(int64_t* nextId, int64_t* maxId)
{
    connect_if_first_time();

    if (sizeof(nextId) != SocketConnection::socket_read(_socketId, nextId, sizeof(nextId)))
    {
        throw SocketIdSenderException("Error reading next object ID from socket");
    }

    if (sizeof(maxId) != SocketConnection::socket_read(_socketId, maxId, sizeof(maxId)))
    {
       throw SocketIdSenderException("Error reading maximum object ID from socket");
    }
}

void SocketIdSender::send(int64_t id, const char* const objectName)
{
    connect_if_first_time();

    if (sizeof(id) != SocketConnection::socket_write(_socketId, &id, sizeof(id)))
    {
        throw SocketIdSenderException("Error writing object ID to socket");
    }

    int64_t nameLen = sizeof(char) * (strlen(objectName) + 1);

    if (sizeof(nameLen) != SocketConnection::socket_write(_socketId, &nameLen, sizeof(nameLen)))
    {
        throw SocketIdSenderException("Error writing object name length to socket");
    }

    if (static_cast<size_t>(nameLen) != SocketConnection::socket_write(_socketId, objectName, nameLen))
    {
        throw SocketIdSenderException("Error writing object name to socket");
    }
}

}
}
}
