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
#include "applog.h"

RemoteAudioEndpointManager::RemoteAudioEndpointManager( Messenger& m ) : messenger_(m), server(NULL), connectionUp_(false)
{
    messenger_.addSubscriber( this );
}

RemoteAudioEndpointManager::~RemoteAudioEndpointManager()
{
    messenger_.removeSubscriber( this );
    server->destroy();
    delete server;
}

void RemoteAudioEndpointManager::sendCreateEndpointMessage()
{
    Message* msg = new Message( CREATE_AUDIO_ENDPOINT_REQ );
    TlvContainer* epTlv = new TlvContainer(TLV_CLIENT);

    //todo: get necessary info from udp listener thread and ep and put in tlv's
    epTlv->addTlv( TLV_PORT, 7789 );
    epTlv->addTlv( TLV_AUDIO_EP_PROTOCOL, LIGHTWEIGHT_UDP );

    /* and a bunch of others... */
    msg->addTlv(epTlv);

    messenger_.queueRequest( msg, this, NULL );
}

void RemoteAudioEndpointManager::createEndpoint( Platform::AudioEndpoint& ep, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    server = new AudioEndpointRemoteSocketServer( ep );

    if ( connectionUp_ )
    {
        sendCreateEndpointMessage();
    }
}

void RemoteAudioEndpointManager::deleteEndpoint( Platform::AudioEndpoint& ep, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( DELETE_AUDIO_ENDPOINT_REQ );
    messenger_.queueRequest( msg, this, NULL );
}
void RemoteAudioEndpointManager::getEndpoints( IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( GET_AUDIO_ENDPOINTS_REQ );
    messenger_.queueRequest( msg, this, new PendingAudioCtrlRequestData(subscriber, userData) );
}

void RemoteAudioEndpointManager::addEndpoint( std::string id, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( ADD_AUDIO_ENDPOINTS_REQ );
    /*todo: should allow multiple endpoints*/
    if ( id != "" )
        msg->addTlv( TLV_LINK, id );
    messenger_.queueRequest( msg, this, new PendingAudioCtrlRequestData(subscriber, userData) );
}
void RemoteAudioEndpointManager::removeEndpoint( std::string id, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( REM_AUDIO_ENDPOINTS_REQ );
    /*todo: should allow multiple endpoints*/
    if ( id != "" )
        msg->addTlv( TLV_LINK, id );
    messenger_.queueRequest( msg, this, new PendingAudioCtrlRequestData(subscriber, userData) );
}

void RemoteAudioEndpointManager::connectionState( bool up )
{
    if ( up )
    {
        sendCreateEndpointMessage();
    }
    connectionUp_ = up;
}
void RemoteAudioEndpointManager::receivedMessage( const Message* msg )
{
    switch( msg->getType() )
    {
        case AUDIO_ENDPOINTS_UPDATED_IND:
            doEndpointsUpdatedNotification();
            break;
        default:
            break;
    }
}
void RemoteAudioEndpointManager::receivedResponse( const Message* rsp, const Message* req, void* userData )
{
    PendingAudioCtrlRequestData* reqData = (PendingAudioCtrlRequestData*) userData;
    if( reqData == NULL )
        return;

    IAudioEndpointCtrlCallbackSubscriber* subscriber = reqData->first;
    void* subscriberData = reqData->second;

    delete reqData;

    log(LOG_DEBUG) << *rsp;

    switch ( rsp->getType() )
    {
    case GET_AUDIO_ENDPOINTS_RSP:
        {
            std::map<std::string, bool> endpoints;

            for ( TlvContainer::const_iterator it = rsp->getTlvRoot()->begin();
                it != rsp->getTlvRoot()->end(); it++ )
            {
                if ( (*it)->getType() == TLV_CLIENT )
                {
                    TlvContainer* tlv = (TlvContainer*)(*it);
                    StringTlv* name = (StringTlv*)tlv->getTlv( TLV_LINK );
                    IntTlv* active = (IntTlv*)tlv->getTlv( TLV_STATE );
                    if ( name && active )
                        endpoints.insert(std::pair<std::string, bool>(name->getString(), active->getVal() != 0 ));
                }
            }

            subscriber->getEndpointsResponse( endpoints, userData );
        }
        break;

    default:
        receivedMessage( rsp ); /* no handler here, might be there's one in receivedMessage() */
        break;
    }
}
