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


#include "AudioEndpointRemotePeer.h"
#include "applog.h"

AudioEndpointRemotePeer::AudioEndpointRemotePeer( Socket* socket, ConfigHandling::AudioEndpointConfig& config )
        : SocketPeer(socket), endpoint_(config)
{
}

AudioEndpointRemotePeer::~AudioEndpointRemotePeer()
{
    endpoint_.destroy();
}

void AudioEndpointRemotePeer::processMessage(const Message* msg)
{
    log(LOG_NOTICE) << *(msg);

    switch(msg->getType())
    {
        case AUDIO_DATA_IND:
        {
            const IntTlv* channelstlv = (const IntTlv*)msg->getTlvRoot()->getTlv(TLV_AUDIO_CHANNELS);
            const IntTlv* ratetlv     = (const IntTlv*)msg->getTlvRoot()->getTlv(TLV_AUDIO_RATE);
            const IntTlv* nsamplestlv = (const IntTlv*)msg->getTlvRoot()->getTlv(TLV_AUDIO_NOF_SAMPLES);
            const BinaryTlv* samplestlv = (const BinaryTlv*)msg->getTlvRoot()->getTlv(TLV_AUDIO_DATA);

            if ( channelstlv && ratetlv && samplestlv && samplestlv )
            {
                unsigned short channels = channelstlv->getVal();
                unsigned int rate       = ratetlv->getVal();
                unsigned int nsamples   = nsamplestlv->getVal();
                const int16_t* samples  = (const int16_t*)samplestlv->getData(); /* todo ntoh me! */

                endpoint_.enqueueAudioData(channels, rate, nsamples, samples);
            }
        }
        break;

        default:
            break;
    }
}
