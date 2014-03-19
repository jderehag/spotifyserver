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
#include <Mfapi.h>
#include <Mftransform.h>
#include <Mferror.h>
#include <iostream>

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

namespace Platform {

#define HRC(x) do{ hr = x; assert( hr == S_OK ); } while(0)

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

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const CLSID CLSID_CResamplerMediaObject = __uuidof(CResamplerMediaObject);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

void AudioEndpointLocal::run()
{
    AudioFifoData* afd = NULL;
    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC / 10;
    REFERENCE_TIME hnsActualDuration;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    IAudioClient *pAudioClient = NULL;
    IAudioRenderClient *pRenderClient = NULL;
    WAVEFORMATEX *pwfxOut = NULL;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    UINT32 numFramesPadding;
    BYTE *pData;
    DWORD flags = 0;
    int i;
    unsigned int prevrate = -1;
    unsigned int prevchannels = -1;
    bool playing = false;
    int msInResampler = 0;
    int msOutResampler = 0;

    while(isCancellationPending() == false)
    {
        HRC( CoInitialize( NULL ) );

        HRC( CoCreateInstance(
               CLSID_MMDeviceEnumerator, NULL,
               CLSCTX_ALL, IID_IMMDeviceEnumerator,
               (void**)&pEnumerator) );

        HRC( pEnumerator->GetDefaultAudioEndpoint(
                            eRender, eConsole, &pDevice) );

        HRC( pDevice->Activate(
                        IID_IAudioClient, CLSCTX_ALL,
                        NULL, (void**)&pAudioClient) );

        HRC( pAudioClient->GetMixFormat(&pwfxOut) );
        /*WAVEFORMATEXTENSIBLE tmp;
        WAVEFORMATEX* pwfxt = (WAVEFORMATEX*)&tmp;
        pwfxt->wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        pwfxt->nChannels = afd->channels;
        pwfxt->nSamplesPerSec = afd->rate;
        pwfxt->wBitsPerSample = afd->channels*sizeof(int16_t)*8;
        pwfxt->nAvgBytesPerSec = pwfxt->wBitsPerSample*pwfxt->nSamplesPerSec/8;
        pwfxt->nBlockAlign = 8;
        pwfxt->cbSize = 22;
        tmp.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

        HRC( pAudioClient->IsFormatSupported( AUDCLNT_SHAREMODE_SHARED, pwfxt, &pwfx );*/

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


        HRC( MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET) );
// check if SUCCEEDED(hr)

//2. Create Resampler MFT Object
/*
CComPtr<IUnknown> spTransformUnk;
IMFTransform *pTransform = NULL; //< this is Resampler MFT

HRC( CoCreateInstance(CLSID_CResamplerMediaObject, NULL, CLSCTX_INPROC_SERVER,
        IID_IUnknown, (void**)&spTransformUnk);

HRC( spTransformUnk->QueryInterface(IID_PPV_ARGS(&pTransform));

CComPtr<IWMResamplerProps> spResamplerProps;
HRC( spTransformUnk->QueryInterface(IID_PPV_ARGS(&spResamplerProps);
HRC( spResamplerProps->SetHalfFilterLength(60); //< best conversion quality
*/
   IMFMediaType* pInputType = NULL;
   IMFMediaType* pOutputType = NULL;
   IMFTransform* pResampler = NULL;

   HRC( CoCreateInstance(CLSID_CResamplerMediaObject, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pResampler)) );

   DWORD instreamid;
   DWORD outstreamid;

   hr = pResampler->GetStreamIDs(1, &instreamid, 1, &outstreamid );

   if ( hr == E_NOTIMPL )
   {
       instreamid = outstreamid = 0;
   }

//3. Specify input/output PCM format to Resampler MFT
// set input PCM format parameters to fmt



//4. Send stream start message to Resampler MFT




        /*if (!device)
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
        alGenSources(1, &source);*/

