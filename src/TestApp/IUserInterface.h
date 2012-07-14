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

#ifndef IUSERINTERFACE_H_
#define IUSERINTERFACE_H_

#include "Messenger.h"
#include "LibSpotifyIf/MediaContainers/Folder.h"
#include "LibSpotifyIf/MediaContainers/Playlist.h"
#include "MessageFactory/TlvDefinitions.h"
#include <string>

using namespace LibSpotify;

class IUserInterface : public IMessageSubscriber
{
private:
    Messenger& messenger_;
protected:
    PlaybackState_t playbackState_;
    Folder rootFolder_;

public:
    IUserInterface(Messenger& messenger);
    virtual ~IUserInterface();

    void receivedMessage( Message* msg );

protected:
    void getImage( std::string uri );
    void previous();
    void next();
    void resume();
    void pause();
    void getStatus();
    void getPlaylists();
    void getTracks( std::string uri );
    void play( std::string uri, int startIndex = 0 );
    void getAlbum( std::string uri );
    void search( std::string query );


    virtual void updateRootFolder(Folder& f) {(void)f;}
    virtual void status() {}
};

#endif /* IUSERINTERFACE_H_ */
