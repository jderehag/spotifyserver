/*
 * SocketClient.cpp
 *
 *  Created on: 9 dec 2012
 *      Author: Jesse
 */

#include "SocketClient.h"
#include "Platform/Socket/Socket.h"
#include "MessageFactory/Message.h"
#include "MessageFactory/MessageDecoder.h"
#include "MessageFactory/MessageEncoder.h"
#include "MessageFactory/SocketReader.h"
#include "MessageFactory/SocketWriter.h"
#include "applog.h"


SocketClient::SocketClient(std::string serveraddr, int serverport) : serveraddr_(serveraddr), serverport_(serverport)
{
    startThread();
}

SocketClient::~SocketClient()
{
}

void SocketClient::run()
{
    while(isCancellationPending() == false)
    {
        Socket socket;

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

            if ( subscriber_ ) subscriber_->connectionState( true );

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
                                PendingMessageMap::iterator msgIt = pendingMessageMap_.find(msg->getId());
                                if (msgIt != pendingMessageMap_.end())
                                {
                                    Message* req = msgIt->second;
                                    if ( subscriber_ ) subscriber_->receivedResponse( msg, req );
                                    pendingMessageMap_.erase(msgIt);
                                    delete req;
                                }
                            }
                            else
                            {
                                if ( subscriber_ ) subscriber_->receivedMessage( msg );
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
                            MessageEncoder* encoder = msg->encode();
                            encoder->printHex();
                            log(LOG_DEBUG) << *msg;
                            if ( MSG_IS_REQUEST( msg->getType() ) )
                            {
                                pendingMessageMap_[msg->getId()] = msg;
                            }
                            else
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
        }
        socket.Close();
        if ( subscriber_ ) subscriber_->connectionState( false );
    }
}

void SocketClient::destroy()
{
    cancelThread();
    joinThread();
}
