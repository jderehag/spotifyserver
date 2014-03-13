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

#include "TestPlayerIf.h"
#include "applog.h"

#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>



static const char* getEventName(TestPlayerIf::EventItem* event);

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * public methods
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
TestPlayerIf::TestPlayerIf( AudioEndpointManager& audioMgr ) : audioMgr_(audioMgr), playBuffer(NULL), playBufferSize(0), playBufferPos(0)
{
    Platform::AudioEndpoint* ep = audioMgr_.getEndpoint( "local" );
    audioOut_.addEndpoint( *ep );
    startThread();
}

TestPlayerIf::~TestPlayerIf()
{

}
void TestPlayerIf::destroy()
{
	cancelThread();
	joinThread();
}

void TestPlayerIf::getPlaylists( IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    //subscriber->getPlaylistsResponse( rootFolder_, userData );
}

void TestPlayerIf::getTracks( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new QueryReqEventItem( EVENT_GET_TRACKS, subscriber, userData, link ) );
}

void TestPlayerIf::search( std::string query, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new QueryReqEventItem( EVENT_GENERIC_SEARCH, subscriber, userData, query ) );
}

void TestPlayerIf::getImage( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new QueryReqEventItem( EVENT_GET_IMAGE, subscriber, userData, link ) );
}

void TestPlayerIf::getAlbum( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new QueryReqEventItem( EVENT_GET_ALBUM, subscriber, userData, link ) );
}

