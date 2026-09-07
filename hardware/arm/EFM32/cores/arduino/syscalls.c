/* Support files for GNU libc.  Files in the system namespace go here.
   Files in the C namespace (ie those that do not start with an
   underscore) go in .c.  */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "efm32/efm32.h"
#include <sys/times.h>
#include <sys/errno.h>
#include <reent.h>
#include <unistd.h>
#include <sys/wait.h>

/* The stack reservation now comes from the linker script (__STACK_SIZE /
 * __StackLimit), so the old hard-coded MAX_STACK_SIZE is gone. */

extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

#if (FREERTOS ==0)
  register char * stack_ptr asm("sp");
#endif


#pragma GCC diagnostic ignored "-Wunused-parameter"  

__attribute__((weak))
caddr_t _sbrk(int incr)
{
	extern char end asm("end");
	extern char __StackLimit asm("__StackLimit");
	static char *heap_end;
	char *prev_heap_end;
	char *limit;

	if (heap_end == 0)
		heap_end = &end;

	prev_heap_end = heap_end;

	/* The linker reserves __STACK_SIZE bytes at the top of RAM; __StackLimit
	 * is the bottom of that reservation. The old check compared against the
	 * *current* stack pointer, which left no headroom whatsoever: the heap was
	 * allowed to grow right up to where the stack happened to be at that
	 * moment, and the next deeper call chain or interrupt frame then wrote
	 * straight over freshly allocated memory. */
	limit = &__StackLimit;

#if (FREERTOS == 0)
	/* Belt and braces: if the stack has already grown past its reservation,
	 * do not hand out memory it is currently using. */
	if (stack_ptr < limit)
		limit = stack_ptr;
#endif

	if (incr < 0) {
		/* newlib releases the top chunk this way; never fall below the start
		 * of the heap. */
		if (heap_end + incr < &end) {
			errno = ENOMEM;
			return (caddr_t) -1;
		}
	} else if (heap_end + incr > limit) {
//		write(1, "Heap and stack collision\n", 25);
//		abort();
		errno = ENOMEM;
		return (caddr_t) -1;
	}

	heap_end += incr;

	return (caddr_t) prev_heap_end;
}

/*
 * _gettimeofday primitive (Stub function)
 * */
int _gettimeofday (struct timeval * tp, struct timezone * tzp)
{
  /* Return fixed data for the timezone.  */
  if (tzp)
    {
      tzp->tz_minuteswest = 0;
      tzp->tz_dsttime = 0;
    }

  return 0;
}
void initialise_monitor_handles()
{
}

int _getpid(void)
{
	return 1;
}

int _kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

void _exit (int status)
{
	_kill(status, -1);
	while (1) {}
}

/*
int _write(int file, char *ptr, int len)
{
	int DataIdx;

		for (DataIdx = 0; DataIdx < len; DataIdx++)
		{
		   __io_putchar( *ptr++ );
		}
	return len;
}
*/
int _close(int file)
{
	return -1;
}

int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	return 0;
}

int _read(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
	  *ptr++ = __io_getchar();
	}

   return len;
}

int _open(char *path, int flags, ...)
{
	/* Pretend like we always fail */
	return -1;
}

int _wait(int *status)
{
	return -1;
}

int _unlink(char *name)
{
	return -1;
}

int _times(struct tms *buf)
{
	return -1;
}

int _stat(char *file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _link(char *old, char *new)
{
	errno = EMLINK;
	return -1;
}

int _fork(void)
{
	errno = EAGAIN;
	return -1;
}

int _execve(char *name, char **argv, char **env)
{
	errno = ENOMEM;
	return -1;
}
