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

#include "SocketClient.h"
#include "Platform/Socket/Socket.h"
#include "MessageFactory/Message.h"
#include "MessageFactory/MessageDecoder.h"
#include "MessageFactory/MessageEncoder.h"
#include "MessageFactory/SocketReader.h"
#include "MessageFactory/SocketWriter.h"
#include "applog.h"


SocketClient::SocketClient(const std::string& serveraddr, const std::string& serverport) : serveraddr_(serveraddr), serverport_(serverport)
{
    startThread();
}

SocketClient::~SocketClient()
{
}

void SocketClient::run()
{
    while( serveraddr_ == "" && isCancellationPending() == false)
    {
        uint8_t msg[] = {'?'};
        Socket socket( SOCKTYPE_DATAGRAM );

        socket.SendTo( msg, sizeof(msg), "255.255.255.255", "7788");

        std::set<Socket*> readset;

        readset.insert( &socket );

        if ( select(&readset, NULL, NULL, 10000) > 0 )
        {
            if ( readset.find( &socket ) != readset.end() )
            {
                std::string addr, port;
                if ( socket.ReceiveFrom( msg, 1, addr, port ) == 1 )
                {
                    serveraddr_ = addr;
                }
            }
        }
    }

    while(isCancellationPending() == false)
    {
        Socket socket(SOCKTYPE_STREAM);

        if ( socket.Connect( serveraddr_, serverport_ ) == 0 )
        {
            log(LOG_NOTICE) << "Connected";
#if 0
            Message hello(HELLO_REQ);

            hello.setId(messageId++);

            hello.addTlv(TLV_PROTOCOL_VERSION_MAJOR, PROTOCOL_VERSION_MAJOR);
            hello.addTlv(TLV_PROTOCOL_VERSION_MINOR, PROTOCOL_VERSION_MINOR);
            hello.addTlv(TLV_LOGIN_USERNAME, "wonder");
            hello.addTlv(TLV_LOGIN_PASSWORD, "wall");

            MessageEncoder* enc = hello.encode();
            enc->printHex();

            if (socket.Send(enc->getBuffer(), enc->getLength()) <= 0)
            {
                log(LOG_WARN) << "Error writing to socket";
                delete enc;
                continue;
            }
            delete enc;
#endif
            SocketReader reader(&socket);
            SocketWriter writer(&socket);

            callbackSubscriberMtx_.lock();
            for( std::set<IMessageSubscriber*>::iterator it = callbackSubscriberList_.begin();
                    it != callbackSubscriberList_.end(); it++)
            {
                (*it)->connectionState( true );
            }
            callbackSubscriberMtx_.unlock();

            while(isCancellationPending() == false)
            {
                std::set<Socket*> readset;
                std::set<Socket*> writeset;
                std::set<Socket*> errorset;

                errorset.insert( &socket );
                readset.insert( &socket );
                if ( pendingSend() || !writer.isEmpty() )
                {
                    writeset.insert( &socket );
                }

                if ( select( &readset, &writeset, &errorset, 100 ) < 0 )
                    break;

                if ( !errorset.empty() )
                {
                    break;
                }

                if ( !readset.empty() )
                {
                    if ( reader.doread() < 0 )
                        break;

                    if ( reader.done() )
                    {
                        MessageDecoder decoder;

                        log(LOG_DEBUG) << "Receive complete";

                        printHexMsg(reader.getMessage(), reader.getLength());

                        Message* msg = decoder.decode(reader.getMessage());

                        if ( msg != NULL )
                        {
                            if ( MSG_IS_RESPONSE( msg->getType() ) )
                            {
                                PendingDataMap::iterator msgIt = pendingMessageMap_.find(msg->getId());
                                if (msgIt != pendingMessageMap_.end())
                                {
                                    PendingData pdata = msgIt->second;
                                    Message* req = pdata.req;
                                    void* userData = pdata.userData;
                                    /*todo: verify origin still exists*/
                                    if ( pdata.origin ) pdata.origin->receivedResponse( msg, req, userData );
                                    pendingMessageMap_.erase(msgIt);
                                    delete req;
                                }
                            }
                            else
                            {
                                callbackSubscriberMtx_.lock();
                                for( std::set<IMessageSubscriber*>::iterator it = callbackSubscriberList_.begin();
                                        it != callbackSubscriberList_.end(); it++)
                                {
                                    (*it)->receivedMessage( msg );
                                }
                                callbackSubscriberMtx_.unlock();
                            }
                            delete msg;
                        }

                        reader.reset();
                    }
                }

                if ( !writeset.empty() )
                {
                    if ( writer.isEmpty() )
                    {
                        Message* msg = popMessage();
                        if ( msg != NULL )
                        {
                            if ( !msg->hasId() )
                            {
                                log(LOG_WARN) << "Message without id!";
                            }

                            MessageEncoder* encoder = msg->encode();
                            encoder->printHex();
                            log(LOG_DEBUG) << *msg;

                            if ( !MSG_IS_REQUEST( msg->getType() ) )
                            {
                                delete msg;
                            }
                            writer.setData(encoder); // SocketWriter takes ownership of encoder
                        }
                    }

                    if ( !writer.isEmpty() && ( writer.doWrite() < 0 ) )
                    {
                        log(LOG_NOTICE) << "Write failed!";
                        break;
                    }
                }
            }
            log(LOG_NOTICE) << "Disconnected";

            callbackSubscriberMtx_.lock();
            for( std::set<IMessageSubscriber*>::iterator it = callbackSubscriberList_.begin();
                    it != callbackSubscriberList_.end(); it++)
            {
                (*it)->connectionState( false );
            }
            callbackSubscriberMtx_.unlock();
        }
        socket.Close();
    }
}

void SocketClient::destroy()
{
    cancelThread();
    joinThread();
}
