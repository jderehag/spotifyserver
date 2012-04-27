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

#ifndef ILIBSPOTIFYIFCALLBACKSUBSCRIBER_H_
#define ILIBSPOTIFYIFCALLBACKSUBSCRIBER_H_

#include "MediaContainers/Track.h"
#include "MediaContainers/Album.h"
#include <deque>
#include <string.h>

namespace LibSpotify {

class ILibSpotifyIfCallbackSubscriber
{
public:
    /* spurious callbacks // ALL subscriber notifications */
    virtual void rootFolderUpdatedInd() = 0;
    virtual void playingInd(Track& currentTrack) = 0;
    virtual void pausedInd(Track& currentTrack) = 0;
    virtual void trackEndedInd() = 0;

    /* Metadata specific callbacks*/
    virtual void getTrackResponse(unsigned int reqId, const std::deque<Track>& tracks) = 0;
    virtual void getImageResponse(unsigned int reqId, const void* data, size_t dataSize) = 0;
    virtual void getAlbumResponse(unsigned int reqId, const Album& album) = 0;

    /* search callbacks */
    virtual void genericSearchCallback(unsigned int reqId, std::deque<Track>& listOfTracks, const std::string& didYouMean) = 0;
};
}



#endif /* ILIBSPOTIFYIFCALLBACKSUBSCRIBER_H_ */
