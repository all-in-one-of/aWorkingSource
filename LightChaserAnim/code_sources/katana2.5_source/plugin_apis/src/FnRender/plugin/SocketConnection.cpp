// Copyright (c) 2015 The Foundry Visionmongers Ltd. All Rights Reserved.
#if defined(_WIN32)
#error "SocketConnection.cpp is obsolete, and not available on Windows."
#endif

#include "FnRender/plugin/SocketConnection.h"

#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <sstream>

/*!
 * Returns a monotonically-increasing timestamp, in milliseconds.
 */
static int64_t get_monotonic_milliseconds()
{
    struct timespec ts;
    int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(ret == 0);
    return ts.tv_sec * 1000ll + ts.tv_nsec / 1000000ll;
}

/*!
 * Given the timeout for some event in milliseconds and a start time of that
 * event in milliseconds based on the CLOCK_MONOTONIC clock, calculates the
 * remaining timeout in milliseconds.
 */
static int64_t calculate_timeout(int64_t timeout, int64_t startTime)
{
    int64_t elapsedTime = get_monotonic_milliseconds() - startTime;
    return elapsedTime > 0 ? elapsedTime : 0;
}

namespace Foundry
{
namespace Katana
{
namespace Render
{
namespace SocketConnection
{

size_t socket_read(int sockfd, void *buffer, size_t n)
{
    size_t totalRead = 0;
    int timeout = 60 * 1000;  // in millis

    while (n)
    {
        struct pollfd fds[1] = {};
        fds[0].fd = sockfd;
        fds[0].events = POLLIN;

        const int64_t startTime = get_monotonic_milliseconds();
        int retval;
        bool wasInterrupted;
        do
        {
            retval = poll(fds, 1, timeout);
            wasInterrupted = retval == -1 && errno == EINTR;
            if (wasInterrupted)
            {
                timeout =
                    static_cast<int>(calculate_timeout(timeout, startTime));
            }
        } while (wasInterrupted);

        if (retval < 0)
            break;

        ssize_t numRead = 0;
        do
        {
            numRead = read(sockfd, buffer, n);
        } while (numRead == -1 && errno == EINTR);

        if (numRead <= 0)
            break;

        totalRead += numRead;
        n -= numRead;
        buffer = (char *) buffer + numRead;
    }

    return totalRead;
}

bool parse_hostname(const char* hostnameString, char* hostName, unsigned long* portNumber)
{
    while (*hostnameString == '_') ++hostnameString;

    const char* p = hostnameString;
    while ((p - hostnameString < 4096) && *p && ':' != *p)
    {
        ++p;
    }

    if (*p != ':')
    {
        return false;
    }

    std::memcpy(hostName, hostnameString, p - hostnameString);
    hostName[p - hostnameString] = '\0';

    char* dummy;
    *portNumber = std::strtoul(p+1, &dummy, 0);

    return true;
}

int connect_socket(const char* hostnameString)
{
    char hostName[4096];
    unsigned long portNumber;

    if (!parse_hostname(hostnameString, hostName, &portNumber))
        return 0;

    struct hostent *hp;
    if (0 == (hp = gethostbyname(hostName)))
    {
        if (0 == (hp = gethostbyname("localhost")))
        {
            return 0;
        }
    }

    struct sockaddr_in sin;
    std::memset((char*)&sin, 0, sizeof(sin));
    sin.sin_family = hp->h_addrtype;
    std::memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
    sin.sin_port = htons(portNumber);

    int sock;
    if (-1 == (sock = socket(PF_INET, SOCK_STREAM, 0)))
    {
        return 0;
    }

    if (-1 == connect(sock, (sockaddr*)&sin, sizeof(sin)))
    {
        return 0;
    }

    return sock;
}

size_t socket_write(int sockfd, const void* buffer, size_t n)
{
    size_t totalWritten = 0;
    int timeout = 60 * 1000;  // in millis

    while (n)
    {
        struct pollfd fds[1] = {};
        fds[0].fd = sockfd;
        fds[0].events = POLLOUT;

        const int64_t startTime = get_monotonic_milliseconds();
        int retval;
        bool wasInterrupted;
        do
        {
            retval = poll(fds, 1, timeout);
            wasInterrupted = retval == -1 && errno == EINTR;
            if (wasInterrupted)
            {
                timeout =
                    static_cast<int>(calculate_timeout(timeout, startTime));
            }
        } while (wasInterrupted);

        if (retval < 0)
            break;

        ssize_t numWritten = 0;
        do
        {
            numWritten = write(sockfd, buffer, n);
        } while (numWritten == -1 && errno == EINTR);

        if (numWritten <= 0)
            break;

        totalWritten += numWritten;
        n -= numWritten;
        buffer = (const char *) buffer + numWritten;
    }

    return totalWritten;
}

}
}
}
}
