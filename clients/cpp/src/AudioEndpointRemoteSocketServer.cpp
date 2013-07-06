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

#include "AudioEndpointRemoteSocketServer.h"
#include "MessageFactory/Message.h"
#include "MessageFactory/MessageDecoder.h"
#include "MessageFactory/SocketReader.h"
#include "Platform/Utils/Utils.h"
#include <set>
#include "applog.h"
#include <stdint.h>

/*
#include "Freertos.h"
#include "task.h"

extern "C"
{
extern uint16_t AUDIO_SAMPLE[];
extern const uint32_t audio_sample_size;
}*/
AudioEndpointRemoteSocketServer::AudioEndpointRemoteSocketServer(Platform::AudioEndpoint& endpoint) : Platform::Runnable(true, SIZE_MEDIUM, PRIO_MID),
                                                                                                      /*sock_(SOCKTYPE_DATAGRAM),*/
                                                                                                      ep_(endpoint)
{
    startThread();
}

AudioEndpointRemoteSocketServer::~AudioEndpointRemoteSocketServer()
{
}

void AudioEndpointRemoteSocketServer::run()
{
    int rc;
    char buf[2000];
    int n = 0;
    /*while(1)
    {
        n += ep_.enqueueAudioData(2, 44100, 350, (int16_t*)&AUDIO_SAMPLE[n*2]);
        if ( n*4 >= audio_sample_size - 1764 )
            n=0;
        vTaskDelay(5);
    }*/

    while( !isCancellationPending() )
    {
        Socket sock_(SOCKTYPE_DATAGRAM);
        SocketReader reader(&sock_);

        if ( sock_.BindToAddr("ANY", "7789") < 0 )
        {
            continue;
        }

        while( !isCancellationPending() )
        {
            std::set<Socket*> readsockets;
            std::set<Socket*> writesockets;
            std::set<Socket*> errsockets;

            readsockets.insert(&sock_);
            errsockets.insert(&sock_);

            rc = select(&readsockets, &writesockets, &errsockets, 1000);

            if ( !readsockets.empty() )
            {
                header_t* hdr;
                uint32_t len;

                rc = sock_.Receive(buf, sizeof(buf));

                if ( rc < 0 )
                    break;

                hdr = (header_t*) buf;
                len = Ntohl(hdr->len);

                /* sanity check, actual received bytes must match message length */
                if ( len != rc )
                    continue;

                if ( Ntohl(hdr->type) == AUDIO_DATA_IND )
                {
                    uint32_t n = sizeof(header_t);
                    uint32_t* pchannels = NULL;
                    uint32_t* prate = NULL;
                    uint32_t* pnsamples = NULL;
                    int16_t* psamples = NULL;

                    while( n < len )
                    {
                        tlvheader_t* tlvhdr = (tlvheader_t*) &buf[n];

                        switch( Ntohl(tlvhdr->type) )
                        {
                            case TLV_AUDIO_CHANNELS:
                            {
                                pchannels = (uint32_t*)&buf[n+sizeof(tlvheader_t)];
                                break;
                            }

                            case TLV_AUDIO_RATE:
                            {
                                prate = (uint32_t*)&buf[n+sizeof(tlvheader_t)];
                                break;
                            }

                            case TLV_AUDIO_NOF_SAMPLES:
                            {
                                pnsamples = (uint32_t*)&buf[n+sizeof(tlvheader_t)];
                                break;
                            }

                            case TLV_AUDIO_DATA:
                            {
                                psamples = (int16_t*)&buf[n+sizeof(tlvheader_t)];
                                break;
                            }
                        }

                        n += sizeof(tlvheader_t) + Ntohl(tlvhdr->len);

                    }

                    if ( pchannels && prate && pnsamples && psamples )
                    {
                        uint32_t channels = Ntohl(*pchannels);
                        uint32_t rate = Ntohl(*prate);
                        uint32_t nsamples = Ntohl(*pnsamples);

                        for ( uint32_t i = 0; i < nsamples*2; i++ )
                            psamples[i] = Ntohs(psamples[i]);

                        ep_.enqueueAudioData(channels, rate, nsamples, psamples);
                        /*while( ep_.enqueueAudioData(channels, rate, nsamples, psamples) == 0 )
                            vTaskDelay(2);*/
                    }
                }
            }
        }
    }
}

void AudioEndpointRemoteSocketServer::destroy()
{
    cancelThread();
    joinThread();
}
