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

#include "AudioEndpointRemote.h"
#include "MessageFactory/Message.h"
#include "MessageFactory/MessageDecoder.h"
#include "ClockSync/ClockSyncServer.h"
#include "Platform/Utils/Utils.h"
#include "applog.h"
#include <stdlib.h>

#include <iostream>
#include <errno.h>

extern bool simPacketDrop;

namespace Platform {

AudioEndpointRemote::AudioEndpointRemote( IAudioEndpointRemoteCtrlInterface* ctrlIf,
                                          const std::string& id,
                                          const std::string& serveraddr,
                                          const std::string& serverport,
                                          uint8_t volume,
                                          unsigned int bufferNSecs) : AudioEndpoint(volume),
                                                                      Platform::Runnable( true, SIZE_SMALL, PRIO_HIGH ),
                                                                      sock_(SOCKTYPE_DATAGRAM),
                                                                      id_(id),
                                                                      remoteBufferSize(0),
                                                                      ctrlIf_(ctrlIf)
{
    fifo_.setFifoBuffer(bufferNSecs);
    sock_.Connect(serveraddr, serverport);

    startThread();
}

std::string AudioEndpointRemote::getId() const
{
    return id_;
}

/* todo some of this stuff should be set remotely */
#define SAMPLE_BATCH_SIZE 330
#define BUCKET_INITIAL_VALUE (20*SAMPLE_BATCH_SIZE)
#define BUCKET_REFILL_TOKENS 441
#define BUCKET_REFILL_INTERVAL_MS 10

void AudioEndpointRemote::run()
{
    AudioFifoData* afd = NULL;
    static uint32_t reqId = 0;
    unsigned int bucket = BUCKET_INITIAL_VALUE;
    uint32_t currentTime;
    uint32_t lastTime = getTick_ms();
    uint32_t count = 0;
    uint32_t highestdiff = 0;
    ClockSyncServer cs;

    while ( isCancellationPending() == false )
    {
        int rc = 0;
        std::set<Socket*> readsockets;
        readsockets.insert( &sock_ );

        rc = select( &readsockets, NULL, NULL, 5 );

        if ( isCancellationPending() != false ) break;

        if ( rc > 0 && !readsockets.empty() )
        {
            uint8_t buf[100];
            if ( sock_.Receive( buf, sizeof(buf) ) > 0 )
            {
                MessageDecoder dec;

                Message* req = dec.decode( buf );

                Message* rsp = cs.handleRequest( req );

                MessageEncoder* enc = rsp->encode();

                sock_.Send( enc->getBuffer(), enc->getLength() );

                delete rsp;
                delete req;
                delete enc;
            }
        }

        if ( afd == NULL )
        {
            afd = fifo_.getFifoDataTimedWait(0);
            if ( afd == NULL )
            {
                continue;
            }
        }

        {
            currentTime = getTick_ms();

            if ( afd->timestamp != 0 )
            {
                if ( afd->timestamp > currentTime && 
                    (afd->timestamp - currentTime) > BUCKET_INITIAL_VALUE * 1000 / afd->rate )
                {
                    /* too early to send this packet, wait a while */
                    continue;
                }
            }

            {
                if ( currentTime > lastTime + BUCKET_REFILL_INTERVAL_MS )
                {
                    if ( currentTime > lastTime + ((BUCKET_INITIAL_VALUE * BUCKET_REFILL_INTERVAL_MS ) / BUCKET_REFILL_TOKENS ) )
                    {
                        /* the delay since last update would more than refill the bucket, reset bucket instead of calculating and capping since it may have been a while */
                        bucket = BUCKET_INITIAL_VALUE;
                        lastTime = currentTime;
                    }
                    else
                    {
                        bucket += BUCKET_REFILL_TOKENS;
                        lastTime += BUCKET_REFILL_INTERVAL_MS;
                    }
                }

                if ( bucket < afd->nsamples )
                {
                    /* not allowed to send this packet yet, wait a while */
                    continue;
                }
            }
        }

        bucket -= afd->nsamples;

        count++;

        Message* msg = new Message( AUDIO_DATA_IND );
        msg->setId(reqId++); //for debug
        msg->addTlv( TLV_AUDIO_CHANNELS, afd->channels );
        msg->addTlv( TLV_AUDIO_RATE, afd->rate );
        msg->addTlv( TLV_AUDIO_NOF_SAMPLES, afd->nsamples );

        for( uint32_t i = 0 ; i < (afd->nsamples*afd->channels) ; i++ )
        {
            afd->samples[i] = Htons(afd->samples[i]);
        }

        /* only send timestamp if it's valid (we're playing by timestamp) */
        if ( afd->timestamp != 0 )
        {
            msg->addTlv( TLV_AUDIO_TIMESTAMP, afd->timestamp );
        }

        msg->addTlv( new BinaryTlv( TLV_AUDIO_DATA, (const uint8_t*) afd->samples, afd->nsamples * sizeof(int16_t) * afd->channels ) );

        MessageEncoder* enc = msg->encode();
        delete msg;

        if ( !simPacketDrop || count % 100 != 0)
        if((rc = sock_.Send(enc->getBuffer(), enc->getLength()) < 0))
        {
            /* TODO: its probably NOT ok to use errno here, doubt its valid on WIN */
            std::cout << "JESPER: rc=" << rc << " perror=" << errno;
        }

        delete enc;
        fifo_.returnFifoDataBuffer( afd );
        afd = NULL;
    }

    if ( afd )
        fifo_.returnFifoDataBuffer( afd );
}

int AudioEndpointRemote::enqueueAudioData( unsigned int timestamp, unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples )
{
    unsigned int n = 0;
    unsigned int taken = 0;
    unsigned int samplesleft = nsamples;
    double offset = 0;

    if (nsamples == 0)
        return 0; // Audio discontinuity, do nothing

    do
    {
        taken = fifo_.addFifoDataBlocking( timestamp + offset, channels, rate, ( samplesleft > SAMPLE_BATCH_SIZE ? SAMPLE_BATCH_SIZE : samplesleft ), &samples[n*2] );
        n += taken;
        samplesleft -= taken;
        if ( timestamp != 0 )
            offset += (double)taken * 1000 / rate;
    } while( taken > 0 && n < nsamples );

    return n;
}

unsigned int AudioEndpointRemote::getNumberOfQueuedSamples()
{
    // suppose we could use remoteBufferSize instead of BUCKET_INITIAL_VALUE here but remoteBufferSize might not be up to date,
    // probably a better idea to use remoteBufferSize to adjust bucket so actual queued samples matches BUCKET_INITIAL_VALUE
    return fifo_.getNumberOfQueuedSamples() + BUCKET_INITIAL_VALUE;
}


void AudioEndpointRemote::flushAudioData()
{
    fifo_.flush();
}

void AudioEndpointRemote::destroy()
{
    cancelThread();
    joinThread();
}

void AudioEndpointRemote::setMasterVolume( uint8_t volume )
{
    masterVolume_ = volume;
    ctrlIf_->setMasterVolume(volume);
}
void AudioEndpointRemote::setRelativeVolume( uint8_t volume )
{
    relativeVolume_ = volume;
    ctrlIf_->setRelativeVolume(volume);
}
}
