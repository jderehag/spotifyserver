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

#include "Messenger.h"
#include "MessageFactory/Message.h"
#include "applog.h"

Messenger::Messenger() : reqId(0)
{
}

Messenger::~Messenger()
{
}


Message* Messenger::popMessage()
{
    return mb_.pop_front();
}


void Messenger::queueRequest( Message* msg, IMessageSubscriber* origin, void* userData )
{
    if ( MSG_IS_REQUEST( msg->getType() ) )
    {
        /* todo: protect this */
        msg->setId( reqId++ );
        pendingMessageMap_[msg->getId()] = (PendingData){msg, origin, userData};
        mb_.push_back( msg );
    }
    else
    {
        log(LOG_WARN) << "Wrong message type!\n" << msg;
        delete msg;
    }
}

void Messenger::queueMessage( Message* msg )
{
    /* request and indication type messages needs their id set */
    if ( MSG_IS_REQUEST( msg->getType() ) || MSG_IS_INDICATION( msg->getType() ) )
    {
        if( msg->hasId() )
            log(LOG_WARN) << "Indication already has id set!\n" << msg; /*nevermind, let's override it*/

        /* todo: protect this */
        msg->setId( reqId++ );
        mb_.push_back( msg );
    }
    else
    {
        if( msg->hasId() )
        {
            mb_.push_back( msg );
        }
        else
        {
            log(LOG_WARN) << "Response doesn't have an id set!\n" << msg;
            delete msg;
        }
    }
}


bool Messenger::pendingSend()
{
    return !mb_.empty();
}

void Messenger::addSubscriber(IMessageSubscriber* subscriber)
{
    callbackSubscriberMtx_.lock();
    callbackSubscriberList_.insert(subscriber);
    callbackSubscriberMtx_.unlock();
}

void Messenger::removeSubscriber(IMessageSubscriber* subscriber)
{
    callbackSubscriberMtx_.lock();
    callbackSubscriberList_.erase(subscriber);
    callbackSubscriberMtx_.unlock();
}
