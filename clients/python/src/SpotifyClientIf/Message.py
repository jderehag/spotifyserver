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
import TlvDefinitions
from MediaContainers import *
import struct

class Message(object):
    def __init__(self, msgType, msgId):
        self._TlvSet = []
        self._MsgType = msgType
        self._MsgId = msgId
        
    def Decode(self, msg):
        msgLength, self._MsgType, self._MsgId = struct.unpack('!III', msg[0:12])
        messagePtr = 12
        while messagePtr < msgLength:
            tlv = Tlv.Decode(msg[messagePtr:msgLength])
            self.addTlv(tlv)
            messagePtr += tlv.getLength()
        
    def addTlv(self, tlv):
        self._TlvSet.append(tlv)
        
    def toByteStream(self):
        # First construct header (calculates length, not so efficient)
        output = struct.pack('!III', self.getMsgLength() , self._MsgType ,self._MsgId)
        for tlv in self._TlvSet[:]:
            output += tlv.toByteStream()
        return output
    
    def getMsgLength(self): #Gets total message length in bytes
        length = 12  # Header is always 12
        for tlv in self._TlvSet[:]:
            length += tlv.getLength()
        return length
        


class Tlv(object):
    def __init__(self):
        self._TlvSet = []
        self._TlvType = None
        self._TlvLength = 0
        self._TlvValue = None
    
    @classmethod
    def Create(cls, TlvType, TlvLength, TlvValue):
        obj = cls()
        obj._TlvType = TlvType
        obj._TlvLength = TlvLength
        obj._TlvValue = TlvValue
        return obj
    
    @classmethod
    def CreateContainer(cls, TlvType):
        obj = cls()
        obj._TlvType = TlvType
        return obj
    
    @classmethod
    def Decode(cls, msg):
        obj = cls()
        # First collect this instance TLV header
        messagePtr = 0
        obj._TlvType, obj._TlvLength = struct.unpack('!II', msg[messagePtr:messagePtr+8])
        messagePtr += 8
        
        # if this is a container type, parse sub TLV:s
        if (obj.isContainerType()):
            while messagePtr <= obj._TlvLength:
                tlv = Tlv.Decode(msg[messagePtr:messagePtr + obj._TlvLength])
                #Avoid using addTlv() since it will incr. length
                obj._TlvSet.append(tlv)
                messagePtr += tlv.getLength()
        else:
            obj._TlvValue = msg[messagePtr:messagePtr + obj._TlvLength]
        return obj
    
    def toByteStream(self):
        output = struct.pack('!II', self._TlvType , self._TlvLength)
        if(self._TlvValue == None): # This is a container type!
            for tlv in self._TlvSet[:]:
                output += tlv.toByteStream()
        else:
            # Agreeably ugly to be type-aware! 
            # But rather than refactoring the entire TLV class lets do it this way for now..
            if(isinstance(self._TlvValue, basestring)):
                output += self._TlvValue
            else:
                output += struct.pack('!I', int(self._TlvValue))
        return output
    
    def getLength(self):
        #Gets total TLV length in bytes
        return self._TlvLength + 8 # TLV header is always 8    
    
    def addTlv(self, tlv):
        self._TlvLength += tlv.getLength()
        self._TlvSet.append(tlv)
    
    def isContainerType(self):
        if (self._TlvType == TlvDefinitions.TlvContainerType.TLV_FOLDER or 
            self._TlvType == TlvDefinitions.TlvContainerType.TLV_PLAYLIST or
            self._TlvType == TlvDefinitions.TlvContainerType.TLV_TRACK or 
            self._TlvType == TlvDefinitions.TlvContainerType.TLV_ALBUM or
            self._TlvType == TlvDefinitions.TlvContainerType.TLV_ARTIST or
            self._TlvType == TlvDefinitions.TlvContainerType.TLV_IMAGE):
            return True
        else:
            return False
        
    def getAllTlvsOfType(self, tlvType):
        output = []
        for tlv in self._TlvSet[:]:
            if(tlv._TlvType == tlvType):
                output.append(tlv)
            elif(tlv.isContainerType()):
                output.extend(tlv.getAllTlvsOfType(tlvType))
        return output
    
    def getSubTlv(self, tlvType):
        for tlv in self._TlvSet[:]:
            if(tlv._TlvType == tlvType):
                return tlv
                


