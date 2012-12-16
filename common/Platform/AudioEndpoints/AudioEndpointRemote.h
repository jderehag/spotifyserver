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

#ifndef AUDIOENDPOINTREMOTE_H_
#define AUDIOENDPOINTREMOTE_H_

#include "AudioEndpoint.h"
#include "SocketHandling/SocketClient.h"
#include "Platform/Threads/Mutex.h"

namespace Platform
{

class AudioEndpointRemote : public AudioEndpoint, public IMessageSubscriber
{
private:
    SocketClient m;

    void sendAudioData();

public:
    AudioEndpointRemote();
    virtual ~AudioEndpointRemote();

    /* AudioEndpoint implementation */
    virtual int enqueueAudioData(unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples);
    virtual void flushAudioData();

    /* IMessageSubscriber implementation */
    virtual void connectionState( bool up );
    virtual void receivedMessage( Message* msg );
    virtual void receivedResponse( Message* rsp, Message* req );

};
}
#endif /* AUDIOENDPOINTREMOTE_H_ */
