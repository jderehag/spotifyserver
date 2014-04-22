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

#include "Client.h"
#include "applog.h"
#include "Platform/Socket/Socket.h" /* needed so that client can resolve getSocket()->getRemoteAddr()*/

uint32_t Client::count;

Client::Client(Socket* socket, MediaInterface& spotifyif, AudioEndpointCtrlInterface& audioCtrl, EndpointsDb& epDb ) :
                                                            SocketPeer(socket),
                                                            spotify_(spotifyif),
                                                            audioCtrl_(audioCtrl),
                                                            epDb_(epDb),
                                                            audioEp(NULL),
                                                            loggedIn_(true),
                                                            networkUsername_(""),
                                                            networkPassword_("")
{
    std::ostringstream idStr;
    idStr << "client-" << count++;
    id = idStr.str();

    std::cout << "Client " << id << " connected from " << getSocket()->getRemoteAddr() << std::endl;

    epDb_.registerId( this );
    spotify_.registerForCallbacks(*this);
    audioCtrl_.registerForCallbacks(*this);
}

Client::~Client()
{
    log(LOG_DEBUG) << "~Client";
    spotify_.unRegisterForCallbacks(*this);
    audioCtrl_.unRegisterForCallbacks(*this);

    if ( audioEp )
    {
        audioCtrl_.deleteEndpoint( *audioEp, NULL, NULL );
        audioEp->destroy();
        delete audioEp;
        audioEp = NULL;
    }

    epDb_.unregisterId( this );
}

void Client::setUsername(std::string username) { networkUsername_ = username; }
void Client::setPassword(std::string password) { networkPassword_ = password; }

void Client::processMessage(const Message* msg)
{
    log(LOG_NOTICE) << *(msg);

    /*require login before doing anything else (??if either username or password is set, for backward compatibility??)*/
    if ( /*(!networkUsername_.empty() || !networkPassword_.empty()) && */!loggedIn_ && msg->getType() != HELLO_REQ )
    {
        return;
    }

    switch(msg->getType())
    {
        case HELLO_REQ:          handleHelloReq(msg);         break;
        case GET_PLAYLISTS_REQ:  handleGetPlaylistsReq(msg);  break;
        case GET_TRACKS_REQ:     handleGetTracksReq(msg);     break;
        case PLAY_REQ:           handlePlayReq(msg);          break;
        case PLAY_TRACK_REQ:     handlePlayTrackReq(msg);     break;
        case PLAY_CONTROL_REQ:   handlePlayControlReq(msg);   break;
        case SET_VOLUME_REQ:     handleSetVolumeReq(msg);     break;
        case GENERIC_SEARCH_REQ: handleGenericSearchReq(msg); break;
        case GET_STATUS_REQ:     handleGetStatusReq(msg);     break;
        case GET_IMAGE_REQ:      handleGetImageReq(msg);      break;
        case GET_ALBUM_REQ:      handleGetAlbumReq(msg);      break;
        case GET_ARTIST_REQ:     handleGetArtistReq(msg);     break;

        case ADD_AUDIO_ENDPOINTS_REQ:         handleAddAudioEpReq(msg);        break;
        case REM_AUDIO_ENDPOINTS_REQ:         handleRemAudioEpReq(msg);        break;
        case GET_CURRENT_AUDIO_ENDPOINTS_REQ: handleGetCurrentAudioEpReq(msg); break;

        case CREATE_AUDIO_ENDPOINT_REQ: handleCreateAudioEpReq(msg); break;
        case DELETE_AUDIO_ENDPOINT_REQ: handleDeleteAudioEpReq(msg); break;
        case GET_AUDIO_ENDPOINTS_REQ:   handleGetAudioEpReq(msg); break;

        default:
            break;
    }
}

void Client::processResponse( const Message* rsp, void* userData )
{
}

void Client::connectionState( bool up )
{}
void Client::rootFolderUpdatedInd()
{
    log(LOG_DEBUG) << "Client::rootFolderUpdatedInd()";
}
static void addStatusMsgMandatoryParameters( Message* msg, PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume )
{
    msg->addTlv( TLV_STATE, state );
    msg->addTlv( TLV_PLAY_MODE_REPEAT, repeatStatus );
    msg->addTlv( TLV_PLAY_MODE_SHUFFLE, shuffleStatus );
    msg->addTlv( TLV_VOLUME, volume );
}
static void addStatusMsgOptionalParameters( Message* msg, const Track& currentTrack, unsigned int progress )
{
    msg->addTlv( currentTrack.toTlv() );
    msg->addTlv( TLV_PROGRESS, progress );
}

