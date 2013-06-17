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
#include "applog.h"
#include <stdlib.h>


namespace Platform {

AudioDispatch::AudioDispatch()
{
}

AudioDispatch::~AudioDispatch()
{
}

void AudioDispatch::addEndpoint( AudioEndpoint& ep )
{
    mtx.lock();
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

    mtx.lock();
    /* TODO: This is really dangerous, we should sync all endpoints to the same rate
     * letting the slowest one be the dictator */
    for(AudioEndpointContainer::const_iterator it = audioEndpoints_.begin(); it != audioEndpoints_.end(); ++it)
    {
        /*
        newN = (*it)->enqueueAudioData(format->channels, format->sample_rate, num_frames, static_cast<const int16_t*>(frames));
        if((*it)->isLocal())
            n = newN;*/

        /* For now automatically disable local endpoint if we have any remote endpoints.
         * In the future it shall be possible for clients to disable local endpoints */
        if(audioEndpoints_.size() > 1)
        {
           /* if ( nsamples > rate/100)
                nsamples = rate/100;*/

            if(!(*it)->isLocal())
                n = (*it)->enqueueAudioData(channels, rate, nsamples, samples);
        }
        else
        {
            n = (*it)->enqueueAudioData(channels, rate, nsamples, samples);
        }
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
    mtx.unlock();
}


}
