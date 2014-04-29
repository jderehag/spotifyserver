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

RemoteMediaInterface::RemoteMediaInterface(Messenger& m) : messenger_(m)
{
    messenger_.addSubscriber(this);
}

RemoteMediaInterface::~RemoteMediaInterface()
{
    messenger_.removeSubscriber(this);
}

void RemoteMediaInterface::connectionState( bool up )
{
    MediaInterface::connectionState( up );
}

void RemoteMediaInterface::receivedMessage( const Message* msg )
{
    log(LOG_DEBUG) << *msg;

    switch( msg->getType() )
    {
        case STATUS_IND:
        {
            PlaybackState_t playbackState = PLAYBACK_IDLE;
            bool shuffleState = false;
            bool repeatState = false;
            uint8_t volume = 0;
            const IntTlv* tlv = (const IntTlv*) msg->getTlv(TLV_STATE);
            if ( tlv ) playbackState = (PlaybackState_t)tlv->getVal();
            tlv = (const IntTlv*) msg->getTlv(TLV_PLAY_MODE_REPEAT);
            if ( tlv ) repeatState = (tlv->getVal() != 0);
            tlv = (const IntTlv*) msg->getTlv(TLV_PLAY_MODE_SHUFFLE);
            if ( tlv ) shuffleState = (tlv->getVal() != 0);
            tlv = (const IntTlv*) msg->getTlv(TLV_VOLUME);
            if ( tlv ) volume = (uint8_t)tlv->getVal();

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
                    (*it)->statusUpdateInd( playbackState, repeatState, shuffleState, volume, Track( trackTlv ), progress );
                }
            }
            else
            {
                for( std::set<IMediaInterfaceCallbackSubscriber*>::iterator it = callbackSubscriberList_.begin();
                     it != callbackSubscriberList_.end(); it++)
                {
                    (*it)->statusUpdateInd( playbackState, repeatState, shuffleState, volume );
                }
            }
            callbackSubscriberMtx_.unlock();
        }
        break;


        default:
            break;
    }
}

