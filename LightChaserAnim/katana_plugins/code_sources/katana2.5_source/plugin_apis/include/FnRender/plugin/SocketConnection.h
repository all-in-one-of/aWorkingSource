// Copyright (c) 2016 The Foundry Visionmongers Ltd.
#ifndef FNRENDER_SOCKETCONNECTION_H
#define FNRENDER_SOCKETCONNECTION_H
#include <cstring>

#ifdef _WIN32
#include <WinSock2.h>
#define close(x) closesocket(x)
#endif //_WIN32

#include "FnRender/FnRenderAPI.h"
#include "FnPlatform/internal/Portability.h"

namespace Foundry
{
namespace Katana
{
namespace Render
{
namespace SocketConnection
{
    /*
    * Given a string of the form "hostname:portNum",
    * copy hostname into the hostName string, and put the port number
    * into portNumber. Ignores any leading "_"
    */
    FNRENDER_API FNKAT_DEPRECATED
    bool parse_hostname(const char* hostnameString, char* hostName, unsigned long* portNumber);
    /*
    * Get a connected socket to "hostname:portNum"
    */
    FNRENDER_API FNKAT_DEPRECATED
    int connect_socket(const char* hostnameString);
    /*
    * Write n bytes to socket sockfd.
    */
    FNRENDER_API FNKAT_DEPRECATED
    size_t socket_write(int sockfd, const void* buffer, size_t n);

    /* Read n bytes from socket sockfd.
    */
    FNRENDER_API FNKAT_DEPRECATED
    size_t socket_read(int sockfd, void *buffer, size_t n);
}
}
}
}

namespace FnKat = Foundry::Katana;

#endif
