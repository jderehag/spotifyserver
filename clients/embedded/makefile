#
# This makefile is based on:
#
# WinARM template makefile 
# by Martin Thomas, Kaiserslautern, Germany 
# <eversmith(at)heizung-thomas(dot)de>
#


# Toolchain prefix (i.e arm-elf- -> arm-elf-gcc.exe)
TCHAIN_PREFIX = arm-none-eabi-
#REMOVE_CMD=rm
REMOVE_CMD=cs-rm

FLASH_TOOL = OPENOCD

# YES enables -mthumb option to flags for source-files listed 
# in SRC and CPPSRC and -mthumb-interwork option for all source
USE_THUMB_MODE = YES
#USE_THUMB_MODE = NO

# MCU name, submodel and board
# - MCU used for compiler-option (-mcpu)
# - SUBMDL used for linker-script name (-T) and passed as define
# - BOARD just passed as define (optional)
MCU      = cortex-m4
CHIP     = STM32F4XX
F_XTAL   = 8000000

# *** This example only supports "flash" ***
# RUN_MODE is passed as define and used for the linker-script filename,
# the user has to implement the necessary operations for 
# the used mode(s) (i.e. no copy of .data, remapping)
# Create FLASH-Image
RUN_MODE=flash

# Exception-vectors placement option is just passed as define,
# the user has to implement the necessary operations (i.e. remapping)
# Exception vectors in FLASH:
##VECTOR_TABLE_LOCATION=VECT_TAB_FLASH
# Exception vectors in RAM:
VECTOR_TABLE_LOCATION=VECT_TAB_RAM

# Directory for output files (lst, obj, dep, elf, sym, map, hex, bin etc.)
OUTDIR = .out

# Target file name (without extension).
TARGET = spotifyclient_rom

# root directories to look for source files in
VPATH += ../../src/ ../../common/ ./source/

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files

SRC  =	syscalls.c powerHandler.c

#FreeRTOS
SRC +=	$(addprefix freeRTOS/, list.c queue.c tasks.c timers.c heap_3.c) \
		$(addprefix freeRTOS/port/, port.c) \
		hooks.c

#ST drivers and libraries
SRC +=	$(addprefix STM32F4XX/, stm32f4_discovery.c system_stm32f4xx.c stm32f4xx_it.c stm32f4x7_eth_bsp.c netconf.c) \
		$(addprefix STM32F4XX/STM32F4xx_StdPeriph_Driver/src/, misc.c stm32f4xx_dma.c stm32f4xx_exti.c stm32f4xx_gpio.c stm32f4xx_syscfg.c stm32f4xx_rcc.c stm32f4xx_pwr.c stm32f4xx_rtc.c) \
		STM32F4XX/STM32F4x7_ETH_Driver/src/stm32f4x7_eth.c

#lwIP		
SRC +=	$(addprefix lwip_v1.4.0/src/core/, init.c tcp_out.c memp.c netif.c pbuf.c raw.c stats.c sys.c tcp.c tcp_in.c udp.c dhcp.c def.c timers.c ipv4/ip.c ipv4/ip_addr.c ipv4/icmp.c ipv4/ip_frag.c ipv4/inet.c ipv4/inet_chksum.c ipv4/autoip.c) \
		$(addprefix lwip_v1.4.0/src/api/, tcpip.c api_msg.c err.c api_lib.c netbuf.c sockets.c ) \
		$(addprefix lwip_v1.4.0/src/netif/, etharp.c ) \
		$(addprefix lwip_v1.4.0/port/STM32F4x7/FreeRTOS/, sys_arch.c ethernetif.c)


# List C++ source files here.
# use file-extension .cpp for C++-files (not .C)
CPPSRC = main.cpp buttonHandler.cpp UIEmbedded.cpp heapWrap.cpp \
		$(addprefix MediaContainers/, Album.cpp Artist.cpp Folder.cpp Playlist.cpp Track.cpp) \
		$(addprefix MessageFactory/, Message.cpp MessageDecoder.cpp MessageEncoder.cpp TlvDefinitions.cpp Tlvs.cpp SocketReader.cpp SocketWriter.cpp) \
		$(addprefix TestApp/, RemoteMediaInterface.cpp ) \
		$(addprefix SocketHandling/, Messenger.cpp SocketClient.cpp ) \
		MediaInterface/MediaInterface.cpp \
		Logger/Logger.cpp \
		ConfigHandling/Configs/LoggerConfig.cpp \
		Platform/Socket/LwIP/LwIPSocket.cpp \
		$(addprefix Platform/Threads/FreeRTOS/, FreeRTOSMessagebox.cpp FreeRTOSMutex.cpp FreeRTOSCondition.cpp FreeRTOSRunnable.cpp)

