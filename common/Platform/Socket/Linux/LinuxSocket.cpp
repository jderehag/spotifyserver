/*
 * Copyright (c) 2012, Jens Nielsen
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL JENS NIELSEN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "../Socket.h"
#include "applog.h"
#include <string.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

typedef struct SocketHandle_t
{
    int fd;
}SocketHandle_t;

static void* INETADDR_ADDRESS( const struct sockaddr* a)
{
    if (a->sa_family == AF_INET6)
    {
        return (void*)&((struct sockaddr_in6*)a)->sin6_addr;
    }
    else
    {
        return (void*)&((struct sockaddr_in*)a)->sin_addr;
    }
}

static void toIpv6( const struct addrinfo* in, struct sockaddr_in6* out)
{
    if (AF_INET == in->ai_family)
    {
        assert(0); // seems like AI_V4MAPPED works on linux, we should not end up here then?
        /*char str[INET_ADDRSTRLEN];
        struct addrinfo Hints, *AddrInfo;
        int RetVal;

        inet_ntop( AF_INET, INETADDR_ADDRESS((struct sockaddr*)in->ai_addr), str, sizeof(str));
        std::string mappedAddr = "::ffff:" + std::string(str);

        log(LOG_EMERG) << "CONVERTING!!!!!!!!!!!!!!!!!!!";
        memset(&Hints, 0, sizeof (Hints));
        Hints.ai_socktype = in->ai_socktype;
        Hints.ai_flags = 0;// | AI_ALL | AI_V4MAPPED;

        Hints.ai_family = PF_INET6;
        RetVal = getaddrinfo(mappedAddr.c_str(), "0", &Hints, &AddrInfo);

        *out = *(struct sockaddr_in6 *)AddrInfo->ai_addr;
        out->sin6_port = ((struct sockaddr_in *)in->ai_addr)->sin_port;*/
    }
    else
    {
        *out = *(struct sockaddr_in6 *) (in->ai_addr);
    }
}

static struct addrinfo* toAddrinfo( const std::string& addr, const std::string& port, bool forBind )
{
    int SocketType = SOCK_STREAM;  // TCP
    int RetVal;
    struct addrinfo Hints, *AddrInfo;

    memset(&Hints, 0, sizeof (Hints));
    Hints.ai_socktype = SocketType;
    Hints.ai_flags = AI_ALL | AI_V4MAPPED;
    if ( forBind ) Hints.ai_flags |= AI_PASSIVE;

    if ( addr == "ANY" )
    {
        Hints.ai_family = PF_INET6;     // Get the IPv6 "any" address
        RetVal = getaddrinfo(NULL, port.c_str(), &Hints, &AddrInfo);
    }
    else
    {
        Hints.ai_family = PF_INET6;    // Request IPv6, we'll get a IPv6 mapped address if addr is IPv4
        RetVal = getaddrinfo(addr.c_str(), port.c_str(), &Hints, &AddrInfo);
    }

    if (RetVal != 0)
    {
        log(LOG_WARN) << "getaddrinfo for addr=" << addr << " failed with error " << RetVal << " " << gai_strerror(RetVal);
        return NULL;
    }

    return AddrInfo;
}


Socket::Socket(SocketHandle_t* socket) : socket_(socket)
{
    int flags;

    /* Set socket non-blocking */
    flags = fcntl( socket_->fd, F_GETFL, 0 );
    fcntl( socket_->fd, F_SETFL, flags | O_NONBLOCK );
}

Socket::Socket(SockType_t type)
{
    int on = 1;
    int flags;

    socket_ = new SocketHandle_t;

    switch(type)
    {
        case SOCKTYPE_STREAM:
            socket_->fd = socket(PF_INET6, SOCK_STREAM, 0);
            break;
        case SOCKTYPE_DATAGRAM:
            socket_->fd = socket(PF_INET6, SOCK_DGRAM, 0);
            break;
        default:
            assert(false);
            break;
    }

    on = 0;
    setsockopt(socket_->fd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&on, sizeof(on) );

    on = 1;
    setsockopt(socket_->fd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));

    /* Set socket non-blocking */
    flags = fcntl( socket_->fd, F_GETFL, 0 );
    fcntl( socket_->fd, F_SETFL, flags | O_NONBLOCK );
}

