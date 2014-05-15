/*
 * Copyright (c) 2012, Jesper Derehag
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
 * DISCLAIMED. IN NO EVENT SHALL JESPER DEREHAG BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "LibSpotifyIf.h"
#include "LibSpotifyIfCallbackWrapper.h"
#include "LibSpotifyIfHelpers.h"
#include "applog.h"
#include "MediaContainers/Artist.h"

#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>


namespace LibSpotify {


static const char* getEventName(LibSpotifyIf::EventItem* event);
void seekCb( void* arg, uint32_t sec );
void volumeCb( void* arg, uint32_t volume );

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * public methods
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
LibSpotifyIf::LibSpotifyIf(const ConfigHandling::SpotifyConfig& config) :config_(config),
                                                                         rootFolder_("root", 0, 0),
                                                                         state_(STATE_INVALID),
                                                                         nextTimeoutForLibSpotify(0),
                                                                         playbackHandler_(*this),
                                                                         seekFilter( 500, seekCb, this ),
                                                                         volumeFilter( 100, volumeCb, this ),
                                                                         itsCallbackWrapper_(*this),
                                                                         trackState_(TRACK_STATE_NOT_LOADED),
                                                                         currentTrack_("","")
{
    libSpotifySessionCreate();
    startThread();
}

LibSpotifyIf::~LibSpotifyIf()
{
	/*
	sp_playlistcontainer_remove_callbacks(sp_session_playlistcontainer(spotifySession_),
										  itsCallbackWrapper_.getRegisteredPlaylistContainerCallbacks(),
										  NULL);*/
	sp_session_release(spotifySession_);
}
void LibSpotifyIf::destroy()
{
	cancelThread();
    cond_.signal();
	joinThread();
}

void LibSpotifyIf::logIn()
{
    postToEventThread( new EventItem( EVENT_LOGGING_IN ) );
}

void LibSpotifyIf::logOut()
{
    postToEventThread( new EventItem( EVENT_LOGGING_OUT ) );
}

void LibSpotifyIf::getPlaylists( IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    subscriber->getPlaylistsResponse( rootFolder_, userData );
}

void LibSpotifyIf::getTracks( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new QueryReqEventItem( EVENT_GET_TRACKS, subscriber, userData, link ) );
}

void LibSpotifyIf::playlistAddTracks( const std::string& playlistlink, const std::list<const std::string>& tracklinks, int index, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new AddToPlaylistEventItem( subscriber, userData, playlistlink, tracklinks, index ) );
}
void LibSpotifyIf::playlistRemoveTracks( const std::string& playlistlink, const std::set<int>& indexes, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new ModifyPlaylistEventItem( EVENT_REMOVE_FROM_PLAYLIST, subscriber, userData, playlistlink, indexes ) );
}
void LibSpotifyIf::playlistMoveTracks( const std::string& playlistlink, const std::set<int>& indexes, int toIndex, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new MoveTracksEventItem( subscriber, userData, playlistlink, indexes, toIndex ) );
}

void LibSpotifyIf::search( const std::string& query, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new QueryReqEventItem( EVENT_GENERIC_SEARCH, subscriber, userData, query ) );
}

void LibSpotifyIf::getImage( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new QueryReqEventItem( EVENT_GET_IMAGE, subscriber, userData, link ) );
}

void LibSpotifyIf::getAlbum( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new QueryReqEventItem( EVENT_GET_ALBUM, subscriber, userData, link ) );
}

void LibSpotifyIf::getArtist( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new QueryReqEventItem( EVENT_GET_ARTIST, subscriber, userData, link ) );
}

void LibSpotifyIf::getStatus( IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    switch ( trackState_ )
    {
        case TRACK_STATE_NOT_LOADED:
            subscriber->getStatusResponse( PLAYBACK_IDLE,
                                           playbackHandler_.getRepeat(),
                                           playbackHandler_.getShuffle(),
                                           audioOut_.getVolume(),
                                           userData );
            break;
        case TRACK_STATE_PAUSED:
            subscriber->getStatusResponse( PLAYBACK_PAUSED,
                                           playbackHandler_.getRepeat(),
                                           playbackHandler_.getShuffle(),
                                           audioOut_.getVolume(),
                                           currentTrack_,
                                           progress_/10,
                                           userData );
            break;
        case TRACK_STATE_PLAYING:
            subscriber->getStatusResponse( PLAYBACK_PLAYING,
                                           playbackHandler_.getRepeat(),
                                           playbackHandler_.getShuffle(),
                                           audioOut_.getVolume(),
                                           currentTrack_,
                                           progress_/10,
                                           userData );
            break;
    }
}
void LibSpotifyIf::doStatusNtf()
{
    callbackSubscriberMtx_.lock();
    std::set<IMediaInterfaceCallbackSubscriber*>::iterator it = callbackSubscriberList_.begin();

    switch ( trackState_ )
    {
        case TRACK_STATE_NOT_LOADED:
            for( ; it != callbackSubscriberList_.end(); it++)
            {
                (*it)->statusUpdateInd( PLAYBACK_IDLE,
                                        playbackHandler_.getRepeat(),
                                        playbackHandler_.getShuffle(),
                                        audioOut_.getVolume() );
            }
            break;
        case TRACK_STATE_PAUSED:
            for( ; it != callbackSubscriberList_.end(); it++)
            {
                (*it)->statusUpdateInd( PLAYBACK_PAUSED,
                                        playbackHandler_.getRepeat(),
                                        playbackHandler_.getShuffle(),
                                        audioOut_.getVolume(),
                                        currentTrack_,
                                        progress_/10 );
            }
            break;
        case TRACK_STATE_PLAYING:
            for( ; it != callbackSubscriberList_.end(); it++)
            {
                (*it)->statusUpdateInd( PLAYBACK_PLAYING,
                                        playbackHandler_.getRepeat(),
                                        playbackHandler_.getShuffle(),
                                        audioOut_.getVolume(),
                                        currentTrack_,
                                        progress_/10 );
            }
            break;
    }
    callbackSubscriberMtx_.unlock();
}

