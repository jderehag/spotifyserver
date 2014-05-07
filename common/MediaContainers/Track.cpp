/*
 * Copyright (c) 2012, Jesper Derehag
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
 * DISCLAIMED. IN NO EVENT SHALL JESPER DEREHAG BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Track.h"
#include "MessageFactory/Tlvs.h"
#include "Artist.h"

namespace LibSpotify
{

Track::Track(const std::string& name, const std::string& link) : MediaBaseInfo(name, link), album_("", ""), index_(-1) { }
Track::Track( const TlvContainer* tlv ) : MediaBaseInfo( tlv ), album_("", "")
{
    const IntTlv* tlvIndex = (const IntTlv*) tlv->getTlv(TLV_TRACK_INDEX);
    const IntTlv* tlvDuration = (const IntTlv*) tlv->getTlv(TLV_TRACK_DURATION);
    const IntTlv* tlvAvailable = (const IntTlv*) tlv->getTlv( TLV_IS_AVAILABLE );
    const TlvContainer* tlvAlbum = (const TlvContainer*) tlv->getTlv(TLV_ALBUM);

    index_ = ( tlvIndex ? (int)tlvIndex->getVal() : -1);
    durationMillisecs_ = ( tlvDuration ? (int)tlvDuration->getVal() : 0 );
    isAvailable_ = ( tlvAvailable ? tlvAvailable->getVal() != 0 : true );

    if ( tlvAlbum )
    {
        album_ = MediaBaseInfo(tlvAlbum);
    }

    for ( TlvContainer::const_iterator it = tlv->begin();
            it != tlv->end(); it++ )
    {
        if ( (*it)->getType() == TLV_ARTIST )
        {
            MediaBaseInfo artist( (const TlvContainer*)(*it) );
            addArtist( artist );
        }
    }
}
Track::~Track() { }

const std::vector<MediaBaseInfo>& Track::getArtists() const { return artistList_; }
void Track::addArtist(MediaBaseInfo& artist) {artistList_.push_back(artist); }

void Track::setAlbum(const std::string& name, const std::string& link){ album_ = MediaBaseInfo(name, link); }
const std::string& Track::getAlbum() const { return album_.getName(); }
const std::string& Track::getAlbumLink() const { return album_.getLink(); }

unsigned int Track::getDurationMillisecs() const {return durationMillisecs_;}
void Track::setDurationMillisecs(unsigned int duration){durationMillisecs_ = duration;}

bool Track::isStarred() const { return isStarred_; }
void Track::setIsStarred(bool isStarred) { isStarred_ = isStarred; }

bool Track::isLocal() const { return isLocal_; }
void Track::setIsLocal(bool isLocal) { isLocal_ = isLocal; }

bool Track::isAutoLinked() const { return isAutoLinked_; }
void Track::setIsAutoLinked(bool isAutoLinked) { isAutoLinked_ = isAutoLinked; }

bool Track::isAvailable() const { return isAvailable_; }
void Track::setIsAvailable( bool available ) { isAvailable_ = available; }

int Track::getIndex() const { return index_; }
void Track::setIndex(int index) { index_ = index; }

TlvContainer* Track::toTlv() const
{
    TlvContainer* track = createTlv(TLV_TRACK);

    for(std::vector<MediaBaseInfo>::const_iterator it = artistList_.begin(); it != artistList_.end(); it++)
    {
        track->addTlv(it->createTlv(TLV_ARTIST));
    }

    track->addTlv(album_.createTlv(TLV_ALBUM));

    track->addTlv(TLV_TRACK_DURATION, durationMillisecs_);
    if ( index_ >= 0 )
    {
        track->addTlv(TLV_TRACK_INDEX, index_);
    }
    track->addTlv( TLV_IS_AVAILABLE, isAvailable_ );
    return track;
}
/*
bool Track::operator==(const Track& rhs) const
{
	return (name_ == rhs.name_) &&
			(isStarred_ == rhs.isStarred_) &&
			(isLocal_ == rhs.isLocal_) &&
			(isAutoLinked_ == rhs.isAutoLinked_);
}

bool Track::operator!=(const Track& rhs) const
{
	return !(*this == rhs);
}*/

}
