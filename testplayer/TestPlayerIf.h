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
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TESTPLAYERIF_H_
#define TESTPLAYERIF_H_

#include "AudioEndpointManager/AudioEndpointManager.h"
#include "Platform/AudioEndpoints/AudioDispatch.h"
#include "MessageFactory/Message.h"
#include "Platform/Threads/Runnable.h"
#include "Platform/Threads/Condition.h"
#include "MediaInterface/MediaInterface.h"
#include <set>
#include <vector>
#include <queue>



class TestPlayerIf : Platform::Runnable, public MediaInterface
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

		/* Playback Handling */
		EVENT_PLAY_REQ,
		EVENT_STOP_REQ,
		EVENT_ENQUEUE_TRACK_REQ,
		EVENT_PLAY_TRACK,
        EVENT_TRACK_ENDED,
        EVENT_PAUSE_PLAYBACK,
        EVENT_RESUME_PLAYBACK,

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
        TrackEventItem(Event event, const Track& track) : EventItem(event), track_(track) {}
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
        QueryReqEventItem(Event event, IMediaInterfaceCallbackSubscriber* callbackSubscriber, void* subscriberData, std::string query) : ReqEventItem(event, callbackSubscriber, subscriberData), query_(query) {}
    };

    class PlayReqEventItem : public QueryReqEventItem
    {
    public:
        int startIndex_;
        PlayReqEventItem( IMediaInterfaceCallbackSubscriber* callbackSubscriber, void* subscriberData, std::string uri, int startIndex) : QueryReqEventItem(EVENT_PLAY_REQ, callbackSubscriber, subscriberData, uri), startIndex_(startIndex) {}
    };


private:


/**********************
 *
 * member declarations
 *
 ***********************/
private:

    AudioEndpointManager& audioMgr_;
    Platform::AudioDispatch audioOut_;

private:

	/****************
	 * Event queue
	 ****************/
    Platform::Condition cond_;
	Platform::Mutex eventQueueMtx_;
	typedef std::queue<EventItem*> EventQueueMessageQueue;
	EventQueueMessageQueue eventQueue_;

	/************************
	 * Statemachine handling
	 ************************/
	void stateMachineEventHandler(EventItem* event);
	void postToEventThread(EventItem* event);

	int16_t* playBuffer;
	size_t playBufferSize;
	size_t playBufferPos;

public:
	TestPlayerIf(AudioEndpointManager& audioMgr );
	virtual ~TestPlayerIf();

	void logIn();
	void logOut();

    /* Implements MediaInterface */
    virtual void getImage( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void previous();
    virtual void next();
    virtual void resume();
    virtual void pause();
    virtual void setShuffle( bool shuffleOn );
    virtual void setRepeat( bool repeatOn );
    virtual void getStatus( IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void getPlaylists( IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void getTracks( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void play( std::string link, int startIndex, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void play( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void getAlbum( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void search( std::string query, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );

    virtual void addAudioEndpoint( const std::string& id, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void removeAudioEndpoint( const std::string& id, IMediaInterfaceCallbackSubscriber* subscriber, void* userData );
    virtual void getCurrentAudioEndpoints( IMediaInterfaceCallbackSubscriber* subscriber, void* userData );


    void run();
    void destroy();

};

#endif /* TESTPLAYERIF_H_ */
