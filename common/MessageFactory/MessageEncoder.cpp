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

#include "MessageEncoder.h"
#include "Platform/Socket/Socket.h"
#include "applog.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#ifndef ntohl
#define ntohl Ntohl
#endif
#ifndef htonl
#define htonl Htonl
#endif

struct tlvgroup_s
{
    unsigned int startpos;
};

/*
 * Constructors, initialiser, destructor
 */
void MessageEncoder::init()
{
    msgbuf = (uint8_t*) malloc(1000);
    cursize = 1000;
    wpos = sizeof(header_t);
}

MessageEncoder::MessageEncoder()
{
    init();
}

MessageEncoder::MessageEncoder(MessageType_t type)
{
    init();
    getHeader()->type = htonl(type);
}

MessageEncoder::MessageEncoder(const MessageEncoder& from)
{
    /* this can't possibly work..? */
    assert(0);
    cursize = from.getLength();
    msgbuf = (uint8_t*) malloc(cursize);
    wpos = cursize;
}

MessageEncoder::~MessageEncoder()
{
    free(msgbuf);
    msgbuf=NULL;
}


/*
 * Getters
 */

const uint8_t* MessageEncoder::getBuffer() const
{
    return msgbuf;
}
unsigned int MessageEncoder::getLength() const
{
    return wpos;
}


/*
 * Header manipulation
 */

void MessageEncoder::setId(unsigned int id)
{
    getHeader()->id = htonl(id);
}

header_t* MessageEncoder::getHeader()
{
    return (header_t*)(msgbuf);
}

/*
 * Encoding
 */
uint8_t* MessageEncoder::getBufferForTlv(unsigned int size)
{
    uint8_t* ret;
    unsigned int needed = wpos + sizeof (tlvheader_t) + size;

    if (needed > cursize)
    {
        void* tmp = realloc(msgbuf, (needed > cursize*2) ? needed : cursize*2);
        if (!tmp)
            return NULL;
        msgbuf = (uint8_t*) tmp;
        cursize *= 2;
    }

    ret = &msgbuf[wpos];
    wpos += sizeof(tlvheader_t) + size;

    return ret;
}

void MessageEncoder::encode(TlvType_t tlv, unsigned int val)
{
    tlvheader_t* header = (tlvheader_t*) getBufferForTlv(4);
    uint32_t* val_;
    header->type = htonl(tlv);
    header->len = htonl(4);

    val_ = (uint32_t*) (header+1);
    *val_ = htonl(val);
}

void MessageEncoder::encode(TlvType_t tlv, const std::string& str)
{
    /*encode padded to even 32 bit*/
    uint32_t len = str.length() + 4 - (str.length()%4);
    tlvheader_t* header = (tlvheader_t*) getBufferForTlv(len);
    char* val_;
    header->type = htonl(tlv);
    header->len = htonl(len);

    val_ = (char*) (header+1);

    strcpy(val_, str.c_str());

    for (unsigned int i=str.length();i<len;i++)
    {
        val_[i] = '\0';
    }
}

void MessageEncoder::encode(TlvType_t tlv, const uint8_t* data, uint32_t len)
{
    tlvheader_t* header = (tlvheader_t*) getBufferForTlv(len);
    header->type = htonl(tlv);
    header->len = htonl(len);
    void* val_ = (void*) (header+1);

    memcpy(val_, data, len);
}


tlvgroup_t* MessageEncoder::createNewGroup(TlvType_t tlv)
{
    tlvgroup_t* group = (tlvgroup_t*)malloc(sizeof(tlvgroup_t));
    group->startpos = wpos;

    tlvheader_t* header = (tlvheader_t*) getBufferForTlv(0);
    header->type = htonl(tlv);

    return group;
}

void MessageEncoder::finalizeGroup(tlvgroup_t* group)
{
    tlvheader_t* header = (tlvheader_t*)&msgbuf[group->startpos];
    header->len = (uint32_t) htonl(wpos - (group->startpos + sizeof(tlvheader_t)));

    free(group);
}

void MessageEncoder::finalize()
{
    getHeader()->len = htonl(wpos);
}




void MessageEncoder::printHex()
{
    uint32_t len = ntohl(getHeader()->len);
    printHexMsg((uint8_t*)msgbuf, len);
}

void printHexMsg(const uint8_t* msgbuf, uint32_t len)
{
    std::string message;
    int rowCount = 0;
    if ( logger->getConfiguredLogLevel() < LOG_DEBUG )
        return;

    for(uint32_t i=0; i<len; i++)
    {
        char s[3];

        //sprintf(s, "%.2x", wbuf[i]);
        if(((msgbuf[i] >> 4) & 0xf) <= 9)
            s[0] = ((msgbuf[i] >> 4) & 0xf) + '0';
        else
            s[0] = ((msgbuf[i] >> 4) & 0xf) - 10 + 'a';

        if((msgbuf[i] & 0xf) <= 9)
            s[1] = (msgbuf[i] & 0xf) + '0';
        else
            s[1] = (msgbuf[i] & 0xf) - 10 + 'a';

        s[2]  = '\0';
        message += s;

        if(i%32 == 31)
        {
            message += '\n';
            rowCount++;
            if ( rowCount >= 16 )
            {
                message += "Printout truncated...";
                break;
            }
        }
        else if(i%4 == 3)
            message += " ";
    }

    log(LOG_DEBUG) << "Message type " << messageTypeToString(static_cast<MessageType_t>(ntohl(((header_t*)msgbuf)->type))) << "=0x" << std::hex << ntohl(((header_t*)msgbuf)->type) << ", length " << std::dec << len <<
                        " bytes, id " << ntohl(((header_t*)msgbuf)->id) << " message:\n" << message;

}
