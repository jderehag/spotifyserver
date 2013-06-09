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
#include <stdlib.h>

extern "C"
{
#include "stm32f4_discovery_audio_codec.h"
#include "FreeRTOS.h"
#include "semphr.h"
}
namespace Platform {

xSemaphoreHandle xSemaphore;

AudioEndpointLocal::AudioEndpointLocal(const ConfigHandling::AudioEndpointConfig& config) : Platform::Runnable(false, SIZE_SMALL, PRIO_HIGH), config_(config)
{
    vSemaphoreCreateBinary( xSemaphore );

    /* Initialize I2S interface */
    EVAL_AUDIO_SetAudioInterface(AUDIO_INTERFACE_I2S);

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
    STM_EVAL_LEDToggle( LED3 );
    return fifo_.addFifoDataBlocking(channels, rate, nsamples, samples);
}

void AudioEndpointLocal::flushAudioData()
{
    fifo_.flush();
}

void AudioEndpointLocal::run()
{
    AudioFifoData* afd = NULL;
    unsigned int currentrate = 0;

    while(isCancellationPending() == false)
    {
        STM_EVAL_LEDToggle( LED6 );
        /* check if there's more audio available */
        if ( ( afd = fifo_.getFifoDataTimedWait(1) ) == NULL )
            continue;

        if ( afd->rate != currentrate )
        {
            /* first data or rate changed */
            currentrate = afd->rate;
            /* Initialize the Audio codec and all related peripherals (I2S, I2C, IOExpander, IOs...) */
            EVAL_AUDIO_Init(OUTPUT_DEVICE_AUTO, 100, currentrate );
        }

        EVAL_AUDIO_Play((uint16_t*)afd->samples, afd->nsamples * afd->channels * sizeof(uint16_t) );
        free( afd );
        xSemaphoreTake( xSemaphore, portMAX_DELAY ); // wait until play complete
    }
}
extern "C"
{
uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{

    return 0;
}

void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)
{
    xSemaphoreGive( xSemaphore );
}

uint32_t Codec_TIMEOUT_UserCallback(void)
{
    /*we should reset something...*/
    return 1;
}
}

}
