#define _GNU_SOURCE 1
#include "ev-common.h"
#include "file-common.h"
#include "unix/io.h"
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

int ev__fs_fstat(ev_os_file_t file, ev_file_stat_t* statbuf)
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
    statbuf->st_flags = 0;
    statbuf->st_gen = 0;
#else
    struct stat pbuf;
    ret = fstat(fd, &pbuf);
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
    statbuf->st_flags = 0;
    statbuf->st_gen = 0;
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
    statbuf->st_flags = 0;
    statbuf->st_gen = 0;
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

    return EV_SUCCESS;

err_errno:
    errcode = errno;
    return ev__translate_sys_error(errcode);
}

int ev__fs_close(ev_os_file_t file)
{
    int errcode;
    if (close(file) != 0)
    {
        errcode = errno;
        return ev__translate_sys_error(errcode);
    }
    return EV_SUCCESS;
}

int ev__fs_open(ev_os_file_t* file, const char* path, int flags, int mode)
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
    if ((errcode = ev__cloexec(fd, 1)) != EV_SUCCESS)
    {
        close(fd);
        return errcode;
    }
#endif

    *file = fd;
    return EV_SUCCESS;
}

ssize_t ev__fs_preadv(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf, ssize_t offset)
{
    ssize_t read_size = preadv(file, (struct iovec*)bufs, nbuf, offset);
    if (read_size >= 0)
    {
        return read_size;
    }

    int errcode = errno;
    return ev__translate_sys_error(errcode);
}

ssize_t ev__fs_pwritev(ev_os_file_t file, ev_buf_t* bufs, size_t nbuf, ssize_t offset)
{
    ssize_t write_size = pwritev(file, (struct iovec*)bufs, nbuf, offset);
    if (write_size >= 0)
    {
        return write_size;
    }

    int errcode = errno;
    return ev__translate_sys_error(errcode);
}

int ev__fs_readdir(const char* path, ev_fs_readdir_cb cb, void* arg)
{
    int ret;
    DIR* dir = opendir(path);

    if (dir == NULL)
    {
        ret = errno;
        return ev__translate_sys_error(ret);
    }

    struct dirent* res;
    ev_dirent_t info;

    ret = 0;
    while ((res = readdir(dir)) != NULL)
    {
        if (strcmp(res->d_name, ".") == 0 || strcmp(res->d_name, "..") == 0)
        {
            continue;
        }

        info.name = res->d_name;
        info.type = _ev_fs_get_dirent_type(res);

        ret++;
        if (cb(&info, arg) != 0)
        {
            break;
        }
    }

    closedir(dir);

    return ret;
}
