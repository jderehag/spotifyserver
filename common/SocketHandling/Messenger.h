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
#include <set>
#include <map>
#include <stdint.h>


class Message;

class IMessageSubscriber
{
public:
    virtual void connectionState( bool up ) = 0;
    virtual void receivedMessage( const Message* msg ) = 0;
    virtual void receivedResponse( const Message* rsp, const Message* req, void* userData ) = 0;
};

class Messenger
{
private:
    /* message box for other threads to transfer messages to derived class */
    Platform::Messagebox<Message*> mb_;

protected:
    Platform::Mutex callbackSubscriberMtx_;
    typedef std::set<IMessageSubscriber*> MessageSubscriberSet;
    MessageSubscriberSet callbackSubscriberList_;

    /* pending requests sent to other side of connection */
    typedef struct { Message* req; IMessageSubscriber* origin; void* userData; } PendingData;
    typedef std::map<unsigned int, PendingData>  PendingDataMap;
    PendingDataMap pendingMessageMap_;

    Message* popMessage();

    uint32_t reqId;

public:
    Messenger();
    virtual ~Messenger();

    void queueRequest( Message* msg, IMessageSubscriber* source, void* userData ); /* request type messages */
    void queueMessage( Message* msg ); /* response and indication type messages, can also be used for request if response is ignored */

    void addSubscriber( IMessageSubscriber* subscriber );
    void removeSubscriber( IMessageSubscriber* subscriber );

    virtual bool pendingSend();
};

#endif /* MESSENGER_H_ */