AUDIOSRC =	STM32F4XX/stm32f4_discovery_audio_codec.c \
			$(addprefix STM32F4XX/STM32F4xx_StdPeriph_Driver/src/, stm32f4xx_i2c.c stm32f4xx_dac.c stm32f4xx_spi.c)
		
AUDIOSRCCPP =		ConfigHandling/Configs/AudioEndpointConfig.cpp \
					ConfigHandling/Configs/NetworkConfig.cpp \
					$(addprefix SocketHandling/, SocketServer.cpp SocketPeer.cpp ) \
					Platform/AudioEndpoints/AudioFifo.cpp \
					Platform/AudioEndpoints/Endpoints/AudioEndpoint-CS43L22.cpp \
					TestApp/AudioEndpointRemoteSocketServer.cpp \
					TestApp/AudioEndpointRemotePeer.cpp


SRC += $(AUDIOSRC)
CPPSRC += $(AUDIOSRCCPP)


# List Assembler source files here.
# Make them always end in a capital .S. Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC = STM32F4XX/startup_stm32f4xx.s

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS  += ./source/freertos/include \
		  ./source/freertos/port \
		  ./source \
		  ./source/STM32F4XX \
		  ./source/STM32F4XX/CMSIS \
		  ./source/STM32F4XX/STM32F4xx_StdPeriph_Driver/inc \
		  ./source/STM32F4XX/STM32F4x7_ETH_Driver/inc \
		  ./../../src \
		  ./../../common \
		  ./../../common/Logger \
		  ./source/lwip_v1.4.0/src/include \
		  ./source/lwip_v1.4.0/src/include/ipv4 \
		  ./source/lwip_v1.4.0/port/STM32F4x7/FreeRTOS \
		  ./source/lwip_v1.4.0/port/STM32F4x7

# Extra libraries
#    Each library-name must be seperated by a space.
#    i.e. to link with libxyz.a, libabc.a and libefsl.a: 
#    EXTRA_LIBS = xyz abc efsl
# for newlib-lpc (file: libnewlibc-lpc.a):
#    EXTRA_LIBS = newlib-lpc
EXTRA_LIBS =

# Path to Linker-Scripts
LINKERSCRIPTPATH = ./linker/
LINKERSCRIPTINC  =

# List any extra directories to look for library files here.
# Also add directories where the linker should search for
# includes from linker-script to the list
#     Each directory must be seperated by a space.
EXTRA_LIBDIRS =

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
#OPT = s
#OPT = 2
#OPT = 3
OPT = 0

# Output format. (can be ihex or binary or both)
#  binary to create a load-image in raw-binary format i.e. for SAM-BA, 
#  ihex to create a load-image in Intel hex format i.e. for lpc21isp
#LOADFORMAT = ihex
#LOADFORMAT = binary
LOADFORMAT = both

# Debugging format.
#DEBUG = stabs
#DEBUG = dwarf-2

# Place project-specific -D (define) and/or 
# -U options for C here.
CDEFS += -DHSE_VALUE=$(F_XTAL)UL
CDEFS += -DUSE_STDPERIPH_DRIVER
CDEFS += -DCORE_M4
# enable parameter-checking in STM's library
CDEFS += -DUSE_FULL_ASSERT

# Place project-specific -D and/or -U options for 
# Assembler with preprocessor here.
#ADEFS = -DUSE_IRQ_ASM_WRAPPER
ADEFS = -D__ASSEMBLY__

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

#-----

ifdef VECTOR_TABLE_LOCATION
CDEFS += -D$(VECTOR_TABLE_LOCATION)
ADEFS += -D$(VECTOR_TABLE_LOCATION)
endif

CDEFS += -D$(CHIP)
ADEFS += -D$(CHIP)


# Compiler flags.

ifeq ($(USE_THUMB_MODE),YES)
THUMB    = -mthumb
THUMB_IW = -mthumb-interwork
else 
THUMB    = 
THUMB_IW = 
endif

