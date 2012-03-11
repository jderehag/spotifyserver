/*
 * SocketWriter.cpp
 *
 *  Created on: Mar 10, 2012
 *      Author: magnus
 */

#include <string.h>

#include "SocketWriter.h"
#include "Platform/Socket/Socket.h"
#include "MessageEncoder.h"
#include "applog.h"


#define WRITE_MAX_BYTES (4096)

SocketWriter::SocketWriter(Socket* socket) : socket_(socket), messageEncoder_(NULL), sentLen(0)
{
}

SocketWriter::~SocketWriter()
{
}

int SocketWriter::doWrite()
{
    log(LOG_DEBUG);
    if (messageEncoder_ == NULL)
        return 0;

    int toWrite = messageEncoder_->getLength() - sentLen;
    const char *fromBuf = messageEncoder_->getBuffer() + sentLen;

    if (toWrite > WRITE_MAX_BYTES)
        toWrite = WRITE_MAX_BYTES;

    int len = socket_->Send(fromBuf, toWrite);

    if (len > 0)
    {
        sentLen += len ;
        log(LOG_DEBUG) << "Sent: " << len << " bytes";

        if (sentLen >= messageEncoder_->getLength())
        {
            log(LOG_DEBUG) << "Send complete " << messageEncoder_->getLength() << " bytes";
            reset();
        }
        return 1;
    }
    else if (len < 0)
    {
        log(LOG_DEBUG) << "Send error";
        reset();
        return -1;
    }

    return 0;
}

void SocketWriter::setData(MessageEncoder* messageEncoder)
{
    if (messageEncoder_ != NULL)
    {
        log(LOG_WARN) << "Overwriting unsent data. use isEmpty() before calling me like this";
        reset();
    }

    messageEncoder_ = messageEncoder;
}


bool SocketWriter::isEmpty()
{
    return (messageEncoder_ == NULL);
}

void SocketWriter::reset()
{
    if (messageEncoder_ != NULL)
    {
        delete messageEncoder_;
        messageEncoder_ = NULL;
    }

    sentLen = 0;
}
