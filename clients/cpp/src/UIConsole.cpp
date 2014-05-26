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
#include "Platform/Utils/Utils.h"
#include <limits.h>

static void printFolder( const Folder& f, int indent );
static void printTracks( const std::deque<Track>& tracks );


UIConsole::UIConsole( MediaInterface& m, EndpointCtrlInterface& epMgr) : 
                                            m_(m),
                                            epMgr_(epMgr),
                                            doLogin(false),
                                            isShuffle(false),
                                            isRepeat(false)
{
    m_.registerForCallbacks( *this );
    epMgr_.registerForCallbacks( *this );
    startThread();
}

UIConsole::~UIConsole()
{
}

void UIConsole::destroy()
{
    m_.unRegisterForCallbacks( *this );
    epMgr_.unRegisterForCallbacks( *this );
    cancelThread();
    joinThread();
}

bool simPacketDrop = false;
void UIConsole::run()
{
    char c;
    std::string cmd;

    consoleMtx.lock();

    while(isCancellationPending() == false)
    {
        std::vector<std::string> argv;
        std::string arg;


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

        consoleMtx.unlock();
        getline( std::cin, cmd );
        consoleMtx.lock();

        if ( doLogin )
        {
            loginRes.username = cmd;

            std::cout << "Enter password: " << std::endl;
            disableStdinEcho();
            std::cin >> loginRes.password;
            enableStdinEcho();
            std::cout << std::endl;

            loginRes.rememberMe = true;

            loginSequenceDone.signal();
            doLogin = false;
            continue;
        }
        std::istringstream iss(cmd);

        while( iss >> arg )
        {
            argv.push_back( arg );
        }

        int argc = argv.size();
        if ( argc == 0)
            continue;

        if ( argv[0] == "getEndpoints")
        {
            epMgr_.getEndpoints( this, NULL );
            continue;
        }

        if ( argv[0] == "rename")
        {
            if ( argc == 3 )
            {
                epMgr_.renameEndpoint( argv[1], argv[2], this, NULL );
            }
            continue;
        }

        if ( argv[0] == "addAudio")
        {
            std::string id = "";
            if ( argc > 1 )
                id = argv[1];
            epMgr_.addAudioEndpoint(id, this, NULL);
            continue;
        }
        if ( argv[0] == "remAudio")
        {
            std::string id = "";
            if ( argc > 1 )
                id = argv[1];
            epMgr_.removeAudioEndpoint(id, this, NULL);
            continue;
        }
        if ( argv[0] == "curAudio")
        {
            m_.getCurrentAudioEndpoints( this, NULL );
            continue;
        }
        if ( argv[0] == "getAudio")
        {
            epMgr_.getAudioEndpoints( this, NULL );
            continue;
        }

        if ( argv[0] == "setVol")
        {
            if ( argc == 2 )
                m_.setVolume( atoi( argv[1].c_str() ) );
            else if ( argc == 3 )
                epMgr_.setRelativeVolume( argv[1], atoi( argv[2].c_str() ) );
            continue;
        }

        if ( argv[0] == "seek")
        {
            if ( argc == 2 )
                m_.seek( atoi( argv[1].c_str() ) );
            continue;
        }

        if ( argv[0] == "getArtist")
        {
            if ( argc == 2 )
                m_.getArtist( argv[1], this, NULL );
            continue;
        }

        if ( argv[0] == "play")
        {
            if ( argc == 2 )
                m_.play( argv[1], this, NULL );
            continue;
        }

        if ( argv[0] == "enqueue")
        {
            if ( argc == 2 )
                m_.enqueue( argv[1], this, NULL );
            continue;
        }

        if ( argv[0] == "addTracks" )
        {
            if ( argc >= 3 )
            {
                std::string& playlist = argv[1];
                std::list<std::string> tracks;
                int index;
                char* endptr;
                for ( int i = 2; i < argc-1; i++ )
                    tracks.push_back(argv[i]);
                index = strtol(argv[argc-1].c_str(), &endptr, 10);
                if ( *endptr != '\0')
                {
                    index = -1;
                    tracks.push_back(argv[argc-1]);
                }
                if ( tracks.size() > 0 )
                    m_.playlistAddTracks( playlist, tracks, index, this, NULL );
            }
            continue;
        }

        if ( argv[0] == "removeTracks" )
        {
            if ( argc >= 3 )
            {
                std::string& playlist = argv[1];
                std::set<int> tracks;
                char* endptr;
                for ( int i = 2; i < argc; i++ )
                {
                    int index = strtol(argv[i].c_str(), &endptr, 10);
                    if ( *endptr == '\0')
                        tracks.insert(index);
                }
                if ( tracks.size() > 0 )
                    m_.playlistRemoveTracks( playlist, tracks, this, NULL );
            }
            continue;
        }

        if ( argv[0] == "moveTracks" )
        {
            if ( argc >= 4 )
            {
                std::string& playlist = argv[1];
                std::set<int> tracks;
                int toIndex;
                char* endptr;
                for ( int i = 2; i < argc-1; i++ )
                {
                    int index = strtol(argv[i].c_str(), &endptr, 10);
                    if ( *endptr == '\0')
                        tracks.insert(index);
                }
                toIndex = strtol(argv[argc-1].c_str(), &endptr, 10);
                if ( *endptr == '\0' && tracks.size() > 0 )
                    m_.playlistMoveTracks( playlist, tracks, toIndex, this, NULL );
            }
            continue;
        }

        //handle the old commands the old way for now..
        c = argv[0][0];
        switch(c)
        {
        case 'd':
            simPacketDrop = !simPacketDrop;
            std::cout << "sim drops" << simPacketDrop << std::endl;
            break;
        case 'i':
        {
            std::string uri;
            std::cout << "Enter Album URI" << std::endl;
            std::cin >> uri;

            m_.getImage( uri, this, NULL );
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
            m_.getStatus( this, NULL );
            break;
        }
        case 'g':
        {
            m_.getPlaylists( this, NULL );
            break;
        }
        case 't':
        {
            std::string uri;
            std::cout << "Enter Spotify URI" << std::endl;
            std::cin >> uri;

            m_.getTracks( uri, this, NULL );
            break;
        }
        case 'p':
        {
            std::string uri;
            std::cout << "Enter Spotify URI ('w' for wonderwall, 'r' for something better...)" << std::endl;
            std::cin >> uri;

            if (uri == "w") uri = "spotify:track:2CT3r93YuSHtm57mjxvjhH";
            else if (uri == "r" ) uri = "spotify:track:7J0UB6XoaehZBvEoD3vPrM";
            m_.play( uri, this, NULL );
            break;
        }
        case 'a':
        {
            std::string uri;
            std::cout << "Enter Spotify URI ('w' for wonderwall)" << std::endl;
            std::cin >> uri;

            if (uri == "w") uri = "spotify:album:1f4I0SpE0O8yg4Eg2ywwv1";
            m_.getAlbum( uri, this, NULL );
            break;
        }

        case '?':
        {
            std::string query;
            std::cout << "Write your search query, end with enter:" << std::endl;
            std::cin >> query;

            m_.search( query, this, NULL );
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


LibSpotifyLoginParams UIConsole::getLoginParams( const std::string& message, const std::string& oldUsername, bool oldRememberMe )
{
    LibSpotifyLoginParams res;
    
    std::cout << std::endl;
    if ( !message.empty() )
        std::cout << message << std::endl << std::endl;

    // wait for console to become available (in case the user is doing something)
    consoleMtx.lock();
    doLogin = true;
    std::cout << "Enter username: " << std::endl;
    //wait for user to enter details
    loginSequenceDone.wait(consoleMtx);
    res = loginRes;
    loginRes.password.clear(); //wipe evidence, don't leave password in memory..
    consoleMtx.unlock();
    return res;
}

void UIConsole::rootFolderUpdatedInd()
{}
void UIConsole::playlistUpdatedInd( const std::string& link )
{}
void UIConsole::connectionState( bool up )
{}
void UIConsole::getPlaylistsResponse( const Folder& rootfolder, void* userData )
{
    printFolder( rootfolder, 2 );
}
void UIConsole::getTracksResponse( const std::deque<Track>& tracks, void* userData )
{
    printTracks( tracks );
}
void UIConsole::getImageResponse( const void* data, size_t dataSize, void* userData )
{
    std::cout << "Got " << dataSize << " bytes image " << std::endl;
}
void UIConsole::getAlbumResponse( const Album& album, void* userData )
{
    std::cout << "  " << album.getName() << " - " << album.getLink() << std::endl;
    std::cout << "  By " << album.getArtist().getName() << " - " << album.getArtist().getLink() << std::endl;
    printTracks( album.getTracks() );
}
void UIConsole::getArtistResponse( const Artist& artist, void* userData )
{
    std::cout << "  " << artist.getName() << " - " << artist.getLink() << std::endl;
    AlbumContainer albums = artist.getAlbums();
    for ( AlbumContainer::const_iterator it = albums.begin();
          it != albums.end(); it++ )
    {
        std::cout << "  " << (*it).getName() << " - " << (*it).getLink() << std::endl;
        printTracks( (*it).getTracks() );
        std::cout << std::endl;
    }
}
void UIConsole::genericSearchCallback( const std::deque<Track>& listOfTracks, const std::string& didYouMean, void* userData )
{
    printTracks( listOfTracks );
}

void UIConsole::statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress )
{
    statusUpdateInd( state, repeatStatus, shuffleStatus, volume );

    std::cout << "  Current track: " << currentTrack.getName() << "  -  "  << currentTrack.getLink() << std::endl;
    std::cout << "    " << currentTrack.getAlbum() << "  -  "  << currentTrack.getAlbumLink() << std::endl;
    for ( std::vector<MediaBaseInfo>::const_iterator it = currentTrack.getArtists().begin();
            it != currentTrack.getArtists().end(); it++ )
    {
        std::cout << "    " << (*it).getName() << "  -  "  << (*it).getLink() << std::endl;
    }
    std::cout << "  Duration " << currentTrack.getDurationMillisecs() << std::endl;
    std::cout << "  Progress " << progress << std::endl << std::endl;
}
void UIConsole::statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume )
{
    isRepeat = repeatStatus;
    isShuffle = shuffleStatus;
    switch( state )
    {
        case PLAYBACK_IDLE:    std::cout << "  Playback stopped "; break;
        case PLAYBACK_PLAYING: std::cout << "  Playback playing "; break;
        case PLAYBACK_PAUSED:  std::cout << "  Playback paused  "; break;
    }
    std::cout << " - Repeat " << (repeatStatus ? "on" : "off") << ", Shuffle " << (shuffleStatus ? "on" : "off") << ", Volume " << (uint32_t)volume << std::endl << std::endl;
}
void UIConsole::getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, const Track& currentTrack, unsigned int progress, void* userData )
{
    statusUpdateInd( state, repeatStatus, shuffleStatus, volume, currentTrack, progress );
}
void UIConsole::getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, uint8_t volume, void* userData )
{
    statusUpdateInd( state, repeatStatus, shuffleStatus, volume );
}


