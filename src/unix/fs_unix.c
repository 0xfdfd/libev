#define _GNU_SOURCE
#include "ev.h"
#include "loop.h"
#include "fs.h"
#include "allocator.h"
#include "misc_unix.h"
#include "io_unix.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

static ev_dirent_type_t _ev_fs_get_dirent_type(struct dirent* dent)
{
    ev_dirent_type_t type;

    switch (dent->d_type)
    {
    case DT_DIR:
        type = EV_DIRENT_DIR;
        break;
    case DT_REG:
        type = EV_DIRENT_FILE;
        break;
    case DT_LNK:
        type = EV_DIRENT_LINK;
        break;
    case DT_FIFO:
        type = EV_DIRENT_FIFO;
        break;
    case DT_SOCK:
        type = EV_DIRENT_SOCKET;
        break;
    case DT_CHR:
        type = EV_DIRENT_CHR;
        break;
    case DT_BLK:
        type = EV_DIRENT_BLOCK;
        break;
    default:
        type = EV_DIRENT_UNKNOWN;
        break;
    }

    return type;
}

static int _ev_fs_mkpath(char* file_path, int mode)
{
    char* p;
    int errcode;
    assert(file_path && *file_path);

    for (p = strchr(file_path + 1, '/'); p != NULL; p = strchr(p + 1, '/'))
    {
        *p = '\0';
        if (mkdir(file_path, mode) == -1)
        {
            errcode = errno;
            if (errcode != EEXIST)
            {
                *p = '/';
                return ev__translate_sys_error(errcode);
            }
        }
        *p = '/';
    }

    if (mkdir(file_path, mode) == -1)
    {
        errcode = errno;
        if (errcode != EEXIST)
        {
            return ev__translate_sys_error(errcode);
        }
    }

    return 0;
}

