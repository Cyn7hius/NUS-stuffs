#include "zc_io.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>

// The zc_file struct is analogous to the FILE struct that you get from fopen.
struct zc_file
{
    // Insert the fields you need here.
    int fd;          // file descriptor to the opened file
    int num_readers; // number of readers
    void *ptr;       // pointer to the virtual memory space
    sem_t r_mutex;   // read mutex
    sem_t w_mutex;   // write mutex
    size_t size;     // total size of the file
    size_t offset;   // offset from the start of the virtual memory
};

/**************
 * Exercise 1A *
 **************/
zc_file *zc_open(const char *path)
{
    int fd = open(path, O_RDWR | O_CREAT, S_IRWXU); // create and set user permission to rwx
    struct stat buf;
    fstat(fd, &buf);
    off_t size = buf.st_size; /* total size, in bytes */

    // open fail
    if (fd == -1)
        return NULL;

    // open success
    else
    {
        void *ptr;
        zc_file *file = malloc(sizeof(zc_file));

        // new file --> call mmap with 1 byte to be used
        if (size == 0)
            size = 1;

        ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        // save the file details
        file->fd = fd;
        file->offset = 0;
        file->num_readers = 0;
        file->size = size;
        file->ptr = ptr;

        // Init the mutex
        sem_init(&file->r_mutex, 0, 1);
        sem_init(&file->w_mutex, 0, 1);

        return file;
    }
}

int zc_close(zc_file *file)
{
    int error = msync(file->ptr, file->size, MS_SYNC) | munmap(file->ptr, file->size) | close(file->fd);
    free(file);
    return error;
}

const char *zc_read_start(zc_file *file, size_t *size)
{
    // should not happen, but just in case
    if (file->offset > file->size)
        return NULL;

    sem_wait(&file->r_mutex);

    // cannot write if file is being read
    file->num_readers += 1;
    if (file->num_readers == 1)
        sem_wait(&file->w_mutex);

    sem_post(&file->r_mutex);

    char *chunk = (char *)file->ptr + file->offset;

    // If the file contains less than *size bytes remaining, then the number of bytes available should be written to *size
    if (file->offset + *size > file->size)
        *size = file->size - file->offset;

    file->offset += *size;
    return chunk;
}

void zc_read_end(zc_file *file)
{
    sem_wait(&file->r_mutex);

    // unlock write mutex if no file is reading
    file->num_readers -= 1;
    if (file->num_readers == 0)
        sem_post(&file->w_mutex);

    sem_post(&file->r_mutex);
}

/**************
 * Exercise 1B *
 **************/
char *zc_write_start(zc_file *file, size_t size)
{
    sem_wait(&file->w_mutex);

    // if not enough space, need truncate
    if (file->offset + size >= file->size)
    {
        ftruncate(file->fd, file->offset + size);
        file->ptr = mremap(file->ptr, file->size, file->offset + size, MREMAP_MAYMOVE);

        if (file->ptr == MAP_FAILED)
            return NULL;

        file->size = file->offset + size; // set new file size
    }

    char *chunk = (char *)file->ptr + file->offset;
    file->offset += size; // set new offset for future use
    return chunk;
}

void zc_write_end(zc_file *file)
{
    // flush and unlock the mutex
    msync(file->ptr, file->size, MS_SYNC);
    sem_post(&file->w_mutex);
}

/**************
 * Exercise 2 *
 **************/

off_t zc_lseek(zc_file *file, long offset, int whence)
{
    sem_wait(&file->w_mutex);

    off_t new_offset = -1;
    switch (whence)
    {
    case SEEK_SET:
        if (offset >= 0)
        {
            file->offset = offset;
            new_offset = file->offset;
        }
        break;
    case SEEK_CUR:
        if ((long)file->offset + offset >= 0)
        {
            file->offset += offset;
            new_offset = file->offset;
        }
        break;
    case SEEK_END:
        if ((long)file->size + offset >= 0)
        {
            file->offset = file->size + offset;
            new_offset = file->offset;
        }
        break;
    }

    sem_post(&file->w_mutex);
    return new_offset;
}

/**************
 * Exercise 3 *
 **************/
int zc_copyfile(const char *source, const char *dest)
{
    zc_file *src = zc_open(source);
    zc_file *dst = zc_open(dest);
    off_t sourceOffset = 0;
    off_t destOffset = 0;

    ftruncate(dst->fd, src->size);

    zc_read_start(src, &(src->size));
    zc_write_start(dst, src->size);

    copy_file_range(src->fd, &(sourceOffset), dst->fd, &(destOffset), src->size, 0);

    zc_read_end(src);
    zc_write_end(dst);
    zc_close(src);
    zc_close(dst);

    return 0;
}

/**************
 * Bonus Exercise *
 **************/

const char *zc_read_offset(zc_file *file, size_t *size, long offset)
{
    // To implement
    return NULL;
}

char *zc_write_offset(zc_file *file, size_t size, long offset)
{
    // To implement
    return NULL;
}
