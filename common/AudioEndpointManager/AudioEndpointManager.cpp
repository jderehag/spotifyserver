/*
 * Copyright (c) 2013, Jens Nielsen
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

#include "AudioEndpointManager.h"
#include "applog.h"

AudioEndpointManager::AudioEndpointManager( AudioProvider& m ) : m_(m)
{
}

AudioEndpointManager::~AudioEndpointManager()
{
}

void AudioEndpointManager::createEndpoint( Platform::AudioEndpoint& ep, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    audioEndpoints.insert( AudioEndpointItem(&ep, NULL) );

    //todo: subscriber->createEndpointResponse();

    doEndpointsUpdatedNotification(); //todo only if didn't exist already?
}

void AudioEndpointManager::deleteEndpoint( Platform::AudioEndpoint& ep, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    if ( audioEndpoints.find(&ep) != audioEndpoints.end() )
    {
        if ( audioEndpoints[&ep] != NULL )
        {
            audioEndpoints[&ep]->removeAudioEndpoint(ep);
        }
        audioEndpoints.erase( &ep );

        doEndpointsUpdatedNotification();
    }

    //todo: subscriber->deleteEndpointResponse();
}

void AudioEndpointManager::addEndpoint( std::string id, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    Platform::AudioEndpoint* ep = getEndpoint( id );
    if ( ep )
    {
        m_.addAudioEndpoint( *ep );
        audioEndpoints[ep] = &m_;

        doEndpointsUpdatedNotification();
    }
    //todo: subscriber->addEndpointResponse();
}
void AudioEndpointManager::removeEndpoint( std::string id, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    Platform::AudioEndpoint* ep = getEndpoint( id );
    if ( ep )
    {
        m_.removeAudioEndpoint( *ep );
        audioEndpoints[ep] = NULL;

        doEndpointsUpdatedNotification();
    }
    //todo: subscriber->removeEndpointResponse();
}

void AudioEndpointManager::getEndpoints( IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    std::map<std::string, bool> result;

    AudioEndpointMap::const_iterator it = audioEndpoints.begin();
    for( ; it != audioEndpoints.end(); it++ )
    {
        log(LOG_DEBUG) << (*it).first->getId();
        result.insert( std::pair<std::string, bool>((*it).first->getId(), (*it).second != NULL ) );
    }

    subscriber->getEndpointsResponse( result, userData );
}


Platform::AudioEndpoint* AudioEndpointManager::getEndpoint( std::string id )
{
    AudioEndpointMap::const_iterator it = audioEndpoints.begin();
    for( ; it != audioEndpoints.end(); it++ )
    {
        if ( (*it).first->getId() == id )
        {
            return (*it).first;
        }
    }
    return NULL;
}

