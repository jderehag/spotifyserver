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

#include "Tlvs.h"
#include "applog.h"
#include <sstream>
#include <string.h> //for memcpy



/*
 * Tlv Base
 */

Tlv::Tlv(TlvType_t type) : type_(type), depth_(0)
{
}

Tlv::~Tlv()
{
}

Tlv* Tlv::clone() const
{
    return new Tlv(type_);
}

TlvType_t Tlv::getType() const
{
    return type_;
}

void Tlv::setDepth(int depth)
{
    depth_ = depth;
}

void Tlv::encode(MessageEncoder* msg) const
{
    /* todo: implement msg->encode(type_);*/
}

std::string Tlv::print() const
{
    std::stringstream ss;

    ss << std::string((size_t)depth_*2, ' ');
    ss << "TLV: " << tlvTypeToString(type_) << std::endl;
    return ss.str();
}

/*
 * StringTlv
 */

StringTlv::StringTlv(TlvType_t type, const uint8_t* data, uint32_t len) : Tlv(type)
{
    if ( data[len-1] == '\0' )
        data_ = std::string((const char*) data); //null is included in len, this will give incorrect std::str.length() if initialized with (data, len)
    else
        data_ = std::string((const char*) data, (size_t)len);
}

StringTlv::StringTlv(TlvType_t type, const std::string& data) : Tlv(type), data_(data)
{

}

Tlv* StringTlv::clone() const
{
    StringTlv* clone = new StringTlv(*this);

    return clone;
}

const std::string& StringTlv::getString() const
{
    return data_;
}

void StringTlv::encode(MessageEncoder* msg) const
{
    msg->encode(type_, data_);
}

std::string StringTlv::print() const
{
    std::stringstream ss;

    ss << std::string((size_t)depth_*2, ' ');
    ss << "TLV: " << tlvTypeToString(type_) << " = " << data_ << "\n";
    return ss.str();
}


/*
 * IntTlv
 */

IntTlv::IntTlv(TlvType_t type, uint32_t data) : Tlv(type), data_(data)
{
}

Tlv* IntTlv::clone() const
{
    IntTlv* clone = new IntTlv(*this);

    return clone;
}

uint32_t IntTlv::getVal() const
{
    return data_;
}

void IntTlv::encode(MessageEncoder* msg) const
{
    msg->encode(type_, data_);
}

std::string IntTlv::print() const
{
    std::stringstream ss;

    ss << std::string((size_t)depth_*2, ' ');
    ss << "TLV: "  << tlvTypeToString(type_) << " = " << data_ << "\n";
    return ss.str();
}


/*
 * BinaryTlv
 */

BinaryTlv::BinaryTlv(TlvType_t type, const uint8_t* data, uint32_t len) : Tlv(type)
{
    data_ = new uint8_t[len];
    memcpy(data_, data, len);
    len_ = len;
}

BinaryTlv::BinaryTlv(const BinaryTlv& from) : Tlv(from)
{
    len_ = from.getLen();
    data_ = new uint8_t[len_];
    memcpy(data_, from.getData(), len_);
}

BinaryTlv::~BinaryTlv()
{
    delete[] data_;
}

/*todo: overload assignment operator*/

Tlv* BinaryTlv::clone() const
{
    BinaryTlv* clone = new BinaryTlv(*this);

    return clone;
}

uint8_t* BinaryTlv::getData() const
{
    return data_;
}

uint32_t BinaryTlv::getLen() const
{
    return len_;
}

void BinaryTlv::encode(MessageEncoder* msg) const
{
    msg->encode(type_, data_, len_);
}

std::string BinaryTlv::print() const
{
    std::stringstream ss;

    ss << std::string((size_t)depth_*2, ' ');
    ss << "TLV: " << tlvTypeToString(type_) << " = " << len_ << " bytes binary data\n"; //print the data too?
    return ss.str();
}


/*
 * TlvContainer
 */

TlvContainer::TlvContainer(TlvType_t type) : Tlv(type)
{
}

