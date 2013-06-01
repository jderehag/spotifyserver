/*
 * Copyright (c) 2012, Jens Nielsen
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

#ifndef REMOTEMEDIAINTERFACE_H_
#define REMOTEMEDIAINTERFACE_H_

#include "SocketHandling/Messenger.h"
#include "MessageFactory/TlvDefinitions.h"
#include "MediaInterface/MediaInterface.h"
#include <string>
#include <map>

using namespace LibSpotify;

class RemoteMediaInterface : public IMessageSubscriber, public MediaInterface
{
private:
    Messenger& messenger_;

public:

    RemoteMediaInterface(Messenger& messenger);
    virtual ~RemoteMediaInterface();

    /*Implements IMessageSubscriber*/
    virtual void receivedMessage( Message* msg );
    virtual void receivedResponse( Message* rsp, Message* req, void* userData );
    virtual void connectionState( bool up );

    /*Implements MediaInterface*/
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
    virtual void addAudioEndpoint(Platform::AudioEndpoint& endpoint);

};

#endif /* REMOTEMEDIAINTERFACE_H_ */
