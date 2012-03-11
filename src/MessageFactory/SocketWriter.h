/*
 * SocketWriter.h
 *
 *  Created on: Mar 10, 2012
 *      Author: magnus
 */

#ifndef SOCKETWRITER_H_
#define SOCKETWRITER_H_
class Socket;
class MessageEncoder;

class SocketWriter
{
public:
    SocketWriter(Socket* socket);
    virtual ~SocketWriter();

    int doWrite();
    void setData(MessageEncoder* messageEncoder);
    bool isEmpty();
private:
    void reset();

    Socket* socket_;
    MessageEncoder *messageEncoder_;
    unsigned int sentLen;
};

#endif /* SOCKETWRITER_H_ */
