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
import Message
import TlvDefinitions
import MediaContainers

import socket
import select
import struct
from threading import Thread, Event, Lock

class SpotifyClient(Thread):
    PLAYBACK_IDLE           = TlvDefinitions.TlvPlaybackState.PLAYBACK_IDLE
    PLAYBACK_PLAYING        = TlvDefinitions.TlvPlaybackState.PLAYBACK_PLAYING
    PLAYBACK_PAUSED         = TlvDefinitions.TlvPlaybackState.PLAYBACK_PAUSED
    
    PLAY_OP_PAUSE           = TlvDefinitions.TlvPlayOperation.PLAY_OP_PAUSE
    PLAY_OP_RESUME          = TlvDefinitions.TlvPlayOperation.PLAY_OP_RESUME
    PLAY_OP_NEXT            = TlvDefinitions.TlvPlayOperation.PLAY_OP_NEXT
    PLAY_OP_PREV            = TlvDefinitions.TlvPlayOperation.PLAY_OP_PREV
    
    IMAGE_FORMAT_UNKNOWN    = TlvDefinitions.TlvImageFormat.IMAGE_FORMAT_UNKNOWN
    IMAGE_FORMAT_JPEG       = TlvDefinitions.TlvImageFormat.IMAGE_FORMAT_JPEG
        
        
    def __init__(self, host='127.0.0.1', port=7788):
        self.host = host
        self.port = port
        self.__msgIdCounter = 0
        self.__cancellationPending = Event()
        self.__isConnected = Event()
        self.__playlists = []
        
        self.__connectionObserverLock = Lock()
        self.__connectionObservers = []
        
        self.__playlistUpdatedObserversLock = Lock() 
        self.__playlistUpdatedObservers = []
        
        self.__statusIndObserversLock = Lock() 
        self.__statusIndObservers = []
        
        self.__getRspMsgObserversLock = Lock()
        self.__getRspMsgObservers = {}
        
        self.fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        Thread.__init__(self)
        self.daemon = True
        self.start()
  
    def reconnect(self, host, port):
        self.host = host
        self.port = port
        self.fd.shutdown(2) #SHUT_RDWR
        self.fd.close()
        self.fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__isConnected.clear()
                    
    def run(self):
        while (self.__cancellationPending.is_set() == False):
            if(self.__isConnected.is_set() == False):
                try:
                    self.__connectionObserverLock.acquire()
                    for obs in self.__connectionObservers:
                        obs.disconnectedIndCb()
                    self.__connectionObserverLock.release()
                    
                    self.fd.connect((self.host, self.port))
                    self.__isConnected.set()
                    self.__connectionObserverLock.acquire()
                    for obs in self.__connectionObservers:
                        obs.connectedIndCb()
                    self.__connectionObserverLock.release()
                except socket.error:
                    continue
                    
            try:
                inputFds,outputFds,exceptFds = select.select([self.fd], [], [], 1)
            except:
                self.__isConnected.clear()
                self.__connectionObserverLock.acquire()
                for obs in self.__connectionObservers:
                    obs.disconnectedIndCb()
                self.__connectionObserverLock.release()
                continue
                        
            for fd in inputFds:
                if fd == self.fd:
                    msg = self.recvMsg()
                    if(msg == None):
                        continue
                    msgType, msgId = struct.unpack('!II', msg[4:12])
                    
                    if(msgType == TlvDefinitions.TlvMessageType.GET_PLAYLISTS_RSP):
                        print "Received msg GET_PLAYLISTS_RSP"
                        msgObj = Message.GetPlaylistRspMsg(msgId)
                        msgObj.Decode(msg)
                        self.__playlists = msgObj.getAllPlaylists()
                        self.__playlistUpdatedObserversLock.acquire()
                        for observer in self.__playlistUpdatedObservers:
                            observer.playlistUpdatedIndCb()
                        self.__playlistUpdatedObserversLock.release()
                        
                    elif(msgType == TlvDefinitions.TlvMessageType.GET_TRACKS_RSP):
                        print "Received msg GET_TRACKS_RSP"
                        msgObj = Message.GetTracksRspMsg(msgId)
                        msgObj.Decode(msg)
                        tracks = msgObj.getAllTracks()
                        self.__getRspMsgObserversLock.acquire()
                        try:
                            self.__getRspMsgObservers[msgId].getTracksRspCb(tracks)
                            del self.__getRspMsgObservers[msgId]
                        except KeyError:
                            print "Unknown msgId " + str(msgId)
                        self.__getRspMsgObserversLock.release()
                        
                    elif(msgType == TlvDefinitions.TlvMessageType.GENERIC_SEARCH_RSP):
                        print "Received msg GENERIC_SEARCH_RSP"
                        msgObj = Message.GenericSearchRspMsg(msgId)
                        msgObj.Decode(msg)
                        tracks = msgObj.getAllTracks()
                        self.__getRspMsgObserversLock.acquire()
                        try:
                            self.__getRspMsgObservers[msgId].searchRspCb(tracks)
                            del self.__getRspMsgObservers[msgId]
                        except KeyError:
                            print "Unknown msgId " + str(msgId)
                        self.__getRspMsgObserversLock.release()
                        
                    elif(msgType == TlvDefinitions.TlvMessageType.STATUS_IND):
                        print "Received msg STATUS_IND"
                        msgObj = Message.StatusIndMsg(msgId)
                        msgObj.Decode(msg)
                        state = msgObj.getState()
                        if(state == TlvDefinitions.TlvPlaybackState.PLAYBACK_IDLE):
                            self.__statusIndObserversLock.acquire()
                            for obs in self.__statusIndObservers:
                                obs.statusIndCb(self.PLAYBACK_IDLE, None, msgObj.getProgress())
                            self.__statusIndObserversLock.release()
                        
                        elif(state == TlvDefinitions.TlvPlaybackState.PLAYBACK_PLAYING):
                            self.__statusIndObserversLock.acquire()
                            for obs in self.__statusIndObservers:
                                obs.statusIndCb(self.PLAYBACK_PLAYING, msgObj.getPlayingTrack(), msgObj.getProgress())
                            self.__statusIndObserversLock.release()
                            
                        elif(state == TlvDefinitions.TlvPlaybackState.PLAYBACK_PAUSED):
                            self.__statusIndObserversLock.acquire()
                            for obs in self.__statusIndObservers:
                                obs.statusIndCb(self.PLAYBACK_PAUSED, msgObj.getPlayingTrack(), msgObj.getProgress())
                            self.__statusIndObserversLock.release()
                        else:
                            print "Unknown state=" + str(state)
                            
                    elif(msgType == TlvDefinitions.TlvMessageType.PLAY_RSP):
                        print "Received msg PLAY_RSP"
                        msgObj = Message.PlayRspMsg(msgId)
                        msgObj.Decode(msg)
    
                    elif(msgType == TlvDefinitions.TlvMessageType.PLAY_CONTROL_RSP):
                        print "Received msg PLAY_CONTROL_RSP"
                        msgObj = Message.PlayOperationRspMsg(msgId)
                        msgObj.Decode(msg)
                    
                    elif(msgType == TlvDefinitions.TlvMessageType.GET_STATUS_RSP):
                        print "Received msg GET_STATUS_RSP"
                        msgObj = Message.GetStatusRspMsg(msgId)
                        msgObj.Decode(msg)
                        state = msgObj.getState()
                        if(state == TlvDefinitions.TlvPlaybackState.PLAYBACK_IDLE):
                            self.__statusIndObserversLock.acquire()
                            for obs in self.__statusIndObservers:
                                obs.statusIndCb(self.PLAYBACK_IDLE, None, msgObj.getProgress())
                            self.__statusIndObserversLock.release()
                        
                        elif(state == TlvDefinitions.TlvPlaybackState.PLAYBACK_PLAYING):
                            self.__statusIndObserversLock.acquire()
                            for obs in self.__statusIndObservers:
                                obs.statusIndCb(self.PLAYBACK_PLAYING, msgObj.getPlayingTrack(), msgObj.getProgress())
                            self.__statusIndObserversLock.release()
                            
                        elif(state == TlvDefinitions.TlvPlaybackState.PLAYBACK_PAUSED):
                            self.__statusIndObserversLock.acquire()
                            for obs in self.__statusIndObservers:
                                obs.statusIndCb(self.PLAYBACK_PAUSED, msgObj.getPlayingTrack(), msgObj.getProgress())
                            self.__statusIndObserversLock.release()
                        else:
                            print "Unknown state=" + str(state)
                    
                    elif(msgType == TlvDefinitions.TlvMessageType.GET_IMAGE_RSP):
                        print "Received msg GET_IMAGE_RSP"
                        msgObj = Message.GetImageRspMsg(msgId)
                        msgObj.Decode(msg)
                        self.__getRspMsgObserversLock.acquire()
                        try:
                            self.__getRspMsgObservers[msgId].getImageReqCb(msgObj.getImageFormat(), 
                                                                            msgObj.getImageData())
                            del self.__getRspMsgObservers[msgId]
                        except KeyError:
                            print "Unknown msgId " + str(msgId)
                        self.__getRspMsgObserversLock.release()
                        
                        
                    elif(msgType == TlvDefinitions.TlvMessageType.GET_TRACKS_REQ or
                        msgType == TlvDefinitions.TlvMessageType.GENERIC_SEARCH_REQ or
                        msgType == TlvDefinitions.TlvMessageType.PLAY_REQ or
                        msgType == TlvDefinitions.TlvMessageType.PLAY_CONTROL_REQ or
                        msgType == TlvDefinitions.TlvMessageType.GET_STATUS_REQ or
                        msgType == TlvDefinitions.TlvMessageType.GET_IMAGE_REQ):
                        print "Received REQ message eventhough we are the client!! Strange! type=" + hex(msgType)
                        break
                    else:
                        msgObj = None
                        
        return
    def stop(self):
        self.__cancellationPending.set()
            
    def getNextMsgId(self):
        if(self.__msgIdCounter < 0xFFFFFFFF):
            self.__msgIdCounter = self.__msgIdCounter + 1
        else:
            self.__msgIdCounter = self.__msgIdCounter = 0
        return self.__msgIdCounter
    
    def sendGetPlaylistReq(self):
        if self.__isConnected.is_set():
            self.fd.sendall(Message.GetPlaylistReqMsg(self.getNextMsgId()).toByteStream())
    
    def sendGetTracksReq(self, requester, playlistUri):
        if self.__isConnected.is_set():
            msgId = self.getNextMsgId()
            self.__getRspMsgObserversLock.acquire()
            self.__getRspMsgObservers[msgId] = requester
            self.__getRspMsgObserversLock.release()
            self.fd.sendall(Message.GetTracksReqMsg(msgId, playlistUri).toByteStream())
    
    def sendPlayReq(self, uri):
        if self.__isConnected.is_set():
            self.fd.sendall(Message.PlayReqMsg(self.getNextMsgId(), uri).toByteStream())
    
    def sendPlayOperation(self, playOp):
        if self.__isConnected.is_set():
            if(playOp == self.PLAY_OP_PAUSE):
                self.fd.sendall(Message.PlayOperationReqMsg(self.getNextMsgId(), 
                                                            TlvDefinitions.TlvPlayOperation.PLAY_OP_PAUSE).toByteStream())
            elif(playOp == self.PLAY_OP_RESUME):
                self.fd.sendall(Message.PlayOperationReqMsg(self.getNextMsgId(), 
                                                            TlvDefinitions.TlvPlayOperation.PLAY_OP_RESUME).toByteStream())
            elif(playOp == self.PLAY_OP_NEXT):
                self.fd.sendall(Message.PlayOperationReqMsg(self.getNextMsgId(), 
                                                            TlvDefinitions.TlvPlayOperation.PLAY_OP_NEXT).toByteStream())
            elif(playOp == self.PLAY_OP_PREV):
                self.fd.sendall(Message.PlayOperationReqMsg(self.getNextMsgId(), 
                                                            TlvDefinitions.TlvPlayOperation.PLAY_OP_PREV).toByteStream())
            else:
                print "sendPlayOperation, unknown playOp=" + str(playOp)
        
    def sendSearchReq(self, requester, searchReq):
        if self.__isConnected.is_set():
            msgId = self.getNextMsgId()
            self.__getRspMsgObserversLock.acquire()
            self.__getRspMsgObservers[msgId] = requester
            self.__getRspMsgObserversLock.release()
            self.fd.sendall(Message.GenericSearchReqMsg(msgId, searchReq).toByteStream())
                
    def sendGetImageReq(self, requester, uri):
        if self.__isConnected.is_set():
            msgId = self.getNextMsgId()
            self.__getRspMsgObserversLock.acquire()
            self.__getRspMsgObservers[msgId] = requester
            self.__getRspMsgObserversLock.release()
            self.fd.sendall(Message.GetImageReqMsg(msgId, uri).toByteStream())
    
    def sendGetStatusReq(self):
        if self.__isConnected.is_set():
            self.fd.sendall(Message.GetStatusReqMsg(self.getNextMsgId()).toByteStream())
                
    def recvMsg(self):
        try:
            inputStr = self.fd.recv(1500)
            if(inputStr == None):
                return
        except:
            return
        header = struct.unpack('!I', inputStr[0:4]) 
        totalLength, = header
        while totalLength > len(inputStr):
            try:
                inputStr += self.fd.recv(1500)
            except:
                return
        return inputStr
    
    def getPlaylists(self):
        return self.__playlists
    
    def registerPlaylistUpdatedIndObserver(self, observer):
        self.__playlistUpdatedObserversLock.acquire()
        self.__playlistUpdatedObservers.append(observer)
        self.__playlistUpdatedObserversLock.release()
        
    def registerStatusIndObserver(self, observer):
        self.__statusIndObserversLock.acquire()
        self.__statusIndObservers.append(observer)
        self.__statusIndObserversLock.release()
    
    def registerConnectionObserver(self, observer):
        self.__connectionObserverLock.acquire()
        self.__connectionObservers.append(observer)
        self.__connectionObserverLock.release()
        