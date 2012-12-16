/*
 * AudioEndpointLocal.h
 *
 *  Created on: 31 okt 2012
 *      Author: Jesse
 */

#ifndef AUDIOENDPOINTLOCAL_H_
#define AUDIOENDPOINTLOCAL_H_

#include "AudioEndpoint.h"
#include "ConfigHandling/ConfigHandler.h"
#include "../Threads/Runnable.h"

namespace Platform {

class AudioEndpointLocal : public AudioEndpoint, Runnable
{
    ConfigHandling::AudioEndpointConfig config_;

public:
    AudioEndpointLocal(const ConfigHandling::AudioEndpointConfig& config);
    virtual ~AudioEndpointLocal();

    virtual int enqueueAudioData(unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples);
    virtual void flushAudioData();

    virtual void run();
    virtual void destroy();
};

}
#endif /* AUDIOENDPOINTLOCAL_H_ */
