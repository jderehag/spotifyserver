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
static void insertTabs(std::ostream& os, unsigned short numberOfTabs);

Folder::Folder(const std::string& name, unsigned long long id, Folder* parentFolder) : MediaBaseInfo(name, ""), FolderItem(true), id_(id), parentFolder_(parentFolder) { }
Folder::Folder( const TlvContainer* tlv ) : MediaBaseInfo(tlv), FolderItem(true)
{
    parentFolder_ = NULL;

    for ( TlvContainer::const_iterator it = tlv->begin(); it != tlv->end(); it++ )
    {
        switch( (*it)->getType() )
        {
            case TLV_FOLDER:
            {
                Folder* subfolder = new Folder( (TlvContainer*) (*it) );
                addFolder( subfolder );
            }
            break;
            case TLV_PLAYLIST:
            {
                Playlist* playlist = new Playlist( (TlvContainer*) (*it) );
                addPlaylist( playlist );
            }
            break;
            default:
                break;
        }
    }
}

Folder::~Folder()
{
    for (FolderItemContainer::iterator it = children_.begin(); it != children_.end(); it++)
    {
        delete *it;
    }
    children_.clear();
}

void Folder::addFolder(Folder* folder)
{
    folder->parentFolder_ = this;
	children_.push_back(folder);
}
void Folder::addPlaylist(Playlist* playlist)
{
	children_.push_back(playlist);
}

Folder* Folder::getParentFolder()
{
	return parentFolder_;
}

bool Folder::findPlaylist(const std::string& playlist, Playlist& pl)
{
    for (FolderItemContainer::iterator it = children_.begin(); it != children_.end(); it++)
    {
        if ( (*it)->isFolder )
        {
            Folder* f = dynamic_cast<Folder*>(*it);
            if ( f->findPlaylist( playlist,pl ) == true )
                return true;
        }
        else
        {
            Playlist* p = dynamic_cast<Playlist*>(*it);
            if ( p->getLink().compare(playlist) == 0 )
            {
                pl = *p;
                return true;
            }
        }
    }

    return false; /*nope, not in here*/
}


const FolderItemContainer& Folder::getItems() const
{
    return children_;
}
FolderItemContainer& Folder::getItems()
{
    return children_;
}

void Folder::getAllTracks(std::deque<Track>& trackList) const
{
    for (FolderItemContainer::const_iterator it = children_.begin(); it != children_.end(); it++)
    {
        if ( (*it)->isFolder )
        {
            const Folder* f = dynamic_cast<const Folder*>(*it);
            f->getAllTracks(trackList);
        }
        else
        {
            const Playlist* p = dynamic_cast<const Playlist*>(*it);
            trackList.insert(trackList.end(), p->getTracks().begin(), p->getTracks().end());
        }
    }
}

Tlv* Folder::toTlv() const
{
    TlvContainer* folder = new TlvContainer(TLV_FOLDER);

    folder->addTlv(TLV_NAME, name_);

    for (FolderItemContainer::const_iterator it = children_.begin(); it != children_.end(); it++)
    {
        folder->addTlv( (*it)->toTlv() );
    }

    return folder;
}


bool Folder::operator==(const Folder& rhs) const
{
    bool contentsEqual = true;
    if ( children_.size() != rhs.children_.size() )
        contentsEqual = false;
    else
    {
        FolderItemContainer::const_iterator it1 = children_.begin();
        FolderItemContainer::const_iterator it2 = rhs.children_.begin();
        for (; it1 != children_.end() && contentsEqual; it1++, it2++)
        {
            if ( (*it1)->isFolder != (*it2)->isFolder )
                contentsEqual = false;
            else
            {
                if ( (*it1)->isFolder )
                    contentsEqual = (*dynamic_cast<const Folder*>(*it1) == *dynamic_cast<const Folder*>(*it2));
                else
                    contentsEqual = (*dynamic_cast<const Playlist*>(*it1) == *dynamic_cast<const Playlist*>(*it2));
            }
        }
    }
	return (name_ == rhs.name_ ) &&
			(id_ == rhs.id_) &&
            (contentsEqual);
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
    for (FolderItemContainer::const_iterator it = rhs.children_.begin(); it != rhs.children_.end(); it++)
    {
        if ( (*it)->isFolder )
        {
            const Folder* f = dynamic_cast<const Folder*>(*it);
            if ( it == rhs.children_.begin() )
            {
                insertTabs(os, tabSpaces);
                os << "Folder:" << std::endl;
            }
            insertTabs(os, tabSpaces+1);
            os << f;
        }
        else
        {
            const Playlist* p = dynamic_cast<const Playlist*>(*it);
            insertTabs(os, tabSpaces+1);
            os << *p << std::endl;
        }
    }

    return os;
}

static void insertTabs(std::ostream& os, unsigned short numberOfTabs)
{
	for(unsigned short i = 0; i < numberOfTabs;i++)os << "\t";
}
}
