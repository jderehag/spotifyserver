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

#ifndef FOLDER_H_
#define FOLDER_H_

#include "Playlist.h"
#include "MessageFactory/Tlvs.h"
#include <string>
#include <deque>
#include <vector>

namespace LibSpotify
{

typedef std::vector<class Folder> FolderContainer;  //can't be deque since it's incomplete type and the template mumbojumbo can't handle that, 
                                                    //it's undefined behaviour according to C++ std and doesn't compile in VS. For some reason vector is ok.
typedef std::deque<Playlist> PlaylistContainer;

class Folder : public MediaBaseInfo
{
private:
	unsigned long long id_;
	PlaylistContainer playlists_;
	FolderContainer folders_;
	Folder* parentFolder_;

public:
	Folder(const std::string& name, unsigned long long id, Folder* parentFolder);
    Folder( const TlvContainer* tlv );
	virtual ~Folder();

	void addFolder(Folder& folder);
	void addPlaylist(Playlist& playlist);
	Folder* getParentFolder();

	bool findPlaylist(const std::string& playlist , Playlist& pl);

    PlaylistContainer& getPlaylists();
    const PlaylistContainer& getPlaylists() const;
    FolderContainer& getFolders();
    const FolderContainer& getFolders() const;
	void getAllTracks(std::deque<Track>& allTracks) const;

    Tlv* toTlv() const;

	bool operator!=(const Folder& rhs) const;
	bool operator==(const Folder& rhs) const;
	friend std::ostream& operator <<(std::ostream& os, const Folder& rhs);

};



}

#endif /* FOLDER_H_ */
