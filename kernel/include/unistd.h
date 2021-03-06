/*
 * include/unistd.h
 * 
 * Author: Fernando Rodriguez
 * Email: frodriguez.developer@outlook.com
 * 
 * Copyright 2014-2015 Fernando Rodriguez
 * All rights reserved.
 * 
 * This code is distributed for educational purposes
 * only. It may not be distributed without written 
 * permission from the author.
 *
 */

#ifndef __UNISTD_H__
#define __UNISTD_H__

#define MODE_TYPE_DIRECTORY		(0x1)
#define MODE_TYPE_LINK			(0x2)
#define MODE_TYPE_MOUNT			(0x4)
#define MODE_TYPE_CHARDEVICE		(0x8)

#if 0
#define S_ISDIR(mode)			(mode & MODE_TYPE_DIRECTORY)
#define S_ISCHR(mode)			(mode & MODE_TYPE_CHARDEVICE)
#endif

#define SEEK_SET 				(0x1)
#define SEEK_CUR				(0x2)
#define SEEK_END				(0x3)

#define STDERR_FILENO   2 	/* used by newlib */

/* #include <vfs.h> */
#include <_ansi.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdarg.h>

typedef unsigned long size_t;
typedef long ssize_t;


/*
 * Opens a file descriptor
 */
int open(const char *pathname, int flags, ...);

/*
 * Closes a file descriptor
 */
int close(int fd);

/*
 * Open a directory.
 */
DIR *opendir(char *path);

/*
 * Close a directory.
 */
int closedir(DIR* dir);

/*
 * Read a directory (re-entrant).
 */
int readdir_r(DIR *dir, struct dirent *entry, struct dirent **result);

/*
 * read the next entry in a directory
 */
/*int readdir(unsigned int fd, struct dirent *dirent, unsigned int count);*/
struct dirent *readdir(DIR* dir);

/*
 * Writes to a file descriptor
 */
ssize_t write(int fd, const void* buf, size_t count);

/*
 * Reads from a file descriptor
 */
ssize_t read(int fd, void* buf, size_t count);

/*
 * Get information about a file.
 */
int stat(const char *path, struct stat *buf);


/*
 * forks the current task
 */
int fork(void);

/*
 * Forks a new task that shares the entire
 * parent process userspace.
 */
int vfork(void);

/*
 * Send a signal to a process.
 */
int kill(pid_t pid, int signum);

/*
 * clone the current task
 */
int clone(void *stack);

/*
 * exit the current task
 */
void exit(int exit_code) __attribute__ ((noreturn));

/*
 * gets the PID of the running process
 */
pid_t getpid(void);

/*
 * gets the UID of the current task.
 */
uid_t getuid(void);

/*
 * gets the GID of the current task.
 */
uid_t getgid(void);

/*
 * Creates a pipe.
 */
int pipe(int pipefd[2]);

/*
 * executes a file with context and environment
 */
int execve(const char *filename, 
	char *const argv[], char *const envp[]);

/*
 * Change the working directory.
 */
int chdir(const char *path);

/*
 * Get the current working directory
 */
char *getcwd(char *buf, size_t size);

/*
 * Wait for a process to exit
 */
int waitpid(pid_t pid, int *status, int options);

/*
 * Send a signal to the current process.
 */
int raise(int sig);

/*
 * Sets and environment variable
 */
int setenv(const char *name, const char *value, int overwrite);

/*
 * Unsets an environment variable
 */
int unsetenv(const char *name);

/*
 * Set the UID
 */
int setuid(uid_t uid);

/*
 * Sets the GID.
 */
int setgid(gid_t gid);

/*
 * Get the current UID
 */
uid_t geteuid(void);

/*
 * Get the current GID
 */
gid_t getegid(void);

/*
 * Set the effective UID
 */
int seteuid(uid_t uid);

/*
 * Set the effective GID
 */
int setegid(gid_t gid);

/*
 * Send IOCTL request to device driver.
 */
int ioctl(int fd, unsigned int request, ...);

/*
 * Duplicate a file descriptor.
 */
int dup(int oldfd);

/*
 * Duplicate a file descriptor and assign newfd
 * to the it. If newfd is already open it is closed.
 */
int dup2(int oldfd, int newfd);

/*
 * Yield the CPU
 */
void yield(void);

/*
 * Get the thread id of the current task.
 */
pid_t gettid(void);

/*
 * Send a signal to the thread specified by
 * task_id.
 */
int killtask(pid_t tid, int signum);

/*
 * Flush filesystem buffers.
 */
int sync(void);

/*
 * Change the owner of a file
 */
int chown(const char *path, uid_t uid, gid_t gid);

/*
 * Change root directory.
 */
int chroot(const char *path);

/*
 * Execute command on a file.
 */
int fcntl(int fd, int cmd, ...);
int kfcntl(int fd, int cmd, va_list args);

/*
 * Flush the buffer for an open file descriptor.
 */
int fsync(int fd);

/*
 * Gets the hostname.
 */
int gethostname(char *name, size_t len);

/*
 * Set the hostname
 */
int sethostname(const char *name, size_t len);

/*
 * Wait for a child process to exit.
 */
pid_t wait(int *status);

/*
 * Read value of a symbolic link
 */
ssize_t readlink(const char *path, char *buf, size_t bufsize);

/*
 * Unlink a file.
 */
int unlink(const char *path);

/*
 * Change data segment size
 */
int brk(void *addr);

/*
 * Change data segment size
 */
void *sbrk(intptr_t inc);

/*
 * Reposition read/write file offset
 */
off_t lseek(int fd, off_t offset, int whence);

/*
 * Create new name for file.
 */
int link(const char *oldpath, const char *newpath);

/*
 * Returns 1 if the file descriptor is a tty,
 * zero otherwise.
 */
int isatty(int fd);

/*
 * wait for a signal
 */
void pause(void);

/*
 * Rewind file descriptor
 */
int rewindfd(int fd);

#endif

