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
#include "Platform/Threads/Runnable.h"
#include "Platform/Socket/Socket.h"
#include <string>

namespace Platform
{

class AudioEndpointRemote : public AudioEndpoint, public Platform::Runnable
{
private:
    Socket sock_;
    std::string id_;

    void sendAudioData();

public:
    AudioEndpointRemote(const std::string& id, const std::string& serveraddr, const std::string& serverport, unsigned int bufferNSecs);

    /* AudioEndpoint implementation */
    virtual int enqueueAudioData(unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples);
    virtual void flushAudioData();

    virtual std::string getId() const;

    virtual bool isLocal() const {return false;};

    /* Runnable implementation*/
    virtual void run();

    virtual void destroy();
};

}
#endif /* AUDIOENDPOINTREMOTE_H_ */
