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

#include "Client.h"
#include "applog.h"
#include "MessageFactory/TlvDefinitions.h"
#include "MessageFactory/MessageEncoder.h"
#include "MessageFactory/MessageDecoder.h"


Client::Client(Socket* socket, LibSpotifyIf& spotifyif) : requestId(0),
                                                          spotify_(spotifyif),
                                                          reader_(socket),
                                                          socket_(socket)
{
    spotify_.registerForCallbacks(*this);
}

Client::~Client()
{
    log(LOG_DEBUG) << "~Client";
    delete socket_;
    spotify_.unRegisterForCallbacks(*this);
}


int Client::doRead()
{
    do
    {
        int n = reader_.doread();

        if (n == 0)
        {
            break;
        }
        else if (n < 0)
        {
            log(LOG_DEBUG) << "Socket closed";
            return -1;
        }

        if (reader_.done())
        {
            MessageDecoder m;

            log(LOG_DEBUG) << "Receive complete";

            printHexMsg(reader_.getMessage(), reader_.getLength());

            Message* msg = m.decode(reader_.getMessage());

            if (msg != NULL)
            {
                processMessage(msg);
                delete msg;
            }
            else
            {
                /*error handling?*/
            }

            reader_.reset();
        }
    } while(1);

    return 0;
}


int Client::doWrite()
{
    log(LOG_DEBUG);

    messageQueueMtx.lock();
    Message* msg = messageQueue.front();
    messageQueue.pop();
    messageQueueMtx.unlock();

    MessageEncoder* encoder = msg->encode();
    log(LOG_DEBUG) << *msg;
    delete msg;

    encoder->printHex();
    socket_->Send( encoder->getBuffer(), encoder->getLength() ); /*todo: check return value and handle partial send*/

    delete encoder;

    return 0;
}


void Client::processMessage(const Message* msg)
{
    log(LOG_NOTICE) << *(msg);

    switch(msg->getType())
    {
        case GET_PLAYLISTS_REQ:
        {
            /*get playlist*/
            Message* rsp = new Message(GET_PLAYLISTS_RSP);
            rsp->setId( msg->getId() );
            rsp->addTlv( spotify_.getRootFolder().toTlv() );
            queueMessage( rsp );
        }
        break;

        case GET_TRACKS_REQ:
        {
            GetTracksReq* req = (GetTracksReq*)msg;

            MessageEncoder* msg2 = new MessageEncoder(GET_TRACKS_RSP);
            msg2->setId(req->getId());

            log(LOG_DEBUG) << "get tracks: " << req->getPlaylist();
            spotify_.getRootFolder().writePlaylist(req->getPlaylist().c_str(), msg2);

            msg2->finalize();

            msg2->printHex();

            socket_->Send( msg2->getBuffer(), msg2->getLength() );

            delete msg2;

        }
        break;

        case PLAY_REQ:
        {
            const StringTlv* url = (const StringTlv*)msg->getTlvRoot()->getTlv(TLV_LINK);

            if ( url )
            {
                log(LOG_DEBUG) << "spotify_.play(" << url->getString() << ")";
                spotify_.play(url->getString());
            }

            Message* rsp = new Message( PLAY_RSP );
            rsp->setId( msg->getId() );
            queueMessage(rsp);
        }
        break;

        case PLAY_TRACK_REQ:
        {
            const TlvContainer* track = (const TlvContainer*) msg->getTlvRoot()->getTlv(TLV_TRACK);
            const StringTlv* url = (const StringTlv*) track->getTlv(TLV_LINK);
            log(LOG_DEBUG) << "spotify_.play(" << url->getString() << ")";
            spotify_.play(url->getString());

            Message* rsp = new Message( PLAY_TRACK_RSP );
            rsp->setId( msg->getId() );
            queueMessage(rsp);
        }
        break;

        case PLAY_CONTROL_REQ:
        {
            for (TlvContainer::const_iterator it = msg->getTlvRoot()->begin() ; it != msg->getTlvRoot()->end() ; it++)
            {
                Tlv* tlv = (*it);
                log(LOG_DEBUG) << *tlv;

                switch(tlv->getType())
                {
                    case TLV_PLAY_OPERATION:
                    {
                        switch(((IntTlv*)tlv)->getVal())
                        {
                            case PLAY_OP_NEXT:
                                spotify_.next();
                                break;
                            case PLAY_OP_PREV:
                                spotify_.previous();
                                break;
                            case PLAY_OP_PAUSE:
                                spotify_.pause();
                                break;
                            case PLAY_OP_RESUME:
                                spotify_.resume();
                                break;
                            default:
                                break;
                        }
                    }
                    break;

                    default:
                        break;
                }
            }
            Message* rsp = new Message( PLAY_CONTROL_RSP );
            rsp->setId( msg->getId() );
            queueMessage(rsp);
        }
        break;

        case GENERIC_SEARCH_REQ:
        {
            const StringTlv* query = (const StringTlv*) msg->getTlvRoot()->getTlv(TLV_SEARCH_QUERY);

            if (query && query->getString() != "")
            {
                unsigned int headerId = msg->getId();
                Message* rsp = new Message(GENERIC_SEARCH_RSP);
                rsp->setId(headerId);

                /* make sure that the pending message queue does not already contain such a message */
                if (pendingMessageMap_.find(headerId) != pendingMessageMap_.end())
                    break;

                pendingMessageMap_[headerId] = rsp;

                spotify_.genericSearch(headerId, query->getString(), *this);
            }
            else
            {
                /*error handling?*/
            }
        }
        break;

        case GET_STATUS_REQ:
        {
            Message* rsp = new Message( GET_STATUS_RSP );
            rsp->setId( msg->getId() );
            switch (spotify_.getState())
            {
                case LibSpotifyIf::TRACK_STATE_NOT_LOADED:
                case LibSpotifyIf::TRACK_STATE_WAITING_METADATA:
                    rsp->addTlv(TLV_STATE, PLAYBACK_IDLE);
                    break;
                case LibSpotifyIf::TRACK_STATE_PAUSED:
                    rsp->addTlv(TLV_STATE, PLAYBACK_PAUSED);
                    rsp->addTlv(spotify_.getCurrentTrack().toTlv());
                    rsp->addTlv(TLV_PROGRESS, spotify_.getProgress());
                    break;
                case LibSpotifyIf::TRACK_STATE_PLAYING:
                    rsp->addTlv(TLV_STATE, PLAYBACK_PLAYING);
                    rsp->addTlv(spotify_.getCurrentTrack().toTlv());
                    rsp->addTlv(TLV_PROGRESS, spotify_.getProgress());
                    break;
            }
            queueMessage(rsp);
        }
        break;

        case GET_IMAGE_REQ:
        {
            const StringTlv* link = (const StringTlv*) msg->getTlvRoot()->getTlv(TLV_LINK);

            unsigned int headerId = msg->getId();
            Message* rsp = new Message(GET_IMAGE_RSP);
            rsp->setId(headerId);

            /* make sure that the pending message queue does not already contain such a message */
            if (pendingMessageMap_.find(headerId) != pendingMessageMap_.end())
                break;

            pendingMessageMap_[headerId] = rsp;

            spotify_.getImage(headerId, link ? link->getString() : std::string(""), *this);
        }
        break;

        default:
            break;
    }
}