void TestPlayerIf::getStatus( IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PlaybackHandler requests
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
void TestPlayerIf::play( std::string link, int startIndex, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    postToEventThread( new PlayReqEventItem(subscriber, userData, link, startIndex) );
}
void TestPlayerIf::play( std::string link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    play( link, -1, subscriber, userData );
}


void TestPlayerIf::pause()
{
    postToEventThread( new EventItem( EVENT_PAUSE_PLAYBACK ) );
}

void TestPlayerIf::resume()
{
    postToEventThread( new EventItem( EVENT_RESUME_PLAYBACK ) );
}

void TestPlayerIf::next()
{

}

void TestPlayerIf::previous()
{

}

void TestPlayerIf::setShuffle( bool shuffleOn ) {  }
void TestPlayerIf::setRepeat( bool repeatOn )   {  }

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * private methods
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define MIN(a,b) ( a>b ? b : a )
void TestPlayerIf::run()
{
    while(isCancellationPending() == false)
    {
        eventQueueMtx_.lock();
        {
            cond_.timedWait(eventQueueMtx_, 20);
        }

        if ( playBuffer != NULL )
        {
            unsigned int nsamples = playBufferSize/8;//MIN(2205, playBufferSize - playBufferPos);
            playBufferPos += audioOut_.enqueueAudioData( 2, 44100, nsamples, &playBuffer[playBufferPos*2]);
            if ( playBufferPos >= playBufferSize )
                playBufferPos = 0;
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
    log(LOG_DEBUG) << "Exiting TestPlayerIf::run()";
}

void TestPlayerIf::stateMachineEventHandler(EventItem* event)
{
	if(event->event_ != EVENT_ITERATE_MAIN_LOOP)
	    log(LOG_DEBUG) << "Event received:" << getEventName(event);
	switch(event->event_)
	{
		case EVENT_METADATA_UPDATED:

			break;

		case EVENT_GET_TRACKS:
		{
			break;
		}

        case EVENT_GET_ALBUM:
        {
            QueryReqEventItem* reqEvent = static_cast<QueryReqEventItem*>( event );
            break;
        }

        case EVENT_GENERIC_SEARCH:
        {
            QueryReqEventItem* reqEvent = static_cast<QueryReqEventItem*>( event );

            break;
        }

        case EVENT_GET_IMAGE:
        {
            QueryReqEventItem* reqEvent = static_cast<QueryReqEventItem*>( event );
		}
		break;

		case EVENT_PLAY_REQ:
		{
		    PlayReqEventItem* reqEvent = static_cast<PlayReqEventItem*>( event );
            unsigned int nsamples = atoi(reqEvent->query_.c_str()) *8;
		    unsigned int width = 10;
		    unsigned int amplitude = 0x7fff;

		    if ( playBuffer )
		    {
		        delete playBuffer;
		        playBuffer = NULL;
		    }

		    playBuffer = new int16_t[nsamples*2];
		    playBufferSize = nsamples;
		    playBufferPos = 0;

		    memset( playBuffer, 0, sizeof(playBuffer) );
		    int i;
		    for( i=0; i<width/2; i++ )
		    {
		        playBuffer[i*2] = i * amplitude / (width/2);
		        playBuffer[i*2 + 1] = playBuffer[i*2];
		    }
            for( ; i<width; i++ )
            {
                playBuffer[i*2] = (width - i) * amplitude / (width - i);
                playBuffer[i*2 + 1] = playBuffer[i*2];
            }

            for( i=0; i<width/2; i++ )
            {
                playBuffer[width+i*2] = -playBuffer[i*2];
                playBuffer[width+i*2 + 1] = playBuffer[i*2];
            }
            for( ; i<width; i++ )
            {
                playBuffer[width+i*2] = -playBuffer[i*2];
                playBuffer[width+i*2 + 1] = playBuffer[i*2];
            }

			break;
		}

        case EVENT_STOP_REQ:

            break;

        case EVENT_PAUSE_PLAYBACK:
                audioOut_.pause();

            break;

        case EVENT_RESUME_PLAYBACK:
                audioOut_.resume();
            break;

        case EVENT_TRACK_ENDED:
            break;

		case EVENT_PLAY_TRACK:

		    break;


		default:
			break;
	}
}
void TestPlayerIf::postToEventThread(EventItem* event)
{
	eventQueueMtx_.lock();
	eventQueue_.push(event);
	eventQueueMtx_.unlock();
	cond_.signal();
}



void TestPlayerIf::addAudioEndpoint( const std::string& id, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    Platform::AudioEndpoint* ep;
    if ( id == "" )
        ep = audioMgr_.getEndpoint( "local" );
    else
        ep = audioMgr_.getEndpoint( id );

    if ( ep )
    {
        audioOut_.addEndpoint( *ep );
    }
    //todo: subscriber->
}

void TestPlayerIf::removeAudioEndpoint( const std::string& id, IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{
    Platform::AudioEndpoint* ep;
    if ( id == "" )
        ep = audioMgr_.getEndpoint( "local" );
    else
        ep = audioMgr_.getEndpoint( id );

    if ( ep )
    {
        audioOut_.removeEndpoint( *ep );
    }
    //todo: subscriber->
}

void TestPlayerIf::getCurrentAudioEndpoints( IMediaInterfaceCallbackSubscriber* subscriber, void* userData )
{

}


/* *****************
 * local helpers
 * *****************/
const char* getEventName(TestPlayerIf::EventItem* event)
{
    switch(event->event_)
    {
        case TestPlayerIf::EVENT_ITERATE_MAIN_LOOP:
            return "EVENT_ITERATE_MAIN_LOOP";

            /* Metadata */
        case TestPlayerIf::EVENT_METADATA_UPDATED:
            return "EVENT_METADATA_UPDATED";
        case TestPlayerIf::EVENT_GENERIC_SEARCH:
            return "EVENT_GENERIC_SEARCH";
        case TestPlayerIf::EVENT_GET_TRACKS:
            return "EVENT_GET_TRACKS";
        case TestPlayerIf::EVENT_GET_IMAGE:
            return "EVENT_GET_IMAGE";
        case TestPlayerIf::EVENT_GET_ALBUM:
            return "EVENT_GET_ALBUM";

            /* Playback handling */
        case TestPlayerIf::EVENT_PLAY_REQ:
            return "EVENT_PLAY_REQ";
        case TestPlayerIf::EVENT_STOP_REQ:
            return "EVENT_STOP_REQ";
        case TestPlayerIf::EVENT_ENQUEUE_TRACK_REQ:
            return "EVENT_ENQUEUE_TRACK_REQ";
        case TestPlayerIf::EVENT_PAUSE_PLAYBACK:
            return "EVENT_PAUSE_PLAYBACK";
        case TestPlayerIf::EVENT_RESUME_PLAYBACK:
            return "EVENT_RESUME_PLAYBACK";
        case TestPlayerIf::EVENT_PLAY_TRACK:
            return "EVENT_PLAY_TRACK";
        case TestPlayerIf::EVENT_TRACK_ENDED:
            return "EVENT_TRACK_ENDED";

             /* Session handling */
        case TestPlayerIf::EVENT_LOGGING_IN:
            return "EVENT_LOGGING_IN";
        case TestPlayerIf::EVENT_LOGGED_IN:
            return "EVENT_LOGGED_IN";
        case TestPlayerIf::EVENT_LOGGING_OUT:
            return "EVENT_LOGGING_OUT";
        case TestPlayerIf::EVENT_LOGGED_OUT:
            return "EVENT_LOGGED_OUT";
        case TestPlayerIf::EVENT_CONNECTION_LOST:
            return "EVENT_CONNECTION_LOST";
	}
    return "Unknown event type!";
}