#-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS =  -g$(DEBUG)
CFLAGS += -O$(OPT)
CFLAGS += -mcpu=$(MCU) $(THUMB_IW) 
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
CFLAGS += $(CDEFS)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.
# when using ".ramfunc"s without longcall:
CFLAGS += -mlong-calls
# -mapcs-frame is important if gcc's interrupt attributes are used
# (at least from my eabi tests), not needed if assembler-wrapper is used 
#CFLAGS += -mapcs-frame 
#CFLAGS += -fomit-frame-pointer
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fpromote-loop-indices
CFLAGS += -Wall -Wextra
CFLAGS += -Wcast-align -Wpointer-arith
CFLAGS += -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align
CFLAGS += -Wno-cast-qual
#CFLAGS += -pedantic
CFLAGS += -Wa,-adhlns=$(addprefix $(OUTDIR)/, $(notdir $(addsuffix .lst, $(basename $<))))
# Compiler flags to generate dependency files:
CFLAGS += -MD -MP -MF $(OUTDIR)/dep/$(@F).d

# flags only for C
CONLYFLAGS += -Wimplicit -Wnested-externs 
#CONLYFLAGS += -Wno-missing-prototypes 
#CONLYFLAGS += -Wno-strict-prototypes
#CONLYFLAGS += -Wno-missing-declarations
CONLYFLAGS += $(CSTANDARD)

# flags only for C++ (arm-*-g++)
CPPFLAGS = -fno-rtti -fno-exceptions
CPPFLAGS = 

# Assembler flags.
#  -Wa,...:    tell GCC to pass this to the assembler.
#  -ahlns:     create listing
#  -g$(DEBUG): have the assembler create line number information
ASFLAGS  = -mcpu=$(MCU) $(THUMB_IW) -I. -x assembler-with-cpp
ASFLAGS += $(ADEFS)
ASFLAGS += -Wa,-adhlns=$(addprefix $(OUTDIR)/, $(notdir $(addsuffix .lst, $(basename $<))))
ASFLAGS += -Wa,-g$(DEBUG)
ASFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))

MATH_LIB = -lm

# Link with the GNU C++ stdlib.
CPLUSPLUS_LIB = -lstdc++
#CPLUSPLUS_LIB += -lsupc++

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -Wl,-Map=$(OUTDIR)/$(TARGET).map,--cref,--gc-sections
#not in CPP
#LDFLAGS += -nostartfiles
LDFLAGS += -lc
LDFLAGS += $(MATH_LIB)
LDFLAGS += -lgcc
LDFLAGS += $(CPLUSPLUS_LIB)
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS)) 

# Set linker-script name depending on selected run-mode and submodel name
ifeq ($(RUN_MODE),RAM_RUN)
##LDFLAGS +=-T$(LINKERSCRIPTPATH)/$(CHIP)_ram.ld
##LDFLAGS +=-T$(LINKERSCRIPTPATH)/sram.lds
else 
LDFLAGS +=-T$(LINKERSCRIPTPATH)/stm32f4xxxg_flash.ld
##LDFLAGS +=-T$(LINKERSCRIPTPATH)/flash.lds
endif


# ---------------------------------------------------------------------------
# Options for OpenOCD flash-programming
# see openocd.pdf/openocd.texi for further information
#
OOCD_LOADFILE+=$(OUTDIR)/$(TARGET).hex
# if OpenOCD is in the $PATH just set OPENOCDEXE=openocd
OOCD_EXE=openocd-0.6.1
# debug level
OOCD_CL=-d0
#OOCD_CL=-d3
# interface and board/target settings (using the OOCD target-library here)
OOCD_CL+=-f board\stm32f4discovery.cfg  
# initialize
OOCD_CL+=-c init
# enable "fast mode" - can be disabled for tests
#OOCD_CL+=-c "fast enable"
# commands to prepare flash-write
OOCD_CL+= -c "reset halt" -c "sleep 100" -c "wait_halt 2"
# increase JTAG frequency a little bit - can be disabled for tests
OOCD_CL+= -c "jtag_khz 1200"
# flash-write and -verify
OOCD_CL+=-c "flash write_image erase $(OOCD_LOADFILE)" -c "sleep 100" -c "verify_image $(OOCD_LOADFILE)"
# reset target
OOCD_CL+=-c "reset run"
# terminate OOCD after programming
OOCD_CL+=-c shutdown
# ---------------------------------------------------------------------------



