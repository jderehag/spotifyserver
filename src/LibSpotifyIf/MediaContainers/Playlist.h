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

#ifndef PLAYLIST_H_
#define PLAYLIST_H_

#include "Track.h"
#include "MessageFactory/Tlvs.h"

#include <deque>
#include <string>

namespace LibSpotify
{

class Playlist
{
private:
	std::string name_;
	std::string link_;
	std::deque<Track> tracks_;
	bool isCollaborative_;
	bool isStarred_;

public:
	Playlist(const std::string& name, const std::string& link);
	Playlist(const char* name, const char* link);
	virtual ~Playlist();

	bool isCollaborative();
	void setIsCollaborative(bool isCollaborative);
	void addTrack(Track& track);
	const std::string& getName() const;
	const std::string& getLink() const;
	const std::deque<Track>& getTracks() const;

    Tlv* toTlv() const;

	bool operator==(const Playlist& rhs) const;
	bool operator!=(const Playlist& rhs) const;
	friend std::ostream& operator <<(std::ostream& os, const Playlist& rhs);
};



}

#endif /* PLAYLIST_H_ */
