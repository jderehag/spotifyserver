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

}
