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

#include "UIConsole.h"
#include "applog.h"

UIConsole::UIConsole( MediaInterface& m ) : m_(m),
                                            reqId_(0),
                                            itPlaylists_(playlists.begin()),
                                            isShuffle(false),
                                            isRepeat(false)
{
    m_.registerForCallbacks( *this );
    startThread();
}
UIConsole::~UIConsole()
{
}

void UIConsole::destroy()
{
    m_.unRegisterForCallbacks( *this );
    cancelThread();
    joinThread();
}


void UIConsole::run()
{
    char c;

    while(isCancellationPending() == false)
    {
        std::cout << "'g' get playlists\n"
                     "'t' get tracks\n"
                     "'a' get album\n"
                     "'p' load a play queue\n"
                     "'?' search using query\n"
                     "'s' get status\n"
                     "'z' previous\n"
                     "'x' play\n"
                     "'c' pause\n"
                     "'v' next\n"
                     "'e' toggle shuffle\n" << std::endl;

        c = getchar();

        switch(c)
        {
        case 'i':
        {
            std::string uri;
            std::cout << "Enter Album URI" << std::endl;
            std::cin >> uri;

            m_.getImage( uri, this, reqId_++ );
            break;
        }

        case 'z':
        {
            m_.previous();
            break;
        }
        case 'v':
        {
            m_.next();
            break;
        }
        case 'x':
        {
            m_.resume();
            break;
        }
        case 'c':
        {
            m_.pause();
            break;
        }
        case 'e':
        {
            isShuffle = !isShuffle;
            m_.setShuffle(isShuffle);
            break;
        }
        case 'r':
        {
            isRepeat = !isRepeat;
            m_.setRepeat(isRepeat);
            break;
        }

        case 's':
        {
            m_.getStatus( this, reqId_++ );
            break;
        }
        case 'g':
        {
            m_.getPlaylists( this, reqId_++ );
            break;
        }
        case 't':
        {
            std::string uri;
            std::cout << "Enter Spotify URI" << std::endl;
            std::cin >> uri;

            m_.getTracks( uri, this, reqId_++ );
            break;
        }
        case 'p':
        {
            std::string uri;
            std::cout << "Enter Spotify URI ('w' for wonderwall)" << std::endl;
            std::cin >> uri;

            if (uri == "w") uri = "spotify:track:2CT3r93YuSHtm57mjxvjhH";
            m_.play( uri );
            break;
        }
        case 'a':
        {
            std::string uri;
            std::cout << "Enter Spotify URI ('w' for wonderwall)" << std::endl;
            std::cin >> uri;

            if (uri == "w") uri = "spotify:album:1f4I0SpE0O8yg4Eg2ywwv1";
            m_.getAlbum( uri, this, reqId_++ );
            break;
        }

        case '?':
        {
            std::string query;
            std::cout << "Write your search query, end with enter:" << std::endl;
            std::cin >> query;

            m_.search( query, this, reqId_++ );
            break;
        }

        case 'l':
        {
            m_.addAudio();
            break;
        }

        case 'q':
            cancelThread();
            continue;

        default:
            continue;
        }
    }

    log(LOG_NOTICE) << "Exiting UI";
}

void UIConsole::rootFolderUpdatedInd( Folder& f )
{
    for( LibSpotify::FolderContainer::iterator it = f.getFolders().begin(); it != f.getFolders().end() ; it++)
        playlists.insert( playlists.end(), (*it).getPlaylists().begin(), (*it).getPlaylists().end());

    playlists.insert( playlists.end(), f.getPlaylists().begin(), f.getPlaylists().end());

    itPlaylists_ = playlists.begin();
}

void UIConsole::connectionState( bool up )
{}
void UIConsole::getTracksResponse( MediaInterfaceRequestId reqId, const std::deque<Track>& tracks )
{}
void UIConsole::getImageResponse( MediaInterfaceRequestId reqId, const void* data, size_t dataSize )
{}
void UIConsole::getAlbumResponse( MediaInterfaceRequestId reqId, const Album& album )
{}
void UIConsole::genericSearchCallback( MediaInterfaceRequestId reqId, std::deque<Track>& listOfTracks, const std::string& didYouMean)
{}

