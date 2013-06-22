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
#include "Platform/Utils/Utils.h"
#include "applog.h"
#include <stdlib.h>

#include <iostream>
#include <errno.h>

namespace Platform {

AudioEndpointRemote::AudioEndpointRemote( const std::string& id, const std::string& serveraddr, 
                                          const std::string& serverport, unsigned int bufferNSecs) : Platform::Runnable( true, SIZE_SMALL, PRIO_HIGH ),
                                                                                                     sock_(SOCKTYPE_DATAGRAM),
                                                                                                     id_(id)
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
#define SAMPLE_BATCH_SIZE 350
#define BUCKET_INITIAL_VALUE (38*350)
#define BUCKET_REFILL_TOKENS 441
#define BUCKET_REFILL_INTERVAL_MS 10

void AudioEndpointRemote::run()
{
    AudioFifoData* afd;
    static uint32_t reqId = 0;
    unsigned int bucket = BUCKET_INITIAL_VALUE;
    uint32_t currentTime;
    uint32_t lastTime = getTick_ms();

    while ( isCancellationPending() == false )
    {

        int rc = 0;

        while ( ( afd = fifo_.getFifoDataTimedWait(10) ) == NULL && isCancellationPending() == false );
        if ( isCancellationPending() != false ) break;

        while ( isCancellationPending() == false )
        {
            currentTime = getTick_ms();

            if ( currentTime > lastTime + BUCKET_REFILL_INTERVAL_MS )
            {
                if ( currentTime > lastTime + ((BUCKET_INITIAL_VALUE * BUCKET_REFILL_INTERVAL_MS ) / BUCKET_REFILL_TOKENS ) )
                {
                    /* the delay since last update would more than refill the bucket, reset bucket instead of calculating and capping since it may have been a while */
                    //log(LOG_DEBUG) << "Bucket reset at " << currentTime << " Last time = " << lastTime;
                    bucket = BUCKET_INITIAL_VALUE;
                    lastTime = currentTime;
                }
                else
                {
                    //log(LOG_DEBUG) << "Bucket refill at " << currentTime << " Last time = " << lastTime << " Bucket = " << bucket;
                    bucket += BUCKET_REFILL_TOKENS;
                    lastTime += BUCKET_REFILL_INTERVAL_MS;
                }
            }

            if ( bucket >= afd->nsamples )
            {
                //log(LOG_DEBUG) << "Samples " << afd->nsamples << " Bucket " << bucket << " Now = " << currentTime;
                break;
            }

            sleep_ms( 5 );
        }

        if ( isCancellationPending() != false ) break;

        bucket -= afd->nsamples;

        Message* msg = new Message( AUDIO_DATA_IND );
        msg->setId(reqId++); //for debug
        msg->addTlv( TLV_AUDIO_CHANNELS, afd->channels );
        msg->addTlv( TLV_AUDIO_RATE, afd->rate );
        msg->addTlv( TLV_AUDIO_NOF_SAMPLES, afd->nsamples );

        for( uint32_t i = 0 ; i < afd->nsamples*2 ; i++ )
            afd->samples[i] = Htons(afd->samples[i]);

        msg->addTlv( new BinaryTlv( TLV_AUDIO_DATA, (const uint8_t*) afd->samples, afd->nsamples * sizeof(int16_t) * afd->channels ) );

        MessageEncoder* enc = msg->encode();
        delete msg;

        if((rc = sock_.Send(enc->getBuffer(), enc->getLength()) < 0))
        {
            /* TODO: its probably NOT ok to use errno here, doubt its valid on WIN */
            std::cout << "JESPER: rc=" << rc << " perror=" << errno;
        }

        /* back off a little before next packet to avoid packet storm on client.. */
        sleep_ms(3);

        delete enc;
        fifo_.returnFifoDataBuffer( afd );
        afd = NULL;
    }

    if ( afd )
        fifo_.returnFifoDataBuffer( afd );
}

int AudioEndpointRemote::enqueueAudioData(unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples)
{
    unsigned int n = 0;
    unsigned int taken = 0;
    unsigned int samplesleft = nsamples;
    if (nsamples == 0)
        return 0; // Audio discontinuity, do nothing

    do
    {
        taken = fifo_.addFifoDataBlocking(channels, rate, ( samplesleft > SAMPLE_BATCH_SIZE ? SAMPLE_BATCH_SIZE : samplesleft ), &samples[n*2]);
        n += taken;
        samplesleft -= taken;
    } while( taken > 0 && n < nsamples );

    return n;
}

void AudioEndpointRemote::flushAudioData()
{
}

void AudioEndpointRemote::destroy()
{
    cancelThread();
    joinThread();
}


}
