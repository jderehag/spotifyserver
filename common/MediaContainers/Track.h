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

#include "MediaBaseInfo.h"
#include "MessageFactory/Tlvs.h"
#include <string>
#include <vector>

namespace LibSpotify
{

class Track : public MediaBaseInfo
{
private:
    std::vector<MediaBaseInfo> artistList_;
    MediaBaseInfo album_;
	bool isStarred_;
	bool isLocal_;
	bool isAutoLinked_;
    bool isAvailable_;
	unsigned int durationMillisecs_;
    int index_; /* index in list when referenced by playlist or album */


public:
	Track(const std::string& name, const std::string& link);
    Track( const TlvContainer* tlv );
	virtual ~Track();

	const std::vector<MediaBaseInfo>& getArtists() const;
	void addArtist(MediaBaseInfo& artist);
    const std::string getArtistsPrettyString() const;
	const std::string& getAlbum() const;
	void setAlbum(const std::string& name, const std::string& link);
    const std::string& getAlbumLink() const;
	unsigned int getDurationMillisecs() const;
	void setDurationMillisecs(unsigned int duration);
	bool isStarred() const;
	void setIsStarred(bool isStarred);
	bool isLocal() const;
	void setIsLocal(bool isStarred);
	bool isAutoLinked() const;
	void setIsAutoLinked(bool isStarred);
    bool isAvailable() const;
    void setIsAvailable(bool available);
	int getIndex() const;
	void setIndex(int index);

	void write(MessageEncoder* msg) const;
	TlvContainer* toTlv() const;
/*
	bool operator!=(const Track& rhs) const;
	bool operator==(const Track& rhs) const;*/
};

}

#endif /* TRACK_H_ */
