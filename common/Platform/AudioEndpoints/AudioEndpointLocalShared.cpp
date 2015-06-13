#include "AudioEndpointLocal.h"
#include "applog.h"
#include <string.h>

namespace Platform
{


int AudioEndpointLocal::enqueueAudioData( unsigned int timestamp, unsigned short channels, unsigned int rate, unsigned int nsamples, const int16_t* samples )
{
    int n;
    n = fifo_.addFifoDataBlocking( timestamp, channels, rate, nsamples, samples );
    return n;
}

void AudioEndpointLocal::flushAudioData()
{
    fifo_.flush();
}

unsigned int AudioEndpointLocal::getNumberOfQueuedSamples()
{
    return fifo_.getNumberOfQueuedSamples();
}

void AudioEndpointLocal::adjustSamples( AudioFifoData* afd )
{
    uint32_t i;
    uint32_t thisBufferAdjustSamples = afd->nsamples / 200; //adjust no more than 0.5%
    if ( adjustSamples_ > 0 )
    {
        // we're early, pad buffer to play slower
        uint32_t totalPadSamples = adjustSamples_;
        uint32_t headroom = afd->bufferSize - (afd->nsamples*afd->channels*sizeof(int16_t)); // in bytes
        if ( thisBufferAdjustSamples == 0 )              thisBufferAdjustSamples = 1;
        if ( thisBufferAdjustSamples > totalPadSamples ) thisBufferAdjustSamples = totalPadSamples;
        if ( thisBufferAdjustSamples > headroom/4 )      thisBufferAdjustSamples = headroom/4;
        //if ( thisBufferAdjustSamples > 4 )               thisBufferAdjustSamples = 4;

        uint32_t chunksize = afd->nsamples / (thisBufferAdjustSamples+1);
        uint32_t n = afd->nsamples;

        for ( i=thisBufferAdjustSamples; i>0; i-- )
        {
            int16_t* src = &afd->samples[(n-chunksize)*afd->channels];
            int16_t* dst = &afd->samples[(n+i-chunksize)*afd->channels];
            memmove( dst, src, chunksize * afd->channels * sizeof(int16_t) );
            dst--;
            *dst = (*(dst+2) + *(dst-i*2))/2;
            dst--;
            *dst = (*(dst+2) + *(dst-i*2))/2;

            n -= chunksize;
        }

        afd->nsamples += thisBufferAdjustSamples;
        adjustSamples_ -= thisBufferAdjustSamples;

    }
    else if ( adjustSamples_ < 0 )
    {
        // we're late, remove samples to play faster
        uint32_t totalRemoveSamples = (0-adjustSamples_);
        if ( thisBufferAdjustSamples == 0 )                 thisBufferAdjustSamples = 1;
        if ( thisBufferAdjustSamples > totalRemoveSamples ) thisBufferAdjustSamples = totalRemoveSamples;

        uint32_t chunksize = afd->nsamples / (thisBufferAdjustSamples+1);
        uint32_t n = chunksize;

        for ( i=1; i<thisBufferAdjustSamples; i++ )
        {
            int16_t* src = &afd->samples[(n)*afd->channels];
            int16_t* dst = &afd->samples[(n-i)*afd->channels];

            memmove( dst, src, (chunksize-1) * afd->channels * sizeof(int16_t) );

            n += chunksize;

        }
        memmove( &afd->samples[(n-i)*afd->channels], &afd->samples[(n)*afd->channels], (afd->nsamples-n) * afd->channels * sizeof(int16_t) );

        afd->nsamples -= thisBufferAdjustSamples;
        adjustSamples_ += thisBufferAdjustSamples;
    }
}

void AudioEndpointLocal::setMasterVolume( uint8_t volume )
{
    uint16_t newVolume;
    masterVolume_ = volume;
    newVolume = (uint16_t) relativeVolume_*volume/255;
    actualVolume_ = (uint8_t) newVolume;
}
void AudioEndpointLocal::doSetRelativeVolume( uint8_t volume )
{
    uint16_t newVolume;
    relativeVolume_ = volume;
    newVolume = (uint16_t) masterVolume_*volume/255;
    actualVolume_ = (uint8_t) newVolume;
}

const Counters& AudioEndpointLocal::getStatistics()
{
    return counters;
}

}