TlvContainer::TlvContainer(const TlvContainer& from) : Tlv(from)
{
    std::list<Tlv*> from_tlvs = from.getTlvs();
    for (std::list<Tlv*>::const_iterator it = from_tlvs.begin(); it != from_tlvs.end(); *it++)
    {
        addTlv((*it)->clone());
    }
}

TlvContainer::~TlvContainer()
{
    for (std::list<Tlv*>::iterator it = tlvs.begin(); it != tlvs.end(); *it++)
    {
        delete (*it);
    }
}

Tlv* TlvContainer::clone() const
{
    TlvContainer* clone = new TlvContainer(*this);

    return clone;
}

void TlvContainer::setDepth(int depth)
{
    depth_ = depth;
    for (std::list<Tlv*>::iterator it = tlvs.begin(); it != tlvs.end(); *it++)
    {
        (*it)->setDepth(depth_+1);
    }
}

void TlvContainer::addTlv(Tlv* tlv)
{
    tlv->setDepth(depth_+1);
    tlvs.push_back(tlv);
}

void TlvContainer::addTlv(TlvType_t type, const uint8_t* str, uint32_t len)
{
    StringTlv* tlv = new StringTlv(type, str, len);
    addTlv(tlv);
}

void TlvContainer::addTlv(TlvType_t type, const std::string& str)
{
    StringTlv* tlv = new StringTlv(type, str);
    addTlv(tlv);
}

void TlvContainer::addTlv(TlvType_t type, uint32_t val)
{
    IntTlv* tlv = new IntTlv(type, val);
    addTlv(tlv);
}

const Tlv* TlvContainer::getTlv(TlvType_t type) const
{
    return getTlvInstance(type, 0);
}

const Tlv* TlvContainer::getTlvInstance(TlvType_t type, int instance) const
{
    int count = 0;
    for (std::list<Tlv*>::const_iterator it = tlvs.begin(); it != tlvs.end(); *it++)
    {
        if((*it)->getType() == type)
        {
            if (count == instance)
                return (*it);

            count++;
        }
    }
    return NULL;
}

int TlvContainer::getNumTlv(TlvType_t type) const
{
    int count = 0;
    for (std::list<Tlv*>::const_iterator it = tlvs.begin(); it != tlvs.end(); *it++)
    {
        if((*it)->getType() == type)
        {
            count++;
        }
    }
    return count;
}

void TlvContainer::encode(MessageEncoder* msg) const
{
    tlvgroup_t* container = NULL;
    if (type_ != 0)
        container = msg->createNewGroup(type_);

    for (std::list<Tlv*>::const_iterator it = tlvs.begin(); it != tlvs.end(); *it++)
    {
        (*it)->encode(msg);
    }

    if (type_ != 0)
        msg->finalizeGroup(container);
}

std::string TlvContainer::print() const
{
    std::stringstream ss;

    ss << std::string((size_t)depth_*2, ' ');
    ss << "TLV: " << tlvTypeToString(type_) << '\n';
    if (tlvs.size() > 0)
    {
        for (std::list<Tlv*>::const_iterator it = tlvs.begin(); it != tlvs.end(); *it++)
        {
            ss << (*it)->print();
        }
    }
    return ss.str();
}


TlvRoot::TlvRoot() : TlvContainer((TlvType_t)0)
{
}

void TlvRoot::encode(MessageEncoder* msg) const
{
    for (std::list<Tlv*>::const_iterator it = tlvs.begin(); it != tlvs.end(); *it++)
    {
        (*it)->encode(msg);
    }
}

std::string TlvRoot::print() const
{
    std::stringstream ss;

    if (tlvs.size() > 0)
    {
        ss << "TLVs:\n";
        for (std::list<Tlv*>::const_iterator it = tlvs.begin(); it != tlvs.end(); *it++)
        {
            ss << (*it)->print();
        }
    }
    else
    {
        ss << "No TLVs";
    }
    return ss.str();
}

std::ostream& operator <<(std::ostream& os, const Tlv& rhs)
{
    os << rhs.print();
    return os;
}




