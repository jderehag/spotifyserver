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

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * public methods
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
LibSpotifyIf::LibSpotifyIf(const ConfigHandling::SpotifyConfig& config, Platform::AudioEndpoint& endpoint) :
                                                                         config_(config),
																		 defaultEndpoint_(endpoint),
																		 currentEndpoint_(&defaultEndpoint_),
																		 rootFolder_("root", 0, 0),
																		 state_(STATE_INVALID),
																		 nextTimeoutForLibSpotify(0),
																		 playbackHandler_(*this),
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

void LibSpotifyIf::getPlaylists( IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId reqId )
{
    subscriber->getPlaylistsResponse( reqId, rootFolder_ );
}

void LibSpotifyIf::getTracks( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId reqId )
{
    postToEventThread( new QueryReqEventItem( EVENT_GET_TRACKS, reqId, subscriber, link ) );
}

void LibSpotifyIf::search( std::string query, IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId reqId )
{
    postToEventThread( new QueryReqEventItem( EVENT_GENERIC_SEARCH, reqId, subscriber, query ) );
}

void LibSpotifyIf::getImage( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId reqId )
{
    postToEventThread( new QueryReqEventItem( EVENT_GET_IMAGE, reqId, subscriber, link ) );
}

void LibSpotifyIf::getAlbum( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId reqId )
{
    postToEventThread( new QueryReqEventItem( EVENT_GET_ALBUM, reqId, subscriber, link ) );
}

