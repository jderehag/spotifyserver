

# Directory for output files (lst, obj, dep etc.)
OUTDIR = .output

# Target file name (without extension).
TARGET = spotifyserver

DEPS_PATH = ../../spotifyserver_deps

# root directories to look for source files in
VPATH += ./../common/ ./../clients/cpp/src $(DEPS_PATH)

include arch.mk

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files

SRC  =	appkey.c

# List C++ source files here.
# use file-extension .cpp for C++-files (not .C)
CPPSRC =	spotifyserver.cpp \
			$(addprefix ClientHandler/, ClientHandler.cpp Client.cpp) \
			$(addprefix ConfigHandling/, ConfigHandler.cpp ConfigParser.cpp) \
			$(addprefix ConfigHandling/Configs/, SpotifyConfig.cpp NetworkConfig.cpp AudioEndpointConfig.cpp LoggerConfig.cpp) \
			$(addprefix LibspotifyIf/, LibSpotifyIf.cpp LibSpotifyIfCallbackWrapper.cpp LibSpotifyIfHelpers.cpp LibSpotifyPlaybackHandler.cpp) \
			
CPPSRC +=	$(addprefix MediaContainers/, Album.cpp Artist.cpp Folder.cpp Playlist.cpp Track.cpp) \
			$(addprefix MessageFactory/, Message.cpp MessageDecoder.cpp MessageEncoder.cpp TlvDefinitions.cpp Tlvs.cpp SocketReader.cpp SocketWriter.cpp) \
			$(addprefix SocketHandling/, Messenger.cpp SocketServer.cpp SocketPeer.cpp ) \
			$(addprefix Platform/AudioEndpoints/, AudioEndpointRemote.cpp AudioDispatch.cpp AudioFifo.cpp ) \
			AudioEndpointManager/AudioEndpointManager.cpp \
			Logger/Logger.cpp \
		 	MediaInterface/MediaInterface.cpp \
			UIConsole.cpp

CPPSRC +=	$(ARCH_SRC) $(AUDIO_SRC)

STUB_SRC = LibspotifyIf/LibSpotifyStub/LibSpotifyStub.c

# List Assembler source files here.
ASRC = 

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS  = ../common/ ../common/Logger Utils $(LIBSPOTIFY)/include

EXTRAINCDIRS += ../clients/cpp/src

# Extra libraries
#    Each library-name must be seperated by a space.
#    i.e. to link with libxyz.a, libabc.a and libefsl.a: 
#    EXTRA_LIBS = xyz abc efsl
EXTRA_LIBS = pthread rt spotify


# List any extra directories to look for library files here.
# Also add directories where the linker should search for
# includes from linker-script to the list
#     Each directory must be seperated by a space.
EXTRA_LIBDIRS = $(LIBSPOTIFY)/lib


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


# Compiler flags.

# Flags for C and C++
CFLAGS =  -g$(DEBUG)
CFLAGS += -O$(OPT)
CFLAGS += $(CDEFS)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.
CFLAGS += -Wall -Wextra
CFLAGS += -Wcast-align -Wpointer-arith
CFLAGS += -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align
CFLAGS += -Wno-cast-qual
#CFLAGS += -pedantic
CFLAGS += -Wa,-adhlns=$(addprefix $(OUTDIR)/, $(notdir $(addsuffix .lst, $(basename $<))))
# Compiler flags to generate dependency files:
CFLAGS += -MD -MP -MF $(OUTDIR)/dep/$(@F).d

# flags only for C
CONLYFLAGS += 
CONLYFLAGS += $(CSTANDARD)

# flags only for C++ (arm-*-g++)
CPPFLAGS = 

# Assembler flags.
#  -Wa,...:    tell GCC to pass this to the assembler.
#  -ahlns:     create listing
#  -g$(DEBUG): have the assembler create line number information
ASFLAGS += $(ADEFS)
ASFLAGS += -Wa,-adhlns=$(addprefix $(OUTDIR)/, $(notdir $(addsuffix .lst, $(basename $<))))
ASFLAGS += -Wa,-g$(DEBUG)
ASFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))


# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -Wl,-Map=$(OUTDIR)/$(TARGET).map,--cref,--gc-sections
#not in CPP
#LDFLAGS += -nostartfiles
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS)) 


