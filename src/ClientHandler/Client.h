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

#ifndef CLIENT_H_
#define CLIENT_H_

#include "MediaInterface/MediaInterface.h"
#include "EndpointManager/EndpointManagerCtrlInterface.h"
#include "SocketHandling/SocketPeer.h"
#include "Platform/AudioEndpoints/AudioEndpointRemote.h"
#include "EndpointId/EndpointIdIf.h"
#include "ClientHandler.h"

using namespace LibSpotify;

class Client : IMediaInterfaceCallbackSubscriber, IEndpointCtrlCallbackSubscriber, IAudioEndpointRemoteCtrlInterface, public EndpointIdIf, public SocketPeer
{
private:

    MediaInterface& spotify_;
    EndpointCtrlInterface& epCtrl_;

    Platform::AudioEndpointRemote* audioEp;

    static uint32_t count;

    bool loggedIn_;
    std::string networkUsername_;
    std::string networkPassword_;

    uint32_t peerProtocolMajor_;
    uint32_t peerProtocolMinor_;

    virtual void processMessage( const Message* msg );
    virtual void processResponse( const Message* rsp, void* userData );

    void playingInd(Track& currentTrack);
    void pausedInd(Track& currentTrack);
    void trackEndedInd();

    /* implements IMediaInterfaceCallbackSubscriber */
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


    /* Message Handler functions*/
    void handleGetTracksReq(const Message* msg);
    void handleHelloReq(const Message* msg);
    void handleGetPlaylistsReq(const Message* msg);
    void handleImageReq(const Message* msg);
    void handleGetStatusReq(const Message* msg);
    void handlePlayReq(const Message* msg);
    void handlePlayTrackReq(const Message* msg);
    void handlePlayControlReq(const Message* msg);
    void handleSetVolumeReq(const Message* msg);
    void handleEnqueueReq(const Message* msg);
    void handleGetImageReq(const Message* msg);
    void handleGenericSearchReq(const Message* msg);
    void handleGetAlbumReq(const Message* msg);
    void handleGetArtistReq(const Message* msg);
    void handleGetCurrentAudioEpReq(const Message* msg);


    /* implements IEndpointCtrlCallbackSubscriber */
    virtual void renameEndpointResponse( void* userData );
    virtual void getEndpointsResponse( const EndpointInfoList& endpoints, void* userData );

    virtual void getAudioEndpointsResponse( const AudioEndpointInfoList& endpoints, void* userData );
    virtual void audioEndpointsUpdatedNtf();

    /* endpoint control message handlers */
    void handleRenameEndpointReq(const Message* msg);
    void handleGetEndpointsReq(const Message* msg);
    void handleCreateAudioEpReq(const Message* msg);
    void handleDeleteAudioEpReq(const Message* msg);
    void handleAddAudioEpReq(const Message* msg);
    void handleRemAudioEpReq(const Message* msg);
    void handleGetAudioEpReq(const Message* msg);


    /* implements IAudioEndpointRemoteCtrlInterface */
    virtual void setMasterVolume( uint8_t volume );
    virtual void setRelativeVolume( uint8_t volume );

    /* Implements EndpointIdIf */
    void rename( const std::string& newId );

public:

    Client( Socket* socket, MediaInterface& spotifyif, EndpointCtrlInterface& epCtrl );
    virtual ~Client();

    void setUsername(std::string username);
    void setPassword(std::string password);

};

#endif /* CLIENT_H_ */
