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

#ifndef LIBSPOTIFYIF_H_
#define LIBSPOTIFYIF_H_

#include "LibSpotifyIfCallbackWrapper.h"
#include "LibSpotifyPlaybackHandler.h"

#include "ConfigHandling/ConfigHandler.h"
#include "MediaContainers/Folder.h"
#include "MediaContainers/Track.h"
#include "MessageFactory/Message.h"
#include "Platform/Threads/Runnable.h"
#include "Platform/Threads/Condition.h"
#include "MediaInterface/MediaInterface.h"
#include "MediaInterface/AudioProvider.h"
#include "Utils/ActionFilter.h"
#include <libspotify/api.h>
#include <set>
#include <vector>
#include <queue>

namespace LibSpotify
{

class LibSpotifyIf : Platform::Runnable, public MediaInterface, public AudioProvider
{
/**********************
 *
 * Type declarations
 *
 ***********************/
public:

    enum Event
    {
        /* Session handling */
        EVENT_LOGGING_IN = 0,
        EVENT_LOGGED_IN,
        EVENT_LOGGING_OUT,
        EVENT_LOGGED_OUT,
        EVENT_CONNECTION_LOST,

        /* Metadata */
        EVENT_METADATA_UPDATED,
        EVENT_GET_TRACKS,
        EVENT_GENERIC_SEARCH,
        EVENT_GET_IMAGE,
        EVENT_GET_ALBUM,
        EVENT_GET_ARTIST,

        EVENT_ADD_TO_PLAYLIST,
        EVENT_REMOVE_FROM_PLAYLIST,
        EVENT_MOVE_TRACKS,

        /* Playback Handling */
        EVENT_PLAY_REQ,
        EVENT_STOP_REQ,
        EVENT_ENQUEUE_REQ,
        EVENT_PLAY_TRACK,
        EVENT_PAUSE_PLAYBACK,
        EVENT_RESUME_PLAYBACK,
        EVENT_NEXT_TRACK,
        EVENT_PREVIOUS_TRACK,
        EVENT_SEEK,

        /* Others */
        EVENT_ITERATE_MAIN_LOOP
    };

    class EventItem
    {
    public:
        Event event_;
        EventItem(Event event) : event_(event) {}
    };

    class TrackEventItem : public EventItem
    {
    public:
        Track track_;
        sp_track* spTrack_;
        TrackEventItem(Event event, const Track& track) : EventItem(event), track_(track), spTrack_(NULL) {}
    };

    class ParamEventItem : public EventItem
    {
    public:
        uint32_t param_;
        ParamEventItem(Event event, uint32_t param) : EventItem(event), param_(param) {}
    };

    class ReqEventItem : public EventItem
    {
    public:
        PendingMediaRequestData reqData;
        ReqEventItem(Event event, IMediaInterfaceCallbackSubscriber* callbackSubscriber, void* subscriberData) : EventItem(event), reqData(callbackSubscriber, subscriberData) {}
    };

    class QueryReqEventItem : public ReqEventItem
    {
    public:
        std::string query_;
        QueryReqEventItem(Event event, IMediaInterfaceCallbackSubscriber* callbackSubscriber, void* subscriberData, const std::string& query) : ReqEventItem(event, callbackSubscriber, subscriberData), query_(query) {}
    };

    class ParamReqEventItem : public ReqEventItem, ParamEventItem
    {
    public:
        ParamReqEventItem(Event event, IMediaInterfaceCallbackSubscriber* callbackSubscriber, void* subscriberData, uint32_t param) : ReqEventItem(event, callbackSubscriber, subscriberData), ParamEventItem(event, param) {}
    };

    class PlayReqEventItem : public QueryReqEventItem
    {
    public:
        int startIndex_;
        PlayReqEventItem( IMediaInterfaceCallbackSubscriber* callbackSubscriber, void* subscriberData, const std::string& uri, int startIndex) : QueryReqEventItem(EVENT_PLAY_REQ, callbackSubscriber, subscriberData, uri), startIndex_(startIndex) {}
    };

    class AddToPlaylistEventItem : public QueryReqEventItem
    {
    public:
        std::list<std::string> tracks_;
        int index_;
        AddToPlaylistEventItem( IMediaInterfaceCallbackSubscriber* callbackSubscriber, void* subscriberData, const std::string& playlist, const std::list<std::string>& tracks, int index ) : QueryReqEventItem(EVENT_ADD_TO_PLAYLIST, callbackSubscriber, subscriberData, playlist), tracks_(tracks), index_(index) {}
    };

    class ModifyPlaylistEventItem : public QueryReqEventItem
    {
    public:
        std::set<int> tracks_;
        ModifyPlaylistEventItem( Event event, IMediaInterfaceCallbackSubscriber* callbackSubscriber, void* subscriberData, const std::string& playlist, const std::set<int>& tracks ) : QueryReqEventItem(event, callbackSubscriber, subscriberData, playlist), tracks_(tracks) {}
    };

