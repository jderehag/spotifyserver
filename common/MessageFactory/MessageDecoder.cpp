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

#include "MessageDecoder.h"
#include "TlvDefinitions.h"
#include "Platform/Socket/Socket.h"
#include "applog.h"

#ifndef ntohl
#define ntohl Ntohl
#endif
#ifndef htonl
#define htonl Htonl
#endif

MessageDecoder::MessageDecoder() : hasError_(false), hasUnknownTlv_(false)
{
}

MessageDecoder::~MessageDecoder()
{
}

TlvType_t MessageDecoder::getCurrentTlv()
{
    tlvheader_t* tlv = (tlvheader_t*)&message[rpos_];
    return (TlvType_t)ntohl(tlv->type);
}

uint32_t MessageDecoder::getCurrentTlvLen()
{
    tlvheader_t* tlv = (tlvheader_t*)&message[rpos_];
    return ntohl(tlv->len);
}


void MessageDecoder::nextTlv()
{
    rpos_ += getCurrentTlvLen() + sizeof(tlvheader_t);
}

void MessageDecoder::enterTlvGroup()
{
    rpos_ += sizeof(tlvheader_t);
}

bool MessageDecoder::validateLength(uint32_t max)
{
    if(((uint64_t)rpos_ + getCurrentTlvLen() + sizeof(tlvheader_t)) > max) /* guard vs bad length */
    {
        log(LOG_NOTICE) << "Bad tlv length " << getCurrentTlvLen() << " bytes, tlv 0x" << std::hex << getCurrentTlv() << std::dec << " at byte " << rpos_;
        hasError_ = true;
        return true;
    }
    return false;
}

const uint8_t* MessageDecoder::getTlvData()
{
    return &message[rpos_ + sizeof(tlvheader_t)];
}

uint32_t MessageDecoder::getTlvIntData()
{
    uint32_t* p = (uint32_t*)getTlvData();
    return ntohl(*p);
}

TlvContainer* MessageDecoder::decodeGroupTlv()
{
    uint32_t end;
    uint32_t len = getCurrentTlvLen();
    TlvContainer* groupTlv = new TlvContainer(getCurrentTlv());

    enterTlvGroup();
    end = len + rpos_;

    while (rpos_ < end && !hasError_)
    {
        if(validateLength(end)) break;

        switch(getCurrentTlv())
        {
            case TLV_IP_ADDRESS:
            case TLV_LINK:
            {
                groupTlv->addTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen());
                nextTlv();
            }
            break;

            case TLV_PORT:
            case TLV_AUDIO_EP_PROTOCOL:
            case TLV_STATE:
            case TLV_VOLUME:
            {
                groupTlv->addTlv(getCurrentTlv(), getTlvIntData());
                nextTlv();
            }
            break;

            default:
                log(LOG_NOTICE) << "Unknown TLV 0x" << std::hex << getCurrentTlv() << std::dec << " at byte " << rpos_;
                hasUnknownTlv_ = true;
                nextTlv();
                break;
        }
    }

    /*validation of length field in group tlv*/
    if (!hasError_ && rpos_ != end )
    {
        log(LOG_NOTICE) << "TLV 0x" << std::hex << getCurrentTlv() << std::dec << " has bad length " << len << ", ends at " << rpos_ << ", said it would end at " << end;
        hasError_ = true;
    }
    return groupTlv;
}

TlvContainer* MessageDecoder::decodeImage()
{
    uint32_t end;
    uint32_t len = getCurrentTlvLen();
    TlvContainer* groupTlv = new TlvContainer(TLV_IMAGE);

    enterTlvGroup();
    end = len + rpos_;

    while (rpos_ < end && !hasError_)
    {
        if(validateLength(end)) break;

        switch(getCurrentTlv())
        {
            case TLV_IMAGE_DATA:
            {
                groupTlv->addTlv(new BinaryTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen()));
                nextTlv();
            }
            break;

            case TLV_IMAGE_FORMAT:
            {
                groupTlv->addTlv(getCurrentTlv(), getTlvIntData());
                nextTlv();
            }
            break;

            default:
                log(LOG_NOTICE) << "Unknown TLV 0x" << std::hex << getCurrentTlv() << std::dec << " at byte " << rpos_;
                hasUnknownTlv_ = true;
                nextTlv();
                break;
        }
    }

    /*validation of length field in group tlv*/
    if (!hasError_ && rpos_ != end )
    {
        log(LOG_NOTICE) << "TLV 0x" << std::hex << getCurrentTlv() << std::dec << " has bad length " << len << ", ends at " << rpos_ << ", said it would end at " << end;
        hasError_ = true;
    }
    return groupTlv;
}

