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

#include <Winsock2.h>
#include "../Socket.h"
#include "applog.h"
#include <stdint.h>

typedef struct SocketHandle_t
{
    SOCKET handle;
}SocketHandle_t;


Socket::Socket(SocketHandle_t* socket) : socket_(socket)
{
    u_long on = 1;

    /* Set socket non-blocking */
    ioctlsocket(socket_->handle, FIONBIO, &on);
}

Socket::Socket()
{
    u_long on = 1;
    WSADATA	wsaData;

    socket_ = new SocketHandle_t;

    WSAStartup(MAKEWORD(2,2), &wsaData);

    socket_->handle = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(socket_->handle, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));

    /* Set socket non-blocking */
    ioctlsocket(socket_->handle, FIONBIO, &on);
}

int Socket::BindToAddr(std::string addr, int port)
{
    struct sockaddr_in my_addr;

    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port);

    if (addr != "ANY" && inet_addr(addr.c_str()) != INADDR_NONE)
    {
        my_addr.sin_addr.s_addr = inet_addr(addr.c_str());
    }

    log(LOG_NOTICE) << "binding to " << addr << " -> " << inet_ntoa(my_addr.sin_addr) << " port " << port;

    if (bind(socket_->handle, (struct sockaddr *) &my_addr, sizeof(my_addr)) == SOCKET_ERROR)
    {
        log(LOG_EMERG) << "Error on bind!";
        return -1;
    }

    return 0;
}

int Socket::BindToDevice(std::string device, int port)
{
    struct sockaddr_in my_addr;

    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port);

#if 0 //todo
    log(LOG_NOTICE) << "binding to device " << device;
    if (setsockopt(socket->handle, SOL_SOCKET, SO_BINDTODEVICE, (void *) device.c_str(), device.length()) == SOCKET_ERROR)
    {
        log(LOG_NOTICE) << "setsockopt failed!";
        return -1;
    }

    if (bind(socket->handle, (struct sockaddr *) &my_addr, sizeof(my_addr)) == SOCKET_ERROR)
    {
        log(LOG_EMERG) << "Error on bind!";
        return -1;
    }

    return 0;
#else
    return -1;
#endif
}

int Socket::Connect(std::string addr, int port)
{
    int rc;
    struct sockaddr_in serv_addr;
    memset( &serv_addr, 0, sizeof(struct sockaddr_in) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr( addr.c_str() );
    serv_addr.sin_port = htons( port );
    rc = connect( socket_->handle, (struct sockaddr*)&serv_addr, sizeof(serv_addr) );

    if (rc == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
        return -1;

    return WaitForConnect();
}

int Socket::WaitForConnect()
{
    fd_set fds;
    struct timeval tmo;
    int error;

    FD_ZERO(&fds);
    FD_SET(socket_->handle, &fds);
    tmo.tv_sec = 5;
    tmo.tv_usec = 0;

    if (select(0, NULL, &fds, NULL, &tmo) <= 0)
    {
        return -1;
    }

    /*if ( getsockopt(socket_->fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 ||
         error > 0 )
    {
        return -1;
    }*/

    return 0;
}

int Socket::Listen()
{
    return listen(socket_->handle, 5); /*todo 5?*/
}

Socket::~Socket()
{
    Close();
    delete socket_;
}

Socket* Socket::Accept()
{
    struct sockaddr_in sockaddr;
    int len = sizeof(struct sockaddr_in);
    SOCKET newSocket = accept(socket_->handle, (struct sockaddr*)&sockaddr, &len);

    if(newSocket != INVALID_SOCKET)
    {

        log(LOG_NOTICE) << "accept! " << inet_ntoa(sockaddr.sin_addr) << " fd " << newSocket;
        SocketHandle_t* handle = new SocketHandle_t;
        handle->handle = newSocket;
        return new Socket(handle);
    }
    else
    {
        log(LOG_NOTICE) << "accept " << WSAGetLastError();
        return NULL;
    }
}

int Socket::Send(const void* msg, int msgLen)
{
    return send(socket_->handle, (const char*) msg, msgLen, 0);
}

int Socket::Receive(void* buf, int bufLen)
{
    int n = recv(socket_->handle, (char*) buf, bufLen, 0);
    if (n == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
    {
        return 0;
    }
    else if (n == 0)
    {
        return -1;
    }
    return n;
}

void Socket::Close()
{
    closesocket(socket_->handle);
}

void Socket::Shutdown()
{
    shutdown(socket_->handle, SD_RECEIVE);
}



int select(std::set<Socket*>* readsockets, std::set<Socket*>* writesockets, std::set<Socket*>* errsockets, int timeout)
{
    int rc;
    fd_set readfds, writefds, errfds;
    std::set<Socket*>::iterator it;
    struct timeval tmo;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&errfds);

    for (it = readsockets->begin(); it != readsockets->end(); it++)
    {
        FD_SET((*it)->socket_->handle, &readfds);
    }

    for (it = writesockets->begin(); it != writesockets->end(); it++)
    {
        FD_SET((*it)->socket_->handle, &writefds);
    }

    for (it = errsockets->begin(); it != errsockets->end(); it++)
    {
        FD_SET((*it)->socket_->handle, &errfds);
    }

    tmo.tv_sec = timeout/1000;
    tmo.tv_usec = (timeout % 1000) * 1000;

    if ( (rc = select(0, &readfds, &writefds, &errfds, &tmo) ) == SOCKET_ERROR )
    {
        log(LOG_WARN) << "select() failed";
        return rc;
    }

    it = readsockets->begin();
    while (it != readsockets->end())
    {
        if (!FD_ISSET((*it)->socket_->handle, &readfds))
            readsockets->erase(it++);
        else
            it++;
    }

    it = writesockets->begin();
    while (it != writesockets->end())
    {
        if (!FD_ISSET((*it)->socket_->handle, &writefds))
            writesockets->erase(it++);
        else
            it++;
    }

    it = errsockets->begin();
    while (it != errsockets->end())
    {
        if (!FD_ISSET((*it)->socket_->handle, &errfds))
            errsockets->erase(it++);
        else
            it++;
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




