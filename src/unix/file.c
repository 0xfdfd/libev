#define _GNU_SOURCE 1
#include "ev-common.h"
#include "unix/io.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

static void _ev_file_on_close(ev_handle_t* handle)
{
    ev_file_t* file = EV_CONTAINER_OF(handle, ev_file_t, base);
    if (file->close_cb != NULL)
    {
        file->close_cb(file);
    }
}

static void _ev_file_init_token(ev_fs_req_t* req, ev_file_t* file, ev_file_cb cb)
{
    req->cb = cb;
    req->file = file;
    req->result = EV_EINPROGRESS;
}

static int _ev_file_init_token_as_open(ev_fs_req_t* token, ev_file_t* file,
    const char* path, int flags, int mode, ev_file_cb cb)
{
    _ev_file_init_token(token, file, cb);

    if ((token->req.as_open.path = ev__strdup(path)) == NULL)
    {
        return EV_ENOMEM;
    }

    token->req.as_open.flags = flags;
    token->req.as_open.mode = mode;

    return EV_SUCCESS;
}

static void _ev_file_cleanup_token_as_open(ev_fs_req_t* token)
{
    if (token->req.as_open.path != NULL)
    {
        ev__free(token->req.as_open.path);
        token->req.as_open.path = NULL;
    }
}

static void _ev_file_on_open(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;

    req->result = 0;

    int flags = req->req.as_open.flags;
#if defined(O_CLOEXEC)
    flags |= O_CLOEXEC;
#endif

    file->file = open(req->req.as_open.path, flags, req->req.as_open.mode);

    if (file->file < 0)
    {
        int err = errno;
        req->result = ev__translate_sys_error(err);
        return;
    }

#if defined(O_CLOEXEC)
    if ((req->result = ev__cloexec(file->file, 1)) != EV_SUCCESS)
    {
        close(file->file);
        file->file = -1;
        return;
    }
#endif
}

static void _ev_file_on_done(ev_threadpool_work_t* work, int status, void (*cb)(ev_fs_req_t*))
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;

    if (status == EV_ECANCELED)
    {
        assert(req->result == EV_SUCCESS);
        req->result = EV_ECANCELED;
    }

    if (cb != NULL)
    {
        cb(req);
    }
    req->cb(file, req);
}

static void _ev_file_on_open_done(ev_threadpool_work_t* work, int status)
{
    _ev_file_on_done(work, status, _ev_file_cleanup_token_as_open);
}

static int _ev_file_init_token_as_read(ev_fs_req_t* req, ev_file_t* file,
    ev_buf_t bufs[], size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    _ev_file_init_token(req, file, cb);

    int ret = ev__read_init(&req->req.as_read.read_req, bufs, nbuf);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }
    req->req.as_read.offset = offset;

    return EV_SUCCESS;
}

static void _ev_file_cleanup_token_as_read(ev_fs_req_t* req)
{
    ev__read_exit(&req->req.as_read.read_req);
}

static void _ev_file_on_read(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;
    ev_read_t* read_req = &req->req.as_read.read_req;
    off_t offset = req->req.as_read.offset;

    req->result = preadv(file->file, (struct iovec*)read_req->data.bufs,
        read_req->data.nbuf, offset);
}

static void _ev_file_on_read_done(ev_threadpool_work_t* work, int status)
{
    _ev_file_on_done(work, status, _ev_file_cleanup_token_as_read);
}

static int _ev_file_init_token_as_write(ev_fs_req_t* token, ev_file_t* file,
    ev_buf_t bufs[], size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    _ev_file_init_token(token, file, cb);

    int ret = ev__write_init(&token->req.as_write.write_req, bufs, nbuf);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }
    token->req.as_write.offset = offset;

    return EV_SUCCESS;
}

static void _ev_file_cleanup_token_as_write(ev_fs_req_t* req)
{
    ev__write_exit(&req->req.as_write.write_req);
}

static void _ev_file_on_write(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;
    ev_write_t* write_req = &req->req.as_write.write_req;
    off_t offset = req->req.as_write.offset;

    req->result = pwritev(file->file, (struct iovec*)write_req->data.bufs,
        write_req->data.nbuf, offset);
}

static void _ev_file_on_write_done(ev_threadpool_work_t* work, int status)
{
    _ev_file_on_done(work, status, _ev_file_cleanup_token_as_write);
}

