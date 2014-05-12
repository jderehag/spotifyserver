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

#ifndef ENDPOINTMANAGERCTRLINTERFACE_H_
#define ENDPOINTMANAGERCTRLINTERFACE_H_

#include "Platform/Threads/Mutex.h"
#include "Platform/AudioEndpoints/AudioEndpoint.h"
#include <list>
#include <set>
#include <string>

class AudioEndpointInfo
{
public:
    AudioEndpointInfo( std::string id_, bool active_, uint8_t vol_ ) : id(id_), active(active_), relativeVolume(vol_) {}
    std::string id;
    bool active;
    uint8_t relativeVolume;

    bool operator< (const AudioEndpointInfo& r) const { return id < r.id; }
};

typedef std::list<AudioEndpointInfo> AudioEndpointInfoList;
typedef std::list<std::string> EndpointInfoList;

class IEndpointCtrlCallbackSubscriber
{
public:
    virtual void connectionState( bool up ) {}

    virtual void renameEndpointResponse( void* userData ) {}
    virtual void getEndpointsResponse( const EndpointInfoList& endpoints, void* userData ) {}

    virtual void getAudioEndpointsResponse( const AudioEndpointInfoList& endpoints, void* userData ) {}
    virtual void audioEndpointsUpdatedNtf() {}
};


class EndpointCtrlInterface
{
private:

protected:
    /***********************
     * Callback subscription
     ***********************/
    Platform::Mutex callbackSubscriberMtx_;
    typedef std::set<IEndpointCtrlCallbackSubscriber*> EndpointCtrlCallbackSubscriberSet;
    EndpointCtrlCallbackSubscriberSet callbackSubscriberList_;

    /*typical data related to a request that will be needed when it's time for a response
      i.e. the subscriber the request originated from and the user data allocated with the request*/
    typedef std::pair<IEndpointCtrlCallbackSubscriber*, void*> PendingEndpointCtrlRequestData;

    void doAudioEndpointsUpdatedNotification();

public:

    void registerForCallbacks(IEndpointCtrlCallbackSubscriber& subscriber);
    void unRegisterForCallbacks(IEndpointCtrlCallbackSubscriber& subscriber);

    virtual void registerId( EndpointIdIf& appId ) = 0;
    virtual void unregisterId( EndpointIdIf& appId ) = 0;

    virtual void getEndpoints( IEndpointCtrlCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void renameEndpoint( const std::string& from, const std::string& to, IEndpointCtrlCallbackSubscriber* subscriber, void* userData ) = 0;

    virtual void createAudioEndpoint( Platform::AudioEndpoint& ep, IEndpointCtrlCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void deleteAudioEndpoint( Platform::AudioEndpoint& ep, IEndpointCtrlCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void addAudioEndpoint( std::string id, IEndpointCtrlCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void removeAudioEndpoint( std::string id, IEndpointCtrlCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void getAudioEndpoints( IEndpointCtrlCallbackSubscriber* subscriber, void* userData ) = 0;

    virtual void setRelativeVolume( std::string id, uint8_t volume ) = 0;

};


#endif /* ENDPOINTMANAGERCTRLINTERFACE_H_ */
