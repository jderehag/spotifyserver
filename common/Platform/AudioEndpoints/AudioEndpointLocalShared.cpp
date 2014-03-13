#include "AudioEndpointLocal.h"
#include "applog.h"

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
        if ( thisBufferAdjustSamples > 4 )               thisBufferAdjustSamples = 4;

        for ( i=0; i<thisBufferAdjustSamples; i++ )
        {
            afd->samples[afd->nsamples*2] = afd->samples[afd->nsamples*2-2];
            afd->samples[afd->nsamples*2+1] = afd->samples[afd->nsamples*2-1];
            afd->nsamples++;
            adjustSamples_--;
        }
    }
    else if ( adjustSamples_ < 0 )
    {
        // we're late, remove samples to play faster
        uint32_t totalRemoveSamples = (0-adjustSamples_);
        if ( thisBufferAdjustSamples == 0 )                 thisBufferAdjustSamples = 1;
        if ( thisBufferAdjustSamples > totalRemoveSamples ) thisBufferAdjustSamples = totalRemoveSamples;

        afd->nsamples -= thisBufferAdjustSamples;
        adjustSamples_ += thisBufferAdjustSamples;
    }
}

}
