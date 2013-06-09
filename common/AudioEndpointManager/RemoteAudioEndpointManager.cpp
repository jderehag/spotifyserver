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

#include "RemoteAudioEndpointManager.h"
#include "MessageFactory/Message.h"
#include "MessageFactory/TlvDefinitions.h"

RemoteAudioEndpointManager::RemoteAudioEndpointManager( Messenger& m ) : messenger_(m)
{
    messenger_.addSubscriber( this );
}

RemoteAudioEndpointManager::~RemoteAudioEndpointManager()
{
    messenger_.removeSubscriber( this );
}

void RemoteAudioEndpointManager::addEndpoint( Platform::AudioEndpoint& ep, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    //todo: launch a udp listener thread and provide ep to it

    Message* msg = new Message( CREATE_AUDIO_ENDPOINT_REQ );
    TlvContainer* epTlv = new TlvContainer(TLV_CLIENT);

    //todo: get necessary info from udp listener thread and ep and put in tlv's
    epTlv->addTlv( TLV_PORT, 7789 );
    epTlv->addTlv( TLV_AUDIO_EP_PROTOCOL, LIGHTWEIGHT_UDP );

    /* and a bunch of others... */
    msg->addTlv(epTlv);

    messenger_.queueRequest( msg, this, NULL );
}

void RemoteAudioEndpointManager::removeEndpoint( Platform::AudioEndpoint& ep, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( DELETE_AUDIO_ENDPOINT_REQ );
    messenger_.queueRequest( msg, this, NULL );
}
void RemoteAudioEndpointManager::getEndpoints( IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( GET_AUDIO_ENDPOINTS_REQ );
    messenger_.queueRequest( msg, this, NULL );
}



void RemoteAudioEndpointManager::connectionState( bool up )
{

}
void RemoteAudioEndpointManager::receivedMessage( const Message* msg )
{

}
void RemoteAudioEndpointManager::receivedResponse( const Message* rsp, const Message* req, void* userData )
{
    switch ( rsp->getType() )
    {
    case GET_AUDIO_ENDPOINTS_RSP:

        break;

    default:
        receivedMessage( rsp ); /* no handler here, might be there's one in receivedMessage() */
        break;
    }
}