static int _ev_file_fstat(int fd, ev_file_stat_t* buf)
{
    int ret;
    int errcode;

#if defined(__GLIBC_PREREQ) && __GLIBC_PREREQ(2, 28)
    struct statx statxbuf;
    ret = statx(fd, "", AT_EMPTY_PATH, STATX_ALL, &statxbuf);
    if (ret != 0)
    {
        goto err_errno;
    }

    buf->st_dev                 = makedev(statxbuf.stx_dev_major, statxbuf.stx_dev_minor);
    buf->st_mode                = statxbuf.stx_mode;
    buf->st_nlink               = statxbuf.stx_nlink;
    buf->st_uid                 = statxbuf.stx_uid;
    buf->st_gid                 = statxbuf.stx_gid;
    buf->st_rdev                = makedev(statxbuf.stx_rdev_major, statxbuf.stx_rdev_minor);
    buf->st_ino                 = statxbuf.stx_ino;
    buf->st_size                = statxbuf.stx_size;
    buf->st_blksize             = statxbuf.stx_blksize;
    buf->st_blocks              = statxbuf.stx_blocks;
    buf->st_atim.tv_sec         = statxbuf.stx_atime.tv_sec;
    buf->st_atim.tv_nsec        = statxbuf.stx_atime.tv_nsec;
    buf->st_mtim.tv_sec         = statxbuf.stx_mtime.tv_sec;
    buf->st_mtim.tv_nsec        = statxbuf.stx_mtime.tv_nsec;
    buf->st_ctim.tv_sec         = statxbuf.stx_ctime.tv_sec;
    buf->st_ctim.tv_nsec        = statxbuf.stx_ctime.tv_nsec;
    buf->st_birthtim.tv_sec     = statxbuf.stx_btime.tv_sec;
    buf->st_birthtim.tv_nsec    = statxbuf.stx_btime.tv_nsec;
    buf->st_flags = 0;
    buf->st_gen = 0;
#else
    struct stat pbuf;
    ret = fstat(fd, &pbuf);
    if (ret != 0)
    {
        goto err_errno;
    }
    buf->st_dev                 = pbuf.st_dev;
    buf->st_mode                = pbuf.st_mode;
    buf->st_nlink               = pbuf.st_nlink;
    buf->st_uid                 = pbuf.st_uid;
    buf->st_gid                 = pbuf.st_gid;
    buf->st_rdev                = pbuf.st_rdev;
    buf->st_ino                 = pbuf.st_ino;
    buf->st_size                = pbuf.st_size;
    buf->st_blksize             = pbuf.st_blksize;
    buf->st_blocks              = pbuf.st_blocks;

#   if defined(__APPLE__)
    buf->st_atim.tv_sec         = pbuf.st_atimespec.tv_sec;
    buf->st_atim.tv_nsec        = pbuf.st_atimespec.tv_nsec;
    buf->st_mtim.tv_sec         = pbuf.st_mtimespec.tv_sec;
    buf->st_mtim.tv_nsec        = pbuf.st_mtimespec.tv_nsec;
    buf->st_ctim.tv_sec         = pbuf.st_ctimespec.tv_sec;
    buf->st_ctim.tv_nsec        = pbuf.st_ctimespec.tv_nsec;
    buf->st_birthtim.tv_sec     = pbuf.st_birthtimespec.tv_sec;
    buf->st_birthtim.tv_nsec    = pbuf.st_birthtimespec.tv_nsec;
    buf->st_flags               = pbuf.st_flags;
    buf->st_gen                 = pbuf.st_gen;
#   elif defined(__ANDROID__)
    buf->st_atim.tv_sec         = pbuf.st_atime;
    buf->st_atim.tv_nsec        = pbuf.st_atimensec;
    buf->st_mtim.tv_sec         = pbuf.st_mtime;
    buf->st_mtim.tv_nsec        = pbuf.st_mtimensec;
    buf->st_ctim.tv_sec         = pbuf.st_ctime;
    buf->st_ctim.tv_nsec        = pbuf.st_ctimensec;
    buf->st_birthtim.tv_sec     = pbuf.st_ctime;
    buf->st_birthtim.tv_nsec    = pbuf.st_ctimensec;
    buf->st_flags = 0;
    buf->st_gen = 0;
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
    buf->st_atim.tv_sec         = pbuf.st_atim.tv_sec;
    buf->st_atim.tv_nsec        = pbuf.st_atim.tv_nsec;
    buf->st_mtim.tv_sec         = pbuf.st_mtim.tv_sec;
    buf->st_mtim.tv_nsec        = pbuf.st_mtim.tv_nsec;
    buf->st_ctim.tv_sec         = pbuf.st_ctim.tv_sec;
    buf->st_ctim.tv_nsec        = pbuf.st_ctim.tv_nsec;
#       if defined(__FreeBSD__) || defined(__NetBSD__)
    buf->st_birthtim.tv_sec     = pbuf.st_birthtim.tv_sec;
    buf->st_birthtim.tv_nsec    = pbuf.st_birthtim.tv_nsec;
    buf->st_flags               = pbuf.st_flags;
    buf->st_gen                 = pbuf.st_gen;
#       else
    buf->st_birthtim.tv_sec     = pbuf.st_ctim.tv_sec;
    buf->st_birthtim.tv_nsec    = pbuf.st_ctim.tv_nsec;
    buf->st_flags = 0;
    buf->st_gen = 0;
#       endif
#   else
    buf->st_atim.tv_sec         = pbuf.st_atime;
    buf->st_atim.tv_nsec        = 0;
    buf->st_mtim.tv_sec         = pbuf.st_mtime;
    buf->st_mtim.tv_nsec        = 0;
    buf->st_ctim.tv_sec         = pbuf.st_ctime;
    buf->st_ctim.tv_nsec        = 0;
    buf->st_birthtim.tv_sec     = pbuf.st_ctime;
    buf->st_birthtim.tv_nsec    = 0;
    buf->st_flags               = 0;
    buf->st_gen                 = 0;
#   endif
#endif

    return EV_SUCCESS;

err_errno:
    errcode = errno;
    return ev__translate_sys_error(errcode);
}