/* called from playbackHandler, used when a track is ACTUALLY to be played,
 * All others should just enqueue to the PlayBackHandler */
void LibSpotifyIf::playTrack(const Track& track)
{
    postToEventThread( new TrackEventItem(EVENT_PLAY_TRACK, track) );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PlaybackHandler requests
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void LibSpotifyIf::play( const std::string& link, int startIndex, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new PlayReqEventItem(subscriber, userData, link, startIndex) );
}
void LibSpotifyIf::play( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    play( link, -1, subscriber, userData );
}

void LibSpotifyIf::stop()
{
    postToEventThread( new EventItem( EVENT_STOP_REQ ) );
}

void LibSpotifyIf::enqueue( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new QueryReqEventItem( EVENT_ENQUEUE_REQ, subscriber, userData, link ) );
}

void LibSpotifyIf::pause()
{
    postToEventThread( new EventItem( EVENT_PAUSE_PLAYBACK ) );
}

void LibSpotifyIf::resume()
{
    postToEventThread( new EventItem( EVENT_RESUME_PLAYBACK ) );
}

void LibSpotifyIf::next()
{
    postToEventThread( new EventItem( EVENT_NEXT_TRACK ) );
}

void LibSpotifyIf::previous()
{
    postToEventThread( new EventItem( EVENT_PREVIOUS_TRACK ) );
}

void LibSpotifyIf::seek( uint32_t sec )
{
    seekFilter.Event( sec );
}

void LibSpotifyIf::doSeek( uint32_t sec )
{
    postToEventThread( new ParamEventItem( EVENT_SEEK, sec ) );
}

void seekCb( void* arg, uint32_t sec )
{
    LibSpotifyIf* this_ = static_cast<LibSpotifyIf*>( arg );
    this_->doSeek( sec );
}

void LibSpotifyIf::setShuffle( bool shuffleOn )
{
    if ( playbackHandler_.getShuffle() != shuffleOn )
    {
        playbackHandler_.setShuffle( shuffleOn );
        doStatusNtf();
    }
}

void LibSpotifyIf::setRepeat( bool repeatOn )
{
    if ( playbackHandler_.getRepeat() != repeatOn )
    {
        playbackHandler_.setRepeat( repeatOn );
        doStatusNtf();
    }
}

void LibSpotifyIf::setVolume( uint8_t volume )
{
    volumeFilter.Event( volume );
}

void LibSpotifyIf::doVolume( uint32_t volume )
{
    audioOut_.setVolume( volume );
    doStatusNtf();
}

