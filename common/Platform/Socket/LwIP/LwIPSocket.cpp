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

#include "lwip/sockets.h"
#include "applog.h"

#include <string.h>
#include <stdlib.h>     /* atoi */


typedef struct SocketHandle_t
{
    int fd;
}SocketHandle_t;

Socket::Socket(SocketHandle_t* socket) : socket_(socket)
{
    /* Set socket non-blocking */
    int opt = 1;
    lwip_ioctl(socket_->fd, FIONBIO, &opt);

}

Socket::Socket( SockType_t type )
{
    int on = 1;

    socket_ = new SocketHandle_t;

    socket_->fd = lwip_socket(PF_INET, (type == SOCKTYPE_STREAM) ? SOCK_STREAM : SOCK_DGRAM, 0);

    lwip_setsockopt(socket_->fd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));

    /* Set socket non-blocking */
    lwip_ioctl(socket_->fd, FIONBIO, &on);

}

int Socket::BindToAddr(const std::string& addr, const std::string& port)
{
    struct sockaddr_in my_addr;

    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(atoi(port.c_str()));

    if (addr != "ANY" && inet_addr(addr.c_str()) != INADDR_NONE)
    {
        my_addr.sin_addr.s_addr = inet_addr(addr.c_str());
    }

    log(LOG_NOTICE) << "binding to " << addr << " -> " << inet_ntoa(my_addr.sin_addr) << " port " << port;

    if (lwip_bind(socket_->fd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0)
    {
        log(LOG_EMERG) << "Error on bind!";
        return -1;
    }

    return 0;
}

int Socket::BindToDevice(const std::string& device, const std::string& port)
{
    return -1;
}

int Socket::Connect(const std::string& addr, const std::string& port)
{
    int rc;
    struct sockaddr_in serv_addr;
    memset( &serv_addr, 0, sizeof(struct sockaddr_in) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr( addr.c_str() );
    serv_addr.sin_port = htons(atoi(port.c_str()));
    rc = lwip_connect( socket_->fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr) );

    if (rc < 0 && errno == EINPROGRESS)
        rc = 0;

    return WaitForConnect();
}


int Socket::WaitForConnect()
{
    lwip_fd_set fds;
    struct timeval tmo;
    int error;
    socklen_t len = sizeof(error);

    LWIP_FD_ZERO(&fds);
    LWIP_FD_SET(socket_->fd, &fds);
    tmo.tv_sec = 5;
    tmo.tv_usec = 0;

    //log(LOG_WARN) << "waiting";

    if ( lwip_select(socket_->fd+1, NULL, &fds, NULL, &tmo) <= 0)
    {
        return -1;
    }

    if ( lwip_getsockopt(socket_->fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 ||
         error > 0 )
    {
        return -1;
    }

    return 0;
}

int Socket::Listen()
{
    return lwip_listen(socket_->fd, 5); /*todo 5?*/
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
    int newSocket = lwip_accept(socket_->fd, (struct sockaddr*)&sockaddr, &len);

    if(newSocket >= 0)
    {
        log(LOG_NOTICE) << "accept! " << inet_ntoa(sockaddr.sin_addr) << " fd " << newSocket;
        SocketHandle_t* handle = new SocketHandle_t;
        handle->fd = newSocket;
        return new Socket(handle);
    }
    else
    {
        log(LOG_NOTICE) << "accept " << strerror(errno);
        return NULL;
    }
}

int Socket::Send(const void* msg, int msgLen)
{
    return lwip_send(socket_->fd, msg, msgLen, 0);
}

int Socket::Receive(void* buf, int bufLen)
{
    int n = lwip_recv(socket_->fd, buf, bufLen, 0);
    if (n < 0 && errno == EWOULDBLOCK)
        return 0;
    else if (n == 0)
        return -1;
    return n;
}

void Socket::Close()
{
    lwip_close(socket_->fd);
}

void Socket::Shutdown()
{
    lwip_shutdown(socket_->fd, SHUT_RD);
}



int select(std::set<Socket*>* readsockets, std::set<Socket*>* writesockets, std::set<Socket*>* errsockets, int timeout)
{
    int maxfd = 0;
    int rc;
    lwip_fd_set readfds, writefds, errfds;
    std::set<Socket*>::iterator it;
    struct timeval tmo;

    LWIP_FD_ZERO(&readfds);
    LWIP_FD_ZERO(&writefds);
    LWIP_FD_ZERO(&errfds);

    if (readsockets)
    {
        for (it = readsockets->begin(); it != readsockets->end(); it++)
        {
            LWIP_FD_SET((*it)->socket_->fd, &readfds);
            if ((*it)->socket_->fd > maxfd) maxfd = (*it)->socket_->fd;
        }
    }

    if (writesockets)
    {
        for (it = writesockets->begin(); it != writesockets->end(); it++)
        {
            LWIP_FD_SET((*it)->socket_->fd, &writefds);
            if ((*it)->socket_->fd > maxfd) maxfd = (*it)->socket_->fd;
        }
    }

    if (errsockets)
    {
        for (it = errsockets->begin(); it != errsockets->end(); it++)
        {
            LWIP_FD_SET((*it)->socket_->fd, &errfds);
            if ((*it)->socket_->fd > maxfd) maxfd = (*it)->socket_->fd;
        }
    }

    tmo.tv_sec = timeout/1000;
    tmo.tv_usec = (timeout % 1000) * 1000;

    if ( (rc = lwip_select(maxfd+1, &readfds, &writefds, &errfds, &tmo) ) < 0 )
    {
        log(LOG_WARN) << "select() failed";
        return rc;
    }

    if (readsockets)
    {
        it = readsockets->begin();
        while (it != readsockets->end())
        {
            if (!LWIP_FD_ISSET((*it)->socket_->fd, &readfds))
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
            if (!LWIP_FD_ISSET((*it)->socket_->fd, &writefds))
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
            if (!LWIP_FD_ISSET((*it)->socket_->fd, &errfds))
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

uint32_t Htonl(uint32_t x)
{
    return htonl(x);
}

