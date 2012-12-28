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

#include "MediaContainers/Folder.h"
#include "Platform/Threads/Mutex.h"
#include <set>
#include <string>

class IMediaInterfaceCallbackSubscriber
{
public:
    virtual void connectionState( bool up ) = 0;
    virtual void rootFolderUpdatedInd(LibSpotify::Folder& f) = 0;
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

public:
    MediaInterface();
    virtual ~MediaInterface();

    void registerForCallbacks(IMediaInterfaceCallbackSubscriber& subscriber);
    void unRegisterForCallbacks(IMediaInterfaceCallbackSubscriber& subscriber);

    virtual void getImage( std::string uri ) = 0;
    virtual void previous() = 0;
    virtual void next() = 0;
    virtual void resume() = 0;
    virtual void pause() = 0;
    virtual void setShuffle( bool shuffleOn ) = 0;
    virtual void setRepeat( bool repeatOn ) = 0;
    virtual void getStatus() = 0;
    virtual void getPlaylists() = 0;
    virtual void getTracks( std::string uri ) = 0;
    virtual void play( std::string uri, int startIndex ) = 0;
    virtual void play( std::string uri ) = 0;
    virtual void getAlbum( std::string uri ) = 0;
    virtual void search( std::string query ) = 0;
    virtual void addAudio() = 0;

    virtual PlaybackState_t getCurrentPlaybackState() = 0;

};

#endif /* MEDIAINTERFACE_H_ */