void Client::statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress )
{
    Message* msg = new Message(STATUS_IND);

    addStatusMsgMandatoryParameters( msg, state, repeatStatus, shuffleStatus, volume );
    addStatusMsgOptionalParameters( msg, currentTrack, progress );

    queueMessage( msg );
}

void Client::statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume )
{
    Message* msg = new Message(STATUS_IND);

    addStatusMsgMandatoryParameters( msg, state, repeatStatus, shuffleStatus, volume );

    queueMessage( msg );
}

void Client::getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress, void* userData )
{
    Message* rsp = (Message*) userData;

    addStatusMsgMandatoryParameters( rsp, state, repeatStatus, shuffleStatus, volume );
    addStatusMsgOptionalParameters( rsp, currentTrack, progress );

    queueMessage( rsp );
}
void Client::getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, void* userData )
{
    Message* rsp = (Message*) userData;

    addStatusMsgMandatoryParameters( rsp, state, repeatStatus, shuffleStatus, volume );

    queueMessage( rsp );
}


void Client::getPlaylistsResponse( const Folder& rootfolder, void* userData )
{
    Message* rsp = (Message*) userData;

    rsp->addTlv( rootfolder.toTlv() );

    queueMessage( rsp );
}

void Client::getTracksResponse( const std::deque<Track>& tracks, void* userData )
{
    Message* rsp = (Message*) userData;
    for (std::deque<Track>::const_iterator trackIt = tracks.begin(); trackIt != tracks.end(); trackIt++)
    {
        log(LOG_DEBUG) << "\t" << (*trackIt).getName();
        rsp->addTlv((*trackIt).toTlv());
    }
    log(LOG_DEBUG) << "#tracks found=" << tracks.size();

    queueMessage( rsp );
}

void Client::getAlbumResponse( const Album& album, void* userData )
{
    Message* rsp = (Message*) userData;
    TlvContainer* albumTlv = album.toTlv();
    const std::deque<Track>& tracks = album.getTracks();

    for (std::deque<Track>::const_iterator trackIt = tracks.begin(); trackIt != tracks.end(); trackIt++)
    {
        log(LOG_DEBUG) << "\t" << (*trackIt).getName();
        albumTlv->addTlv((*trackIt).toTlv());
    }

    rsp->addTlv(albumTlv);

    queueMessage( rsp );
}

void Client::getArtistResponse( const Artist& artist, void* userData )
{
    Message* rsp = (Message*) userData;

    TlvContainer* artistTlv = artist.toTlv();
    AlbumContainer albums = artist.getAlbums();
    for ( AlbumContainer::const_iterator it = albums.begin();
          it != albums.end(); it++ )
    {
        TlvContainer* albumTlv = (*it).toTlv();
        const std::deque<Track>& tracks = (*it).getTracks();

        for (std::deque<Track>::const_iterator trackIt = tracks.begin(); trackIt != tracks.end(); trackIt++)
        {
            albumTlv->addTlv((*trackIt).toTlv());
        }
        artistTlv->addTlv(albumTlv);
    }

    rsp->addTlv(artistTlv);

    queueMessage( rsp );
}

void Client::getImageResponse( const void* data, size_t dataSize, void* userData )
{
    Message* rsp = (Message*) userData;
    if(data && dataSize)
    {
        TlvContainer* image = new TlvContainer(TLV_IMAGE);
        image->addTlv(TLV_IMAGE_FORMAT, IMAGE_FORMAT_JPEG);
        image->addTlv(new BinaryTlv(TLV_IMAGE_DATA, (const uint8_t*)data, (uint32_t)dataSize));
        rsp->addTlv(image);
    }
    queueMessage( rsp );
}

void Client::genericSearchCallback( const std::deque<Track>& tracks, const std::string& didYouMean, void* userData )
{
    Message* rsp = (Message*) userData;

    for (std::deque<Track>::const_iterator trackIt = tracks.begin(); trackIt != tracks.end(); trackIt++)
    {
        log(LOG_DEBUG) << "\t" << (*trackIt).getName();
        rsp->addTlv((*trackIt).toTlv());
    }
    log(LOG_DEBUG) << "#tracks found=" << tracks.size();

    queueMessage( rsp );
}