class GetPlaylistReqMsg(Message):
    def __init__(self, msgId):
        Message.__init__(self, TlvDefinitions.TlvMessageType.GET_PLAYLISTS_REQ, msgId)
        
class GetPlaylistRspMsg(Message):
    def __init__(self, msgId):
        Message.__init__(self, TlvDefinitions.TlvMessageType.GET_PLAYLISTS_RSP, msgId)
    
    def getAllPlaylists(self):
        playlistTlvs = []
        playlists = [] #MediaContainerType
        for tlv in self._TlvSet[:]:
            playlistTlvs.extend(tlv.getAllTlvsOfType(TlvDefinitions.TlvContainerType.TLV_PLAYLIST))
        
        for playlistTlv in playlistTlvs[:]:
            name = playlistTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0')
            uri = playlistTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_LINK)._TlvValue.rstrip(' \t\r\n\0')
            playlists.append(Playlist(name, uri))
        return playlists
    
    def getRootFolder(self):
        '''
        TODO: have only started on this one..
        '''
        rootFolder = Folder("tmp", 0)
        for tlv in self._TlvSet[:]:
            if(tlv._TlvType == TlvDefinitions.TlvContainerType.TLV_FOLDER):
                rootFolder.setName(tlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0'))
                

        
class GetTracksReqMsg(Message):
    def __init__(self, msgId, playlistUri):
        Message.__init__(self, TlvDefinitions.TlvMessageType.GET_TRACKS_REQ, msgId)
        playlistUriTlv = Tlv.Create(TlvDefinitions.TlvContainerType.TLV_LINK, len(playlistUri), playlistUri)
        playlistTlv = Tlv.CreateContainer(TlvDefinitions.TlvContainerType.TLV_PLAYLIST)
        playlistTlv.addTlv(playlistUriTlv)
        self.addTlv(playlistTlv)
        
class GetTracksRspMsg(Message):
    def __init__(self, msgId):
        Message.__init__(self, TlvDefinitions.TlvMessageType.GET_TRACKS_RSP, msgId)
    
    def getAllTracks(self):
        trackTlvs = []
        tracks = []
        #Find all Track TLV:s recursively
        for tlv in self._TlvSet[:]:
            if(tlv._TlvType == TlvDefinitions.TlvContainerType.TLV_TRACK):
                trackTlvs.append(tlv)
            trackTlvs.extend(tlv.getAllTlvsOfType(TlvDefinitions.TlvContainerType.TLV_TRACK))
        
        #Create all the track mediacontainers
        for trackTlv in trackTlvs[:]:
            name = trackTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0')
            uri = trackTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_LINK)._TlvValue.rstrip(' \t\r\n\0')
            durationTlv = trackTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_TRACK_DURATION)._TlvValue
            duration = None
            if(durationTlv != None):
                duration, = struct.unpack('!I', durationTlv)
            
            artists = []
            for artistTlv in trackTlv.getAllTlvsOfType(TlvDefinitions.TlvContainerType.TLV_ARTIST):
                artistName = artistTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0')
                artistLink = artistTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_LINK)._TlvValue.rstrip(' \t\r\n\0')
                artists.append(Artist(artistName,artistLink))
            
            album = None
            for albumTlv in trackTlv.getAllTlvsOfType(TlvDefinitions.TlvContainerType.TLV_ALBUM):
                albumName = albumTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0')
                albumLink = albumTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_LINK)._TlvValue.rstrip(' \t\r\n\0')
                album = Album(albumName, albumLink)
            tracks.append(Track(name, uri, artists, album, False, False, False, duration))
        return tracks
           
class GenericSearchReqMsg(Message):
    def __init__(self, msgId, query):
        Message.__init__(self, TlvDefinitions.TlvMessageType.GENERIC_SEARCH_REQ, msgId)
        self._TlvSet.append(Tlv.Create(TlvDefinitions.TlvContainerType.TLV_SEARCH_QUERY, len(query), query))        
        
