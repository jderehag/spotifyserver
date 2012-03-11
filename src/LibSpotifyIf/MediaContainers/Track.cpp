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

namespace LibSpotify
{

Track::Track(const std::string& name, const std::string& link) : name_(name), link_(link) { }
Track::Track(const char* name, const char* link) : name_(name), link_(link) { }
Track::~Track() { }

const std::string& Track::getLink() const { return link_; }
const std::string& Track::getName() const { return name_; }


const std::vector<Artist>& Track::getArtists() const { return artistList_; }
void Track::addArtist(Artist& artist) {artistList_.push_back(artist); }

const std::string& Track::getAlbum() const { return album_; }
void Track::setAlbum(const std::string& name){ album_ = name; }

const std::string& Track::getAlbumLink() const { return albumLink_; }
void Track::setAlbumLink(const std::string& link){ albumLink_ = link; }

unsigned int Track::getDurationMillisecs() const {return durationMillisecs_;}
void Track::setDurationMillisecs(unsigned int duration){durationMillisecs_ = duration;}

bool Track::isStarred() { return isStarred_; }
void Track::setIsStarred(bool isStarred) { isStarred_ = isStarred; }

bool Track::isLocal() { return isLocal_; }
void Track::setIsLocal(bool isLocal) { isLocal_ = isLocal; }

bool Track::isAutoLinked() { return isAutoLinked_; }
void Track::setIsAutoLinked(bool isAutoLinked) { isAutoLinked_ = isAutoLinked; }

void Track::write(MessageEncoder* msg) const
{
	tlvgroup_t* g = msg->createNewGroup(TLV_TRACK);

	msg->encode(TLV_LINK, link_);
	msg->encode(TLV_NAME, name_);

	for(std::vector<Artist>::const_iterator it = artistList_.begin(); it != artistList_.end(); it++)
	{
	    tlvgroup_t* artist = msg->createNewGroup(TLV_ARTIST);
	    msg->encode(TLV_NAME, it->getName());
	    msg->encode(TLV_LINK, it->getLink());
	    msg->finalizeGroup(artist);
	}

	{
	    tlvgroup_t* album = msg->createNewGroup(TLV_ALBUM);
	    msg->encode(TLV_NAME, album_);
	    msg->encode(TLV_LINK, albumLink_);
	    msg->finalizeGroup(album);
	}
	msg->encode(TLV_TRACK_DURATION, durationMillisecs_);

	msg->finalizeGroup(g);
}

Tlv* Track::toTlv() const
{
    TlvContainer* track = new TlvContainer(TLV_TRACK);

    track->addTlv(TLV_LINK, link_);
    track->addTlv(TLV_NAME, name_);

    for(std::vector<Artist>::const_iterator it = artistList_.begin(); it != artistList_.end(); it++)
    {
        TlvContainer* artist = new TlvContainer(TLV_ARTIST);
        artist->addTlv(TLV_NAME, it->getName());
        artist->addTlv(TLV_LINK, it->getLink());
        track->addTlv(artist);
    }

    {
        TlvContainer* album = new TlvContainer(TLV_ALBUM);
        album->addTlv(TLV_NAME, album_);
        album->addTlv(TLV_LINK, albumLink_);
        track->addTlv(album);
    }
    track->addTlv(TLV_TRACK_DURATION, durationMillisecs_);

    return track;
}

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
}

}
