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

#ifndef MESSENGER_H_
#define MESSENGER_H_

#include "Platform/Threads/Mutex.h"
#include "Platform/Threads/Messagebox.h"
#include <stdint.h>


class Message;

class IMessageSubscriber
{
public:
    virtual void connectionState( bool up ) = 0;
    virtual void receivedMessage( Message* msg ) = 0;
    virtual void receivedResponse( Message* rsp, Message* req ) = 0;
};

class Messenger
{
private:
    /* message box for other threads to transfer messages to derived class */
    Platform::Messagebox<Message*> mb_;

protected:
    IMessageSubscriber* subscriber_;

    Message* popMessage();

public:
    Messenger();
    virtual ~Messenger();

    void queueMessage( Message* msg, unsigned int reqId ); /* request and indication type messages */
    void queueResponse( Message* rsp, const Message* req ); /* response type messages */
    void queueResponse( Message* rsp, unsigned int reqId ); /* response type messages */

    void addSubscriber( IMessageSubscriber* subscriber );

    bool pendingSend();
};

#endif /* MESSENGER_H_ */
