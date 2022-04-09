#include "ev-common.h"
#include "unix/io.h"
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>

static void _ev_file_on_close(ev_handle_t* handle)
{
    ev_file_t* file = EV_CONTAINER_OF(handle, ev_file_t, base);
    if (file->close_cb != NULL)
    {
        file->close_cb(file);
    }
}

static int _ev_file_init_token_as_open(ev_file_req_t* token, ev_file_t* file,
    const char* path, int flags, int mode, ev_file_cb cb)
{
    token->file = file;
    token->cb = cb;
    token->result = EV_SUCCESS;

    if ((token->op.as_open.path = ev__strdup(path)) == NULL)
    {
        return EV_ENOMEM;
    }

    token->op.as_open.flags = flags;
    token->op.as_open.mode = mode;

    return EV_SUCCESS;
}

static void _ev_file_cleanup_token_as_open(ev_file_req_t* token)
{
    if (token->op.as_open.path != NULL)
    {
        ev__free(token->op.as_open.path);
        token->op.as_open.path = NULL;
    }
}

static void _ev_file_on_open(ev_threadpool_work_t* work)
{
    ev_file_req_t* req = EV_CONTAINER_OF(work, ev_file_req_t, work_token);
    ev_file_t* file = req->file;

    req->result = 0;

#if defined(O_CLOEXEC)
    file->file = open(req->op.as_open.path, req->op.as_open.flags | O_CLOEXEC, req->op.as_open.mode);
#else
    file->file = open(req->op.as_open.path, req->op.as_open.flags, req->op.as_open.mode);
#endif

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

static void _ev_file_on_open_done(ev_threadpool_work_t* work, int status)
{
    ev_file_req_t* req = EV_CONTAINER_OF(work, ev_file_req_t, work_token);
    ev_file_t* file = req->file;

    if (status == EV_ECANCELED)
    {
        assert(req->result == EV_SUCCESS);
        req->result = EV_ECANCELED;
    }

    _ev_file_cleanup_token_as_open(req);
    req->cb(file, req);
}

static int _ev_file_init_token_as_read(ev_file_req_t* req, ev_file_t* file,
    ev_buf_t bufs[], size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    int ret = ev__read_init(&req->op.as_read.read_req, bufs, nbuf);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }
    req->op.as_read.offset = offset;
    req->cb = cb;
    req->file = file;
    req->result = EV_SUCCESS;

    return EV_SUCCESS;
}

static void _ev_file_cleanup_token_as_read(ev_file_req_t* req)
{
    ev__read_exit(&req->op.as_read.read_req);
}

static void _ev_file_on_read(ev_threadpool_work_t* work)
{
    ev_file_req_t* req = EV_CONTAINER_OF(work, ev_file_req_t, work_token);
    ev_file_t* file = req->file;
    ev_read_t* read_req = &req->op.as_read.read_req;
    off_t offset = req->op.as_read.offset;

    req->result = preadv(file->file, (struct iovec*)read_req->data.bufs,
        read_req->data.nbuf, offset);
}

static void _ev_file_on_read_done(ev_threadpool_work_t* work, int status)
{
    ev_file_req_t* req = EV_CONTAINER_OF(work, ev_file_req_t, work_token);
    ev_file_t* file = req->file;

    if (status == EV_ECANCELED)
    {
        assert(req->result == EV_SUCCESS);
        req->result = EV_ECANCELED;
    }

    _ev_file_cleanup_token_as_read(req);
    req->cb(file, req);
}

static int _ev_file_init_token_as_write(ev_file_req_t* token, ev_file_t* file,
    ev_buf_t bufs[], size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    int ret = ev__write_init(&token->op.as_write.write_req, bufs, nbuf);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }
    token->op.as_write.offset = offset;
    token->cb = cb;
    token->file = file;
    token->result = EV_SUCCESS;

    return EV_SUCCESS;
}

static void _ev_file_cleanup_token_as_write(ev_file_req_t* req)
{
    ev__write_exit(&req->op.as_write.write_req);
}

static void _ev_file_on_write(ev_threadpool_work_t* work)
{
    ev_file_req_t* req = EV_CONTAINER_OF(work, ev_file_req_t, work_token);
    ev_file_t* file = req->file;
    ev_write_t* write_req = &req->op.as_write.write_req;
    off_t offset = req->op.as_write.offset;

    req->result = pwritev(file->file, (struct iovec*)write_req->data.bufs,
        write_req->data.nbuf, offset);
}

static void _ev_file_on_write_done(ev_threadpool_work_t* work, int status)
{
    ev_file_req_t* req = EV_CONTAINER_OF(work, ev_file_req_t, work_token);
    ev_file_t* file = req->file;

    if (status == EV_ECANCELED)
    {
        assert(req->result == EV_SUCCESS);
        req->result = EV_ECANCELED;
    }

    _ev_file_cleanup_token_as_write(req);
    req->cb(file, req);
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
    if (file->file != EV_OS_FILE_INVALID)
    {
        close(file->file);
        file->file = EV_OS_FILE_INVALID;
    }

    file->close_cb = cb;
    ev__handle_exit(&file->base, 0);
}

int ev_file_open(ev_file_t* file, ev_file_req_t* token, const char* path, int flags, int mode, ev_file_cb cb)
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

int ev_file_read(ev_file_t* file, ev_file_req_t* req, ev_buf_t bufs[],
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

int ev_file_write(ev_file_t* file, ev_file_req_t* req, ev_buf_t bufs[],
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
