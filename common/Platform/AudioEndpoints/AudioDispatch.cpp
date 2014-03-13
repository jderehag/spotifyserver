/*
 * Copyright (c) 2013, Jens Nielsen
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

#include "AudioDispatch.h"
#include "MessageFactory/Message.h"
#include "Platform/Utils/Utils.h"
#include "applog.h"
#include <stdlib.h>
#include <assert.h>


namespace Platform {

AudioDispatch::AudioDispatch() : resetTimestamp_(true), sampleCount_(0)
{
}

AudioDispatch::~AudioDispatch()
{
}

void AudioDispatch::addEndpoint( AudioEndpoint& ep )
{
    mtx.lock();
    if ( audioEndpoints_.size() == 1 )
    {
        /* switching to timestamp stream */
        resetTimestamp_ = true;
    }

    audioEndpoints_.insert( &ep );
    /*for(AudioEndpointContainer::const_iterator it = audioEndpoints_.begin(); it != audioEndpoints_.end(); ++it)
    {
        log(LOG_DEBUG) << (*it)->getId();
    }*/
    mtx.unlock();
}

void AudioDispatch::removeEndpoint( AudioEndpoint& ep )
{
    mtx.lock();
    audioEndpoints_.erase( &ep );

/*    for(AudioEndpointContainer::const_iterator it = audioEndpoints_.begin(); it != audioEndpoints_.end(); ++it)
    {
        log(LOG_DEBUG) << (*it)->getId();
    }*/
    mtx.unlock();
}


int AudioDispatch::enqueueAudioData(unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples)
{
    int n = 0;
    int canAccept = 0;

    mtx.lock();
    if( !audioEndpoints_.empty() )
    {
        canAccept = nsamples;
        for(AudioEndpointContainer::const_iterator it = audioEndpoints_.begin(); it != audioEndpoints_.end(); ++it)
        {
            canAccept = (*it)->canAcceptSamples( canAccept, rate );
        }
    }

    if ( canAccept > 0 )
    {
        uint32_t timestamp = 0;

        if ( resetTimestamp_ )
        {
            unsigned int highestQueuedSamples = 0;
            for(AudioEndpointContainer::const_iterator it = audioEndpoints_.begin(); it != audioEndpoints_.end(); ++it)
            {
                unsigned int queuedSamples = (*it)->getNumberOfQueuedSamples();
                if ( queuedSamples > highestQueuedSamples ) 
                    highestQueuedSamples = queuedSamples;
            }
            timestampBase_ = getTick_ms() + ( highestQueuedSamples * 1000 / rate ) + 10;
            sampleCount_ = 0;
            resetTimestamp_ = false;
        }

        if(audioEndpoints_.size() > 1)
        {
            /* more than one stream, play by timestamp */
            timestamp = timestampBase_ + ((uint64_t)sampleCount_ * 1000 / rate);

            if ( timestamp < getTick_ms() )
            {
                resetTimestamp_ = true;
                mtx.unlock();
                return 0;
            }
        }
        else
        {
            /* only one stream, play whenever */
            timestamp = 0;
        }

        for(AudioEndpointContainer::const_iterator it = audioEndpoints_.begin(); it != audioEndpoints_.end(); ++it)
        {
            n = (*it)->enqueueAudioData( timestamp, channels, rate, canAccept, samples );
            assert( n == canAccept ); /* endpoint must accept all samples, since we asked it before what the maximum it can accept is */
        }

        sampleCount_ += n;
    }

    mtx.unlock();

    return n;
}

void AudioDispatch::flushAudioData()
{
    mtx.lock();
    for(AudioEndpointContainer::const_iterator it = audioEndpoints_.begin(); it != audioEndpoints_.end(); ++it)
    {
        (*it)->flushAudioData();
    }
    mtx.unlock();
}

void AudioDispatch::pause()
{
    mtx.lock();
    for(AudioEndpointContainer::const_iterator it = audioEndpoints_.begin(); it != audioEndpoints_.end(); ++it)
    {
        (*it)->pause();
    }
    mtx.unlock();
}

void AudioDispatch::resume()
{
    mtx.lock();
    for(AudioEndpointContainer::const_iterator it = audioEndpoints_.begin(); it != audioEndpoints_.end(); ++it)
    {
        (*it)->resume();
    }
    resetTimestamp_ = true;
    mtx.unlock();
}


}
