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

#ifndef MEDIAINTERFACE_H_
#define MEDIAINTERFACE_H_

#include "Platform/Threads/Mutex.h"
#include <set>
#include <string>

#include "MediaContainers/Folder.h"
#include "MediaContainers/Album.h"
#include "MediaContainers/Artist.h"
#include "MediaContainers/Track.h"
#include <deque>

using namespace LibSpotify;

class IMediaInterfaceCallbackSubscriber
{
public:
    virtual void connectionState( bool up ) {}
    virtual void rootFolderUpdatedInd() {}
    virtual void playlistUpdatedInd( const std::string& link ) {}
    virtual void statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress ) {}
    virtual void statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume ) {}

    virtual void getPlaylistsResponse( const Folder& rootfolder, void* userData ) {}
    virtual void getTracksResponse( const std::deque<Track>& tracks, void* userData ) {}
    virtual void getImageResponse( const void* data, size_t dataSize, void* userData ) {}
    virtual void getAlbumResponse( const Album& album, void* userData ) {}
    virtual void getArtistResponse( const Artist& artist, void* userData ) {}
    virtual void genericSearchCallback( const std::deque<Track>& listOfTracks, const std::string& didYouMean, void* userData ) {}
    virtual void getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress, void* userData ) {}
    virtual void getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, void* userData ) {}

    virtual void getCurrentAudioEndpointsResponse( const std::set<std::string>& endpoints, void* userData ) {}
};


class MediaInterface
{
private:

protected:
    /***********************
     * Callback subscription
     ***********************/
    Platform::Mutex callbackSubscriberMtx_;
    typedef std::set<IMediaInterfaceCallbackSubscriber*> MediaInterfaceCallbackSubscriberSet;
    MediaInterfaceCallbackSubscriberSet callbackSubscriberList_;

    /*typical data related to a request that will be needed when it's time for a response
      i.e. the subscriber the request originated from and the user data allocated with the request*/
    typedef std::pair<IMediaInterfaceCallbackSubscriber*, void*> PendingMediaRequestData;

    bool isConnected;
    void connectionState( bool up );
public:
    MediaInterface();
    virtual ~MediaInterface();

    void registerForCallbacks(IMediaInterfaceCallbackSubscriber& subscriber);
    void unRegisterForCallbacks(IMediaInterfaceCallbackSubscriber& subscriber);

    virtual void getImage( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void previous() = 0;
    virtual void next() = 0;
    virtual void resume() = 0;
    virtual void pause() = 0;
    virtual void seek( uint32_t sec ) = 0;
    virtual void setShuffle( bool shuffleOn ) = 0;
    virtual void setRepeat( bool repeatOn ) = 0;
    virtual void setVolume( uint8_t volume ) = 0;
    virtual void getStatus( IMediaInterfaceCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void getPlaylists( IMediaInterfaceCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void getTracks( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void play( const std::string& link, int startIndex, IMediaInterfaceCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void play( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void enqueue( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void getAlbum( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void getArtist( const std::string& link, IMediaInterfaceCallbackSubscriber* subscriber, void* userData ) = 0;
    virtual void search( const std::string& query, IMediaInterfaceCallbackSubscriber* subscriber, void* userData ) = 0;

    /* todo move to AudioEndpointManager? */
    virtual void getCurrentAudioEndpoints( IMediaInterfaceCallbackSubscriber* subscriber, void* userData ) = 0;

};

#endif /* MEDIAINTERFACE_H_ */
