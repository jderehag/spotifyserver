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

#include "Playlist.h"
#include "MessageFactory/TlvDefinitions.h"
#include <string.h>
#include <stdio.h>
#include <iostream>

namespace LibSpotify
{

Playlist::Playlist(const std::string& name, const std::string& link) : name_(name), link_(link) { }
Playlist::Playlist(const char* name, const char* link) : name_(name), link_(link) { }
Playlist::~Playlist(){ }

bool Playlist::isCollaborative()
{
	return isCollaborative_;
}

void Playlist::setIsCollaborative(bool isCollaborative)
{
	isCollaborative_ = isCollaborative;
}

void Playlist::addTrack(Track& track)
{
	tracks_.push_back(track);
}

const std::string& Playlist::getName() const
{
	return name_;
}

const std::string& Playlist::getLink() const
{
	return link_;
}

const std::deque<Track>& Playlist::getTracks() const
{
	return tracks_;
}

Tlv* Playlist::toTlv() const
{
    PlaylistTlv* playlist = new PlaylistTlv;

    playlist->addTlv(TLV_NAME, name_);
    playlist->addLink(link_);

    return playlist;
}

bool Playlist::operator==(const Playlist& rhs) const
{
	return (name_ == rhs.name_) &&
			(link_ == rhs.link_) &&
			(tracks_ == rhs.tracks_) &&
			(isCollaborative_ == rhs.isCollaborative_) &&
			(isStarred_ == rhs.isStarred_);
}
bool Playlist::operator!=(const Playlist& rhs) const
{
	return !(*this == rhs);
}


std::ostream& operator <<(std::ostream &os, const Playlist& rhs)
{
	os << rhs.name_ << " => " << rhs.link_;
	return os;
}


}
