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

// some debug counters...
#ifdef DEBUG_COUNTERS
int firstpacketclienttime = 0;
int firstpacketservertime = 0;
int servertimeplay = 0;
int clienttimeplay = 0;
uint32_t firstclockdiff = 0;
uint32_t totalrecsamples = 0;
#endif
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
    uint8_t buf[2000];

    while( !isCancellationPending() )
    {
        Socket sock_( SOCKTYPE_DATAGRAM );
        SocketReader reader( &sock_ );

        if ( sock_.BindToAddr("ANY", "7789") < 0 )
        {
            continue;
        }

        while( !isCancellationPending() )
        {
            std::set<Socket*> readsockets;
            std::set<Socket*> errsockets;

            readsockets.insert(&sock_);
            errsockets.insert(&sock_);

            rc = select(&readsockets, NULL, &errsockets, 1000);

            if ( !readsockets.empty() )
            {
                header_t* hdr;
                uint32_t len;

                /* todo here we assume we get the whole message in one read, should have better handling */
                rc = sock_.Receive( buf, sizeof(buf) );

                if ( rc < 0 )
                    break;

                hdr = (header_t*) buf;
                len = Ntohl(hdr->len);

                /* sanity check, actual received bytes must match message length */
                if ( len != (uint32_t)rc )
                    continue;

                if ( Ntohl(hdr->type) == AUDIO_DATA_IND )
                {
                    uint32_t n = sizeof(header_t);
                    uint32_t* ptimestamp = NULL;
                    uint32_t* pchannels = NULL;
                    uint32_t* prate = NULL;
                    uint32_t* pnsamples = NULL;
                    int16_t* psamples = NULL;

                    uint32_t* pserverclock = NULL;

                    while( n < len )
                    {
                        tlvheader_t* tlvhdr = (tlvheader_t*) &buf[n];

                        switch( Ntohl(tlvhdr->type) )
                        {
                            case TLV_CLOCK:
                            {
                                pserverclock = (uint32_t*)&buf[n+sizeof(tlvheader_t)];
                                break;
                            }

                            case TLV_AUDIO_TIMESTAMP:
                            {
                                ptimestamp = (uint32_t*)&buf[n+sizeof(tlvheader_t)];
                                break;
                            }

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
                        uint32_t channels = Ntohl( *pchannels );
                        uint32_t rate = Ntohl( *prate );
                        uint32_t nsamples = Ntohl( *pnsamples );
                        uint32_t clienttimestamp = 0;

                        for ( uint32_t i = 0; i < (nsamples*channels); i++ )
                            psamples[i] = Ntohs(psamples[i]);

                        /* only do clock sync and timestamp stuff if server sends it */
                        if ( pserverclock && ptimestamp )
                        {
                            uint32_t serverclock = Ntohl( *pserverclock );
                            uint32_t servertimestamp = Ntohl( *ptimestamp );
                            uint32_t now;

                            static uint32_t clockDiff = 0;

                            now = getTick_ms();
                            /* calculate difference between server clock and our clock and adjust timestamp*/
#define NETWORK_LAG 1
                            if ( clockDiff == 0 )
                            {
                                clockDiff = now - (serverclock + NETWORK_LAG);
                            }
                            else
                            {
                                int32_t drift = (now - (serverclock + NETWORK_LAG) - clockDiff);
                                clockDiff += (drift/3);
                            }
                            clienttimestamp = servertimestamp + clockDiff;

                            //debug counters
#ifdef DEBUG_COUNTERS
                            if ( firstpacketclienttime == 0 ) firstpacketclienttime = clienttimestamp;
                            if ( firstpacketservertime == 0 ) firstpacketservertime = servertimestamp;
                            servertimeplay = servertimestamp - firstpacketservertime;
                            clienttimeplay = now - firstpacketclienttime;
                        }
                        totalrecsamples += nsamples;
#else
                        }
#endif

                        ep_.enqueueAudioData( clienttimestamp, channels, rate, nsamples, psamples);

                        /*while( ep_.enqueueAudioData( clienttimestamp, channels, rate, nsamples, psamples) == 0 )
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