void Client::queueMessage(Message* msg)
{
    messageQueueMtx.lock();
    messageQueue.push(msg);
    messageQueueMtx.unlock();
}

bool Client::pendingSend()
{
    bool ret;
    messageQueueMtx.lock();
    ret = !messageQueue.empty();
    messageQueueMtx.unlock();
    return ret;
}


void Client::rootFolderUpdatedInd()
{
    log(LOG_DEBUG) << "Client::rootFolderUpdatedInd()";
}
void Client::playingInd(Track& currentTrack)
{
    Message* msg = new Message(STATUS_IND);

    log(LOG_DEBUG) << "Client::playingInd()";

    msg->setId(requestId++);

    msg->addTlv(TLV_STATE, PLAYBACK_PLAYING);

    msg->addTlv(currentTrack.toTlv());
    msg->addTlv(TLV_PROGRESS, spotify_.getProgress());

    queueMessage(msg);
}
void Client::trackEndedInd()
{
    Message* msg = new Message(STATUS_IND);

    log(LOG_DEBUG) << "Client::trackEndedInd()";

    msg->setId(requestId++);

    msg->addTlv(TLV_STATE, PLAYBACK_IDLE);
    msg->addTlv(TLV_PROGRESS, spotify_.getProgress());

    queueMessage(msg);
}
void Client::pausedInd(Track& currentTrack)
{
    Message* msg = new Message(STATUS_IND);

    log(LOG_DEBUG) << "Client::pausedInd()";

    msg->setId(requestId++);

    msg->addTlv(TLV_STATE, PLAYBACK_PAUSED);
    msg->addTlv(currentTrack.toTlv());
    msg->addTlv(TLV_PROGRESS, spotify_.getProgress());

    queueMessage(msg);
}
void Client::getTrackResponse()
{
    log(LOG_DEBUG) << "Client::getTrackResponse()";
}
void Client::getImageResponse(unsigned int reqId, const void* data, size_t dataSize)
{
    log(LOG_DEBUG) << "Client::getImageResponse() " << dataSize << " bytes";
    PendingMessageMap::iterator msgIt = pendingMessageMap_.find(reqId);
    if (msgIt != pendingMessageMap_.end())
    {
        Message* msg = msgIt->second;
        if(data && dataSize)
        {
            TlvContainer* image = new TlvContainer(TLV_IMAGE);
            image->addTlv(TLV_IMAGE_FORMAT, IMAGE_FORMAT_JPEG);
            image->addTlv(new BinaryTlv(TLV_IMAGE_DATA, (const uint8_t*)data, (uint32_t)dataSize));
            msg->addTlv(image);
        }
        queueMessage(msg);
        pendingMessageMap_.erase(msgIt);
    }
    else log(LOG_WARN) << "Could not match the genericSearchCallback() to a pending response";

}
void Client::genericSearchCallback(unsigned int reqId, std::deque<Track>& tracks, const std::string& didYouMean)
{
    log(LOG_DEBUG) << "\tdid you mean:" << didYouMean;

    PendingMessageMap::iterator msgIt = pendingMessageMap_.find(reqId);
    if (msgIt != pendingMessageMap_.end())
    {
        Message* msg = msgIt->second;
        for (std::deque<Track>::const_iterator trackIt = tracks.begin(); trackIt != tracks.end(); trackIt++)
        {
            log(LOG_DEBUG) << "\t" << (*trackIt).getName();
            msg->addTlv((*trackIt).toTlv());
        }
        log(LOG_DEBUG) << "#tracks found=" << tracks.size();

        queueMessage(msg);
        pendingMessageMap_.erase(msgIt);
    }
    else log(LOG_WARN) << "Could not match the genericSearchCallback() to a pending response";
}
