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
#include "applog.h"
#include <iostream>
#include <asoundlib.h>

static snd_pcm_t *alsa_open(const char *dev, int rate, int channels);

namespace Platform {

AudioEndpointLocal::AudioEndpointLocal(const ConfigHandling::AudioEndpointConfig& config) : config_(config)
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


int AudioEndpointLocal::enqueueAudioData(unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples)
{
    return fifo.addFifoDataBlocking(channels, rate, nsamples, samples);
}

void AudioEndpointLocal::flushAudioData()
{
    fifo.flush();
}

void AudioEndpointLocal::run()
{
	snd_pcm_t *devFd = NULL;
	int c;
	unsigned int currentChannels = 0;
	unsigned int currentRate = 0;

	AudioFifoData *afd;

	while(isCancellationPending() == false)
	{
		if((afd = fifo.getFifoDataTimedWait(1)) != 0)
		{
			/* First set up the alsa device with correct parameters (rate & channels) */
			if (!devFd || currentRate != afd->rate || currentChannels != afd->channels)
			{
				if (devFd) snd_pcm_close(devFd);

				currentRate = afd->rate;
				currentChannels = afd->channels;

				if((devFd = alsa_open(config_.getDevice().c_str(), currentRate, currentChannels)) == NULL)
				{
					fprintf(stderr, "Unable to open ALSA device %s (%d channels, %d Hz)\n",
					        config_.getDevice().c_str() , currentChannels, currentRate);
				}
			}

			if(devFd)
			{
				c = snd_pcm_wait(devFd, 1000);

				if (c >= 0)
					c = snd_pcm_avail_update(devFd);

				if (c == -EPIPE)
					snd_pcm_prepare(devFd);

				snd_pcm_writei(devFd, afd->samples, afd->nsamples);
			}
			free( afd );
			afd = 0;
		}
	}

	if (devFd) snd_pcm_close(devFd);

	log(LOG_DEBUG) << "Exiting AudioEndpoint::run()";
}

}

static snd_pcm_t *alsa_open(const char *dev, int rate, int channels)
{
	snd_pcm_hw_params_t *hwp;
	snd_pcm_sw_params_t *swp;
	snd_pcm_t *h;
	int r;
	int dir;
	snd_pcm_uframes_t period_size_min;
	snd_pcm_uframes_t period_size_max;
	snd_pcm_uframes_t buffer_size_min;
	snd_pcm_uframes_t buffer_size_max;
	snd_pcm_uframes_t period_size;
	snd_pcm_uframes_t buffer_size;

	if ((r = snd_pcm_open(&h, dev, SND_PCM_STREAM_PLAYBACK, 0) < 0))
		return NULL;

	hwp = static_cast<snd_pcm_hw_params_t *>(alloca(snd_pcm_hw_params_sizeof()));
	memset(hwp, 0, snd_pcm_hw_params_sizeof());
	snd_pcm_hw_params_any(h, hwp);

	snd_pcm_hw_params_set_access(h, hwp, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(h, hwp, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_rate(h, hwp, rate, 0);
	snd_pcm_hw_params_set_channels(h, hwp, channels);

	/* Configurue period */

	dir = 0;
	snd_pcm_hw_params_get_period_size_min(hwp, &period_size_min, &dir);
	dir = 0;
	snd_pcm_hw_params_get_period_size_max(hwp, &period_size_max, &dir);

	period_size = 1024;

	dir = 0;
	r = snd_pcm_hw_params_set_period_size_near(h, hwp, &period_size, &dir);

	if (r < 0) {
		fprintf(stderr, "audio: Unable to set period size %lu (%s)\n",
		        period_size, snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	dir = 0;
	r = snd_pcm_hw_params_get_period_size(hwp, &period_size, &dir);

	if (r < 0) {
		fprintf(stderr, "audio: Unable to get period size (%s)\n",
		        snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	/* Configurue buffer size */

	snd_pcm_hw_params_get_buffer_size_min(hwp, &buffer_size_min);
	snd_pcm_hw_params_get_buffer_size_max(hwp, &buffer_size_max);
	buffer_size = period_size * 4;

	dir = 0;
	r = snd_pcm_hw_params_set_buffer_size_near(h, hwp, &buffer_size);

	if (r < 0) {
		fprintf(stderr, "audio: Unable to set buffer size %lu (%s)\n",
		        buffer_size, snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	r = snd_pcm_hw_params_get_buffer_size(hwp, &buffer_size);

	if (r < 0) {
		fprintf(stderr, "audio: Unable to get buffer size (%s)\n",
		        snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	/* write the hw params */
	r = snd_pcm_hw_params(h, hwp);

	if (r < 0) {
		fprintf(stderr, "audio: Unable to configure hardware parameters (%s)\n",
		        snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	/*
	 * Software parameters
	 */

	swp = static_cast<snd_pcm_sw_params_t *>(alloca(snd_pcm_sw_params_sizeof()));
	memset(hwp, 0, snd_pcm_sw_params_sizeof());
	snd_pcm_sw_params_current(h, swp);

	r = snd_pcm_sw_params_set_avail_min(h, swp, period_size);

	if (r < 0) {
		fprintf(stderr, "audio: Unable to configure wakeup threshold (%s)\n",
		        snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	snd_pcm_sw_params_set_start_threshold(h, swp, 0);

	if (r < 0) {
		fprintf(stderr, "audio: Unable to configure start threshold (%s)\n",
		        snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	r = snd_pcm_sw_params(h, swp);

	if (r < 0) {
		fprintf(stderr, "audio: Cannot set soft parameters (%s)\n",
		snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	r = snd_pcm_prepare(h);
	if (r < 0) {
		fprintf(stderr, "audio: Cannot prepare audio for playback (%s)\n",
		snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	return h;
}