TlvContainer* MessageDecoder::decodeArtist()
{
    uint32_t end;
    uint32_t len = getCurrentTlvLen();
    TlvContainer* groupTlv = new TlvContainer(TLV_ARTIST);

    enterTlvGroup();
    end = len + rpos_;

    while (rpos_ < end && !hasError_)
    {
        if(validateLength(end)) break;

        switch(getCurrentTlv())
        {
            case TLV_ALBUM:
            {
                groupTlv->addTlv(decodeAlbum());
            }
            break;

            case TLV_LINK:
            {
                if(groupTlv->getNumTlv(getCurrentTlv()) == 0)
                {
                    groupTlv->addTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen());
                }
                else
                {
                    log(LOG_NOTICE) << "TLV 0x" << std::hex << getCurrentTlv() << std::dec << " occurs more than once, at byte " << rpos_;
                    hasError_ = true;
                }
                nextTlv();
            }
            break;

            case TLV_NAME:
            {
                groupTlv->addTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen());
                nextTlv();
            }
            break;

            default:
                log(LOG_NOTICE) << "Unknown TLV 0x" << std::hex << getCurrentTlv() << std::dec << " at byte " << rpos_;
                hasUnknownTlv_ = true;
                nextTlv();
                break;
        }
    }

    /*validation of length field in group tlv*/
    if (!hasError_ && rpos_ != end )
    {
        log(LOG_NOTICE) << "TLV 0x" << std::hex << getCurrentTlv() << std::dec << " has bad length " << len << ", ends at " << rpos_ << ", said it would end at " << end;
        hasError_ = true;
    }
    return groupTlv;
}

TlvContainer* MessageDecoder::decodeAlbum()
{
    uint32_t end;
    uint32_t len = getCurrentTlvLen();
    TlvContainer* groupTlv = new TlvContainer(TLV_ALBUM);

    enterTlvGroup();
    end = len + rpos_;

    while (rpos_ < end && !hasError_)
    {
        if(validateLength(end)) break;

        switch(getCurrentTlv())
        {
            case TLV_TRACK:
            {
                groupTlv->addTlv(decodeTrack());
            }
            break;

            case TLV_ARTIST:
            {
                groupTlv->addTlv(decodeArtist());
            }
            break;

            case TLV_LINK:
            {
                if(groupTlv->getNumTlv(getCurrentTlv()) == 0)
                {
                    groupTlv->addTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen());
                }
                else
                {
                    log(LOG_NOTICE) << "TLV 0x" << std::hex << getCurrentTlv() << std::dec << " occurs more than once, at byte " << rpos_;
                    hasError_ = true;
                }
                nextTlv();
            }
            break;

            case TLV_ALBUM_REVIEW:
            case TLV_NAME:
            {
                groupTlv->addTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen());
                nextTlv();
            }
            break;

            /* Integer TLVs */
            case TLV_ALBUM_RELEASE_YEAR:
            case TLV_IS_AVAILABLE:
            {
                groupTlv->addTlv(getCurrentTlv(), getTlvIntData());
                nextTlv();
            }
            break;

            default:
                log(LOG_NOTICE) << "Unknown TLV 0x" << std::hex << getCurrentTlv() << std::dec << " at byte " << rpos_;
                hasUnknownTlv_ = true;
                nextTlv();
                break;
        }
    }

    /*validation of length field in group tlv*/
    if (!hasError_ && rpos_ != end )
    {
        log(LOG_NOTICE) << "TLV 0x" << std::hex << getCurrentTlv() << std::dec << " has bad length " << len << ", ends at " << rpos_ << ", said it would end at " << end;
        hasError_ = true;
    }
    return groupTlv;
}

