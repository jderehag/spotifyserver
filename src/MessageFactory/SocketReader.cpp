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

#include "Platform/Socket/Socket.h"
#include "SocketReader.h"
#include "MessageEncoder.h"
#include "applog.h"
#include <string.h>


SocketReader::SocketReader(Socket* socket) : buf(NULL),
                                             header_received(false),
                                             recvlen(0),
                                             totlen(0),
                                             socket_(socket)
{
}

void SocketReader::reset()
{
    totlen = 0;
    recvlen = 0;
    header_received = false;
    if (buf != NULL)
        delete[] buf;
    buf = NULL;
}

SocketReader::~SocketReader()
{
    reset();
}

int SocketReader::doread()
{
    int n;
    header_t* header;

    if (!header_received)
    {
        n = socket_->Receive( recv_header, sizeof(header_t) - recvlen );
    }
    else
    {
        n = socket_->Receive( &buf[recvlen], totlen - recvlen );
    }

    if (n < 0)
    {
        log(LOG_WARN) << "Error reading on socket";
        return -1;
    }
    else if (n == 0)
    {
        return 0;
    }

    recvlen += n;

    if (!header_received && recvlen >= sizeof(header_t))
    {
        /* full header received, now we can allocate real buffer and start using that */
        header = ((header_t*) recv_header);
        totlen = Ntohl(header->len);
        log(LOG_DEBUG) << "Received header, message length = " << totlen;
        header_received = true;

        #define MAX_LEN (1024*1024) //1 MB should be enough for anyone
        if (totlen > MAX_LEN)
        {
            log(LOG_WARN) << "Max length " << MAX_LEN << " exceeded, received " << totlen;
            return -1;
        }

        buf = new uint8_t[totlen];
        memcpy(buf, recv_header, sizeof(header_t));
    }

    return n;
}

bool SocketReader::done()
{
    return (header_received && recvlen >= totlen);
}
