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

#ifndef AUDIOENDPOINTMANAGER_H_
#define AUDIOENDPOINTMANAGER_H_

#include "AudioEndpointManagerCtrlInterface.h"
#include "Platform/AudioEndpoints/AudioEndpoint.h"
#include "MediaInterface/AudioProvider.h"
#include <map>

class AudioEndpointManager : public AudioEndpointCtrlInterface
{
private:
    typedef std::pair<Platform::AudioEndpoint*, AudioProvider*> AudioEndpointItem;
    typedef std::map<Platform::AudioEndpoint*, AudioProvider*> AudioEndpointMap;
    AudioEndpointMap audioEndpoints;
    AudioProvider& m_;

    Platform::AudioEndpoint* getEndpoint( std::string id );

public:
    AudioEndpointManager( AudioProvider& m );
    virtual ~AudioEndpointManager();

    virtual void createEndpoint( Platform::AudioEndpoint& ep, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void deleteEndpoint( Platform::AudioEndpoint& ep, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void addEndpoint( std::string ep, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void removeEndpoint( std::string ep, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void getEndpoints( IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData );

    virtual void setRelativeVolume( std::string id, uint8_t volume );

};

#endif /* AUDIOENDPOINTMANAGER_H_ */
