#include "ev-common.h"

typedef struct file_open_info_win_s
{
    DWORD   access;
    DWORD   share;
    DWORD   attributes;
    DWORD   disposition;
}file_open_info_win_t;

static void _ev_file_on_close_win(ev_handle_t* handle)
{
    ev_file_t* file = EV_CONTAINER_OF(handle, ev_file_t, base);
    if (file->close_cb != NULL)
    {
        file->close_cb(file);
    }
}

static int _ev_file_init_token_as_open_win(ev_file_req_t* token, ev_file_t* file,
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

static void _ev_file_cleanup_token_as_open_win(ev_file_req_t* token)
{
    if (token->op.as_open.path != NULL)
    {
        ev__free(token->op.as_open.path);
        token->op.as_open.path = NULL;
    }
}

static int _ev_file_get_open_attributes(ev_file_req_t* req, file_open_info_win_t* info)
{
    info->access = 0;
    info->share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    info->attributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS;
    info->disposition = 0;

    int flags = req->op.as_open.flags;
    int mode = req->op.as_open.mode;

    switch (flags & (EV_FS_O_RDONLY | EV_FS_O_WRONLY | EV_FS_O_RDWR))
    {
    case EV_FS_O_RDONLY:
        info->access = FILE_GENERIC_READ;
        break;
    case EV_FS_O_WRONLY:
        info->access = FILE_GENERIC_WRITE;
        break;
    case EV_FS_O_RDWR:
        info->access = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
        break;
    default:
        return EV_EINVAL;
    }

    if (flags & EV_FS_O_APPEND)
    {
        info->access &= ~FILE_WRITE_DATA;
        info->access |= FILE_APPEND_DATA;
    }

    switch (flags & (EV_FS_O_CREAT | EV_FS_O_EXCL | EV_FS_O_TRUNC))
    {
    case 0:
    case EV_FS_O_EXCL:
        info->disposition = OPEN_EXISTING;
        break;
    case EV_FS_O_CREAT:
        info->disposition = OPEN_ALWAYS;
        break;
    case EV_FS_O_CREAT | EV_FS_O_EXCL:
    case EV_FS_O_CREAT | EV_FS_O_TRUNC | EV_FS_O_EXCL:
        info->disposition = CREATE_NEW;
        break;
    case EV_FS_O_TRUNC:
    case EV_FS_O_TRUNC | EV_FS_O_EXCL:
        info->disposition = TRUNCATE_EXISTING;
        break;
    case EV_FS_O_CREAT | EV_FS_O_TRUNC:
        info->disposition = CREATE_ALWAYS;
        break;
    default:
        return EV_EINVAL;
    }

    if ((flags & EV_FS_O_CREAT) && !(mode & _S_IWRITE))
    {
        info->attributes |= FILE_ATTRIBUTE_READONLY;
    }

    switch (flags & (EV_FS_O_DSYNC | EV_FS_O_SYNC))
    {
    case 0:
        break;
    case EV_FS_O_DSYNC:
#if EV_FS_O_SYNC != EV_FS_O_DSYNC
    case EV_FS_O_SYNC:
#endif
        info->attributes |= FILE_FLAG_WRITE_THROUGH;
        break;
    default:
        return EV_EINVAL;
    }

    return EV_SUCCESS;
}

static void _ev_file_on_open_win(ev_threadpool_work_t* work)
{
    ev_file_req_t* req = EV_CONTAINER_OF(work, ev_file_req_t, work_token);
    ev_file_t* file = req->file;
    int flags = req->op.as_open.flags;

    file_open_info_win_t info;
    req->result = _ev_file_get_open_attributes(req, &info);
    if (req->result != EV_SUCCESS)
    {
        return;
    }

    const char* path = req->op.as_open.path;
    file->file = CreateFile(path, info.access, info.share, NULL, info.disposition, info.attributes, NULL);
    if (file->file == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_EXISTS && (flags & EV_FS_O_CREAT) && !(flags & EV_FS_O_EXCL))
        {
            req->result = EV_EISDIR;
        }
        else
        {
            req->result = ev__translate_sys_error(err);
        }
        return;
    }
}

static void _ev_file_on_open_done_win(ev_threadpool_work_t* work, int status)
{
    ev_file_req_t* req = EV_CONTAINER_OF(work, ev_file_req_t, work_token);
    ev_file_t* file = req->file;

    if (status == EV_ECANCELED)
    {
        assert(req->result == EV_SUCCESS);
        req->result = EV_ECANCELED;
    }

    _ev_file_cleanup_token_as_open_win(req);
    req->cb(file, req);
}

static int _ev_file_init_token_as_read_win(ev_file_req_t* req, ev_file_t* file,
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

static void _ev_file_cleanup_token_as_read_win(ev_file_req_t* req)
{
    ev__read_exit(&req->op.as_read.read_req);
}

static void _ev_file_on_read_win(ev_threadpool_work_t* work)
{
    ev_file_req_t* req = EV_CONTAINER_OF(work, ev_file_req_t, work_token);
    ev_file_t* file = req->file;
    ev_read_t* read_req = &req->op.as_read.read_req;
    ssize_t offset = req->op.as_read.offset;

    if (file->file == INVALID_HANDLE_VALUE)
    {
        req->result = ev__translate_sys_error(ERROR_INVALID_HANDLE);
        return;
    }

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));

    size_t idx;
    LARGE_INTEGER offset_;
    DWORD bytes = 0;
    BOOL read_ret = TRUE;
    for (idx = 0; read_ret && idx < read_req->data.nbuf; idx++)
    {
        offset_.QuadPart = offset + bytes;
        overlapped.Offset = offset_.LowPart;
        overlapped.OffsetHigh = offset_.HighPart;

        DWORD incremental_bytes;
        read_ret = ReadFile(file->file, read_req->data.bufs[idx].data,
            read_req->data.bufs[idx].size, &incremental_bytes, &overlapped);
        bytes += incremental_bytes;
    }

    req->result = bytes;
    if (read_ret || bytes > 0)
    {
        return;
    }

    DWORD err = GetLastError();
    if (err == ERROR_HANDLE_EOF || err == ERROR_BROKEN_PIPE)
    {
        return;
    }

    if (err == ERROR_ACCESS_DENIED)
    {
        err = ERROR_INVALID_FLAGS;
    }
    req->result = ev__translate_sys_error(err);
}

