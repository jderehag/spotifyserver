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
#include "stm32f4_discovery.h"

extern "C"
{
#include "audio_driver.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
}
namespace Platform {

typedef struct
{
    AudioFifoData header;
    int16_t samples[400 * 2];
} audioBuffer_t;

static audioBuffer_t fifobuffers[40] __attribute__ ((section (".audio_buffers")));
static int16_t driverBuffers [2][400*2];
static uint8_t bufferNumber = 0;

//xQueueHandle xQueuedBuffers;
xSemaphoreHandle xSem;


static void AudioCallback(void *context,int buffer);

AudioEndpointLocal::AudioEndpointLocal(const ConfigHandling::AudioEndpointConfig& config) : AudioEndpoint(false),
                                                                                            Platform::Runnable(false, SIZE_SMALL, PRIO_HIGH),
                                                                                            config_(config)
{
    startThread();
}

AudioEndpointLocal::~AudioEndpointLocal()
{
}

void AudioEndpointLocal::destroy()
{
}


int AudioEndpointLocal::enqueueAudioData(unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples)
{
    int n;
    n = fifo_.addFifoDataBlocking(channels, rate, nsamples, samples);
    return n;
}

void AudioEndpointLocal::flushAudioData()
{
    fifo_.flush();
}


void AudioEndpointLocal::run()
{
    AudioFifoData* afd = NULL;
    unsigned int currentrate = 0;
    unsigned int i;

    for ( i = 0; i < 40; i++)
    {
        fifo_.returnFifoDataBuffer((AudioFifoData*)(&fifobuffers[i]));
    }

//    xQueuedBuffers = xQueueCreate( 2, sizeof(AudioFifoData*) );
    xSem = xSemaphoreCreateCounting( 2, 1 );

    InitializeAudio(Audio44100HzSettings);
    SetAudioVolume(160);
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

            STM_EVAL_LEDOff( LED6 );
            xSemaphoreTake( xSem, portMAX_DELAY );
            thisBufferNum = bufferNumber; /* bufferNumber is the last buffer that was requested by driver, it has to be unused*/

            /* Extra copy because DMA can't access the CCM RAM where fifobuffers are placed.
             * TODO: Should probably place FreeRTOS stacks there instead */
            for ( i=0; i<afd->nsamples*2; i++ )
                driverBuffers[thisBufferNum][i] = afd->samples[i];

            if ( ProvideAudioBufferWithoutBlocking( driverBuffers[thisBufferNum], afd->nsamples*2 ) )
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
    xSemaphoreGiveFromISR(xSem, &xHigherPriorityTaskWoken );
    //STM_EVAL_LEDToggle( LED3 );
    if ( xHigherPriorityTaskWoken )
    {
        vPortYieldFromISR();
    }
}

}
