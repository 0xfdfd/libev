#include "loop.h"
#include <stdio.h>

static HANDLE _ev_pipe_make_s(const char* name)
{
    DWORD r_open_mode = FILE_FLAG_OVERLAPPED | WRITE_DAC | FILE_FLAG_FIRST_PIPE_INSTANCE | PIPE_ACCESS_INBOUND;
    HANDLE pip_r = CreateNamedPipe(name, r_open_mode,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 65535, 65535, 0, NULL);

    return pip_r;
}

static HANDLE _ev_pipe_make_c(const char* name)
{
    DWORD w_open_mode = GENERIC_WRITE | WRITE_DAC;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof sa;
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = 0;

    HANDLE pip_w = CreateFile(name, w_open_mode, 0, &sa, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    return pip_w;
}

static void _ev_pipe_cancel_r(ev_pipe_t* pipe, ev_read_t* req)
{
    size_t i;
    for (i = 0; i < req->data.nbuf; i++)
    {
        CancelIoEx(pipe->pipfd, &req->backend.io[i].overlapped);
    }
}

static void _ev_pipe_cancel_all_r(ev_pipe_t* pipe)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.stream.r_queue)) != NULL)
    {
        ev_read_t* req = container_of(it, ev_read_t, node);
        _ev_pipe_cancel_r(pipe, req);
    }
}

static void _ev_pipe_cancel_w(ev_pipe_t* pipe, ev_write_t* req)
{
    size_t i;
    for (i = 0; i < req->data.nbuf; i++)
    {
        CancelIoEx(pipe->pipfd, &req->backend.io[i].overlapped);
    }
}

static void _ev_pipe_cancel_all_w(ev_pipe_t* pipe)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&pipe->backend.stream.w_queue)) != NULL)
    {
        ev_write_t* req = container_of(it, ev_write_t, node);
        _ev_pipe_cancel_w(pipe, req);
    }
}

static void _ev_pipe_on_close_win(ev_handle_t* handle)
{
    ev_pipe_t* pipe = container_of(handle, ev_pipe_t, base);

    if (pipe->close_cb != NULL)
    {
        pipe->close_cb(pipe);
    }
}

static int _ev_pipe_is_last_iocp_w(ev_write_t* req, ev_iocp_t* iocp)
{
    ev_iocp_t* last_iocp = &req->backend.io[req->data.nbuf - 1];
    return last_iocp == iocp;
}

static void _ev_pipe_on_write_iocp_success(ev_write_t* req, size_t transferred)
{
    req->backend.size += transferred;

    size_t write_size = req->backend.size;
    ev__write_exit_win(req);

    req->data.cb(req, write_size, EV_SUCCESS);
}

static void _ev_pipe_on_write_iocp_failure(ev_write_t* req, ev_iocp_t* iocp, size_t transferred)
{
    req->backend.size += transferred;

    size_t write_size = req->backend.size;
    int stat = ev__translate_sys_error(ev__ntstatus_to_winsock_error((NTSTATUS)iocp->overlapped.Internal));
    ev__write_exit_win(req);

    req->data.cb(req, write_size, stat);
}

static void _ev_pipe_smart_deactive_win(ev_pipe_t* pipe)
{
    if (ev_list_size(&pipe->backend.stream.r_queue) == 0
        && ev_list_size(&pipe->backend.stream.w_queue) == 0)
    {
        ev__handle_deactive(&pipe->base);
    }
}

static void _ev_pipe_on_write_iocp(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    ev_write_t* req = arg;
    ev_pipe_t* pipe = req->backend.owner;

    /* only handle the last IOCP request */
    if (!_ev_pipe_is_last_iocp_w(req, iocp))
    {
        return;
    }
    /* Remove record */
    ev_list_erase(&pipe->backend.stream.w_queue, &req->node);
    _ev_pipe_smart_deactive_win(pipe);

    if (NT_SUCCESS(iocp->overlapped.Internal))
    {
        _ev_pipe_on_write_iocp_success(req, transferred);
    }
    else
    {
        _ev_pipe_on_write_iocp_failure(req, iocp, transferred);
    }
}

