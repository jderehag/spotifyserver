/*
 * Copyright (c) 2012, Magnus Bohman
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
 * DISCLAIMED. IN NO EVENT SHALL MAGNUS BOHMAN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <string.h>

#include "SocketWriter.h"
#include "Platform/Socket/Socket.h"
#include "MessageEncoder.h"
#include "applog.h"


#define WRITE_MAX_BYTES (4096)

SocketWriter::SocketWriter(Socket* socket) : socket_(socket), messageEncoder_(NULL), sentLen(0)
{
}

SocketWriter::~SocketWriter()
{
    reset();
}

int SocketWriter::doWrite()
{
    log(LOG_DEBUG);
    if (messageEncoder_ == NULL)
        return 0;

    int toWrite = messageEncoder_->getLength() - sentLen;
    const uint8_t *fromBuf = messageEncoder_->getBuffer() + sentLen;

    if (toWrite > WRITE_MAX_BYTES)
        toWrite = WRITE_MAX_BYTES;

    int len = socket_->Send(fromBuf, toWrite);

    if (len > 0)
    {
        sentLen += len ;
        log(LOG_DEBUG) << "Sent: " << len << " bytes";

        if (sentLen >= messageEncoder_->getLength())
        {
            log(LOG_DEBUG) << "Send complete " << messageEncoder_->getLength() << " bytes";
            reset();
        }
        return 1;
    }
    else if (len < 0)
    {
        log(LOG_DEBUG) << "Send error";
        reset();
        return -1;
    }

    return 0;
}

void SocketWriter::setData(MessageEncoder* messageEncoder)
{
    if (messageEncoder_ != NULL)
    {
        log(LOG_WARN) << "Overwriting unsent data. use isEmpty() before calling me like this";
        reset();
    }

    messageEncoder_ = messageEncoder;
}


bool SocketWriter::isEmpty()
{
    return (messageEncoder_ == NULL);
}

void SocketWriter::reset()
{
    if (messageEncoder_ != NULL)
    {
        delete messageEncoder_;
        messageEncoder_ = NULL;
    }

    sentLen = 0;
}
