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

#ifndef TRACK_H_
#define TRACK_H_

#include "Artist.h"
#include "MessageFactory/Tlvs.h"
#include <string>
#include <vector>

namespace LibSpotify
{
class Track
{
private:
	std::string name_;
	std::string link_;
    std::vector<Artist> artistList_;
    std::string album_;
    std::string albumLink_;
	bool isStarred_;
	bool isLocal_;
	bool isAutoLinked_;
	unsigned int durationMillisecs_;
    int index_; /* index in list when referenced by playlist or album */


public:
	Track(const std::string& name, const std::string& link);
	Track(const char* name, const char* link);
    Track( const TlvContainer* tlv );
	virtual ~Track();

	const std::string& getLink() const;
	const std::string& getName() const;

	const std::vector<Artist>& getArtists() const;
	void addArtist(Artist& artist);
	const std::string& getAlbum() const;
	void setAlbum(const std::string& name);
    const std::string& getAlbumLink() const;
    void setAlbumLink(const std::string& link);
	unsigned int getDurationMillisecs() const;
	void setDurationMillisecs(unsigned int duration);
	bool isStarred() const;
	void setIsStarred(bool isStarred);
	bool isLocal() const;
	void setIsLocal(bool isStarred);
	bool isAutoLinked() const;
	void setIsAutoLinked(bool isStarred);
	int getIndex() const;
	void setIndex(int index);

	void write(MessageEncoder* msg) const;
	Tlv* toTlv() const;

	bool operator!=(const Track& rhs) const;
	bool operator==(const Track& rhs) const;
};

}

#endif /* TRACK_H_ */
