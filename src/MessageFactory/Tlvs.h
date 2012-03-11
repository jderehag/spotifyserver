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

#ifndef TLVS_H_
#define TLVS_H_

#include "TlvDefinitions.h"
#include "MessageEncoder.h"
#include <string>
#include <list>
#include <vector>
#include <stdint.h>

/*
 * Tlv Base (and no-content Tlv)
 */

class Tlv
{
private:

protected:
    TlvType_t type_;
    Tlv(TlvType_t type);

    int depth_;

public:
    virtual Tlv* clone() const;

    TlvType_t getType() const;

    virtual void setDepth(int depth);

    virtual void encode(MessageEncoder* msg) const;

    virtual std::string print() const;

    virtual ~Tlv();
};

/*
 * StringTlv
 */

class StringTlv : public Tlv
{
private:
    std::string data_;

public:
    StringTlv(TlvType_t type, const uint8_t* data, uint32_t len);
    StringTlv(TlvType_t type, const std::string& data);
    Tlv* clone() const;

    const std::string& getString() const;

    void encode(MessageEncoder* msg) const;

    std::string print() const;
};

/*
 * IntTlv
 */

class IntTlv : public Tlv
{
private:
    uint32_t data_;

public:
    IntTlv(TlvType_t type, uint32_t data);
    Tlv* clone() const;

    uint32_t getVal() const;

    void encode(MessageEncoder* msg) const;

    std::string print() const;
};

/*
 * BinaryTlv
 */

class BinaryTlv : public Tlv
{
private:
    //std::vector<uint8_t> data_;
    uint8_t* data_;
    uint32_t len_;

public:
    BinaryTlv(TlvType_t type, const uint8_t* data, uint32_t len);
    BinaryTlv(const BinaryTlv& from);
    Tlv* clone() const;

    //std::vector<uint8_t> getVal() const { return data_; }
    uint8_t* getData() const;
    uint32_t getLen() const;

    void encode(MessageEncoder* msg) const;

    std::string print() const;

    ~BinaryTlv();
};

/*
 * TlvContainer
 */

class TlvContainer : public Tlv
{
public:
    typedef std::list<Tlv*>::iterator iterator;
    typedef std::list<Tlv*>::const_iterator const_iterator;

protected:
    std::list<Tlv*> tlvs;

public:
    TlvContainer(TlvType_t type);
    TlvContainer(const TlvContainer& from);
    Tlv* clone() const;

    iterator begin() { return tlvs.begin(); }
    const_iterator begin() const { return tlvs.begin(); }
    iterator end() { return tlvs.end(); }
    const_iterator end() const { return tlvs.end(); }

    void setDepth(int depth);

    void addTlv(Tlv* tlv);
    void addTlv(TlvType_t type, const std::string& str);
    void addTlv(TlvType_t type, const uint8_t* str, uint32_t len);
    void addTlv(TlvType_t type, uint32_t val);

    std::list<Tlv*> getTlvs() const { return tlvs; }
    int getNumTlv(TlvType_t type) const;
    const Tlv* getTlv(TlvType_t type) const;
    const Tlv* getTlvInstance(TlvType_t type, int instance) const;

    virtual void encode(MessageEncoder* msg) const;

    virtual std::string print() const;
    virtual ~TlvContainer();
};

class TlvRoot : public TlvContainer
{
public:
    TlvRoot();
    void encode(MessageEncoder* msg) const;
    std::string print() const;
};

class PlaylistTlv : public TlvContainer
{
public:
    PlaylistTlv();

    void addLink(const uint8_t* url, uint32_t len);
    void addLink(const std::string& url);
};


std::ostream& operator <<(std::ostream& os, const Tlv& rhs);



#endif /* TLVS_H_ */