void Client::handleGetTracksReq(const Message* msg)
{
    GetTracksReq* req = (GetTracksReq*)msg;
    log(LOG_DEBUG) << "get tracks: " << req->getPlaylist();

    Message* rsp  = msg->createResponse();

    spotify_.getTracks( req->getPlaylist(), this, rsp );
}

void Client::handleHelloReq(const Message* msg)
{
    Message* rsp = msg->createResponse();
    const IntTlv* protoMajorTlv = (const IntTlv*)msg->getTlvRoot()->getTlv(TLV_PROTOCOL_VERSION_MAJOR);
    const IntTlv* protoMinorTlv = (const IntTlv*)msg->getTlvRoot()->getTlv(TLV_PROTOCOL_VERSION_MINOR);

    if ( protoMajorTlv == NULL || protoMinorTlv == NULL )
    {
        /*we probably need to know the protocol version*/
        rsp->addTlv(TLV_FAILURE, FAIL_MISSING_TLV);
    }
    else
    {
        peerProtocolMajor_ = protoMajorTlv->getVal();
        peerProtocolMinor_ = protoMinorTlv->getVal();

        if (peerProtocolMajor_ != PROTOCOL_VERSION_MAJOR)
        {
            /*not compatible if major version differ, we accept differences in minor even though functionality might be missing*/
            rsp->addTlv(TLV_FAILURE, FAIL_PROTOCOL_MISMATCH);
        }
        else
        {
            const StringTlv* idTlv = (const StringTlv*)msg->getTlvRoot()->getTlv(TLV_LINK);
            if ( idTlv != NULL )
            {
                std::string newId = idTlv->getString();
                if ( !newId.empty() )
                {
                    epDb_.unregisterId( this );
                    id = newId;
                    epDb_.registerId( this );
                }
            }
#if 0
            const StringTlv* usernameTlv = (const StringTlv*)msg->getTlvRoot()->getTlv(TLV_LOGIN_USERNAME);
            const StringTlv* passwordTlv = (const StringTlv*)msg->getTlvRoot()->getTlv(TLV_LOGIN_PASSWORD);
            std::string username = usernameTlv ? usernameTlv->getString() : std::string("");
            std::string password = passwordTlv ? passwordTlv->getString() : std::string("");
            if ( (!networkUsername_.empty() && networkUsername_ != username) ||
                 (!networkPassword_.empty() && networkPassword_ != password)    )
            {
                /*incorrect login*/
                log(LOG_DEBUG) << networkUsername_.empty() << " " << networkUsername_ << " " << username;
                log(LOG_DEBUG) << networkPassword_.empty() << " " << networkPassword_ << " " << password;
                rsp->addTlv(TLV_FAILURE, FAIL_BAD_LOGIN);
            }
            else
#endif
            {
                /*all is well!*/
                loggedIn_ = true;
            }
        }
    }
    rsp->addTlv(TLV_PROTOCOL_VERSION_MAJOR, PROTOCOL_VERSION_MAJOR);
    rsp->addTlv(TLV_PROTOCOL_VERSION_MINOR, PROTOCOL_VERSION_MINOR);

    queueMessage( rsp );
}

void Client::handleGetPlaylistsReq(const Message* msg)
{
    Message* rsp = msg->createResponse();

    spotify_.getPlaylists( this, rsp );
}

void Client::handleGetStatusReq(const Message* msg)
{
    Message* rsp = msg->createResponse();

    spotify_.getStatus( this, rsp );
}

void Client::handlePlayReq(const Message* msg)
{
    const StringTlv* url = (const StringTlv*)msg->getTlvRoot()->getTlv(TLV_LINK);

    if ( url )
    {
        const IntTlv* startIndex = (const IntTlv*)msg->getTlvRoot()->getTlv(TLV_TRACK_INDEX);
        log(LOG_DEBUG) << "spotify_.play(" << url->getString() << ")";
        if ( startIndex != NULL )
        {
            spotify_.play( url->getString(), startIndex->getVal(), this, NULL );
        }
        else
        {
            spotify_.play( url->getString(), this, NULL );
        }
    }

    Message* rsp = msg->createResponse();
    queueMessage( rsp );
}

void Client::handlePlayTrackReq(const Message* msg)
{
    const TlvContainer* track = (const TlvContainer*) msg->getTlvRoot()->getTlv(TLV_TRACK);
    const StringTlv* url = (const StringTlv*) track->getTlv(TLV_LINK);
    log(LOG_DEBUG) << "spotify_.play(" << url->getString() << ")";
    spotify_.play( url->getString(), this, NULL );

    Message* rsp = msg->createResponse();
    queueMessage( rsp );
}

