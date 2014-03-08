
# Toolchain prefix (i.e arm-elf- -> arm-elf-gcc.exe)
TCHAIN_PREFIX = arm-none-eabi-

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
CHIP     = STM32F407xx
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
VECTOR_TABLE_LOCATION=VECT_TAB_FLASH
# Exception vectors in RAM:
#VECTOR_TABLE_LOCATION=VECT_TAB_SRAM

# Directory for output files (lst, obj, dep, elf, sym, map, hex, bin etc.)
OUTDIR = .embedded.output

# Target file name (without extension).
TARGET = spotifyclient_rom

EXECUTABLE_EXT = elf

# root directories to look for source files in
VPATH += ../../src/ ../../common/ ./source/ ../cpp/src

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files

FREERTOS = FreeRTOS-8.0.0

SRC  =	syscalls.c powerHandler.c

#FreeRTOS
SRC +=	$(addprefix $(FREERTOS)/, list.c queue.c tasks.c timers.c event_groups.c ) \
		$(addprefix $(FREERTOS)/portable/GCC/ARM_CM4F/, port.c) \
		$(FREERTOS)/portable/MemMang/heap_3.c \
		hooks.c

#ST drivers and libraries
SRC +=	BSP/STM32F4-Discovery/stm32f4_discovery.c \
		$(addprefix STM32F4XX/, system_stm32f4xx.c stm32f4xx_it.c netconf.c) \
		$(addprefix STM32F4XX/STM32F4xx_HAL_Driver/src/, stm32f4xx_hal.c stm32f4xx_hal_dma.c stm32f4xx_hal_gpio.c stm32f4xx_hal_rcc.c stm32f4xx_hal_pwr.c stm32f4xx_hal_rtc.c) \
		$(addprefix STM32F4XX/STM32F4xx_HAL_Driver/src/, stm32f4xx_hal_cortex.c stm32f4xx_hal_eth.c  ) \
		$(addprefix BSP/STM32F4-DIS-BB/, stm32f4_discovery_eth.c )

#LCD display output
WITH_LCD?=1
ifeq ($(WITH_LCD),1)
SRC +=	$(addprefix BSP/STM32F4-DIS-LCD/, stm32f4_discovery_lcd.c fonts.c ) \
		$(addprefix STM32F4XX/STM32F4xx_HAL_Driver/src/, stm32f4xx_ll_fsmc.c )
CDEFS += -DWITH_LCD
endif

#lwIP		
SRC +=	$(addprefix lwip-1.4.1/src/core/, init.c tcp_out.c memp.c netif.c pbuf.c raw.c stats.c sys.c tcp.c tcp_in.c udp.c dhcp.c def.c timers.c dns.c ipv4/ip.c ipv4/ip_addr.c ipv4/icmp.c ipv4/ip_frag.c ipv4/inet.c ipv4/inet_chksum.c ipv4/autoip.c) \
		$(addprefix lwip-1.4.1/src/api/, tcpip.c api_msg.c err.c api_lib.c netbuf.c sockets.c netdb.c ) \
		$(addprefix lwip-1.4.1/src/netif/, etharp.c ) \
		$(addprefix lwip-1.4.1/port/STM32F4x7/FreeRTOS/, sys_arch.c ethernetif.c)

LWIP_MEM = $(addprefix lwip-1.4.1/src/core/, mem.c)
SRC += $(LWIP_MEM)

# List C++ source files here.
# use file-extension .cpp for C++-files (not .C)
CPPSRC = main.cpp buttonHandler.cpp UIEmbedded.cpp heapWrap.cpp LoggerEmbedded.cpp LcdLog.cpp clock.cpp \
		$(addprefix MediaContainers/, Album.cpp Artist.cpp Folder.cpp Playlist.cpp Track.cpp) \
		$(addprefix MessageFactory/, Message.cpp MessageDecoder.cpp MessageEncoder.cpp TlvDefinitions.cpp Tlvs.cpp SocketReader.cpp SocketWriter.cpp) \
		RemoteMediaInterface.cpp \
		$(addprefix SocketHandling/, Messenger.cpp SocketClient.cpp ) \
		MediaInterface/MediaInterface.cpp \
		Logger/Logger.cpp \
		Platform/Socket/LwIP/LwIPSocket.cpp \
		Platform/Socket/LwIP/Select.cpp \
		Platform/Utils/FreeRTOS/FreeRTOSUtils.cpp \
		$(addprefix Platform/Threads/FreeRTOS/, FreeRTOSMessagebox.cpp FreeRTOSMutex.cpp FreeRTOSCondition.cpp FreeRTOSRunnable.cpp)

#AUDIOSRC =	STM32F4XX/stm32f4_discovery_audio_codec.c
AUDIOSRC =	STM32F4XX/audio_driver.c
#AUDIOSRC +=	STM32F4XX/audio_sample.c

AUDIOSRC +=	$(addprefix STM32F4XX/STM32F4xx_HAL_Driver/src/, stm32f4xx_hal_i2c.c stm32f4xx_hal_dac.c stm32f4xx_hal_spi.c)
		