API_LOCAL int ev__fs_fstat(ev_os_file_t file, ev_fs_stat_t* statbuf)
{
    int ret;
    int errcode;

#if defined(__GLIBC_PREREQ) && __GLIBC_PREREQ(2, 28)
    struct statx statxbuf;
    ret = statx(file, "", AT_EMPTY_PATH, STATX_ALL, &statxbuf);
    if (ret != 0)
    {
        goto err_errno;
    }

    statbuf->st_dev                 = makedev(statxbuf.stx_dev_major, statxbuf.stx_dev_minor);
    statbuf->st_mode                = statxbuf.stx_mode;
    statbuf->st_nlink               = statxbuf.stx_nlink;
    statbuf->st_uid                 = statxbuf.stx_uid;
    statbuf->st_gid                 = statxbuf.stx_gid;
    statbuf->st_rdev                = makedev(statxbuf.stx_rdev_major, statxbuf.stx_rdev_minor);
    statbuf->st_ino                 = statxbuf.stx_ino;
    statbuf->st_size                = statxbuf.stx_size;
    statbuf->st_blksize             = statxbuf.stx_blksize;
    statbuf->st_blocks              = statxbuf.stx_blocks;
    statbuf->st_atim.tv_sec         = statxbuf.stx_atime.tv_sec;
    statbuf->st_atim.tv_nsec        = statxbuf.stx_atime.tv_nsec;
    statbuf->st_mtim.tv_sec         = statxbuf.stx_mtime.tv_sec;
    statbuf->st_mtim.tv_nsec        = statxbuf.stx_mtime.tv_nsec;
    statbuf->st_ctim.tv_sec         = statxbuf.stx_ctime.tv_sec;
    statbuf->st_ctim.tv_nsec        = statxbuf.stx_ctime.tv_nsec;
    statbuf->st_birthtim.tv_sec     = statxbuf.stx_btime.tv_sec;
    statbuf->st_birthtim.tv_nsec    = statxbuf.stx_btime.tv_nsec;
    statbuf->st_flags               = 0;
    statbuf->st_gen                 = 0;
#else
    struct stat pbuf;
    ret = fstat(file, &pbuf);
    if (ret != 0)
    {
        goto err_errno;
    }
    statbuf->st_dev                 = pbuf.st_dev;
    statbuf->st_mode                = pbuf.st_mode;
    statbuf->st_nlink               = pbuf.st_nlink;
    statbuf->st_uid                 = pbuf.st_uid;
    statbuf->st_gid                 = pbuf.st_gid;
    statbuf->st_rdev                = pbuf.st_rdev;
    statbuf->st_ino                 = pbuf.st_ino;
    statbuf->st_size                = pbuf.st_size;
    statbuf->st_blksize             = pbuf.st_blksize;
    statbuf->st_blocks              = pbuf.st_blocks;

#   if defined(__APPLE__)
    statbuf->st_atim.tv_sec         = pbuf.st_atimespec.tv_sec;
    statbuf->st_atim.tv_nsec        = pbuf.st_atimespec.tv_nsec;
    statbuf->st_mtim.tv_sec         = pbuf.st_mtimespec.tv_sec;
    statbuf->st_mtim.tv_nsec        = pbuf.st_mtimespec.tv_nsec;
    statbuf->st_ctim.tv_sec         = pbuf.st_ctimespec.tv_sec;
    statbuf->st_ctim.tv_nsec        = pbuf.st_ctimespec.tv_nsec;
    statbuf->st_birthtim.tv_sec     = pbuf.st_birthtimespec.tv_sec;
    statbuf->st_birthtim.tv_nsec    = pbuf.st_birthtimespec.tv_nsec;
    statbuf->st_flags               = pbuf.st_flags;
    statbuf->st_gen                 = pbuf.st_gen;
#   elif defined(__ANDROID__)
    statbuf->st_atim.tv_sec         = pbuf.st_atime;
    statbuf->st_atim.tv_nsec        = pbuf.st_atimensec;
    statbuf->st_mtim.tv_sec         = pbuf.st_mtime;
    statbuf->st_mtim.tv_nsec        = pbuf.st_mtimensec;
    statbuf->st_ctim.tv_sec         = pbuf.st_ctime;
    statbuf->st_ctim.tv_nsec        = pbuf.st_ctimensec;
    statbuf->st_birthtim.tv_sec     = pbuf.st_ctime;
    statbuf->st_birthtim.tv_nsec    = pbuf.st_ctimensec;
    statbuf->st_flags               = 0;
    statbuf->st_gen                 = 0;
#   elif !defined(_AIX) && !defined(__MVS__) && \
        (\
            defined(__DragonFly__)   || \
            defined(__FreeBSD__)     || \
            defined(__OpenBSD__)     || \
            defined(__NetBSD__)      || \
            defined(_GNU_SOURCE)     || \
            defined(_BSD_SOURCE)     || \
            defined(_SVID_SOURCE)    || \
            defined(_XOPEN_SOURCE)   || \
            defined(_DEFAULT_SOURCE)\
        )
    statbuf->st_atim.tv_sec         = pbuf.st_atim.tv_sec;
    statbuf->st_atim.tv_nsec        = pbuf.st_atim.tv_nsec;
    statbuf->st_mtim.tv_sec         = pbuf.st_mtim.tv_sec;
    statbuf->st_mtim.tv_nsec        = pbuf.st_mtim.tv_nsec;
    statbuf->st_ctim.tv_sec         = pbuf.st_ctim.tv_sec;
    statbuf->st_ctim.tv_nsec        = pbuf.st_ctim.tv_nsec;
#       if defined(__FreeBSD__) || defined(__NetBSD__)
    statbuf->st_birthtim.tv_sec     = pbuf.st_birthtim.tv_sec;
    statbuf->st_birthtim.tv_nsec    = pbuf.st_birthtim.tv_nsec;
    statbuf->st_flags               = pbuf.st_flags;
    statbuf->st_gen                 = pbuf.st_gen;
#       else
    statbuf->st_birthtim.tv_sec     = pbuf.st_ctim.tv_sec;
    statbuf->st_birthtim.tv_nsec    = pbuf.st_ctim.tv_nsec;
    statbuf->st_flags               = 0;
    statbuf->st_gen                 = 0;
#       endif
#   else
    statbuf->st_atim.tv_sec         = pbuf.st_atime;
    statbuf->st_atim.tv_nsec        = 0;
    statbuf->st_mtim.tv_sec         = pbuf.st_mtime;
    statbuf->st_mtim.tv_nsec        = 0;
    statbuf->st_ctim.tv_sec         = pbuf.st_ctime;
    statbuf->st_ctim.tv_nsec        = 0;
    statbuf->st_birthtim.tv_sec     = pbuf.st_ctime;
    statbuf->st_birthtim.tv_nsec    = 0;
    statbuf->st_flags               = 0;
    statbuf->st_gen                 = 0;
#   endif
#endif

    return 0;

err_errno:
    errcode = errno;
    return ev__translate_sys_error(errcode);
}

