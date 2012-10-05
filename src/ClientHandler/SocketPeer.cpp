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

#include "SocketPeer.h"
#include "MessageFactory/MessageEncoder.h"
#include "MessageFactory/MessageDecoder.h"
#include "applog.h"

SocketPeer::SocketPeer( Socket* socket ) :  requestId(0),
                            reader_(socket),
                            writer_(socket),
                            socket_(socket)

{
}

SocketPeer::~SocketPeer()
{
    delete socket_;
}

int SocketPeer::doRead()
{
    do
    {
        int n = reader_.doread();

        if (n == 0)
        {
            break;
        }
        else if (n < 0)
        {
            log(LOG_DEBUG) << "Socket closed";
            return -1;
        }

        if (reader_.done())
        {
            MessageDecoder m;

            log(LOG_DEBUG) << "Receive complete";

            printHexMsg(reader_.getMessage(), reader_.getLength());

            Message* msg = m.decode(reader_.getMessage());

            if (msg != NULL)
            {
                processMessage(msg);
                delete msg;
            }
            else
            {
                /*error handling?*/
            }

            reader_.reset();
        }
    } while(1);

    return 0;
}


int SocketPeer::doWrite()
{
    log(LOG_DEBUG);

    if (writer_.isEmpty())
    {
        Message* msg = popMessage();
        if (msg == NULL )
            return 0;

        MessageEncoder* encoder = msg->encode();
        encoder->printHex();
        log(LOG_DEBUG) << *msg;
        delete msg;
        writer_.setData(encoder); // SocketWriter takes ownership of encoder
    }

    return writer_.doWrite();
}

Message* SocketPeer::popMessage()
{
    messageQueueMtx.lock();
    Message* msg = messageQueue.front();
    messageQueue.pop();
    messageQueueMtx.unlock();
    return msg;
}


void SocketPeer::queueMessage(Message* msg)
{
    messageQueueMtx.lock();
    messageQueue.push(msg);
    messageQueueMtx.unlock();
}

void SocketPeer::queueRequest(Message* msg)
{
    msg->setId(requestId++);

    queueMessage(msg);
}

bool SocketPeer::pendingSend()
{
    bool ret;
    messageQueueMtx.lock();
    ret = ((messageQueue.empty() == false) || (writer_.isEmpty() == false));
    messageQueueMtx.unlock();
    return ret;
}

Socket* SocketPeer::getSocket() const
{
    return socket_;
}




