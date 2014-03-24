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

#ifndef AUDIOENDPOINT_H_
#define AUDIOENDPOINT_H_

#include "AudioFifo.h"
#include <string>

namespace Platform {

class AudioEndpoint
{

protected:
    AudioFifo fifo_;

    bool paused_;

    uint8_t masterVolume_;
    uint8_t relativeVolume_;

public:
    AudioEndpoint(uint8_t volume = 255, bool dynamicFifo = true) : fifo_(dynamicFifo), paused_(false), relativeVolume_(volume) {}
    virtual ~AudioEndpoint() {}
    virtual int enqueueAudioData( unsigned int timestamp, unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples ) = 0;
    virtual void flushAudioData() = 0;

    virtual unsigned int canAcceptSamples( unsigned int availableSamples, unsigned int rate ) { return fifo_.canAcceptSamples( availableSamples, rate ); }
    virtual unsigned int getNumberOfQueuedSamples() = 0;

    virtual void setMasterVolume( uint8_t volume ) = 0;
    virtual void setRelativeVolume( uint8_t volume ) = 0;
    uint8_t getRelativeVolume() { return relativeVolume_; }
    virtual std::string getId() const = 0;

    /*todo do something proper with these...*/
    void pause() { paused_ = true; }
    void resume() { paused_ = false; }

    virtual bool isLocal() const = 0;

};
}
#endif /* AUDIOENDPOINT_H_ */
