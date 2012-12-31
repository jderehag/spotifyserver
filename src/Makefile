ROOT = $(CURDIR)

include arch.mk

CCX = g++
CC = gcc
CFLAGS += -O0 -g3 -Wall -c


TARGET = spotifyserver

VPATH += ConfigHandling \
	 ConfigHandling/Configs \
	 LibSpotifyIf \
	 ../common/MediaInterface \
	 ../common/MediaContainers \
	 LibSpotifyIf/LibSpotifyStub \
	 ClientHandler \
	 ../common/Platform/Threads \
	 ../common/Platform/Socket \
	 ../common/Platform/AudioEndpoints \
	 ../common/Platform/AudioEndpoints/Endpoints \
	 ../common/MessageFactory \
	 ../common/Logger \
	 ../common/SocketHandling \
	 TestApp

COMMON_OBJECTS += spotifyserver.o \
		  ConfigHandler.o \
		  ConfigParser.o \
		  SpotifyConfig.o \
		  NetworkConfig.o \
		  AudioEndpointConfig.o \
		  LoggerConfig.o \
		  Logger.o \
		  LibSpotifyIf.o \
		  LibSpotifyIfCallbackWrapper.o \
		  LibSpotifyIfHelpers.o \
		  LibSpotifyPlaybackHandler.o \
		  MediaInterface.o \
		  Folder.o \
		  Playlist.o \
		  Track.o \
		  Artist.o \
		  Album.o \
		  ClientHandler.o \
		  Client.o \
		  SocketServer.o \
		  SocketPeer.o \
		  AudioFifo.o \
		  AudioEndpointRemote.o \
		  MessageDecoder.o \
		  MessageEncoder.o \
		  Message.o \
		  Messenger.o \
		  SocketClient.o \
		  SocketReader.o \
		  SocketWriter.o \
		  Tlvs.o \
		  TlvDefinitions.o
		  
COMMON_OBJECTS += $(ARCH_OBJECTS) $(AUDIO_OBJECTS)

TESTAPP_OBJECTS +=  testapp.o \
					UIConsole.o \
					Messenger.o \
					SocketClient.o \
					RemoteMediaInterface.o \
					MediaInterface.o \
					LoggerConfig.o \
					Logger.o \
					MessageDecoder.o \
					MessageEncoder.o \
					Message.o \
					SocketReader.o \
					SocketWriter.o \
					Tlvs.o \
					TlvDefinitions.o \
					Folder.o \
					Playlist.o \
					Track.o \
					Artist.o \
					Album.o \
					NetworkConfig.o \
					AudioEndpointConfig.o \
					SocketServer.o \
					SocketPeer.o \
					AudioFifo.o \
					AudioEndpointRemoteSocketServer.o \
					AudioEndpointRemotePeer.o
					

TESTAPP_OBJECTS += $(ARCH_OBJECTS) $(AUDIO_OBJECTS)

INCLUDES += -I./ \
		-I../common/ \
		-I../common/Logger \
	    -IUtils \
	    -I$(LIBSPOTIFY)/include

LIBS += -lpthread -lrt
LIBPATHS += -L$(LIBSPOTIFY)/lib
	
APPKEYOBJ=$(addsuffix .o, $(basename $(APPKEY)))

.PHONY: all clean test
.SILENT:

all: $(TARGET) test stub

test: $(TESTAPP_OBJECTS)
	@echo ...Linking testapp for $(ARCH)
	$(CCX) -o testapp.$(EXECUTABLE_EXT) $(TESTAPP_OBJECTS) $(LIBS)

$(TARGET): $(APPKEYOBJ) $(COMMON_OBJECTS)
	@echo ...Linking $(TARGET).$(EXECUTABLE_EXT) for $(ARCH)
	$(CCX) $(LIBPATHS) -o $(TARGET).$(EXECUTABLE_EXT) $(APPKEYOBJ) $(COMMON_OBJECTS) $(LIBS) -lspotify

stub: $(APPKEYOBJ) $(COMMON_OBJECTS) LibSpotifyStub.o
	@echo ...Linking $(TARGET)_stub.$(EXECUTABLE_EXT) for $(ARCH)
	$(CCX) $(LIBPATHS) -o $(TARGET)_stub.$(EXECUTABLE_EXT) $(APPKEYOBJ) $(COMMON_OBJECTS) LibSpotifyStub.o $(LIBS)

%.o: %.c
	@echo Building $@
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES)

%.o: %.cpp
	@echo Building $@
	$(CCX) -o $@ $< $(CFLAGS) $(INCLUDES)
	
clean:
	rm -f *.o
	rm -f *.$(EXECUTABLE_EXT)

