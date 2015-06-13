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

#ifndef REMOTEENDPOINTMANAGER_H_
#define REMOTEENDPOINTMANAGER_H_

#include "EndpointManagerCtrlInterface.h"
#include "SocketHandling/Messenger.h"
#include "AudioEndpointRemoteSocketServer.h"
#include "EndpointId/EndpointIdIf.h"

class RemoteEndpointManager : public EndpointCtrlInterface, public IMessageSubscriber
{
private:
    Messenger& messenger_;
    EndpointIdIf& epId;
    AudioEndpointRemoteSocketServer* server;
    Platform::AudioEndpoint* ep_;

    bool connectionUp_;
    void sendCreateEndpointMessage();
    void handleSetVolumeReq( const Message* msg );
public:
    RemoteEndpointManager( Messenger& m, EndpointIdIf&  epId_ );
    virtual ~RemoteEndpointManager();

    virtual void registerId( EndpointIdIf& appId );
    virtual void unregisterId( EndpointIdIf& appId );

    /* Implements EndpointCtrlInterface */
    virtual void getEndpoints( IEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void renameEndpoint( const std::string& from, const std::string& to, IEndpointCtrlCallbackSubscriber* subscriber, void* userData );

    virtual void createAudioEndpoint( Platform::AudioEndpoint& ep, IEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void deleteAudioEndpoint( Platform::AudioEndpoint& ep, IEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void addAudioEndpoint( const std::string& id, IEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void removeAudioEndpoint( const std::string& id, IEndpointCtrlCallbackSubscriber* subscriber, void* userData );
    virtual void getAudioEndpoints( IEndpointCtrlCallbackSubscriber* subscriber, void* userData );

    virtual void setRelativeVolume( const std::string& id, uint8_t volume );

    virtual void getStatistics( const std::string& id, IEndpointCtrlCallbackSubscriber* subscriber, void* userData );

    /* Implements IMessageSubscriber */
    virtual void connectionState( bool up );
    virtual void receivedMessage( const Message* msg );
    virtual void receivedResponse( const Message* rsp, const Message* req, void* userData );
};

#endif /* REMOTEENDPOINTMANAGER_H_ */
