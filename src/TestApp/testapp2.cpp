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
#include "applog.h"

class SocketListener : public Platform::Runnable
{
private:
    Socket* socket_;
public:
    SocketListener(Socket* socket);
    ~SocketListener();

    void run();
    void destroy();
};

SocketListener::SocketListener(Socket* socket) : socket_(socket)
{
    startThread();
}
SocketListener::~SocketListener()
{
}

void SocketListener::destroy()
{
    cancelThread();
    joinThread();
}

void SocketListener::run()
{
    SocketReader reader(socket_);

    while(isCancellationPending() == false)
    {
        std::set<Socket*> l;
        l.insert(socket_);

        if ( select(&l, NULL, NULL, 100) < 0 )
            cancelThread();

        if ( reader.doread() < 0 )
            cancelThread();

        if(reader.done())
        {
            MessageDecoder m;

            log(LOG_DEBUG) << "Receive complete";

            printHexMsg(reader.getMessage(), reader.getLength());

            Message* msg = m.decode(reader.getMessage());

            if ( msg != NULL )
            {
                log(LOG_DEBUG) << *msg;
                delete msg;
            }

            reader.reset();
        }
    }

    log(LOG_WARN) << "Exiting";
}


class UIConsole : public Platform::Runnable
{
private:
    Socket* socket_;
public:
    UIConsole();
    ~UIConsole();

    void setSocket(Socket* socket) { socket_ = socket; }

    void run();
    void destroy();
};


UIConsole::UIConsole() : socket_(NULL)
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

#define PUT(x, v) ((*(u_int32_t*)x) = htonl(v))

void UIConsole::run()
{
    int seqnum=0;
    int n;
    char c;

    while(isCancellationPending() == false)
    {
        std::cout << "'g' get playlists\n"
                     "'t' get tracks\n"
                     "'p' load a play queue\n"
                     "'?' search using query\n"
                     "'s' get status\n"
                     "'z' previous\n"
                     "'x' play\n"
                     "'c' pause\n"
                     "'v' next\n" << std::endl;

        c = getchar();

        //std::cout << "fuuuuuuu" << std::endl;
        Message msg;
        switch(c)
        {
        case 'i':
        {
            std::string uri;
            std::cout << "Enter Album URI" << std::endl;
            std::cin >> uri;

            msg.setType(GET_IMAGE_REQ);
            msg.addTlv(TLV_LINK, uri);
            break;
        }

        case 'z':
        {
            msg.setType(PLAY_CONTROL_REQ);
            msg.addTlv(TLV_PLAY_OPERATION, PLAY_OP_PREV);
            break;
        }
        case 'v':
        {
            msg.setType(PLAY_CONTROL_REQ);
            msg.addTlv(TLV_PLAY_OPERATION, PLAY_OP_NEXT);
            break;
        }
        case 'x':
        {
            msg.setType(PLAY_CONTROL_REQ);
            msg.addTlv(TLV_PLAY_OPERATION, PLAY_OP_RESUME);
            break;
        }
        case 'c':
        {
            msg.setType(PLAY_CONTROL_REQ);
            msg.addTlv(TLV_PLAY_OPERATION, PLAY_OP_PAUSE);
            break;
        }

        case 's':
        {
            msg.setType(GET_STATUS_REQ);
            break;
        }
        case 'g':
        {
            msg.setType(GET_PLAYLISTS_REQ);
            break;
        }
        case 't':
        {
            msg.setType(GET_TRACKS_REQ);
            TlvContainer* p = new TlvContainer(TLV_PLAYLIST);
            p->addTlv(TLV_LINK, std::string("spotify:playlist:BestOfOasis"));
            msg.addTlv(p);
            break;
        }
        case 'p':
        {
            std::string uri;
            std::cout << "Enter Spotify URI ('w' for wonderwall)" << std::endl;
            std::cin >> uri;

            if (uri == "w") uri = "spotify:track:2CT3r93YuSHtm57mjxvjhH";
            msg.setType(PLAY_REQ);
            msg.addTlv(TLV_LINK, uri);
            break;
        }

        case '?':
        {
            std::string query;
            std::cout << "Write your search query, end with enter:" << std::endl;
            std::cin >> query;

            msg.setType(GENERIC_SEARCH_REQ);
            msg.addTlv(TLV_SEARCH_QUERY, query);
            break;
        }

        case 'q':
            cancelThread();
            continue;

        default:
            continue;
        }

        if(socket_)
        {
            msg.setId(seqnum++);

            MessageEncoder* enc = msg.encode();
            log(LOG_NOTICE) << "Sending: ";
            enc->printHex();

            n = socket_->Send(enc->getBuffer(), enc->getLength());
            if (n <= 0)
            {
                std::cout << "ERROR writing to socket" << std::endl;
            }
            delete enc;
        }

    }

    log(LOG_NOTICE) << "Exiting UI";
}


Socket* connect(std::string servaddr)
{
    Socket* socket;

    socket = new Socket();
    if (socket->Connect(servaddr, 7788) < 0)
    {
        delete socket;
        socket = NULL;
    }

    return socket;
}

#include <unistd.h> /*todo*/

int main(int argc, char *argv[])
{
    Socket* socket;
    std::string servaddr("127.0.0.1");
    ConfigHandling::LoggerConfig cfg;
    cfg.setLogTo(ConfigHandling::LoggerConfig::STDOUT);
    Logger::Logger logger(cfg);

    if(argc > 1)
        servaddr = std::string(argv[1]);

    /*for some freaky reason I have to try to connect once before UIConsole is created, otherwise it will segfault in getchar on cygwin....*/
    socket = connect(servaddr);

    UIConsole ui;

    do
    {
        if(socket == NULL)
        {
            //std::cout << "Trying to connect..." << std::endl;
            socket = connect(servaddr);
        }

        if(socket == NULL)
        {
            continue;
        }

        std::cout << "Connected" << std::endl;

        //UIConsole ui;
        ui.setSocket(socket);

        SocketListener listener(socket);

        while ( !ui.isCancellationPending() && !listener.isCancellationPending() ) usleep(10000);

        std::cout << "Disconnected" << std::endl;

        ui.setSocket(NULL);

        //ui.destroy();
        listener.destroy();

        delete socket;
        socket = NULL;

    //} while ( 1 );
    } while ( !ui.isCancellationPending() );

    std::cout << "Exiting" << std::endl;

    /* cleanup */
    ui.destroy();

    return 0;
}