void UIConsole::getCurrentAudioEndpointsResponse( const std::set<std::string>& endpoints, void* userData )
{
    std::cout << "Current endpoints:" << std::endl;

    for (std::set<std::string>::const_iterator it = endpoints.begin(); it != endpoints.end(); it++)
    {
        std::cout << "  " << (*it) << std::endl;
    }
    std::cout << std::endl;
}

void UIConsole::renameEndpointResponse( void* userData ) {}
void UIConsole::getEndpointsResponse( const EndpointInfoList& endpoints, void* userData )
{
    std::cout << "Endpoints:" << std::endl;

    for ( EndpointInfoList::const_iterator it = endpoints.begin(); it != endpoints.end(); it++ )
    {
        std::cout << "  " << (*it) << std::endl;
    }
    std::cout << std::endl;
}

void UIConsole::getAudioEndpointsResponse( const AudioEndpointInfoList& endpoints, void* userData )
{
    std::cout << "Audio Endpoints:" << std::endl;

    for (AudioEndpointInfoList::const_iterator it = endpoints.begin(); it != endpoints.end(); it++)
    {
        std::cout << "  " << (*it).id << ": " << ((*it).active ? "active" : "inactive") << ", vol " << (uint32_t)(*it).relativeVolume << std::endl;
    }
    std::cout << std::endl;
}

void UIConsole::audioEndpointsUpdatedNtf()
{
}

void printFolder( const Folder& folder, int indent )
{
    std::cout << std::string( indent, ' ') << folder.getName() << std::endl;

    for( LibSpotify::FolderItemContainer::const_iterator it = folder.getItems().begin(); it != folder.getItems().end() ; it++)
    {
        if ( (*it)->isFolder )
        {
            const Folder* f = dynamic_cast<const Folder*>(*it);
            printFolder( *f, indent+2 );
        }
        else
        {
            const Playlist* p = dynamic_cast<const Playlist*>(*it);
            std::cout << std::string( indent+2, ' ') << p->getName() << "  -  " << p->getLink() << std::endl;
        }
    }
}

void printTracks( const std::deque<Track>& tracks )
{
    for( std::deque<Track>::const_iterator it = tracks.begin(); it != tracks.end() ; it++)
    {
        std::cout << "  ";
        if ( !(*it).isAvailable() )
            std::cout << "(";
        if ( (*it).getIndex() >= 0 )
            std::cout << (*it).getIndex() << ". ";
        std::cout << (*it).getName() << "  -  "  << (*it).getLink();
        if ( !(*it).isAvailable() )
            std::cout << ")";

        std::cout << std::endl;
    }
}
