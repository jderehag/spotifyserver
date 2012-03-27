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
#include "ILibSpotifyIfCallbackSubscriber.h"
#include "LibSpotifyPlaybackHandler.h"

#include "ConfigHandling/ConfigHandler.h"
#include "MediaContainers/Folder.h"
#include "MediaContainers/Track.h"
#include "Platform/AudioEndpoints/AudioEndpoint.h"
#include "MessageFactory/Message.h"
#include "Platform/Threads/Runnable.h"
#include "Platform/Threads/Condition.h"

#include <libspotify/api.h>
#include <set>
#include <vector>
#include <queue>

namespace LibSpotify
{

class LibSpotifyIf : Platform::Runnable
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

		/* Playback Handling */
		EVENT_PLAY_REQ,
		EVENT_STOP_REQ,
		EVENT_ENQUEUE_TRACK_REQ,
		EVENT_PLAY_TRACK,
        EVENT_PAUSE_PLAYBACK,
        EVENT_RESUME_PLAYBACK,

        /* Others */
		EVENT_ITERATE_MAIN_LOOP
	};

    enum LibSpotifyTrackStates
    {
        TRACK_STATE_NOT_LOADED = 0,
        TRACK_STATE_WAITING_METADATA,
        TRACK_STATE_PLAYING,
        TRACK_STATE_PAUSED
    };

private:
	typedef struct
	{
		Event event;
		void* data;
	}EventQueueMessage;

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
	Platform::AudioEndpoint& endpoint_;
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
	typedef std::queue<EventQueueMessage> EventQueueMessageQueue;
	EventQueueMessageQueue eventQueue_;

	/***********************
	 * Callback subscription
	 ***********************/
	Platform::Mutex callbackSubscriberMtx_;
	typedef std::set<ILibSpotifyIfCallbackSubscriber*> LibSpotifyCallbackSubscriberSet;
	LibSpotifyCallbackSubscriberSet callbackSubscriberList_;

	/************************
	 * Statemachine handling
	 ************************/
	void stateMachineEventHandler(Event event, void* msg);
	void postToEventThread(Event event, void* msg);

	/************************
     * Playback handling
     ************************/
	friend class LibSpotifyPlaybackHandler;
	LibSpotifyPlaybackHandler playbackHandler_;
	void playTrack(const Track& track);

	/**************************
	 * libspotify interactions
	 **************************/
	void libSpotifySessionCreate();
	void updateRootFolder(sp_playlistcontainer* plContainer);

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
	/* Util callbacks */
	void logMessageCb(sp_session *session, const char *data);

public:
	LibSpotifyIf(const ConfigHandling::SpotifyConfig& config, Platform::AudioEndpoint& endpoint);
	virtual ~LibSpotifyIf();

	void logIn();
	void logOut();

	void getTracks(unsigned int reqId, std::string link, ILibSpotifyIfCallbackSubscriber& callbackSubscriber);
	void genericSearch(unsigned int reqId, std::string query, ILibSpotifyIfCallbackSubscriber& callbackSubscriber);
    void getImage(unsigned int reqId, std::string link, ILibSpotifyIfCallbackSubscriber& callbackSubscriber);
	LibSpotifyTrackStates getState() { return trackState_; }
	unsigned int getProgress() { return progress_/10; }
	Track& getCurrentTrack() { return currentTrack_; }

	void play(const std::string uri);
	void stop();
    void playSearchResult(const char* searchString);
    void enqueueTrack(const char* track_uri);
	void pause();
	void resume();
    void next();
    void previous();

	Folder& getRootFolder();

	void run();
	void destroy();

	void registerForCallbacks(ILibSpotifyIfCallbackSubscriber& subscriber);
	void unRegisterForCallbacks(ILibSpotifyIfCallbackSubscriber& subscriber);

};
}
#endif /* LIBSPOTIFYIF_H_ */