    class MoveTracksEventItem : public ModifyPlaylistEventItem
    {
    public:
        int toIndex_;
        MoveTracksEventItem( IMediaInterfaceCallbackSubscriber* callbackSubscriber, void* subscriberData, const std::string& playlist, const std::set<int>& tracks, int toIndex ) : ModifyPlaylistEventItem(EVENT_MOVE_TRACKS, callbackSubscriber, subscriberData, playlist, tracks), toIndex_(toIndex) {}
    };

    enum LibSpotifyTrackStates
    {
        TRACK_STATE_NOT_LOADED = 0,
        TRACK_STATE_PLAYING,
        TRACK_STATE_PAUSED
    };

private:

	enum LibSpotifyStates
	{
		STATE_INVALID = 0,
		STATE_LOGGING_IN,
		STATE_LOGGED_IN,
		STATE_LOGGING_OUT,
		STATE_LOGGED_OUT
	};


/**********************
 *
 * member declarations
 *
 ***********************/
private:
    const ConfigHandling::SpotifyConfig& config_;

	Folder rootFolder_;

	/*********************
	 * Synchronization
	 * with libspotify
	 ********************/
	Platform::Condition cond_;
	LibSpotifyStates state_;
	int nextTimeoutForLibSpotify;

	/****************
	 * Event queue
	 ****************/
	Platform::Mutex eventQueueMtx_;
	typedef std::queue<EventItem*> EventQueueMessageQueue;
	EventQueueMessageQueue eventQueue_;

	/************************
	 * Statemachine handling
	 ************************/
	void stateMachineEventHandler(EventItem* event);
	void postToEventThread(EventItem* event);

	/************************
     * Playback handling
     ************************/
	friend class LibSpotifyPlaybackHandler;
	LibSpotifyPlaybackHandler playbackHandler_;
	void playTrack(const Track& track);

    ActionFilter seekFilter;
    void doSeek( uint32_t sec );
    friend void seekCb( void* arg, uint32_t sec );

    ActionFilter volumeFilter;
    void doVolume( uint32_t volume );
    friend void volumeCb( void* arg, uint32_t volume );

	/**************************
	 * libspotify interactions
	 **************************/
	void libSpotifySessionCreate();
    void refreshRootFolder();

   	/************************
     * Metadata handling
     ************************/
    std::queue<EventItem*> pendingMetadata;

	//callback wrapper
	friend class LibSpotifyIfCallbackWrapper;
	LibSpotifyIfCallbackWrapper itsCallbackWrapper_;
	sp_session* spotifySession_;
	LibSpotifyTrackStates trackState_;
	Track currentTrack_;
	unsigned int progress_;

	/* Session callbacks */
	void loggedInCb(sp_session *session, sp_error error);
	void loggedOutCb(sp_session *session);
	void connectionErrorCb(sp_session *session, sp_error error);
	void notifyLibSpotifyMainThreadCb(sp_session *session);
	/* playback callbacks */
	void metadataUpdatedCb(sp_session *session);
	int musicDeliveryCb(sp_session *sess, const sp_audioformat *format, const void *frames, int num_frames);
	void endOfTrackCb(sp_session *session);
	/* search callbacks*/
	void genericSearchCb(sp_search *search, void *userdata);
    void imageLoadedCb(sp_image* image, void *userdata);
    void albumLoadedCb(sp_albumbrowse* result, void *userdata);
    void artistLoadedCb(sp_artistbrowse* result, void *userdata);
	/* Util callbacks */
	void logMessageCb(sp_session *session, const char *data);

    /* Playlist callbacks */
    void rootFolderLoaded();
    void playlistAdded( sp_playlist* pl );
    void playlistRemoved( sp_playlist* pl );
    void playlistRenamed( sp_playlist* pl );
    void playlistsMoved();
    void playlistContentsUpdated( sp_playlist* pl );

    void doStatusNtf();
    void sendGetImageRsp( sp_image* img, QueryReqEventItem* reqEvent );
    void loadAndSendImage( const byte* imgRef, QueryReqEventItem* reqEvent );
public:
	LibSpotifyIf(const ConfigHandling::SpotifyConfig& config );
	virtual ~LibSpotifyIf();

	void logIn();
	void logOut();

    /* Implements MediaInterface */
    virtual void getImage( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void previous();
    virtual void next();
    virtual void resume();
    virtual void pause();
    virtual void seek( uint32_t sec );
    virtual void setShuffle( bool shuffleOn );
    virtual void setRepeat( bool repeatOn );
    virtual void setVolume( uint8_t volume );
    virtual void getStatus( IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void getPlaylists( IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void getTracks( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void play( const std::string& link, int startIndex, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void play( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void enqueue( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void getAlbum( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void getArtist( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void search( const std::string& query, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void playlistAddTracks( const std::string& playlistlink, const std::list<std::string>& tracklinks, int index, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void playlistRemoveTracks( const std::string& playlistlink, const std::set<int>& indexes, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void playlistMoveTracks( const std::string& playlistlink, const std::set<int>& indexes, int toIndex, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );

    virtual void getCurrentAudioEndpoints( IMediaInterfaceCallbackSubscriber* subscriber, void* userData );

    void playSearchResult(const char* searchString);
    void stop();


    void run();
    void destroy();

};
}
#endif /* LIBSPOTIFYIF_H_ */
