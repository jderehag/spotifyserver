/*
 * Copyright (c) 2012, Jesper Derehag
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
 * DISCLAIMED. IN NO EVENT SHALL JESPER DEREHAG BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "AudioFifo.h"
#include <iostream>
#include <string.h>
#include <time.h>

namespace Platform {

AudioFifo::AudioFifo(unsigned int bufferNSecs, bool isDynamic) : queuedSamples_(0),
                                                                 bufferNSecs_(bufferNSecs),
                                                                 isDynamic_(isDynamic) 
{ }

AudioFifo::AudioFifo(bool isDynamic) : queuedSamples_(0),
                                       bufferNSecs_(1),
                                       isDynamic_(isDynamic) 
{ }

AudioFifo::~AudioFifo() {flush();}

/* JESPER: Not very efficient at all!!! Fix someday... */
int AudioFifo::addFifoDataBlocking(unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples)
{
    size_t sampleLength = 0;

    if (nsamples == 0)
        return 0; // Audio discontinuity, do nothing

    /* Buffer one second of audio */
    /* JESPER, queue here was probably previously defined as 1 second worth of static queue length,
     * investigate if we should have a const size queue instead */
    /* JENS, reinserted this throttle to avoid fifo growing really big*/
    fifoMtx_.lock();
    if (queuedSamples_ > (rate * bufferNSecs_))
    {
        fifoMtx_.unlock();
        return 0;
    }
    //fifoMtx_.unlock();

    sampleLength = nsamples * sizeof(int16_t) * channels;
    AudioFifoData* data = getFifoDataBuffer(sampleLength);

    if ( data == NULL )
    {
        fifoMtx_.unlock();
        return 0;
    }

    memcpy(data->samples, samples, sampleLength);

    data->nsamples = nsamples;
    data->rate = rate;
    data->channels = channels;

    //fifoMtx_.lock();
    fifo_.push(data);
    queuedSamples_ += nsamples;
    cond_.signal();
    fifoMtx_.unlock();

    return nsamples;
}

AudioFifoData* AudioFifo::getFifoDataBlocking()
{
	AudioFifoData* afd = 0;
	fifoMtx_.lock();
	while(fifo_.empty())cond_.wait(fifoMtx_);
	afd = fifo_.front();
    queuedSamples_ -= afd->nsamples;
	fifo_.pop();
	fifoMtx_.unlock();
	return afd;
}

AudioFifoData* AudioFifo::getFifoDataTimedWait(unsigned int milliSeconds)
{
	AudioFifoData* afd = 0;
	fifoMtx_.lock();
	if(fifo_.empty() == false)
	{
		afd = fifo_.front();
        queuedSamples_ -= afd->nsamples;
		fifo_.pop();
	}
	else afd = 0; /* redundant, but kept for clarity */

	if(afd == 0)
	{
		cond_.timedWait(fifoMtx_, milliSeconds);
		if(fifo_.empty() == false)
		{
			afd = fifo_.front();
            queuedSamples_ -= afd->nsamples;
            fifo_.pop();
        }
	}

	fifoMtx_.unlock();
	return afd;
}

void AudioFifo::flush()
{
    fifoMtx_.lock();
    AudioFifoData* ep = 0;
    while(!fifo_.empty())
    {
        ep = fifo_.front();
        returnFifoDataBuffer(ep);
        ep = 0;
        fifo_.pop();
    }
    queuedSamples_ = 0;
    fifoMtx_.unlock();
}

void AudioFifo::setFifoBuffer(unsigned int bufferNSecs) { bufferNSecs_ = bufferNSecs; };

void AudioFifo::returnFifoDataBuffer(AudioFifoData* afd)
{
    if ( isDynamic_ )
        free(afd);
    else
        bufferPool.push_back(afd);
}

AudioFifoData* AudioFifo::getFifoDataBuffer(size_t sampleLength)
{
    AudioFifoData* buffer = NULL;
    if ( isDynamic_ )
    {
        buffer = static_cast<AudioFifoData*>(malloc(sizeof(AudioFifoData) + sampleLength));
    }
    else
    {
        /* todo should keep track of buffer sizes and make sure sampleLength doesn't exceed */
        if ( !bufferPool.empty() )
            buffer = bufferPool.pop_front();
    }
    return buffer;
}


}



