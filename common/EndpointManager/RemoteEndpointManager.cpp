/*
 * Copyright (c) 2014, Jens Nielsen
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
 * (INCLUDING, BUT NOT LIMITED S; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITWHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "RemoteEndpointManager.h"
#include "MessageFactory/Message.h"
#include "MessageFactory/TlvDefinitions.h"
#include "applog.h"
#include <assert.h>

RemoteEndpointManager::RemoteEndpointManager( Messenger& m, EndpointIdIf&  epId_ ) : messenger_(m), epId(epId_), server(NULL), ep_(NULL), connectionUp_(false)
{
    messenger_.addSubscriber( this );
}

RemoteEndpointManager::~RemoteEndpointManager()
{
    messenger_.removeSubscriber( this );
    server->destroy();
    delete server;
}

void RemoteEndpointManager::registerId( EndpointIdIf& appId )
{
}

void RemoteEndpointManager::unregisterId( EndpointIdIf& appId )
{
}

void RemoteEndpointManager::getEndpoints( IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( GET_ENDPOINTS_REQ );
    messenger_.queueRequest( msg, this, new PendingEndpointCtrlRequestData(subscriber, userData) );
}

void RemoteEndpointManager::renameEndpoint( const std::string& from, const std::string& to, IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    if ( !from.empty() && !to.empty() )
    {
        Message* msg = new Message( RENAME_ENDPOINT_REQ );
        TlvContainer* tlv = new TlvContainer( TLV_CLIENT );
        tlv->addTlv( TLV_LINK, from );
        msg->addTlv( tlv );
        msg->addTlv( TLV_LINK, to );
        messenger_.queueRequest( msg, this, new PendingEndpointCtrlRequestData(subscriber, userData) );
    }
}

void RemoteEndpointManager::sendCreateEndpointMessage()
{
    Message* msg = new Message( CREATE_AUDIO_ENDPOINT_REQ );
    TlvContainer* epTlv = new TlvContainer(TLV_CLIENT);

    //todo: get necessary info from udp listener thread and ep and put in tlv's
    epTlv->addTlv( TLV_PORT, 7789 );
    epTlv->addTlv( TLV_AUDIO_EP_PROTOCOL, LIGHTWEIGHT_UDP );
    epTlv->addTlv( TLV_VOLUME, ep_->getRelativeVolume() );

    /* and a bunch of others... */
    msg->addTlv(epTlv);

    messenger_.queueRequest( msg, this, NULL );
}

void RemoteEndpointManager::createAudioEndpoint( Platform::AudioEndpoint& ep, IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    server = new AudioEndpointRemoteSocketServer( ep );
    ep_ = &ep;
    if ( connectionUp_ )
    {
        sendCreateEndpointMessage();
    }
}

void RemoteEndpointManager::deleteAudioEndpoint( Platform::AudioEndpoint& ep, IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    assert( &ep == ep_ );
    ep_ = NULL;
    Message* msg = new Message( DELETE_AUDIO_ENDPOINT_REQ );
    messenger_.queueRequest( msg, this, NULL );
}
void RemoteEndpointManager::getAudioEndpoints( IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( GET_AUDIO_ENDPOINTS_REQ );
    messenger_.queueRequest( msg, this, new PendingEndpointCtrlRequestData(subscriber, userData) );
}

void RemoteEndpointManager::addAudioEndpoint( std::string id, IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( ADD_AUDIO_ENDPOINTS_REQ );
    /*todo: should allow multiple endpoints*/
    if ( id != "" )
        msg->addTlv( TLV_LINK, id );
    messenger_.queueRequest( msg, this, new PendingEndpointCtrlRequestData(subscriber, userData) );
}
void RemoteEndpointManager::removeAudioEndpoint( std::string id, IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( REM_AUDIO_ENDPOINTS_REQ );
    /*todo: should allow multiple endpoints*/
    if ( id != "" )
        msg->addTlv( TLV_LINK, id );
    messenger_.queueRequest( msg, this, new PendingEndpointCtrlRequestData(subscriber, userData) );
}

void RemoteEndpointManager::setRelativeVolume( std::string id, uint8_t volume )
{
    Message* msg = new Message( SET_VOLUME_REQ );
    TlvContainer* tlv = new TlvContainer( TLV_CLIENT );
    if ( id != "" )
        tlv->addTlv( TLV_LINK, id );
    tlv->addTlv( TLV_VOLUME, volume );
    msg->addTlv( tlv );
    messenger_.queueRequest( msg, this, NULL );
}

