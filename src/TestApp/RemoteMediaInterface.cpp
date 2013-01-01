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

#include "RemoteMediaInterface.h"
#include "MessageFactory/Message.h"
#include "applog.h"

RemoteMediaInterface::RemoteMediaInterface(Messenger& m) : messenger_(m),
                                                           reqId(0)
{
    messenger_.addSubscriber(this);
}

RemoteMediaInterface::~RemoteMediaInterface()
{
}

void RemoteMediaInterface::connectionState( bool up )
{
    if ( up )
    {
        Message* hello = new Message(HELLO_REQ);

        hello->addTlv(TLV_PROTOCOL_VERSION_MAJOR, PROTOCOL_VERSION_MAJOR);
        hello->addTlv(TLV_PROTOCOL_VERSION_MINOR, PROTOCOL_VERSION_MINOR);
        hello->addTlv(TLV_LOGIN_USERNAME, "wonder");
        hello->addTlv(TLV_LOGIN_PASSWORD, "wall");

        messenger_.queueMessage( hello, reqId++ ); /* we should make sure this goes out first, ahead of any old pending messages from an old connection */
    }

    callbackSubscriberMtx_.lock();
    for( std::set<IMediaInterfaceCallbackSubscriber*>::iterator it = callbackSubscriberList_.begin();
            it != callbackSubscriberList_.end(); it++)
    {
        (*it)->connectionState( up );
    }
    callbackSubscriberMtx_.unlock();
}

void RemoteMediaInterface::doRequest( Message* msg, unsigned int mediaReqId, IMediaInterfaceCallbackSubscriber* subscriber )
{
    unsigned int myId = reqId++;
    //pendingReqsMap[myId] = PendingRequest( mediaReqId, subscriber );
    pendingReqsMap.insert( PendingReqsMap::value_type ( myId, PendingRequest( mediaReqId, subscriber ) ) );
    messenger_.queueMessage( msg, myId );
}

void RemoteMediaInterface::receivedMessage( Message* msg )
{
    log(LOG_DEBUG) << *msg;

    switch( msg->getType() )
    {
        case STATUS_IND:
        {
            PlaybackState_t playbackState = PLAYBACK_IDLE;
            bool shuffleState = false;
            bool repeatState = false;
            const IntTlv* tlv = (const IntTlv*) msg->getTlv(TLV_STATE);
            if ( tlv ) playbackState = (PlaybackState_t)tlv->getVal();
            tlv = (const IntTlv*) msg->getTlv(TLV_PLAY_MODE_REPEAT);
            if ( tlv ) repeatState = (tlv->getVal() != 0);
            tlv = (const IntTlv*) msg->getTlv(TLV_PLAY_MODE_SHUFFLE);
            if ( tlv ) shuffleState = (tlv->getVal() != 0);

            const TlvContainer* trackTlv = (const TlvContainer*) msg->getTlv(TLV_TRACK);

            callbackSubscriberMtx_.lock();
            if ( trackTlv != NULL )
            {
                int progress = 0;
                tlv = (const IntTlv*) msg->getTlv(TLV_PROGRESS);
                if ( tlv ) progress = tlv->getVal();

                for( std::set<IMediaInterfaceCallbackSubscriber*>::iterator it = callbackSubscriberList_.begin();
                     it != callbackSubscriberList_.end(); it++)
                {
                    (*it)->statusUpdateInd( playbackState, repeatState, shuffleState, Track( trackTlv ), progress );
                }
            }
            else
            {
                for( std::set<IMediaInterfaceCallbackSubscriber*>::iterator it = callbackSubscriberList_.begin();
                     it != callbackSubscriberList_.end(); it++)
                {
                    (*it)->statusUpdateInd( playbackState, repeatState, shuffleState );
                }
            }
            callbackSubscriberMtx_.unlock();
        }
        break;


        default:
            break;
    }
}