static void _ev_file_on_read_done_win(ev_threadpool_work_t* work, int status)
{
    ev_file_req_t* req = EV_CONTAINER_OF(work, ev_file_req_t, work_token);
    ev_file_t* file = req->file;

    if (status == EV_ECANCELED)
    {
        assert(req->result == EV_SUCCESS);
        req->result = EV_ECANCELED;
    }

    _ev_file_cleanup_token_as_read_win(req);
    req->cb(file, req);
}

static int _ev_file_init_token_as_write_win(ev_file_req_t* token, ev_file_t* file,
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

static void _ev_file_cleanup_token_as_write_win(ev_file_req_t* req)
{
    ev__write_exit(&req->op.as_write.write_req);
}

static void _ev_file_on_write_win(ev_threadpool_work_t* work)
{
    ev_file_req_t* req = EV_CONTAINER_OF(work, ev_file_req_t, work_token);
    ev_file_t* file = req->file;
    ev_write_t* write_req = &req->op.as_write.write_req;
    ssize_t offset = req->op.as_write.offset;

    if (file->file == INVALID_HANDLE_VALUE)
    {
        req->result = ev__translate_sys_error(ERROR_INVALID_HANDLE);
        return;
    }

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));

    size_t idx;
    LARGE_INTEGER offset_;
    DWORD bytes = 0;
    BOOL write_ret = TRUE;
    for (idx = 0; write_ret && idx < write_req->data.nbuf; idx++)
    {
        offset_.QuadPart = offset + bytes;
        overlapped.Offset = offset_.LowPart;
        overlapped.OffsetHigh = offset_.HighPart;

        DWORD incremental_bytes;
        write_ret = WriteFile(file->file, write_req->data.bufs[idx].data,
            write_req->data.bufs[idx].size, &incremental_bytes, &overlapped);
        bytes += incremental_bytes;
    }

    req->result = bytes;
    if (write_ret || bytes > 0)
    {
        return;
    }

    DWORD err = GetLastError();
    if (err == ERROR_ACCESS_DENIED)
    {
        err = ERROR_INVALID_FLAGS;
    }
    req->result = ev__translate_sys_error(err);
}

static void _ev_file_on_write_done_win(ev_threadpool_work_t* work, int status)
{
    ev_file_req_t* req = EV_CONTAINER_OF(work, ev_file_req_t, work_token);
    ev_file_t* file = req->file;

    if (status == EV_ECANCELED)
    {
        assert(req->result == EV_SUCCESS);
        req->result = EV_ECANCELED;
    }

    _ev_file_cleanup_token_as_write_win(req);
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
    ev__handle_init(loop, &file->base, EV_ROLE_EV_FILE, _ev_file_on_close_win);

    return EV_SUCCESS;
}

void ev_file_exit(ev_file_t* file, ev_file_close_cb cb)
{
    if (file->file != EV_OS_FILE_INVALID)
    {
        CloseHandle(file->file);
        file->file = EV_OS_FILE_INVALID;
    }

    file->close_cb = cb;
    ev__handle_exit(&file->base, 0);
}

int ev_file_open(ev_file_t* file, ev_file_req_t* req, const char* path,
    int flags, int mode, ev_file_cb cb)
{
    int ret;
    ev_loop_t* loop = file->base.data.loop;
    ev_threadpool_t* pool = loop->threadpool;

    ret = _ev_file_init_token_as_open_win(req, file, path, flags, mode, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_open_win, _ev_file_on_open_done_win);
    if (ret != EV_SUCCESS)
    {
        _ev_file_cleanup_token_as_open_win(req);
        return ret;
    }

    return EV_SUCCESS;
}

int ev_file_read(ev_file_t* file, ev_file_req_t* req, ev_buf_t bufs[],
    size_t nbuf, ssize_t offset, ev_file_cb cb)
{
    int ret;
    ev_loop_t* loop = file->base.data.loop;
    ev_threadpool_t* pool = loop->threadpool;

    ret = _ev_file_init_token_as_read_win(req, file, bufs, nbuf, offset, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_read_win, _ev_file_on_read_done_win);
    if (ret != EV_SUCCESS)
    {
        _ev_file_cleanup_token_as_read_win(req);
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

    ret = _ev_file_init_token_as_write_win(req, file, bufs, nbuf, offset, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    ret = ev_threadpool_submit(pool, loop, &req->work_token, EV_THREADPOOL_WORK_IO_FAST,
        _ev_file_on_write_win, _ev_file_on_write_done_win);
    if (ret != EV_SUCCESS)
    {
        _ev_file_cleanup_token_as_write_win(req);
        return ret;
    }

    return EV_SUCCESS;
}
