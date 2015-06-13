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
#include <map>

class IAudioEndpointRemoteCtrlInterface
{
public:
//    virtual ~IAudioEndpointRemoteCtrlInterface();
    virtual void setMasterVolume( uint8_t volume ) = 0;
    virtual void setRelativeVolume( uint8_t volume ) = 0;
};


class AudioEndpointRemoteCounters;

namespace Platform
{

class AudioEndpointRemote : public AudioEndpoint, public Platform::Runnable
{
private:
    Socket sock_;
    uint32_t remoteBufferSize;
    IAudioEndpointRemoteCtrlInterface* ctrlIf_;

    class AudioEndpointRemoteCounters : public Counters
    {
    public:
        typedef enum
        {
            PACKETS_RECEIVED,
            PACKETS_SENT,
            TOO_EARLY,
            BUCKET_EMPTY,
            NROF_COUNTERS,
        } AudioEndpointRemoteCounterTypes;
        std::map<AudioEndpointRemoteCounterTypes, uint32_t> counters;

        AudioEndpointRemoteCounters() { for(uint8_t i = 0; i < NROF_COUNTERS; i++) counters[(AudioEndpointRemoteCounterTypes)i] = 0; }

        void increment(AudioEndpointRemoteCounterTypes counter) { counters[counter]++; }
        virtual uint8_t getNrofCounters() const { return NROF_COUNTERS; }
        virtual std::string getCounterName(uint8_t counter) const { switch(counter) {
                                                                        case PACKETS_RECEIVED: return "Packets received";
                                                                        case PACKETS_SENT: return "Packets sent";
                                                                        case TOO_EARLY: return "Data too early";
                                                                        case BUCKET_EMPTY: return "Bucket empty";
                                                                        } return ""; }
        virtual uint32_t getCounterValue(uint8_t counter) const { return counters.at((AudioEndpointRemoteCounterTypes)counter); }
    };

    class AudioEndpointRemoteCounters counters;

public:
    AudioEndpointRemote(IAudioEndpointRemoteCtrlInterface* ctrlIf, const EndpointIdIf& epId, const std::string& serveraddr, const std::string& serverport, uint8_t volume, unsigned int bufferNSecs);

    /* AudioEndpoint implementation */
    virtual int enqueueAudioData( unsigned int timestamp, unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples );
    virtual void flushAudioData();

    virtual unsigned int getNumberOfQueuedSamples();

    virtual void setMasterVolume( uint8_t volume );
    virtual void doSetRelativeVolume( uint8_t volume );

    virtual const Counters& getStatistics();

    virtual bool isLocal() const {return false;};

    /* Runnable implementation*/
    virtual void run();

    virtual void destroy();
};

}
#endif /* AUDIOENDPOINTREMOTE_H_ */
