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

#include "TestApp/MediaInterface.h"
#include "SocketHandling/SocketPeer.h"
#include "MessageFactory/MessageEncoder.h"
#include "Platform/AudioEndpoints/AudioEndpointRemote.h"
#include <map>

using namespace LibSpotify;

class Client : IMediaInterfaceCallbackSubscriber, public SocketPeer
{
private:

    MediaInterface& spotify_;

    bool loggedIn_;
    std::string networkUsername_;
    std::string networkPassword_;

    uint32_t peerProtocolMajor_;
    uint32_t peerProtocolMinor_;

    typedef std::map<unsigned int, Message*>  PendingMessageMap;
    PendingMessageMap pendingMessageMap_;

    Platform::AudioEndpointRemote* audioEp;

    unsigned int reqId_;

    virtual void processMessage(const Message* msg);

    void playingInd(Track& currentTrack);
    void pausedInd(Track& currentTrack);
    void trackEndedInd();

    virtual void connectionState( bool up );
    virtual void rootFolderUpdatedInd();
    virtual void statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, const Track& currentTrack, unsigned int progress );
    virtual void statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus );

    virtual void getPlaylistsResponse( MediaInterfaceRequestId reqId, const Folder& rootfolder );
    virtual void getTracksResponse( MediaInterfaceRequestId reqId, const std::deque<Track>& tracks );
    virtual void getImageResponse( MediaInterfaceRequestId reqId, const void* data, size_t dataSize );
    virtual void getAlbumResponse( MediaInterfaceRequestId reqId, const Album& album );
    virtual void genericSearchCallback( MediaInterfaceRequestId reqId, std::deque<Track>& listOfTracks, const std::string& didYouMean);
    virtual void getStatusResponse( MediaInterfaceRequestId reqId, PlaybackState_t state, bool repeatStatus, bool shuffleStatus, const Track& currentTrack, unsigned int progress );
    virtual void getStatusResponse( MediaInterfaceRequestId reqId, PlaybackState_t state, bool repeatStatus, bool shuffleStatus );

private:
    /* Message Handler functions*/
    void handleGetTracksReq(const Message* msg);
    void handleHelloReq(const Message* msg);
    void handleGetPlaylistsReq(const Message* msg);
    void handleImageReq(const Message* msg);
    void handleGetStatusReq(const Message* msg);
    void handlePlayReq(const Message* msg);
    void handlePlayTrackReq(const Message* msg);
    void handlePlayControlReq(const Message* msg);
    void handleGetImageReq(const Message* msg);
    void handleGenericSearchReq(const Message* msg);
    void handleGetAlbumReq(const Message* msg);
    void handleAddAudioEpReq(const Message* msg);
    void handleRemAudioEpReq(const Message* msg);

public:

    Client(Socket* socket, MediaInterface& spotifyif);
    virtual ~Client();

    void setUsername(std::string username);
    void setPassword(std::string password);

};

#endif /* CLIENT_H_ */
