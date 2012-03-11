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

#include "../AudioEndpoint.h"
#include "applog.h"
#include <Windows.h>
#include <iostream>
#include <al.h>
#include <alc.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
/*#include <unistd.h>
#include <sys/time.h>*/

namespace Platform {

static int queue_buffer(ALuint source, AudioFifoData *af, ALuint buffer);

#define NUM_BUFFERS 3

AudioEndpoint::AudioEndpoint(const ConfigHandling::AudioEndpointConfig& config) : config_(config)
{
	startThread();
}
AudioEndpoint::~AudioEndpoint()
{
}

void AudioEndpoint::destroy()
{
	cancelThread();
	joinThread();
	flushAudioData();
}


int AudioEndpoint::enqueueAudioData(unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples)
{
	return fifo.addFifoDataBlocking(channels, rate, nsamples, samples);
}

void AudioEndpoint::flushAudioData()
{
	fifo.flush();
}


void AudioEndpoint::run()
{
    AudioFifoData* afd;
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
    ALint channels;

    device = alcOpenDevice(NULL); /* Use the default device */
    if (!device) log(LOG_EMERG) << "failed to open device";

    do
    {
        device = alcOpenDevice(NULL); /* Use the default device */
        if (!device) 
            log(LOG_EMERG) << "failed to open device";
        Sleep(1000);
    } while ( device == NULL && isCancellationPending() == false );

    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);
    alListenerf(AL_GAIN, 1.0f);
    alDistanceModel(AL_NONE);
    alGenBuffers((ALsizei)NUM_BUFFERS, buffers);
    alGenSources(1, &source);

    while(isCancellationPending() == false)
    {
        //int cnt = 0;

        /* First prebuffer some audio */
        if((afd = fifo.getFifoDataTimedWait(1)) == 0)
            continue;

        queue_buffer(source, afd, buffers[frame]);
        frame++;

        if (frame < NUM_BUFFERS)
            continue;

        frame = 0; /*ready to start playing, reset frame counter*/
        alSourcePlay(source);
        while(isCancellationPending() == false)
        {
            /* Wait for some audio to play */
//            alGetSourcei(source, AL_BUFFERS_QUEUED, &curbuffers);
            alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed); /*returns the number of buffer entries already processed*/

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
            alSourceUnqueueBuffers(source, 1, &buffers[frame % NUM_BUFFERS]);
            //log(LOG_DEBUG) << "processed " << processed;

            /* wait for some more audio */
            while((afd = fifo.getFifoDataTimedWait(1)) == NULL && isCancellationPending() == false);

            if( isCancellationPending() == true )
                break;

            //log(LOG_DEBUG) << "loading new buffer " << frame;
            alGetBufferi(buffers[frame % NUM_BUFFERS], AL_FREQUENCY, &rate);
            alGetBufferi(buffers[frame % NUM_BUFFERS], AL_CHANNELS, &channels);
            if (afd->rate != rate || afd->channels != channels)
            {
                log(LOG_DEBUG) << "rate or channel count changed, resetting";
                break;
            }
            alBufferData(buffers[frame % NUM_BUFFERS],
                    afd->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
                            afd->samples,
                            afd->nsamples * afd->channels * sizeof(short),
                            afd->rate);

            alSourceQueueBuffers(source, 1, &buffers[frame % NUM_BUFFERS]);

            delete afd;
            afd = NULL;

            if ((error = alcGetError(device)) != AL_NO_ERROR) 
            {
                log(LOG_EMERG) << "openal al error: " << error;
                //exit(1);
            }

            alGetSourcei(source, AL_SOURCE_STATE, &state);
            if (state == AL_STOPPED && processed == 1) /* if stopped and we have just refilled last buffer*/
            {
                /* player will stop when it runs out of buffers so it has to be restarted */
                log(LOG_DEBUG) << "Restarting playback";
                alSourcePlay(source);
            }

            frame++;
        }

        if( isCancellationPending() == true )
            break;

        /* Format or rate changed, so we need to reset all buffers */
        alSourcei(source, AL_BUFFER, 0);
        alSourceStop(source);

        /* Make sure we don't lose the audio packet that caused the change */
        alBufferData(buffers[0],
                     afd->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
                     afd->samples,
                     afd->nsamples * afd->channels * sizeof(short),
                     afd->rate);

        alSourceQueueBuffers(source, 1, &buffers[0]);

        delete afd;
        afd = NULL;

        frame = 1;
    }

	log(LOG_DEBUG) << "Exiting AudioEndpoint::run()";
}

static int queue_buffer(ALuint source, AudioFifoData* afd, ALuint buffer)
{
    alBufferData(buffer,
         afd->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
         afd->samples,
         afd->nsamples * afd->channels * sizeof(short),
         afd->rate);
    alSourceQueueBuffers(source, 1, &buffer);
    free(afd);
    return 1;
}

}


