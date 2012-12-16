/*
 * SocketClient.h
 *
 *  Created on: 9 dec 2012
 *      Author: Jesse
 */

#ifndef SOCKETCLIENT_H_
#define SOCKETCLIENT_H_

#include "Messenger.h"
#include "Platform/Threads/Runnable.h"
#include <string>
#include <map>

class SocketClient : public Messenger, public Platform::Runnable
{
private:
    std::string serveraddr_;
    int serverport_;

    /* pending requests sent to other side of connection */
    typedef std::map<unsigned int, Message*>  PendingMessageMap;
    PendingMessageMap pendingMessageMap_;

public:
    SocketClient(std::string serveraddr, int serverport);
    virtual ~SocketClient();

    void run();
    void destroy();

};

#endif /* SOCKETCLIENT_H_ */