void RemoteMediaInterface::receivedResponse( Message* rsp, Message* req )
{
    PendingReqsMap::iterator it = pendingReqsMap.find( rsp->getId() );
    if ( it != pendingReqsMap.end() )
    {
        log(LOG_DEBUG) << *rsp;
        PendingRequest mediaReq = it->second;

        callbackSubscriberMtx_.lock();
        /* todo verify subscriber still exists */

        switch( rsp->getType() )
        {
            case GET_PLAYLISTS_RSP:
            {
                mediaReq.subscriber->getPlaylistsResponse( mediaReq.mediaReqId, Folder( (const TlvContainer*) rsp->getTlv(TLV_FOLDER) ) );
            }
            break;

            case GET_TRACKS_RSP:
            {
                std::deque<Track> tracks;

                for ( TlvContainer::const_iterator it = rsp->getTlvRoot()->begin();
                        it != rsp->getTlvRoot()->end(); it++ )
                {
                    if ( (*it)->getType() == TLV_TRACK )
                    {
                        tracks.push_back( Track( (const TlvContainer*)(*it) ) );
                    }
                }

                mediaReq.subscriber->getTracksResponse( mediaReq.mediaReqId, tracks );
            }
            break;

            case GET_STATUS_RSP:
            {
                PlaybackState_t playbackState = PLAYBACK_IDLE;
                bool shuffleState = false;
                bool repeatState = false;
                const IntTlv* tlv = (const IntTlv*) rsp->getTlv(TLV_STATE);
                if ( tlv ) playbackState = (PlaybackState_t)tlv->getVal();
                tlv = (const IntTlv*) rsp->getTlv(TLV_PLAY_MODE_REPEAT);
                if ( tlv ) repeatState = (tlv->getVal() != 0);
                tlv = (const IntTlv*) rsp->getTlv(TLV_PLAY_MODE_SHUFFLE);
                if ( tlv ) shuffleState = (tlv->getVal() != 0);

                const TlvContainer* trackTlv = (const TlvContainer*) rsp->getTlv(TLV_TRACK);

                if ( trackTlv != NULL )
                {
                    int progress = 0;
                    tlv = (const IntTlv*) rsp->getTlv(TLV_PROGRESS);
                    if ( tlv ) progress = tlv->getVal();

                    mediaReq.subscriber->getStatusResponse( mediaReq.mediaReqId, playbackState, repeatState, shuffleState, Track( trackTlv ), progress );
                }
                else
                {
                    mediaReq.subscriber->getStatusResponse( mediaReq.mediaReqId, playbackState, repeatState, shuffleState );
                }
            }
            break;

            case GET_IMAGE_RSP:
            {
                void* data; /*todo decode message*/
                size_t dataSize;
                mediaReq.subscriber->getImageResponse( mediaReq.mediaReqId, data, dataSize );
            }
            break;

            case GET_ALBUM_RSP:
            {
                const TlvContainer* albumTlv = (const TlvContainer*)rsp->getTlv(TLV_ALBUM);
                if ( albumTlv )
                {
                    Album album( albumTlv );
                    mediaReq.subscriber->getAlbumResponse( mediaReq.mediaReqId, album );
                }
            }
            break;

            case GENERIC_SEARCH_RSP:
            {
                std::deque<Track> tracks;

                for ( TlvContainer::const_iterator it = rsp->getTlvRoot()->begin();
                        it != rsp->getTlvRoot()->end(); it++ )
                {
                    if ( (*it)->getType() == TLV_TRACK )
                    {
                        tracks.push_back( Track( (const TlvContainer*)(*it) ) );
                    }
                }

                mediaReq.subscriber->genericSearchCallback( mediaReq.mediaReqId, tracks, "" );
            }
            break;

            default:
                break;
        }
        callbackSubscriberMtx_.unlock();
        pendingReqsMap.erase( it );
    }
    else
    {
        /*todo remove me when all requests use pendingReqsMap*/
        receivedMessage( rsp );
    }
}


