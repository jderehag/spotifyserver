/*
 * syscalls.c
 *
 *  Created on: 03.12.2009
 *      Author: Martin Thomas, 3BSD license
 */

#include <reent.h>
#include <errno.h>
#include <stdlib.h> /* abort */
#include <sys/types.h>
#include <sys/stat.h>

#include "stm32f4xx.h"

//#include "term_io.h"


#undef errno
extern int errno;

int _kill(int pid, int sig)
{
	(void)pid;
	(void)sig; /* avoid warnings */
	errno = EINVAL;
	return -1;
}

void _exit(int status)
{
	(void)status;
//	xprintf("_exit called with parameter %d\n", status);
	while(1) {;}
}

int _getpid(void)
{
	return 1;
}


extern char __end__; /* Defined by the linker */
static char *heap_end;

char* get_heap_end(void)
{
	return (char*) heap_end;
}

char* get_stack_top(void)
{
	return (char*) __get_MSP();
	// return (char*) __get_PSP();
}

caddr_t _sbrk(int incr)
{
	char *prev_heap_end;
	if (heap_end == 0) {
		heap_end = &__end__;
	}
	prev_heap_end = heap_end;
#if 1
	if (heap_end + incr > get_stack_top()) {
	//	xprintf("Heap and stack collision\n");
		//abort();
	    errno = ENOMEM;
	    return (caddr_t) -1;
	}
#endif
	heap_end += incr;
	return (caddr_t) prev_heap_end;
}

int _open(const char *name, int flags, int mode)
{
	(void)name;
	(void)flags;
	(void)mode;
    return -1;
}

int _close(int file)
{
	(void)file; /* avoid warning */
	return -1;
}

int _fstat(int file, struct stat *st)
{
	(void)file; /* avoid warning */
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	(void)file; /* avoid warning */
	return 1;
}

int _lseek(int file, int ptr, int dir) {
	(void)file; /* avoid warning */
	(void)ptr;  /* avoid warning */
	(void)dir;  /* avoid warning */
	return 0;
}

int _read(int file, char *ptr, int len)
{
	(void)file; /* avoid warning */
	(void)ptr;  /* avoid warning */
	(void)len;  /* avoid warning */
	return 0;
}

int _write(int file, char *ptr, int len)
{
	int todo;
	(void)file; /* avoid warning */
	(void)ptr;

	for (todo = 0; todo < len; todo++) {
	//	xputc(*ptr++);
	}
	return len;
}
