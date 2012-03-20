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

#include "ClientHandler.h"
#include "applog.h"
#include <stdlib.h>
/*#include <iostream>*/
#include <string.h>

ClientHandler::ClientHandler(const ConfigHandling::NetworkConfig& config, LibSpotifyIf& spotifyif) : config_(config),
                                                                                                     spotify_(spotifyif),
                                                                                                     socket_(NULL)
{
    startThread();
}

ClientHandler::~ClientHandler()
{
    while(!clients_.empty())
    {
        Client* c = clients_.front();
        delete c;
        clients_.pop_front();
    }
}

void ClientHandler::run()
{
    Socket* clientsock;
    int rc = -1;
    bool activity=false;;

    socket_ = new Socket();

    if (config_.getBindType() == ConfigHandling::NetworkConfig::IP)
    {
        rc = socket_->BindToAddr(config_.getIp(), config_.getPort());
    }
    else if (config_.getBindType() == ConfigHandling::NetworkConfig::DEVICE)
    {
        rc = socket_->BindToDevice(config_.getDevice(), config_.getPort());
    }

    if ( rc < 0 )
        exit(1);

    rc = socket_->Listen();

    if ( rc < 0 )
        exit(1);

    log(LOG_NOTICE) << "Server is listening";

    while (isCancellationPending() == false)
    {
        std::set<Socket*> readsockets;
        std::set<Socket*> writesockets;
        std::set<Socket*> errsockets;
        std::list<Client*>::iterator it;

        readsockets.insert(socket_);
        errsockets.insert(socket_);

        for (it = clients_.begin(); it != clients_.end(); it++)
        {
            readsockets.insert((*it)->getSocket());
            if ((*it)->pendingSend())
                writesockets.insert((*it)->getSocket());
        }
        activity=false;
        rc = select(&readsockets, &writesockets, &errsockets, ((activity) ? 0 : 100));

        if ( rc < 0 )
            break;

        activity = false;

        /* Check listen socket first */
        if (errsockets.find(socket_) != errsockets.end())
            break; /*uh-oh we're shutting down*/

        if (readsockets.find(socket_) != readsockets.end())
        {
            /* read on listen socket, accept! */
            clientsock = socket_->Accept();
            if (clientsock == NULL)
            {
                log(LOG_WARN) << "Error on accept!";
                break;
            }
            else
            {
                Client* c = new Client(clientsock, spotify_);
                c->setUsername(config_.getUsername());
                c->setPassword(config_.getPassword());
                clients_.push_front(c);
            }
        }

        it = clients_.begin();
        while ( it != clients_.end() )
        {
            if (readsockets.find((*it)->getSocket()) != readsockets.end())
            {
                if ((*it)->doRead() < 0)
                {
                    log(LOG_NOTICE) << "read failed, removing client";
                    delete (*it);
                    it = clients_.erase(it);
                    continue;
                }
                activity = true;
            }
            if (writesockets.find((*it)->getSocket()) != writesockets.end())
            {
                if ((*it)->doWrite() < 0)
                {
                    log(LOG_NOTICE) << "write failed, removing client";
                    delete (*it);
                    it = clients_.erase(it);
                    continue;
                }
                activity = true;
            }

            it++;
        }
    }


    log(LOG_DEBUG) << "Exit ClientHandler::run()";
}

void ClientHandler::destroy()
{
    cancelThread();
    if(socket_ != NULL) socket_->Shutdown();
    joinThread();
}

