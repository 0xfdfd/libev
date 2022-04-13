#include "ev-common.h"
#include "fs-common.h"

typedef struct ev_dirent_record_s
{
    ev_list_node_t      node;   /**< List node */
    ev_dirent_t         data;   /**< Dirent info */
}ev_dirent_record_t;

static void _ev_fs_erase_req(ev_file_t* file, ev_fs_req_t* req)
{
    ev_list_erase(&file->work_queue, &req->node);
}

static void _ev_fs_cleanup_req_as_open(ev_fs_req_t* token)
{
    if (token->req.as_open.path != NULL)
    {
        ev__free(token->req.as_open.path);
        token->req.as_open.path = NULL;
    }
}

static void _ev_fs_cleanup_req_as_read(ev_fs_req_t* req)
{
    ev__read_exit(&req->req.as_read.read_req);
}

static void _ev_fs_cleanup_req_as_write(ev_fs_req_t* req)
{
    ev__write_exit(&req->req.as_write.write_req);
}

static void _ev_fs_cleanup_req_as_fstat(ev_fs_req_t* req)
{
    (void)req;
}

static void _ev_fs_cleanup_req_as_readdir(ev_fs_req_t* req)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&req->rsp.dirents)) != NULL)
    {
        ev_dirent_record_t* rec = EV_CONTAINER_OF(it, ev_dirent_record_t, node);
        ev__free((char*)rec->data.name);
        ev__free(rec);
    }

    if (req->req.as_readdir.path != NULL)
    {
        ev__free(req->req.as_readdir.path);
        req->req.as_readdir.path = NULL;
    }
}

static void _ev_fs_cleanup_req_as_readfile(ev_fs_req_t* req)
{
    if (req->req.as_readfile.path != NULL)
    {
        ev__free(req->req.as_readfile.path);
        req->req.as_readfile.path = NULL;
    }

    if (req->rsp.filecontent.data != NULL)
    {
        ev__free(req->rsp.filecontent.data);
        req->rsp.filecontent.data = NULL;
    }
    req->rsp.filecontent.size = 0;
}

static void _ev_fs_cleanup_req_as_mkdir(ev_fs_req_t* req)
{
    if (req->req.as_mkdir.path != NULL)
    {
        ev__free(req->req.as_mkdir.path);
        req->req.as_mkdir.path = NULL;
    }
}

static void _ev_fs_init_req(ev_fs_req_t* req, ev_file_t* file, ev_file_cb cb, ev_fs_req_type_t type)
{
    req->req_type = type;
    req->cb = cb;
    req->file = file;
    req->result = EV_EINPROGRESS;

    if (file != NULL)
    {
        ev_list_push_back(&file->work_queue, &req->node);
    }
}

static int _ev_fs_init_req_as_open(ev_fs_req_t* token, ev_file_t* file,
    const char* path, int flags, int mode, ev_file_cb cb)
{
    _ev_fs_init_req(token, file, cb, EV_FS_REQ_OPEN);

    if ((token->req.as_open.path = ev__strdup(path)) == NULL)
    {
        return EV_ENOMEM;
    }

    token->req.as_open.flags = flags;
    token->req.as_open.mode = mode;

    return EV_SUCCESS;
}

static int _ev_fs_init_req_as_read(ev_fs_req_t* req, ev_file_t* file,
    ev_buf_t bufs[], size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    _ev_fs_init_req(req, file, cb, EV_FS_REQ_READ);

    int ret = ev__read_init(&req->req.as_read.read_req, bufs, nbuf);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }
    req->req.as_read.offset = offset;

    return EV_SUCCESS;
}

static int _ev_fs_init_req_as_write(ev_fs_req_t* token, ev_file_t* file,
    ev_buf_t bufs[], size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    _ev_fs_init_req(token, file, cb, EV_FS_REQ_WRITE);

    int ret = ev__write_init(&token->req.as_write.write_req, bufs, nbuf);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }
    token->req.as_write.offset = offset;

    return EV_SUCCESS;
}

static void _ev_fs_init_req_as_fstat(ev_fs_req_t* req, ev_file_t* file, ev_file_cb cb)
{
    _ev_fs_init_req(req, file, cb, EV_FS_REQ_FSTAT);
}

static int _ev_fs_init_req_as_readdir(ev_fs_req_t* req, const char* path, ev_file_cb cb)
{
    _ev_fs_init_req(req, NULL, cb, EV_FS_REQ_READDIR);
    req->req.as_readdir.path = ev__strdup(path);
    if (req->req.as_readdir.path == NULL)
    {
        return EV_ENOMEM;
    }

    ev_list_init(&req->rsp.dirents);

    return EV_SUCCESS;
}

static int _ev_fs_init_req_as_readfile(ev_fs_req_t* req, const char* path,
    ev_file_cb cb)
{
    _ev_fs_init_req(req, NULL, cb, EV_FS_REQ_READFILE);

    req->req.as_readfile.path = ev__strdup(path);
    if (req->req.as_readfile.path == NULL)
    {
        return EV_ENOMEM;
    }

    req->rsp.filecontent = ev_buf_make(NULL, 0);

    return EV_SUCCESS;
}

