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

#ifndef MESSAGEDECODER_H_
#define MESSAGEDECODER_H_

#include "Tlvs.h"
#include "Message.h"
#include <string>



class MessageDecoder
{
private:
    const uint8_t* message;
    uint32_t rpos_;
    bool hasError_;
    bool hasUnknownTlv_;

    TlvType_t getCurrentTlv();
    uint32_t getCurrentTlvLen();
    void nextTlv();
    void enterTlvGroup();
    bool validateLength(uint32_t max);
    const uint8_t* getTlvData();
    uint32_t getTlvIntData();

    TlvContainer* decodeFolder();
    TlvContainer* decodePlaylist();
    TlvContainer* decodeTrack();
    TlvContainer* decodeAlbum();
    TlvContainer* decodeArtist();
    TlvContainer* decodeImage();
    TlvContainer* decodeGroupTlv();
    void decodeTlvs(TlvContainer* parent, uint32_t endPos);

    Message* decodeMessage(const uint8_t* message);

public:
    MessageDecoder();
    virtual ~MessageDecoder();

    Message* decode(const uint8_t* message);
};

#endif /* MESSAGEDECODER_H_ */