void RemoteMediaInterface::receivedResponse( const Message* rsp, const Message* req, void* userData )
{
    PendingMediaRequestData* reqData = (PendingMediaRequestData*) userData;
    if( reqData == NULL )
        return;

    IMediaInterfaceCallbackSubscriber* subscriber = reqData->first;
    void* subscriberData = reqData->second;

    delete reqData;

    log(LOG_DEBUG) << *rsp;

        callbackSubscriberMtx_.lock();
        /* todo verify subscriber still exists */

        switch( rsp->getType() )
        {
            case GET_PLAYLISTS_RSP:
            {
                subscriber->getPlaylistsResponse( Folder( (const TlvContainer*) rsp->getTlv(TLV_FOLDER) ), subscriberData );
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

                subscriber->getTracksResponse( tracks, subscriberData );
            }
            break;

            case GET_STATUS_RSP:
            {
                PlaybackState_t playbackState = PLAYBACK_IDLE;
                bool shuffleState = false;
                bool repeatState = false;
                uint8_t volume = 0;
                const IntTlv* tlv = (const IntTlv*) rsp->getTlv(TLV_STATE);
                if ( tlv ) playbackState = (PlaybackState_t)tlv->getVal();
                tlv = (const IntTlv*) rsp->getTlv(TLV_PLAY_MODE_REPEAT);
                if ( tlv ) repeatState = (tlv->getVal() != 0);
                tlv = (const IntTlv*) rsp->getTlv(TLV_PLAY_MODE_SHUFFLE);
                if ( tlv ) shuffleState = (tlv->getVal() != 0);
                tlv = (const IntTlv*) rsp->getTlv(TLV_VOLUME);
                if ( tlv ) volume = (uint8_t)tlv->getVal();

                const TlvContainer* trackTlv = (const TlvContainer*) rsp->getTlv(TLV_TRACK);

                if ( trackTlv != NULL )
                {
                    int progress = 0;
                    tlv = (const IntTlv*) rsp->getTlv(TLV_PROGRESS);
                    if ( tlv ) progress = tlv->getVal();

                    subscriber->getStatusResponse( playbackState, repeatState, shuffleState, volume, Track( trackTlv ), progress, subscriberData );
                }
                else
                {
                    subscriber->getStatusResponse( playbackState, repeatState, shuffleState, volume, subscriberData );
                }
            }
            break;

            case GET_IMAGE_RSP:
            {
                const TlvContainer* imgTlv = (const TlvContainer*)rsp->getTlv( TLV_IMAGE );
                const BinaryTlv* imgdataTlv = imgTlv ? (const BinaryTlv*)imgTlv->getTlv( TLV_IMAGE_DATA ) : NULL;

                void* data = 0;
                size_t dataSize = 0;
                if ( imgdataTlv )
                {
                    data = imgdataTlv->getData();
                    dataSize = imgdataTlv->getLen();
                }
                subscriber->getImageResponse( data, dataSize, subscriberData );
            }
            break;

            case GET_ALBUM_RSP:
            {
                const TlvContainer* albumTlv = (const TlvContainer*)rsp->getTlv(TLV_ALBUM);
                if ( albumTlv )
                {
                    Album album( albumTlv );
                    subscriber->getAlbumResponse( album, subscriberData );
                }
            }
            break;

            case GET_ARTIST_RSP:
            {
                const TlvContainer* artistTlv = (const TlvContainer*)rsp->getTlv(TLV_ARTIST);
                if ( artistTlv )
                {
                    Artist artist( artistTlv );
                    subscriber->getArtistResponse( artist, subscriberData );
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

                subscriber->genericSearchCallback( tracks, "", subscriberData );
            }
            break;

            case GET_CURRENT_AUDIO_ENDPOINTS_RSP:
            {
                std::set<std::string>endpoints;
                for ( TlvContainer::const_iterator it = rsp->getTlvRoot()->begin();
                        it != rsp->getTlvRoot()->end(); it++ )
                {
                    if ( (*it)->getType() == TLV_LINK )
                    {
                        endpoints.insert( ((const StringTlv*)(*it))->getString() );
                    }
                }
                subscriber->getCurrentAudioEndpointsResponse( endpoints, subscriberData );
            }
            break;

            default:
                break;
        }
        callbackSubscriberMtx_.unlock();
}


void RemoteMediaInterface::getImage( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( GET_IMAGE_REQ );
    msg->addTlv(TLV_LINK, link);
    messenger_.queueRequest( msg, this, new PendingMediaRequestData(subscriber, userData) );
}

void RemoteMediaInterface::previous()
{
    Message* msg = new Message( PLAY_CONTROL_REQ );
    msg->addTlv( TLV_PLAY_OPERATION, PLAY_OP_PREV );
    messenger_.queueRequest( msg, this, NULL );
}

void RemoteMediaInterface::next()
{
    Message* msg = new Message( PLAY_CONTROL_REQ );
    msg->addTlv( TLV_PLAY_OPERATION, PLAY_OP_NEXT );
    messenger_.queueRequest( msg, this, NULL );
}

void RemoteMediaInterface::resume()
{
    Message* msg = new Message( PLAY_CONTROL_REQ );
    msg->addTlv( TLV_PLAY_OPERATION, PLAY_OP_RESUME );
    messenger_.queueRequest( msg, this, NULL );
}

void RemoteMediaInterface::pause()
{
    Message* msg = new Message( PLAY_CONTROL_REQ );
    msg->addTlv( TLV_PLAY_OPERATION, PLAY_OP_PAUSE );
    messenger_.queueRequest( msg, this, NULL );
}

void RemoteMediaInterface::seek( uint32_t sec )
{
    Message* msg = new Message( PLAY_CONTROL_REQ );
    msg->addTlv( TLV_PROGRESS, sec );
    messenger_.queueRequest( msg, this, NULL );
}

void RemoteMediaInterface::setShuffle( bool shuffleOn )
{
    Message* msg = new Message( PLAY_CONTROL_REQ );
    msg->addTlv( TLV_PLAY_MODE_SHUFFLE, shuffleOn ? 1 : 0 );
    messenger_.queueRequest( msg, this, NULL );
}

void RemoteMediaInterface::setRepeat( bool repeatOn )
{
    Message* msg = new Message( PLAY_CONTROL_REQ );
    msg->addTlv( TLV_PLAY_MODE_REPEAT, repeatOn ? 1 : 0 );
    messenger_.queueRequest( msg, this, NULL );
}

void RemoteMediaInterface::setVolume( uint8_t volume )
{
    Message* msg = new Message( SET_VOLUME_REQ );
    msg->addTlv( TLV_VOLUME, volume );
    messenger_.queueRequest( msg, this, NULL );
}


void RemoteMediaInterface::getStatus( IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( GET_STATUS_REQ );
    messenger_.queueRequest( msg, this, new PendingMediaRequestData(subscriber, userData) );
}

void RemoteMediaInterface::getPlaylists( IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message(GET_PLAYLISTS_REQ);
    messenger_.queueRequest( msg, this, new PendingMediaRequestData(subscriber, userData) );
}

void RemoteMediaInterface::getTracks( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message(GET_TRACKS_REQ);
    TlvContainer* p = new TlvContainer(TLV_PLAYLIST);
    p->addTlv(TLV_LINK, link);
    msg->addTlv(p);
    messenger_.queueRequest( msg, this, new PendingMediaRequestData(subscriber, userData) );
}

void RemoteMediaInterface::play( std::string link, int startIndex, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( PLAY_REQ );
    msg->addTlv(TLV_LINK, link);
    msg->addTlv(TLV_TRACK_INDEX, startIndex);
    messenger_.queueRequest( msg, this, new PendingMediaRequestData(subscriber, userData) );
}

void RemoteMediaInterface::play( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( PLAY_REQ );
    msg->addTlv(TLV_LINK, link);
    messenger_.queueRequest( msg, this, new PendingMediaRequestData(subscriber, userData) );
}

void RemoteMediaInterface::getAlbum( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( GET_ALBUM_REQ);
    msg->addTlv(TLV_LINK, link);
    messenger_.queueRequest( msg, this, new PendingMediaRequestData(subscriber, userData) );
}

void RemoteMediaInterface::getArtist( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( GET_ARTIST_REQ);
    msg->addTlv(TLV_LINK, link);
    messenger_.queueRequest( msg, this, new PendingMediaRequestData(subscriber, userData) );
}

void RemoteMediaInterface::search( std::string query, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( GENERIC_SEARCH_REQ );
    msg->addTlv( TLV_SEARCH_QUERY, query );
    messenger_.queueRequest( msg, this, new PendingMediaRequestData(subscriber, userData) );
}

void RemoteMediaInterface::getCurrentAudioEndpoints( IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    Message* msg = new Message( GET_CURRENT_AUDIO_ENDPOINTS_REQ );
    messenger_.queueRequest( msg, this, new PendingMediaRequestData(subscriber, userData) );
}

