
# Compiler flags.

# Flags for C and C++
CFLAGS +=  -g$(DEBUG)
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
CPPFLAGS += 

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
LDFLAGS += -Wl,-Map=$(OUTDIR)/$(TARGET).map,--cref,--gc-sections
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


# Assemble: create object files from assembler source files.
define ASSEMBLE_TEMPLATE
$(OUTDIR)/$(basename $(1)).o : $(1)
	@echo $(MSG_ASSEMBLING) $$< to $$@
	$$(call makedir, $$(OUTDIR)/$$(dir $(1)))
	@$$(CC) -c $$(THUMB) $$(ASFLAGS) $$< -o $$@ 
endef
$(foreach src, $(ASRC), $(eval $(call ASSEMBLE_TEMPLATE, $(src)))) 

# Compile: create object files from C source files.
define COMPILE_C_TEMPLATE
$(OUTDIR)/$(basename $(1)).o : $(1)
	@echo $(MSG_COMPILING) $$< to $$@
	$$(call makedir, $$(OUTDIR)/$$(dir $(1)))
	@$$(CC) -c $$(THUMB) $$(CFLAGS) $$(CONLYFLAGS) $$< -o $$@ 
endef
$(foreach src, $(SRC), $(eval $(call COMPILE_C_TEMPLATE, $(src)))) 

# Compile: create object files from C++ source files.
define COMPILE_CPP_TEMPLATE
$(OUTDIR)/$(basename $(1)).o : $(1)
	@echo $(MSG_COMPILINGCPP) $$< to $$@
	$$(call makedir, $$(OUTDIR)/$$(dir $(1)))
	@$$(CC) -c $$(THUMB) $$(CFLAGS) $$(CPPFLAGS) $$< -o $$@ 
endef
$(foreach src, $(CPPSRC), $(eval $(call COMPILE_CPP_TEMPLATE, $(src)))) 

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
	@$(CC) $(THUMB) $(CFLAGS) $(ALLOBJ) --output $@ -nostartfiles $(LDFLAGS)
else
	@$(CPP) $(THUMB) $(CFLAGS) $(ALLOBJ) --output $@ $(LDFLAGS)
endif


# Target: clean project.
clean: 
	@echo $(MSG_CLEANING)
	$(REMOVE) $(OUTDIR)
	$(REMOVE) *.$(EXECUTABLE_EXT)

.PHONY : gccversion build clean dirs

