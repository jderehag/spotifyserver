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

#ifndef AUDIOENDPOINTMANAGERCTRLINTERFACE_H_
#define AUDIOENDPOINTMANAGERCTRLINTERFACE_H_

#include "Platform/Threads/Mutex.h"
#include "Platform/AudioEndpoints/AudioEndpoint.h"
#include <set>
#include <string>

class IAudioEndpointCtrlCallbackSubscriber
{
public:
    virtual void connectionState( bool up ) = 0;

    virtual void getEndpointsResponse( const std::set<std::string> endpoints, void* userData ) = 0;
};


class AudioEndpointCtrlInterface
{
private:

protected:
    /***********************
     * Callback subscription
     ***********************/
    Platform::Mutex callbackSubscriberMtx_;
    typedef std::set<IAudioEndpointCtrlCallbackSubscriber*> AudioEndpointCtrlCallbackSubscriberSet;
    AudioEndpointCtrlCallbackSubscriberSet callbackSubscriberList_;

public:

    void registerForCallbacks(IAudioEndpointCtrlCallbackSubscriber& subscriber);
    void unRegisterForCallbacks(IAudioEndpointCtrlCallbackSubscriber& subscriber);

    virtual void addEndpoint( Platform::AudioEndpoint& ep ,IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void removeEndpoint( Platform::AudioEndpoint& ep, IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void getEndpoints( IAudioEndpointCtrlCallbackSubscriber* subscriber, void* userData ) = 0;

};


#endif /* AUDIOENDPOINTMANAGERCTRLINTERFACE_H_ */