int Socket::BindToAddr(const std::string& addr, const std::string& port)
{
    char str[INET6_ADDRSTRLEN];
    bool success = false;

    struct addrinfo *AddrInfo, *AI;

    AddrInfo = toAddrinfo( addr, port, true );

    for ( AI = AddrInfo; AI != NULL; AI = AI->ai_next )
    {
        struct sockaddr_in6 bindAddr;

        toIpv6( AI, &bindAddr );

        if ( ( IN6_IS_ADDR_LINKLOCAL((struct in6_addr *) &bindAddr.sin6_addr) ) &&
             ( bindAddr.sin6_scope_id == 0) )
        {
            log(LOG_WARN) << "IPv6 link local addresses should specify a scope ID!";
        }

        inet_ntop( AF_INET6, &bindAddr.sin6_addr, str, sizeof(str));
        log(LOG_DEBUG) << "attempting bind to \"" << addr << "\" -> ip " << str << " port " << ntohs(bindAddr.sin6_port);

        if ( bind( socket_->fd, (struct sockaddr*) &bindAddr, sizeof(bindAddr) ) < 0 )
        {
            log(LOG_EMERG) << "bind attempt failed with error " << strerror(errno);
        }
        else
        {
            success = true;
            break;
        }
    }
    localAddr_ = str;

    if ( AddrInfo )
        freeaddrinfo( AddrInfo );

    return success ? 0 : -1;
}

int Socket::BindToDevice(const std::string& device, const std::string& port)
{
#ifndef __CYGWIN__ //todo
    log(LOG_NOTICE) << "binding to device " << device;
    if (setsockopt(socket_->fd, SOL_SOCKET, SO_BINDTODEVICE, (void *) device.c_str(), device.length()) < 0)
    {
        log(LOG_NOTICE) << "setsockopt failed!";
        return -1;
    }

    return BindToAddr("ANY", port);
#else
    return -1;
#endif
}

int Socket::Connect(const std::string& addr, const std::string& port)
{
    int rc;
    char str[INET6_ADDRSTRLEN];
    bool success = false;

    struct addrinfo *AddrInfo, *AI;

    AddrInfo = toAddrinfo( addr, port, false );
    remoteAddr_ = addr;


    for ( AI = AddrInfo; AI != NULL; AI = AI->ai_next )
    {
        struct sockaddr_in6 connectAddr;

        toIpv6( AI, &connectAddr );

        if ( ( IN6_IS_ADDR_LINKLOCAL((struct in6_addr *) &connectAddr.sin6_addr) ) &&
             ( connectAddr.sin6_scope_id == 0) )
        {
            log(LOG_WARN) << "IPv6 link local addresses should specify a scope ID!";
        }

        inet_ntop( AF_INET6, &connectAddr.sin6_addr, str, sizeof(str));
        log(LOG_DEBUG) << "attempting connect to \"" << addr << "\" -> ip " << str << " port " << ntohs(connectAddr.sin6_port);

        rc = connect( socket_->fd, (struct sockaddr*) &connectAddr, sizeof(connectAddr) );

        if (rc < 0 && errno != EINPROGRESS)
        {
            log(LOG_WARN) << "connect attempt failed with error " << strerror(errno);
        }
        else
        {
            success = true;
            break;
        }
    }

    if ( AddrInfo )
        freeaddrinfo( AddrInfo );

    return success ? WaitForConnect() : -1;
}


