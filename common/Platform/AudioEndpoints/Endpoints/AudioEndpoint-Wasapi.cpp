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
#include <Wmcodecdsp.h>
#include <Mmdeviceapi.h>
#include <Audioclient.h>
#include <Audiopolicy.h>
#include <Mfapi.h>
#include <Mftransform.h>
#include <Mferror.h>
#include <iostream>

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

namespace Platform {

#define HRC(x) do{ hr = x; if( hr != S_OK ) { reset = true; goto cleanup; } } while(0)

AudioEndpointLocal::AudioEndpointLocal(const ConfigHandling::AudioEndpointConfig& config,
                                       const EndpointIdIf& epId ) :  AudioEndpoint( epId ),
                                                                     Runnable(true, SIZE_SMALL, PRIO_HIGH),
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

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const CLSID CLSID_CResamplerMediaObject = __uuidof(CResamplerMediaObject);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_ISimpleAudioVolume = __uuidof(ISimpleAudioVolume);
const IID IID_IAudioSessionControl = __uuidof(IAudioSessionControl);

static IAudioClient* getAudioClient()
{
    IAudioClient *pAudioClient = NULL;

    /* create an 'enumerator' ... */
    IMMDeviceEnumerator *pEnumerator = NULL;
    if ( CoCreateInstance(
            CLSID_MMDeviceEnumerator, NULL,
            CLSCTX_ALL, IID_IMMDeviceEnumerator,
            (void**)&pEnumerator) != S_OK )
    {
        log( LOG_EMERG ) << "Failed creating enumerator";
        return NULL;
    }

    /* ... from which we get the 'default device' ... */
    IMMDevice *pDevice = NULL;
    if ( pEnumerator->GetDefaultAudioEndpoint(
                        eRender, eConsole, &pDevice) != S_OK )
    {
        log( LOG_EMERG ) << "Failed getting default endpoint";
        pEnumerator->Release();
        return NULL;
    }

    /* ... from which we get an 'audio client' */
    if ( pDevice->Activate(
                    IID_IAudioClient, CLSCTX_ALL,
                    NULL, (void**)&pAudioClient) != S_OK )
    {
        log( LOG_EMERG ) << "Failed getting audio client";
        pAudioClient = NULL;
    }

    pEnumerator->Release();
    pDevice->Release();
    return pAudioClient;
}

void AudioEndpointLocal::run()
{
    AudioFifoData* afd = NULL;
    HRESULT hr;
    bool playing = false;
    bool reset = false;
    uint8_t lastVolume = 0;

    /* audio output data structures*/
    IAudioClient *pAudioClient = NULL;
    IAudioRenderClient *pRenderClient = NULL;
    ISimpleAudioVolume * pStreamVolume = NULL;
//    IAudioSessionControl* pSessionControl = NULL; // for callbacks? not implemented yet
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC / 10;
    UINT32 bufferFrameCount;

    /* resampler data structures */
    IMFTransform* pResampler = NULL;
    IMFMediaType* pInputType = NULL; /* resampler input format (spotify format) */
    IMFMediaType* pOutputType = NULL; /* resampler output format (audio client format) */
    WAVEFORMATEX *pwfxOut = NULL; /* audio client format */

    /* resampler input and output containers */
    IMFSample *pResamplerInSample = NULL;
    IMFSample *pResamplerOutSample = NULL;
    IMFMediaBuffer *pResamplerInBuffer = NULL;
    IMFMediaBuffer *pResamplerOutBuffer = NULL;
    IMFMediaBuffer* pResamplerContiguousOutBuffer = NULL;


    HRC( CoInitialize( NULL ) );

    while(isCancellationPending() == false)
    {
        unsigned int prevrate = -1;
        unsigned int prevchannels = -1;

        pAudioClient = getAudioClient();

        HRC( pAudioClient->GetMixFormat(&pwfxOut) );

        HRC( pAudioClient->Initialize(
                                AUDCLNT_SHAREMODE_SHARED,
                                0,
                                hnsRequestedDuration,
                                0,
                                pwfxOut,
                                NULL) );

        HRC( pAudioClient->GetBufferSize(&bufferFrameCount) );

        HRC( pAudioClient->GetService(
                IID_IAudioRenderClient,
                (void**)&pRenderClient) );

/*        HRC( pAudioClient->GetService(
                IID_IAudioSessionControl,
                (void**)&pSessionControl) );*/

        HRC( pAudioClient->GetService(
                IID_ISimpleAudioVolume,
                (void**)&pStreamVolume) );

        HRC( MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET) );

        HRC( CoCreateInstance(CLSID_CResamplerMediaObject, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pResampler)) );