TlvContainer* MessageDecoder::decodeFolder()
{
    uint32_t end;
    uint32_t len = getCurrentTlvLen();
    TlvContainer* groupTlv = new TlvContainer(TLV_FOLDER);

    enterTlvGroup();
    end = len + rpos_;

    while (rpos_ < end && !hasError_)
    {
        if(validateLength(end)) break;

        switch(getCurrentTlv())
        {
            case TLV_PLAYLIST:
            {
                groupTlv->addTlv(decodePlaylist());
            }
            break;

            case TLV_FOLDER:
            {
                groupTlv->addTlv(decodeFolder());
            }
            break;

            case TLV_LINK:
            {
                if(groupTlv->getNumTlv(getCurrentTlv()) == 0)
                {
                    groupTlv->addTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen());
                }
                else
                {
                    log(LOG_NOTICE) << "TLV 0x" << std::hex << getCurrentTlv() << std::dec << " occurs more than once, at byte " << rpos_;
                    hasError_ = true;
                }
                nextTlv();
            }
            break;

            case TLV_NAME:
            {
                groupTlv->addTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen());
                nextTlv();
            }
            break;

            default:
                log(LOG_NOTICE) << "Unknown TLV 0x" << std::hex << getCurrentTlv() << std::dec << " at byte " << rpos_;
                hasUnknownTlv_ = true;
                nextTlv();
                break;
        }
    }

    /*validation of length field in group tlv*/
    if (!hasError_ && rpos_ != end )
    {
        log(LOG_NOTICE) << "TLV 0x" << std::hex << getCurrentTlv() << std::dec << " has bad length " << len << ", ends at " << rpos_ << ", said it would end at " << end;
        hasError_ = true;
    }
    return groupTlv;
}

TlvContainer* MessageDecoder::decodePlaylist()
{
    uint32_t end;
    uint32_t len = getCurrentTlvLen();
    TlvContainer* groupTlv = new TlvContainer( TLV_PLAYLIST );

    enterTlvGroup();
    end = len + rpos_;

    while (rpos_ < end && !hasError_)
    {
        if(validateLength(end)) break;

        switch(getCurrentTlv())
        {
            case TLV_LINK:
            {
                if(groupTlv->getNumTlv(getCurrentTlv()) == 0)
                {
                    groupTlv->addTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen());
                }
                else
                {
                    log(LOG_NOTICE) << "TLV 0x" << std::hex << getCurrentTlv() << std::dec << " occurs more than once, at byte " << rpos_;
                    hasError_ = true;
                }
                nextTlv();
            }
            break;

            case TLV_NAME:
            {
                groupTlv->addTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen());
                nextTlv();
            }
            break;

            default:
                log(LOG_NOTICE) << "Unknown TLV 0x" << std::hex << getCurrentTlv() << std::dec << " at byte " << rpos_;
                hasUnknownTlv_ = true;
                nextTlv();
                break;
        }
    }

    /*validation of length field in group tlv*/
    if (!hasError_ && rpos_ != end )
    {
        log(LOG_NOTICE) << "TLV 0x" << std::hex << getCurrentTlv() << std::dec << " has bad length " << len << ", ends at " << rpos_ << ", said it would end at " << end;
        hasError_ = true;
    }
    return groupTlv;
}

TlvContainer* MessageDecoder::decodeTrack()
{
    uint32_t end;
    uint32_t len = getCurrentTlvLen();
    TlvContainer* groupTlv = new TlvContainer(TLV_TRACK);

    enterTlvGroup();
    end = len + rpos_;

    while (rpos_ < end && !hasError_)
    {
        if(validateLength(end)) break;

        switch(getCurrentTlv())
        {
            case TLV_ALBUM:
            {
                groupTlv->addTlv(decodeAlbum());
            }
            break;

            case TLV_ARTIST:
            {
                groupTlv->addTlv(decodeArtist());
            }
            break;

            /* String TLVs */
            case TLV_LINK:
            {
                if(groupTlv->getNumTlv(getCurrentTlv()) == 0)
                {
                    groupTlv->addTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen());
                }
                else
                {
                    log(LOG_NOTICE) << "TLV 0x" << std::hex << getCurrentTlv() << std::dec << " occurs more than once, at byte " << rpos_;
                    hasError_ = true;
                }
                nextTlv();
            }
            break;

            case TLV_NAME:
            {
                groupTlv->addTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen());
                nextTlv();
            }
            break;

            /* Integer TLVs */
            case TLV_TRACK_DURATION:
            case TLV_TRACK_INDEX:
            case TLV_IS_AVAILABLE:
            {
                groupTlv->addTlv(getCurrentTlv(), getTlvIntData());
                nextTlv();
            }
            break;
            default:
                log(LOG_NOTICE) << "Unknown TLV 0x" << std::hex << getCurrentTlv() << std::dec << " at byte " << rpos_;
                hasUnknownTlv_ = true;
                nextTlv();
                break;
        }
    }

    /*validation of length field in group tlv*/
    if (!hasError_ && rpos_ != end )
    {
        log(LOG_NOTICE) << "TLV 0x" << std::hex << getCurrentTlv() << std::dec << " has bad length " << len << ", ends at " << rpos_ << ", said it would end at " << end;
        hasError_ = true;
    }
    return groupTlv;
}

