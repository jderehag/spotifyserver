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

#ifndef AUDIOFIFO_H_
#define AUDIOFIFO_H_

#include "Platform/Threads/Condition.h"
#include "Platform/Threads/Mutex.h"
#include "Platform/Threads/Messagebox.h"

#include <stdint.h>
#include <stdlib.h>   /* size_t */
#include <queue>

class AudioFifoData
{
public:
    unsigned int timestamp;
    unsigned short channels;
    unsigned int rate;
    unsigned int nsamples;
    size_t bufferSize; // size (in bytes) of this buffer
    int16_t samples[0];
};

namespace Platform {

class AudioFifo
{
private:
    std::queue<AudioFifoData*> fifo_;
    Condition cond_;
    Mutex fifoMtx_;
    Messagebox<AudioFifoData*> bufferPool;
    unsigned int queuedSamples_;
    unsigned int bufferNSecs_;
    const bool isDynamic_;

public:
    AudioFifo(unsigned int bufferNSecs = 1, bool isDynamic = true);
    AudioFifo(bool isDynamic);
    virtual ~AudioFifo();
    AudioFifoData* getFifoDataBuffer( size_t length );
    int addFifoDataBlocking( AudioFifoData* afd );
    int addFifoDataBlocking( unsigned int timestamp, unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples );
    AudioFifoData* getFifoDataBlocking();
    AudioFifoData* getFifoDataTimedWait(unsigned int milliSeconds);
    void setFifoBuffer(unsigned int bufferNSecs);
    void flush();
    unsigned int canAcceptSamples( unsigned int availableSamples, unsigned int rate );
    unsigned int getNumberOfQueuedSamples();

    void returnFifoDataBuffer(AudioFifoData* afd);
};

}


#endif /* AUDIOFIFO_H_ */