class GenericSearchRspMsg(Message):
    def __init__(self, msgId):
        Message.__init__(self, TlvDefinitions.TlvMessageType.GENERIC_SEARCH_RSP, msgId)
        
    def getAllTracks(self):
        trackTlvs = []
        tracks = []
        #Find all Track TLV:s recursively
        for tlv in self._TlvSet[:]:
            if(tlv._TlvType == TlvDefinitions.TlvContainerType.TLV_TRACK):
                trackTlvs.append(tlv)
            trackTlvs.extend(tlv.getAllTlvsOfType(TlvDefinitions.TlvContainerType.TLV_TRACK))
        
        #Create all the track mediacontainers
        for trackTlv in trackTlvs[:]:
            name = trackTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0')
            uri = trackTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_LINK)._TlvValue.rstrip(' \t\r\n\0')
            durationTlv = trackTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_TRACK_DURATION)._TlvValue
            duration = None
            if(durationTlv != None):
                duration, = struct.unpack('!I', durationTlv)
            
            artists = []
            for artistTlv in trackTlv.getAllTlvsOfType(TlvDefinitions.TlvContainerType.TLV_ARTIST):
                artistName = artistTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0')
                artistLink = artistTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_LINK)._TlvValue.rstrip(' \t\r\n\0')
                artists.append(Artist(artistName,artistLink))
            
            album = None
            for albumTlv in trackTlv.getAllTlvsOfType(TlvDefinitions.TlvContainerType.TLV_ALBUM):
                albumName = albumTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0')
                albumLink = albumTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_LINK)._TlvValue.rstrip(' \t\r\n\0')
                album = Album(albumName, albumLink)
            tracks.append(Track(name, uri, artists, album, False, False, False, duration))
        return tracks
        
class StatusIndMsg(Message):
    def __init__(self, msgId):
        Message.__init__(self, TlvDefinitions.TlvMessageType.STATUS_IND, msgId)

    def getState(self):
        for tlv in self._TlvSet[:]:
            if(tlv._TlvType == TlvDefinitions.TlvContainerType.TLV_STATE):
                state, = struct.unpack('!I', tlv._TlvValue)
                return state
        return None
    
    def getPlayingTrack(self):
        for tlv in self._TlvSet[:]:
            if(tlv._TlvType == TlvDefinitions.TlvContainerType.TLV_TRACK):
                name = tlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0')
                uri = tlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_LINK)._TlvValue.rstrip(' \t\r\n\0')
                
                durationTlv = tlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_TRACK_DURATION)._TlvValue
                duration = None
                if(durationTlv != None):
                    duration, = struct.unpack('!I', durationTlv)
                
                artists = []
                for artistTlv in tlv.getAllTlvsOfType(TlvDefinitions.TlvContainerType.TLV_ARTIST):
                    artistName = artistTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0')
                    artistLink = artistTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_LINK)._TlvValue.rstrip(' \t\r\n\0')
                    artists.append(Artist(artistName,artistLink))
                
                album = None
                for albumTlv in tlv.getAllTlvsOfType(TlvDefinitions.TlvContainerType.TLV_ALBUM):
                    albumName = albumTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0')
                    albumLink = albumTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_LINK)._TlvValue.rstrip(' \t\r\n\0')
                    album = Album(albumName, albumLink)
                return Track(name, uri, artists, album, False, False, False, duration)
        return None
    
    def getProgress(self):
        for tlv in self._TlvSet[:]:
            if(tlv._TlvType == TlvDefinitions.TlvContainerType.TLV_PROGRESS):
                progress, = struct.unpack('!I', tlv._TlvValue)
                return progress
        return None

        
class PlayReqMsg(Message):
    def __init__(self, msgId, uri):
        Message.__init__(self, TlvDefinitions.TlvMessageType.PLAY_REQ, msgId)
        self.addTlv(Tlv.Create(TlvDefinitions.TlvContainerType.TLV_LINK, len(uri), uri))
        