void LibSpotifyIf::getStatus( IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId reqId )
{
    switch ( trackState_ )
    {
        case TRACK_STATE_NOT_LOADED:
            subscriber->getStatusResponse( reqId,
                                           PLAYBACK_IDLE,
                                           playbackHandler_.getRepeat(),
                                           playbackHandler_.getShuffle() );
            break;
        case TRACK_STATE_PAUSED:
            subscriber->getStatusResponse( reqId,
                                           PLAYBACK_PAUSED,
                                           playbackHandler_.getRepeat(),
                                           playbackHandler_.getShuffle(),
                                           currentTrack_,
                                           progress_/10 );
            break;
        case TRACK_STATE_PLAYING:
            subscriber->getStatusResponse( reqId,
                                           PLAYBACK_PLAYING,
                                           playbackHandler_.getRepeat(),
                                           playbackHandler_.getShuffle(),
                                           currentTrack_,
                                           progress_/10 );
            break;
    }
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
void LibSpotifyIf::play( std::string link, int startIndex, IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId reqId )
{
    postToEventThread( new PlayReqEventItem(reqId, subscriber, link, startIndex) );
}
void LibSpotifyIf::play( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, MediaInterfaceRequestId reqId )
{
    play( link, -1, subscriber, reqId );
}

void LibSpotifyIf::stop()
{
    postToEventThread( new EventItem( EVENT_STOP_REQ ) );
}

void LibSpotifyIf::enqueueTrack(const char* track_uri)
{
    //todo: we need a callback subscriber here, right?
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
    stop();

    if (trackState_ == TRACK_STATE_PLAYING || trackState_ == TRACK_STATE_PAUSED)
    {
        playbackHandler_.trackEndedInd(); /*playNext() only picks from enqueued tracks? and this is the same thing as if track ended...*/
    }
}

void LibSpotifyIf::previous()
{
    stop();

    if (trackState_ == TRACK_STATE_PLAYING || trackState_ == TRACK_STATE_PAUSED)
    {
        playbackHandler_.playPrevious();
    }
}

void LibSpotifyIf::setShuffle( bool shuffleOn ) { playbackHandler_.setShuffle( shuffleOn ); }
void LibSpotifyIf::setRepeat( bool repeatOn )   { playbackHandler_.setRepeat( repeatOn ); }

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
			updateRootFolder(sp_session_playlistcontainer(spotifySession_));

            while(!pendingMetadata.empty())
            {
                EventItem* item = pendingMetadata.front();
                postToEventThread(item);
                pendingMetadata.pop();
            }

			break;

		case EVENT_GET_TRACKS:
		{
		    QueryReqEventItem* reqEvent = static_cast<QueryReqEventItem*>( event );

			const char* playlist_uri = reqEvent->query_.c_str();
		    if (state_ == STATE_LOGGED_IN)
			{
				sp_link* link = sp_link_create_from_string(playlist_uri);
				if(link)
				{
					if(sp_link_type(link) == SP_LINKTYPE_PLAYLIST)
					{
						sp_playlist* playlist = sp_playlist_create(spotifySession_, link);

						if (sp_playlist_is_loaded(playlist))
						{
							log(LOG_DEBUG) << "Got playlist " << playlist_uri;
	                         Playlist playlistObj = spotifyGetPlaylist(playlist, spotifySession_);


	                         reqEvent->callbackSubscriber_->getTracksResponse(reqEvent->reqId_, playlistObj.getTracks());
						}
						else
						{
							/*not sure if we're waiting for metadata or playlist_state_changed here*/
                            pendingMetadata.push(new QueryReqEventItem(*reqEvent) );
							log(LOG_DEBUG) << "Waiting for metadata for playlist " << playlist_uri;
						}
					}
					else log(LOG_WARN) << "Link is not a playlist: " << playlist_uri;

					sp_link_release(link);
				}
				else log(LOG_WARN) << "Bad link: " << playlist_uri;

			}
			break;
		}

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

		case EVENT_GENERIC_SEARCH:
		{
		    QueryReqEventItem* reqEvent = static_cast<QueryReqEventItem*>( event );
			/* search, but only interested in the first 100 results,
			 * 0, = track_offset
			 * 100, = track_count
			 * 0, = album_offset
			 * 100, = album_count
			 * 0, = artist_offset
			 * 100 = artist_count */
			sp_search_create(spotifySession_,
			                reqEvent->query_.c_str(),
							0,
							100,
							0,
							100,
							0,
							100,
							&LibSpotifyIfCallbackWrapper::genericSearchCallback,
							new QueryReqEventItem(*reqEvent));

			break;
		}

		case EVENT_GET_IMAGE:
		{
            QueryReqEventItem* reqEvent = static_cast<QueryReqEventItem*>( event );

            std::string linkStr = reqEvent->query_.empty() ? currentTrack_.getAlbumLink() : reqEvent->query_;

            if (!linkStr.empty())
            {
                sp_link* link = sp_link_create_from_string(linkStr.c_str());
                if (link)
                {
                    const byte* imgRef = NULL;
                    if(sp_link_type(link) == SP_LINKTYPE_ALBUM)
                    {
                        sp_album* album = sp_link_as_album(link);

                        if ( sp_album_is_loaded(album) )
                        {
                            imgRef = sp_album_cover(album);
                        }
                        else
                        {
                            log(LOG_WARN) << "No metadata for album";
                            sp_albumbrowse_create( spotifySession_, album, &LibSpotifyIfCallbackWrapper::albumLoadedCallback, new QueryReqEventItem( *reqEvent ));
                        }
                    }
                    else
                    {
                        /*not supported yet*/
                        reqEvent->callbackSubscriber_->getImageResponse(reqEvent->reqId_, NULL, 0);
                    }

                    if (imgRef)
                    {
                        sp_image* img = sp_image_create(spotifySession_, imgRef);
                        if ( img )
                        {
                            sp_error error;
                            error = sp_image_error(img);

                            if ( sp_image_is_loaded(img) )
                            {
                                size_t dataSize;
                                const void* data = sp_image_data(img, &dataSize);
                                reqEvent->callbackSubscriber_->getImageResponse(reqEvent->reqId_, data, dataSize);
                                sp_image_release(img);
                            }
                            else if ( error == SP_ERROR_IS_LOADING )
                            {
                                log(LOG_DEBUG) << "waiting for image load";
                                sp_image_add_load_callback(img, &LibSpotifyIfCallbackWrapper::imageLoadedCallback, new QueryReqEventItem(*reqEvent));
                            }
                            else
                            {
                                log(LOG_WARN) << "Image load error: " << sp_error_message(error);
                                reqEvent->callbackSubscriber_->getImageResponse(reqEvent->reqId_, NULL, 0);
                                sp_image_release(img);
                            }
                        }
                    }
                    sp_link_release(link);
                }
                else
                {
                    log(LOG_WARN) << "Bad link?" << linkStr;
                    reqEvent->callbackSubscriber_->getImageResponse(reqEvent->reqId_, NULL, 0);
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
                            {
                                sp_track* track = sp_link_as_track(link);
                                Track trackObj(spotifyGetTrack(track, spotifySession_));
                                playbackHandler_.playTrack(trackObj);
                            }
                            break;


				        case SP_LINKTYPE_PLAYLIST:
                            {
                                sp_playlist* playlist = sp_playlist_create(spotifySession_, link);
                                Playlist playlistObj(spotifyGetPlaylist(playlist, spotifySession_));
                                playbackHandler_.playPlaylist(playlistObj, reqEvent->startIndex_);
                                log(LOG_NOTICE) << "Adding " << reqEvent->query_ << " to playbackhandler";
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
				            assert(false);
				            break;
				    }
				}
			}

			break;
		}

        case EVENT_STOP_REQ:
            if (trackState_ != TRACK_STATE_NOT_LOADED)
            {
                callbackSubscriberMtx_.lock();
                for( std::set<IMediaInterfaceCallbackSubscriber*>::iterator it = callbackSubscriberList_.begin();
                        it != callbackSubscriberList_.end(); it++)
                {
                    (*it)->statusUpdateInd( PLAYBACK_IDLE,
                                            playbackHandler_.getRepeat(),
                                            playbackHandler_.getShuffle() );
                }
                callbackSubscriberMtx_.unlock();

                sp_session_player_play(spotifySession_, 0);
                currentEndpoint_->flushAudioData();
                trackState_ = TRACK_STATE_NOT_LOADED;
                progress_ = 0;
            }
            break;

		case EVENT_PAUSE_PLAYBACK:
		    if (trackState_ == TRACK_STATE_PLAYING)
		    {
		        sp_session_player_play(spotifySession_, 0);
		        currentEndpoint_->pause();
		        trackState_ = TRACK_STATE_PAUSED;

		        callbackSubscriberMtx_.lock();
		        for(std::set<IMediaInterfaceCallbackSubscriber*>::iterator it = callbackSubscriberList_.begin();
		                it != callbackSubscriberList_.end(); it++)
		        {
		            (*it)->statusUpdateInd( PLAYBACK_PAUSED,
		                                    playbackHandler_.getRepeat(),
		                                    playbackHandler_.getShuffle(),
		                                    currentTrack_,
		                                    progress_/10 );
		        }
		        callbackSubscriberMtx_.unlock();
		    }
			break;

		case EVENT_RESUME_PLAYBACK:
            if (trackState_ == TRACK_STATE_PAUSED)
            {
                sp_session_player_play(spotifySession_, 1);
                currentEndpoint_->resume();
                trackState_ = TRACK_STATE_PLAYING;

                callbackSubscriberMtx_.lock();
                for(std::set<IMediaInterfaceCallbackSubscriber*>::iterator it = callbackSubscriberList_.begin();
                        it != callbackSubscriberList_.end(); it++)
                {
                    (*it)->statusUpdateInd( PLAYBACK_PLAYING,
                                            playbackHandler_.getRepeat(),
                                            playbackHandler_.getShuffle(),
                                            currentTrack_,
                                            progress_/10 );
                }
                callbackSubscriberMtx_.unlock();
            }
			break;

		case EVENT_PLAY_TRACK:
		    {

		        TrackEventItem* trackEvent = static_cast<TrackEventItem*>(event);
		        Track* trackObj = &trackEvent->track_;
		        sp_link* link = sp_link_create_from_string(trackObj->getLink().c_str());
                if(link)
                {
                    assert( sp_link_type(link) == SP_LINKTYPE_TRACK || sp_link_type(link) == SP_LINKTYPE_LOCALTRACK );

                    sp_error err;
                    sp_track* track = sp_link_as_track(link);
                    if ((err = sp_track_error(track)) == SP_ERROR_OK)
                    {
                        if ((err = sp_session_player_load(spotifySession_, track)) == SP_ERROR_OK)
                        {
                            currentTrack_ = spotifyGetTrack(track, spotifySession_); //*trackObj; is not updated if we had to wait for metadata
                            currentTrack_.setIndex(trackObj->getIndex()); /*kind of a hack.. but only trackObj know where it came from, and thus which index it has (if it has one) */
                            trackState_ = TRACK_STATE_PLAYING;
                            sp_session_player_play(spotifySession_, 1);
                            currentEndpoint_->resume();
                            progress_ = 0;

                            callbackSubscriberMtx_.lock();
                            /* Tell all subscribers that the track is playing */
                            for(std::set<IMediaInterfaceCallbackSubscriber*>::iterator it = callbackSubscriberList_.begin();
                                it != callbackSubscriberList_.end(); it++)
                            {
                                (*it)->statusUpdateInd( PLAYBACK_PLAYING,
                                                        playbackHandler_.getRepeat(),
                                                        playbackHandler_.getShuffle(),
                                                        currentTrack_,
                                                        progress_/10 );
                            }
                            callbackSubscriberMtx_.unlock();
                        }
                        else
                        {
                            log(LOG_WARN) << "Player load error for " << trackObj->getLink().c_str() << " (" << sp_error_message(err) << ")";
                            playbackHandler_.trackEndedInd(); /*todo: some proper handling here, this will put track on the history list */
                        }
                    }
                    else if (err == SP_ERROR_IS_LOADING)
                    {
                        pendingMetadata.push(new TrackEventItem( *trackEvent ));
                        log(LOG_DEBUG) << "Waiting for metadata for track " << trackObj->getLink().c_str();
                    }
                    else
                    {
                        log(LOG_WARN) << "Track error for " << trackObj->getLink().c_str() << " (" << sp_error_message(err) << ")";
                        /* return some sort of error to client? */
                    }
                }
                sp_link_release(link);
	        }
		    break;

		/* Session Handling*/

		case EVENT_LOGGING_IN:
            state_ = STATE_LOGGING_IN;
            log(LOG_NOTICE) << "Logging in as " << config_.getUsername();
            sp_session_login(spotifySession_, config_.getUsername().c_str(), config_.getPassword().c_str(), 0);
            break;

        case EVENT_LOGGED_IN:
            state_ = STATE_LOGGED_IN;
            updateRootFolder(sp_session_playlistcontainer(spotifySession_));
            break;

		case EVENT_CONNECTION_LOST:
			if(state_ == STATE_LOGGED_IN ||
			   state_ == STATE_LOGGING_IN)
			{
				state_ = STATE_LOGGING_IN;
				sp_session_relogin(spotifySession_);
			}
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

void LibSpotifyIf::updateRootFolder(sp_playlistcontainer* plContainer)
{
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
	/* Should perhaps use its own event?
	 * metadataCb is (if I have understood it correctly)
	 * only used for when additional track & playlist info has loaded into
	 * memory (inside libspotify)
	 *
	if (sp_track_error(track_) == SP_ERROR_OK)
	{
		postToEventThread(EVENT_START_REQUESTED_TRACK, NULL);
	}
	*/
}

void LibSpotifyIf::endOfTrackCb(sp_session *session)
{
    log(LOG_DEBUG) << "End of track";
    log(LOG_WARN) << progress_;
    trackState_ = TRACK_STATE_NOT_LOADED; /*todo, this should happen when buffer is finished*/

	/* Tell all subscribers that the track has ended */
	/* JESPER: perhaps this should be done from the main thread?*/
	callbackSubscriberMtx_.lock();
	for(std::set<IMediaInterfaceCallbackSubscriber*>::iterator it = callbackSubscriberList_.begin();
		it != callbackSubscriberList_.end(); it++)
	{
        (*it)->statusUpdateInd( PLAYBACK_IDLE,
                                playbackHandler_.getRepeat(),
                                playbackHandler_.getShuffle() );
	}
	callbackSubscriberMtx_.unlock();

	/* special handling for the playbackhandler */
	playbackHandler_.trackEndedInd();
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
    int n = currentEndpoint_->enqueueAudioData(format->channels, format->sample_rate, num_frames, static_cast<const int16_t*>(frames));
    progress_ += (n*10000)/format->sample_rate;
    return n;
}

void LibSpotifyIf::setAudioEndpoint(Platform::AudioEndpoint* endpoint)
{
    currentEndpoint_ = endpoint;
}
void LibSpotifyIf::addAudio()
{
}


void LibSpotifyIf::genericSearchCb(sp_search *search, void *userdata)
{
	QueryReqEventItem* msg = static_cast<QueryReqEventItem*>(userdata);

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


	msg->callbackSubscriber_->genericSearchCallback(msg->reqId_, searchReply, didYouMean);
	sp_search_release(search);
	delete msg;
}

void LibSpotifyIf::imageLoadedCb(sp_image* image, void *userdata)
{
    QueryReqEventItem* msg = static_cast<QueryReqEventItem*>(userdata);
    postToEventThread( msg );
}

void LibSpotifyIf::albumLoadedCb(sp_albumbrowse* result, void *userdata)
{
    EventItem* ev = static_cast<EventItem*>(userdata);
    Album album = spotifyGetAlbum(result, spotifySession_);

    log(LOG_NOTICE) << "Album \"" << album.getName() << "\" loaded";

    switch ( ev->event_ )
    {
        case EVENT_GET_ALBUM:
        {
            QueryReqEventItem* msg = static_cast<QueryReqEventItem*>(userdata);
            msg->callbackSubscriber_->getAlbumResponse(msg->reqId_, album);
            delete msg;
            break;
        }
        case EVENT_PLAY_REQ:
        {
            PlayReqEventItem* msg = static_cast<PlayReqEventItem*>(userdata);
            playbackHandler_.playAlbum(album, msg->startIndex_);
            delete msg;
            break;
        }
        case EVENT_GET_IMAGE:
        {
            QueryReqEventItem* msg = static_cast<QueryReqEventItem*>(userdata);
            postToEventThread( msg );
            break;
        }
        default:
            assert(1);
    }

    sp_albumbrowse_release(result);
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

			/* Playback handling */
		case LibSpotifyIf::EVENT_PLAY_REQ:
		    return "EVENT_PLAY_REQ";
        case LibSpotifyIf::EVENT_STOP_REQ:
            return "EVENT_STOP_REQ";
		case LibSpotifyIf::EVENT_ENQUEUE_TRACK_REQ:
		    return "EVENT_ENQUEUE_TRACK_REQ";
		case LibSpotifyIf::EVENT_PAUSE_PLAYBACK:
		    return "EVENT_PAUSE_PLAYBACK";
		case LibSpotifyIf::EVENT_RESUME_PLAYBACK:
		    return "EVENT_RESUME_PLAYBACK";
		case LibSpotifyIf::EVENT_PLAY_TRACK:
		    return "EVENT_PLAY_TRACK";

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