void MessageDecoder::decodeTlvs(TlvContainer* parent, uint32_t len)
{
    while(rpos_ < len && !hasError_)
    {
        if(validateLength(len)) return;

        switch (getCurrentTlv())
        {
            case TLV_FOLDER:
            {
                parent->addTlv(decodeFolder());
            }
            break;

            case TLV_PLAYLIST:
            {
                parent->addTlv(decodePlaylist());
            }
            break;

            case TLV_TRACK:
            {
                parent->addTlv(decodeTrack());
            }
            break;

            case TLV_IMAGE:
            {
                parent->addTlv(decodeImage());
            }
            break;

            case TLV_ALBUM:
            {
                parent->addTlv(decodeAlbum());
            }
            break;

            case TLV_ARTIST:
            {
                parent->addTlv(decodeArtist());
            }
            break;

            case TLV_CLIENT:
            {
                parent->addTlv(decodeGroupTlv());
            }
            break;

            /* String TLVs */

            case TLV_SEARCH_QUERY:
            case TLV_LINK:
            case TLV_LOGIN_USERNAME:
            case TLV_LOGIN_PASSWORD:
            {
                parent->addTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen());
                nextTlv();
            }
            break;

            /* Integer TLVs */

            case TLV_STATE:
            case TLV_PROGRESS:
            case TLV_PLAY_OPERATION:
            case TLV_PLAY_MODE_SHUFFLE:
            case TLV_PLAY_MODE_REPEAT:
            case TLV_VOLUME:
            case TLV_PROTOCOL_VERSION_MAJOR:
            case TLV_PROTOCOL_VERSION_MINOR:
            case TLV_FAILURE:
            case TLV_TRACK_INDEX:
            case TLV_AUDIO_CHANNELS:
            case TLV_AUDIO_RATE:
            case TLV_AUDIO_NOF_SAMPLES:
            case TLV_AUDIO_PROTOCOL_TYPE:
            case TLV_AUDIO_BUFFERED_SAMPLES:
            case TLV_CLIENT_CLOCK:
            case TLV_SERVER_CLOCK:
            {
                parent->addTlv(getCurrentTlv(), getTlvIntData());
                nextTlv();
            }
            break;

            case TLV_AUDIO_DATA:
            {
                parent->addTlv( new BinaryTlv(getCurrentTlv(), getTlvData(), getCurrentTlvLen()));
                nextTlv();
            }
            break;

            default:
                log(LOG_NOTICE) << "Unknown TLV 0x" << std::hex << getCurrentTlv() << std::dec <<" at byte " << rpos_;
                hasUnknownTlv_ = true;
                nextTlv();
                break;
        }
    }

    /*validation of length field in message*/
    if (!hasError_ && rpos_ != len )
    {
        log(LOG_NOTICE) << "Message has bad length, ends at " << rpos_ << ", said it would end at " << len;
        hasError_ = true;
    }
}


Message* MessageDecoder::decodeMessage(const uint8_t* m)
{
    header_t* header = ((header_t*)m);
    uint32_t end = ntohl(header->len);

    message = m;
    rpos_ = sizeof(header_t);

    switch(ntohl(header->type))
    {
        case GET_TRACKS_REQ:
        {
            GetTracksReq* msg = new GetTracksReq();
            decodeTlvs(&msg->tlvs, end);
            return msg;
        }
        break;

        case GET_PLAYLISTS_REQ:
        case PLAY_TRACK_REQ:
        case PLAY_REQ:
        case GENERIC_SEARCH_REQ:
        default:
        {
            Message* msg = new Message();
            decodeTlvs(&msg->tlvs, end);
            return msg;
        }
        break;
    }
    return NULL;
}

Message* MessageDecoder::decode(const uint8_t* message)
{
    Message* msg = NULL;

    msg = decodeMessage(message);

    if(msg != NULL)
    {
        if ((hasError_ == true) || (msg->validate() == false))
        {
            log(LOG_NOTICE) << "Error decoding";
            delete msg;
            msg = NULL;
        }
        else
        {
            header_t* header = ((header_t*)message);
            msg->setType((MessageType_t)ntohl(header->type));
            msg->setId(ntohl(header->id));
        }
    }

    return msg;
}