void Client::handlePlayControlReq(const Message* msg)
{
    for (TlvContainer::const_iterator it = msg->getTlvRoot()->begin() ; it != msg->getTlvRoot()->end() ; it++)
    {
        Tlv* tlv = (*it);
        log(LOG_DEBUG) << *tlv;

        switch(tlv->getType())
        {
            case TLV_PLAY_OPERATION:
            {
                switch(((IntTlv*)tlv)->getVal())
                {
                    case PLAY_OP_NEXT:
                        spotify_.next();
                        break;
                    case PLAY_OP_PREV:
                        spotify_.previous();
                        break;
                    case PLAY_OP_PAUSE:
                        spotify_.pause();
                        break;
                    case PLAY_OP_RESUME:
                        spotify_.resume();
                        break;
                    default:
                        break;
                }
            }
            break;

            case TLV_PLAY_MODE_SHUFFLE:
            {
                spotify_.setShuffle( (((IntTlv*)tlv)->getVal() != 0) );
            }
            break;

            case TLV_PLAY_MODE_REPEAT:
            {
                spotify_.setRepeat( (((IntTlv*)tlv)->getVal() != 0) );
            }
            break;

            case TLV_PROGRESS:
            {
                spotify_.seek(((IntTlv*)tlv)->getVal());
            }
            break;

            default:
                break;
        }
    }
    Message* rsp = msg->createResponse();
    queueMessage( rsp );
}

void Client::handleGetImageReq(const Message* msg)
{
    const StringTlv* link = (const StringTlv*) msg->getTlv(TLV_LINK);
    Message* rsp = msg->createResponse();
    spotify_.getImage( link ? link->getString() : std::string(""), this, rsp );
}

void Client::handleGenericSearchReq(const Message* msg)
{
    const StringTlv* query = (const StringTlv*) msg->getTlv(TLV_SEARCH_QUERY);

    if (query && query->getString() != "")
    {
        Message* rsp = msg->createResponse();
        spotify_.search( query->getString(), this, rsp );
    }
    else
    {
        /*error handling?*/
    }
}

void Client::handleGetAlbumReq(const Message* msg)
{
    const StringTlv* link = (const StringTlv*) msg->getTlvRoot()->getTlv(TLV_LINK);
    Message* rsp = msg->createResponse();
    spotify_.getAlbum( link ? link->getString() : std::string(""), this, rsp );
}

void Client::handleGetArtistReq(const Message* msg)
{
    const StringTlv* link = (const StringTlv*) msg->getTlvRoot()->getTlv(TLV_LINK);
    Message* rsp = msg->createResponse();
    spotify_.getArtist( link ? link->getString() : std::string(""), this, rsp );
}

void Client::handleAddAudioEpReq( const Message* msg )
{
    Message* rsp = msg->createResponse();
    const StringTlv* idTlv = (const StringTlv*) msg->getTlv( TLV_LINK );
    std::string endpointId = idTlv ? idTlv->getString() : "";
    if (endpointId == "")
        endpointId = id; /* no id specified means this client*/

    //todo handle multiple tlvs
    audioCtrl_.addEndpoint( endpointId, this, rsp );
    queueMessage( rsp );
}

void Client::handleRemAudioEpReq( const Message* msg )
{
    Message* rsp = msg->createResponse();
    const StringTlv* idTlv = (const StringTlv*) msg->getTlv( TLV_LINK );
    std::string endpointId = idTlv ? idTlv->getString() : id; /* no id specified means this client*/

    //todo handle multiple tlvs
    audioCtrl_.removeEndpoint( endpointId, this, rsp );
    queueMessage( rsp );
}

void Client::handleGetCurrentAudioEpReq( const Message* msg )
{
    Message* rsp = msg->createResponse();
    spotify_.getCurrentAudioEndpoints( this, rsp );
}

void Client::getCurrentAudioEndpointsResponse( const std::set<std::string> endpoints, void* userData )
{
    Message* rsp = (Message*) userData;

    for (std::set<std::string>::const_iterator it = endpoints.begin(); it != endpoints.end(); it++)
    {
        log(LOG_DEBUG) << (*it);
        rsp->addTlv(TLV_LINK, (*it) );
    }

    queueMessage( rsp );
}


