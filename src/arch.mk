ARCH=$(shell uname)_$(shell uname -m)


########################################
# ARCH
########################################
ifeq ($(ARCH),Windows_whatever)
	EXECUTABLE_EXT = exe
	ARCH_SRC = 	$(addprefix Platform/Threads/Windows/, WindowsRunnable.cpp WindowsMutex.cpp WindowsCondition.cpp WindowsMessagebox.cpp) \
				Platform/Socket/Windows/WindowsSocket.cpp \ 
				Platform/Utils/Windows/WindowsUtils.cpp
	LIBSPOTIFY = $(DEPS_PATH)/lib/libspotify-Windows-

else ifeq ($(ARCH),Linux_x86_64)
	EXECUTABLE_EXT = elf
	ARCH_SRC = 	$(addprefix Platform/Threads/Linux/, LinuxRunnable.cpp LinuxMutex.cpp LinuxCondition.cpp LinuxMessagebox.cpp) \
				Platform/Socket/Linux/LinuxSocket.cpp \ 
				Platform/Utils/Linux/LinuxUtils.cpp
	LIBSPOTIFY = $(DEPS_PATH)/lib/libspotify-Linux-x86_64-release
	AUDIO_DRIVER=alsa

else ifeq ($(ARCH),Linux_i686)
	EXECUTABLE_EXT = elf
	ARCH_SRC = 	$(addprefix Platform/Threads/Linux/, LinuxRunnable.cpp LinuxMutex.cpp LinuxCondition.cpp LinuxMessagebox.cpp) \
				Platform/Socket/Linux/LinuxSocket.cpp \ 
				Platform/Utils/Linux/LinuxUtils.cpp
	LIBSPOTIFY = $(DEPS_PATH)/lib/libspotify-Linux-i686-release
	AUDIO_DRIVER=alsa

else ifeq ($(ARCH),CYGWIN_NT-6.1_i686)

ifdef NO_CYGWIN
	ARCH_SRC = 	$(addprefix Platform/Threads/Windows/, WindowsRunnable.cpp WindowsMutex.cpp WindowsCondition.cpp WindowsMessagebox.cpp) \
				Platform/Socket/Windows/WindowsSocket.cpp \ 
				Platform/Utils/Windows/WindowsUtils.cpp
	LIBSPOTIFY = $(DEPS_PATH)/lib/libspotify-Linux-i686-release
	CFLAGS += -mno-cygwin
else
	ARCH_SRC = 	$(addprefix Platform/Threads/Linux/, LinuxRunnable.cpp LinuxMutex.cpp LinuxCondition.cpp LinuxMessagebox.cpp) \
				Platform/Socket/Linux/LinuxSocket.cpp \ 
				Platform/Utils/Linux/LinuxUtils.cpp
	LIBSPOTIFY = $(DEPS_PATH)/lib/libspotify-10.1.16-win32-release
endif
	EXECUTABLE_EXT = exe
	AUDIO_DRIVER=stub
	CFLAGS += -maccumulate-outgoing-args -fomit-frame-pointer

else ifeq ($(ARCH),CYGWIN_NT-6.2-WOW64_i686)

	ARCH_SRC = 	$(addprefix Platform/Threads/Linux/, LinuxRunnable.cpp LinuxMutex.cpp LinuxCondition.cpp LinuxMessagebox.cpp) \
				Platform/Socket/Linux/LinuxSocket.cpp \
				Platform/Utils/Linux/LinuxUtils.cpp
	LIBSPOTIFY = $(DEPS_PATH)/lib/libspotify-10.1.16-win32-release
	EXECUTABLE_EXT = exe
	AUDIO_DRIVER=stub
	CFLAGS += -maccumulate-outgoing-args -fomit-frame-pointer

else
	error = $(shell echo invalid arch $(ARCH))
endif

########################################
# AUDIO DRIVERS
########################################
AUDIO_PATH = Platform/AudioEndpoints/Endpoints
ifeq ($(AUDIO_DRIVER),alsa)
	INCLUDES  += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags alsa)
	LIBPATHS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs-only-L alsa)
	LIBS  += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs-only-l --libs-only-other alsa) 
	AUDIO_SRC += $(AUDIO_PATH)/AudioEndpoint-Alsa.cpp
else ifeq ($(AUDIO_DRIVER),stub)
	AUDIO_SRC += $(AUDIO_PATH)/AudioEndpoint-Stub.cpp
else
	error = $(shell echo No audio driver supplied AUDIO_DRIVER=$(AUDIO_DRIVER))
endif