static int _ev_pipe_read_one_buffer(ev_pipe_t* pipe, ev_buf_t* buf, size_t* read_size)
{
    int ret = EV_SUCCESS;
    size_t pos = 0;

    while (pos < buf->size)
    {
        DWORD bytes_read = 0;
        void* read_pos = (uint8_t*)buf->data + pos;
        size_t left_size = buf->size - pos;
        if (!ReadFile(pipe->pipfd, read_pos, (DWORD)left_size, &bytes_read, NULL))
        {
            int err = GetLastError();
            ret = err == ERROR_BROKEN_PIPE ? EV_EOF : ev__translate_sys_error(err);
            goto fin;
        }

        pos += bytes_read;
    }

fin:
    *read_size = pos;
    return ret;
}

static void _ev_pipe_on_read_iocp(ev_iocp_t* iocp, size_t transferred, void* arg)
{
    (void)iocp; (void)transferred;

    ev_read_t* req = arg;
    ev_pipe_t* pipe = req->backend.owner;
    size_t read_size = 0;
    int stat = EV_SUCCESS;

    size_t i;
    for (i = 0; i < req->data.nbuf; i++)
    {
        size_t bytes_read = 0;
        stat = _ev_pipe_read_one_buffer(pipe, &req->data.bufs[i], &bytes_read);
        read_size += bytes_read;
        if (stat != EV_SUCCESS)
        {
            goto fin;
        }
    }

fin:
    ev_list_erase(&pipe->backend.stream.r_queue, &req->node);
    ev__read_exit_win(req);
    _ev_pipe_smart_deactive_win(pipe);
    req->data.cb(req, read_size, stat);
}

int ev_pipe_make(ev_os_handle_t fds[2])
{
    static long volatile s_pipe_serial_no = 0;

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "\\\\.\\pipe\\LOCAL\\libev\\RemoteExeAnon.%08lx.%08lx",
        (long)GetCurrentProcessId(), InterlockedIncrement(&s_pipe_serial_no));

    int err;
    HANDLE pip_r = _ev_pipe_make_s(buffer);
    if (pip_r == INVALID_HANDLE_VALUE)
    {
        err = GetLastError();
        return ev__translate_sys_error(err);
    }

    HANDLE pip_w = _ev_pipe_make_c(buffer);
    if (pip_w == INVALID_HANDLE_VALUE)
    {
        err = GetLastError();
        CloseHandle(pip_r);
        return ev__translate_sys_error(err);
    }

    if (!ConnectNamedPipe(pip_r, NULL))
    {
        err = GetLastError();
        if (err != ERROR_PIPE_CONNECTED)
        {
            CloseHandle(pip_r);
            CloseHandle(pip_w);
            return ev__translate_sys_error(err);
        }
    }

    fds[0] = pip_r;
    fds[1] = pip_w;

    return EV_SUCCESS;
}

int ev_pipe_init(ev_loop_t* loop, ev_pipe_t* pipe)
{
    ev__handle_init(loop, &pipe->base, _ev_pipe_on_close_win);
    pipe->close_cb = NULL;
    pipe->pipfd = EV_OS_HANDLE_INVALID;

    ev_list_init(&pipe->backend.stream.r_queue);
    ev_list_init(&pipe->backend.stream.w_queue);

    return EV_SUCCESS;
}

void ev_pipe_exit(ev_pipe_t* pipe, ev_pipe_cb cb)
{
    if (pipe->pipfd != EV_OS_HANDLE_INVALID)
    {
        _ev_pipe_cancel_all_r(pipe);
        _ev_pipe_cancel_all_w(pipe);

        CloseHandle(pipe->pipfd);
        pipe->pipfd = EV_OS_HANDLE_INVALID;
    }

    pipe->close_cb = cb;
    ev__handle_exit(&pipe->base);
}

