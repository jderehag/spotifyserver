CCX = g++
CC = gcc
CFLAGS += -O0 -g3 -Wall -c

TARGET = TestRunner
EXECUTABLE_EXT = elf

VPATH +=	../src/ConfigHandling

INCLUDES +=	-I../src/ConfigHandling


OBJS :=		ConfigParser_TEST.o	 	\
		TestRunner.o			\
		ConfigParser.o


		
		  
.PHONY: clean
.SILENT:


ut: $(TARGET).$(EXECUTABLE_EXT) run_ut

$(TARGET).$(EXECUTABLE_EXT): $(OBJS)
	@echo ..Linking $@
	$(CCX) -o $@ $(OBJS)

%.o: %.c
	@echo Building $@
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES)

%.o: %.cpp
	@echo Building $@
	$(CCX) -o $@ $< $(CFLAGS) $(INCLUDES)
run_ut:
	./$(TARGET).$(EXECUTABLE_EXT)
clean:
	rm -Rf *.o
	rm -f $(TARGET).$(EXECUTABLE_EXT)
