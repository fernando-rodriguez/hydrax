#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <syscall.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <dirent.h>
#include "syscalls.h"

/* ssyscall3(SYSCALL_READDIR, int, readdir, unsigned int, fd, struct dirent*, dirent, unsigned int, count); */

struct dirent hydrax_dirent;

struct dirent *readdir(DIR* dirp)
{
	struct dirent *direntp;
	if (readdir_r(dirp, &hydrax_dirent, &direntp))
		return NULL;
	return direntp;
}