# Define programs and commands.
CC      = $(TCHAIN_PREFIX)gcc
CPP     = $(TCHAIN_PREFIX)g++
AR      = $(TCHAIN_PREFIX)ar
OBJCOPY = $(TCHAIN_PREFIX)objcopy
OBJDUMP = $(TCHAIN_PREFIX)objdump
SIZE    = $(TCHAIN_PREFIX)size
NM      = $(TCHAIN_PREFIX)nm
REMOVE  = $(REMOVE_CMD) -rf
SHELL   = sh
###COPY    = cp
ifneq ($(or $(COMSPEC), $(ComSpec)),)
$(info COMSPEC detected $(COMSPEC) $(ComSpec))
ifeq ($(findstring cygdrive,$(shell env)),)
SHELL:=$(or $(COMSPEC),$(ComSpec))
SHELL_IS_WIN32=1
else
$(info cygwin detected)
endif
endif
$(info SHELL is $(SHELL))

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = --------  begin, mode: $(RUN_MODE)  --------
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before: 
MSG_SIZE_AFTER = Size after build:
MSG_LOAD_FILE = Creating load file:
MSG_EXTENDED_LISTING = Creating Extended Listing/Disassembly:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = ---- Linking :
MSG_COMPILING = ---- Compiling C :
MSG_COMPILING_ARM = ---- Compiling C ARM-only:
MSG_COMPILINGCPP = ---- Compiling C++ :
MSG_COMPILINGCPP_ARM = ---- Compiling C++ ARM-only:
MSG_ASSEMBLING = ---- Assembling:
MSG_ASSEMBLING_ARM = ---- Assembling ARM-only:
MSG_CLEANING = Cleaning project:
MSG_FORMATERROR = Can not handle output-format
MSG_LPC21_RESETREMINDER = You may have to bring the target in bootloader-mode now.
MSG_ASMFROMC = "Creating asm-File from C-Source:"
MSG_ASMFROMC_ARM = "Creating asm-File from C-Source (ARM-only):"

# List of all source files.
ALLSRC     = $(ASRC) $(SRC) $(CPPSRC)
# List of all source files with directory but without file-extension.
ALLSRCBASE = $(basename $(ALLSRC))

# Define all object files.
ALLOBJFILES = $(addsuffix .o, $(ALLSRCBASE))
ALLOBJ     = $(addprefix $(OUTDIR)/, $(ALLOBJFILES))

# Define all listing files (used for make clean).
LSTFILES   = $(addprefix $(OUTDIR)/, $(addsuffix .lst, $(ALLSRCBASE)))
# Define all depedency-files (used for make clean).
DEPFILES   = $(addprefix $(OUTDIR)/dep/, $(addsuffix .o.d, $(ALLSRCBASE)))

# Default target.
#all: begin gccversion sizebefore build sizeafter finished end
all: begin createdirs gccversion build sizeafter finished end

elf: $(OUTDIR)/$(TARGET).elf
lss: $(OUTDIR)/$(TARGET).lss 
sym: $(OUTDIR)/$(TARGET).sym
hex: $(OUTDIR)/$(TARGET).hex
bin: $(OUTDIR)/$(TARGET).bin


ifeq ($(LOADFORMAT),ihex)
build: elf hex lss sym
else 
ifeq ($(LOADFORMAT),binary)
build: elf bin lss sym
else 
ifeq ($(LOADFORMAT),both)
build: elf hex bin lss sym
else 
$(error "$(MSG_FORMATERROR) $(FORMAT)")
endif
endif
endif

# Create output directories.
ifdef SHELL_IS_WIN32
createdirs:
	-@md $(OUTDIR) >NUL 2>&1 || echo "" >NUL
	-@md $(OUTDIR)\dep >NUL 2>&1 || echo "" >NUL
	-@md $(foreach objdir, $(subst /,\,$(filter-out ./,$(dir $(ALLOBJFILES)))), $(OUTDIR)\$(objdir)) >NUL 2>&1 || echo "" >NUL
