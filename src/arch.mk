ARCH=$(shell uname)_$(shell uname -m)


DEPS_PATH = ../../spotifyserver_deps
APPKEY= $(DEPS_PATH)/appkey.c

########################################
# ARCH
########################################
ifeq ($(ARCH),Windows_whatever)
	EXECUTABLE_EXT = exe
	VPATH += Platform/Socket/Windows Platform/Threads/Windows
	LIBSPOTIFY = $(DEPS_PATH)/lib/libspotify-Windows-

else ifeq ($(ARCH),Linux_x86_64)
	EXECUTABLE_EXT = elf
	VPATH += Platform/Socket/Linux Platform/Threads/Linux Platform/Utils/Linux
	ARCH_OBJECTS +=  LinuxRunnable.o LinuxMutex.o LinuxCondition.o LinuxSocket.o LinuxUtils.o
	LIBSPOTIFY = $(DEPS_PATH)/lib/libspotify-Linux-x86_64-release
	AUDIO_DRIVER=alsa

else ifeq ($(ARCH),Linux_i686)
	EXECUTABLE_EXT = elf
	VPATH += Platform/Socket/Linux Platform/Threads/Linux Platform/Utils/Linux
	ARCH_OBJECTS +=  LinuxRunnable.o LinuxMutex.o LinuxCondition.o LinuxSocket.o LinuxUtils.o
	LIBSPOTIFY = $(DEPS_PATH)/lib/libspotify-Linux-i686-release
	AUDIO_DRIVER=alsa

else ifeq ($(ARCH),CYGWIN_NT-6.1_i686)

ifdef NO_CYGWIN
	VPATH += Platform/Socket/Windows Platform/Threads/Windows
	ARCH_OBJECTS +=  WindowsRunnable.o WindowsMutex.o WindowsCondition.o WindowsSocket.o
	LIBSPOTIFY = $(DEPS_PATH)/lib/libspotify-Linux-i686-release
	CFLAGS += -mno-cygwin
else
	VPATH += Platform/Socket/Linux Platform/Threads/Linux Platform/Utils/Linux
	ARCH_OBJECTS +=  LinuxRunnable.o LinuxMutex.o LinuxCondition.o LinuxSocket.o LinuxUtils.o
	LIBSPOTIFY = $(DEPS_PATH)/lib/libspotify-10.1.16-win32-release
endif
	EXECUTABLE_EXT = exe
	AUDIO_DRIVER=stub
	CFLAGS += -maccumulate-outgoing-args -fomit-frame-pointer

else
	error = $(shell echo invalid arch $(ARCH))
endif

########################################
# AUDIO DRIVERS
########################################
ifeq ($(AUDIO_DRIVER),alsa)
	INCLUDES  += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags alsa)
	LIBPATHS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs-only-L alsa)
	LIBS  += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs-only-l --libs-only-other alsa) 
	AUDIO_OBJECTS += AudioEndpoint-Alsa.o
else ifeq ($(AUDIO_DRIVER),stub)
	AUDIO_OBJECTS += AudioEndpoint-Stub.o
else
	error = $(shell echo No audio driver supplied AUDIO_DRIVER=$(AUDIO_DRIVER))
endif
