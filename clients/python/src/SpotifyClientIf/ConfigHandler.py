'''
Created on 26 feb 2012

@author: jeppe
'''
import pickle
from threading import Lock

class ConfigHandler(object):
    def __init__(self):
        self.__ipaddr = "127.0.0.1"
        self.__port = 7788
        
        self.__configSubscriberLock = Lock()
        self.__configSubscribers = []
    
    def __getstate__(self):
        result = self.__dict__.copy()
        # Not very pretty, is it possible to append class name automatically?
        del result['_ConfigHandler__configSubscriberLock']
        del result['_ConfigHandler__configSubscribers']
        return result
    
    def __setstate__(self,state):
        self.__dict__.update(state)
        
    def setIpAddr(self, ipaddr):
        self.__ipaddr = ipaddr
        
    def getIpAddr(self):
        return self.__ipaddr
    
    def setPort(self, port):
        self.__port = port
        
    def getPort(self):
        return self.__port
    
    def configUpdateDoneInd(self):
        self.__configSubscriberLock.acquire()
        for subscriber in self.__configSubscribers:
            subscriber.configUpdateIndCb()
        self.__configSubscriberLock.release()
    
    def registerForConfigUpdates(self, registrator):
        self.__configSubscriberLock.acquire()
        self.__configSubscribers.append(registrator)
        self.__configSubscriberLock.release()
        
    def writeToFile(self):
        outputFile = open("config.dat", "wb")
        pickle.dump(self,outputFile)
        outputFile.close()
        print "wrote to file"
            
    def readFromFile(self):
        inputFile = open("config.dat", "rb")
        # Not very pretty, if I pickle directly to self, things wont get updated properly,
        # is there another way to do this?
        tmp = pickle.load(inputFile)
        self.__ipaddr = tmp.__ipaddr
        self.__port = tmp.__port
        inputFile.close()