static int _ev_fs_init_req_as_mkdir(ev_fs_req_t* req, const char* path, int mode,
    ev_file_cb cb)
{
    _ev_fs_init_req(req, NULL, cb, EV_FS_REQ_MKDIR);

    req->req.as_mkdir.path = ev__strdup(path);
    if (req->req.as_mkdir.path == NULL)
    {
        return EV_ENOMEM;
    }
    req->req.as_mkdir.mode = mode;

    return EV_SUCCESS;
}

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

static void _ev_file_smart_deactive(ev_file_t* file)
{
    if (ev_list_size(&file->work_queue) == 0)
    {
        ev__handle_deactive(&file->base);
    }

    _ev_file_do_close_callback_if_necessary(file);
}

static int _ev_fs_on_readdir_entry(ev_dirent_t* info, void* arg)
{
    ev_fs_req_t* req = arg;

    ev_dirent_record_t* rec = ev__malloc(sizeof(ev_dirent_record_t));
    if (rec == NULL)
    {
        goto err_nomem;
    }

    rec->data.type = info->type;
    rec->data.name = ev__strdup(info->name);
    if (rec->data.name == NULL)
    {
        goto err_nomem;
    }

    ev_list_push_back(&req->rsp.dirents, &rec->node);

    return 0;

err_nomem:
    req->result = EV_ENOMEM;
    return -1;
}

static void _ev_fs_on_done(ev_threadpool_work_t* work, int status)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;

    if (status == EV_ECANCELED)
    {
        assert(req->result == EV_SUCCESS);
        req->result = EV_ECANCELED;
    }

    if (file != NULL)
    {
        _ev_fs_erase_req(file, req);
        _ev_file_smart_deactive(file);
    }
    req->cb(req);
}

static void _ev_file_on_open(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;

    req->result = ev__fs_open(&file->file, req->req.as_open.path,
        req->req.as_open.flags, req->req.as_open.mode);
}

static void _ev_file_on_read(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;
    ev_read_t* read_req = &req->req.as_read.read_req;

    req->result = ev__fs_preadv(file->file, read_req->data.bufs,
        read_req->data.nbuf, req->req.as_read.offset);
}

static void _ev_file_on_write(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;
    ev_write_t* write_req = &req->req.as_write.write_req;

    req->result = ev__fs_pwritev(file->file, write_req->data.bufs,
        write_req->data.nbuf, req->req.as_write.offset);
}

static void _ev_file_on_fstat(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    ev_file_t* file = req->file;

    req->result = ev__fs_fstat(file->file, &req->rsp.fileinfo);
}

static void _ev_fs_on_readdir(ev_threadpool_work_t* work)
{
    int ret;
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);

    req->result = EV_SUCCESS;
    ret = ev__fs_readdir(req->req.as_readdir.path,
        _ev_fs_on_readdir_entry, req);

    if (req->result == EV_SUCCESS)
    {
        req->result = ret;
    }
}

static void _ev_fs_on_readfile(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    const char* path = req->req.as_readfile.path;

    ev_os_file_t file;
    req->result = ev__fs_open(&file, path, EV_FS_O_RDONLY, 0);
    if (req->result != EV_SUCCESS)
    {
        return;
    }

    ev_fs_stat_t statbuf;
    req->result = ev__fs_fstat(file, &statbuf);
    if (req->result != EV_SUCCESS)
    {
        goto close_file;
    }

    void* data = ev__malloc(statbuf.st_size);
    req->rsp.filecontent = ev_buf_make(data, statbuf.st_size);

    if (req->rsp.filecontent.data == NULL)
    {
        req->result = EV_ENOMEM;
        goto close_file;
    }

    req->result = ev__fs_preadv(file, &req->rsp.filecontent, 1, 0);

close_file:
    ev__fs_close(file);
}

