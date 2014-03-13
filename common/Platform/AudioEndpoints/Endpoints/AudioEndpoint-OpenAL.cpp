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

#include "../AudioEndpointLocal.h"
#include "Platform/Utils/Utils.h"
#include "applog.h"
#include <Windows.h>
#include <iostream>
#include <al.h>
#include <alc.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>

namespace Platform {


#define NUM_BUFFERS 3

AudioEndpointLocal::AudioEndpointLocal(const ConfigHandling::AudioEndpointConfig& config) : Platform::Runnable(true, SIZE_SMALL, PRIO_HIGH), 
                                                                                            config_(config), 
                                                                                            adjustSamples_(0)
{
    startThread();
}
AudioEndpointLocal::~AudioEndpointLocal()
{
}

void AudioEndpointLocal::destroy()
{
    cancelThread();
    joinThread();
    flushAudioData();
}

#include <assert.h>
void AudioEndpointLocal::run()
{
    AudioFifoData* afd = NULL;
    unsigned int frame = 0;
    ALCdevice *device = NULL;
    ALCcontext *context = NULL;
    ALuint buffers[NUM_BUFFERS];
    ALuint source;
    ALint processed;
    ALint curbuffers;
    ALint state;
    ALenum error;
    ALint rate;
    ALint prevrate = -1;
//    ALint channels;
    ALint prevchannels = -1;
    unsigned int bufferedSamples[NUM_BUFFERS] = {0};
    int i;

    while(isCancellationPending() == false)
    {
        device = alcOpenDevice(NULL); /* Use the default device */
        if (!device)
        {
            log(LOG_EMERG) << "failed to open device";
            Sleep(1000);
            continue;
        }

        context = alcCreateContext(device, NULL);
        alcMakeContextCurrent(context);
        alListenerf(AL_GAIN, 1.0f);
        alDistanceModel(AL_NONE);
        alGenBuffers((ALsizei)NUM_BUFFERS, buffers);
        alGenSources(1, &source);

        while(isCancellationPending() == false)
        {
            if ((error = alcGetError(device)) != AL_NO_ERROR) 
            {
                log(LOG_EMERG) << "openal al error: " << error;
                break;
            }

            alGetSourcei(source, AL_BUFFERS_QUEUED, &curbuffers);
            alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed); /*returns the number of buffer entries already processed*/

            if ( curbuffers == NUM_BUFFERS )
            {
                /* if buffer is fully loaded we wait for some frame to finish */
                if (!processed)
                {
                    if ((error = alcGetError(device)) != AL_NO_ERROR) 
                    {
                        log(LOG_EMERG) << "openal al error: " << error;
                        //exit(1);
                    }

                    Sleep(1);
                    continue;
                }

                /* Remove old audio from the queue.. */
                alSourceUnqueueBuffers(source, 1, &buffers[frame]);
                //log(LOG_DEBUG) << "processed " << processed;
            }

            if ( paused_ )
            {
                alSourcePause(source);
                Sleep(10);
                continue;
            }

            /* check if there's more audio available */
            if ( ( afd = fifo_.getFifoDataTimedWait(1) ) == NULL )
                continue;

            if (prevrate != -1 && prevchannels != -1 && (afd->rate != prevrate || afd->channels != prevchannels) )
            {
                /* Format or rate changed, so we need to reset all buffers */
                alSourcei(source, AL_BUFFER, 0);
                alSourceStop(source);
                frame = 0;
            }
            prevrate = afd->rate;
            prevchannels = afd->channels;

            bufferedSamples[frame] = 0;

            if ( afd->timestamp != 0 )
            {
#define NSAMPLES 3
                static double timetoplaysamples[NSAMPLES] = {0};
                static uint8_t ntimetoplaysamples = 0;
                static uint8_t j = 0;
                unsigned int now = getTick_ms();
                int timeToPlayThisPacket = afd->timestamp - now;
                ALint totalBufferedSamples = 0;
                ALint offsetInCurrentBuffer;
                for ( i=0; i<NUM_BUFFERS; i++ )
                    totalBufferedSamples += bufferedSamples[i];
                alGetSourcei(source, AL_SAMPLE_OFFSET,  &offsetInCurrentBuffer);
                assert( offsetInCurrentBuffer <= totalBufferedSamples );
                totalBufferedSamples -= offsetInCurrentBuffer;
                int timeToBufferUnderrun = (totalBufferedSamples * 1000 / prevrate);
                int offset = timeToPlayThisPacket - timeToBufferUnderrun;
                
                timetoplaysamples[j] = offset;
                j=(j+1)%NSAMPLES;
                if(ntimetoplaysamples < NSAMPLES) ntimetoplaysamples++;

                double timetoplay = 0;
                for ( uint8_t i=0; i<ntimetoplaysamples; i++ )
                {
                    timetoplay += timetoplaysamples[i];
                }
                timetoplay /= ntimetoplaysamples;


                /*static int timetoplay = timeToPlayThisPacket;
                timetoplay += (timeToPlayThisPacket - timetoplay)/3;*/

                if ( timetoplay > 25 )
                {
                    sleep_ms( timetoplay-10 );
                    ntimetoplaysamples = 0;
                    j=0;
                }
                else if ( timetoplay < -25 )
                {
                    fifo_.returnFifoDataBuffer( afd );
                    ntimetoplaysamples = 0;
                    j=0;
                    continue;
                }
                else if ( timetoplay < -1 || timetoplay > 1 )
                {
                    //we're off, adjust playback
                    adjustSamples_ = timetoplay * (int)afd->rate / 1000;
                    for ( uint8_t i=0; i<ntimetoplaysamples; i++ )
                    {
                        timetoplaysamples[i] -= timetoplay;
                    }
                }
                else if ( timetoplay == 0 )
                {
                    adjustSamples_ = 0;
                }
            }
            if ( adjustSamples_ != 0 )
            {
                adjustSamples( afd );
            }

            alBufferData(buffers[frame],
                    afd->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
                            afd->samples,
                            afd->nsamples * afd->channels * sizeof(short),
                            afd->rate);

            alSourceQueueBuffers(source, 1, &buffers[frame]);
            bufferedSamples[frame] = afd->nsamples;
            frame++;
            frame = frame % NUM_BUFFERS;

            fifo_.returnFifoDataBuffer( afd );
            afd = NULL;

            alGetSourcei(source, AL_SOURCE_STATE, &state);
            alGetSourcei(source, AL_BUFFERS_QUEUED, &curbuffers);
            alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
            if (state == AL_STOPPED || state == AL_INITIAL)
            {
                log(LOG_DEBUG) << "Stopped, curbuffers = " << curbuffers << ", processed = " << processed;
            }
            /* start playback when all buffers are ready. player will also stop when it runs out of buffers so it has to be restarted */
            if ((state != AL_PLAYING) && curbuffers == NUM_BUFFERS && processed == 0)
            {
                log(LOG_DEBUG) << "Starting playback";
                alSourcePlay(source);
            }
        }
    }

    log(LOG_DEBUG) << "Exiting AudioEndpoint::run()";
}
}