void RemoteMediaInterface::getImage( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId mediaReqId )
{
    Message* msg = new Message( GET_IMAGE_REQ );
    msg->addTlv(TLV_LINK, link);
    doRequest( msg, mediaReqId, subscriber );
}

void RemoteMediaInterface::previous()
{
    Message* msg = new Message( PLAY_CONTROL_REQ );
    msg->addTlv( TLV_PLAY_OPERATION, PLAY_OP_PREV );
    messenger_.queueMessage( msg, reqId++ );
}

void RemoteMediaInterface::next()
{
    Message* msg = new Message( PLAY_CONTROL_REQ );
    msg->addTlv( TLV_PLAY_OPERATION, PLAY_OP_NEXT );
    messenger_.queueMessage( msg, reqId++ );
}

void RemoteMediaInterface::resume()
{
    Message* msg = new Message( PLAY_CONTROL_REQ );
    msg->addTlv( TLV_PLAY_OPERATION, PLAY_OP_RESUME );
    messenger_.queueMessage( msg, reqId++ );
}

void RemoteMediaInterface::pause()
{
    Message* msg = new Message( PLAY_CONTROL_REQ );
    msg->addTlv( TLV_PLAY_OPERATION, PLAY_OP_PAUSE );
    messenger_.queueMessage( msg, reqId++ );
}

void RemoteMediaInterface::setShuffle( bool shuffleOn )
{
    Message* msg = new Message( PLAY_CONTROL_REQ );
    msg->addTlv( TLV_PLAY_MODE_SHUFFLE, shuffleOn ? 1 : 0 );
    messenger_.queueMessage( msg, reqId++ );
}

void RemoteMediaInterface::setRepeat( bool repeatOn )
{
    Message* msg = new Message( PLAY_CONTROL_REQ );
    msg->addTlv( TLV_PLAY_MODE_REPEAT, repeatOn ? 1 : 0 );
    messenger_.queueMessage( msg, reqId++ );
}

void RemoteMediaInterface::getStatus( IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId mediaReqId )
{
    Message* msg = new Message( GET_STATUS_REQ );
    doRequest( msg, mediaReqId, subscriber );
}

void RemoteMediaInterface::getPlaylists( IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId mediaReqId )
{
    Message* msg = new Message(GET_PLAYLISTS_REQ);
    doRequest( msg, mediaReqId, subscriber );
}

void RemoteMediaInterface::getTracks( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId mediaReqId )
{
    Message* msg = new Message(GET_TRACKS_REQ);
    TlvContainer* p = new TlvContainer(TLV_PLAYLIST);
    p->addTlv(TLV_LINK, link);
    msg->addTlv(p);
    doRequest( msg, mediaReqId, subscriber );
}

void RemoteMediaInterface::play( std::string link, int startIndex, IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId mediaReqId )
{
    Message* msg = new Message( PLAY_REQ );
    msg->addTlv(TLV_LINK, link);
    msg->addTlv(TLV_TRACK_INDEX, startIndex);
    doRequest( msg, mediaReqId, subscriber );
}

void RemoteMediaInterface::play( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId mediaReqId )
{
    Message* msg = new Message( PLAY_REQ );
    msg->addTlv(TLV_LINK, link);
    doRequest( msg, mediaReqId, subscriber );
}

void RemoteMediaInterface::getAlbum( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId mediaReqId )
{
    Message* msg = new Message( GET_ALBUM_REQ);
    msg->addTlv(TLV_LINK, link);
    doRequest( msg, mediaReqId, subscriber );
}

void RemoteMediaInterface::search( std::string query, IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId mediaReqId )
{
    Message* msg = new Message( GENERIC_SEARCH_REQ );
    msg->addTlv( TLV_SEARCH_QUERY, query );
    doRequest( msg, mediaReqId, subscriber );
}

void RemoteMediaInterface::addAudio()
{
    Message* msg = new Message( ADD_AUDIO_ENDPOINT_REQ );
    messenger_.queueMessage( msg, reqId++ );
}
