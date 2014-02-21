/*
 * Copyright (c) 2014, Jens Nielsen
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

#include "lwip/sockets.h"
#include "../Socket.h"
#include "LwipSocketPimpl.h"
#include <string.h>


int select(std::set<Socket*>* readsockets, std::set<Socket*>* writesockets, std::set<Socket*>* errsockets, int timeout)
{
    int maxfd = 0;
    int rc;
    fd_set readfds, writefds, errfds;
    std::set<Socket*>::iterator it;
    struct timeval tmo;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&errfds);

    if (readsockets)
    {
        for (it = readsockets->begin(); it != readsockets->end(); it++)
        {
            FD_SET((*it)->socket_->fd, &readfds);
            if ((*it)->socket_->fd > maxfd) maxfd = (*it)->socket_->fd;
        }
    }

    if (writesockets)
    {
        for (it = writesockets->begin(); it != writesockets->end(); it++)
        {
            FD_SET((*it)->socket_->fd, &writefds);
            if ((*it)->socket_->fd > maxfd) maxfd = (*it)->socket_->fd;
        }
    }

    if (errsockets)
    {
        for (it = errsockets->begin(); it != errsockets->end(); it++)
        {
            FD_SET((*it)->socket_->fd, &errfds);
            if ((*it)->socket_->fd > maxfd) maxfd = (*it)->socket_->fd;
        }
    }

    tmo.tv_sec = timeout/1000;
    tmo.tv_usec = (timeout % 1000) * 1000;

    if ( (rc = lwip_select(maxfd+1, &readfds, &writefds, &errfds, &tmo) ) < 0 )
    {
        return rc;
    }

    if (readsockets)
    {
        it = readsockets->begin();
        while (it != readsockets->end())
        {
            if (!FD_ISSET((*it)->socket_->fd, &readfds))
                readsockets->erase(it++);
            else
                it++;
        }
    }

    if (writesockets)
    {
        it = writesockets->begin();
        while (it != writesockets->end())
        {
            if (!FD_ISSET((*it)->socket_->fd, &writefds))
            {
                writesockets->erase(it++);
            }
            else
                it++;
        }
    }

    if (errsockets)
    {
        it = errsockets->begin();
        while (it != errsockets->end())
        {
            if (!FD_ISSET((*it)->socket_->fd, &errfds))
                errsockets->erase(it++);
            else
                it++;
        }
    }

    return rc;
}