int ev_pipe_open(ev_pipe_t* pipe, ev_os_handle_t handle)
{
    if (pipe->pipfd != EV_OS_HANDLE_INVALID)
    {
        return EV_EEXIST;
    }
    if (handle == EV_OS_HANDLE_INVALID)
    {
        return EV_EBADF;
    }

    IO_STATUS_BLOCK io_status;
    FILE_ACCESS_INFORMATION access;
    NTSTATUS nt_status = g_ev_loop_win_ctx.NtQueryInformationFile(handle,
        &io_status, &access, sizeof(access), FileAccessInformation);
    if (nt_status != STATUS_SUCCESS)
    {
        return EV_EINVAL;
    }

    DWORD mode = PIPE_READMODE_BYTE | PIPE_WAIT;
    if (!SetNamedPipeHandleState(handle, &mode, NULL, NULL))
    {
        DWORD err = GetLastError();
        if (err != ERROR_ACCESS_DENIED)
        {
            return EV_EBADF;
        }

        DWORD current_mode = 0;
        if (!GetNamedPipeHandleState(handle, &current_mode, NULL, NULL, NULL, NULL, 0))
        {
            return ev__translate_sys_error(GetLastError());
        }
        if (current_mode & PIPE_NOWAIT)
        {
            return ev__translate_sys_error(ERROR_ACCESS_DENIED);
        }
    }

    FILE_MODE_INFORMATION mode_info;
    nt_status = g_ev_loop_win_ctx.NtQueryInformationFile(handle,
        &io_status, &mode_info, sizeof(mode_info), FileModeInformation);
    if (nt_status != STATUS_SUCCESS)
    {
        return ev__translate_sys_error(GetLastError());
    }

    if (CreateIoCompletionPort(handle, pipe->base.data.loop->backend.iocp, (ULONG_PTR)pipe, 0) == NULL)
    {
        return ev__translate_sys_error(GetLastError());
    }

    pipe->pipfd = handle;

    return EV_SUCCESS;
}

int ev_pipe_write(ev_pipe_t* pipe, ev_write_t* req, ev_buf_t bufs[], size_t nbuf, ev_write_cb cb)
{
    int ret;
    int result;

    ret = ev__write_init_win(req, bufs, nbuf, pipe, EV_EINPROGRESS, _ev_pipe_on_write_iocp, req, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    int flag_failure = 0;
    DWORD err = 0;

    size_t idx;
    for (idx = 0; idx < nbuf; idx++)
    {
        result = WriteFile(pipe->pipfd, bufs[idx].data, bufs[idx].size, NULL,
            &req->backend.io[idx].overlapped);

        /* write success */
        if (result)
        {
            continue;
        }

        /* ERROR_IO_PENDING is not a failure */
        err = GetLastError();
        if (err == ERROR_IO_PENDING)
        {
            continue;
        }

        flag_failure = 1;
        break;
    }

    if (flag_failure)
    {
        size_t i;
        for (i = 0; i <= idx; i++)
        {
            CancelIoEx(pipe->pipfd, &req->backend.io[i].overlapped);
        }
        return ev__translate_sys_error(err);
    }

    ev__handle_active(&pipe->base);
    ev_list_push_back(&pipe->backend.stream.w_queue, &req->node);
    return EV_SUCCESS;
}

int ev_pipe_read(ev_pipe_t* pipe, ev_read_t* req, ev_buf_t bufs[], size_t nbuf, ev_read_cb cb)
{
    static char s_ev_zero[] = "";

    int ret = ev__read_init_win(req, bufs, nbuf, pipe, EV_EINPROGRESS, _ev_pipe_on_read_iocp, req, cb);
    if (ret != EV_SUCCESS)
    {
        return ret;
    }

    int result = ReadFile(pipe->pipfd, s_ev_zero, 0, NULL, &req->backend.io[0].overlapped);
    int err = GetLastError();
    if (!result && err != ERROR_IO_PENDING)
    {
        return ev__translate_sys_error(err);
    }

    ev__handle_active(&pipe->base);
    ev_list_push_back(&pipe->backend.stream.r_queue, &req->node);
    return EV_SUCCESS;
}
