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

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "Tlvs.h"
#include "MessageEncoder.h"
#include <stdint.h>
#include <string>

class Message
{
private:
    MessageType_t type_;
    uint32_t id_;

protected:
    TlvRoot tlvs;

public:
    Message();
    Message(MessageType_t type);
    Message(MessageType_t type, uint32_t id);

    Message* createResponse() const;

    void setType (MessageType_t type);
    MessageType_t getType(void) const;

    void setId( uint32_t id );
    uint32_t getId( void ) const;
    bool hasId( void ) const;

    const TlvContainer* getTlvRoot() const;
    const Tlv* getTlv(TlvType_t tlv) const;

    void addTlv(Tlv* tlv);
    void addTlv(TlvType_t type, const std::string& str);
    void addTlv(TlvType_t type, const uint8_t* str, uint32_t len);
    void addTlv(TlvType_t type, uint32_t val);

    virtual bool validate();

    MessageEncoder* encode();

    virtual ~Message();

    friend class MessageDecoder;
};

class GetTracksReq : public Message
{
public:
    const std::string getPlaylist();

    GetTracksReq();
    virtual ~GetTracksReq();

    bool validate();
};


std::ostream& operator <<(std::ostream& os, const Message& rhs);

#endif /* MESSAGE_H_ */