class PlayRspMsg(Message):
    def __init__(self, msgId):
        Message.__init__(self, TlvDefinitions.TlvMessageType.PLAY_RSP, msgId)

class PlayOperationReqMsg(Message):
    def __init__(self, msgId, operation):
        Message.__init__(self,TlvDefinitions.TlvMessageType.PLAY_CONTROL_REQ, msgId)
        self._TlvSet.append(Tlv.Create(TlvDefinitions.TlvContainerType.TLV_PLAY_OPERATION, 4, operation))

class PlayOperationRspMsg(Message):
    def __init__(self, msgId):
        Message.__init__(self, TlvDefinitions.TlvMessageType.PLAY_CONTROL_RSP, msgId)
    
class GetStatusReqMsg(Message):
    def __init__(self, msgId):
        Message.__init__(self, TlvDefinitions.TlvMessageType.GET_STATUS_REQ, msgId)
    
class GetStatusRspMsg(Message):
    def __init__(self, msgId):
        Message.__init__(self, TlvDefinitions.TlvMessageType.GET_STATUS_RSP, msgId)
    
    def getState(self):
        for tlv in self._TlvSet[:]:
            if(tlv._TlvType == TlvDefinitions.TlvContainerType.TLV_STATE):
                state, = struct.unpack('!I', tlv._TlvValue)
                return state
        return None
    
    def getPlayingTrack(self):
        for tlv in self._TlvSet[:]:
            if(tlv._TlvType == TlvDefinitions.TlvContainerType.TLV_TRACK):
                name = tlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0')
                uri = tlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_LINK)._TlvValue.rstrip(' \t\r\n\0')
                
                durationTlv = tlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_TRACK_DURATION)._TlvValue
                duration = None
                if(durationTlv != None):
                    duration, = struct.unpack('!I', durationTlv)
                
                artists = []
                for artistTlv in tlv.getAllTlvsOfType(TlvDefinitions.TlvContainerType.TLV_ARTIST):
                    artistName = artistTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0')
                    artistLink = artistTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_LINK)._TlvValue.rstrip(' \t\r\n\0')
                    artists.append(Artist(artistName,artistLink))
                
                album = None
                for albumTlv in tlv.getAllTlvsOfType(TlvDefinitions.TlvContainerType.TLV_ALBUM):
                    albumName = albumTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_NAME)._TlvValue.rstrip(' \t\r\n\0')
                    albumLink = albumTlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_LINK)._TlvValue.rstrip(' \t\r\n\0')
                    album = Album(albumName, albumLink)
                return Track(name, uri, artists, album, False, False, False, duration)
        return None
    
    def getProgress(self):
        for tlv in self._TlvSet[:]:
            if(tlv._TlvType == TlvDefinitions.TlvContainerType.TLV_PROGRESS):
                progress, = struct.unpack('!I', tlv._TlvValue)
                return progress
        return None
    
    
class GetImageReqMsg(Message):
    def __init__(self, msgId, uri):
        Message.__init__(self, TlvDefinitions.TlvMessageType.GET_IMAGE_REQ, msgId)
        self.addTlv(Tlv.Create(TlvDefinitions.TlvContainerType.TLV_LINK, len(uri), uri))
    
class GetImageRspMsg(Message):
    def __init__(self, msgId):
        Message.__init__(self, TlvDefinitions.TlvMessageType.GET_IMAGE_RSP, msgId)
        
    def getImageData(self):
        for tlv in self._TlvSet[:]:
            if(tlv._TlvType == TlvDefinitions.TlvContainerType.TLV_IMAGE):
                return tlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_IMAGE_DATA)._TlvValue
        return None
               
    def getImageFormat(self):
        for tlv in self._TlvSet[:]:
            if(tlv._TlvType == TlvDefinitions.TlvContainerType.TLV_IMAGE):
                imageFormat, = struct.unpack('!I', tlv.getSubTlv(TlvDefinitions.TlvContainerType.TLV_IMAGE_FORMAT)._TlvValue)
                return imageFormat
        return None