AUDIOSRCCPP =		ConfigHandling/Configs/AudioEndpointConfig.cpp \
					Platform/AudioEndpoints/AudioFifo.cpp \
					Platform/AudioEndpoints/AudioEndpointLocalShared.cpp \
					Platform/AudioEndpoints/Endpoints/AudioEndpoint-CS43L22.cpp \
					AudioEndpointRemoteSocketServer.cpp \
					AudioEndpointManager/RemoteAudioEndpointManager.cpp

SRC += $(AUDIOSRC)
CPPSRC += $(AUDIOSRCCPP)


WITH_TIME?=1
ifeq ($(WITH_TIME),1)
CPPSRC += NtpClient.cpp 
CDEFS += -DWITH_TIME
endif


#List assembler files
ASRC = STM32F4XX/startup_stm32f4xx.s

# List any extra directories to look for include files here.
#    Each directory must be seperated by a space.
EXTRAINCDIRS  += ./source/$(FREERTOS)/include \
		  ./source/$(FREERTOS)/portable/GCC/ARM_CM4F \
		  ./source \
		  ./source/STM32F4XX \
		  ./source/STM32F4XX/CMSIS \
		  ./source/STM32F4XX/CMSIS/Device \
		  ./source/STM32F4XX/STM32F4xx_HAL_Driver/inc \
		  ./source/BSP/STM32F4-Discovery/ \
		  ./source/BSP/STM32F4-DIS-BB/ \
		  ./source/BSP/STM32F4-DIS-LCD/ \
		  ./../cpp/src \
		  ./../../src \
		  ./../../common \
		  ./../../common/Logger \
		  ./source/lwip-1.4.1/src/include \
		  ./source/lwip-1.4.1/src/include/ipv4 \
		  ./source/lwip-1.4.1/port/STM32F4x7/FreeRTOS \
		  ./source/lwip-1.4.1/port/STM32F4x7

EXTRAINCDIRS += ./source/STM32F4XX/STM32F4DIS_LCD_Driver/inc

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
DEBUG = dwarf-2

# Place project-specific -D (define) and/or 
# -U options for C here.
CDEFS += -DHSE_VALUE=$(F_XTAL)UL
CDEFS += -DUSE_HAL_DRIVER
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
CFLAGS += -mcpu=$(MCU) $(THUMB_IW) 
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
CFLAGS += -mlong-calls
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fpromote-loop-indices

# flags only for C
CONLYFLAGS += -Wimplicit -Wnested-externs 
#CONLYFLAGS += -Wno-missing-prototypes 
#CONLYFLAGS += -Wno-strict-prototypes
#CONLYFLAGS += -Wno-missing-declarations

# flags only for C++ (arm-*-g++)
CPPFLAGS = -fno-rtti -fno-exceptions
CPPFLAGS = 

# Assembler flags.
#  -Wa,...:    tell GCC to pass this to the assembler.
#  -ahlns:     create listing
#  -g$(DEBUG): have the assembler create line number information
ASFLAGS += -mcpu=$(MCU) $(THUMB_IW) -I. -x assembler-with-cpp

MATH_LIB = -lm

# Link with the GNU C++ stdlib.
CPLUSPLUS_LIB = -lstdc++
#CPLUSPLUS_LIB += -lsupc++

# Linker flags.
#not in CPP
#LDFLAGS += -nostartfiles
LDFLAGS += -lc
LDFLAGS += $(MATH_LIB)
LDFLAGS += -lgcc
LDFLAGS += $(CPLUSPLUS_LIB)

# Set linker-script name depending on selected run-mode and submodel name
ifeq ($(RUN_MODE),RAM_RUN)
#todo
else 
LDFLAGS +=-T$(LINKERSCRIPTPATH)/stm32f4xxxg_flash.ld
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



# Default target.
#all: begin gccversion sizebefore build sizeafter finished end
all: gccversion dirs build sizeafter

include ../../build/build.mk

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


# Display sizes of sections.
ELFSIZE = $(SIZE) -A  $(OUTDIR)/$(TARGET).elf
##ELFSIZE = $(SIZE) --format=Berkeley --common $(OUTDIR)/$(TARGET).elf
sizebefore:
#	@if [ -f  $(OUTDIR)/$(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); echo; fi

sizeafter: elf
#	@if [ -f  $(OUTDIR)/$(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi
	@echo $(MSG_SIZE_AFTER)
	$(ELFSIZE)
	
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


# Compile: create assembler files from C source files. ARM/Thumb
$(SRC:.c=.s) : %.s : %.c
	@echo $(MSG_ASMFROMC) $< to $@
	$(CC) $(THUMB) -S $(CFLAGS) $(CONLYFLAGS) $< -o $@


# Include the dependency files.
-include $(wildcard $(OUTDIR)/dep/*)

# Listing of phony targets.
.PHONY : all sizebefore sizeafter \
build elf hex bin lss sym program

