
include ../build/arch.mk

TARGET=spotifyserver

# Directory for output files (lst, obj, dep etc.)
#we need specific name for this since we still have dependencies between client and server
OUTDIR = .spotifyserver.output

DEPS_PATH ?= ../../spotifyserver_deps

# root directories to look for source files in
VPATH += ./../common/ ./../clients/cpp/src $(DEPS_PATH)


# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files

SRC =	appkey.c

# List C++ source files here.
# use file-extension .cpp for C++-files (not .C)
CPPSRC =	spotifyserver.cpp \
			$(addprefix ClientHandler/, ClientHandler.cpp Client.cpp) \
			$(addprefix ConfigHandling/, ConfigHandler.cpp ConfigParser.cpp) \
			$(addprefix ConfigHandling/Configs/, SpotifyConfig.cpp NetworkConfig.cpp AudioEndpointConfig.cpp LoggerConfig.cpp) \
			$(addprefix LibSpotifyIf/, LibSpotifyIf.cpp LibSpotifyIfCallbackWrapper.cpp LibSpotifyIfHelpers.cpp LibSpotifyPlaybackHandler.cpp) \

CPPSRC +=	$(addprefix MediaContainers/, Album.cpp Artist.cpp Folder.cpp Playlist.cpp Track.cpp) \
			$(addprefix MessageFactory/, Message.cpp MessageDecoder.cpp MessageEncoder.cpp TlvDefinitions.cpp Tlvs.cpp SocketReader.cpp SocketWriter.cpp) \
			$(addprefix SocketHandling/, Messenger.cpp SocketServer.cpp SocketPeer.cpp ) \
			$(addprefix Platform/AudioEndpoints/, AudioEndpointLocalShared.cpp AudioEndpointRemote.cpp AudioDispatch.cpp AudioFifo.cpp ) \
			AudioEndpointManager/AudioEndpointManager.cpp \
			Logger/Logger.cpp Logger/LoggerImpl.cpp \
			MediaInterface/MediaInterface.cpp \
			UIConsole.cpp

CPPSRC +=	$(ARCH_SRC) $(AUDIO_SRC)

STUB_SRC = LibSpotifyIf/LibSpotifyStub/LibSpotifyStub.c

# List Assembler source files here.
ASRC = 

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS += ../common/ ../common/Logger Utils $(LIBSPOTIFY)/include

EXTRAINCDIRS += ../clients/cpp/src

# Extra libraries
#    Each library-name must be seperated by a space.
#    i.e. to link with libxyz.a, libabc.a and libefsl.a: 
#    EXTRA_LIBS = xyz abc efsl
EXTRA_LIBS += pthread rt spotify


# List any extra directories to look for library files here.
# Also add directories where the linker should search for
# includes from linker-script to the list
#     Each directory must be seperated by a space.
EXTRA_LIBDIRS += $(LIBSPOTIFY)/lib


# Place project-specific -D (define) and/or 
# -U options for C and C++ here.
CDEFS += 

# Place project-specific -D and/or -U options for 
# Assembler with preprocessor here.
ADEFS = -D__ASSEMBLY__

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
#OPT = s
#OPT = 2
#OPT = 3
OPT = 0

DEBUG = 3

#-----

# Default target.
.DEFAULT_GOAL := all
all: spotifyserver stub

# General make rules
include ../build/build.mk

# Specific targets.
spotifyserver: dirs gccversion spotifyserver.$(EXECUTABLE_EXT)

STUB_OBJ = $(OUTDIR)/$(basename $(STUB_SRC)).o
$(eval $(call COMPILE_C_TEMPLATE, $(STUB_SRC)))

stub: SRC += $(STUB_SRC)
stub: TARGET=spotifyserver_stub
stub: dirs $(STUB_OBJ) spotifyserver_stub.$(EXECUTABLE_EXT)
spotifyserver_stub: stub

# Listing of phony targets.
.PHONY : spotifyserver spotifyserver_stub stub

# Include the dependency files.
ifneq ($(MAKECMDGOALS),clean)
-include $(wildcard $(OUTDIR)/dep/*)
endif