void RemoteEndpointManager::connectionState( bool up )
{
    if ( up )
    {
        sendCreateEndpointMessage();
    }
    connectionUp_ = up;
}
void RemoteEndpointManager::receivedMessage( const Message* msg )
{
    switch( msg->getType() )
    {
        case AUDIO_ENDPOINTS_UPDATED_IND:
            doAudioEndpointsUpdatedNotification();
            break;
        case SET_VOLUME_REQ:
            handleSetVolumeReq( msg );
            break;
        case RENAME_ENDPOINT_REQ:
            {
                StringTlv* tlv = (StringTlv*) msg->getTlv( TLV_LINK );
                std::string newId = tlv ? tlv->getString() : "";
                if ( !newId.empty() )
                {
                    epId.rename( newId );

                    /* confirm name change */
                    Message* rsp = msg->createResponse();
                    messenger_.queueMessage( rsp );
                }
            }
            break;
        default:
            break;
    }
}

void RemoteEndpointManager::handleSetVolumeReq( const Message* msg )
{
    if ( ep_ )
    {
        for (TlvContainer::const_iterator it = msg->getTlvRoot()->begin() ; it != msg->getTlvRoot()->end() ; it++)
        {
            Tlv* tlv = (*it);
            log(LOG_DEBUG) << *tlv;

            switch(tlv->getType())
            {
                case TLV_VOLUME:
                    ep_->setMasterVolume( ((IntTlv*)tlv)->getVal() );
                    break;
                case TLV_CLIENT:
                {
                    IntTlv* volume = (IntTlv*)((TlvContainer*)tlv)->getTlv( TLV_VOLUME );
                    if ( volume )
                        ep_->setRelativeVolume( volume->getVal() );
                    break;
                }
                default:
                    break;
            }
        }
    }

    Message* rsp = msg->createResponse();
    messenger_.queueMessage( rsp );
}

void RemoteEndpointManager::receivedResponse( const Message* rsp, const Message* req, void* userData )
{
    PendingEndpointCtrlRequestData* reqData = (PendingEndpointCtrlRequestData*) userData;
    if( reqData == NULL )
        return;

    IEndpointCtrlCallbackSubscriber* subscriber = reqData->first;
    void* subscriberData = reqData->second;

    delete reqData;

    log(LOG_DEBUG) << *rsp;

    switch ( rsp->getType() )
    {
    case GET_AUDIO_ENDPOINTS_RSP:
        {
            AudioEndpointInfoList endpoints;

            for ( TlvContainer::const_iterator it = rsp->getTlvRoot()->begin();
                it != rsp->getTlvRoot()->end(); it++ )
            {
                if ( (*it)->getType() == TLV_CLIENT )
                {
                    TlvContainer* tlv = (TlvContainer*)(*it);
                    StringTlv* name = (StringTlv*)tlv->getTlv( TLV_LINK );
                    IntTlv* active = (IntTlv*)tlv->getTlv( TLV_STATE );
                    IntTlv* volume = (IntTlv*)tlv->getTlv( TLV_VOLUME );
                    if ( name && active )
                        endpoints.push_back( AudioEndpointInfo( name->getString(), (active->getVal()!=0), volume->getVal() ) );
                }
            }

            subscriber->getAudioEndpointsResponse( endpoints, subscriberData );
        }
        break;
    case GET_ENDPOINTS_RSP:
        {
            EndpointInfoList endpoints;

            for ( TlvContainer::const_iterator it = rsp->getTlvRoot()->begin();
                it != rsp->getTlvRoot()->end(); it++ )
            {
                if ( (*it)->getType() == TLV_CLIENT )
                {
                    TlvContainer* tlv = (TlvContainer*)(*it);
                    StringTlv* name = (StringTlv*)tlv->getTlv( TLV_LINK );
                    if ( name )
                        endpoints.push_back( name->getString() );
                }
            }

            subscriber->getEndpointsResponse( endpoints, subscriberData );
        }
        break;
    case RENAME_ENDPOINT_RSP:
        {
            subscriber->renameEndpointResponse( subscriberData );
        }
        break;
    default:
        receivedMessage( rsp ); /* no handler here, might be there's one in receivedMessage() */
        break;
    }
}