int Socket::WaitForConnect()
{
    fd_set fds;
    struct timeval tmo;
    int error;
    socklen_t len = sizeof(error);

    FD_ZERO(&fds);
    FD_SET(socket_->fd, &fds);
    tmo.tv_sec = 5;
    tmo.tv_usec = 0;

    //log(LOG_WARN) << "waiting";

    if (select(socket_->fd+1, NULL, &fds, NULL, &tmo) <= 0)
    {
        return -1;
    }

    if ( getsockopt(socket_->fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 ||
         error > 0 )
    {
        return -1;
    }

    return 0;
}

int Socket::Listen()
{
    return listen(socket_->fd, 5); /*todo 5?*/
}

Socket::~Socket()
{
    Close();
    delete socket_;
}

Socket* Socket::Accept()
{
    struct sockaddr_in6 sockaddr;
    socklen_t len = sizeof(struct sockaddr_in6);
    int newSocket = accept(socket_->fd, (struct sockaddr*)&sockaddr, &len);

    if(newSocket >= 0)
    {
        char str[INET6_ADDRSTRLEN];
        inet_ntop( AF_INET6, INETADDR_ADDRESS((struct sockaddr*)&sockaddr), str, sizeof(str));

        log(LOG_DEBUG) << "accept! " << str << " fd " << newSocket;
        SocketHandle_t* handle = new SocketHandle_t;
        handle->fd = newSocket;
        Socket* sock = new Socket(handle);
        sock->remoteAddr_ = str;
        return sock;
    }
    else
    {
        log(LOG_WARN) << "accept failed: " << strerror(errno);
        return NULL;
    }
}

int Socket::Send(const void* msg, int msgLen)
{
    return send(socket_->fd, msg, msgLen, 0);
}

int Socket::SendTo(const void* msg, int msgLen, const std::string& addr, const std::string& port)
{
    struct addrinfo *AddrInfo;
    struct sockaddr_in6 toAddr;
    int rc = -1;

    AddrInfo = toAddrinfo( addr, port, false );

    if ( AddrInfo )
    {
        toIpv6( AddrInfo, &toAddr );
        rc = sendto(socket_->fd, msg, msgLen, 0, (struct sockaddr*) &toAddr, sizeof(toAddr) );

        freeaddrinfo( AddrInfo );
    }
    return rc;
}


int Socket::Receive(void* buf, int bufLen)
{
    int n = recv(socket_->fd, buf, bufLen, 0);
    if (n < 0 && errno == EWOULDBLOCK)
        return 0;
    else if (n == 0)
        return -1;
    return n;
}

int Socket::ReceiveFrom(void* buf, int bufLen, std::string& addr, std::string& port)
{
    struct sockaddr_in6 sockaddr;
    socklen_t len = sizeof(struct sockaddr_in6);
    char str[INET6_ADDRSTRLEN];
    int n;

    n = recvfrom( socket_->fd, buf, bufLen, 0, (struct sockaddr*)&sockaddr, &len );
    if (n < 0 && errno == EWOULDBLOCK)
        return 0;
    else if (n == 0)
        return -1;

    inet_ntop( AF_INET6, INETADDR_ADDRESS((struct sockaddr*)&sockaddr), str, sizeof(str));
    addr = str;

    std::ostringstream portStr;
    portStr << ntohs(sockaddr.sin6_port);
    port = portStr.str();

    return n;
}

void Socket::Close()
{
    close(socket_->fd);
}

void Socket::Shutdown()
{
    shutdown(socket_->fd, SHUT_RD);
}

int select(std::set<Socket*>* readsockets, std::set<Socket*>* writesockets, std::set<Socket*>* errsockets, int timeout)
{
    int maxfd = 0;
    int rc;
    fd_set readfds, writefds, errfds;
    std::set<Socket*>::iterator it;
    struct timeval tmo;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&errfds);

    if (readsockets)
    {
        for (it = readsockets->begin(); it != readsockets->end(); it++)
        {
            FD_SET((*it)->socket_->fd, &readfds);
            if ((*it)->socket_->fd > maxfd) maxfd = (*it)->socket_->fd;
        }
    }

    if (writesockets)
    {
        for (it = writesockets->begin(); it != writesockets->end(); it++)
        {
            FD_SET((*it)->socket_->fd, &writefds);
            if ((*it)->socket_->fd > maxfd) maxfd = (*it)->socket_->fd;
        }
    }

    if (errsockets)
    {
        for (it = errsockets->begin(); it != errsockets->end(); it++)
        {
            FD_SET((*it)->socket_->fd, &errfds);
            if ((*it)->socket_->fd > maxfd) maxfd = (*it)->socket_->fd;
        }
    }

    tmo.tv_sec = timeout/1000;
    tmo.tv_usec = (timeout % 1000) * 1000;

    if ( (rc = select(maxfd+1, &readfds, &writefds, &errfds, &tmo) ) < 0 )
    {
        log(LOG_WARN) << "select() failed";
        return rc;
    }

    if (readsockets)
    {
        it = readsockets->begin();
        while (it != readsockets->end())
        {
            if (!FD_ISSET((*it)->socket_->fd, &readfds))
                readsockets->erase(it++);
            else
                it++;
        }
    }

    if (writesockets)
    {
        it = writesockets->begin();
        while (it != writesockets->end())
        {
            if (!FD_ISSET((*it)->socket_->fd, &writefds))
            {
                writesockets->erase(it++);
            }
            else
                it++;
        }
    }

    if (errsockets)
    {
        it = errsockets->begin();
        while (it != errsockets->end())
        {
            if (!FD_ISSET((*it)->socket_->fd, &errfds))
                errsockets->erase(it++);
            else
                it++;
        }
    }

    return rc;
}


uint32_t Ntohl(uint32_t x)
{
    return ntohl(x);
}

uint16_t Ntohs(uint16_t x)
{
    return ntohs(x);
}

uint32_t Htonl(uint32_t x)
{
    return htonl(x);
}

uint16_t Htons(uint16_t x)
{
    return htons(x);
}

