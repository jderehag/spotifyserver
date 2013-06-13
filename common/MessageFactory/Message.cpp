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


#include "Message.h"

Message::Message() : type_((MessageType_t)0xffffffff),
                     id_(0xffffffff)
{
}

Message::Message(MessageType_t type) : type_(type),
                                       id_(0xffffffff)
{
}

Message::Message(MessageType_t type, uint32_t id) : type_(type),
                                                    id_(id)
{
}

Message::~Message()
{
}

Message* Message::createResponse() const
{
    if ( MSG_IS_REQUEST( type_ ) )
    {
        return new Message( (MessageType_t)(type_ | RSP_BIT), id_ );
    }
    else
    {
        return NULL;
    }
}

void Message::setType (MessageType_t type) { type_ = type; }
MessageType_t Message::getType(void) const { return type_; }

void Message::setId( uint32_t id ) { id_ = id; }
uint32_t Message::getId( void ) const { return id_; }
bool Message::hasId( void ) const { return ( id_ != 0xffffffff ); }

const TlvContainer* Message::getTlvRoot() const { return &tlvs; }
const Tlv* Message::getTlv(TlvType_t tlv) const { return tlvs.getTlv(tlv); }

void Message::addTlv(Tlv* tlv) { tlvs.addTlv(tlv); }
void Message::addTlv(TlvType_t type, const std::string& str) { tlvs.addTlv(type, str); }
void Message::addTlv(TlvType_t type, const uint8_t* str, uint32_t len) { tlvs.addTlv(type, str, len); }
void Message::addTlv(TlvType_t type, uint32_t val) { tlvs.addTlv(type, val); }

bool Message::validate()
{
    return true;
}



MessageEncoder* Message::encode()
{
    MessageEncoder* msg = new MessageEncoder(type_);
    msg->setId(id_);
    tlvs.encode(msg);
    msg->finalize();
    return msg;
}



GetTracksReq::GetTracksReq()
{
}

GetTracksReq::~GetTracksReq()
{
}

const std::string GetTracksReq::getPlaylist()
{
    TlvContainer* playlist = (TlvContainer*)tlvs.getTlv(TLV_PLAYLIST);
    const StringTlv* link = NULL;
    if (playlist)
    {
        link = (const StringTlv*)playlist->getTlv(TLV_LINK);
    }

    if (link)
    {
        return link->getString();
    }
    return std::string("");
}

bool GetTracksReq::validate()
{
    /*todo: check that it has a playlist with a link*/
    return true;
}


std::ostream& operator <<(std::ostream& os, const Message& rhs)
{
    os << messageTypeToString(rhs.getType()) << " (0x" << std::hex << rhs.getType() << ") "  << std::dec << " # " << rhs.getId() << '\n';
    os << rhs.getTlvRoot()->print();
    return os;
}