void volumeCb( void* arg, uint32_t volume )
{
    LibSpotifyIf* this_ = static_cast<LibSpotifyIf*>( arg );
    this_->doVolume( volume );
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * private methods
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void LibSpotifyIf::run()
{
    while(isCancellationPending() == false)
    {
        eventQueueMtx_.lock();
        if (nextTimeoutForLibSpotify == 0)
        {
            while(eventQueue_.empty())cond_.wait(eventQueueMtx_);
        }
        else if(eventQueue_.empty())
        {
            cond_.timedWait(eventQueueMtx_, nextTimeoutForLibSpotify);
        }

        EventItem* currentEvent;

        if(eventQueue_.empty())
        {
            currentEvent = new EventItem( EVENT_ITERATE_MAIN_LOOP );
        }
        else
        {
            currentEvent = eventQueue_.front();
            eventQueue_.pop();
        }
        eventQueueMtx_.unlock();
        stateMachineEventHandler(currentEvent);
        delete currentEvent;
    }
    log(LOG_DEBUG) << "Exiting LibSpotifyIf::run()";
}

void LibSpotifyIf::stateMachineEventHandler(EventItem* event)
{
    if(event->event_ != EVENT_ITERATE_MAIN_LOOP)
        log(LOG_DEBUG) << "Event received:" << getEventName(event);
    switch(event->event_)
    {
        case EVENT_METADATA_UPDATED:
            while(!pendingMetadata.empty())
            {
                EventItem* item = pendingMetadata.front();
                postToEventThread(item);
                pendingMetadata.pop();
            }
            break;

        case EVENT_GET_TRACKS:
            {
                if ( state_ == STATE_LOGGED_IN )
                {
                    QueryReqEventItem* reqEvent = static_cast<QueryReqEventItem*>( event );

                    const char* playlist_uri = reqEvent->query_.c_str();
                    sp_link* link = sp_link_create_from_string( playlist_uri );
                    if ( link )
                    {
                        if( sp_link_type( link ) == SP_LINKTYPE_PLAYLIST )
                        {
                            sp_playlist* playlist = sp_playlist_create(spotifySession_, link);
                            if (sp_playlist_is_loaded(playlist))
                            {
                                PendingMediaRequestData reqData = reqEvent->reqData;
                                log(LOG_DEBUG) << "Got playlist " << playlist_uri;
                                Playlist playlistObj = spotifyGetPlaylist(playlist, spotifySession_);

                                reqData.first->getTracksResponse( playlistObj.getTracks(), reqData.second );
                            }
                            else
                            {
                                /*not sure if we're waiting for metadata or playlist_state_changed here*/
                                pendingMetadata.push(new QueryReqEventItem(*reqEvent) );
                                log(LOG_DEBUG) << "Waiting for metadata for playlist " << playlist_uri;
                            }
                        }
                        else
                        {
                            log(LOG_WARN) << "Link is not a playlist: " << playlist_uri;
                        }
                        sp_link_release(link);
                    }
                    else
                    {
                        log(LOG_WARN) << "Bad link: " << playlist_uri;
                    }
                }
            }
            break;

        case EVENT_GET_ALBUM:
        {
            QueryReqEventItem* reqEvent = static_cast<QueryReqEventItem*>( event );
            const char* album_uri = reqEvent->query_.c_str();
            if (state_ == STATE_LOGGED_IN)
            {
                sp_link* link = sp_link_create_from_string(album_uri);
                if (link)
                {
                    if (sp_link_type(link) == SP_LINKTYPE_ALBUM)
                    {
                        sp_album* album = sp_link_as_album(link);
                        sp_albumbrowse_create( spotifySession_, album, &LibSpotifyIfCallbackWrapper::albumLoadedCallback, new QueryReqEventItem(*reqEvent));
                    }
                    else
                    {
                        log(LOG_WARN) << "Link is not an album: " << album_uri;
                    }
                    sp_link_release(link);
                }
                else
                {
                    log(LOG_WARN) << "Bad link: " << album_uri;
                }
            }
            break;
        }

        case EVENT_GET_ARTIST:
        {
            QueryReqEventItem* reqEvent = static_cast<QueryReqEventItem*>( event );
            const char* artist_uri = reqEvent->query_.c_str();
            if (state_ == STATE_LOGGED_IN)
            {
                sp_link* link = sp_link_create_from_string(artist_uri);
                if (link)
                {
                    if (sp_link_type(link) == SP_LINKTYPE_ARTIST)
                    {
                        sp_artist* artist = sp_link_as_artist(link);
                        sp_artistbrowse_create( spotifySession_, artist, SP_ARTISTBROWSE_NO_TRACKS, &LibSpotifyIfCallbackWrapper::artistLoadedCallback, new QueryReqEventItem(*reqEvent));
                    }
                    else
                    {
                        log(LOG_WARN) << "Link is not an album: " << artist_uri;
                    }
                    sp_link_release(link);
                }
                else
                {
                    log(LOG_WARN) << "Bad link: " << artist_uri;
                }
            }
            break;
        }

        case EVENT_GENERIC_SEARCH:
        {
            QueryReqEventItem* reqEvent = static_cast<QueryReqEventItem*>( event );
            /* search, but only interested in the first 100 results,*/
            sp_search_create(spotifySession_,
                             reqEvent->query_.c_str(),
                             0,   /* track_offset */
                             100, /* track_count */
                             0,   /* album_offset */
                             100, /* album_count */
                             0,   /* artist_offset */
                             100, /* artist_count */
                             0,   /* playlist_offset */
                             100, /* playlist_count */
                             SP_SEARCH_STANDARD,
                             &LibSpotifyIfCallbackWrapper::genericSearchCallback,
                             new QueryReqEventItem(*reqEvent));
            break;
        }

        case EVENT_GET_IMAGE:
        {
            QueryReqEventItem* reqEvent = static_cast<QueryReqEventItem*>( event );

            const std::string& linkStr = reqEvent->query_.empty() ? currentTrack_.getAlbumLink() : reqEvent->query_;

            if (!linkStr.empty())
            {
                sp_link* link = sp_link_create_from_string(linkStr.c_str());
                if (link)
                {
                    const byte* imgRef = NULL;
                    switch ( sp_link_type(link) )
                    {
                        case SP_LINKTYPE_ALBUM:
                        {
                            sp_album* album = sp_link_as_album(link);

                            if ( sp_album_is_loaded(album) )
                            {
                                imgRef = sp_album_cover( album, SP_IMAGE_SIZE_NORMAL ); //todo image size should be in remote interface
                                loadAndSendImage( imgRef, reqEvent );
                            }
                            else
                            {
                                log(LOG_WARN) << "No metadata for album";
                                sp_albumbrowse_create( spotifySession_, album, &LibSpotifyIfCallbackWrapper::albumLoadedCallback, new QueryReqEventItem( *reqEvent ));
                            }
                        }
                        break;
                    
                        case SP_LINKTYPE_ARTIST:
                        {
                            sp_artist* artist = sp_link_as_artist(link);
                            /* -- sp_artist_portrait seems broken, always gets null here
                            if ( sp_artist_is_loaded( artist ) )
                            {
                                imgRef = sp_artist_portrait( artist, SP_IMAGE_SIZE_NORMAL );
                                loadAndSendImage( imgRef, reqEvent );
                            }
                            else*/
                            {
                                sp_artistbrowse_create( spotifySession_, artist, SP_ARTISTBROWSE_NO_ALBUMS, &LibSpotifyIfCallbackWrapper::artistLoadedCallback, new QueryReqEventItem( *reqEvent ));
                            }
                        }
                        break;

                        case SP_LINKTYPE_PLAYLIST:
                        {
                            sp_playlist* playlist = sp_playlist_create( spotifySession_, link );
                            if ( sp_playlist_is_loaded( playlist ) )
                            {
                                byte img[20];
                                if ( sp_playlist_get_image( playlist, img ) )
                                {
                                    loadAndSendImage( img, reqEvent );
                                }
                                else
                                {
                                    loadAndSendImage( NULL, reqEvent );
                                }
                            }
                            else
                            {
                                pendingMetadata.push( new QueryReqEventItem( *reqEvent ) );
                            }
                        }
                        break;

                        default:
                        {
                            /*not supported yet*/
                            PendingMediaRequestData reqData = reqEvent->reqData;
                            reqData.first->getImageResponse( NULL, 0, reqData.second );
                        }
                    }

                    sp_link_release(link);
                }
                else
                {
                    log(LOG_WARN) << "Bad link?" << linkStr;
                    PendingMediaRequestData reqData = reqEvent->reqData;
                    reqData.first->getImageResponse( NULL, 0, reqData.second );
                }

            }
        }
        break;

        case EVENT_PLAY_REQ:
        {
            PlayReqEventItem* reqEvent = static_cast<PlayReqEventItem*>( event );
            if (state_ == STATE_LOGGED_IN)
            {
                sp_link* link = sp_link_create_from_string(reqEvent->query_.c_str());
                if(link)
                {
                    switch(sp_link_type(link))
                    {
                        case SP_LINKTYPE_TRACK:
                        case SP_LINKTYPE_LOCALTRACK:
                            {
                                sp_track* track = sp_link_as_track(link);
                                Track trackObj(spotifyGetTrack(track, spotifySession_));
                                playbackHandler_.playTrack(trackObj);
                            }
                            break;

                        case SP_LINKTYPE_PLAYLIST:
                            {
                                sp_playlist* playlist = sp_playlist_create( spotifySession_, link );
                                if ( sp_playlist_is_loaded( playlist ) )
                                {
                                    Playlist playlistObj( spotifyGetPlaylist( playlist, spotifySession_ ) );
                                    playbackHandler_.playPlaylist( playlistObj, reqEvent->startIndex_ );
                                    log(LOG_NOTICE) << "Adding " << reqEvent->query_ << " (" << playlistObj.getLink() << ") with " << playlistObj.getTracks().size() << " tracks to playbackhandler";
                                }
                                else
                                {
                                    pendingMetadata.push( new PlayReqEventItem( *reqEvent ) );
                                    log(LOG_DEBUG) << "Waiting for metadata for playlist " << reqEvent->query_;
                                }
                            }
                            break;

                        case SP_LINKTYPE_ALBUM:
                            {
                                sp_album* album = sp_link_as_album(link);
                                sp_albumbrowse_create (spotifySession_, album, &LibSpotifyIfCallbackWrapper::albumLoadedCallback, new PlayReqEventItem( *reqEvent ) );
                                log(LOG_NOTICE) << "Created sp_albumbrowse for " << reqEvent->query_ << ", waiting for load finished callback";
                            }
                            break;

                        /* FALL_THROUGH */
                        case SP_LINKTYPE_SEARCH:
                        case SP_LINKTYPE_ARTIST:
                        default:
                            log(LOG_EMERG) << "Unknown link type!";
                            break;
                    }
                    sp_link_release(link);
                }
            }

            break;
        }

        case EVENT_ENQUEUE_REQ:
            if (state_ == STATE_LOGGED_IN)
            {
                QueryReqEventItem* reqEvent = static_cast<QueryReqEventItem*>( event );
                sp_link* link = sp_link_create_from_string(reqEvent->query_.c_str());
                if ( link )
                {
                    switch(sp_link_type(link))
                    {
                        case SP_LINKTYPE_TRACK:
                        case SP_LINKTYPE_LOCALTRACK:
                        {
                            sp_track* track = sp_link_as_track(link);
                            Track trackObj(spotifyGetTrack(track, spotifySession_));
                            playbackHandler_.enqueueTrack(trackObj);
                        }
                        break;
                    }
                }
            }
            break;

        case EVENT_STOP_REQ:
            if (trackState_ != TRACK_STATE_NOT_LOADED)
            {
                sp_session_player_play(spotifySession_, 0);
                audioOut_.flushAudioData();

                trackState_ = TRACK_STATE_NOT_LOADED;
                progress_ = 0;

                doStatusNtf();
            }
            break;

        case EVENT_PAUSE_PLAYBACK:
            if (trackState_ == TRACK_STATE_PLAYING)
            {
                sp_session_player_play(spotifySession_, 0);
                audioOut_.pause();
                trackState_ = TRACK_STATE_PAUSED;

                doStatusNtf();
            }
            break;

        case EVENT_RESUME_PLAYBACK:
            if (trackState_ == TRACK_STATE_PAUSED)
            {
                sp_session_player_play(spotifySession_, 1);
                audioOut_.resume();

                trackState_ = TRACK_STATE_PLAYING;

                doStatusNtf();
            }
            break;

        case EVENT_SEEK:
            if ( trackState_ != TRACK_STATE_NOT_LOADED )
            {
                ParamEventItem* seekEvent = static_cast<ParamEventItem*>(event);
                sp_session_player_seek( spotifySession_, seekEvent->param_ * 1000 );
                progress_ = seekEvent->param_ * 10000;
                doStatusNtf();
            }
            break;

        case EVENT_NEXT_TRACK:
            {
                {
                    log(LOG_DEBUG) << "Next track, progress of current: " << progress_;
                    progress_ = 0;

                    /* unload track, otherwise end of track callback will just be called repeatedly until a new track is loaded */
                    sp_session_player_unload(spotifySession_);

                    /* Tell all subscribers that the track has ended */
                    if ( trackState_ != TRACK_STATE_NOT_LOADED )
                    {
                        trackState_ = TRACK_STATE_NOT_LOADED; /*todo, this should happen when buffer is finished*/
                        doStatusNtf();
                    }

                    /* notify playbackhandler so it can load a new track */
                    playbackHandler_.playNext();
                }
            }
            break;

        case EVENT_PREVIOUS_TRACK:
            {
                {
                    unsigned int progress = progress_/10;
                    log(LOG_DEBUG) << "Previous track, progress of current: " << progress_;
                    progress_ = 0;

                    /* unload track */
                    sp_session_player_unload(spotifySession_);

                    if ( trackState_ != TRACK_STATE_NOT_LOADED )
                    {
                        /* Tell all subscribers that the track has ended */
                        trackState_ = TRACK_STATE_NOT_LOADED;
                        doStatusNtf();
                    }

                    /* notify playbackhandler so it can load a new track */
                    playbackHandler_.playPrevious( progress );
                }
            }
            break;

        case EVENT_PLAY_TRACK:
            {
                TrackEventItem* trackEvent = static_cast<TrackEventItem*>(event);
                sp_track* track = trackEvent->spTrack_;
                Track& trackObj = trackEvent->track_;
                sp_error err;
                if ( track == NULL )
                {
                    sp_link* link = sp_link_create_from_string(trackObj.getLink().c_str());
                    if( link )
                    {
                        assert( sp_link_type(link) == SP_LINKTYPE_TRACK || sp_link_type(link) == SP_LINKTYPE_LOCALTRACK );

                        track = sp_link_as_track(link);
                        sp_track_add_ref( track );
                    }
                    else
                    {
                        log(LOG_WARN) << "invalid link: " << trackObj.getLink();
                        break;
                    }
                    sp_link_release(link);
                }

                if ((err = sp_track_error(track)) == SP_ERROR_OK)
                {
                    if ((err = sp_session_player_load(spotifySession_, track)) == SP_ERROR_OK)
                    {
                        currentTrack_ = spotifyGetTrack(track, spotifySession_); //*trackObj; is not updated if we had to wait for metadata
                        currentTrack_.setIndex(trackObj.getIndex()); /*kind of a hack.. but only trackObj know where it came from, and thus which index it has (if it has one) */
                        trackState_ = TRACK_STATE_PLAYING;
                        sp_session_player_play(spotifySession_, 1);
                        audioOut_.resume();
                        progress_ = 0;

                        /* Tell all subscribers that the track is playing */
                        doStatusNtf();
                    }
                    else
                    {
                        log(LOG_WARN) << "Player load error for " << trackObj.getLink().c_str() << " (" << sp_error_message(err) << ")";
                        playbackHandler_.playNext(); /*todo: some proper handling here, this will put track on the history list */
                    }
                    sp_track_release( track );
                }
                else if (err == SP_ERROR_IS_LOADING)
                {
                    sp_track_add_ref( track );
                    trackEvent->spTrack_ = track;
                    pendingMetadata.push(new TrackEventItem( *trackEvent ));
                    log(LOG_DEBUG) << "Waiting for metadata for track " << trackObj.getLink();
                }
                else
                {
                    sp_track_release( track );
                    log(LOG_WARN) << "Track error for " << trackObj.getLink() << " (" << sp_error_message(err) << ")";
                    /* return some sort of error to client? */
                }
            }
                
            break;

        case EVENT_ADD_TO_PLAYLIST:
            {
                AddToPlaylistEventItem* addEvent = static_cast<AddToPlaylistEventItem*>(event);
                const char* playlist_uri = addEvent->query_.c_str();
                sp_link* link = sp_link_create_from_string( playlist_uri );
                if ( link )
                {
                    if( sp_link_type( link ) == SP_LINKTYPE_PLAYLIST )
                    {
                        sp_playlist* playlist = sp_playlist_create(spotifySession_, link);
                        sp_track** tracks = (sp_track**)malloc( sizeof(sp_track*) * addEvent->tracks_.size() );
                        int i = 0;
                        std::list<const std::string>::const_iterator it = addEvent->tracks_.begin();
                        for( ; it != addEvent->tracks_.end(); it++ )
                        {
                            sp_link* tlink = sp_link_create_from_string( (*it).c_str() );
                            if ( tlink )
                            {
                                if( sp_link_type( tlink ) == SP_LINKTYPE_TRACK || sp_link_type( tlink ) == SP_LINKTYPE_LOCALTRACK )
                                {
                                    tracks[i] = sp_link_as_track(tlink);
                                    i++;
                                }
                            }
                        }

                        if ( i > 0 )
                        {
                            int index = addEvent->index_;
                            if ( index < 0 )
                                index = sp_playlist_num_tracks( playlist );
                            sp_error err = sp_playlist_add_tracks( playlist, tracks, i, index, spotifySession_ );
                            if ( err != SP_ERROR_OK )
                                log(LOG_DEBUG) << sp_error_message(err);
                        }
                        free( tracks );
                    }
                }
            }
            break;

        case EVENT_REMOVE_FROM_PLAYLIST:
            {
                ModifyPlaylistEventItem* removeEvent = static_cast<ModifyPlaylistEventItem*>(event);
                const char* playlist_uri = removeEvent->query_.c_str();
                sp_link* link = sp_link_create_from_string( playlist_uri );
                if ( link )
                {
                    if( sp_link_type( link ) == SP_LINKTYPE_PLAYLIST )
                    {
                        sp_playlist* playlist = sp_playlist_create(spotifySession_, link);
                        int* tracks = (int*)malloc( sizeof(int) * removeEvent->tracks_.size() );
                        int i = 0;
                        for( std::set<int>::const_iterator it = removeEvent->tracks_.begin() ; it != removeEvent->tracks_.end(); it++ )
                        {
                            tracks[i] = (*it);
                            i++;
                        }

                        if ( i > 0 )
                        {
                            sp_error err = sp_playlist_remove_tracks( playlist, tracks, i );
                            if ( err != SP_ERROR_OK )
                                log(LOG_DEBUG) << sp_error_message(err);
                        }
                        free( tracks );
                    }
                }
            }
            break;

        case EVENT_MOVE_TRACKS:
            {
                MoveTracksEventItem* moveEvent = static_cast<MoveTracksEventItem*>(event);
                const char* playlist_uri = moveEvent->query_.c_str();
                sp_link* link = sp_link_create_from_string( playlist_uri );
                if ( link )
                {
                    if( sp_link_type( link ) == SP_LINKTYPE_PLAYLIST )
                    {
                        sp_playlist* playlist = sp_playlist_create(spotifySession_, link);
                        int* tracks = (int*)malloc( sizeof(int) * moveEvent->tracks_.size() );
                        int i = 0;
                        for( std::set<int>::const_iterator it = moveEvent->tracks_.begin() ; it != moveEvent->tracks_.end(); it++ )
                        {
                            tracks[i] = (*it);
                            i++;
                        }

                        if ( i > 0 )
                        {
                            sp_error err = sp_playlist_reorder_tracks( playlist, tracks, i, moveEvent->toIndex_ );
                            if ( err != SP_ERROR_OK )
                                log(LOG_DEBUG) << sp_error_message(err);
                        }
                        free( tracks );
                    }
                }
            }
            break;

        /* Session Handling*/
        case EVENT_LOGGING_IN:
            state_ = STATE_LOGGING_IN;
            log(LOG_NOTICE) << "Logging in as " << config_.getUsername();
            sp_session_login(spotifySession_, config_.getUsername().c_str(), config_.getPassword().c_str(), 0, NULL);
            break;

        case EVENT_LOGGED_IN:
            state_ = STATE_LOGGED_IN;

            sp_playlistcontainer_add_callbacks(
                sp_session_playlistcontainer(spotifySession_),
                itsCallbackWrapper_.getRegisteredPlaylistContainerCallbacks(),
                NULL);

            sp_session_preferred_bitrate( spotifySession_, SP_BITRATE_320k );

            connectionState( true );

            break;

        case EVENT_CONNECTION_LOST:
            // spotify will relogin automatically
            // we should probably keep track of this though and subscribe to connectionstate_updated
            break;

        case EVENT_LOGGING_OUT:
            state_ = STATE_LOGGING_OUT;
            log(LOG_NOTICE) << "Connection state=" << sp_session_connectionstate(spotifySession_);
            sp_session_logout(spotifySession_);
            break;

        case EVENT_LOGGED_OUT:
            state_ = STATE_LOGGED_OUT;
            break;

        default:
            break;
    }
    do
    {
        sp_session_process_events(spotifySession_, &nextTimeoutForLibSpotify);
    } while (nextTimeoutForLibSpotify == 0);
}

void LibSpotifyIf::postToEventThread(EventItem* event)
{
	eventQueueMtx_.lock();
	eventQueue_.push(event);
	eventQueueMtx_.unlock();
	cond_.signal();
}


void LibSpotifyIf::rootFolderLoaded()
{
    /* first time only */
    static bool isLoaded = false;
    if ( isLoaded )
        return;
    isLoaded = true;

        sp_playlistcontainer* plContainer = sp_session_playlistcontainer(spotifySession_);
    int numberOfPlaylists = sp_playlistcontainer_num_playlists(plContainer);
    if(numberOfPlaylists < 0)return;

    /* register callbacks for all known playlists */
    for (int playlistIndex = 0; playlistIndex < numberOfPlaylists; ++playlistIndex)
    {
        switch (sp_playlistcontainer_playlist_type(plContainer, playlistIndex))
        {
            case SP_PLAYLIST_TYPE_PLAYLIST:
            {
                sp_playlist* pl = sp_playlistcontainer_playlist(plContainer, playlistIndex);
                sp_playlist_add_callbacks( pl, itsCallbackWrapper_.getRegisteredPlaylistCallbacks(), NULL );
            }
            break;

            default:
                break;
        }
    }

    refreshRootFolder();
}

void LibSpotifyIf::playlistAdded( sp_playlist* pl )
{
    log( LOG_DEBUG ) << sp_playlist_name( pl );
    sp_playlist_add_callbacks( pl, itsCallbackWrapper_.getRegisteredPlaylistCallbacks(), NULL );
    // no need to refresh yet, playlist contains no information. refresh will be triggered by rename callback
}
void LibSpotifyIf::playlistRemoved( sp_playlist* pl )
{
    log( LOG_DEBUG ) << sp_playlist_name( pl );
    sp_playlist_remove_callbacks( pl, itsCallbackWrapper_.getRegisteredPlaylistCallbacks(), NULL );
    refreshRootFolder();
}
void LibSpotifyIf::playlistRenamed( sp_playlist* pl )
{
    log( LOG_DEBUG ) << sp_playlist_name( pl );
    refreshRootFolder();
}
void LibSpotifyIf::playlistsMoved()
{
    log( LOG_DEBUG );
    refreshRootFolder();
}

void LibSpotifyIf::playlistContentsUpdated( sp_playlist* pl )
{
    char uri[MAX_LINK_NAME_LENGTH];
    uri[0] = '\0';
    sp_link* link = sp_link_create_from_playlist( pl );
    if( link )
    {
        sp_link_as_string(link, uri, sizeof(uri));
        sp_link_release(link);

        std::string linkstring(uri);

        callbackSubscriberMtx_.lock();
        /* Tell all subscribers that this playlist has been updated */
        for(std::set<IMediaInterfaceCallbackSubscriber*>::iterator it = callbackSubscriberList_.begin();
            it != callbackSubscriberList_.end(); it++)
        {
            (*it)->playlistUpdatedInd( linkstring );
        }
        callbackSubscriberMtx_.unlock();
    }
}
void LibSpotifyIf::refreshRootFolder()
{
    sp_playlistcontainer* plContainer = sp_session_playlistcontainer(spotifySession_);
    int numberOfPlaylists = sp_playlistcontainer_num_playlists(plContainer);
    if(numberOfPlaylists < 0)return;

    /* create the root folder */
    Folder tmpRootFolder("root", 0, 0);

    Folder* currentFolder = &tmpRootFolder;

    for (int playlistIndex = 0; playlistIndex < numberOfPlaylists; ++playlistIndex)
    {
        switch (sp_playlistcontainer_playlist_type(plContainer, playlistIndex))
        {
            case SP_PLAYLIST_TYPE_PLAYLIST:
            {
                sp_playlist* pl = sp_playlistcontainer_playlist(plContainer, playlistIndex);
                Playlist playlist(spotifyGetPlaylist(pl, spotifySession_));
                currentFolder->addPlaylist(playlist);
                break;
            }
            case SP_PLAYLIST_TYPE_START_FOLDER:
            {
                char folderName[200];
                sp_playlistcontainer_playlist_folder_name(plContainer, playlistIndex, folderName, sizeof(folderName));
                unsigned long long id = sp_playlistcontainer_playlist_folder_id(plContainer, playlistIndex);
                Folder folder(folderName, id, currentFolder);
                currentFolder->addFolder(folder);
                currentFolder = &currentFolder->getFolders().back();
                break;
            }
            case SP_PLAYLIST_TYPE_END_FOLDER:
            {
                if(currentFolder != 0)
                    currentFolder = currentFolder->getParentFolder();
                break;
            }
            case SP_PLAYLIST_TYPE_PLACEHOLDER:
                break;
        }
    }

    if(rootFolder_ != tmpRootFolder)
    {
        log(LOG_DEBUG) << "Root folder updated!";
        rootFolder_ = tmpRootFolder;
        callbackSubscriberMtx_.lock();
        /* Tell all subscribers that the rootFolder has been updated */
        for(std::set<IMediaInterfaceCallbackSubscriber*>::iterator it = callbackSubscriberList_.begin();
            it != callbackSubscriberList_.end(); it++)
        {
            (*it)->rootFolderUpdatedInd();
        }
        callbackSubscriberMtx_.unlock();
    }
}



void LibSpotifyIf::sendGetImageRsp( sp_image* img, QueryReqEventItem* reqEvent )
{
    PendingMediaRequestData reqData = reqEvent->reqData;
    size_t dataSize;
    const void* data = sp_image_data(img, &dataSize);
    reqData.first->getImageResponse( data, dataSize, reqData.second );
    sp_image_release(img);
}

void LibSpotifyIf::loadAndSendImage( const byte* imgRef, QueryReqEventItem* reqEvent )
{
    bool imgOk = false;
    if ( imgRef )
    {
        sp_image* img = sp_image_create(spotifySession_, imgRef);
        if ( img )
        {
            sp_error error;
            error = sp_image_error(img);

            if ( sp_image_is_loaded(img) )
            {
                sendGetImageRsp(img, reqEvent);
                imgOk = true;
            }
            else if ( error == SP_ERROR_IS_LOADING )
            {
                log(LOG_DEBUG) << "waiting for image load";
                sp_image_add_load_callback(img, &LibSpotifyIfCallbackWrapper::imageLoadedCallback, new QueryReqEventItem(*reqEvent));
                imgOk = true;
            }
            else
            {
                log(LOG_WARN) << "Image load error: " << sp_error_message(error);
                sp_image_release(img);
            }
        }
    }

    if ( imgOk == false )
    {
        PendingMediaRequestData reqData = reqEvent->reqData;
        reqData.first->getImageResponse( NULL, 0, reqData.second );
    }
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * registered callbacks from libspotify
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
 * This callback is called when an attempt to login has succeeded or failed.
*/
void LibSpotifyIf::loggedInCb(sp_session *session, sp_error error)
{
	sp_user *me;
	const char *my_name;
	int cc;


	if (SP_ERROR_OK != error) {
		std::cerr << "failed to log in to Spotify: " << sp_error_message(error) << std::endl;
		sp_session_release(session);
		exit(4);
	}

	// Let us print the nice message...
	me = sp_session_user(session);
	my_name = (sp_user_is_loaded(me) ? sp_user_display_name(me) : sp_user_canonical_name(me));
	cc = sp_session_user_country(session);
	log(LOG_NOTICE) << "Logged in to Spotify as user " << my_name << "(registered in country: " <<
													(cc >> 8) << (cc & 0xff) << ")";


    postToEventThread( new EventItem( EVENT_LOGGED_IN ) );
}

void LibSpotifyIf::metadataUpdatedCb(sp_session *session)
{
    postToEventThread( new EventItem ( EVENT_METADATA_UPDATED ) );
}

void LibSpotifyIf::endOfTrackCb(sp_session *session)
{
    log(LOG_DEBUG) << "Track ended";
    postToEventThread( new EventItem( EVENT_NEXT_TRACK ) );
}

void LibSpotifyIf::loggedOutCb(sp_session *session)
{
	/*sp_session_user returns NULL if logged out, no username to print*/
    log(LOG_DEBUG) << "Logged out";
	postToEventThread( new EventItem( EVENT_LOGGED_OUT ) );
}

void LibSpotifyIf::logMessageCb(sp_session *session, const char *data)
{
    log(LOG_NOTICE) << data;
}

void LibSpotifyIf::connectionErrorCb(sp_session *session, sp_error error)
{

    log(LOG_WARN) << sp_error_message(error);
	postToEventThread( new EventItem( EVENT_CONNECTION_LOST ) );
}

void LibSpotifyIf::notifyLibSpotifyMainThreadCb(sp_session *session)
{
	postToEventThread( new EventItem( EVENT_ITERATE_MAIN_LOOP ) );
}

int LibSpotifyIf::musicDeliveryCb(sp_session *sess, const sp_audioformat *format,
                                  const void *frames, int num_frames)
{
    int n = 0;

    n = audioOut_.enqueueAudioData(format->channels, format->sample_rate, num_frames, static_cast<const int16_t*>(frames));

    progress_ += (n*10000)/format->sample_rate;
    return n;
}

void LibSpotifyIf::genericSearchCb(sp_search *search, void *userdata)
{
    QueryReqEventItem* msg = static_cast<QueryReqEventItem*>(userdata);
    PendingMediaRequestData reqData = msg->reqData;

    std::deque<Track> searchReply;
    std::string didYouMean(sp_search_did_you_mean(search));


    log(LOG_DEBUG) << "Tracks in total: " << sp_search_total_tracks(search);

    for (int i = 0; i < sp_search_num_tracks(search); ++i)
    {
        sp_track* track = sp_search_track(search, i);
        Track trackObj(spotifyGetTrack(track, spotifySession_));
        searchReply.push_back(trackObj);
    }

    //for (i = 0; i < sp_search_num_albums(search); ++i)
        //print_album(sp_search_album(search, i));

    //for (i = 0; i < sp_search_num_artists(search); ++i)
        //print_artist(sp_search_artist(search, i));

    reqData.first->genericSearchCallback( searchReply, didYouMean, reqData.second );
    sp_search_release(search);
    delete msg;
}

void LibSpotifyIf::imageLoadedCb(sp_image* image, void *userdata)
{
    QueryReqEventItem* msg = static_cast<QueryReqEventItem*>(userdata);
    sendGetImageRsp( image, msg );
    delete msg;
}

void LibSpotifyIf::albumLoadedCb(sp_albumbrowse* result, void *userdata)
{
    EventItem* ev = static_cast<EventItem*>(userdata);
    switch ( ev->event_ )
    {
        case EVENT_GET_ALBUM:
        {
            QueryReqEventItem* msg = static_cast<QueryReqEventItem*>(userdata);

            if ( sp_albumbrowse_error( result ) == SP_ERROR_OK )
            {
                Album album = spotifyGetAlbum(result, spotifySession_);

                log(LOG_NOTICE) << "Album \"" << album.getName() << "\" loaded";

                PendingMediaRequestData reqData = msg->reqData;
                reqData.first->getAlbumResponse( album, reqData.second );
            }
            break;
        }
        case EVENT_PLAY_REQ:
        {
            PlayReqEventItem* msg = static_cast<PlayReqEventItem*>(userdata);
            if ( sp_albumbrowse_error( result ) == SP_ERROR_OK )
            {
                Album album = spotifyGetAlbum(result, spotifySession_);

                log(LOG_NOTICE) << "Album \"" << album.getName() << "\" loaded";
                playbackHandler_.playAlbum(album, msg->startIndex_);
            }
            break;
        }
        case EVENT_GET_IMAGE:
        {
            QueryReqEventItem* msg = static_cast<QueryReqEventItem*>(userdata);
            if ( sp_albumbrowse_error( result ) == SP_ERROR_OK )
            {
                sp_album* album = sp_albumbrowse_album( result );
                const byte* imgRef = sp_album_cover( album, SP_IMAGE_SIZE_NORMAL );
                loadAndSendImage( imgRef, msg );
            }
            else
            {
                log(LOG_WARN) << "Image load error!";
                PendingMediaRequestData reqData = msg->reqData;
                reqData.first->getImageResponse( NULL, 0, reqData.second );
            }
            break;
        }
        default:
            assert(0);
    }

    delete ev;
    sp_albumbrowse_release(result);
}

void LibSpotifyIf::artistLoadedCb(sp_artistbrowse* result, void *userdata)
{
    EventItem* ev = static_cast<EventItem*>(userdata);
    switch ( ev->event_ )
    {
        case EVENT_GET_ARTIST:
        {
            QueryReqEventItem* msg = static_cast<QueryReqEventItem*>(userdata);

            if ( sp_artistbrowse_error( result ) == SP_ERROR_OK )
            {
                Artist artist = spotifyGetArtist(result, spotifySession_);

                log(LOG_NOTICE) << "Artist \"" << artist.getName() << "\" loaded";

                PendingMediaRequestData reqData = msg->reqData;
                reqData.first->getArtistResponse( artist, reqData.second );
            }
            break;
        }
        case EVENT_GET_IMAGE:
        {
            QueryReqEventItem* msg = static_cast<QueryReqEventItem*>(userdata);
            if ( sp_artistbrowse_error( result ) == SP_ERROR_OK && sp_artistbrowse_num_portraits( result ) )
            {
                sp_artist* artist = sp_artistbrowse_artist( result );
                const byte* imgRef = sp_artist_portrait( artist, SP_IMAGE_SIZE_NORMAL ); //sp_artistbrowse_portrait( result, 0 ); 
                loadAndSendImage( imgRef, msg );
            }
            else
            {
                PendingMediaRequestData reqData = msg->reqData;
                reqData.first->getImageResponse( NULL, 0, reqData.second );
            }
            break;
        }
        default:
            assert(0);
    }

    delete ev;
    sp_artistbrowse_release(result);
}
/* *****************
 * Audio endpoints
 * *****************/

void LibSpotifyIf::getCurrentAudioEndpoints( IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    subscriber->getCurrentAudioEndpointsResponse( audioOut_.getCurrentEndpoints(), userData );
}

/* *****************
 * local helpers
 * *****************/
const char* getEventName(LibSpotifyIf::EventItem* event)
{
    switch(event->event_)
    {
        case LibSpotifyIf::EVENT_ITERATE_MAIN_LOOP:
            return "EVENT_ITERATE_MAIN_LOOP";

            /* Metadata */
        case LibSpotifyIf::EVENT_METADATA_UPDATED:
            return "EVENT_METADATA_UPDATED";
        case LibSpotifyIf::EVENT_GENERIC_SEARCH:
            return "EVENT_GENERIC_SEARCH";
        case LibSpotifyIf::EVENT_GET_TRACKS:
            return "EVENT_GET_TRACKS";
        case LibSpotifyIf::EVENT_GET_IMAGE:
            return "EVENT_GET_IMAGE";
        case LibSpotifyIf::EVENT_GET_ALBUM:
            return "EVENT_GET_ALBUM";
        case LibSpotifyIf::EVENT_GET_ARTIST:
            return "EVENT_GET_ARTIST";

            /* Playback handling */
        case LibSpotifyIf::EVENT_PLAY_REQ:
            return "EVENT_PLAY_REQ";
        case LibSpotifyIf::EVENT_STOP_REQ:
            return "EVENT_STOP_REQ";
        case LibSpotifyIf::EVENT_ENQUEUE_REQ:
            return "EVENT_ENQUEUE_REQ";
        case LibSpotifyIf::EVENT_PAUSE_PLAYBACK:
            return "EVENT_PAUSE_PLAYBACK";
        case LibSpotifyIf::EVENT_RESUME_PLAYBACK:
            return "EVENT_RESUME_PLAYBACK";
        case LibSpotifyIf::EVENT_SEEK:
            return "EVENT_SEEK";
        case LibSpotifyIf::EVENT_PLAY_TRACK:
            return "EVENT_PLAY_TRACK";
        case LibSpotifyIf::EVENT_NEXT_TRACK:
            return "EVENT_NEXT_TRACK";
        case LibSpotifyIf::EVENT_PREVIOUS_TRACK:
            return "EVENT_PREVIOUS_TRACK";

             /* Session handling */
        case LibSpotifyIf::EVENT_LOGGING_IN:
            return "EVENT_LOGGING_IN";
        case LibSpotifyIf::EVENT_LOGGED_IN:
            return "EVENT_LOGGED_IN";
        case LibSpotifyIf::EVENT_LOGGING_OUT:
            return "EVENT_LOGGING_OUT";
        case LibSpotifyIf::EVENT_LOGGED_OUT:
            return "EVENT_LOGGED_OUT";
        case LibSpotifyIf::EVENT_CONNECTION_LOST:
            return "EVENT_CONNECTION_LOST";
    }
    return "Unknown event type!";
}

}