void Client::handleCreateAudioEpReq( const Message* msg )
{
    Message* rsp = msg->createResponse();
    const TlvContainer* epTlv = (const TlvContainer*) msg->getTlv( TLV_CLIENT );

    if ( epTlv )
    {
        const IntTlv* portTlv = (const IntTlv*) epTlv->getTlv( TLV_PORT );
        //const IntTlv* protoTlv = (const IntTlv*) epTlv->getTlv( TLV_AUDIO_EP_PROTOCOL );

        if ( portTlv /* && protoTlv */ )
        {
            const std::string& ip = getSocket()->getRemoteAddr();
            const uint32_t port = portTlv->getVal();
            const IntTlv* volumeTlv = (const IntTlv*) epTlv->getTlv( TLV_VOLUME );
            uint8_t relativeVolume = volumeTlv ? volumeTlv->getVal() : 0;

            std::ostringstream portStr;
            portStr << port;

            if ( audioEp )
            {
                audioCtrl_.deleteEndpoint( *audioEp, NULL, NULL );
                audioEp->destroy();
                delete audioEp;
            }

            audioEp = new Platform::AudioEndpointRemote( this, *this, ip, portStr.str(), relativeVolume, 1);
            audioCtrl_.createEndpoint(*audioEp, NULL, NULL);
        }
        else
        {
            log(LOG_NOTICE) << "Port TLV missing from message, add errorcode in response here!";
        }
    }

    queueMessage( rsp );
}

void Client::handleDeleteAudioEpReq( const Message* msg )
{
    Message* rsp = msg->createResponse();
    if ( audioEp )
    {
        audioCtrl_.deleteEndpoint( *audioEp, NULL, NULL );
        audioEp->destroy();
        delete audioEp;
    }
    audioEp = NULL;

    queueMessage( rsp );
}

void Client::endpointsUpdatedNtf()
{
    Message* ind = new Message( AUDIO_ENDPOINTS_UPDATED_IND );
    queueMessage( ind );
}

void Client::getEndpointsResponse( const AudioEndpointInfoList endpoints, void* userData )
{
    Message* rsp = (Message*) userData;

    for ( AudioEndpointInfoList::const_iterator it = endpoints.begin(); it != endpoints.end(); it++ )
    {
        TlvContainer* epTlv = new TlvContainer( TLV_CLIENT );
        epTlv->addTlv( TLV_LINK, (*it).id );
        epTlv->addTlv( TLV_STATE, (*it).active ? 1 : 0 );
        epTlv->addTlv( TLV_VOLUME, (*it).relativeVolume );
        rsp->addTlv( epTlv );
    }

    queueMessage( rsp );
}
void Client::handleGetAudioEpReq( const Message* msg )
{
    Message* rsp = msg->createResponse();
    audioCtrl_.getEndpoints( this, rsp );
}

void Client::handleSetVolumeReq(const Message* msg)
{
    for (TlvContainer::const_iterator it = msg->getTlvRoot()->begin() ; it != msg->getTlvRoot()->end() ; it++)
    {
        Tlv* tlv = (*it);
        log(LOG_DEBUG) << *tlv;

        switch(tlv->getType())
        {
            case TLV_VOLUME:
                spotify_.setVolume( ((IntTlv*)tlv)->getVal() );
                break;
            case TLV_CLIENT:
            {
                TlvContainer* epTlv = (TlvContainer*)tlv;
                StringTlv* idTlv  = (StringTlv*)epTlv->getTlv( TLV_LINK );
                IntTlv* volume = (IntTlv*)epTlv->getTlv( TLV_VOLUME );
                if ( idTlv && volume )
                    audioCtrl_.setRelativeVolume( idTlv->getString(), volume->getVal() );
                break;
            }
            default:
                break;
        }
    }

    Message* rsp = msg->createResponse();
    queueMessage( rsp );
}

void Client::setMasterVolume( uint8_t volume )
{
    Message* msg = new Message( SET_VOLUME_REQ );
    msg->addTlv( TLV_VOLUME, volume );
    queueRequest( msg, NULL, NULL );
}

void Client::setRelativeVolume( uint8_t volume )
{
    Message* msg = new Message( SET_VOLUME_REQ );
    TlvContainer* tlv = new TlvContainer( TLV_CLIENT );
    tlv->addTlv( TLV_VOLUME, volume );
    msg->addTlv( tlv );
    queueRequest( msg, NULL, NULL );
}


void Client::rename( std::string& newId )
{
    id = newId;

    // todo: send to client
}
