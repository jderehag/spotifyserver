#!/usr/bin/python
'''
Created on 12 feb 2012

@author: jeppe
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
    def __init__(self, name, link, artistlist, album, isStarred, isLocal, isAutoLinked, durationMillisecs):
        self.__name = name
        self.__uri = link
        self.__artistList = artistlist
        self.__album = album
        self.__isStarred = isStarred
        self.__isLocal = isLocal
        self.__isAutoLinked = isAutoLinked
        self.__durationMillisecs = durationMillisecs
    
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