API_LOCAL int ev__fs_close(ev_os_file_t file)
{
    int errcode;
    if (close(file) != 0)
    {
        errcode = errno;
        return ev__translate_sys_error(errcode);
    }
    return 0;
}

API_LOCAL int ev__fs_open(ev_os_file_t* file, const char* path, int flags, int mode)
{
    int errcode;

#if defined(O_CLOEXEC)
    flags |= O_CLOEXEC;
#endif

    int fd = open(path, flags, mode);
    if (fd < 0)
    {
        errcode = errno;
        return ev__translate_sys_error(errcode);
    }

#if defined(O_CLOEXEC)
    if ((errcode = ev__cloexec(fd, 1)) != 0)
    {
        close(fd);
        return errcode;
    }
#endif

    *file = fd;
    return 0;
}

API_LOCAL int ev__fs_seek(ev_os_file_t file, int whence, ssize_t offset)
{
    int errcode;
    if (lseek(file, offset, whence) == (off_t)-1)
    {
        errcode = errno;
        return ev__translate_sys_error(errcode);
    }
    return 0;
}

API_LOCAL ssize_t ev__fs_readv(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf)
{
    ssize_t read_size = readv(file, (struct iovec*)bufs, nbuf);
    if (read_size >= 0)
    {
        return read_size;
    }

    int errcode = errno;
    return ev__translate_sys_error(errcode);
}

API_LOCAL ssize_t ev__fs_preadv(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf, ssize_t offset)
{
    ssize_t read_size = preadv(file, (struct iovec*)bufs, nbuf, offset);
    if (read_size >= 0)
    {
        return read_size;
    }

    int errcode = errno;
    return ev__translate_sys_error(errcode);
}

API_LOCAL ssize_t ev__fs_writev(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf)
{
    ssize_t write_size = writev(file, (struct iovec*)bufs, nbuf);
    if (write_size >= 0)
    {
        return write_size;
    }

    int errcode = errno;
    return ev__translate_sys_error(errcode);
}

API_LOCAL ssize_t ev__fs_pwritev(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf, ssize_t offset)
{
    ssize_t write_size = pwritev(file, (struct iovec*)bufs, nbuf, offset);
    if (write_size >= 0)
    {
        return write_size;
    }

    int errcode = errno;
    return ev__translate_sys_error(errcode);
}

API_LOCAL int ev__fs_readdir(const char* path, ev_fs_readdir_cb cb, void* arg)
{
    int ret = 0;
    DIR* dir = opendir(path);

    if (dir == NULL)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    struct dirent* res;
    ev_dirent_t info;

    while ((res = readdir(dir)) != NULL)
    {
        if (strcmp(res->d_name, ".") == 0 || strcmp(res->d_name, "..") == 0)
        {
            continue;
        }

        info.name = res->d_name;
        info.type = _ev_fs_get_dirent_type(res);

        if (cb(&info, arg) != 0)
        {
            break;
        }
    }

    closedir(dir);

    return ret;
}

API_LOCAL int ev__fs_mkdir(const char* path, int mode)
{
    char* dup_path = ev__strdup(path);
    if (dup_path == NULL)
    {
        return EV_ENOMEM;
    }

    int ret = _ev_fs_mkpath(dup_path, mode);
    ev_free(dup_path);

    return ret;
}