        while(isCancellationPending() == false)
        {
            Sleep(1);
            /*if ((error = alcGetError(device)) != AL_NO_ERROR) 
            {
                log(LOG_EMERG) << "openal al error: " << error;
                break;
            }*/

            /* check if there's more audio available */
            if ( afd == NULL && ( afd = fifo_.getFifoDataTimedWait(10) ) == NULL )
                continue;


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
                REFERENCE_TIME outputLatency = 0;
                pAudioClient->GetCurrentPadding( &totalBufferedSamples );
                pAudioClient->GetStreamLatency( &outputLatency );
                int timeToBufferUnderrun = (totalBufferedSamples * 1000 / pwfxOut->nSamplesPerSec) + outputLatency/REFTIMES_PER_MILLISEC;
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
                    afd = NULL;
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

            DWORD bytes = afd->nsamples*afd->channels*sizeof(int16_t);
            IMFMediaBuffer *pBuffer = NULL;
            HRC( MFCreateMemoryBuffer( bytes, &pBuffer) );

            BYTE  *pByteBufferTo = NULL;
            HRC( pBuffer->Lock(&pByteBufferTo, NULL, NULL) );
            assert(pByteBufferTo != NULL);
            memcpy(pByteBufferTo, (BYTE*)afd->samples, bytes);
            pBuffer->Unlock();
            pByteBufferTo = NULL;

            HRC( pBuffer->SetCurrentLength(bytes) );

            IMFSample *pSample = NULL;
            HRC( MFCreateSample(&pSample) );
            HRC( pSample->AddBuffer(pBuffer) );

            msInResampler += afd->nsamples;
            HRC( pResampler->ProcessInput(instreamid, pSample, 0) );

            pBuffer->Release();
            pBuffer = NULL;

            MFT_OUTPUT_DATA_BUFFER outputDataBuffer;
            do {
                DWORD dwStatus;
                IMFSample *pOutSample = NULL;
                DWORD outbytes = (afd->nsamples*pwfxOut->nAvgBytesPerSec/afd->rate);
                outbytes += 320;

                if ( outbytes > bufferFrameCount * pwfxOut->nBlockAlign );
                    outbytes = bufferFrameCount * pwfxOut->nBlockAlign;
                HRC( MFCreateSample(&pOutSample) );
                HRC( MFCreateMemoryBuffer( outbytes, &pBuffer ) );
                HRC( pOutSample->AddBuffer( pBuffer ) );

                memset( &outputDataBuffer, 0, sizeof(outputDataBuffer) );
                outputDataBuffer.pSample = pOutSample;

                hr = pResampler->ProcessOutput(0, 1, &outputDataBuffer, &dwStatus);
                if ( hr == MF_E_TRANSFORM_NEED_MORE_INPUT) {
                    // conversion end
                    pBuffer->Release();
                    pOutSample->Release();
                    break;
                }

                // output PCM data is set in outputDataBuffer.pSample;
                IMFSample *pSample = outputDataBuffer.pSample;
 
                IMFMediaBuffer* spBuffer;
                HRC( pSample->ConvertToContiguousBuffer(&spBuffer) );
                DWORD cbBytes = 0;
                HRC( spBuffer->GetCurrentLength(&cbBytes) );

                BYTE  *pByteBuffer = NULL;
                HRC( spBuffer->Lock(&pByteBuffer, NULL, NULL) );

                unsigned int outsamples = cbBytes/pwfxOut->nBlockAlign;
                msOutResampler += outsamples;

                do
                {
                    hr = pRenderClient->GetBuffer(outsamples, &pData);
                    if ( hr != S_OK )
                    {
                        assert ( hr == AUDCLNT_E_BUFFER_TOO_LARGE );

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

                HRC( pRenderClient->ReleaseBuffer( outsamples, flags ) );

                HRC( spBuffer->Unlock() );

                spBuffer->Release();

                pOutSample->Release();
                pBuffer->Release();
            } while (1);

            pSample->Release();

            fifo_.returnFifoDataBuffer( afd );
            afd = NULL;
        }
    }

    log(LOG_DEBUG) << "Exiting AudioEndpoint::run()";
}
}