        while ( isCancellationPending() == false && reset == false )
        {
            Sleep(1);

            if ( lastVolume != actualVolume_ )
            {
                float vol = (float)actualVolume_ / 255;
                pStreamVolume->SetMasterVolume( vol, NULL );
                lastVolume = actualVolume_;
            }

            /* check if there's more audio available */
            if ( afd == NULL && ( afd = fifo_.getFifoDataTimedWait(10) ) == NULL )
                continue;

            /** Set up resampler if necessary */
            if ( afd->rate != prevrate || afd->channels != prevchannels )
            {
                // resampler input type
                MFCreateMediaType(&pInputType);

                HRC( pInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio) );
                HRC( pInputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM ) );
                HRC( pInputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS,         afd->channels) );
                HRC( pInputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND,   afd->rate) );
                HRC( pInputType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT,      afd->channels*sizeof(int16_t)) );
                HRC( pInputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, afd->rate*afd->channels*sizeof(int16_t)) );
                HRC( pInputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,      sizeof(int16_t)*8) );
                HRC( pInputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT,    TRUE) );

                HRC( pResampler->SetInputType(0, pInputType, 0) );

                // output type - has to be set after input type
                HRC( MFCreateMediaType( &pOutputType ) );
                HRC( MFInitMediaTypeFromWaveFormatEx( pOutputType, pwfxOut, sizeof(WAVEFORMATEX)+pwfxOut->cbSize ) );

                HRC( pResampler->SetOutputType( 0, pOutputType, 0 ) ); 

                // go!
                HRC( pResampler->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL) );
                HRC( pResampler->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL) );
                HRC( pResampler->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL) );

                prevrate = afd->rate;
                prevchannels = afd->channels;
            }

            if ( afd->timestamp != 0 )
            {
#define NSAMPLES 1
                static double timetoplaysamples[NSAMPLES] = {0};
                static uint8_t ntimetoplaysamples = 0;
                static uint8_t j = 0;
                unsigned int now = getTick_ms();
                int timeToPlayThisPacket = afd->timestamp - now;

                UINT32 totalBufferedSamples = 0;
                pAudioClient->GetCurrentPadding( &totalBufferedSamples );
                int timeToBufferUnderrun = (totalBufferedSamples * 1000 / pwfxOut->nSamplesPerSec) + 30; // hack! hardcoded 30 ms delay measured on my machine.. no idea if this is proper but better than nothing
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

                if ( timetoplay > 25 )
                {
                    sleep_ms( (unsigned int)timetoplay-10 );
                    ntimetoplaysamples = 0;
                    j=0;
                }
                else if ( timetoplay < -25 )
                {
                    fifo_.returnFifoDataBuffer( afd );
                    afd = NULL;
                    ntimetoplaysamples = 0;
                    j=0;
                    continue;
                }
                else if ( timetoplay < -1 || timetoplay > 1 )
                {
                    //we're off, adjust playback
                    adjustSamples_ = (int)timetoplay * (int)afd->rate / 1000;
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

            /** Put incoming data into resampler */
            DWORD bytes = afd->nsamples*afd->channels*sizeof(int16_t);
            HRC( MFCreateMemoryBuffer( bytes, &pResamplerInBuffer) );

            BYTE  *pByteBufferTo = NULL;
            HRC( pResamplerInBuffer->Lock(&pByteBufferTo, NULL, NULL) );
            assert(pByteBufferTo != NULL);
            memcpy(pByteBufferTo, (BYTE*)afd->samples, bytes);
            pResamplerInBuffer->Unlock();
            pByteBufferTo = NULL;

            HRC( pResamplerInBuffer->SetCurrentLength(bytes) );

            HRC( MFCreateSample(&pResamplerInSample) );
            HRC( pResamplerInSample->AddBuffer(pResamplerInBuffer) );

            HRC( pResampler->ProcessInput(0, pResamplerInSample, 0) );

            do {
                MFT_OUTPUT_DATA_BUFFER outputDataBuffer;
                DWORD dwStatus;
                DWORD outbytes = (afd->nsamples*pwfxOut->nAvgBytesPerSec/afd->rate);
                outbytes += 320;

                // cap resampler output chunk to half of audio client buffer
                if ( outbytes > bufferFrameCount * pwfxOut->nBlockAlign / 2 )
                    outbytes = bufferFrameCount * pwfxOut->nBlockAlign / 2;

                /** Get data from resampler */
                HRC( MFCreateSample(&pResamplerOutSample) );
                HRC( MFCreateMemoryBuffer( outbytes, &pResamplerOutBuffer ) );
                HRC( pResamplerOutSample->AddBuffer( pResamplerOutBuffer ) );

                memset( &outputDataBuffer, 0, sizeof(outputDataBuffer) );
                outputDataBuffer.pSample = pResamplerOutSample;

                hr = pResampler->ProcessOutput(0, 1, &outputDataBuffer, &dwStatus);
                if ( hr == MF_E_TRANSFORM_NEED_MORE_INPUT) {
                    // conversion end
                    pResamplerOutBuffer->Release();
                    pResamplerOutBuffer = NULL;
                    pResamplerOutSample->Release();
                    pResamplerOutSample = NULL;
                    break;
                }

                /** Convert resampler output to one contiguous buffer (todo optmize?) */
                HRC( pResamplerOutSample->ConvertToContiguousBuffer(&pResamplerContiguousOutBuffer) );
                DWORD cbBytes = 0;
                HRC( pResamplerContiguousOutBuffer->GetCurrentLength(&cbBytes) );

                BYTE  *pByteBuffer = NULL;
                HRC( pResamplerContiguousOutBuffer->Lock(&pByteBuffer, NULL, NULL) );

                /** Send resampler output to audio client */
                unsigned int outsamples = cbBytes/pwfxOut->nBlockAlign;

                BYTE *pData = NULL;
                do
                {
                    hr = pRenderClient->GetBuffer(outsamples, &pData);
                    if ( hr != S_OK )
                    {
                        if ( hr != AUDCLNT_E_BUFFER_TOO_LARGE )
                        {
                            log(LOG_WARN) << "Wasapi error " << hr;
                            reset = true;
                            goto cleanup;
                        }

                        if ( !playing )
                        {
                            pAudioClient->Start();
                            playing = true;
                        }
                        Sleep(1);
                    }
                } while( hr == AUDCLNT_E_BUFFER_TOO_LARGE );

                assert( pData );
                memcpy(pData, pByteBuffer, outsamples*pwfxOut->nBlockAlign);

                HRC( pRenderClient->ReleaseBuffer( outsamples, 0 /* AUDCLNT_BUFFERFLAGS_SILENT for mute */ ) );

                HRC( pResamplerContiguousOutBuffer->Unlock() );

                /** Done with this output chunk, clean up */
cleanup:
                if ( pResamplerContiguousOutBuffer ) pResamplerContiguousOutBuffer->Release();
                pResamplerContiguousOutBuffer = NULL;
                if ( pResamplerOutSample ) pResamplerOutSample->Release();
                pResamplerOutSample = NULL;
                if ( pResamplerOutBuffer ) pResamplerOutBuffer->Release();
                pResamplerOutBuffer = NULL;
            } while ( !reset && !isCancellationPending() );

            /** Done with this input chunk, clean up */
            if ( pResamplerInBuffer ) pResamplerInBuffer->Release();
            pResamplerInBuffer = NULL;
            if ( pResamplerInSample ) pResamplerInSample->Release();
            pResamplerInSample = NULL;

            fifo_.returnFifoDataBuffer( afd );
            afd = NULL;
        }

        log(LOG_NOTICE) << "Resetting audio client (" << hr <<")";

        if ( pAudioClient )
        {
            hr = pAudioClient->Stop();
            hr = pAudioClient->Reset();
            pAudioClient->Release();
            pAudioClient = NULL;
        }

        if ( pStreamVolume )
        {
            pStreamVolume->Release();
            pStreamVolume = NULL;
        }

        if ( pRenderClient )
        {
            pRenderClient->Release();
            pRenderClient = NULL;
        }

        if ( pResampler )
        {
            hr = pResampler->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, NULL);
            hr = pResampler->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, NULL);
            hr = pResampler->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL);
            pResampler->Release();
            pResampler = NULL;
        }

        if ( pInputType ) pInputType->Release();
        pInputType = NULL;
        if ( pOutputType ) pOutputType->Release();
        pOutputType = NULL;

        reset = false;
        playing = false;
    }

    log(LOG_DEBUG) << "Exiting AudioEndpoint::run()";
}
}


