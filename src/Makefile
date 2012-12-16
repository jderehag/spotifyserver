ROOT = $(CURDIR)

include arch.mk

CCX = g++
CC = gcc
CFLAGS += -O0 -g3 -Wall -c


TARGET = spotifyserver

VPATH += ConfigHandling \
	 ConfigHandling/Configs \
	 LibSpotifyIf \
	 LibSpotifyIf/MediaContainers \
	 LibSpotifyIf/LibSpotifyStub \
	 ClientHandler \
	 Platform/Threads \
	 Platform/Socket \
	 Platform/AudioEndpoints \
	 Platform/AudioEndpoints/Endpoints \
	 MessageFactory \
	 Logger \
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

TESTAPP_OBJECTS +=  testapp2.o \
					Messenger.o \
					SocketClient.o \
					IUserInterface.o \
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
		-ILogger \
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

