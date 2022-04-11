#include "ev-common.h"
#include "file-common.h"

/**
 * @brief Check if \p file have pending task.
 * @return  bool
 */
static int _ev_file_have_pending(ev_file_t* file)
{
    return ev_list_size(&file->work_queue) != 0;
}

static void _ev_file_do_close_callback(ev_file_t* file)
{
    if (file->close_cb != NULL)
    {
        file->close_cb(file);
    }
}

static void _ev_file_do_close_callback_if_necessary(ev_file_t* file)
{
    if (ev__handle_is_closing(&file->base) && !_ev_file_have_pending(file))
    {
        _ev_file_do_close_callback(file);
    }
}

static void _ev_file_on_close(ev_handle_t* handle)
{
    ev_file_t* file = EV_CONTAINER_OF(handle, ev_file_t, base);
    _ev_file_do_close_callback_if_necessary(file);
}

static void _ev_file_cancel_all_pending_task(ev_file_t* file)
{
    ev_list_node_t* it = ev_list_begin(&file->work_queue);
    for (; it != NULL; it = ev_list_next(it))
    {
        ev_fs_req_t* req = EV_CONTAINER_OF(it, ev_fs_req_t, node);
        ev_threadpool_cancel(&req->work_token);
    }
}

static void _ev_file_on_open(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;

    req->result = ev__fs_open(&file->file, req->req.as_open.path,
        req->req.as_open.flags, req->req.as_open.mode);
}

static void _ev_file_exit_token(ev_fs_req_t* req)
{
    ev_file_t* file = req->file;
    ev_list_erase(&file->work_queue, &req->node);
}

static void _ev_file_cleanup_token_as_open(ev_fs_req_t* token)
{
    _ev_file_exit_token(token);

    if (token->req.as_open.path != NULL)
    {
        ev__free(token->req.as_open.path);
        token->req.as_open.path = NULL;
    }
}

static void _ev_file_init_token(ev_fs_req_t* req, ev_file_t* file, ev_file_cb cb)
{
    req->cb = cb;
    req->file = file;
    req->result = EV_EINPROGRESS;

    ev_list_push_back(&file->work_queue, &req->node);
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

static void _ev_file_smart_deactive(ev_file_t* file)
{
    if (ev_list_size(&file->work_queue) == 0)
    {
        ev__handle_deactive(&file->base);
    }

    _ev_file_do_close_callback_if_necessary(file);
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

    _ev_file_smart_deactive(file);
}

static void _ev_file_on_open_done(ev_threadpool_work_t* work, int status)
{
    _ev_file_on_done(work, status, _ev_file_cleanup_token_as_open);
}

static void _ev_file_on_read(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;
    ev_read_t* read_req = &req->req.as_read.read_req;

    req->result = ev__fs_preadv(file->file, read_req->data.bufs,
        read_req->data.nbuf, req->req.as_read.offset);
}

static void _ev_file_cleanup_token_as_read(ev_fs_req_t* req)
{
    _ev_file_exit_token(req);
    ev__read_exit(&req->req.as_read.read_req);
}

static void _ev_file_on_read_done(ev_threadpool_work_t* work, int status)
{
    _ev_file_on_done(work, status, _ev_file_cleanup_token_as_read);
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

static void _ev_file_on_write(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;
    ev_write_t* write_req = &req->req.as_write.write_req;

    req->result = ev__fs_pwritev(file->file, write_req->data.bufs,
        write_req->data.nbuf, req->req.as_write.offset);
}

static void _ev_file_cleanup_token_as_write(ev_fs_req_t* req)
{
    _ev_file_exit_token(req);
    ev__write_exit(&req->req.as_write.write_req);
}

static void _ev_file_on_write_done(ev_threadpool_work_t* work, int status)
{
    _ev_file_on_done(work, status, _ev_file_cleanup_token_as_write);
}

static void _ev_file_on_fstat(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;

    req->result = ev__fs_fstat(file->file, &req->rsp.as_fstat.fileinfo);
}

static void _ev_file_on_fstat_done(ev_threadpool_work_t* work, int status)
{
    _ev_file_on_done(work, status, _ev_file_exit_token);
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
    ev_list_init(&file->work_queue);

    return EV_SUCCESS;
}

void ev_file_exit(ev_file_t* file, ev_file_close_cb cb)
{
    _ev_file_cancel_all_pending_task(file);

    /* It should be safe to close handle. */
    if (file->file != EV_OS_FILE_INVALID)
    {
        ev__fs_close(file->file);
        file->file = EV_OS_FILE_INVALID;
    }

    file->close_cb = cb;
    ev__handle_exit(&file->base, 0);
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

    ev__handle_active(&file->base);

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_read, _ev_file_on_read_done);
    if (ret != EV_SUCCESS)
    {
        _ev_file_cleanup_token_as_read(req);
        return ret;
    }

    return EV_SUCCESS;
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

    ev__handle_active(&file->base);

    ret = ev_threadpool_submit(pool, loop, &token->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_open, _ev_file_on_open_done);
    if (ret != EV_SUCCESS)
    {
        _ev_file_cleanup_token_as_open(token);
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

    ev__handle_active(&file->base);

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
    ev__handle_active(&file->base);

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_fstat, _ev_file_on_fstat_done);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    return EV_SUCCESS;
}

ev_file_stat_t* ev_fs_get_statbuf(ev_fs_req_t* req)
{
    return &req->rsp.as_fstat.fileinfo;
}
