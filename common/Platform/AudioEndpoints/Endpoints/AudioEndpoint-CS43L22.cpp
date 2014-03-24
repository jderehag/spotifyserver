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

#include "../AudioEndpointLocal.h"
#include "Platform/Utils/Utils.h"
#include "stm32f4_discovery.h"
#include <string.h>

#include "audio_driver.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "applog.h"

#ifdef DEBUG_COUNTERS
int lasttimetoplay = 0;
int lasttimestamp = 0;
int lastmissingsamples = 0;
int lastpacketsamples = 0;
int lastpacketms = 0;
int lastpadsamples = 0;
int totalpadsamples = 0;
int totalsamples = 0;
int firsttimestamp = 0;
int totaltimems = 0;
int totalservertimems = 0;
#endif
namespace Platform {

#define SAMPLES_PER_BUFFER (400 * 2)
typedef struct
{
    AudioFifoData header;
    int16_t samples[SAMPLES_PER_BUFFER];
} audioBuffer_t;
#define NOF_AUDIO_BUFFERS 34
static audioBuffer_t fifobuffers[NOF_AUDIO_BUFFERS] __attribute__ ((section (".audio_buffers")));
static int16_t driverBuffers [2][SAMPLES_PER_BUFFER+2]; // + 2 for possible padding in driver...
static unsigned int bufferedSamples[2] = {0};

#define BUFFER1 ( 1 << 0 )
#define BUFFER2 ( 1 << 1 )
EventGroupHandle_t xAvailableBuffers;

static void AudioCallbackFromISR(void *context __attribute__((unused)),int buffer);

AudioEndpointLocal::AudioEndpointLocal(const ConfigHandling::AudioEndpointConfig& config) : AudioEndpoint(165, false),
                                                                                            Platform::Runnable(false, SIZE_SMALL, PRIO_VERY_HIGH),
                                                                                            config_(config),
                                                                                            adjustSamples_(0),
                                                                                            actualVolume_(0)
{
    startThread();
}

AudioEndpointLocal::~AudioEndpointLocal()
{
}

void AudioEndpointLocal::destroy()
{
}

void AudioEndpointLocal::run()
{
    AudioFifoData* afd = NULL;
    unsigned int currentrate = 0;
    unsigned int i;
    bool isPlaying = false;
    uint8_t lastVolume = 0;

    for ( i = 0; i < NOF_AUDIO_BUFFERS; i++)
    {
        fifobuffers[i].header.bufferSize = sizeof(fifobuffers[i].samples);
        fifo_.returnFifoDataBuffer((AudioFifoData*)(&fifobuffers[i]));
    }

    xAvailableBuffers = xEventGroupCreate();
    xEventGroupSetBits( xAvailableBuffers, BUFFER1 | BUFFER2 );

    SetAudioVolume(0);

    while(isCancellationPending() == false)
    {
        do
        {
            /* check if there's more audio available */
            afd = fifo_.getFifoDataBlocking();
        } while( afd == NULL && isCancellationPending() == false );

        if ( afd != NULL )
        {
            uint8_t thisBufferNum;
            EventBits_t avail;

            if ( actualVolume_ != lastVolume )
            {
                SetAudioVolume(actualVolume_);
                lastVolume = actualVolume_;
            }

            if ( !isPlaying )
            {
                EnableAudio(Audio44100HzSettings, AudioCallbackFromISR, NULL);
                isPlaying = true;
            }

            avail = xEventGroupWaitBits( xAvailableBuffers, BUFFER1 | BUFFER2, pdFALSE, pdFALSE, portMAX_DELAY );
            if ( avail & BUFFER1 )
            {
                thisBufferNum = 0;
            }
            else
            {
                thisBufferNum = 1;
            }
            xEventGroupClearBits( xAvailableBuffers, ( 1 << (thisBufferNum) ) );

            if ( afd->timestamp != 0 )
            {
                unsigned int now = getTick_ms();
                int timeToPlayThisPacket = afd->timestamp - now;
                int timeToBufferUnderrun = bufferedSamples[thisBufferNum ^ 1] * 1000 / afd->rate;
                int offset = timeToPlayThisPacket - timeToBufferUnderrun;

                /*static*/ int timetoplay = offset;
                //timetoplay += (timeToPlayThisPacket - timetoplay)/3;

                if ( timetoplay < -25 || timetoplay > 500 /*this has to be some error*/ )
                {
                    //log( LOG_NOTICE ) << "discarding packet " << timetoplay;
                    fifo_.returnFifoDataBuffer( afd );
                    xEventGroupSetBits( xAvailableBuffers, ( 1 << (thisBufferNum) ) );
                    continue;
                }
                else if ( timetoplay > 25 )
                {
                    //log( LOG_NOTICE ) << "Received packet way too early " << timetoplay;
                    vTaskDelay( timetoplay );
                }
                else if ( timetoplay < -1 || timetoplay > 1 )
                {
                    //we're off, adjust playback
                    adjustSamples_ = timetoplay * (int)afd->rate / 1000;
                }
                else if ( timetoplay == 0 )
                {
                    adjustSamples_ = 0;
                }

#ifdef DEBUG_COUNTERS
                if ( firsttimestamp == 0 ) firsttimestamp = now;
                totalservertimems = afd->timestamp - firsttimestamp;
                totaltimems = now - firsttimestamp;
                lastpacketsamples = afd->nsamples;
                lastpacketms = afd->timestamp - lasttimestamp;
                lasttimetoplay = timetoplay;
                lasttimestamp = afd->timestamp;
                lastmissingsamples = adjustSamples_;
#endif
            }
            else /* not playing by timestamp */
            {
                /* if we're risking buffer underrun, play a little slower... */
                /* we should fill up buffer at the beginning of each track as well,
                 * but this is just to avoid buffer underrun during the track */
                if ( fifo_.getNumberOfQueuedSamples() < afd->rate / 50 )
                {
                    adjustSamples_ = 44;
                }
            }

            if ( adjustSamples_ != 0 )
            {
                BSP_LED_On(LED4);
                adjustSamples( afd );
            }
            else
                BSP_LED_Off(LED4);

            /* Copy buffer to the driver buffer so we can free the afd */
            if ( afd->channels == 1 )
            {
                //todo, test this... don't know any mono tracks
                for ( i = 0; i < (afd->nsamples); i++ )
                {
                    driverBuffers[thisBufferNum][i*2] = afd->samples[i];
                    driverBuffers[thisBufferNum][i*2+1] = afd->samples[i];
                }
            }
            else
            {
                if ( afd->channels != 2 )
                    log( LOG_WARN ) << "Unsupported number of channels: " << afd->channels;
                memcpy( driverBuffers[thisBufferNum], afd->samples, afd->nsamples*afd->channels*sizeof(int16_t) );
            }

            bufferedSamples[thisBufferNum] = afd->nsamples;
#ifdef DEBUG_COUNTERS
            totalsamples += nsamples/2;
#endif

            if ( !ProvideAudioBufferWithoutBlocking( driverBuffers[thisBufferNum], afd->nsamples * 2 ) )
            {
                log( LOG_EMERG ) << "failed to provide buffer to audio driver!";
            }
            fifo_.returnFifoDataBuffer( afd );
        }
    }
}


static void AudioCallbackFromISR(void *context __attribute__((unused)),int buffer)
{
    portBASE_TYPE xHigherPriorityTaskWoken;
    bufferedSamples[buffer] = 0;
    xEventGroupSetBitsFromISR( xAvailableBuffers, ( 1 << (buffer) ), &xHigherPriorityTaskWoken );

    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

}
