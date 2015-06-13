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

#ifndef UICONSOLE_H_
#define UICONSOLE_H_

#include "Platform/Threads/Runnable.h"
#include "Platform/Threads/Mutex.h"
#include "MediaInterface/MediaInterface.h"
#include "EndpointManager/EndpointManagerCtrlInterface.h"
#include "LibSpotifyIf/ILibSpotifyLoginUI.h"

class UIConsole : public Platform::Runnable, public IMediaInterfaceCallbackSubscriber, public IEndpointCtrlCallbackSubscriber, public ILibSpotifyLoginUI
{
private:
    MediaInterface& m_;
    EndpointCtrlInterface& epMgr_;

    Platform::Mutex consoleMtx;
    bool doLogin;
    LibSpotifyLoginParams loginRes;
    Platform::Condition loginSequenceDone;

    bool isShuffle;
    bool isRepeat;
public:
    UIConsole(MediaInterface& m, EndpointCtrlInterface& epMgr);
    ~UIConsole();

    void run();
    void destroy();

    /* Implements IMediaInterfaceCallbackSubscriber */
    virtual void connectionState( bool up );
    virtual void rootFolderUpdatedInd();
    virtual void playlistUpdatedInd( const std::string& link );
    virtual void statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress );
    virtual void statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume );

    virtual void getPlaylistsResponse( const Folder& rootfolder, void* userData );
    virtual void getTracksResponse( const std::deque<Track>& tracks, void* userData );
    virtual void getImageResponse( const void* data, size_t dataSize, void* userData );
    virtual void getAlbumResponse( const Album& album, void* userData );
    virtual void getArtistResponse( const Artist& artist, void* userData );
    virtual void genericSearchCallback( const std::deque<Track>& listOfTracks, const std::string& didYouMean, void* userData );
    virtual void getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress, void* userData );
    virtual void getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, void* userData );

    virtual void getCurrentAudioEndpointsResponse( const std::set<std::string>& endpoints, void* userData );

    /* Implements IEndpointCtrlCallbackSubscriber */
    virtual void renameEndpointResponse( void* userData );
    virtual void getEndpointsResponse( const EndpointInfoList& endpoints, void* userData );

    virtual void getAudioEndpointsResponse( const AudioEndpointInfoList& endpoints, void* userData );
    virtual void audioEndpointsUpdatedNtf();

    virtual void getStatisticsResponse( const std::string& id, const CounterList& counters, void* userData );

    /* ILibSpotifyLoginUI */
    virtual LibSpotifyLoginParams getLoginParams( const std::string& message, const std::string& oldUsername, bool oldRememberMe );
};

#endif /* UICONSOLE_H_ */
