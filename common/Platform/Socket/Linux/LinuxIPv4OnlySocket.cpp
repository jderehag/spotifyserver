/*
* Copyright (c) 2012, Jens Nielsen
* All rights reserved.

* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* * Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of the <organization> nor the
* names of its contributors may be used to endorse or promote products
* derived from this software without specific prior written permission.
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
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

typedef struct SocketHandle_t
{
    int fd;
}SocketHandle_t;

Socket::Socket(SocketHandle_t* socket) : socket_(socket)
{
    int flags;

    /* Set socket non-blocking */
    flags = fcntl( socket_->fd, F_GETFL, 0 );
    fcntl( socket_->fd, F_SETFL, flags | O_NONBLOCK );
}

Socket::Socket( SockType_t type )
{
    int on = 1;
    int flags;

    socket_ = new SocketHandle_t;

    switch(type)
    {
        case SOCKTYPE_STREAM:
            socket_->fd = socket(PF_INET, SOCK_STREAM, 0);
            break;
        case SOCKTYPE_DATAGRAM:
            socket_->fd = socket(PF_INET, SOCK_DGRAM, 0);
            break;
        default:
            assert(false);
            break;
    }

    setsockopt(socket_->fd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));

    /* Set socket non-blocking */
    flags = fcntl( socket_->fd, F_GETFL, 0 );
    fcntl( socket_->fd, F_SETFL, flags | O_NONBLOCK );
}

int Socket::BindToAddr( const std::string& addr, const std::string& port )
{
    struct sockaddr_in my_addr;

    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons( strtol( port.c_str(), NULL, 10 ) );

    if (addr != "ANY" && inet_addr(addr.c_str()) != INADDR_NONE)
    {
        my_addr.sin_addr.s_addr = inet_addr(addr.c_str());
    }

    log(LOG_NOTICE) << "binding to " << addr << " -> " << inet_ntoa(my_addr.sin_addr) << " port " << port;

    if (bind(socket_->fd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0)
    {
        log(LOG_EMERG) << "Error on bind!";
        return -1;
    }

    return 0;
}

int Socket::BindToDevice( const std::string& device, const std::string& port )
{
    struct sockaddr_in my_addr;

    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons( strtol( port.c_str(), NULL, 10 ) );

#ifndef __CYGWIN__ //todo
    log(LOG_NOTICE) << "binding to device " << device;
    if (setsockopt(socket_->fd, SOL_SOCKET, SO_BINDTODEVICE, (void *) device.c_str(), device.length()) < 0)
    {
        log(LOG_NOTICE) << "setsockopt failed!";
        return -1;
    }

    if (bind(socket_->fd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0)
    {
        log(LOG_EMERG) << "Error on bind!";
        return -1;
    }

    return 0;
#else
    return -1;
#endif
}

int Socket::Connect( const std::string& addr, const std::string& port )
{
    int rc;
    struct sockaddr_in serv_addr;
    memset( &serv_addr, 0, sizeof(struct sockaddr_in) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr( addr.c_str() );
    serv_addr.sin_port = htons( strtol( port.c_str(), NULL, 10 ) );
    rc = connect( socket_->fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr) );

    if (rc < 0 && errno != EINPROGRESS)
        return -1;

    remoteAddr_ = addr;

    return WaitForConnect();
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
    struct sockaddr_in sockaddr;
    socklen_t len = sizeof(struct sockaddr_in);
    int newSocket = accept(socket_->fd, (struct sockaddr*)&sockaddr, &len);

    if(newSocket >= 0)
    {
        std::string str = inet_ntoa(sockaddr.sin_addr);
        log(LOG_NOTICE) << "accept! " << str << " fd " << newSocket;
        SocketHandle_t* handle = new SocketHandle_t;
        handle->fd = newSocket;

        Socket* sock =  new Socket(handle);
        sock->remoteAddr_ = str;
        return sock;
    }
    else
    {
        log(LOG_NOTICE) << "accept " << strerror(errno);
        return NULL;
    }
}

int Socket::Send(const void* msg, int msgLen)
{
    return send(socket_->fd, msg, msgLen, 0);
}

int Socket::SendTo(const void* msg, int msgLen, const std::string& addr, const std::string& port)
{
    struct sockaddr_in toAddr;
    int rc = -1;
    memset( &toAddr, 0, sizeof(struct sockaddr_in) );

    toAddr.sin_family = AF_INET;
    toAddr.sin_port = htons( strtol( port.c_str(), NULL, 10 ) );
    if ( addr == "255.255.255.255" )
    {
        int on = 1;
        setsockopt(socket_->fd, SOL_SOCKET, SO_BROADCAST, (char*) &on, sizeof(on));
        toAddr.sin_addr.s_addr = INADDR_BROADCAST;
    }
    else
    {
        toAddr.sin_addr.s_addr = inet_addr( addr.c_str() );
    }

    log(LOG_NOTICE) << "sendto " << toAddr.sin_addr.s_addr << " : " << toAddr.sin_port;

    rc = sendto(socket_->fd, msg, msgLen, 0, (struct sockaddr*) &toAddr, sizeof(toAddr) );

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
    struct sockaddr_in sockaddr;
    socklen_t len = sizeof(struct sockaddr_in6);
    int n;

    n = recvfrom( socket_->fd, buf, bufLen, 0, (struct sockaddr*)&sockaddr, &len );
    if (n < 0 && errno == EWOULDBLOCK)
        return 0;
    else if (n == 0)
        return -1;

    addr = inet_ntoa(sockaddr.sin_addr);

    std::ostringstream portStr;
    portStr << ntohs(sockaddr.sin_port);
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
                log(LOG_WARN) << "nooooo ";
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

