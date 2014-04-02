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

#include "Album.h"
#include "Artist.h"

namespace LibSpotify
{


Album::Album(const char* name, const char* link) : Playlist(name, link), 
                                                   year_(0), 
                                                   review_(""),
                                                   isAvailable_(false), 
                                                   artist_(Artist("", ""))
{ }
Album::Album(const TlvContainer* tlv) : Playlist(tlv), artist_(Artist("", ""))
{
    const IntTlv* tlvYear = (const IntTlv*) tlv->getTlv(TLV_ALBUM_RELEASE_YEAR);
    const StringTlv* tlvReview = (const StringTlv*) tlv->getTlv(TLV_ALBUM_REVIEW);
    const IntTlv* tlvIsAvailable = (const IntTlv*) tlv->getTlv(TLV_ALBUM_IS_AVAILABLE);
    const TlvContainer* tlvArtist = (const TlvContainer*) tlv->getTlv(TLV_ARTIST);
    year_ = (tlvYear ? tlvYear->getVal() : 0);
    review_ = (tlvReview ? tlvReview->getString() : "");
    isAvailable_ = (tlvIsAvailable ? tlvIsAvailable->getVal() != 0 : false);
    if ( tlvArtist )
        artist_ = Artist( tlvArtist );
}
Album::~Album() { }

void Album::setYear(int year) { year_ = year; }
int Album::getYear() const { return year_; }

void Album::setReview( std::string review ) { review_ = review; }
const std::string& Album::getReview() const { return review_; }

void Album::setIsAvailable( bool isAvailable ) { isAvailable_ = isAvailable; }
int Album::getIsAvailable() const { return isAvailable_; }

void Album::setArtist( MediaBaseInfo& artist ) { artist_ = artist; }
const MediaBaseInfo& Album::getArtist() const { return artist_; }

TlvContainer* Album::toTlv() const
{
    TlvContainer* album = createTlv(TLV_ALBUM);

    album->addTlv( TLV_ALBUM_RELEASE_YEAR, year_ );
    album->addTlv( TLV_ALBUM_REVIEW, review_ );
    album->addTlv( TLV_ALBUM_IS_AVAILABLE, isAvailable_ ? 1 : 0 );

    album->addTlv( artist_.createTlv(TLV_ARTIST) );

    return album;
}

} /* namespace LibSpotify */