static void _ev_file_on_fstat(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;

    req->result = _ev_file_fstat(file->file, &req->rsp.as_fstat.fileinfo);
}

static void _ev_file_on_fstat_done(ev_threadpool_work_t* work, int status)
{
    _ev_file_on_done(work, status, NULL);
}

int ev_file_init(ev_loop_t* loop, ev_file_t* file)
{
    if (loop->threadpool == NULL)
    {
        return EV_ENOTHREADPOOL;
    }

    file->file = EV_OS_FILE_INVALID;
    file->close_cb = NULL;
    ev__handle_init(loop, &file->base, EV_ROLE_EV_FILE, _ev_file_on_close);

    return EV_SUCCESS;
}

void ev_file_exit(ev_file_t* file, ev_file_close_cb cb)
{
    /**
     * TODO: we may have pending work in threadpool, so need to stop or wait for result.
     */

    if (file->file != EV_OS_FILE_INVALID)
    {
        close(file->file);
        file->file = EV_OS_FILE_INVALID;
    }

    file->close_cb = cb;
    ev__handle_exit(&file->base, 0);
}

int ev_file_open(ev_file_t* file, ev_fs_req_t* token, const char* path, int flags, int mode, ev_file_cb cb)
{
    ev_loop_t* loop = file->base.data.loop;
    ev_threadpool_t* pool = loop->threadpool;

    int ret = _ev_file_init_token_as_open(token, file, path, flags, mode, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ret = ev_threadpool_submit(pool, loop, &token->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_open, _ev_file_on_open_done);
    if (ret != EV_SUCCESS)
    {
        _ev_file_cleanup_token_as_open(token);
        return ret;
    }

    return EV_SUCCESS;
}

int ev_file_read(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    ev_loop_t* loop = file->base.data.loop;
    ev_threadpool_t* pool = loop->threadpool;

    int ret = _ev_file_init_token_as_read(req, file, bufs, nbuf, offset, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_read, _ev_file_on_read_done);
    if (ret != EV_SUCCESS)
    {
        _ev_file_cleanup_token_as_read(req);
        return ret;
    }

    return EV_SUCCESS;
}

int ev_file_write(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    int ret;
    ev_loop_t* loop = file->base.data.loop;
    ev_threadpool_t* pool = loop->threadpool;

    ret = _ev_file_init_token_as_write(req, file, bufs, nbuf, offset, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_write, _ev_file_on_write_done);
    if (ret != EV_SUCCESS)
    {
        _ev_file_cleanup_token_as_write(req);
        return ret;
    }

    return EV_SUCCESS;
}

int ev_file_stat(ev_file_t* file, ev_fs_req_t* req, ev_file_cb cb)
{
    int ret;
    ev_loop_t* loop = file->base.data.loop;
    ev_threadpool_t* pool = loop->threadpool;

    _ev_file_init_token(req, file, cb);

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_fstat, _ev_file_on_fstat_done);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    return EV_SUCCESS;
}