static void _ev_fs_on_mkdir(ev_threadpool_work_t* work)
{
    ev_fs_req_t* req = EV_CONTAINER_OF(work, ev_fs_req_t, work_token);
    const char* path = req->req.as_mkdir.path;
    int mode = req->req.as_mkdir.mode;

    req->result = ev__fs_mkdir(path, mode);
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

int ev_file_open(ev_file_t* file, ev_fs_req_t* token, const char* path, int flags, int mode, ev_file_cb cb)
{
    ev_loop_t* loop = file->base.data.loop;
    ev_threadpool_t* pool = loop->threadpool;

    int ret = _ev_fs_init_req_as_open(token, file, path, flags, mode, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ev__handle_active(&file->base);

    ret = ev_threadpool_submit(pool, loop, &token->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_open, _ev_fs_on_done);
    if (ret != EV_SUCCESS)
    {
        _ev_fs_cleanup_req_as_open(token);
        return ret;
    }

    return EV_SUCCESS;
}

int ev_file_read(ev_file_t* file, ev_fs_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    ev_loop_t* loop = file->base.data.loop;
    ev_threadpool_t* pool = loop->threadpool;

    int ret = _ev_fs_init_req_as_read(req, file, bufs, nbuf, offset, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ev__handle_active(&file->base);

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_read, _ev_fs_on_done);
    if (ret != EV_SUCCESS)
    {
        _ev_fs_cleanup_req_as_read(req);
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

    ret = _ev_fs_init_req_as_write(req, file, bufs, nbuf, offset, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ev__handle_active(&file->base);

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_write, _ev_fs_on_done);
    if (ret != EV_SUCCESS)
    {
        _ev_fs_cleanup_req_as_write(req);
        return ret;
    }

    return EV_SUCCESS;
}

int ev_file_stat(ev_file_t* file, ev_fs_req_t* req, ev_file_cb cb)
{
    int ret;
    ev_loop_t* loop = file->base.data.loop;
    ev_threadpool_t* pool = loop->threadpool;

    _ev_fs_init_req_as_fstat(req, file, cb);
    ev__handle_active(&file->base);

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_fstat, _ev_fs_on_done);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    return EV_SUCCESS;
}

int ev_fs_readdir(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
    ev_file_cb cb)
{
    int ret;
    ev_threadpool_t* pool = loop->threadpool;

    if (pool == NULL)
    {
        return EV_ENOTHREADPOOL;
    }

    ret = _ev_fs_init_req_as_readdir(req, path, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_fs_on_readdir, _ev_fs_on_done);
    if (ret != EV_SUCCESS)
    {
        _ev_fs_cleanup_req_as_readdir(req);
        return ret;
    }

    return EV_SUCCESS;
}

int ev_fs_readfile(ev_loop_t* loop, ev_fs_req_t* req, const char* path,
    ev_file_cb cb)
{
    int ret;
    ev_threadpool_t* pool = loop->threadpool;

    if (pool == NULL)
    {
        return EV_ENOTHREADPOOL;
    }

    ret = _ev_fs_init_req_as_readfile(req, path, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_fs_on_readfile, _ev_fs_on_done);
    if (ret != EV_SUCCESS)
    {
        _ev_fs_cleanup_req_as_readfile(req);
        return ret;
    }

    return EV_SUCCESS;
}

int ev_fs_mkdir(ev_loop_t* loop, ev_fs_req_t* req, const char* path, int mode,
    ev_file_cb cb)
{
    int ret;
    ev_threadpool_t* pool = loop->threadpool;

    if (pool == NULL)
    {
        return EV_ENOTHREADPOOL;
    }

    ret = _ev_fs_init_req_as_mkdir(req, path, mode, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_fs_on_mkdir, _ev_fs_on_done);
    if (ret != EV_SUCCESS)
    {
        _ev_fs_cleanup_req_as_mkdir(req);
        return ret;
    }

    return EV_SUCCESS;
}

ev_fs_stat_t* ev_fs_get_statbuf(ev_fs_req_t* req)
{
    return &req->rsp.fileinfo;
}

void ev_fs_req_cleanup(ev_fs_req_t* req)
{
    switch (req->req_type)
    {
    case EV_FS_REQ_OPEN:
        _ev_fs_cleanup_req_as_open(req);
        break;

    case EV_FS_REQ_READ:
        _ev_fs_cleanup_req_as_read(req);
        break;

    case EV_FS_REQ_WRITE:
        _ev_fs_cleanup_req_as_write(req);
        break;

    case EV_FS_REQ_FSTAT:
        _ev_fs_cleanup_req_as_fstat(req);
        break;

    case EV_FS_REQ_READDIR:
        _ev_fs_cleanup_req_as_readdir(req);
        break;

    case EV_FS_REQ_READFILE:
        _ev_fs_cleanup_req_as_readfile(req);
        break;

    case EV_FS_REQ_MKDIR:
        _ev_fs_cleanup_req_as_mkdir(req);
        break;

    default:
        abort();
    }
}

ev_file_t* ev_fs_get_file(ev_fs_req_t* req)
{
    return req->file;
}

ev_dirent_t* ev_fs_get_first_dirent(ev_fs_req_t* req)
{
    ev_list_node_t* it = ev_list_begin(&req->rsp.dirents);
    if (it == NULL)
    {
        return NULL;
    }

    ev_dirent_record_t* rec = EV_CONTAINER_OF(it, ev_dirent_record_t, node);
    return &rec->data;
}

ev_dirent_t* ev_fs_get_next_dirent(ev_dirent_t* curr)
{
    if (curr == NULL)
    {
        return NULL;
    }

    ev_dirent_record_t* rec = EV_CONTAINER_OF(curr, ev_dirent_record_t, data);

    ev_list_node_t* next_node = ev_list_next(&rec->node);
    if (next_node == NULL)
    {
        return NULL;
    }

    ev_dirent_record_t* next_rec = EV_CONTAINER_OF(next_node, ev_dirent_record_t, node);
    return &next_rec->data;
}

ev_buf_t* ev_fs_get_filecontent(ev_fs_req_t* req)
{
    return &req->rsp.filecontent;
}