# Define programs and commands.
CC      = $(TCHAIN_PREFIX)gcc
CPP     = $(TCHAIN_PREFIX)g++
AR      = $(TCHAIN_PREFIX)ar
OBJCOPY = $(TCHAIN_PREFIX)objcopy
OBJDUMP = $(TCHAIN_PREFIX)objdump
SIZE    = $(TCHAIN_PREFIX)size
NM      = $(TCHAIN_PREFIX)nm
REMOVE  = rm -rf
SHELL   = sh
###COPY    = cp

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = ---- Linking :
MSG_COMPILING = ---- Compiling C :
MSG_COMPILINGCPP = ---- Compiling C++ :
MSG_ASSEMBLING = ---- Assembling:
MSG_CLEANING = Cleaning project:

# List of all source files.
ALLSRC     = $(ASRC) $(SRC) $(CPPSRC)
# List of all source files with directory but without file-extension.
ALLSRCBASE = $(basename $(ALLSRC))

# Define all object files.
ALLOBJFILES = $(addsuffix .o, $(ALLSRCBASE))
ALLOBJ     = $(addprefix $(OUTDIR)/, $(ALLOBJFILES))

STUB_OBJ = $(OUTDIR)/$(basename $(STUB_SRC)).o

# Default target.
all: spotifyserver stub


# Assemble: create object files from assembler source files.
define ASSEMBLE_TEMPLATE
$(OUTDIR)/$(basename $(1)).o : $(1)
	@echo $(MSG_ASSEMBLING) $$< to $$@
	$$(call makedir, $$(OUTDIR)/$$(dir $(1)))
	@$(CC) -c $(THUMB) $$(ASFLAGS) $$< -o $$@ 
endef
$(foreach src, $(ASRC), $(eval $(call ASSEMBLE_TEMPLATE, $(src)))) 

# Compile: create object files from C source files.
define COMPILE_C_TEMPLATE
$(OUTDIR)/$(basename $(1)).o : $(1)
	@echo $(MSG_COMPILING) $$< to $$@
	$$(call makedir, $$(OUTDIR)/$$(dir $(1)))
	@$(CC) -c $(THUMB) $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@ 
endef
$(foreach src, $(SRC), $(eval $(call COMPILE_C_TEMPLATE, $(src)))) 
$(eval $(call COMPILE_C_TEMPLATE, $(STUB_SRC)))

# Compile: create object files from C++ source files.
define COMPILE_CPP_TEMPLATE
$(OUTDIR)/$(basename $(1)).o : $(1)
	@echo $(MSG_COMPILINGCPP) $$< to $$@
	$$(call makedir, $$(OUTDIR)/$$(dir $(1)))
	@$(CC) -c $(THUMB) $$(CFLAGS) $$(CPPFLAGS) $$< -o $$@ 
endef
$(foreach src, $(CPPSRC), $(eval $(call COMPILE_CPP_TEMPLATE, $(src)))) 



# Specific targets.
spotifyserver: dirs gccversion spotifyserver.$(EXECUTABLE_EXT)

stub: SRC += $(STUB_SRC)
stub: dirs $(STUB_OBJ) spotifyserver_stub.$(EXECUTABLE_EXT)
spotifyserver_stub: stub

# Create output directories.
ifdef SHELL_IS_WIN32
define makedir
	@md $(subst /,\,$(1)) >NUL 2>&1 || echo "" >NUL
endef
else
define makedir
	@mkdir -p $(1) #2>/dev/null || echo "" >/dev/null
endef
endif

dirs: $(OUTDIR) $(OUTDIR)/dep

$(OUTDIR): 
	$(call makedir, $(OUTDIR))

$(OUTDIR)/dep: 
	$(call makedir, $(OUTDIR)/dep)


# Display compiler version information.
gccversion : 
	@$(CC) --version


# Create a symbol table from ELF output file.
$(OUTDIR)/%.sym: %.$(EXECUTABLE_EXT)
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@

# Link: create ELF output file from object files.
#.SECONDARY : $(TARGET).elf
.PRECIOUS : $(ALLOBJ)
%.$(EXECUTABLE_EXT):  $(ALLOBJ)
	@echo $(MSG_LINKING) $@
# use $(CC) for C-only projects or $(CPP) for C++-projects:
ifeq "$(strip $(CPPSRC)$(CPPARM))" ""
	@$(CC) $(CFLAGS) $(ALLOBJ) --output $@ -nostartfiles $(LDFLAGS)
else
	@$(CPP) $(CFLAGS) $(ALLOBJ) --output $@ $(LDFLAGS)
endif


# Target: clean project.
clean: 
	@echo $(MSG_CLEANING)
	$(REMOVE) $(OUTDIR)
	$(REMOVE) *.(EXECUTABLE_EXT)

# Include the dependency files.
-include $(wildcard $(OUTDIR)/dep/*)

# Listing of phony targets.
.PHONY : gccversion build clean dirs spotifyserver spotifyserver_stub stub

