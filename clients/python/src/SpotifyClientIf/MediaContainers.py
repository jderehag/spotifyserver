#!/usr/bin/python
'''
Copyright (c) 2012, Jesper Derehag
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the <organization> nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL JESPER DEREHAG BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
'''
class Folder(object):
    def __init__(self, name, folderId):
        self.__Name = name
        self.__Id = folderId
        self.__Playlists = []
        self.__Folders = []
        
    def getName(self):
        return self.__Name
    def setName(self, name):
        self.__Name = name
        
    def getId(self):
        return self.__Id
    def setId(self, folderId):
        self.__Id = folderId
        
    def getPlaylists(self):
        return self.__Playlists
    def addPlaylist(self, playlist):
        self.__Playlists.append(playlist)
    
    
    def getSubFolders(self):
        return self.__Folders
    def addSubFolder(self, folder):
        self.__Folders.append(folder)
    

class Playlist(object):
    
    def __init__(self, name, uri):
        self.__name = name
        self.__uri = uri
    
    def __str__(self):
        return self.__name
    
    def getName(self):
        return self.__name
    
    def getUri(self):
        return self.__uri 

class Track(object):
    def __init__(self, name, link, artistlist, album, isStarred, isLocal, isAutoLinked, durationMillisecs, index):
        self.__name = name
        self.__uri = link
        self.__artistList = artistlist
        self.__album = album
        self.__isStarred = isStarred
        self.__isLocal = isLocal
        self.__isAutoLinked = isAutoLinked
        self.__durationMillisecs = durationMillisecs
        self.__index = index
    
    def __str__(self):
        return self.__name
    
    def getName(self):
        return self.__name
    
    def getUri(self):
        return self.__uri
    
    def getArtistList(self):
        return self.__artistList
    
    def getAlbum(self):
        return self.__album
    
    def getDurationMillisecs(self):
        return self.__durationMillisecs
    
    def getIndex(self):
        return self.__index
    
class Artist(object):
    def __init__(self, name, uri):
        self.__name = name
        self.__uri = uri
    def getName(self):
        return self.__name
    def getUri(self):
        return self.__uri
    
    
class Album(object):
    def __init__(self, name, uri):
        self.__name = name
        self.__uri = uri
    def getName(self):
        return self.__name
    def getUri(self):
        return self.__uri