/*
 * include/syscall.h
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

#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <stdint.h>

/* 
 * system calls
 */
#define SYSCALL_TEST            	(0)
#define SYSCALL_FORK            	(1)
#define SYSCALL_EXIT            	(2)
#define SYSCALL_GETPID          	(3)
#define SYSCALL_GETUID			(4)
#define SYSCALL_GETGID			(5)
#define SYSCALL_OPEN			(6)
#define SYSCALL_CLOSE			(7)
#define SYSCALL_WRITE			(8)
#define SYSCALL_READ			(9)
#define SYSCALL_KILL			(10)
#define SYSCALL_READDIR			(11)
#define SYSCALL_REBOOT			(12)
#define SYSCALL_STAT			(13)
#define SYSCALL_TIME			(14)
#define SYSCALL_CHDIR			(15)
#define SYSCALL_GETCWD			(16)
#define SYSCALL_EXECVE			(17)
#define SYSCALL_WAITPID			(18)
#define SYSCALL_SETENV			(19)
#define SYSCALL_UNSETENV			(20)
#define SYSCALL_SETUID			(21)
#define SYSCALL_SETGID			(22)
#define SYSCALL_GETEUID			(23)
#define SYSCALL_GETEGID			(24)
#define SYSCALL_SETEUID			(25)
#define SYSCALL_SETEGID			(26)
#define SYSCALL_PIPE			(27)
#define SYSCALL_VFORK			(28)
#define SYSCALL_IOCTL			(29)
#define SYSCALL_DUP				(30)
#define SYSCALL_YIELD			(31)
#define SYSCALL_CLONE			(32)
#define SYSCALL_GETTID			(33)
#define SYSCALL_DUP2			(34)
#define SYSCALL_SYNC			(35)
#define SYSCALL_CHOWN			(36)
#define SYSCALL_CHROOT			(37)
#define SYSCALL_FCNTL			(38)
#define SYSCALL_FSYNC			(39)
#define SYSCALL_SETHOSTNAME		(40)
#define SYSCALL_GETHOSTNAME		(41)
#define SYSCALL_WAIT			(42)
#define SYSCALL_READLINK			(43)
#define SYSCALL_UNLINK			(44)
#define SYSCALL_BRK				(45)
#define SYSCALL_LSEEK			(46)
#define SYSCALL_LINK			(47)
#define SYSCALL_ISATTY			(48)
#define SYSCALL_SIGSET			(49)
#define SYSCALL_SIGHOLD			(50)
#define SYSCALL_SIGRELSE			(51)
#define SYSCALL_SIGIGNORE		(52)
#define SYSCALL_SIGPROCMASK		(53)
#define SYSCALL_SIGNAL			(54)
#define SYSCALL_SIGACTION		(55)
#define SYSCALL_SIGPAUSE			(56)
#define SYSCALL_SIGQUEUE			(57)
#define SYSCALL_SIGWAIT			(58)
#define SYSCALL_PTHREAD_KILL		(59)
#define SYSCALL_PTHREAD_CREATE	(60)
#define SYSCALL_PTHREAD_EXIT		(61)
#define SYSCALL_UNAME			(62)
#define SYSCALL_OPENDIR			(63)
#define SYSCALL_CLOSEDIR			(64)
#define SYSCALL_READDIR_R		(65)

#define SYSCALL_COUNT			(66)

/*
 * initialize system call interface
 */
void syscall_init(void);

int syscall(uint32_t num, ...);


#endif

