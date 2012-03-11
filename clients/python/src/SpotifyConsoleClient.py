#!/usr/bin/python
'''
Created on 12 feb 2012

@author: jeppe
'''
from SpotifyClientIf import SpotifyClient
import time

class SpotifyClientObserver(object):
    def __init__(self, client):
        self.__client = client
        
        
    def playlistUpdatedIndCb(self):
        for playlist in self.__client.getPlaylists():
            print "Playlist=" + playlist.getName()
        
def SpotifyConsoleClient():
    
    client = SpotifyClient.SpotifyClient()
    spotifyClientObserver = SpotifyClientObserver(client)
    client.registerPlaylistUpdatedIndObserver(spotifyClientObserver)
    client.sendPlaylistReq()
    
    
    time.sleep(2)
    print "stopping client"
    client.stop()
    return
    
    
    
if __name__ == '__main__':
    SpotifyConsoleClient()