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

#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <set>
#include <stdint.h>

typedef enum SockType_s
{
    SOCKTYPE_STREAM,
    SOCKTYPE_DATAGRAM
}SockType_t;

class Socket
{
private:
    struct SocketHandle_t* socket_;
    std::string remoteAddr_;
    std::string localAddr_;

    Socket(struct SocketHandle_t* socket);

    int WaitForConnect();
public:
    Socket(SockType_t type);
    int BindToAddr(const std::string& addr, const std::string& port);
    int BindToDevice(const std::string& device, const std::string& port);
    int Listen();
    int Connect(const std::string& addr, const std::string& port);
    int Send(const void* msg, int msgLen);
    int SendTo(const void* msg, int msgLen, const std::string& addr, const std::string& port);
    int Receive(void* buf, int bufLen);
    int ReceiveFrom(void* buf, int bufLen, std::string& addr, std::string& port);
    int EnableBroadcast();
    Socket* Accept();
    void Close();
    void Shutdown();
    ~Socket();

    friend int select(std::set<Socket*>* readsockets, std::set<Socket*>* writesockets, std::set<Socket*>* errsockets, int timeout);

    const std::string& getRemoteAddr() const {return remoteAddr_;};
    const std::string& getLocalAddr() const {return localAddr_;};
};


uint32_t Ntohl(uint32_t x);
uint16_t Ntohs(uint16_t x);
uint32_t Htonl(uint32_t x);
uint16_t Htons(uint16_t x);


#endif
