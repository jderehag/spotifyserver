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

#include "Platform/Threads/Runnable.h"
#include "Platform/Socket/Socket.h"
#include "MessageFactory/Message.h"
#include "MessageFactory/MessageDecoder.h"
#include "MessageFactory/MessageEncoder.h"
#include "MessageFactory/SocketReader.h"
#include "Messenger.h"
#include "IUserInterface.h"
#include "applog.h"



class UIConsole : public Platform::Runnable, public IUserInterface
{
private:
    LibSpotify::PlaylistContainer playlists;
    LibSpotify::PlaylistContainer::iterator itPlaylists_;
public:
    UIConsole(Messenger& m);
    ~UIConsole();

    void run();
    void destroy();

    void updateRootFolder(Folder& f);

};


UIConsole::UIConsole(Messenger& m) : IUserInterface(m),
        itPlaylists_(playlists.begin())
{
    startThread();
}
UIConsole::~UIConsole()
{
}

void UIConsole::destroy()
{
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
                     "'v' next\n" << std::endl;

        c = getchar();

        switch(c)
        {
        case 'i':
        {
            std::string uri;
            std::cout << "Enter Album URI" << std::endl;
            std::cin >> uri;

            getImage( uri );
            break;
        }

        case 'z':
        {
            previous();
            break;
        }
        case 'v':
        {
            next();
            break;
        }
        case 'x':
        {
            resume();
            break;
        }
        case 'c':
        {
            pause();
            break;
        }

        case 's':
        {
            getStatus();
            break;
        }
        case 'g':
        {
            getPlaylists();
            break;
        }
        case 't':
        {
            getTracks("spotify:playlist:BestOfOasis");
            break;
        }
        case 'p':
        {
            std::string uri;
            std::cout << "Enter Spotify URI ('w' for wonderwall)" << std::endl;
            std::cin >> uri;

            if (uri == "w") uri = "spotify:track:2CT3r93YuSHtm57mjxvjhH";
            play( uri );
            break;
        }
        case 'a':
        {
            std::string uri;
            std::cout << "Enter Spotify URI ('w' for wonderwall)" << std::endl;
            std::cin >> uri;

            if (uri == "w") uri = "spotify:album:1f4I0SpE0O8yg4Eg2ywwv1";
            getAlbum( uri );
            break;
        }

        case '?':
        {
            std::string query;
            std::cout << "Write your search query, end with enter:" << std::endl;
            std::cin >> query;

            search( query );
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

void UIConsole::updateRootFolder(Folder& f)
{
    for( LibSpotify::FolderContainer::iterator it = f.getFolders().begin(); it != f.getFolders().end() ; it++)
        playlists.insert( playlists.end(), (*it).getPlaylists().begin(), (*it).getPlaylists().end());

    playlists.insert( playlists.end(), f.getPlaylists().begin(), f.getPlaylists().end());

    itPlaylists_ = playlists.begin();
}

#include <unistd.h> /*todo*/

int main(int argc, char *argv[])
{
    std::string servaddr("127.0.0.1");
    ConfigHandling::LoggerConfig cfg;
    cfg.setLogTo(ConfigHandling::LoggerConfig::STDOUT);
    Logger::Logger logger(cfg);

    if(argc > 1)
        servaddr = std::string(argv[1]);

    Messenger m(servaddr);
    UIConsole ui(m);

    do
    {
        usleep(10000);
    } while ( !ui.isCancellationPending() );

    std::cout << "Exiting" << std::endl;

    /* cleanup */
    ui.destroy();
    m.destroy();

    return 0;
}
