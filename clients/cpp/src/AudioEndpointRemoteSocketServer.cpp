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
#include "Platform/Utils/Utils.h"
#include "Platform/Encryption/Encryption.h"
#include "ClockSync/ClockSyncClient.h"
#include "applog.h"
#include <stdint.h>
#include <string.h>


extern "C" const uint8_t g_appkey[];


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
AudioEndpointRemoteSocketServer::AudioEndpointRemoteSocketServer(Platform::AudioEndpoint& endpoint) : Platform::Runnable(true, SIZE_MEDIUM, PRIO_HIGH),
                                                                                                      ep_(endpoint),
                                                                                                      isPlaying_(false)
{
    startThread();
}

AudioEndpointRemoteSocketServer::~AudioEndpointRemoteSocketServer()
{
}
void AudioEndpointRemoteSocketServer::run()
{
    int rc;
    uint8_t buf[1500];
    ClockSyncClient cs;
    std::string addr = "";
    std::string port = "";
    uint32_t drops = 0;

    while( !isCancellationPending() )
    {
        Socket sock_( SOCKTYPE_DATAGRAM );
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

            rc = select(&readsockets, NULL, &errsockets, 25);

            if ( isPlaying_ && cs.timeToSync() )
            {
                uint32_t len = 0;
                header_t* hdr = (header_t*)buf;
                tlvheader_t* tlv;
                uint32_t* val;
                hdr->type = Htonl(AUDIO_SYNC_REQ);
                hdr->id = 7;
                len += sizeof(header_t);

                tlv = (tlvheader_t*)&buf[len];
                tlv->type = Htonl( TLV_AUDIO_BUFFERED_SAMPLES );
                tlv->len = Htonl( sizeof(uint32_t) );
                len += sizeof(tlvheader_t);

                val = (uint32_t*)&buf[len];
                *val = Htonl( ep_.getNumberOfQueuedSamples() );
                len += sizeof(uint32_t);

                tlv = (tlvheader_t*)&buf[len];
                tlv->type = Htonl( TLV_CLIENT_CLOCK );
                tlv->len = Htonl( sizeof(uint32_t) );
                len += sizeof(tlvheader_t);

                val = (uint32_t*)&buf[len];
                *val = Htonl( cs.convertToServerTime( getTick_ms() ) );
                len += sizeof(uint32_t);

                hdr->len = Htonl(len);

                sock_.SendTo( buf, len, addr, port);
            }

            if ( !readsockets.empty() )
            {
                header_t* hdr;
                uint32_t len;

                rc = sock_.ReceiveFrom( buf, sizeof(buf), addr, port );

                if ( rc < 0 )
                    break;

                hdr = (header_t*) buf;
                len = Ntohl(hdr->len);

                /* sanity check, actual received bytes must match message length */
                if ( len != (uint32_t)rc )
                    continue;

                if ( Ntohl(hdr->type) == AUDIO_SYNC_RSP )
                {
                    MessageDecoder dec;
                    Message* rsp = dec.decode( buf );
                    cs.handleResponse( rsp );
                    delete rsp;
                }
                else if ( Ntohl(hdr->type) == AUDIO_DATA_IND )
                {
                    uint32_t n = sizeof(header_t);
                    uint32_t channels = 0;
                    bool haschannels = false;
                    uint32_t rate = 0;
                    bool hasrate = false;
                    uint32_t nsamples = 0;
                    bool hasnsamples = false;
                    uint32_t servertimestamp = 0;
                    bool hasservertimestamp = false;
                    int16_t* psamples = NULL;
                    uint32_t datasize = 0;
                    uint8_t* iv = NULL;

                    while( n < len )
                    {
                        tlvheader_t* tlvhdr = (tlvheader_t*) &buf[n];

                        switch( Ntohl(tlvhdr->type) )
                        {
                            case TLV_AUDIO_TIMESTAMP:
                            {
                                memcpy( &servertimestamp, &buf[n+sizeof(tlvheader_t)], sizeof(uint32_t));
                                servertimestamp = Ntohl(servertimestamp);
                                hasservertimestamp = true;
                                break;
                            }

                            case TLV_AUDIO_CHANNELS:
                            {
                                memcpy( &channels, &buf[n+sizeof(tlvheader_t)], sizeof(uint32_t));
                                channels = Ntohl(channels);
                                haschannels = true;
                                break;
                            }

                            case TLV_AUDIO_RATE:
                            {
                                memcpy( &rate, &buf[n+sizeof(tlvheader_t)], sizeof(uint32_t));
                                rate = Ntohl(rate);
                                hasrate = true;
                                break;
                            }

                            case TLV_AUDIO_NOF_SAMPLES:
                            {
                                memcpy( &nsamples, &buf[n+sizeof(tlvheader_t)], sizeof(uint32_t));
                                nsamples = Ntohl(nsamples);
                                hasnsamples = true;
                                break;
                            }

                            case TLV_AUDIO_DATA:
                            {
                                psamples = (int16_t*)&buf[n+sizeof(tlvheader_t)];
                                datasize = Ntohl(tlvhdr->len);
                                break;
                            }

                            case TLV_ENCRYPTION_IV:
                            {
                                iv = (uint8_t*)&buf[n+sizeof(tlvheader_t)];
                                break;
                            }
                        }

                        n += sizeof(tlvheader_t) + Ntohl(tlvhdr->len);

                    }

                    if ( haschannels && hasrate && hasnsamples && psamples && iv )
                    {
                        uint32_t clienttimestamp = 0;
                        uint32_t length = nsamples*channels*sizeof(int16_t);
                        AudioFifoData* afd = ep_.getFifoDataBuffer( length );

                        if ( afd != NULL && Decrypt( g_appkey, iv, (uint8_t*)psamples, datasize, (uint8_t*)afd->samples, afd->bufferSize ) >= 0 )
                        {
                            for ( uint32_t i = 0; i < (nsamples*channels); i++ )
                                afd->samples[i] = Ntohs(afd->samples[i]);

                            /* only do timestamp stuff if server sends it */
                            if ( hasservertimestamp && cs.hasValidSync() )
                            {
                                // todo we should make sure clock sync is up to date

                                clienttimestamp = cs.convertToLocalTime( servertimestamp );

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

                            afd->timestamp = clienttimestamp;
                            afd->nsamples = nsamples;
                            afd->rate = rate;
                            afd->channels = channels;

                            if ( ep_.enqueueAudioData( afd ) == 0 )
                                drops++;

                            isPlaying_ = true;
                            /*while( ep_.enqueueAudioData( clienttimestamp, channels, rate, nsamples, psamples) == 0 )
                                vTaskDelay(2);*/
                        }
                        else
                        {
                            drops++;
                            ep_.returnFifoDataBuffer( afd );
                        }
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

