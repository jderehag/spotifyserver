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

#include "Folder.h"
#include "MessageFactory/TlvDefinitions.h"
#include "MessageFactory/Message.h"
#include <string.h>
#include <iostream>
#include "applog.h"

namespace LibSpotify
{
Playlist nullPlayList("NULL NAME","NULL LINK",true);
static void insertTabs(std::ostream& os, unsigned short numberOfTabs);

Folder::Folder(const std::string& name, unsigned long long id, Folder* parentFolder) : name_(name), id_(id), parentFolder_(parentFolder) { }
Folder::Folder(const char* name, unsigned long long id, Folder* parentFolder) : name_(name), id_(id), parentFolder_(parentFolder) { }
Folder::~Folder(){ }

void Folder::addFolder(Folder& folder)
{
	folders_.push_back(folder);
}
void Folder::addPlaylist(Playlist& playlist)
{
	playlists_.push_back(playlist);
}

Folder* Folder::getParentFolder()
{
	return parentFolder_;
}

const std::string& Folder::getName() const
{
	return name_;
}

const Playlist & Folder::findPlaylist(const std::string& playlist)
{
    for (std::deque<LibSpotify::Playlist>::iterator p = playlists_.begin(); p != playlists_.end(); *p++)
    {
        if((*p).getLink().compare(playlist) == 0)
        {
            return *p; /*found it*/
        }
    }

    for (std::vector<Folder>::iterator f = folders_.begin(); f != folders_.end(); *f++)
    {
        const Playlist& pl = f->findPlaylist(playlist);

        if (!pl.nullObject())
            return pl ; /*found it*/
    }
    return nullPlayList; /*nope, not in here*/
}

std::deque<Playlist>& Folder::getPlaylists()
{
	return playlists_;
}

std::vector<Folder>& Folder::getFolders()
{
	return folders_;
}

void Folder::getAllTracks(std::deque<Track>& trackList) const
{
    for(std::vector<Folder>::const_iterator it = folders_.begin(); it != folders_.end(); it++)
    {
        (*it).getAllTracks(trackList);
    }

    for(std::deque<Playlist>::const_iterator it = playlists_.begin(); it != playlists_.end(); it++)
    {
        trackList.insert(trackList.end(), (*it).getTracks().begin(), (*it).getTracks().end());
    }
}

Tlv* Folder::toTlv() const
{
    TlvContainer* folder = new TlvContainer(TLV_FOLDER);

    folder->addTlv(TLV_NAME, name_);

    for (std::vector<Folder>::const_iterator f = folders_.begin(); f != folders_.end(); *f++)
    {
        folder->addTlv( (*f).toTlv() );
    }

    for (std::deque<Playlist>::const_iterator p = playlists_.begin(); p != playlists_.end(); *p++)
    {
        folder->addTlv( (*p).toTlv() );
    }

    return folder;
}


bool Folder::operator==(const Folder& rhs) const
{
	return (name_ == rhs.name_ ) &&
			(id_ == rhs.id_) &&
			(playlists_ == rhs.playlists_) &&
			(folders_ == rhs.folders_);
}

bool Folder::operator!=(const Folder& rhs) const
{
	return !(*this == rhs);
}

std::ostream &operator <<(std::ostream& os, const Folder& rhs)
{
	/* calculate tabspaces by looking at the number of parent folders*/
	unsigned short tabSpaces = 0;

	/* causes segfault for some reasen (parentFolder->parentFolder_ points to
	 * inaccessible data) */
	/*Folder* parentFolder = rhs.parentFolder_;
	while(parentFolder != 0)
	{
		tabSpaces++;
		parentFolder = parentFolder->parentFolder_;
	}*/

	/* Then print all the subfolders and playlists */
	os << rhs.name_ << std::endl;
	for(std::vector<Folder>::const_iterator it = rhs.folders_.begin(); it != rhs.folders_.end(); it++)
	{
		if(it == rhs.folders_.begin())
		{
			insertTabs(os, tabSpaces);
			os << "Folder:" << std::endl;
		}
		insertTabs(os, tabSpaces+1);
		os << *it;
	}

	for(std::deque<Playlist>::const_iterator it = rhs.playlists_.begin(); it != rhs.playlists_.end(); it++)
	{
		insertTabs(os, tabSpaces+1);
		os << *it << std::endl;
	}
	return os;
}

static void insertTabs(std::ostream& os, unsigned short numberOfTabs)
{
	for(unsigned short i = 0; i < numberOfTabs;i++)os << "\t";
}
}
