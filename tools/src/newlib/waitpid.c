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

ssyscall3(SYSCALL_WAITPID, int, waitpid, pid_t, pid, int*, status, int, options);

