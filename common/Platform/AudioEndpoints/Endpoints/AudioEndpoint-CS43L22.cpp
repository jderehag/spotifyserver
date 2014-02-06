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
#include "applog.h"

extern "C"
{
#include "audio_driver.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
}

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

static audioBuffer_t fifobuffers[40] __attribute__ ((section (".audio_buffers")));
static int16_t driverBuffers [2][SAMPLES_PER_BUFFER+2]; // + 2 for possible padding in driver...
static uint8_t bufferNumber = 0;
static unsigned int bufferedSamples[2] = {0};

//xQueueHandle xQueuedBuffers;
xSemaphoreHandle xSem;

static void AudioCallback(void *context,int buffer);

AudioEndpointLocal::AudioEndpointLocal(const ConfigHandling::AudioEndpointConfig& config) : AudioEndpoint(false),
                                                                                            Platform::Runnable(false, SIZE_SMALL, PRIO_HIGH),
                                                                                            config_(config),
                                                                                            missingSamples_(0)
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

    for ( i = 0; i < 40; i++)
    {
        fifobuffers[i].header.bufferSize = sizeof(fifobuffers[i].samples);
        fifo_.returnFifoDataBuffer((AudioFifoData*)(&fifobuffers[i]));
    }

//    xQueuedBuffers = xQueueCreate( 2, sizeof(AudioFifoData*) );
    xSem = xSemaphoreCreateCounting( 2, 1 );

    InitializeAudio(Audio44100HzSettings);
    SetAudioVolume(170);
    PlayAudioWithCallback(AudioCallback, NULL);

    while(isCancellationPending() == false)
    {
        do
        {
            /* check if there's more audio available */
            afd = fifo_.getFifoDataTimedWait(1);
        } while( afd == NULL && isCancellationPending() == false );

        if ( afd != NULL )
        {
            uint8_t thisBufferNum;
            uint32_t nsamples;

            xSemaphoreTake( xSem, portMAX_DELAY );
            thisBufferNum = bufferNumber; /* bufferNumber is the last buffer that was requested by driver, it has to be unused */

            /* Extra copy because DMA can't access the CCM RAM where fifobuffers are placed.
             * TODO: Should probably place FreeRTOS stacks there instead */
            for ( i = 0; i < (afd->nsamples * 2); i++ )
                driverBuffers[thisBufferNum][i] = afd->samples[i];

            nsamples = afd->nsamples * 2;

            if ( afd->timestamp != 0 )
            {
                unsigned int now = getTick_ms();
                int timeToPlayThisPacket = afd->timestamp - now;
                timeToPlayThisPacket -= bufferedSamples[thisBufferNum ^ 1] * 1000 / afd->rate;

                /*static*/ int timetoplay = timeToPlayThisPacket;
                //timetoplay += (timeToPlayThisPacket - timetoplay)/3;

                if ( timetoplay > 25 )
                {
                    vTaskDelay( timetoplay );
                }
                else if ( timetoplay < -25 )
                {
                    fifo_.returnFifoDataBuffer( afd );
                    xSemaphoreGive( xSem );
                    continue;
                }
                else if ( timetoplay < -3 )
                {
                    //we're late, drop a few samples off this packet
                    if ( nsamples > 10 ) nsamples -= 4;

                    //whatever we had here, no need to pad now
                    missingSamples_ = 0;
                }
                else if ( timetoplay > 3 )
                {
                    //we're early, slow down by adding fake missing samples
                    missingSamples_ = timetoplay * afd->rate / 1000;
                }

#ifdef DEBUG_COUNTERS
                if ( firsttimestamp == 0 ) firsttimestamp = now;
                totalservertimems = afd->timestamp - firsttimestamp;
                totaltimems = now - firsttimestamp;
                lastpacketsamples = afd->nsamples;
                lastpacketms = afd->timestamp - lasttimestamp;
                lasttimetoplay = timetoplay;
                lasttimestamp = afd->timestamp;
                lastmissingsamples = missingSamples_;
#endif
            }
            else /* not playing by timestamp */
            {
                /* if we're risking buffer underrun, play a little slower... */
                /* we should fill up buffer at the beginning of each track as well,
                 * but this is just to avoid buffer underrun during the track */
                if ( fifo_.getNumberOfQueuedSamples() < afd->rate / 50 )
                {
                    missingSamples_ = 44;
                }
            }

            if ( missingSamples_ )
            {
                uint32_t headroom = afd->bufferSize - (afd->nsamples*afd->channels*sizeof(int16_t)); // in bytes
                uint32_t padrate = (128 * (missingSamples_ + 2*afd->rate)) / (2*afd->rate); // approx percentage (or rather per-128-age for simpler calculation) that needs to be extended to catch up in 2 seconds
                padrate -= 128;
                uint32_t padsamples = padrate * afd->nsamples / 128; // number of samples this buffer should be padded with to keep up
                //if ( padsamples > 5 ) padsamples = 5; //cap so we don't sacrifice too much on sound quality
                if ( padsamples == 0 ) padsamples = 1;
                if ( padsamples > missingSamples_ ) padsamples = missingSamples_;
                if ( padsamples > headroom/4 ) padsamples = headroom/4;
                STM_EVAL_LEDOn(LED4);
                for ( i=0; i<padsamples; i++ )
                {
                    driverBuffers[thisBufferNum][nsamples] = driverBuffers[thisBufferNum][nsamples-2];
                    nsamples++;
                    driverBuffers[thisBufferNum][nsamples] = driverBuffers[thisBufferNum][nsamples-2];
                    nsamples++;
                    missingSamples_--;
                }
#ifdef DEBUG_COUNTERS
                lastpadsamples = padsamples;
                totalpadsamples += padsamples;
#endif
            }
            else
                STM_EVAL_LEDOff(LED4);

            bufferedSamples[thisBufferNum] = nsamples/2;
#ifdef DEBUG_COUNTERS
            totalsamples += nsamples/2;
#endif

            if ( ProvideAudioBufferWithoutBlocking( driverBuffers[thisBufferNum], nsamples ) )
            {
                fifo_.returnFifoDataBuffer( afd );
            }
            else
                while(1); /* hang to catch error, we should not be able to take the semaphore if we're not allowed to provide data */
        }
    }
}


/* big TODO! this function isn't always called from interrupt! */

static void AudioCallback(void *context __attribute__((unused)),int buffer)
{
    portBASE_TYPE xHigherPriorityTaskWoken;
    bufferNumber = buffer;
    bufferedSamples[buffer] = 0;
    xSemaphoreGiveFromISR(xSem, &xHigherPriorityTaskWoken );

    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

}
