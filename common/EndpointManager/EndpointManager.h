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

#ifndef ENDPOINTMANAGER_H_
#define ENDPOINTMANAGER_H_

#include "EndpointManagerCtrlInterface.h"
#include "Platform/AudioEndpoints/AudioEndpoint.h"
#include "MediaInterface/AudioProvider.h"
#include <map>

class EndpointManager : public EndpointCtrlInterface
{
private:
    class EndpointItem
    {
    public:
        EndpointIdIf& epId;
        Platform::AudioEndpoint* audioEp;
        AudioProvider* audioSource;
        EndpointItem(EndpointIdIf& epId_) : epId(epId_), audioEp(NULL), audioSource(NULL) {}
    };

    typedef std::list<EndpointItem> EndpointContainer;
    EndpointContainer endpoints;
    AudioProvider& m_;

    Platform::AudioEndpoint* getAudioEndpoint( const std::string& id );
    EndpointContainer::iterator find( const std::string& id );

public:
    EndpointManager( AudioProvider& m );
    virtual ~EndpointManager();

    virtual void registerId( EndpointIdIf& appId );
    virtual void unregisterId( EndpointIdIf& appId );

    virtual void getEndpoints( IEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void renameEndpoint( const std::string& from, const std::string& to, IEndpointCtrlCallbackSubscriber* subscriber, void* userData );

    virtual void createAudioEndpoint( Platform::AudioEndpoint& ep, IEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void deleteAudioEndpoint( Platform::AudioEndpoint& ep, IEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void addAudioEndpoint( std::string ep, IEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void removeAudioEndpoint( std::string ep, IEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void getAudioEndpoints( IEndpointCtrlCallbackSubscriber* subscriber, void* userData );

    virtual void setRelativeVolume( std::string id, uint8_t volume );

};

#endif /* ENDPOINTMANAGER_H_ */