else
createdirs:
	-@mkdir $(OUTDIR) 2>/dev/null || echo "" >/dev/null
	-@mkdir $(OUTDIR)/dep 2>/dev/null || echo "" >/dev/null
	-@mkdir -p $(foreach objdir, $(dir $(ALLOBJFILES)), $(OUTDIR)/$(objdir)) 2>/dev/null || echo "" >/dev/null
endif

# Eye candy.
begin:
	@echo $(MSG_BEGIN)

finished:
##	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)

# Display sizes of sections.
ELFSIZE = $(SIZE) -A  $(OUTDIR)/$(TARGET).elf
##ELFSIZE = $(SIZE) --format=Berkeley --common $(OUTDIR)/$(TARGET).elf
sizebefore:
#	@if [ -f  $(OUTDIR)/$(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); echo; fi

sizeafter:
#	@if [ -f  $(OUTDIR)/$(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi
	@echo $(MSG_SIZE_AFTER)
	$(ELFSIZE)
	
# Display compiler version information.
gccversion : 
	@$(CC) --version

# Program the device with Dominic Rath's OPENOCD in "batch-mode", needs cfg and "reset-script".
program: $(OUTDIR)/$(TARGET).elf
	@echo "Programming with OPENOCD"
ifdef SHELL_IS_WIN32 
	$(subst /,\,$(OOCD_EXE)) $(OOCD_CL)
else
	$(OOCD_EXE) $(OOCD_CL)
endif

# Create final output file (.hex) from ELF output file.
%.hex: %.elf
	@echo $(MSG_LOAD_FILE) $@
	$(OBJCOPY) -O ihex $< $@
	
# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	@echo $(MSG_LOAD_FILE) $@
	$(OBJCOPY) -O binary $< $@

# Create extended listing file/disassambly from ELF output file.
# using objdump testing: option -C
%.lss: %.elf
	@echo skipping $(MSG_EXTENDED_LISTING) $@
#	$(OBJDUMP) -h -S -C -r $< > $@
#	$(OBJDUMP) -x -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(ALLOBJ)
%.elf:  $(ALLOBJ)
	@echo $(MSG_LINKING) $@
# use $(CC) for C-only projects or $(CPP) for C++-projects:
ifeq "$(strip $(CPPSRC)$(CPPARM))" ""
	$(CC) $(THUMB) $(CFLAGS) $(ALLOBJ) --output $@ -nostartfiles $(LDFLAGS)
else
	$(CPP) $(THUMB) $(CFLAGS) $(ALLOBJ) --output $@ $(LDFLAGS)
endif


# Assemble: create object files from assembler source files.
define ASSEMBLE_TEMPLATE
$(OUTDIR)/$(basename $(1)).o : $(1)
	@echo $(MSG_ASSEMBLING) $$< to $$@
	$(CC) -c $(THUMB) $$(ASFLAGS) $$< -o $$@ 
endef
$(foreach src, $(ASRC), $(eval $(call ASSEMBLE_TEMPLATE, $(src)))) 

# Compile: create object files from C source files.
define COMPILE_C_TEMPLATE
$(OUTDIR)/$(basename $(1)).o : $(1)
	@echo $(MSG_COMPILING) $$< to $$@
	$(CC) -c $(THUMB) $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@ 
endef
$(foreach src, $(SRC), $(eval $(call COMPILE_C_TEMPLATE, $(src)))) 

# Compile: create object files from C++ source files.
define COMPILE_CPP_TEMPLATE
$(OUTDIR)/$(basename $(1)).o : $(1)
	@echo $(MSG_COMPILINGCPP) $$< to $$@
	$(CC) -c $(THUMB) $$(CFLAGS) $$(CPPFLAGS) $$< -o $$@ 
endef
$(foreach src, $(CPPSRC), $(eval $(call COMPILE_CPP_TEMPLATE, $(src)))) 


# Compile: create assembler files from C source files. ARM/Thumb
$(SRC:.c=.s) : %.s : %.c
	@echo $(MSG_ASMFROMC) $< to $@
	$(CC) $(THUMB) -S $(CFLAGS) $(CONLYFLAGS) $< -o $@

# Target: clean project.
clean: begin clean_list finished end

clean_list :
	@echo $(MSG_CLEANING)
	$(REMOVE) $(OUTDIR)

# Include the dependency files.
-include $(wildcard $(OUTDIR)/dep/*)

# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion \
build elf hex bin lss sym clean clean_list program createdirs

