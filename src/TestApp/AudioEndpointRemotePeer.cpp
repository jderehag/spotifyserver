/*
 * AudioEndpointRemotePeer.cpp
 *
 *  Created on: 14 nov 2012
 *      Author: Jesse
 */

#include "AudioEndpointRemotePeer.h"
#include "applog.h"

AudioEndpointRemotePeer::AudioEndpointRemotePeer( Socket* socket, ConfigHandling::AudioEndpointConfig& config )
        : SocketPeer(socket), endpoint_(config)
{
}

AudioEndpointRemotePeer::~AudioEndpointRemotePeer()
{
    endpoint_.destroy();
}

void AudioEndpointRemotePeer::processMessage(const Message* msg)
{
    log(LOG_NOTICE) << *(msg);

    switch(msg->getType())
    {
        case AUDIO_DATA_IND:
        {
            const IntTlv* channelstlv = (const IntTlv*)msg->getTlvRoot()->getTlv(TLV_AUDIO_CHANNELS);
            const IntTlv* ratetlv     = (const IntTlv*)msg->getTlvRoot()->getTlv(TLV_AUDIO_RATE);
            const IntTlv* nsamplestlv = (const IntTlv*)msg->getTlvRoot()->getTlv(TLV_AUDIO_NOF_SAMPLES);
            const BinaryTlv* samplestlv = (const BinaryTlv*)msg->getTlvRoot()->getTlv(TLV_AUDIO_DATA);

            if ( channelstlv && ratetlv && samplestlv && samplestlv )
            {
                unsigned short channels = channelstlv->getVal();
                unsigned int rate       = ratetlv->getVal();
                unsigned int nsamples   = nsamplestlv->getVal();
                const int16_t* samples  = (const int16_t*)samplestlv->getData(); /* todo ntoh me! */

                endpoint_.enqueueAudioData(channels, rate, nsamples, samples);
            }
        }
        break;

        default:
            break;
    }
}
