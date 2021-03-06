/*
 * io.c
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

#include <arch/arch.h>
#include <io.h>
#include <scheduler.h>
#include <vfs.h>
#include <string.h>
#include <kheap.h>
#include <unistd.h>
#include <semaphore.h>
#include <assert.h>
#include <errno.h>
#include <printk.h>
#include "config.inc"
#include <stdarg.h>

static inline bool check_file_permissions(vfs_node_t* node, uint32_t mask)
{
	assert(node != NULL);
	return true;
	#if 0
	return (node->uid == get_current_task()->uid || 
			node->gid == get_current_task()->gid ||
			node->mask & mask) ? true : false;
	#endif
}

/*
 * Gets a file descriptor
 */
file_t *get_file_descriptor(unsigned int fd)
{
	file_t *f;
	desc_info_t *desc_table;
	desc_table = (desc_info_t*) current_task->descriptors_info;
	if (desc_table == NULL || desc_table->file_descriptors == NULL)
		return NULL;
	f = (file_t*) desc_table->file_descriptors;
	while (f != NULL && f->fd != fd)
		f = f->next; 
	return f;
}

/*
 * Creates a file descriptor in the current task
 * descriptors table
 */
static inline int create_file_descriptor(vfs_node_t *node, int fd, unsigned int mode)
{
	desc_info_t *desc_table;
	file_t *f;
	
	f = (file_t*) malloc(sizeof(file_t));
	if (f == NULL)
	{
		current_task->errno = ENOMEM;
		return -1;
	}
	
	f->next = NULL;
	f->offset = 0;
	f->mode = mode;
	f->node = node;
	f->next = NULL;

	/*
	 * Lock the descriptor table and make sure
	 * it's initialized
	 */
	mutex_wait(&current_task->descriptors_info->lock);
	desc_table = (desc_info_t*) current_task->descriptors_info;
	assert(desc_table != NULL);

	#if 1
	if (likely(fd == -1))
		f->fd = desc_table->fd_next++;
	else
		f->fd = fd;
	#else
	if (unlikely(fd >= 0))
		f->fd = fd;
	else
		f->fd = desc_table->fd_next++;
	#endif

	if (unlikely(desc_table->file_descriptors == NULL))
	{
		desc_table->file_descriptors = f;
	}
	else
	{
		file_t *tmp;
		tmp = (file_t*) desc_table->file_descriptors;
		while (tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = f;
	}
	mutex_release(&current_task->descriptors_info->lock);
	return f->fd;
}

/*
 * Clones a file descriptor table.
 */
desc_entry_t *clone_descriptor_table(void)
{
	file_t *f, *tmp, **next;
	desc_entry_t *src, *table;
	table = (desc_entry_t*) malloc(sizeof(desc_entry_t));
	if (unlikely(table == NULL))
	{
		current_task->errno = ENOMEM;
		return NULL;
	}
	
	table->fd_next = 0;
	table->lock = MUTEX_INITIALIZER;
	table->refs = 0;
	table->file_descriptors = NULL;
	next = &table->file_descriptors;

	mutex_wait(&current_task->descriptors_info->lock);
	src = (desc_entry_t*) current_task->descriptors_info;
	assert(src != NULL);
	table->fd_next = src->fd_next;
	tmp = src->file_descriptors;
	
	while (tmp != NULL)
	{
		f = (file_t*) malloc(sizeof(file_t));
		if (unlikely(f == NULL))
		{
			mutex_release(&current_task->descriptors_info->lock);
			tmp = table->file_descriptors;
			while (tmp != NULL)
			{
				f = tmp->next;
				free(tmp);
				tmp = f;					
			}
			free(table);
			current_task->errno = ENOMEM;
			return NULL;
		}
		*f = *tmp;
		mutex_wait(&f->node->lock);
		f->node->refs++;
		mutex_release(&f->node->lock);
		f->next = NULL;		
		*next = f;
		next = &f->next;
		tmp = tmp->next;
	}
	mutex_release(&current_task->descriptors_info->lock);
	return table;
}
/*
 * Destroy the current task's descriptor table.
 */
void destroy_descriptor_table(void)
{
	mutex_wait(&current_task->descriptors_info->lock);
	if (current_task->descriptors_info->refs == 0)
	{
		file_t *f, *next;
		/*
		 * If we're here then there's no other threads to
		 * hold the lock so we just release it now so close()
		 * can grab it.
		 */
		mutex_release(&current_task->descriptors_info->lock);	
		f = current_task->descriptors_info->file_descriptors;
		while (f != NULL)
		{
			next = f->next;
			close(f->fd);
			f = next;
		}
		free(current_task->descriptors_info);
		current_task->descriptors_info = NULL;
		return;
	}
	else
	{
		current_task->descriptors_info->refs--;
	}
	mutex_release(&current_task->descriptors_info->lock);
}

/*
 * Read from a pipe.
 */
static size_t pipe_read(vfs_node_t* node, uint32_t offset, uint32_t len, uint8_t *buf)
{
	pipe_t *pipe;
	size_t c;
	
	assert(node->data != NULL);
	pipe = (pipe_t*)node->data;
	c = 0;
	while (len--)
	{
		semaphore_waitsleep(&pipe->semaphore);
		*buf++ = *pipe->read_ptr++;
		if (pipe->read_ptr > pipe->buf_end)
			pipe->read_ptr = pipe->buf;
		c++;
	}
	return c;
}

/*
 * Write to a pipe.
 */
static size_t pipe_write(vfs_node_t* node, uint32_t offset, uint32_t len, uint8_t *buf)
{
	pipe_t *pipe;
	uint32_t c;

	assert(node != NULL);
	assert(node->data != NULL);
	
	/*
	 * If the other end of the pipe has been closed
	 * raise the SIGPIPE signal
	 */
	if (node->refs < 2)
		raise(SIGPIPE);
	
	pipe = (pipe_t*) node->data;
	c = 0;
	
	while (len--)
	{
		*pipe->write_ptr++ = *buf++;
		if (pipe->write_ptr > pipe->buf_end)
			pipe->write_ptr = pipe->buf;
		semaphore_signal_sleep(&pipe->semaphore);
		c++;
	}
	
	return c;
}

/*
 * Rewind file descriptor
 */
int rewindfd(int fd)
{
	file_t *f;
	f = get_file_descriptor(fd);
	if (unlikely(f == NULL))
	{
		current_task->errno = EBADF;
		return -1;
	}
	f->offset = 0;
	return 0;
}

/*
 * Duplicates a file descriptor
 */
int dup(int oldfd)
{
	return dup2(oldfd, -1);
}

/*
 * Duplicate a file descriptor and assign newfd
 * to the it. If newfd is already open it is closed.
 */
int dup2(int oldfd, int newfd)
{
	int fd;
	file_t *old_desc, *new_desc;
	
	old_desc = get_file_descriptor(oldfd);
	if (old_desc == NULL)
	{
		current_task->errno = EBADF;
		return -1;
	}
	assert(old_desc->node != NULL);
	
	new_desc = get_file_descriptor(newfd);
	if (new_desc != NULL)
		close(new_desc->fd);
	
	mutex_wait(&old_desc->node->lock);
	old_desc->node->refs++;
	fd = create_file_descriptor(old_desc->node, newfd, old_desc->mode);
	if (fd < 0)
	{
		mutex_release(&old_desc->node->lock);
		return fd;
	}
	
	mutex_release(&old_desc->node->lock);
	new_desc = get_file_descriptor(fd);
	assert(new_desc != NULL);
	new_desc->offset = old_desc->offset;	
	return fd;
}

/*
 * Flush file system caches.
 */
int sync(void)
{
	current_task->errno = ENOSYS;
	return -1;
}

/*
 * Flush the buffer for an open file descriptor.
 */
int fsync(int fd)
{
	return sync();
}

int unlink(const char *path)
{
	current_task->errno = ENOSYS;
	return -1;
}

/*
 * Reposition read/write file offset
 */
off_t lseek(int fd, off_t offset, int whence)
{
	file_t *f;
	f = get_file_descriptor(fd);
	if (unlikely(f == NULL))
	{
		current_task->errno = EBADF;
		return (off_t) -1;
	}
	switch (whence)
	{
		case SEEK_SET:
			f->offset = offset;
			break;
		case SEEK_CUR:
			f->offset += offset;
			break;
		case SEEK_END:
			f->offset = f->node->length + offset;
			break;
		default:
			current_task->errno = EINVAL;
			return (off_t) -1;
		
	}
	return (off_t) f->offset;
}

/*
 * Create new name for file.
 */
int link(const char *oldpath, const char *newpath)
{
	current_task->errno = ENOSYS;
	return -1;
}

/*
 * Returns 1 if the file descriptor is a tty,
 * zero otherwise.
 */
int isatty(int fd)
{
	return 0;
}

/*
 * Change the owner of a file
 */
int chown(const char *path, uid_t uid, gid_t gid)
{
	vfs_node_t *node;
	node = vfs_open(path, NULL);
	if (unlikely(node == NULL))
	{
		current_task->errno = ENOENT;
		return -1;
	}
	node->uid = uid;
	node->gid = gid;
	vfs_close(node);
	return 0;
}

/*
 * Change root directory.
 */
int chroot(const char *path)
{
	vfs_node_t *node;
	node = vfs_open(path, NULL);
	if (node == NULL)
	{
		current_task->errno = ENOENT;
		return -1;
	}
	if (current_task->root != NULL)
		vfs_close(current_task->root);
	current_task->root = node;
	return 0;
}

int kfcntl(int fd, int cmd, va_list args)
{
	current_task->errno = ENOSYS;
	return -1;
}

int fcntl(int fd, int cmd, ...)
{
	int ret;
	va_list args;
	va_start(args, cmd);
	ret = kfcntl(fd, cmd, args);
	va_end(args);
	return ret;
}

/*
 * Read value of a symbolic link
 */
ssize_t readlink(const char *path, char *buf, size_t bufsize)
{
	current_task->errno = ENOSYS;
	return -1;
}

/*
 * Create a pipe.
 */
int pipe(int pipefd[2])
{
	vfs_node_t *node;
	pipe_t *pipe;
	
	node = (vfs_node_t*) malloc(sizeof(vfs_node_t));
	if (unlikely(node == NULL))
	{
		current_task->errno = ENOMEM;
		return -1;
	}
	
	pipe = (pipe_t*) malloc(sizeof(pipe_t));
	if (unlikely(pipe == NULL))
	{
		free(node);
		current_task->errno = ENOMEM;
		return -1;
	}
	
	pipe->buf = (void*) malloc(CONFIG_PIPE_BUFFER_SIZE);
	if (unlikely(pipe->buf == NULL))
	{
		free(pipe);
		free(node);
		current_task->errno = ENOMEM;
		return -1;
	}
	
	pipe->buf_end = pipe->buf + CONFIG_PIPE_BUFFER_SIZE;
	pipe->read_ptr = pipe->buf;
	pipe->write_ptr = pipe->buf;
	pipe->semaphore = SEMAPHORE_INITIALIZER(CONFIG_PIPE_BUFFER_SIZE, CONFIG_PIPE_BUFFER_SIZE);
	
	*node = VFS_NODE_INITIALIZER(FS_PIPE | FS_AUTOFREE);
	node->read = (vfs_read_fn_t) &pipe_read;
	node->write = (vfs_write_fn_t) &pipe_write;
	node->data = pipe;
	node->refs = 2;
	
	pipefd[0] = create_file_descriptor(node, -1, FD_MODE_READ);
	pipefd[1] = create_file_descriptor(node, -1, FD_MODE_WRITE);
	return 0;
}

/*
 * opens a file and returns a file descriptor
 */
int open(const char *pathname, int flags, ...)
{
	vfs_node_t *node;

	if (unlikely(strlen(pathname) > MAX_PATH))
	{
		current_task->errno = ENAMETOOLONG;
		return -1;
	}

	/* open file node */
	node = vfs_open(pathname, flags);
	if (unlikely(node == NULL))
		return -1;	/* errno set by vfs_open() */
	
	if (!check_file_permissions(node, FS_USR_READ | FS_GRP_READ))
	{
		vfs_close(node);
		current_task->errno = EACCES;
		return -1;
	}
	
	return create_file_descriptor(node, -1, FD_MODE_READ | FD_MODE_WRITE);
}

/*
 * Close a file descriptor.
 *
 * Removes the file from the file descriptor table of the current task
 * and frees it's memory
 */
int close(int fd)
{
	file_t *f, *parent;
	desc_info_t *desc_table;

	desc_table = current_task->descriptors_info;
	if (unlikely(desc_table == NULL || desc_table->file_descriptors == NULL))
	{
		current_task->errno = EBADF;
		return -1;
	}

	mutex_wait(&current_task->descriptors_info->lock);
	f = (file_t*) desc_table->file_descriptors;
	parent = NULL;
	while (f != NULL && f->fd != fd)
	{
		parent = f;
		f = f->next;
	}
	assert(f->node != NULL);
	
	if (unlikely(parent == NULL))
		desc_table->file_descriptors = f->next;
	else
		parent->next = f->next;

	vfs_close(f->node);
	free(f);
	mutex_release(&current_task->descriptors_info->lock);
	return 0;
}

/*
 * Open a directory.
 */
DIR *opendir(char *path)
{
	int fd;
	file_t *desc;
	fd = open(path, NULL, NULL);
	if (fd == -1)
		return NULL;
	desc = get_file_descriptor(fd);
	assert(desc != NULL);
	return (DIR*) &desc->fd;
}

/*
 * Read a directory entry.
 */
int readdir_r(DIR *dir, struct dirent *entry, struct dirent **result)
{
	file_t *f;
	f = get_file_descriptor(*dir);
	if (f == NULL)
	{
		current_task->errno = EBADF;
		return -1;
	}
	*result = vfs_readdir(f->node, f->offset++, entry);
	return 0;
}

/*
 * Close a directory.
 */
int closedir(DIR *dir)
{
	return close((int) *dir);
}

/*
 * read a directory
 */
#if 0
int readdir(unsigned int fd, struct dirent *dirent, unsigned int count)
{
	int i = 0;
	file_t *f; 
	
	f = get_file_descriptor(fd);
	if (f == NULL)
		return EBADF;

	while (count--)
	{
		if (vfs_readdir(f->node, f->offset++, dirent++) == NULL) 
			break;
		i++;
	}
	return i;
}
#endif

/*
 * Writes to a file descriptor
 */
ssize_t write(int fd, const void* buf, size_t count)
{
	file_t *f;
	size_t bytes_written = 0;

	f = get_file_descriptor(fd);
	if (unlikely(f == NULL))
	{
		current_task->errno = EBADF;
		return -1;
	}

	if (f->node->write == NULL)
		return -1;

	bytes_written = vfs_write(f->node, f->offset, count, buf);
	f->offset += bytes_written;
	return bytes_written;
}

/*
 * Reads from the file system
 */
ssize_t read(int fd, void *buf, size_t count)
{
	file_t *f;
	size_t bytes_read = 0;
	f = get_file_descriptor(fd);
	if (unlikely(f == NULL))
		return EBADF;
	bytes_read = vfs_read(f->node, f->offset, count, buf);
	f->offset += bytes_read;
	return bytes_read;
}

/*
 * Get information about a file.
 */
int stat(const char *path, struct stat *buf)
{
	if (unlikely(strlen(path) > MAX_PATH))
	{
		current_task->errno = ENAMETOOLONG;
		return -1;
	}
	if (unlikely(buf == NULL))
	{
		current_task->errno = EFAULT;
		return -1;
	}
	if (vfs_stat(NULL, path, buf) == NULL)
	{
		current_task->errno = EFAULT;
		return -1;
	}
	return 0;
}

/*
 * Get file information
 */
int fstat(int fd, struct stat *buf)
{
	file_t *f;
	if (unlikely(buf == NULL))
	{
		current_task->errno = EFAULT;
		return -1;
	}
	f = get_file_descriptor(fd);
	if (unlikely(f == NULL))
	{
		current_task->errno = EBADF;
		return -1;
	}
	assert(f->node != NULL);
	if (vfs_stat(f->node, NULL, buf) == NULL)
	{
		current_task->errno = EFAULT;
		return -1;
	}
	return 0;
}

/*
 * Get information about a file
 */
int lstat (const char *path, struct stat *buf)
{
	current_task->errno = ENOSYS;
	return -1;
}

/*
 * Change working directory
 */
int chdir(const char *path)
{
	vfs_node_t *node;
	if (unlikely(strlen(path) > MAX_PATH))
	{
		current_task->errno = ENAMETOOLONG;
		return -1;
	}
	node = vfs_open(path, NULL);
	if (unlikely(node == NULL))
	{
		current_task->errno = ENOENT;
		return -1;
	}
	scheduler_chdir(node);
	return 0;
}

/*
 * Create a directory.
 */
int mkdir(const char *path)
{
	current_task->errno = ENOSYS;
	return -1;
}

/*
 * Get the current working directory
 */
char *getcwd(char *buf, size_t size)
{
	vfs_node_t *node;
	char path[MAX_PATH + 1];
	
	if (unlikely(buf == NULL))
	{
		current_task->errno = EFAULT;
		return NULL;
	}
	node = get_current_task()->cwd;
	if (node == NULL)
	{
		buf[0] = '/';
		buf[1] = NULL;
		return buf;
	}
			
	vfs_get_path(node, &path[0]);
	if (unlikely(strlen(path) > size))
	{
		current_task->errno = ENAMETOOLONG;
		return NULL;
	}
	assert(strlen(path) < size);
	strcpy(buf, path);
	return buf;
}

/*
 * Send an IOCTL request to a device
 */
int ioctl(int fd, unsigned int request, ...)
{
	file_t *f;
	f = get_file_descriptor(fd);
	if (unlikely(f == NULL))
	{
		current_task->errno = EBADF;
		return -1;
	}
	assert(f->node != NULL);
	return vfs_ioctl(f->node, request, &request);
}
